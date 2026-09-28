/*
    SPDX-FileCopyrightText: 2007 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "fmotf.h"
#include "fontmatrix_debug.h"

#include <hb-ft.h>

#include <QSet>
#include <QVarLengthArray>

#include <algorithm>

#include FT_MULTIPLE_MASTERS_H

QList<int> FMOtf::altGlyphs;

namespace
{
// Glyph indices handed over as "characters" live here, in a private use
// plane: nothing in it is a default ignorable, a mark or a joiner, so
// HarfBuzz leaves them alone.
const hb_codepoint_t GlyphIndexBase = 0xF0000;

hb_tag_t tagOf(const QString &name)
{
    // a short name is padded with spaces, "lao" is 'lao '
    const QByteArray n(name.toLatin1());
    return hb_tag_from_string(n.constData(), n.size());
}

QString nameOf(hb_tag_t tag)
{
    // always four characters, trailing spaces kept: saved settings hold them
    char buf[4];
    hb_tag_to_string(tag, buf);
    return QString::fromLatin1(buf, 4);
}

bool isDefaultLanguage(const QString &lang)
{
    return lang.isEmpty() || lang == QLatin1String("dflt") || lang == QLatin1String("default");
}

// The enumerating calls of HarfBuzz hand out their result by pages
template<typename Getter>
QList<hb_tag_t> allTags(Getter get)
{
    const unsigned int pageSize(32);
    QList<hb_tag_t> ret;
    hb_tag_t page[pageSize];
    unsigned int offset(0);
    unsigned int count(pageSize);
    while (count == pageSize) {
        get(offset, &count, page);
        for (unsigned int i(0); i < count; ++i)
            ret << page[i];
        offset += count;
    }
    return ret;
}

hb_bool_t glyphByFreeType(hb_font_t *, void *fontData, hb_codepoint_t unicode, hb_codepoint_t *glyph, void *)
{
    *glyph = FT_Get_Char_Index(static_cast<FT_Face>(fontData), unicode);
    return *glyph != 0;
}

hb_bool_t glyphIsGiven(hb_font_t *, void *, hb_codepoint_t unicode, hb_codepoint_t *glyph, void *)
{
    if (unicode < GlyphIndexBase)
        return false;
    *glyph = unicode - GlyphIndexBase;
    return true;
}

// A font that maps characters its own way and leaves the rest to its parent
hb_font_t *subFont(hb_font_t *parent, hb_font_get_nominal_glyph_func_t mapper, void *fontData)
{
    hb_font_t *font(hb_font_create_sub_font(parent));
    hb_font_funcs_t *funcs(hb_font_funcs_create());
    hb_font_funcs_set_nominal_glyph_func(funcs, mapper, nullptr, nullptr);
    hb_font_set_funcs(font, funcs, fontData, nullptr);
    hb_font_funcs_destroy(funcs);
    return font;
}
}

// fontitem.cpp declares these itself
QString OTF_tag_name(unsigned int tag)
{
    return nameOf(tag);
}

unsigned int OTF_name_tag(QString s)
{
    return tagOf(s);
}

FMOtf::FMOtf(FT_Face f, double)
    : _face(f)
    , hbFace(nullptr)
    , hbFont(nullptr)
    , hbGlyphFont(nullptr)
    , GSUB(false)
    , GPOS(false)
{
    // The tables are read through FreeType, which keeps the face alive for us
    hbFace = hb_ft_face_create_referenced(_face);
    // No size is set: the scale of a new font is the units per em of its face,
    // so that advances and offsets come out in font units
    hb_font_t *base(hb_font_create(hbFace));
    hb_ot_font_set_funcs(base);
    // a variable font is shaped where the face is set (FontItem::setVariationCoordinates())
    const unsigned int axisCount = hb_ot_var_get_axis_count(hbFace);
    if (axisCount > 0) {
        QVarLengthArray<FT_Fixed, 8> fixed(axisCount);
        if (FT_Get_Var_Design_Coordinates(_face, axisCount, fixed.data()) == 0) {
            QVarLengthArray<float, 8> coords;
            for (const FT_Fixed f : fixed)
                coords.append(static_cast<float>(f / 65536.0));
            hb_font_set_var_coords_design(base, coords.data(), axisCount);
        }
    }
    hbFont = subFont(base, glyphByFreeType, _face);
    hbGlyphFont = subFont(base, glyphIsGiven, nullptr);
    hb_font_destroy(base);

    GSUB = hb_ot_layout_has_substitution(hbFace);
    GPOS = hb_ot_layout_has_positioning(hbFace);

    QSet<hb_tag_t> seen;
    for (const hb_tag_t table : {HB_OT_TAG_GSUB, HB_OT_TAG_GPOS}) {
        const QList<hb_tag_t> tags(allTags([&](unsigned int offset, unsigned int *count, hb_tag_t *page) {
            hb_ot_layout_table_get_feature_tags(hbFace, table, offset, count, page);
        }));
        for (const hb_tag_t t : tags) {
            if (!seen.contains(t)) {
                seen << t;
                fontFeatures << t;
            }
        }
    }
    // HarfBuzz falls back on a 'kern' table when GPOS does not kern
    const hb_tag_t kern(HB_TAG('k', 'e', 'r', 'n'));
    if (!seen.contains(kern))
        fontFeatures << kern;
}

FMOtf::~FMOtf()
{
    hb_font_destroy(hbGlyphFont);
    hb_font_destroy(hbFont);
    hb_face_destroy(hbFace);
}

int FMOtf::get_glyph(int index)
{
    return lastGlyphs.value(index);
}

QList<hb_feature_t> FMOtf::featureList(const QStringList &enabled) const
{
    QSet<hb_tag_t> on;
    for (const QString &name : enabled)
        on << tagOf(name);

    QList<hb_feature_t> ret;
    QSet<hb_tag_t> done;
    const auto add = [&](hb_tag_t tag) {
        if (done.contains(tag))
            return;
        done << tag;
        hb_feature_t f;
        f.tag = tag;
        f.value = on.contains(tag) ? 1 : 0;
        f.start = HB_FEATURE_GLOBAL_START;
        f.end = HB_FEATURE_GLOBAL_END;
        ret << f;
    };
    for (const hb_tag_t tag : fontFeatures)
        add(tag);
    for (const hb_tag_t tag : std::as_const(on))
        add(tag);
    return ret;
}

GlyphList FMOtf::shapeBuffer(hb_buffer_t *buffer, hb_font_t *font, const QString &script, const QString &lang, const QList<hb_feature_t> &features, bool ltr)
{
    // Glyphs stay in logical order, the layout engine takes care of the
    // progression. A script still gets its shaper: joining, reordering.
    // Without a script name the text tells, from its first character that has
    // one. The direction and the language this also guesses are set below.
    if (script.isEmpty())
        hb_buffer_guess_segment_properties(buffer);
    else {
        // 'DFLT' gives no script, and HarfBuzz then takes the DFLT table
        hb_buffer_set_script(buffer, hb_ot_tag_to_script(tagOf(script)));
    }
    // Right-to-left text is shaped right to left, so that its marks are positioned
    // for that direction; `ltr` asks for the direction of the script, not for LTR.
    hb_direction_t direction(ltr ? hb_script_get_horizontal_direction(hb_buffer_get_script(buffer)) : HB_DIRECTION_RTL);
    if (direction != HB_DIRECTION_RTL)
        direction = HB_DIRECTION_LTR;
    hb_buffer_set_direction(buffer, direction);
    // Not guessed from the locale: it would pick the forms of one language
    hb_buffer_set_language(buffer, isDefaultLanguage(lang) ? HB_LANGUAGE_INVALID : hb_ot_tag_to_language(tagOf(lang)));
    // A mark keeps the index of its own character
    hb_buffer_set_cluster_level(buffer, HB_BUFFER_CLUSTER_LEVEL_CHARACTERS);

    hb_shape(font, buffer, features.isEmpty() ? nullptr : features.constData(), features.count());

    unsigned int count(0);
    const hb_glyph_info_t *infos(hb_buffer_get_glyph_infos(buffer, &count));
    const hb_glyph_position_t *positions(hb_buffer_get_glyph_positions(buffer, &count));

    GlyphList renderedString;
    lastGlyphs.clear();
    for (unsigned int i(0); i < count; ++i) {
        RenderedGlyph gl;
        gl.glyph = infos[i].codepoint;
        gl.log = infos[i].cluster;
        gl.xadvance = positions[i].x_advance;
        gl.xoffset = positions[i].x_offset;
        // up is positive, as in the font: a scene, whose y goes down, subtracts it
        gl.yoffset = positions[i].y_offset;
        // The advance of vertical progressions. It is FreeType's, which makes
        // one up for a font without vertical metrics.
        if (!FT_Load_Glyph(_face, gl.glyph, FT_LOAD_NO_SCALE))
            gl.yadvance = _face->glyph->metrics.vertAdvance;
        renderedString << gl;
        lastGlyphs << infos[i].codepoint;
    }
    // HarfBuzz gives right-to-left text in the order it is drawn; back to the order it
    // is written in, which the layout turns around again line by line (FMBidi)
    if (direction == HB_DIRECTION_RTL) {
        std::reverse(renderedString.begin(), renderedString.end());
        std::reverse(lastGlyphs.begin(), lastGlyphs.end());
    }
    return renderedString;
}

void FMOtf::collectAlternates(const QString &s, const QString &script, const QString &lang)
{
    const hb_tag_t table(HB_OT_TAG_GSUB);
    unsigned int scriptIndex(0);
    if (!hb_ot_layout_table_find_script(hbFace, table, tagOf(script), &scriptIndex))
        return;
    unsigned int langIndex(HB_OT_LAYOUT_DEFAULT_LANGUAGE_INDEX);
    if (!isDefaultLanguage(lang)) {
        const hb_tag_t langTag(tagOf(lang));
        hb_ot_layout_script_select_language(hbFace, table, scriptIndex, 1, &langTag, &langIndex);
    }
    unsigned int featureIndex(0);
    if (!hb_ot_layout_language_find_feature(hbFace, table, scriptIndex, langIndex, HB_TAG('a', 'a', 'l', 't'), &featureIndex))
        return;

    const unsigned int pageSize(32);
    QList<unsigned int> lookups;
    {
        unsigned int page[pageSize];
        unsigned int offset(0);
        unsigned int count(pageSize);
        while (count == pageSize) {
            hb_ot_layout_feature_get_lookups(hbFace, table, featureIndex, offset, &count, page);
            for (unsigned int i(0); i < count; ++i)
                lookups << page[i];
            offset += count;
        }
    }

    for (const QChar c : s) {
        const hb_codepoint_t glyph(FT_Get_Char_Index(_face, c.unicode()));
        if (!glyph)
            continue;
        for (const unsigned int lookup : std::as_const(lookups)) {
            hb_codepoint_t page[pageSize];
            unsigned int offset(0);
            unsigned int count(pageSize);
            while (count == pageSize) {
                hb_ot_layout_lookup_get_glyph_alternates(hbFace, lookup, glyph, offset, &count, page);
                for (unsigned int i(0); i < count; ++i) {
                    if (!altGlyphs.contains(page[i]))
                        altGlyphs << page[i];
                }
                offset += count;
            }
        }
    }
}

QList<RenderedGlyph> FMOtf::procstring(QString s, OTFSet set)
{
    altGlyphs.clear();
    if (set.gsub_features.contains(QStringLiteral("aalt")))
        collectAlternates(s, set.script, set.lang);

    procstring(s, set.script, set.lang, set.gsub_features, set.gpos_features);
    return m_lastRun;
}

int FMOtf::procstring(QString s, QString script, QString lang, QStringList gsub, QStringList gpos)
{
    curString = s;
    hb_buffer_t *buffer(hb_buffer_create());
    hb_buffer_add_utf16(buffer, s.utf16(), s.length(), 0, s.length());
    m_lastRun = shapeBuffer(buffer, hbFont, script, lang, featureList(gsub + gpos));
    hb_buffer_destroy(buffer);

    for (RenderedGlyph &gl : m_lastRun)
        gl.lChar = (gl.log < s.length()) ? s.at(gl.log).unicode() : 0;
    return m_lastRun.count();
}

QList<RenderedGlyph> FMOtf::procstring(QList<Character> shaped, QString script)
{
    // The features of each character become features limited to its cluster.
    // Everything else of the font is off, as in the other procstring().
    curString.clear();
    QStringList none;
    QList<hb_feature_t> features(featureList(none));

    hb_buffer_t *buffer(hb_buffer_create());
    const int n(shaped.count());
    for (int i(0); i < n; ++i) {
        hb_buffer_add(buffer, shaped[i].unicode(), i);
        curString += QChar(shaped[i].unicode());
        for (const auto &cProp : std::as_const(shaped[i].CustomProperties)) {
            hb_feature_t f;
            f.tag = tagOf(cProp);
            f.value = 1;
            f.start = i;
            f.end = i + 1;
            features << f;
        }
    }
    hb_buffer_set_content_type(buffer, HB_BUFFER_CONTENT_TYPE_UNICODE);
    m_lastRun = shapeBuffer(buffer, hbFont, script, QStringLiteral("dflt"), features);
    hb_buffer_destroy(buffer);

    for (RenderedGlyph &gl : m_lastRun)
        gl.lChar = (gl.log < n) ? shaped.at(gl.log).unicode() : 0;
    return m_lastRun;
}

QList<RenderedGlyph> FMOtf::procstring(QList<unsigned int> glyList, QString script, QString lang, QStringList gsub, QStringList gpos)
{
    hb_buffer_t *buffer(hb_buffer_create());
    for (int i(0); i < glyList.count(); ++i)
        hb_buffer_add(buffer, GlyphIndexBase + glyList[i], i);
    hb_buffer_set_content_type(buffer, HB_BUFFER_CONTENT_TYPE_UNICODE);
    m_lastRun = shapeBuffer(buffer, hbGlyphFont, script, lang, featureList(gsub + gpos));
    hb_buffer_destroy(buffer);
    return m_lastRun;
}

QList<RenderedGlyph> FMOtf::shape(const QString &s, const QString &script, bool ltr)
{
    curString = s;
    hb_buffer_t *buffer(hb_buffer_create());
    hb_buffer_add_utf16(buffer, s.utf16(), s.length(), 0, s.length());
    // no feature list: HarfBuzz applies what the script asks for
    m_lastRun = shapeBuffer(buffer, hbFont, script, QStringLiteral("dflt"), QList<hb_feature_t>(), ltr);
    hb_buffer_destroy(buffer);

    for (RenderedGlyph &gl : m_lastRun)
        gl.lChar = (gl.log < s.length()) ? s.at(gl.log).unicode() : 0;
    return m_lastRun;
}

hb_tag_t FMOtf::tableTag() const
{
    return curTable == QLatin1String("GPOS") ? HB_OT_TAG_GPOS : HB_OT_TAG_GSUB;
}

bool FMOtf::currentScript(unsigned int *scriptIndex) const
{
    const bool present(tableTag() == HB_OT_TAG_GPOS ? GPOS : GSUB);
    return present && hb_ot_layout_table_find_script(hbFace, tableTag(), tagOf(curScriptName), scriptIndex);
}

unsigned int FMOtf::currentLanguage(unsigned int scriptIndex) const
{
    unsigned int langIndex(HB_OT_LAYOUT_DEFAULT_LANGUAGE_INDEX);
    if (!isDefaultLanguage(curLangName)) {
        const hb_tag_t langTag(tagOf(curLangName));
        hb_ot_layout_script_select_language(hbFace, tableTag(), scriptIndex, 1, &langTag, &langIndex);
    }
    return langIndex;
}

QStringList FMOtf::get_tables()
{
    QStringList ret;
    if (GPOS)
        ret << QStringLiteral("GPOS");
    if (GSUB)
        ret << QStringLiteral("GSUB");
    return ret;
}

void FMOtf::set_table(QString s)
{
    curTable = s;
}

QStringList FMOtf::get_scripts()
{
    QStringList ret;
    if ((tableTag() == HB_OT_TAG_GPOS) ? !GPOS : !GSUB)
        return ret;
    const QList<hb_tag_t> tags(allTags([&](unsigned int offset, unsigned int *count, hb_tag_t *page) {
        hb_ot_layout_table_get_script_tags(hbFace, tableTag(), offset, count, page);
    }));
    for (const hb_tag_t t : tags)
        ret << nameOf(t);
    return ret;
}

void FMOtf::set_script(QString s)
{
    curScriptName = s;
}

QStringList FMOtf::get_langs()
{
    QStringList ret;
    ret << QStringLiteral("dflt");
    unsigned int scriptIndex(0);
    if (!currentScript(&scriptIndex))
        return ret;
    const QList<hb_tag_t> tags(allTags([&](unsigned int offset, unsigned int *count, hb_tag_t *page) {
        hb_ot_layout_script_get_language_tags(hbFace, tableTag(), scriptIndex, offset, count, page);
    }));
    for (const hb_tag_t t : tags)
        ret << nameOf(t);
    return ret;
}

void FMOtf::set_lang(QString s)
{
    curLangName = isDefaultLanguage(s) ? QStringLiteral("dflt") : s;
}

QStringList FMOtf::get_features(bool required)
{
    QStringList ret;
    unsigned int scriptIndex(0);
    if (!currentScript(&scriptIndex))
        return ret;
    const unsigned int langIndex(currentLanguage(scriptIndex));

    if (required) {
        unsigned int featureIndex(0);
        hb_tag_t tag(HB_TAG_NONE);
        if (hb_ot_layout_language_get_required_feature(hbFace, tableTag(), scriptIndex, langIndex, &featureIndex, &tag))
            ret << nameOf(tag);
        return ret;
    }

    const QList<hb_tag_t> tags(allTags([&](unsigned int offset, unsigned int *count, hb_tag_t *page) {
        hb_ot_layout_language_get_feature_tags(hbFace, tableTag(), scriptIndex, langIndex, offset, count, page);
    }));
    for (const hb_tag_t t : tags)
        ret << nameOf(t);
    return ret;
}

void FMOtf::set_features(QStringList ls)
{
    curFeatures = ls;
}
