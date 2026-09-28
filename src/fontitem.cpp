/*
    SPDX-FileCopyrightText: 2007 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "fontitem.h"
#include "fmaltcontext.h"
#include "fmbaseshaper.h"
#include "fmbidi.h"
#include "fmcolorglyphitem.h"
#include "fmcolorpainter.h"
#include "fmconfig.h"
#include "fmencdata.h"
#include "fmfontdb.h"
#include "fmfontstrings.h"
#include "fmfreetypelib.h"
#include "fmglyphsview.h"
#include "fmhyphenator.h"
#include "fmkernfeat.h"
#include "fmotf.h"
#include "fmsvgglyphs.h"
#include "fmuniblocks.h"
#include "fontmatrix_debug.h"
#include "glyphtosvghelper.h"
#include "typotek.h"

#include <KLocalizedString>
#include <cmath>

#include <QApplication>
#include <QByteArrayView>
#include <QDebug>
#include <QFileInfo>
#include <QGraphicsObject>
#include <QGraphicsPathItem>
#include <QGraphicsPixmapItem>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QLocale>
#include <QPainter>
#include <QStringDecoder>

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProgressDialog>
#include <QUrl>
#include <QVarLengthArray>

#include <climits>

#include "QDebug"

#include FT_XFREE86_H
#include FT_COLOR_H
#include FT_GLYPH_H
#include FT_OUTLINE_H
#include FT_MULTIPLE_MASTERS_H
#include FT_SFNT_NAMES_H
#include FT_TYPE1_TABLES_H
#include FT_TRUETYPE_TABLES_H
#include FT_TRUETYPE_IDS_H

// #include <QWaitCondition>
// #include <QMutex>

int fm_num_face_opened = 0;

QGraphicsScene *FontItem::theOneLineScene = nullptr;

QList<int> FontItem::legitimateNonPathChars;

QVector<QRgb> gray256Palette;
QVector<QRgb> invertedGray256Palette;

// QWaitCondition theCondition;
// QMutex theMutex;

unsigned int OTF_name_tag(QString s);

/** functions set for decomposition
 */

struct SizedPath {
    QPainterPath *p;
    double s;
};

// an anticipation of further changes in Freetype
struct FM_Vector // :)
{
    double x;
    double y;

    explicit FM_Vector(const FT_Vector *vect)
    {
        x = double(vect->x);
        y = double(vect->y);

        // 		qDebug()<<"x26"<<vect26dot6->x<<"y26"<<vect26dot6->y <<"x"<<x<<"y"<<y;
    };
};

static int _moveTo(const FT_Vector *to26, void *user)
{
    FM_Vector to(to26);
    auto sp = reinterpret_cast<SizedPath *>(user);
    QPainterPath *p(sp->p);
    double sf(sp->s);
    p->moveTo(to.x * sf, to.y * sf * -1.0);
    return 0;
}
static int _lineTo(const FT_Vector *to26, void *user)
{
    FM_Vector to(to26);
    auto sp = reinterpret_cast<SizedPath *>(user);
    QPainterPath *p(sp->p);
    double sf(sp->s);
    p->lineTo(to.x * sf, to.y * sf * -1.0);
    return 0;
}
static int _conicTo(const FT_Vector *control26, const FT_Vector *to26, void *user)
{
    FM_Vector control(control26);
    FM_Vector to(to26);
    auto sp = reinterpret_cast<SizedPath *>(user);
    QPainterPath *p(sp->p);
    double sf(sp->s);
    p->quadTo(control.x * sf, control.y * sf * -1.0, to.x * sf, to.y * sf * -1.0);
    return 0;
}
static int _cubicTo(const FT_Vector *control126, const FT_Vector *control226, const FT_Vector *to26, void *user)
{
    FM_Vector control1(control126);
    FM_Vector control2(control226);
    FM_Vector to(to26);
    auto sp = reinterpret_cast<SizedPath *>(user);
    QPainterPath *p(sp->p);
    double sf(sp->s);
    p->cubicTo(control1.x * sf, control1.y * sf * -1.0, control2.x * sf, control2.y * sf * -1.0, to.x * sf, to.y * sf * -1.0);
    return 0;
}

FT_Outline_Funcs outline_funcs = {_moveTo, _lineTo, _conicTo, _cubicTo, 0, 0};
/** **************************************************/

void FontItem::fillLegitimateSpaces()
{
    legitimateNonPathChars << 0x0020;
    legitimateNonPathChars << 0x00A0;
    legitimateNonPathChars << 0x1680;
    legitimateNonPathChars << 0x180E;
    legitimateNonPathChars << 0x2002;
    legitimateNonPathChars << 0x2003;
    legitimateNonPathChars << 0x2004;
    legitimateNonPathChars << 0x2005;
    legitimateNonPathChars << 0x2006;
    legitimateNonPathChars << 0x2007;
    legitimateNonPathChars << 0x2008;
    legitimateNonPathChars << 0x2009;
    legitimateNonPathChars << 0x200A;
    legitimateNonPathChars << 0x200B;
    legitimateNonPathChars << 0x200C;
    legitimateNonPathChars << 0x200D;
    legitimateNonPathChars << 0x202F;
    legitimateNonPathChars << 0x205F;
    legitimateNonPathChars << 0x2060;
    legitimateNonPathChars << 0x3000;
    legitimateNonPathChars << 0xFEFF;
}

void FontItem::fill256Palette()
{
    // #ifdef   PLATFORM_APPLE
    // 	for ( int i = 0; i < 256 ; ++i )
    // 	{
    // 		gray256Palette << qRgb (255-i, 255-i,255- i );
    // 	}
    // #else
    for (int i = 0; i < 256; ++i) {
        gray256Palette << qRgba(0, 0, 0, i);
    }
    // #endif
}

void FontItem::fillInvertedPalette()
{
    for (int i = 0; i < 256; ++i) {
        invertedGray256Palette << qRgb(i, i, i);
    }
}

FontItem::FontItem(QString path, bool remote, bool faststart)
{
    // 	qDebug()<<"FONT ITEM"<<path;
    m_valid = false;
    m_active = false;
    m_remote = remote;
    remoteCached = false;
    stopperDownload = false;
    m_face = nullptr;
    lastFace = nullptr;
    m_glyphsPerRow = 5;
    m_isEncoded = false;
    currentChar = -1;
    m_isOpenType = false;
    otf = nullptr;
    m_rasterFreetype = false;
    m_progression = PROGRESSION_LTR;
    m_shaperType = 1;
    renderReturnWidth = false;
    unitPerEm = 0;
    m_FTHintMode = 0;
    allIsRendered = false;
    isUpToDate = false;
    m_path = path;

    /// STATIC INITIALISATIONS

    if (legitimateNonPathChars.isEmpty())
        fillLegitimateSpaces();
    if (gray256Palette.isEmpty())
        fill256Palette();
    if (invertedGray256Palette.isEmpty())
        fillInvertedPalette();
    if (!theOneLineScene) {
        theOneLineScene = new QGraphicsScene;
    }
    /// EndOF S I

    if (m_remote || faststart) {
        m_valid = true;
        return;
    }

    QFileInfo infopath(m_path);
    m_name = infopath.fileName();
    m_fileSize = QString::number(infopath.size(), 10);

    if (!ensureFace()) {
        return;
    }

    if (infopath.suffix() == "pfb" || infopath.suffix() == "PFB") {
        m_afm = m_path;
        if (infopath.suffix() == "pfb") {
            m_afm.replace(".pfb", ".afm");
            if (!QFile::exists(m_afm)) {
                m_afm.replace(".afm", ".AFM");
                if (!QFile::exists(m_afm)) {
                    m_afm = "";
                }
            }
        } else if (infopath.suffix() == "PFB") {
            m_afm.replace(".PFB", ".AFM");
            if (!QFile::exists(m_afm)) {
                m_afm.replace(".AFM", ".afm");
                if (!QFile::exists(m_afm)) {
                    m_afm = "";
                }
            }
        }
    }

    if (testFlag(m_face->face_flags, FT_FACE_FLAG_SFNT, "1", "0") == "1") {
        m_isOpenType = true;
    }

    if (m_isOpenType)
        moreInfo_sfnt();
    else
        moreInfo_type1();

    m_type = FT_Get_X11_Font_Format(m_face);
    // 	if ( typotek::getInstance()->familySchemeFreetype() || !m_isOpenType )
    {
        m_family = m_face->family_name;
        m_variant = m_face->style_name;
    }
    // 	else
    // 	{
    // 		m_family = getAlternateFamilyName();
    // 		m_variant = getAlternateVariantName();
    // 	}
    m_numGlyphs = m_face->num_glyphs;
    m_numFaces = m_face->num_faces;

    // 	for ( int i = 0 ;i < m_face->num_charmaps; ++i )
    // 	{
    // 		m_charsets << charsetMap[m_face->charmaps[i]->encoding];
    // 	}

    // 	m_lock = false;
    pixList.clear();

    if (m_family.isEmpty())
        return;
    if (m_variant.isEmpty())
        return;

    m_valid = true;
    releaseFace();
}

FontItem::FontItem(QString path, QString family, QString variant, QString type, bool active)
{
    m_valid = true;
    m_remote = isRemotePath(path);
    remoteCached = false;
    if (m_remote) {
        // downloaded in an earlier session?
        remoteHerePath = typotek::getInstance()->remoteTmpDir() + QDir::separator() + QFileInfo(QUrl(path).path()).fileName();
        remoteCached = QFileInfo::exists(remoteHerePath);
    }
    stopperDownload = false;
    m_face = nullptr;
    lastFace = nullptr;
    m_glyphsPerRow = 5;
    m_isEncoded = false;
    currentChar = -1;
    m_isOpenType = false;
    otf = nullptr;
    m_rasterFreetype = false;
    m_progression = PROGRESSION_LTR;
    m_shaperType = 1;
    renderReturnWidth = false;
    unitPerEm = 0;
    m_FTHintMode = 0;
    // 	m_lock = false;
    allIsRendered = false;
    isUpToDate = false;

    if (legitimateNonPathChars.isEmpty())
        fillLegitimateSpaces();
    if (gray256Palette.isEmpty())
        fill256Palette();
    if (invertedGray256Palette.isEmpty())
        fillInvertedPalette();
    if (!theOneLineScene) {
        theOneLineScene = new QGraphicsScene;
    }

    m_path = path;
    m_family = family;
    m_variant = variant;
    m_active = active;
    m_type = type;
}

FontItem *FontItem::Clone()
{
    auto fitem = new FontItem(m_path, m_family, m_variant, m_type, m_active);
    return fitem;
}

void FontItem::updateItem()
{
    if (isUpToDate)
        return;
    QFileInfo infopath(m_path);
    m_name = infopath.fileName();
    if (!ensureFace()) {
        return;
    }
    if (infopath.suffix() == "pfb" || infopath.suffix() == "PFB") {
        m_afm = m_path;
        if (infopath.suffix() == "pfb") {
            m_afm.replace(".pfb", ".afm");
            if (!QFile::exists(m_afm)) {
                m_afm.replace(".afm", ".AFM");
                if (!QFile::exists(m_afm)) {
                    m_afm = "";
                }
            }
        } else if (infopath.suffix() == "PFB") {
            m_afm.replace(".PFB", ".AFM");
            if (!QFile::exists(m_afm)) {
                m_afm.replace(".AFM", ".afm");
                if (!QFile::exists(m_afm)) {
                    m_afm = "";
                }
            }
        }
    }

    if (testFlag(m_face->face_flags, FT_FACE_FLAG_SFNT, "1", "0") == "1") {
        m_isOpenType = true;
    }

    m_type = FT_Get_X11_Font_Format(m_face);
    m_family = m_face->family_name;
    m_variant = m_face->style_name;
    m_numGlyphs = m_face->num_glyphs;
    m_numFaces = m_face->num_faces;

    releaseFace();
    isUpToDate = true;
}

FontItem::~FontItem()
{
    if (m_isOpenType && otf) {
        // 		delete otf;
    }
}

void FontItem::encodeFace()
{
    if (!m_face)
        return;

    m_charsets.clear();
    m_unicodeBuiltIn = (m_face->charmap == nullptr) ? false : true;
    if (QString(FT_Get_X11_Font_Format(m_face)) == QString("Type 1"))
        m_unicodeBuiltIn = false;

    QMap<FT_Encoding, FT_CharMap> cmaps;
    for (int u = 0; u < m_face->num_charmaps; u++) {
        cmaps[m_face->charmaps[u]->encoding] = m_face->charmaps[u];
    }

    bool mapped(false);
    // Stop kidding
    if (/* (!isType1) && UnicodeBuiltIn && */ cmaps.contains(FT_ENCODING_UNICODE)) {
        FT_Set_Charmap(m_face, cmaps[FT_ENCODING_UNICODE]);
        m_charsets << FT_ENCODING_UNICODE;
        mapped = true;
        m_isEncoded = true;
        m_currentEncoding = FT_ENCODING_UNICODE;
    }
    // uncomment below to get Unicode cmap synthetized by FT
    // 	else if(cmaps.contains( FT_ENCODING_UNICODE ))
    // 	{
    // 		FT_Set_Charmap(m_face, cmaps[FT_ENCODING_UNICODE]);
    // 		m_charsets << FontStrings::Encoding(FT_ENCODING_UNICODE) +"*";
    // 		mapped = true;
    // 		cmaps.remove(FT_ENCODING_UNICODE);
    // 		m_isEncoded = true;
    // 	}
    for (const auto cmapsKeys = cmaps.keys(); const auto &e : cmapsKeys) {
        // 		QString cs(FontStrings::Encoding(e));
        // 		if(isType1 && (e == FT_ENCODING_UNICODE))
        // 			continue;
        if (!m_charsets.contains(e))
            m_charsets << e;
        if (!mapped) {
            FT_Set_Charmap(m_face, cmaps[e]);
            mapped = true;
            m_isEncoded = true;
            m_currentEncoding = e;
        }
    }
}

bool FontItem::ensureFace()
{
    FT_Library ftlib = FMFreetypeLib::lib(thread());
    //	qDebug()<<"FontItem::ensureFace"<<thread();

    if (m_face) {
        ++facesRef;
        return true;
    }
    if (m_remote && !remoteCached)
        return false; // the file is not here; getFromNetwork() brings it
    const QString trueFile(localPath());
    ft_error = FT_New_Face(ftlib, trueFile.toUtf8().constData(), 0, &m_face);
    if (ft_error) {
        qCWarning(FONTMATRIX_LOG) << "Error loading face [" << trueFile << "]";
        return false;
    }
    m_faceSeen = true;
    m_hasColor = FT_HAS_COLOR(m_face);
    if (m_hasColor) {
        m_paintFont = FMColorPainter::paintFont(m_face);
        m_svgGlyphs = FMSvgGlyphs::create(m_face);
    }
    if (m_face->units_per_EM == 0 && m_headUnitsPerEm <= 0.0) {
        // a bitmap-only font: FreeType has no scale for it, its head table has the units
        if (auto *head = static_cast<TT_Header *>(FT_Get_Sfnt_Table(m_face, FT_SFNT_HEAD)))
            m_headUnitsPerEm = head->Units_Per_EM;
    }
    readVariation();
    applyVariation();
    encodeFace();
    if (spaceIndex.isEmpty()) {
        int gIndex(0);
        for (int i(0); i < legitimateNonPathChars.size(); ++i) {
            gIndex = FT_Get_Char_Index(m_face, legitimateNonPathChars.at(i));
            if (gIndex) {
                spaceIndex << gIndex;
            }
        }
    }
    unitPerEm = unitsPerEm();
    m_glyph = m_face->glyph;
    facesRef = 1;
    ++fm_num_face_opened;
    return true;
}

void FontItem::releaseFace()
{
    if (m_face) {
        --facesRef;
        if (facesRef == 0) {
            if (m_paintFont) {
                hb_font_destroy(m_paintFont);
                m_paintFont = nullptr;
            }
            delete m_svgGlyphs;
            m_svgGlyphs = nullptr;
            FT_Done_Face(m_face);
            m_face = nullptr;
            --fm_num_face_opened;
        }
    }
}

int FontItem::glyphsCount() const
{
    if (m_numGlyphs > 0) // this is normal case
    {
        return m_numGlyphs;
    }
    auto that(const_cast<FontItem *>(this));
    that->ensureFace();
    that->m_numGlyphs = m_face->num_glyphs;
    that->releaseFace();
    return m_numGlyphs;
}

QString FontItem::testFlag(long flag, long against, QString yes, QString no)
{
    if ((flag & against) == against)
        return yes;
    else
        return no;
}

// QString FontItem::value ( QString k )
// {
// 	// I don’t know if something relies o it so I keep it, for the moment.
// 	if ( k == "family" )
// 		return m_family;
// 	else if ( k == "variant" )
// 		return m_variant;
//
// 	// 0 is default language
// 	// TODO inspect all available languages
// // 	if(moreInfo.isEmpty())
// // 	{
// // 		if(isOpenType())
// // 			moreInfo_sfnt();
// // 		else
// // 			moreInfo_type1();
// // 	}
// 	FontInfoMap moreInfo( FMFontDb::DB()->getInfoMap(m_path) );
// 	QMap<int, QString> namap ( moreInfo.value ( 0 ) );
// 	return namap.value ( name_meaning.indexOf( k ) );
// }

// QString FontItem::panose ( QString k )
// {
// 	return panoseInfo.value ( k );
// }

QString FontItem::name()
{
    return m_name;
}

QGraphicsPathItem *FontItem::itemFromChar(int charcode, double size)
{
    if (!ensureFace())
        return nullptr;
    uint glyphIndex = 0;
    currentChar = charcode;
    glyphIndex = FT_Get_Char_Index(m_face, charcode);

    QGraphicsPathItem *ret(itemFromGindex(glyphIndex, size));
    releaseFace();
    return ret;
}

QGraphicsPathItem *FontItem::itemFromGindex(int index, double size)
{
    if (!ensureFace())
        return nullptr;
    int charcode = index;
    double scalefactor = size / unitsPerEm();
    ft_error = loadUnscaled(charcode);
    if (ft_error) {
        QPainterPath glyphPath;
        glyphPath.addRect(0.0, 0.0, size, size);
        auto glyph = new QGraphicsPathItem;
        glyph->setBrush(QBrush(Qt::red));
        glyph->setPath(glyphPath);
        glyph->setData(GLYPH_DATA_GLYPH, index);
        glyph->setData(GLYPH_DATA_HADVANCE, (double)size * scalefactor);
        glyph->setData(GLYPH_DATA_HADVANCE_SCALED, (double)size);
        glyph->setData(GLYPH_DATA_ERROR, true);
        releaseFace();
        return glyph;
    }

    FT_Outline *outline = &m_glyph->outline;
    QPainterPath glyphPath(QPointF(0.0, 0.0));
    SizedPath sp{};
    sp.p = &glyphPath;
    sp.s = scalefactor;
    FT_Outline_Decompose(outline, &outline_funcs, &sp);
    glyphPath.closeSubpath();

    if (m_hasColor) {
        // the base glyph of a colour glyph has no outline of its own, or a fallback one
        const FT_Glyph_Metrics metrics = m_glyph->metrics;
        if (QGraphicsPathItem *colored = colorItem(index, scalefactor)) {
            colored->setData(GLYPH_DATA_GLYPH, index);
            colored->setData(GLYPH_DATA_HADVANCE, (double)metrics.horiAdvance);
            colored->setData(GLYPH_DATA_HADVANCE_SCALED, (double)metrics.horiAdvance * scalefactor);
            colored->setData(5, (double)metrics.vertAdvance);
            colored->setData(GLYPH_DATA_ERROR, false);
            releaseFace();
            return colored;
        }
    }
    auto glyph = new QGraphicsPathItem;

    if (glyphPath.elementCount() < 3 && !spaceIndex.contains(index)) {
        QBrush brush(Qt::SolidPattern);
        brush.setColor(Qt::red);
        QPen pen(brush, 0);
        QPainterPath errPath;
        errPath.addRect(0.0, -size, size, size);
        glyph->setBrush(brush);
        glyph->setPen(pen);
        glyph->setPath(errPath);
        glyph->setData(GLYPH_DATA_HADVANCE, (double)m_glyph->metrics.horiAdvance);
        glyph->setData(GLYPH_DATA_HADVANCE_SCALED, (double)m_glyph->metrics.horiAdvance * scalefactor);
        glyph->setData(GLYPH_DATA_GLYPH, index);
        glyph->setData(GLYPH_DATA_ERROR, true);
    } else {
        glyph->setBrush(QBrush(Qt::SolidPattern));
        glyph->setPath(glyphPath);
        glyph->setData(GLYPH_DATA_GLYPH, index);
        glyph->setData(GLYPH_DATA_HADVANCE, (double)m_glyph->metrics.horiAdvance);
        glyph->setData(GLYPH_DATA_HADVANCE_SCALED, (double)m_glyph->metrics.horiAdvance * scalefactor);
        glyph->setData(5, (double)m_glyph->metrics.vertAdvance);
        glyph->setData(GLYPH_DATA_ERROR, false);
        // 		glyph->scale ( scalefactor,-scalefactor );
    }
    releaseFace();
    return glyph;
}

QGraphicsPixmapItem *FontItem::itemFromCharPix(int charcode, double size)
{
    if (!ensureFace())
        return nullptr;
    uint glyphIndex = 0;
    currentChar = charcode;
    glyphIndex = FT_Get_Char_Index(m_face, charcode);

    releaseFace();
    return itemFromGindexPix(glyphIndex, size);
}

QGraphicsPixmapItem *FontItem::itemFromGindexPix(int index, double size)
{
    if (!ensureFace())
        return nullptr;
    int charcode = index;

    // Set size
    setSize(size);

    // Grab metrics in FONT UNIT
    ft_error = loadUnscaled(charcode);
    if (ft_error) {
        QPixmap square(qRound(size), qRound(size));
        square.fill(Qt::red);
        auto glyph = new QGraphicsPixmapItem(square);
        glyph->setData(GLYPH_DATA_GLYPH, index);
        glyph->setData(GLYPH_DATA_BITMAPLEFT, 0);
        glyph->setData(GLYPH_DATA_BITMAPTOP, size);
        glyph->setData(GLYPH_DATA_HADVANCE, size / (size / unitsPerEm()));
        releaseFace();
        return glyph;
    }

    double takeAdvanceBeforeRender = m_glyph->metrics.horiAdvance * (typotek::getInstance()->getDpiX() / 72.0);
    double takeVertAdvanceBeforeRender = m_glyph->metrics.vertAdvance * (typotek::getInstance()->getDpiY() / 72.0);
    double takeLeftBeforeRender = double(m_glyph->metrics.horiBearingX) * (typotek::getInstance()->getDpiX() / 72.0);

    // 	if(m_FTHintMode != FT_LOAD_NO_HINTING)
    {
        ft_error = FT_Load_Glyph(m_face, charcode, loadFlags(FT_LOAD_DEFAULT | m_FTHintMode));
    }
    // Render the glyph into a grayscale bitmap
    ft_error = FT_Render_Glyph(m_face->glyph, FT_RENDER_MODE_NORMAL);
    if (ft_error) {
        QPixmap square(qRound(size), qRound(size));
        square.fill(Qt::red);
        auto glyph = new QGraphicsPixmapItem(square);
        glyph->setData(GLYPH_DATA_GLYPH, index);
        glyph->setData(GLYPH_DATA_BITMAPLEFT, 0);
        glyph->setData(GLYPH_DATA_BITMAPTOP, size);
        glyph->setData(GLYPH_DATA_HADVANCE, size / (size / unitsPerEm()));
        releaseFace();
        return glyph;
    }

    QImage img(glyphImage());
    double bitmapTopPx = bitmapTop();
    double paintedLeft = takeLeftBeforeRender;
    if (paintedGlyph(index, img, paintedLeft, bitmapTopPx)) // the pixels into the units the layout scales
        takeLeftBeforeRender = paintedLeft * unitsPerEm() / size;
    auto glyph = new QGraphicsPixmapItem;

    if (img.isNull() && !spaceIndex.contains(index)) {
        QPixmap square(qRound(size), qRound(size));
        square.fill(Qt::red);
        glyph->setPixmap(square);
        glyph->setData(GLYPH_DATA_GLYPH, index);
        glyph->setData(GLYPH_DATA_BITMAPLEFT, 0);
        glyph->setData(GLYPH_DATA_BITMAPTOP, size);
        glyph->setData(GLYPH_DATA_HADVANCE, size / (size / unitsPerEm()));
    } else {
#ifndef PLATFORM_APPLE
        // Convert from Format_Indexed8 (with ARGB color table) to ARGB32 so
        // that the alpha channel is preserved correctly on all Qt6 backends,
        // including the Windows Direct3D / RHI backend.
        glyph->setPixmap(QPixmap::fromImage(img.convertToFormat(QImage::Format_ARGB32)));
#else
        QPixmap aPix(img.width(), img.height());
        aPix.fill(QColor(0, 0, 0, 0));
        QPainter aPainter(&aPix);
        aPainter.drawImage(0, 0, img);
        glyph->setPixmap(aPix);
#endif
        // we need to transport more data
        glyph->setData(GLYPH_DATA_GLYPH, index);
        glyph->setData(GLYPH_DATA_BITMAPLEFT, takeLeftBeforeRender);
        glyph->setData(GLYPH_DATA_BITMAPTOP, bitmapTopPx);
        glyph->setData(GLYPH_DATA_HADVANCE, takeAdvanceBeforeRender);
        glyph->setData(GLYPH_DATA_VADVANCE, takeVertAdvanceBeforeRender);
    }

    releaseFace();
    return glyph;
}

MetaGlyphItem *FontItem::itemFromGindexPix_mt(int index, double size)
{
    if (!ensureFace())
        return nullptr;
    int charcode = index;
    //	qDebug()<<"FontItem::itemFromGindexPix_mt"<< thread();
    auto glyph = new MetaGlyphItem;
    double scaleFactor = size / unitsPerEm();

    // Set size
    setSize(size);

    // Grab metrics in FONT UNIT
    ft_error = loadUnscaled(charcode);
    if (ft_error) {
        glyph->setMetaData(GLYPH_DATA_GLYPH, index);
        glyph->setMetaData(GLYPH_DATA_BITMAPLEFT, 0);
        glyph->setMetaData(GLYPH_DATA_BITMAPTOP, size);
        glyph->setMetaData(GLYPH_DATA_HADVANCE, size / scaleFactor);
        releaseFace();
        return glyph;
    }

    double takeAdvanceBeforeRender = m_glyph->metrics.horiAdvance * (typotek::getInstance()->getDpiX() / 72.0);
    double takeVertAdvanceBeforeRender = m_glyph->metrics.vertAdvance * (typotek::getInstance()->getDpiX() / 72.0);
    double takeLeftBeforeRender = (double)m_glyph->metrics.horiBearingX * (typotek::getInstance()->getDpiX() / 72.0);

    // 	if(m_FTHintMode != FT_LOAD_NO_HINTING)
    {
        ft_error = FT_Load_Glyph(m_face, charcode, loadFlags(FT_LOAD_DEFAULT | m_FTHintMode));
    }
    // Render the glyph into a grayscale bitmap
    ft_error = FT_Render_Glyph(m_face->glyph, FT_RENDER_MODE_NORMAL);
    if (ft_error) {
        glyph->setMetaData(GLYPH_DATA_GLYPH, index);
        glyph->setMetaData(GLYPH_DATA_BITMAPLEFT, 0);
        glyph->setMetaData(GLYPH_DATA_BITMAPTOP, size);
        glyph->setMetaData(GLYPH_DATA_HADVANCE, size / scaleFactor);
        releaseFace();
        return glyph;
    }

    QImage img(glyphImage());
    double bitmapTopPx = bitmapTop();
    double paintedLeft = takeLeftBeforeRender;
    if (paintedGlyph(index, img, paintedLeft, bitmapTopPx)) // the pixels into the units the layout scales
        takeLeftBeforeRender = paintedLeft * unitsPerEm() / size;

    if (img.isNull() && !spaceIndex.contains(index)) {
        glyph->setMetaData(GLYPH_DATA_GLYPH, index);
        glyph->setMetaData(GLYPH_DATA_BITMAPLEFT, 0);
        glyph->setMetaData(GLYPH_DATA_BITMAPTOP, size);
        glyph->setMetaData(GLYPH_DATA_HADVANCE, size / scaleFactor);
    } else {
        glyph->setMetaData(GLYPH_DATA_GLYPH, index);
        glyph->setMetaData(GLYPH_DATA_BITMAPLEFT, takeLeftBeforeRender);
        glyph->setMetaData(GLYPH_DATA_BITMAPTOP, bitmapTopPx);
        glyph->setMetaData(GLYPH_DATA_HADVANCE, takeAdvanceBeforeRender);
        glyph->setMetaData(GLYPH_DATA_VADVANCE, takeVertAdvanceBeforeRender);
    }

    releaseFace();
    return glyph;
}

QImage FontItem::charImage(int charcode, double size)
{
    if (!ensureFace())
        return QImage();

    // Set size
    setSize(size);
    if (FT_Load_Char(m_face, charcode, loadFlags(FT_LOAD_DEFAULT))) {
        releaseFace();
        return QImage();
    }
    if (FT_Render_Glyph(m_face->glyph, FT_RENDER_MODE_NORMAL)) {
        releaseFace();
        return QImage();
    }

    QImage cImg(glyphImage());
    double left = 0.0;
    double top = 0.0;
    paintedGlyph(int(FT_Get_Char_Index(m_face, FT_ULong(charcode))), cImg, left, top);
    releaseFace();
    return cImg;
}

QImage FontItem::glyphImage(int index, double size)
{
    if (!ensureFace())
        return QImage();

    // Set size
    setSize(size);
    if (FT_Load_Glyph(m_face, index, loadFlags(FT_LOAD_DEFAULT))) {
        releaseFace();
        return QImage();
    }
    if (FT_Render_Glyph(m_face->glyph, FT_RENDER_MODE_NORMAL)) {
        releaseFace();
        return QImage();
    }

    QImage cImg(glyphImage());
    double left = 0.0;
    double top = 0.0;
    paintedGlyph(index, cImg, left, top);
    releaseFace();
    return cImg;
}
/// Nature line
double FontItem::renderLine(QGraphicsScene *scene, QString spec, QPointF origine, double lineWidth, double fsize, double zindex)
{
    // 	qDebug() <<fancyName() <<"::"<<"renderLine("<<scene<<spec<<lineWidth<<fsize<<zindex<<record<<")";
    double retValue(0.0);
    if (spec.isEmpty())
        return retValue;

    ensureFace();

    double sizz = fsize;
    double scalefactor = sizz / unitsPerEm();
    double pWidth = lineWidth;
    const double distance = 20;
    QPointF pen(origine);
    if (m_rasterFreetype) {
        QList<QGraphicsPixmapItem *> mayBeRemoved;
        for (int i = 0; i < spec.length(); ++i) {
            QGraphicsPixmapItem *glyph = itemFromCharPix(spec.at(i).unicode(), sizz);
            if (spec.at(i).category() == QChar::Separator_Space) {
                mayBeRemoved.clear();
            }
            if (!glyph) {
                continue;
            }
            if (m_progression == PROGRESSION_RTL) {
                pen.rx() -= (glyph->data(GLYPH_DATA_HADVANCE).toDouble() + glyph->data(GLYPH_DATA_BITMAPLEFT).toDouble()) * scalefactor;
                pWidth -= glyph->data(GLYPH_DATA_HADVANCE).toDouble() * scalefactor;
                if (pWidth < distance) {
                    delete glyph;

                    retValue -= mayBeRemoved.size() - 1;
                    for (auto *rm : mayBeRemoved) {
                        scene->removeItem(rm);
                        delete rm;
                    }

                    break;
                }
            } else if (m_progression == PROGRESSION_BTT) {
                pen.ry() -= glyph->data(GLYPH_DATA_VADVANCE).toDouble() * scalefactor;
                pWidth -= glyph->data(GLYPH_DATA_VADVANCE).toDouble() * scalefactor;
                if (pWidth < distance) {
                    delete glyph;

                    retValue -= mayBeRemoved.size() - 1;
                    for (auto *rm : mayBeRemoved) {
                        scene->removeItem(rm);
                        delete rm;
                    }

                    break;
                }
            } else if (m_progression == PROGRESSION_LTR) {
                pWidth -= glyph->data(GLYPH_DATA_HADVANCE).toDouble() * scalefactor;
                if (pWidth < distance) {
                    delete glyph;

                    retValue -= mayBeRemoved.size() - 1;
                    for (auto *rm : mayBeRemoved) {
                        scene->removeItem(rm);
                        delete rm;
                    }

                    break;
                }
            } else if (m_progression == PROGRESSION_TTB) {
                pWidth -= glyph->data(GLYPH_DATA_VADVANCE).toDouble() * scalefactor;
                if (pWidth < distance) {
                    delete glyph;

                    retValue -= mayBeRemoved.size() - 1;
                    for (auto *rm : mayBeRemoved) {
                        scene->removeItem(rm);
                        delete rm;
                    }

                    break;
                }
            }
            if (renderReturnWidth)
                retValue += glyph->data(GLYPH_DATA_HADVANCE).toDouble() * scalefactor;

            /************************************/

            mayBeRemoved.append(glyph);

            scene->addItem(glyph);

            if (renderReturnWidth)
                retValue += glyph->data(GLYPH_DATA_HADVANCE).toDouble() * scalefactor;
            else
                retValue += 1;

            glyph->setPos(pen.x() + glyph->data(GLYPH_DATA_BITMAPLEFT).toDouble() * scalefactor, pen.y() - glyph->data(GLYPH_DATA_BITMAPTOP).toInt());
            glyph->setZValue(zindex);
            glyph->setData(GLYPH_DATA_GLYPH, "glyph");
            glyph->setData(GLYPH_DATA_FONTNAME, fancyName());
            /************************************/

            if (m_progression == PROGRESSION_LTR)
                pen.rx() += glyph->data(GLYPH_DATA_HADVANCE).toDouble() * scalefactor;
            else if (m_progression == PROGRESSION_TTB) {
                pen.ry() += glyph->data(GLYPH_DATA_VADVANCE).toDouble() * scalefactor;
            }
        }
    } else {
        QList<QGraphicsPathItem *> mayBeRemoved;
        for (int i = 0; i < spec.length(); ++i) {
            //			if ( !scene->sceneRect().contains ( pen ) && record )
            //				break;
            QGraphicsPathItem *glyph = itemFromChar(spec.at(i).unicode(), sizz);
            if (!glyph)
                continue;
            if (spec.at(i).category() == QChar::Separator_Space) {
                mayBeRemoved.clear();
            }
            if (m_progression == PROGRESSION_RTL) {
                pen.rx() -= glyph->data(GLYPH_DATA_HADVANCE).toDouble() * scalefactor;

                pWidth -= glyph->data(GLYPH_DATA_HADVANCE).toDouble() * scalefactor;
                if (pWidth < distance) {
                    delete glyph;

                    retValue -= mayBeRemoved.size() - 1;
                    for (auto *rm : mayBeRemoved) {
                        scene->removeItem(rm);
                        delete rm;
                    }

                    break;
                }
            } else if (m_progression == PROGRESSION_BTT) {
                pen.ry() -= glyph->data(GLYPH_DATA_VADVANCE).toDouble() * scalefactor;
                pWidth -= glyph->data(GLYPH_DATA_VADVANCE).toDouble() * scalefactor;
                if (pWidth < distance) {
                    delete glyph;

                    retValue -= mayBeRemoved.size() - 1;
                    for (auto *rm : mayBeRemoved) {
                        scene->removeItem(rm);
                        delete rm;
                    }

                    break;
                }
            } else if (m_progression == PROGRESSION_LTR) {
                pWidth -= glyph->data(GLYPH_DATA_HADVANCE).toDouble() * scalefactor;
                if (pWidth < distance) {
                    delete glyph;

                    retValue -= mayBeRemoved.size() - 1;
                    for (auto *rm : mayBeRemoved) {
                        scene->removeItem(rm);
                        delete rm;
                    }

                    break;
                }
            } else if (m_progression == PROGRESSION_TTB) {
                pWidth -= glyph->data(GLYPH_DATA_VADVANCE).toDouble() * scalefactor;
                if (pWidth < distance) {
                    delete glyph;

                    retValue -= mayBeRemoved.size() - 1;
                    for (auto *rm : mayBeRemoved) {
                        scene->removeItem(rm);
                        delete rm;
                    }

                    break;
                }
            }

            /*********************************/
            scene->addItem(glyph);
            glyph->setPen(Qt::NoPen);

            mayBeRemoved.append(glyph);

            if (renderReturnWidth)
                retValue += glyph->data(GLYPH_DATA_HADVANCE).toDouble() * scalefactor;
            else
                retValue += 1;

            glyph->setPos(pen);
            glyph->setZValue(zindex);
            glyph->setData(GLYPH_DATA_GLYPH, "glyph");
            glyph->setData(GLYPH_DATA_FONTNAME, fancyName());
            /*********************************/

            if (m_progression == PROGRESSION_LTR) {
                pen.rx() += glyph->data(GLYPH_DATA_HADVANCE).toDouble() * scalefactor;
            } else if (m_progression == PROGRESSION_TTB) {
                pen.ry() += glyph->data(GLYPH_DATA_VADVANCE).toDouble() * scalefactor;
            }
        }
    }

    releaseFace();
    return retValue;
}

/// Featured line
double FontItem::renderLine(OTFSet set, QGraphicsScene *scene, QString spec, QPointF origine, double lineWidth, double fsize)
{
    // 	qDebug()<<"Featured("<< spec <<")";
    double retValue(0.0);
    if (spec.isEmpty())
        return retValue;
    if (!m_isOpenType)
        return retValue;
    ensureFace();

    otf = new FMOtf(m_face, 0x10000); // You think "What’s this 0x10000?", so am I! Just accept Harfbuzz black magic :)
    if (!otf)
        return retValue;
    double sizz = fsize;
    double scalefactor = sizz / unitsPerEm();
    double pixelAdjustX = scalefactor * (typotek::getInstance()->getDpiX() / 72.0);
    double pixelAdjustY = scalefactor * (typotek::getInstance()->getDpiX() / 72.0);
    double pWidth = lineWidth;
    const double distance = 20;
    QList<RenderedGlyph> refGlyph = otf->procstring(spec, set);
    // drawn left to right: a right-to-left line is turned into that order first
    markCharacters(refGlyph, spec);
    FMBidi::toVisualOrder(refGlyph, FMBidi::isRightToLeft(refGlyph));
    // 	qDebug() << "Get line "<<spec;
    delete otf;
    otf = nullptr;
    // 	qDebug() << "Deleted OTF";
    if (refGlyph.isEmpty()) {
        return 0;
    }
    QPointF pen(origine);

    if (m_rasterFreetype) {
        QList<QGraphicsPixmapItem *> mayBeRemoved;
        for (int i = 0; i < refGlyph.size(); ++i) {
            QGraphicsPixmapItem *glyph = itemFromGindexPix(refGlyph[i].glyph, sizz);
            if (!glyph)
                continue;
            // Now, all is in the log!
            if (spec.at(refGlyph[i].log).category() == QChar::Separator_Space) {
                mayBeRemoved.clear();
            }

            if (m_progression == PROGRESSION_RTL) {
                pen.rx() -= refGlyph[i].xadvance * pixelAdjustX;
                pWidth -= refGlyph[i].xadvance * pixelAdjustX;
                if (pWidth < distance) {
                    delete glyph;

                    retValue -= mayBeRemoved.size() - 1;
                    for (auto *rm : mayBeRemoved) {
                        scene->removeItem(rm);
                        delete rm;
                    }

                    break;
                }
            } else if (m_progression == PROGRESSION_BTT) {
                pen.ry() -= refGlyph[i].yadvance * pixelAdjustY;
                pWidth -= refGlyph[i].yadvance * pixelAdjustY;
                if (pWidth < distance) {
                    delete glyph;

                    retValue -= mayBeRemoved.size() - 1;
                    for (auto *rm : mayBeRemoved) {
                        scene->removeItem(rm);
                        delete rm;
                    }

                    break;
                }
            } else if (m_progression == PROGRESSION_LTR) {
                pWidth -= refGlyph[i].xadvance * pixelAdjustX;
                if (pWidth < distance) {
                    delete glyph;

                    retValue -= mayBeRemoved.size() - 1;
                    for (auto *rm : mayBeRemoved) {
                        scene->removeItem(rm);
                        delete rm;
                    }

                    break;
                }
            } else if (m_progression == PROGRESSION_TTB) {
                pWidth -= refGlyph[i].yadvance * pixelAdjustY;
                if (pWidth < distance) {
                    delete glyph;

                    retValue -= mayBeRemoved.size() - 1;
                    for (auto *rm : mayBeRemoved) {
                        scene->removeItem(rm);
                        delete rm;
                    }

                    break;
                }
            }

            /*************************************************/

            mayBeRemoved.append(glyph);

            if (renderReturnWidth)
                retValue += glyph->data(GLYPH_DATA_HADVANCE).toDouble() * scalefactor;
            else
                retValue = refGlyph[i].log;

            scene->addItem(glyph);
            glyph->setZValue(100.0);
            glyph->setData(GLYPH_DATA_GLYPH, "glyph");
            glyph->setData(GLYPH_DATA_FONTNAME, fancyName());
            glyph->setPos(pen.x() + (refGlyph[i].xoffset * pixelAdjustX) + glyph->data(GLYPH_DATA_BITMAPLEFT).toDouble() * scalefactor,
                          pen.y() - (refGlyph[i].yoffset * pixelAdjustY) - glyph->data(GLYPH_DATA_BITMAPTOP).toInt());
            /*************************************************/

            if (m_progression == PROGRESSION_LTR)
                pen.rx() += refGlyph[i].xadvance * pixelAdjustX;
            else if (m_progression == PROGRESSION_TTB)
                pen.ry() += refGlyph[i].yadvance * pixelAdjustY;
        }
    } else {
        QList<QGraphicsPathItem *> mayBeRemoved;
        for (int i = 0; i < refGlyph.size(); ++i) {
            QGraphicsPathItem *glyph = itemFromGindex(refGlyph[i].glyph, sizz);
            if (!glyph)
                continue;
            if (spec.at(refGlyph[i].log).category() == QChar::Separator_Space) {
                mayBeRemoved.clear();
            }

            if (m_progression == PROGRESSION_RTL) {
                pen.rx() -= refGlyph[i].xadvance * scalefactor;
                pWidth -= refGlyph[i].xadvance * scalefactor;
                if (pWidth < distance) {
                    delete glyph;

                    retValue -= mayBeRemoved.size() - 1;
                    for (auto *rm : mayBeRemoved) {
                        scene->removeItem(rm);
                        delete rm;
                    }

                    break;
                }
            } else if (m_progression == PROGRESSION_BTT) {
                pen.ry() -= refGlyph[i].yadvance * scalefactor;
                pWidth -= refGlyph[i].yadvance * scalefactor;
                if (pWidth < distance) {
                    delete glyph;

                    retValue -= mayBeRemoved.size() - 1;
                    for (auto *rm : mayBeRemoved) {
                        scene->removeItem(rm);
                        delete rm;
                    }

                    break;
                }
            } else if (m_progression == PROGRESSION_LTR) {
                pWidth -= refGlyph[i].xadvance * scalefactor;
                if (pWidth < distance) {
                    delete glyph;

                    retValue -= mayBeRemoved.size() - 1;
                    for (auto *rm : mayBeRemoved) {
                        scene->removeItem(rm);
                        delete rm;
                    }

                    break;
                }
            } else if (m_progression == PROGRESSION_TTB) {
                pWidth -= refGlyph[i].yadvance * scalefactor;
                if (pWidth < distance) {
                    delete glyph;

                    retValue -= mayBeRemoved.size() - 1;
                    for (auto *rm : mayBeRemoved) {
                        scene->removeItem(rm);
                        delete rm;
                    }

                    break;
                }
            }

            /**********************************************/
            mayBeRemoved.append(glyph);

            if (renderReturnWidth)
                retValue += glyph->data(GLYPH_DATA_HADVANCE).toDouble() * scalefactor;
            else
                retValue = refGlyph[i].log;

            scene->addItem(glyph);
            glyph->setPen(Qt::NoPen);
            glyph->setPos(pen.x() + (refGlyph[i].xoffset * scalefactor), pen.y() + (refGlyph[i].yoffset * scalefactor));
            glyph->setZValue(100.0);
            glyph->setData(GLYPH_DATA_GLYPH, "glyph");
            glyph->setData(GLYPH_DATA_FONTNAME, fancyName());
            /*******************************************/

            if (m_progression == PROGRESSION_LTR)
                pen.rx() += refGlyph[i].xadvance * scalefactor;
            if (m_progression == PROGRESSION_TTB)
                pen.ry() += refGlyph[i].yadvance * scalefactor;
        }
    }

    releaseFace();
    return retValue + 1;
}

/// Shaped line
double FontItem::renderLine(QString script, QGraphicsScene *scene, QString spec, QPointF origine, double lineWidth, double fsize)
{
    qCDebug(FONTMATRIX_LOG) << "Shaped(" << spec << ")";
    double retValue(0.0);
    if (spec.isEmpty())
        return 0;
    if (!m_isOpenType)
        return 0;
    ensureFace();

    otf = new FMOtf(m_face, 0x10000);
    if (!otf)
        return 0;

    FMShaperFactory *shaperfactory = nullptr;
    //	switch(m_shaperType)
    //	{
    //		case FMShaperFactory::FONTMATRIX : shaperfactory = new FMShaperFactory(otf,script, FMShaperFactory::FONTMATRIX );
    //		break;
    //		case FMShaperFactory::HARFBUZZ : shaperfactory = new FMShaperFactory(otf,script, FMShaperFactory::HARFBUZZ );
    //		break;
    //		case FMShaperFactory::ICU : shaperfactory = new FMShaperFactory(otf,script, FMShaperFactory::ICU );
    //		break;
    //		case FMShaperFactory::M17N : shaperfactory = new FMShaperFactory(otf,script, FMShaperFactory::M17N );
    //		break;
    //		case FMShaperFactory::PANGO : shaperfactory = new FMShaperFactory(otf,script, FMShaperFactory::PANGO );
    //		break;
    //		case FMShaperFactory::OMEGA : shaperfactory = new FMShaperFactory(otf,script, FMShaperFactory::OMEGA);
    //		break;
    //		default : shaperfactory = new FMShaperFactory(otf,script, FMShaperFactory::FONTMATRIX );
    //	}

    shaperfactory = new FMShaperFactory(otf, script, FMShaperFactory::FONTMATRIX);

    GlyphList refGlyph(shaperfactory->doShape(spec));
    delete shaperfactory;
    // drawn left to right: a right-to-left line is turned into that order first
    markCharacters(refGlyph, spec);
    FMBidi::toVisualOrder(refGlyph, FMBidi::isRightToLeft(refGlyph));

    double sizz = fsize;
    double scalefactor = sizz / unitsPerEm();
    double pixelAdjustX = scalefactor * (typotek::getInstance()->getDpiX() / 72.0);
    double pixelAdjustY = scalefactor * (typotek::getInstance()->getDpiX() / 72.0);
    double pWidth = lineWidth;
    const double distance = 20;

    // 	qDebug() << "Get line "<<spec;
    delete otf;
    otf = nullptr;
    // 	qDebug() << "Deleted OTF";
    if (refGlyph.isEmpty()) {
        return 0;
    }
    QPointF pen(origine);

    if (m_rasterFreetype) {
        QList<QGraphicsPixmapItem *> mayBeRemoved;
        for (int i = 0; i < refGlyph.size(); ++i) {
            QGraphicsPixmapItem *glyph = itemFromGindexPix(refGlyph[i].glyph, sizz);
            if (!glyph)
                continue;
            if (spec.at(refGlyph[i].log).category() == QChar::Separator_Space) {
                mayBeRemoved.clear();
            }

            if (m_progression == PROGRESSION_RTL) {
                pen.rx() -= refGlyph[i].xadvance * pixelAdjustX;
                pWidth -= refGlyph[i].xadvance * pixelAdjustX;
                if (pWidth < distance) {
                    delete glyph;

                    retValue -= mayBeRemoved.size() - 1;
                    for (auto *rm : mayBeRemoved) {
                        scene->removeItem(rm);
                        delete rm;
                    }

                    break;
                }
            } else if (m_progression == PROGRESSION_BTT) {
                pen.ry() -= refGlyph[i].yadvance * pixelAdjustY;
                pWidth -= refGlyph[i].yadvance * pixelAdjustY;
                if (pWidth < distance) {
                    delete glyph;

                    retValue -= mayBeRemoved.size() - 1;
                    for (auto *rm : mayBeRemoved) {
                        scene->removeItem(rm);
                        delete rm;
                    }

                    break;
                }
            } else if (m_progression == PROGRESSION_LTR) {
                pWidth -= refGlyph[i].xadvance * pixelAdjustX;
                if (pWidth < distance) {
                    delete glyph;

                    retValue -= mayBeRemoved.size() - 1;
                    for (auto *rm : mayBeRemoved) {
                        scene->removeItem(rm);
                        delete rm;
                    }

                    break;
                }
            } else if (m_progression == PROGRESSION_TTB) {
                pWidth -= refGlyph[i].yadvance * pixelAdjustY;
                if (pWidth < distance) {
                    delete glyph;

                    retValue -= mayBeRemoved.size() - 1;
                    for (auto *rm : mayBeRemoved) {
                        scene->removeItem(rm);
                        delete rm;
                    }

                    break;
                }
            }

            /*************************************************/

            mayBeRemoved.append(glyph);

            if (renderReturnWidth)
                retValue += glyph->data(GLYPH_DATA_HADVANCE).toDouble() * scalefactor;
            else
                retValue = refGlyph[i].log;

            scene->addItem(glyph);
            glyph->setZValue(100.0);
            glyph->setData(GLYPH_DATA_GLYPH, "glyph");
            glyph->setData(GLYPH_DATA_FONTNAME, fancyName());
            glyph->setPos(pen.x() + (refGlyph[i].xoffset * pixelAdjustX) + glyph->data(GLYPH_DATA_BITMAPLEFT).toDouble() * scalefactor,
                          pen.y() - (refGlyph[i].yoffset * pixelAdjustY) - glyph->data(GLYPH_DATA_BITMAPTOP).toInt());
            /*************************************************/

            if (m_progression == PROGRESSION_LTR)
                pen.rx() += refGlyph[i].xadvance * pixelAdjustX;
            else if (m_progression == PROGRESSION_TTB)
                pen.ry() += refGlyph[i].yadvance * pixelAdjustY;
        }
    } else {
        QList<QGraphicsPathItem *> mayBeRemoved;
        for (int i = 0; i < refGlyph.size(); ++i) {
            QGraphicsPathItem *glyph = itemFromGindex(refGlyph[i].glyph, sizz);
            if (!glyph)
                continue;
            if (spec.at(refGlyph[i].log).category() == QChar::Separator_Space) {
                mayBeRemoved.clear();
            }

            if (m_progression == PROGRESSION_RTL) {
                pen.rx() -= refGlyph[i].xadvance * scalefactor;
                pWidth -= refGlyph[i].xadvance * scalefactor;
                if (pWidth < distance) {
                    delete glyph;

                    retValue -= mayBeRemoved.size() - 1;
                    for (auto *rm : mayBeRemoved) {
                        scene->removeItem(rm);
                        delete rm;
                    }

                    break;
                }
            } else if (m_progression == PROGRESSION_BTT) {
                pen.ry() -= refGlyph[i].yadvance * scalefactor;
                pWidth -= refGlyph[i].yadvance * scalefactor;
                if (pWidth < distance) {
                    delete glyph;

                    retValue -= mayBeRemoved.size() - 1;
                    for (auto *rm : mayBeRemoved) {
                        scene->removeItem(rm);
                        delete rm;
                    }

                    break;
                }
            } else if (m_progression == PROGRESSION_LTR) {
                pWidth -= refGlyph[i].xadvance * scalefactor;
                if (pWidth < distance) {
                    delete glyph;

                    retValue -= mayBeRemoved.size() - 1;
                    for (auto *rm : mayBeRemoved) {
                        scene->removeItem(rm);
                        delete rm;
                    }

                    break;
                }
            } else if (m_progression == PROGRESSION_TTB) {
                pWidth -= refGlyph[i].yadvance * scalefactor;
                if (pWidth < distance) {
                    delete glyph;

                    retValue -= mayBeRemoved.size() - 1;
                    for (auto *rm : mayBeRemoved) {
                        scene->removeItem(rm);
                        delete rm;
                    }

                    break;
                }
            }

            /**********************************************/
            mayBeRemoved.append(glyph);

            if (renderReturnWidth)
                retValue += glyph->data(GLYPH_DATA_HADVANCE).toDouble() * scalefactor;
            else
                retValue = refGlyph[i].log;

            scene->addItem(glyph);
            glyph->setPen(Qt::NoPen);
            glyph->setPos(pen.x() + (refGlyph[i].xoffset * scalefactor), pen.y() - (refGlyph[i].yoffset * scalefactor));
            glyph->setZValue(100.0);
            glyph->setData(GLYPH_DATA_GLYPH, "glyph");
            /*******************************************/

            if (m_progression == PROGRESSION_LTR)
                pen.rx() += refGlyph[i].xadvance * scalefactor;
            if (m_progression == PROGRESSION_TTB)
                pen.ry() += refGlyph[i].yadvance * scalefactor;
        }
    }

    releaseFace();
    return retValue + 1;
}

void FontItem::deRenderAll()
{
    // 	qDebug() << m_name  <<"::deRenderAll()";
    // 	QSet<QGraphicsScene*> collectedScenes;
    for (int i = 0; i < pixList.size(); ++i) {
        if (pixList.at(i)->scene()) {
            // 			collectedScenes.insert ( pixList[i]->scene() );
            pixList.at(i)->scene()->removeItem(pixList.at(i));
            delete pixList.at(i);
        }
    }
    pixList.clear();
    for (int i = 0; i < glyphList.size(); ++i) {
        if (glyphList.at(i)->scene()) {
            // 			collectedScenes.insert ( pixList[i]->scene() );
            glyphList.at(i)->scene()->removeItem(glyphList.at(i));
            delete glyphList.at(i);
        }
    }
    glyphList.clear();
    for (int i = 0; i < labList.size(); ++i) {
        if (labList.at(i)->scene()) {
            // 			collectedScenes.insert ( pixList[i]->scene() );
            labList.at(i)->scene()->removeItem(labList.at(i));
            delete labList.at(i);
        }
    }
    labList.clear();
    for (int i = 0; i < selList.size(); ++i) {
        if (selList.at(i)->scene()) {
            // 			collectedScenes.insert ( pixList[i]->scene() );
            selList.at(i)->scene()->removeItem(selList.at(i));
            delete selList.at(i);
        }
    }
    selList.clear();
    allIsRendered = false;
}

QByteArray FontItem::pixarray(uchar *b, int len)
{
    uchar *imgdata = b;
    QByteArray buffer(len * 4, static_cast<char>(-1));
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    for (int i = 0; i < len; ++i) {
        stream << (quint8)~imgdata[i];
        stream << (quint8)~imgdata[i];
        stream << (quint8)~imgdata[i];
        stream << (quint8)imgdata[i];
    }

    return buffer;
}

int FontItem::firstChar()
{
    if (!ensureFace())
        return 0;

    FT_UInt anIndex(1);
    FT_UInt fc(FT_Get_First_Char(m_face, &anIndex));
    releaseFace();

    return fc;
}

int FontItem::lastChar()
{
    if (!ensureFace())
        return 0;

    FT_UInt index(1);
    FT_UInt cc = FT_Get_First_Char(m_face, &index);
    int lc(0);
    while (index) {
        lc = cc;
        cc = FT_Get_Next_Char(m_face, cc, &index);
    }

    releaseFace();
    return lc;
}

int FontItem::countChars()
{
    if (!ensureFace())
        return 0;

    FT_UInt index(1);
    FT_UInt cc = FT_Get_First_Char(m_face, &index);
    int n(0);
    while (index) {
        ++n;
        cc = FT_Get_Next_Char(m_face, cc, &index);
    }

    releaseFace();
    return n;
}

int FontItem::nextChar(int from, int offset)
{
    if (!ensureFace())
        return 0;

    FT_UInt index(1);
    int cc(from);
    for (int i(0); i < offset; ++i) {
        cc = FT_Get_Next_Char(m_face, cc, &index);
    }

    releaseFace();
    return cc;
}

int FontItem::countCoverage(int begin_code, int end_code)
{
    if (!ensureFace())
        return 0;
    // 	qDebug()<<"CC B E"<<begin_code<<end_code;
    FT_ULong charcode = begin_code;
    int count = 0;
    if (begin_code >= 0) {
        for (; charcode <= static_cast<FT_ULong>(end_code); ++charcode) {
            if (FT_Get_Char_Index(m_face, charcode))
                ++count;
        }
    } else {
        FT_UInt anIndex = 0;
        count = m_numGlyphs;
        FT_UInt anyChar = FT_Get_First_Char(m_face, &anIndex);
        while (anIndex) {
            anyChar = FT_Get_Next_Char(m_face, anyChar, &anIndex);
            if (anIndex)
                --count;
        }
    }
    releaseFace();
    return count; // something weird with freetype which put a valid glyph at the beginning of each lang ??? Or a bug here...
}

bool FontItem::hasCharcode(int cc)
{
    if (!ensureFace())
        return false;
    bool ret(true);
    if (!FT_Get_Char_Index(m_face, cc)) {
        ret = false;
    }
    releaseFace();
    return ret;
}

bool FontItem::hasChars(const QString &s)
{
    if (!ensureFace())
        return false;
    bool ret(true);

    for (const auto &c : s) {
        if (!FT_Get_Char_Index(m_face, c.unicode())) {
            ret = false;
            break;
        }
    }

    releaseFace();
    return ret;
}

void FontItem::renderAll(QGraphicsScene *scene, int begin_code, int end_code)
{
    ensureFace();

    FMGlyphsView *allView(nullptr);
    if (!scene->views().isEmpty())
        allView = reinterpret_cast<FMGlyphsView *>(scene->views().at(0));
    else {
        releaseFace();
        return;
    }

    deRenderAll();
    if (!allView->isVisible()) {
        releaseFace();
        return;
    }

    adjustGlyphsPerRow(allView->width());
    QRectF exposedRect(allView->visibleSceneRect());
    //        qDebug() << exposedRect;

    double leftMargin = ((exposedRect.width() - (100 * m_glyphsPerRow)) / 2) + 30;
    double aestheticTopMargin = 12;
    QPointF pen(leftMargin, 50 + aestheticTopMargin);

    int nl = 0;

    FT_ULong charcode;
    FT_UInt gindex = 1;
    double sizz = 50;
    charcode = begin_code;
    QPen selPen(Qt::gray);

    // A cell is the 100 x 100 square at (pen.x - 30, pen.y - 50); the glyph sits on the
    // baseline at pen, so it may reach 68 to the right, 28 to the left and 49 up before
    // it runs into the frame or the neighbour (a three-em dash, "Blackoak", the basmala).
    // Such a glyph is scaled down about its origin, which keeps it on the baseline.
    const auto fitToCell = [](QGraphicsPathItem *item) {
        const QRectF box(item->path().boundingRect());
        double scale = 1.0;
        if (box.right() > 68.0)
            scale = qMin(scale, 68.0 / box.right());
        if (box.left() < -28.0)
            scale = qMin(scale, -28.0 / box.left());
        if (box.top() < -49.0)
            scale = qMin(scale, -49.0 / box.top());
        if (scale < 1.0)
            item->setScale(scale);
    };

    QFont infoFont(typotek::getInstance()->getChartInfoFontName(), typotek::getInstance()->getChartInfoFontSize());
    QBrush selBrush(QColor(255, 255, 255, 0));
    QColor txtColor(60, 60, 60, 255);
    if (begin_code >= 0) {
        if (m_isEncoded) {
            while (charcode <= static_cast<FT_ULong>(end_code) && gindex) {
                if (nl == m_glyphsPerRow) {
                    nl = 0;
                    pen.rx() = leftMargin;
                    pen.ry() += 100;
                }
                if ((pen.y() + 100) < exposedRect.y() || pen.y() - 100 > (exposedRect.y() + exposedRect.height())) {
                    charcode = FT_Get_Next_Char(m_face, charcode, &gindex);
                    // 					qDebug() << "charcode = "<< charcode <<" ; gindex = "<< gindex;
                    pen.rx() += 100;
                    ++nl;

                    continue;
                }

                QGraphicsPathItem *pitem = itemFromChar(charcode, sizz);
                if (pitem) {
                    uint ucharcode = charcode;

                    scene->addItem(pitem);
                    pitem->setPen(Qt::NoPen);
                    pitem->setPos(pen);
                    fitToCell(pitem);
                    pitem->setData(1, "glyph");
                    pitem->setData(2, gindex);
                    pitem->setData(3, ucharcode);
                    glyphList.append(pitem);

                    pitem->setZValue(10);

                    QGraphicsTextItem *tit = scene->addText(glyphName(charcode), infoFont);
                    tit->setDefaultTextColor(txtColor);
                    tit->setPos(pen.x() - 27, pen.y() + 15);
                    tit->setData(1, "label");
                    tit->setData(2, gindex);
                    tit->setData(3, ucharcode);
                    labList.append(tit);
                    tit->setZValue(1);

                    QGraphicsTextItem *tit2 =
                        scene->addText("U+" + QString("%1").arg(charcode, 4, 16, QLatin1Char('0')) + " (" + QString::number(charcode) + ")", infoFont);
                    tit2->setDefaultTextColor(txtColor);
                    tit2->setPos(pen.x() - 27, pen.y() + 28);
                    tit2->setData(1, "label");
                    tit2->setData(2, gindex);
                    tit2->setData(3, ucharcode);
                    labList.append(tit2);
                    tit2->setZValue(1);

                    QGraphicsRectItem *rit = scene->addRect(pen.x() - 30, pen.y() - 50, 100, 100, selPen, selBrush);
                    rit->setFlag(QGraphicsItem::ItemIsSelectable, true);
                    rit->setData(1, "select");
                    rit->setData(2, gindex);
                    rit->setData(3, ucharcode);
                    rit->setZValue(100);
                    selList.append(rit);

                    pen.rx() += 100;
                    ++nl;
                }
                charcode = FT_Get_Next_Char(m_face, charcode, &gindex);
            }
        } else // Has not Unicode
        {
            // Here are fake charcodes (glyph index)
            while (charcode <= static_cast<FT_ULong>(end_code)) {
                if (nl == m_glyphsPerRow) {
                    nl = 0;
                    pen.rx() = leftMargin;
                    pen.ry() += 100;
                }

                if ((pen.y() + 100) < exposedRect.y() || pen.y() - 100 > (exposedRect.y() + exposedRect.height())) {
                    ++charcode;
                    ++nl;

                    continue;
                }

                QGraphicsPathItem *pitem = itemFromGindex(charcode, sizz);
                if (pitem) {
                    scene->addItem(pitem);
                    pitem->setPos(pen);
                    fitToCell(pitem);
                    pitem->setData(1, "glyph");
                    pitem->setData(2, gindex);
                    pitem->setData(3, 0);
                    glyphList.append(pitem);
                    pitem->setZValue(10);

                    QGraphicsTextItem *tit = scene->addText(QString("%1").arg(charcode, 4, 16, QLatin1Char('0')), infoFont);
                    tit->setDefaultTextColor(txtColor);
                    tit->setPos(pen.x(), pen.y() + 15);
                    tit->setData(1, "label");
                    tit->setData(2, gindex);
                    tit->setData(3, 0);
                    labList.append(tit);
                    tit->setZValue(1);

                    QGraphicsRectItem *rit = scene->addRect(pen.x() - 30, pen.y() - 50, 100, 100, selPen, selBrush);
                    rit->setFlag(QGraphicsItem::ItemIsSelectable, true);
                    rit->setData(1, "select");
                    rit->setData(2, gindex);
                    rit->setData(3, 0);
                    rit->setZValue(100);
                    selList.append(rit);

                    pen.rx() += 100;
                    ++nl;
                } else {
                    break;
                }
                ++charcode;
            }
        }
    } else // beginCode is negative - it means search for out charmap glyphs
    {
        // 1/ what is "out charmap"?
        FT_UInt anIndex = 1;
        QList<bool> notCovered;
        for (int i = 1; i < m_numGlyphs + 1; ++i)
            notCovered << true;
        FT_UInt anyChar = FT_Get_First_Char(m_face, &anIndex);
        while (anIndex) {
            anyChar = FT_Get_Next_Char(m_face, anyChar, &anIndex);
            if (anIndex && (anIndex <= static_cast<FT_UInt>(m_numGlyphs))) {
                notCovered[anIndex] = false;
            }
        }

        // 2/ fill with glyphs
        for (int i = 1; i < notCovered.size(); ++i) {
            if (!notCovered[i])
                continue;
            if (nl == m_glyphsPerRow) {
                nl = 0;
                pen.rx() = leftMargin;
                pen.ry() += 100;
            }

            if ((pen.y() + 100) < exposedRect.y() || pen.y() - 100 > (exposedRect.y() + exposedRect.height())) {
                ++nl;

                continue;
            }

            QGraphicsPathItem *pitem = itemFromGindex(i, sizz);
            if (pitem) {
                scene->addItem(pitem);
                pitem->setPos(pen);
                pitem->setData(1, "glyph");
                pitem->setData(2, i);
                pitem->setData(3, 0);
                glyphList.append(pitem);
                pitem->setZValue(10);

                QGraphicsTextItem *tit = scene->addText(QString("I+%1").arg(i), infoFont);
                tit->setDefaultTextColor(txtColor);
                tit->setPos(pen.x(), pen.y() + 15);
                tit->setData(1, "label");
                tit->setData(2, i);
                tit->setData(3, 0);
                labList.append(tit);
                tit->setZValue(1);

                QGraphicsTextItem *tit2 = scene->addText(glyphName(i, false), infoFont);
                tit2->setDefaultTextColor(txtColor);
                tit2->setPos(pen.x() - 27, pen.y() + 30);
                tit2->setData(1, "label");
                tit2->setData(2, i);
                labList.append(tit2);
                tit2->setZValue(1);

                QGraphicsRectItem *rit = scene->addRect(pen.x() - 30, pen.y() - 50, 100, 100, selPen, selBrush);
                rit->setFlag(QGraphicsItem::ItemIsSelectable, true);
                rit->setData(1, "select");
                rit->setData(2, i);
                rit->setData(3, 0);
                rit->setZValue(100);
                selList.append(rit);

                pen.rx() += 100;
                ++nl;
            }
        }
    }

    scene->setSceneRect(QRectF(0, 0, m_glyphsPerRow * 100 + 30, pen.y() + 100));
    allIsRendered = true;
    releaseFace();

    //        scene->blockSignals(false);
    //	exposedRect = allView->visibleSceneRect();
    // 	qDebug() << "ENDOFRENDERALL" <<exposedRect.x() << exposedRect.y() << exposedRect.width() << exposedRect.height();
}

int FontItem::renderChart(QGraphicsScene *scene, int begin_code, int end_code, double pwidth, double pheight)
{
    // 	qDebug() <<"FontItem::renderChart ("<< begin_code<<end_code <<")";

    ensureFace();
    int nl(0);
    int retValue(0);

    FT_ULong charcode;
    FT_UInt gindex = 1;
    double sizz = 50;
    charcode = begin_code;
    adjustGlyphsPerRow(qRound(pwidth));

    double leftMargin = 30 + ((pwidth - (m_glyphsPerRow * 100)) / 2);
    double aestheticTopMargin = 0;
    QPointF pen(leftMargin, sizz + aestheticTopMargin);

    QPen selPen(Qt::gray);
    QFont infoFont(typotek::getInstance()->getChartInfoFontName(), typotek::getInstance()->getChartInfoFontSize());
    QBrush selBrush(QColor(255, 255, 255, 0));

    while (charcode <= static_cast<FT_ULong>(end_code) && gindex) {
        if (nl == m_glyphsPerRow) {
            nl = 0;
            pen.rx() = leftMargin;
            pen.ry() += 100;
        }
        if (pen.y() > pheight - 30) {
            releaseFace();
            return retValue;
        }

        QGraphicsPathItem *pitem = itemFromChar(charcode, sizz);
        if (pitem) {
            uint ucharcode = charcode;

            scene->addItem(pitem);
            pitem->setPos(pen);
            pitem->setData(1, "glyph");
            pitem->setData(2, gindex);
            pitem->setData(3, ucharcode);
            // 			glyphList.append ( pitem );

            pitem->setZValue(10);

            QGraphicsTextItem *tit = scene->addText(glyphName(charcode), infoFont);
            tit->setPos(pen.x() - 27, pen.y() + 15);
            tit->setData(1, "label");
            tit->setData(2, gindex);
            tit->setData(3, ucharcode);
            // 			labList.append ( tit );
            tit->setZValue(1);

            QGraphicsTextItem *tit2 =
                scene->addText("U+" + QString("%1").arg(charcode, 4, 16, QLatin1Char('0')) + " (" + QString::number(charcode) + ")", infoFont);
            tit2->setPos(pen.x() - 27, pen.y() + 28);
            tit2->setData(1, "label");
            tit2->setData(2, gindex);
            tit2->setData(3, ucharcode);
            // 			labList.append ( tit2 );
            tit2->setZValue(1);

            QGraphicsRectItem *rit = scene->addRect(pen.x() - 30, pen.y() - 50, 100, 100, selPen, selBrush);
            rit->setFlag(QGraphicsItem::ItemIsSelectable, true);
            rit->setData(1, "select");
            rit->setData(2, gindex);
            rit->setData(3, ucharcode);
            rit->setZValue(100);

            pen.rx() += 100;
            ++nl;
            ++retValue;
        }
        retValue = charcode;
        charcode = FT_Get_Next_Char(m_face, charcode, &gindex);
    }
    releaseFace();

    return retValue;
}

QString FontItem::glyphName(int codepoint, bool codeIsChar)
{
    ensureFace();

    int index(0);

    if (codeIsChar) {
        index = FT_Get_Char_Index(m_face, codepoint);
        if (index == 0) {
            return "noname";
        }
    } else
        index = codepoint;

    QByteArray key(1001, 0);
    if (FT_HAS_GLYPH_NAMES(m_face)) {
        FT_Get_Glyph_Name(m_face, index, key.data(), 1000);
        if (key[0] == char(0)) {
            key = "noname";
        }
    } else {
        key = "noname";
    }
    return QString(key);
}

QString FontItem::infoGlyph([[maybe_unused]] int index, int code)
{
    ensureFace();
    QString ret;
    ret += glyphName(code);
    ret += ", " + i18nc("@info", "codepoint is U+");
    ret += QString("%1").arg(code, 4, 16, QChar(0x0030));
    ret += " (int" + QString::number(code) + ")";

    releaseFace();
    return ret;
}

// deprecated
QString FontItem::toElement()
{
    QString ret;
    ret = "<fontfile><file>%1</file><tag>%2</tag></fontfile>";
    return ret.arg(name(), tags().join("</tag><tag>"));
}

QGraphicsPathItem *FontItem::hasCodepointLoaded(int code)
{
    for (int i = 0; i < glyphList.size(); ++i) {
        if (glyphList.at(i)->data(3).toInt() == code)
            return glyphList.at(i);
    }
    return nullptr;
}

QPixmap FontItem::oneLinePreviewPixmap(QString oneline, QColor fg_color, QColor bg_color, int size_w, int size_f, const QList<double> &coords)
{
    if (m_remote && !remoteCached) {
        // what the directory gave, or nothing: the model then shows the name
        if (m_remotePreview.isNull() || size_w <= 0)
            return m_remotePreview;
        return m_remotePreview.scaledToWidth(qMin(size_w, m_remotePreview.width()), Qt::SmoothTransformation);
    }
    //	if ( m_remote )
    //		return fixedPixmap;
    //	if ( !theOneLinePreviewPixmap.isNull() )
    //	{
    //		if ( theOneLinePreviewPixmap.width() == size_w )
    //			return theOneLinePreviewPixmap;
    //	}
    if (!ensureFace())
        return QPixmap();
    // only for this drawing: the font keeps the coordinates it is shown with
    const bool otherCoords(!coords.isEmpty() && !m_axes.isEmpty());
    if (otherCoords)
        applyVariation(coords);
    double theSize = (size_f == 0) ? typotek::getInstance()->getPreviewSize() : size_f;
    double pt2px = typotek::getInstance()->getDpiX() / 72.0;
    double theHeight = theSize * 1.3 * pt2px;
    double theWidth;
    if (size_w == 0) {
        theWidth = theSize * pt2px * oneline.size() * 1.2;
    } else {
        theWidth = size_w;
    }
    // 	qDebug() << theSize << theHeight << theWidth;
    theOneLineScene->setSceneRect(0, 0, theWidth, theHeight);
    bool pRTL = typotek::getInstance()->getPreviewRTL();
    QPointF pen(pRTL ? theWidth - 16 : 16, theSize * pt2px);

    QPixmap linePixmap(qRound(theWidth), qRound(theHeight));
    linePixmap.fill(bg_color);
    QPainter apainter(&linePixmap);

    bool canRender(true);
    for (int i(0); i < oneline.size(); ++i) {
        if (FT_Get_Char_Index(m_face, oneline[i].unicode()) == 0) {
            canRender = false;
            break;
        }
    }
    if (canRender) {
        for (int i(0); i < oneline.size(); ++i) {
            int glyphIndex = FT_Get_Char_Index(m_face, oneline[i].unicode());

            if (!setSize(qRound(theSize)))
                continue;
            if (FT_Load_Glyph(m_face, glyphIndex, loadFlags(FT_LOAD_DEFAULT | FT_LOAD_NO_HINTING)) > 0)
                continue;
            if (FT_Render_Glyph(m_face->glyph, FT_RENDER_MODE_NORMAL) > 0)
                continue;

            QImage img(glyphImage(fg_color));
            double left = bitmapLeft();
            double top = bitmapTop();
            paintedGlyph(glyphIndex, img, left, top);
            if (pRTL)
                pen.rx() -= qRound(bitmapAdvance());
            apainter.drawImage(QPointF(pen.x() + left, pen.y() - top), img);
            if (!pRTL)
                pen.rx() += qRound(bitmapAdvance());
        }
    } else {
        apainter.drawText(pen.x(), pen.y(), i18nc("when doing the font preview, used to denote a font that can not displayed its name", "(%1)", oneline));
    }

    apainter.end();
    if (otherCoords)
        applyVariation();
    releaseFace();

    return linePixmap;
    //	theOneLinePreviewPixmap = linePixmap;

    //	if ( !theOneLinePreviewPixmap.isNull() )
    //		return theOneLinePreviewPixmap;

    //	theOneLinePreviewPixmap = QPixmap ( qRound(theWidth), qRound(theHeight) );
    //	theOneLinePreviewPixmap.fill ( Qt::lightGray );
    //	return theOneLinePreviewPixmap;
}

void FontItem::clearPreview()
{
    //	if ( m_remote )
    //		return;
    //	if ( !theOneLinePreviewPixmap.isNull() )
    //		theOneLinePreviewPixmap = QPixmap();
}

/// Variable fonts *******************************************

/// the name of the record, English first, then any other language of the record
QString FontItem::sfntName(unsigned int nameId)
{
    if (!m_face)
        return QString();
    QString english;
    QString other;
    const FT_UInt count = FT_Get_Sfnt_Name_Count(m_face);
    for (FT_UInt n = 0; n < count && english.isEmpty(); ++n) {
        FT_SfntName sn;
        if (FT_Get_Sfnt_Name(m_face, n, &sn) != 0 || sn.name_id != nameId || sn.string_len == 0)
            continue;
        const QByteArrayView bytes(reinterpret_cast<const char *>(sn.string), sn.string_len);
        QString value;
        if (sn.platform_id == TT_PLATFORM_MICROSOFT) {
            // every Microsoft name record is UTF-16BE
            value = QString(QStringDecoder(QStringDecoder::Utf16BE)(bytes)).trimmed();
        } else if (sn.platform_id == TT_PLATFORM_MACINTOSH && sn.encoding_id == TT_MAC_ID_ROMAN) {
            value = QString::fromLatin1(bytes).trimmed();
        }
        if (value.isEmpty())
            continue;
        if (sn.platform_id == TT_PLATFORM_MICROSOFT && sn.language_id == TT_MS_LANGID_ENGLISH_UNITED_STATES)
            english = value;
        else if (other.isEmpty())
            other = value;
    }
    return english.isEmpty() ? other : english;
}

void FontItem::readVariation()
{
    if (m_variationRead || !m_face)
        return;
    m_variationRead = true;
    if (!FT_HAS_MULTIPLE_MASTERS(m_face))
        return;
    FT_MM_Var *mm = nullptr;
    if (FT_Get_MM_Var(m_face, &mm) != 0 || !mm)
        return;
    constexpr double fixedOne = 65536.0;
    for (FT_UInt a = 0; a < mm->num_axis; ++a) {
        const FT_Var_Axis &ax = mm->axis[a];
        FontVariationAxis axis;
        const char tag[4] = {char(ax.tag >> 24), char(ax.tag >> 16), char(ax.tag >> 8), char(ax.tag)};
        axis.tag = QString::fromLatin1(tag, 4);
        axis.name = sfntName(ax.strid);
        if (axis.name.isEmpty() && ax.name)
            axis.name = QString::fromLatin1(ax.name);
        if (axis.name.isEmpty())
            axis.name = axis.tag;
        axis.minimum = ax.minimum / fixedOne;
        axis.def = ax.def / fixedOne;
        axis.maximum = ax.maximum / fixedOne;
        FT_UInt flags = 0;
        if (FT_Get_Var_Axis_Flags(mm, a, &flags) == 0)
            axis.hidden = (flags & FT_VAR_AXIS_FLAG_HIDDEN) != 0;
        m_axes << axis;
    }
    for (FT_UInt i = 0; i < mm->num_namedstyles; ++i) {
        const FT_Var_Named_Style &ns = mm->namedstyle[i];
        FontNamedInstance instance;
        instance.name = sfntName(ns.strid);
        if (instance.name.isEmpty())
            instance.name = i18nc("@item:inlistbox a named instance of a variable font that has no name, with its number", "Instance %1", i + 1);
        for (FT_UInt a = 0; a < mm->num_axis; ++a)
            instance.coords << ns.coords[a] / fixedOne;
        m_instances << instance;
    }
    FT_Done_MM_Var(FMFreetypeLib::lib(thread()), mm);

    // the coordinates the font was last shown with, in a session before (rememberVariation())
    const QString stored(FMConfig::value(QStringLiteral("Variations/") + m_path, QString()).toString());
    if (m_coords.isEmpty() && !stored.isEmpty()) {
        QHash<QString, double> byTag;
        for (const QString &part : stored.split(QLatin1Char(','), Qt::SkipEmptyParts)) {
            bool ok(false);
            const double value(part.section(QLatin1Char('='), 1).toDouble(&ok));
            if (ok)
                byTag.insert(part.section(QLatin1Char('='), 0, 0), value);
        }
        // by tag, not by position: an axis the font no longer has is dropped, a new one is at its default
        for (const FontVariationAxis &axis : std::as_const(m_axes))
            m_coords << byTag.value(axis.tag, axis.def);
    }
}

void FontItem::rememberVariation()
{
    if (!isVariable())
        return;
    const QString key(QStringLiteral("Variations/") + m_path);
    bool atDefault(true);
    for (int a(0); a < m_coords.size() && a < m_axes.size(); ++a)
        atDefault = atDefault && qFuzzyCompare(1.0 + m_coords.at(a), 1.0 + m_axes.at(a).def);
    if (atDefault) {
        FMConfig::remove(key);
        return;
    }
    QStringList parts;
    for (int a(0); a < m_coords.size() && a < m_axes.size(); ++a)
        parts << QStringLiteral("%1=%2").arg(m_axes.at(a).tag, QString::number(m_coords.at(a), 'g', 10));
    FMConfig::setValue(key, parts.join(QLatin1Char(',')));
}

void FontItem::applyVariation()
{
    applyVariation(m_coords);
}

void FontItem::applyVariation(const QList<double> &coords)
{
    if (!m_face || m_axes.isEmpty())
        return;
    if (coords.isEmpty()) {
        // back to the default of the font
        FT_Set_Var_Design_Coordinates(m_face, 0, nullptr);
        return;
    }
    QVarLengthArray<FT_Fixed, 8> fixed;
    for (const double c : coords)
        fixed.append(FT_Fixed(std::lround(c * 65536.0)));
    FT_Set_Var_Design_Coordinates(m_face, static_cast<FT_UInt>(fixed.size()), fixed.data());
}

bool FontItem::isVariable()
{
    if (!m_variationRead) {
        if (!ensureFace())
            return false;
        releaseFace();
    }
    return !m_axes.isEmpty();
}

QList<FontVariationAxis> FontItem::variationAxes()
{
    isVariable();
    return m_axes;
}

QList<FontNamedInstance> FontItem::namedInstances()
{
    isVariable();
    return m_instances;
}

void FontItem::setVariationCoordinates(const QList<double> &coords)
{
    if (!isVariable())
        return;
    QList<double> c(coords);
    // one coordinate per axis, whatever was given
    while (c.size() > m_axes.size())
        c.removeLast();
    for (int a = c.size(); a < m_axes.size() && !c.isEmpty(); ++a)
        c << m_axes.at(a).def;
    if (c == m_coords)
        return;
    m_coords = c;
    if (m_face) // held open by a caller: the next glyph it loads is at the new place already
        applyVariation();
    Q_EMIT variationChanged();
}

int FontItem::namedInstance()
{
    if (!isVariable())
        return -1;
    QList<double> shown(m_coords);
    if (shown.isEmpty()) {
        for (const FontVariationAxis &axis : std::as_const(m_axes))
            shown << axis.def;
    }
    for (int i = 0; i < m_instances.size(); ++i) {
        const QList<double> &c = m_instances.at(i).coords;
        bool same = c.size() == shown.size();
        for (int a = 0; same && a < c.size(); ++a)
            same = qAbs(c.at(a) - shown.at(a)) < 0.001;
        if (same)
            return i;
    }
    return -1;
}

FontInfoMap FontItem::moreInfo()
{
    FontInfoMap ret;
    if ((m_remote && !remoteCached) || !ensureFace())
        return ret;

    if (testFlag(m_face->face_flags, FT_FACE_FLAG_SFNT, "1", "0") == "1") {
        m_isOpenType = true;
    }

    if (m_isOpenType) {
        ret = moreInfo_sfnt();
    } else {
        ret = moreInfo_type1();
    }
    releaseFace();
    return ret;
}

QString FontItem::panose()
{
    if ((m_remote && !remoteCached) || !ensureFace())
        return QString("0:0:0:0:0:0:0:0:0:0");
    QStringList pl;
    auto os2 = static_cast<TT_OS2 *>(FT_Get_Sfnt_Table(m_face, ft_sfnt_os2));
    if (os2) {
        for (int bI(0); bI < 10; ++bI) {
            pl << QString::number(os2->panose[bI]);
        }
    } else {
        for (int bI(0); bI < 10; ++bI) {
            pl << QString::number(0);
        }
    }
    releaseFace();
    return pl.join(":");
}

QStringList FontItem::supportedLangDeclaration()
{
    QStringList ret;
    if (!ensureFace())
        return ret;

    auto os2 = static_cast<TT_OS2 *>(FT_Get_Sfnt_Table(m_face, ft_sfnt_os2));
    if (os2) {
        QList<FT_ULong> uMaskList;
        uMaskList << os2->ulUnicodeRange1 << os2->ulUnicodeRange2 << os2->ulUnicodeRange3 << os2->ulUnicodeRange4;
        const QMap<int, QPair<int, int>> &uranges(FMEncData::Os2URanges());
        unsigned int mask(1);
        for (int i(0); i < uMaskList.size(); ++i) {
            for (int j(0); j < 32; ++j) {
                unsigned int set(mask << j);
                if ((set & uMaskList[i]) > 0) {
                    int pos((i * 32) + j);
                    if (uranges.contains(pos)) {
                        QString b(FMUniBlocks::block(uranges[pos]));
                        if (!b.isEmpty())
                            ret << b;
                    }
                }
            }
        }
    }
    releaseFace();
    return ret;
}

double FontItem::italicAngle()
{
    double ret(0);
    if (!ensureFace())
        return ret;
    if (testFlag(m_face->face_flags, FT_FACE_FLAG_SFNT, "1", "0") == "1") {
        auto post = static_cast<TT_Postscript *>(FT_Get_Sfnt_Table(m_face, ft_sfnt_post));
        if (post)
            ret = (double(post->italicAngle) / double(0x10000));
    } else {
        PS_FontInfoRec sinfo;
        int err = FT_Get_PS_Font_Info(m_face, &sinfo);
        if (!err)
            ret = sinfo.italic_angle;
    }

    releaseFace();
    return ret;
}

FontItem::FsType FontItem::getFsType()
{
    // After some thinking, it appears that it would be a nonsense to not retrieve it from the actual font file.
    FsType fst(NOT_RESTRICTED);
    if (!ensureFace())
        return fst;

    auto os2 = static_cast<TT_OS2 *>(FT_Get_Sfnt_Table(m_face, ft_sfnt_os2));

    if (os2) {
        fst = FsType(os2->fsType);
    }

    releaseFace();
    return fst;
}

int FontItem::table(const QString &tableName)
{
    if (!ensureFace())
        return 0;

    if (!FT_IS_SFNT(m_face)) {
        releaseFace();
        return 0;
    }

    uint tag(OTF_name_tag(tableName));
    FT_ULong length(0);
    FT_Load_Sfnt_Table(m_face, tag, 0, nullptr, &length);

    releaseFace();

    return int(length);
}

QByteArray FontItem::tableData(const QString &tableName)
{
    QByteArray ret;
    if (!ensureFace())
        return ret;

    if (!FT_IS_SFNT(m_face)) {
        releaseFace();
        return ret;
    }

    uint tag(OTF_name_tag(tableName));
    FT_ULong length(0);
    if (!FT_Load_Sfnt_Table(m_face, tag, 0, nullptr, &length)) {
        if (length > 0) {
            ret.resize(length);
            FT_Load_Sfnt_Table(m_face, tag, 0, (FT_Byte *)ret.data(), &length);
        }
    }
    releaseFace();
    return ret;
}

/** reminder
FT_SfntName::name_id
Code  	Meaning
0 	Copyright
1 	Font Family
2 	Font Subfamily
3 	Unique font identifier
4 	Full font name
5 	Version string
6 	Postscript name for the font
7 	Trademark
8 	Manufacturer Name.
9 	Designer
10 	Description
11 	URL Vendor
12 	URL Designer
13 	License Description
14 	License Info URL
15 	Reserved; Set to zero.
16 	Preferred Family
17 	Preferred Subfamily
18 	Compatible Full (Macintosh only)
19 	Sample text
20 	PostScript CID findfont name
*/
// Mac OS Roman to Unicode for the upper half of the byte range; the lower
// half is ASCII.
static QString decodeMacRoman(const QByteArray &bytes)
{
    static const char16_t upper[128] = {
        0x00C4, 0x00C5, 0x00C7, 0x00C9, 0x00D1, 0x00D6, 0x00DC, 0x00E1, 0x00E0, 0x00E2, 0x00E4, 0x00E3, 0x00E5, 0x00E7, 0x00E9, 0x00E8, 0x00EA, 0x00EB, 0x00ED,
        0x00EC, 0x00EE, 0x00EF, 0x00F1, 0x00F3, 0x00F2, 0x00F4, 0x00F6, 0x00F5, 0x00FA, 0x00F9, 0x00FB, 0x00FC, 0x2020, 0x00B0, 0x00A2, 0x00A3, 0x00A7, 0x2022,
        0x00B6, 0x00DF, 0x00AE, 0x00A9, 0x2122, 0x00B4, 0x00A8, 0x2260, 0x00C6, 0x00D8, 0x221E, 0x00B1, 0x2264, 0x2265, 0x00A5, 0x00B5, 0x2202, 0x2211, 0x220F,
        0x03C0, 0x222B, 0x00AA, 0x00BA, 0x03A9, 0x00E6, 0x00F8, 0x00BF, 0x00A1, 0x00AC, 0x221A, 0x0192, 0x2248, 0x2206, 0x00AB, 0x00BB, 0x2026, 0x00A0, 0x00C0,
        0x00C3, 0x00D5, 0x0152, 0x0153, 0x2013, 0x2014, 0x201C, 0x201D, 0x2018, 0x2019, 0x00F7, 0x25CA, 0x00FF, 0x0178, 0x2044, 0x20AC, 0x2039, 0x203A, 0xFB01,
        0xFB02, 0x2021, 0x00B7, 0x201A, 0x201E, 0x2030, 0x00C2, 0x00CA, 0x00C1, 0x00CB, 0x00C8, 0x00CD, 0x00CE, 0x00CF, 0x00CC, 0x00D3, 0x00D4, 0xF8FF, 0x00D2,
        0x00DA, 0x00DB, 0x00D9, 0x0131, 0x02C6, 0x02DC, 0x00AF, 0x02D8, 0x02D9, 0x02DA, 0x00B8, 0x02DD, 0x02DB, 0x02C7};
    QString result;
    result.reserve(bytes.size());
    for (const char c : bytes) {
        const auto b = static_cast<unsigned char>(c);
        result.append(b < 0x80 ? QChar(b) : QChar(upper[b - 0x80]));
    }
    return result;
}

FontInfoMap FontItem::moreInfo_sfnt()
{
    if (!ensureFace())
        return FontInfoMap();

    FontInfoMap moreInfo;
    FT_SfntName tname;

    int tname_count = FT_Get_Sfnt_Name_Count(m_face);

    // TODO check encodings and platforms
    for (int i = 0; i < tname_count; ++i) {
        FT_Get_Sfnt_Name(m_face, i, &tname);
        int akey;
        if (tname.name_id > 255) {
            // 			qDebug() << name() <<" has vendor’s specific name id ->" << tname.name_id;
            if (tname.string_len > 0) {
                // 				akey = "VendorKey_" + QString::number ( tname.name_id );
                akey = tname.name_id;
            } else {
                continue;
            }

        } else if (tname.name_id <= FontStrings::Names().size()) {
            akey = tname.name_id;
        } else {
            // 			qDebug() << name() <<" : It seems there are new name IDs in TT spec ("<< tname.name_id <<")!";
            continue;
        }

        QString avalue;
        /// New plan, we’ll put here _user contributed_ statements!
        if (tname.platform_id == TT_PLATFORM_MICROSOFT
            && tname.encoding_id == TT_MS_ID_UNICODE_CS) // Corresponds to a Microsoft WGL4 charmap, matching Unicode.
        {
            QByteArray array((const char *)tname.string, tname.string_len);
            QStringDecoder decoder(QStringDecoder::Utf16BE);
            avalue = decoder(array);
        } else if (tname.platform_id == TT_PLATFORM_MICROSOFT
                   && tname.encoding_id
                       == TT_MS_ID_SYMBOL_CS) // Corresponds to Microsoft symbol encoding. PM - don(t understand what it does here? seen in StandardSym.ttf
        {
            avalue = "Here, imagine some nice symbols!";
        } else if (tname.platform_id == TT_PLATFORM_APPLE_UNICODE) // every Unicode platform encoding is UTF-16BE
        {
            QByteArray array((const char *)tname.string, tname.string_len);
            QStringDecoder decoder(QStringDecoder::Utf16BE);
            avalue = decoder(array);
        } else if (tname.platform_id == TT_PLATFORM_MACINTOSH) {
            // Encoding 0 is Mac Roman. The other Macintosh script encodings
            // are decoded the same way, as an approximation.
            avalue = decodeMacRoman(QByteArray((const char *)tname.string, tname.string_len));
        } else {
            avalue = "Unexpected platform - encoding pair (" + QString::number(tname.platform_id) + "," + QString::number(tname.encoding_id)
                + ")\nPlease contact Fontmatrix team.\nRun Fontmatrix in console to see more info.\nPlease, if possible, provide a font file to test.";

            qCDebug(FONTMATRIX_LOG) << m_name << "platform_id(" << tname.platform_id << ") - encoding_id(" << tname.encoding_id << ") - "
                                    << QString::number(tname.language_id) << FMEncData::LangIdMap()[tname.language_id];
        }

        if (!avalue.isEmpty()) {
            moreInfo[tname.language_id][akey] = avalue;
        }
    }

    // Is there an OS/2 table?
    // 	TT_OS2 *os2 = static_cast<TT_OS2*> ( FT_Get_Sfnt_Table ( m_face, ft_sfnt_os2 ) );
    // 	if ( os2 /* and  wantAutoTag*/ )
    // 	{
    // 		// PANOSE
    // 		QStringList pl;
    // 		for ( int bI ( 0 ); bI < 10; ++bI )
    // 		{
    //
    // 			pl << QString::number ( os2->panose[bI] ) ;
    // 		}
    //
    // 		moreInfo[0][FMFontDb::Panose] = pl.join(":");
    // 		// FSTYPE (embedding status)
    // 		if(!os2->fsType)
    // 			m_OSFsType = NOT_RESTRICTED;
    // 		else
    // 		{
    // 			if(os2->fsType & RESTRICTED)
    // 				m_OSFsType |= RESTRICTED;
    // 			if(os2->fsType & PREVIEW_PRINT)
    // 				m_OSFsType |= PREVIEW_PRINT;
    // 			if(os2->fsType &  EDIT_EMBED)
    // 				m_OSFsType |= EDIT_EMBED;
    // 			if(os2->fsType & NOSUBSET)
    // 				m_OSFsType |= NOSUBSET;
    // 			if(os2->fsType & BITMAP_ONLY)
    // 				m_OSFsType |=  BITMAP_ONLY;
    // 		}
    //
    // 	}

    releaseFace();
    return moreInfo;
}

QString FontItem::getAlternateFamilyName()
{
    if (!ensureFace())
        return QString();

    FT_SfntName tname;
    int tname_count = FT_Get_Sfnt_Name_Count(m_face);
    for (int i = 0; i < tname_count; ++i) {
        FT_Get_Sfnt_Name(m_face, i, &tname);
        if (tname.name_id == 1 && tname.language_id == 0) {
            return QString(QByteArray((const char *)tname.string, tname.string_len));
        }
    }

    releaseFace();
    return QString();
}

QString FontItem::getAlternateVariantName()
{
    if (!ensureFace())
        return QString();

    FT_SfntName tname;
    int tname_count = FT_Get_Sfnt_Name_Count(m_face);
    for (int i = 0; i < tname_count; ++i) {
        FT_Get_Sfnt_Name(m_face, i, &tname);
        if (tname.name_id == 2 && tname.language_id == 0) {
            return QString(QByteArray((const char *)tname.string, tname.string_len));
        }
    }

    releaseFace();
    return QString();
}

FontInfoMap FontItem::moreInfo_type1()
{
    if (!ensureFace())
        return FontInfoMap();

    FontInfoMap moreInfo;
    PS_FontInfoRec sinfo;
    int err = FT_Get_PS_Font_Info(m_face, &sinfo);
    if (err) {
        qCDebug(FONTMATRIX_LOG) << "FT_Get_PS_Font_Info(" << m_name << ")" << " failed :" << err;
        return FontInfoMap();
    }

    moreInfo[0][1] = sinfo.family_name;
    moreInfo[0][2] = sinfo.weight;
    moreInfo[0][4] = sinfo.full_name;
    moreInfo[0][5] = sinfo.version;
    moreInfo[0][10] = sinfo.notice;

    releaseFace();
    return moreInfo;
}

QStringList FontItem::tags() const
{
    return FMFontDb::DB()->getValue(m_path, FMFontDb::Tags).toStringList();
}

void FontItem::addTag(const QString &t)
{
    FMFontDb::DB()->addTag(m_path, t);
}

void FontItem::setTags(QStringList l)
{
    FMFontDb::DB()->setTags(m_path, l);
}

/// When glyphsView is resized we wantto adjust the number of columns
void FontItem::adjustGlyphsPerRow(int width)
{
    m_glyphsPerRow = 1;
    int extraAdjust = 30;
    for (int i = 1; i < 30; ++i) {
        if ((i * 100) + extraAdjust > width)
            return;
        else
            m_glyphsPerRow = i;
    }
}

bool FontItem::isActivated() const
{
    // 	if ( FMFontDb::DB()->getValue(m_path,FMFontDb::Activation ).toInt() > 0 )
    // 		return true;
    //
    return m_active;
}

void FontItem::setActivated(bool act)
{
    m_active = act;
    if (act) {
        FMFontDb::DB()->setValue(m_path, FMFontDb::Activation, 1);
    } else {
        FMFontDb::DB()->setValue(m_path, FMFontDb::Activation, 0);
    }
}

FMOtf *FontItem::takeOTFInstance()
{
    ensureFace();
    if (m_isOpenType)
        otf = new FMOtf(m_face);
    return otf;

    // It is a case where we don’t release face, thr caller have to call releaseOTFInstance;
}

void FontItem::releaseOTFInstance(FMOtf *rotf)
{
    if (rotf == otf) {
        delete otf;
        otf = nullptr;
    }
    releaseFace();
}

QStringList FontItem::features()
{
    QStringList ret;
    if (!takeOTFInstance()) {
        releaseOTFInstance(otf);
        return ret;
    }

    for (const auto tables = otf->get_tables(); const auto &table : tables) {
        otf->set_table(table);
        for (const auto scripts = otf->get_scripts(); const auto &script : scripts) {
            otf->set_script(script);
            for (const auto langs = otf->get_langs(); const auto &lang : langs) {
                otf->set_lang(lang);
                for (const auto featuresList = otf->get_features(); const auto &feature : featuresList) {
                    if (ret.contains(feature))
                        ret << feature;
                }
            }
        }
    }
    releaseOTFInstance(otf);
    return ret;
}

int FontItem::showFancyGlyph(QGraphicsView *view, int charcode, bool charcodeIsAGlyphIndex)
{
    ensureFace();

    int ref(fancyGlyphs.size());
    QRect allRect(view->rect());
    QRect targetRect(view->mapToScene(allRect.topLeft()).toPoint(), view->mapToScene(allRect.bottomRight()).toPoint());
    // 	qDebug() <<  allRect.topLeft() << view->mapToScene ( allRect.topLeft() );

    // We’ll try to have a square subRect that fit in view ;-)
    int squareSideUnit = qMin(allRect.width() * 0.1, allRect.height() * 0.1);
    int squareSide = 8 * squareSideUnit;
    int squareXOffset = (allRect.width() - squareSide) / 2;
    int squareYOffset = (allRect.height() - squareSide) / 2;
    QRect subRect(QPoint(squareXOffset, squareYOffset), QSize(squareSide, squareSide));
    QPixmap pix(allRect.width(), allRect.height());
    pix.fill(QColor(30, 0, 0, 120));
    QPainter painter(&pix);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setBrush(Qt::white);
    painter.setPen(QPen(QBrush(QColor(0, 0, 0, 255)), 3 /*, Qt::DashLine*/));
    painter.drawRoundedRect(subRect, 5, 5);
    painter.setPen(QPen(QColor(0, 0, 255, 120)));

    if (!setPixelSize(subRect.height() * 0.8)) {
        return -1;
    }
    if (!charcodeIsAGlyphIndex)
        ft_error = FT_Load_Char(m_face, charcode, loadFlags(FT_LOAD_RENDER));
    else
        ft_error = FT_Load_Glyph(m_face, charcode, loadFlags(FT_LOAD_RENDER));
    if (ft_error) {
        return -1;
    }

    // black, or the colours of the glyph, on the white box painted above
    QImage img(glyphImage());
    double left = bitmapLeft();
    double top = bitmapTop();
    paintedGlyph(charcodeIsAGlyphIndex ? charcode : int(FT_Get_Char_Index(m_face, FT_ULong(charcode))), img, left, top);

    double scaledBy = 1.0;
    if (img.width() > subRect.width()) {
        scaledBy = (double)subRect.width() / (double)img.width() * 0.8;
        // 		qDebug() <<"scaledBy = " << scaledBy ;
        img = img.scaledToWidth(qRound(subRect.width() * 0.8), Qt::SmoothTransformation);
    }

    QPoint gPos(subRect.topLeft());
    gPos.rx() += (subRect.width() - img.width()) / 2;
    gPos.ry() += (subRect.height() - img.height()) / 2;
    painter.drawImage(gPos, img);

    /// Draw metrics
    int iAngle(italicAngle());
    QPoint pPos(gPos);
    pPos.rx() -= qRound(left * scaledBy);
    pPos.ry() += qRound(top * scaledBy);
    double aF(tan((3.14 / 180.0) * iAngle));
    double asc(subRect.top() - pPos.y());
    double desc(pPos.y() - subRect.bottom());
    // left
    painter.drawLine(pPos.x() + (asc * aF), subRect.top(), pPos.x() - (desc * aF), subRect.bottom());
    // right
    painter.drawLine(qRound(pPos.x() + bitmapAdvance() * scaledBy) + (asc * aF),
                     subRect.top(),
                     qRound(pPos.x() + bitmapAdvance() * scaledBy) - (desc * aF),
                     subRect.bottom());
    // baseline
    painter.drawLine(subRect.left(), pPos.y(), subRect.right(), pPos.y());

    painter.end();

    auto fancyGlyph = new QGraphicsPixmapItem;
    fancyGlyph->setPixmap(pix);
    fancyGlyph->setZValue(10000);
    fancyGlyph->setPos(targetRect.topLeft());
    view->scene()->addItem(fancyGlyph);
    fancyGlyphs[ref] = fancyGlyph;

    auto textIt = new QGraphicsTextItem;
    textIt->setTextWidth(allRect.width());

    QString itemNameStyle("background-color:#000;color:#fff;font-weight:bold;font-size:13pt;padding:0 3px;");
    QString itemValueStyle("background-color:#fff;color:#000;font-size:9pt;padding:0 3px;");

    if (charcodeIsAGlyphIndex) {
        QString html(QString("<span style=\"%1\"> %2 </span> <span style=\"%3\"> - Index %4 <span>")
                         .arg(itemNameStyle, glyphName(charcode), itemValueStyle, QString::number(charcode)));
        textIt->setHtml(html);
    } else {
        QString catString;
        catString = FontStrings::UnicodeCategory(QChar::category(static_cast<uint>(charcode)));

        QString html(QString("<span style=\"%1\"> %2 </span> <span style=\"%3\"> %4 - U+%5  &#60;&#38;#%6;&#62; <span>")
                         .arg(itemNameStyle,
                              glyphName(charcode),
                              itemValueStyle,
                              catString,
                              QString("%1").arg(charcode, 4, 16, QChar('0')).toUpper(),
                              QString::number(charcode)));

        textIt->setHtml(html);
    }

    // 	qDebug()<< textIt->toHtml();
    //	QPointF tPos ( subRect.left() + 18.0 , subRect.bottom() );
    QRectF tRect(textIt->boundingRect());
    QPointF tPos(-3, targetRect.bottom() - tRect.height() + 5);
    textIt->setPos(tPos);
    textIt->setZValue(2000000);
    textIt->setEnabled(true);
    textIt->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
    textIt->setData(10, "FancyText");
    view->scene()->addItem(textIt);
    fancyTexts[ref] = textIt;

    // Alternates
    if (!charcodeIsAGlyphIndex && m_isOpenType) {
        QList<int> alts(getAlternates(charcode));
        qCDebug(FONTMATRIX_LOG) << "PALTS" << alts;
        double altSize(squareSide / 6.0);
        double altXOffset(subRect.top() + 10);
        for (int a(0); a < alts.size(); ++a) {
            QGraphicsPixmapItem *gpi(itemFromGindexPix(alts.at(a), altSize));
            fancyAlternates[ref] << gpi;

            QImage tmp_(gpi->pixmap().toImage());
            QImage altI(tmp_.width(), tmp_.height(), QImage::Format_Grayscale8);
            for (int ay = 0; ay < tmp_.height(); ++ay)
                for (int ax = 0; ax < tmp_.width(); ++ax)
                    altI.setPixel(ax, ay, qGray(qAlpha(tmp_.pixel(ax, ay)), qAlpha(tmp_.pixel(ax, ay)), qAlpha(tmp_.pixel(ax, ay))));
            QPixmap altP(altI.width() * 2, altI.height() * 2);
            altP.fill(Qt::transparent);
            QPainter altPainter(&altP);
            altPainter.setRenderHint(QPainter::Antialiasing, true);
            altPainter.setBrush(Qt::black);
            altPainter.drawRoundedRect(5, 5, altP.width() - 10, altP.height() - 10, 20, 20);
            altPainter.drawImage(qRound(altI.width() / 2.0), qRound(altI.height() / 2.0), altI);

            gpi->setPixmap(altP);

            view->scene()->addItem(gpi);
            gpi->setPos(view->mapToScene(subRect.right(), altXOffset));
            altXOffset += altP.height();
            gpi->setZValue(9999999);
            qCDebug(FONTMATRIX_LOG) << gpi->pos() << gpi->scenePos();
        }
    }

    releaseFace();
    return ref;
}

void FontItem::hideFancyGlyph(int ref)
{
    if (fancyGlyphs.contains(ref)) {
        QGraphicsPixmapItem *it = fancyGlyphs.value(ref);
        it->scene()->removeItem(it);
        fancyGlyphs.remove(ref);
        delete it;
    }
    if (fancyTexts.contains(ref)) {
        QGraphicsTextItem *it = fancyTexts.value(ref);
        it->scene()->removeItem(it);
        fancyTexts.remove(ref);
        delete it;
    }
    if (!fancyAlternates.value(ref).isEmpty()) {
        QList<QGraphicsPixmapItem *> pil(fancyAlternates.value(ref));
        for (int pidx(0); pidx < pil.size(); ++pidx) {
            QGraphicsPixmapItem *it = pil.at(pidx);
            it->scene()->removeItem(it);
            delete it;
        }
        fancyAlternates.remove(ref);
    }
}

bool FontItem::isLocal()
{
    QString shem = m_url.scheme();
    if (shem.isEmpty() || shem == "file")
        return true;
    return false;
}

/// We don’t want to download fonts yet. We just want something to fill font tree
bool FontItem::isRemotePath(const QString &path)
{
    return path.startsWith(QLatin1String("http://"), Qt::CaseInsensitive) || path.startsWith(QLatin1String("https://"), Qt::CaseInsensitive);
}

void FontItem::fileRemote(const QString &f, const QString &v, const QString &t, const QString &i, const QPixmap &p)
{
    m_family = f;
    m_variant = v;
    m_type = t;
    m_name = QFileInfo(QUrl(m_path).path()).fileName();
    m_remoteInfo = i;
    m_remotePreview = p;
    remoteHerePath = typotek::getInstance()->remoteTmpDir() + QDir::separator() + m_name;
    remoteCached = QFileInfo::exists(remoteHerePath);
}

/// the same, but just for speedup startup with a lot of font files
void FontItem::fileLocal(QString f, QString v, QString t, [[maybe_unused]] QString p)
{
    m_family = f;
    m_variant = v;
    m_type = t;
}

void FontItem::fileLocal(FontLocalInfo fli)
{
    m_family = fli.family;
    m_variant = fli.variant;
    m_type = fli.type;
    // 	m_panose = fli.panose;
    // 	moreInfo = fli.info;
    // 	if ( !fli.panose.isEmpty() )
    // 	{
    // 		for ( int bI ( 0 ); bI < 10; ++bI )
    // 		{
    // 			panoseInfo[ panoseKeys[bI] ] = panoseMap.value ( panoseKeys[bI] ).value ( m_panose.mid ( bI,1 ).toInt() ) ;
    //
    // 		}
    //
    // 	}
}

/// Finally, we have to download the font file
int FontItem::getFromNetwork()
{
    if (!m_remote || remoteCached)
        return 1;
    if (stopperDownload)
        return 2;

    const QDir dir(typotek::getInstance()->remoteTmpDir());
    if (!dir.exists() && !QDir().mkpath(dir.absolutePath())) {
        qCWarning(FONTMATRIX_LOG) << "Cannot create the directory for remote fonts" << dir.absolutePath();
        return 0;
    }
    rFile = new QFile(remoteHerePath);
    if (!rFile->open(QIODevice::WriteOnly)) {
        qCWarning(FONTMATRIX_LOG) << "Cannot write" << remoteHerePath;
        delete rFile;
        rFile = nullptr;
        return 0;
    }
    stopperDownload = true;

    rProgressDialog = new QProgressDialog(typotek::getInstance());
    rProgressDialog->setWindowTitle(i18nc("@title:window", "Downloading a Font"));
    rProgressDialog->setLabelText(i18nc("@info:progress", "Downloading %1", m_path));
    rProgressDialog->setMinimumDuration(1000);

    QNetworkReply *reply = typotek::getInstance()->network()->get(QNetworkRequest(QUrl(m_path)));
    connect(reply, &QNetworkReply::readyRead, this, [this, reply]() {
        if (rFile)
            rFile->write(reply->readAll());
    });
    connect(reply, &QNetworkReply::downloadProgress, this, [this](qint64 done, qint64 total) {
        if (rProgressDialog && total > 0) {
            rProgressDialog->setMaximum(static_cast<int>(qMin<qint64>(total, INT_MAX)));
            rProgressDialog->setValue(static_cast<int>(qMin<qint64>(done, INT_MAX)));
        }
    });
    connect(rProgressDialog, &QProgressDialog::canceled, reply, &QNetworkReply::abort);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        downloadEnd(reply);
    });
    return 2;
}

void FontItem::downloadEnd(QNetworkReply *reply)
{
    reply->deleteLater();
    const bool ok = (reply->error() == QNetworkReply::NoError);
    if (rFile) {
        if (ok)
            rFile->write(reply->readAll());
        rFile->close();
        if (!ok)
            rFile->remove();
        delete rFile;
        rFile = nullptr;
    }
    delete rProgressDialog;
    rProgressDialog = nullptr;
    stopperDownload = false;
    if (ok) {
        remoteCached = true;
        qCDebug(FONTMATRIX_LOG) << m_path << "downloaded to" << remoteHerePath;
    } else
        qCWarning(FONTMATRIX_LOG) << "Download of" << m_path << "failed:" << reply->errorString();
    Q_EMIT downloadFinished(ok);
}

void FontItem::trimSpacesIndex()
{
    if (!spaceIndex.isEmpty())
        return;
    if (!ensureFace())
        return;

    int gIndex(0);
    for (int i(0); i < legitimateNonPathChars.size(); ++i) {
        gIndex = FT_Get_Char_Index(m_face, legitimateNonPathChars.at(i));
        if (gIndex) {
            // 			qDebug()<<"Space : " << legitimateNonPathChars[i] << " is : "<<gIndex;
            spaceIndex << gIndex;
        }
    }

    releaseFace();
}

QString FontItem::activationName()
{
    // a remote font is activated through its downloaded copy
    if (m_remote && !remoteCached)
        return QString();

    QFileInfo fi(localPath());
    QString prefix("%1-");
    return prefix.arg(fi.size()) + fi.fileName();
}

QString FontItem::activationAFMName()
{
    if (m_remote && !remoteCached)
        return QString();
    if (m_afm.isEmpty())
        return QString();

    QFileInfo afi(m_afm);
    QFileInfo fi(m_path);
    QString prefix("%1-");
    return prefix.arg(fi.size()) + afi.fileName();
}

QList<int> FontItem::getAlternates(int ccode)
{
    QList<int> ret;
    if (!ensureFace())
        return ret;
    if (!otf && m_isOpenType) {
        otf = new FMOtf(m_face);
        if (!otf)
            return ret;
    }

    int glyphIndex(FT_Get_Char_Index(m_face, ccode));
    QList<OTFSet> setList;
    setList.clear();

    otf->set_table("GSUB");
    for (const auto scriptsList = otf->get_scripts(); const auto &script : scriptsList) {
        otf->set_script(script);
        for (const auto langsList = otf->get_langs(); const auto &lang : langsList) {
            otf->set_lang(lang);
            QStringList fl(otf->get_features());
            if (fl.contains("aalt")) {
                OTFSet set;
                set.script = script;
                set.lang = lang;
                set.gpos_features.clear();
                set.gsub_features = QStringList("aalt");
                setList << set;
                qCDebug(FONTMATRIX_LOG) << "AALT" << script << lang;
            }
        }
    }

    QString spec;
    spec = QChar(ccode);

    for (const auto &set : std::as_const(setList)) {
        QList<RenderedGlyph> rendered(otf->procstring(spec, set));
        if (rendered.isEmpty())
            continue;
        if (rendered.at(0).glyph != glyphIndex) {
            if (!ret.contains(rendered.at(0).glyph))
                ret << rendered.at(0).glyph;
        }
        if (!otf->altGlyphs.isEmpty()) {
            QList<int> l(otf->altGlyphs);
            for (const auto &g : l) {
                if (!ret.contains(g) && g != glyphIndex)
                    ret << g;
            }
        }
    }

    delete otf;
    otf = nullptr;

    releaseFace();
    return ret;
}

QImage FontItem::glyphImage(QColor color)
{
    const FT_Bitmap &bitmap = m_face->glyph->bitmap;
    if (bitmap.pixel_mode == FT_PIXEL_MODE_BGRA) {
        // a colour glyph: FreeType's BGRA, premultiplied, is Qt's ARGB32_Premultiplied byte for byte
        QImage colorImg(int(bitmap.width), int(bitmap.rows), QImage::Format_ARGB32_Premultiplied);
        const unsigned char *row = bitmap.buffer;
        for (int r = 0; r < int(bitmap.rows); ++r) {
            memcpy(colorImg.scanLine(r), row, size_t(bitmap.width) * 4);
            row += bitmap.pitch;
        }
        return scaledToStrike(colorImg);
    }
    QImage img(m_face->glyph->bitmap.width, m_face->glyph->bitmap.rows, QImage::Format_Indexed8);
    // 	QImage img ( m_face->glyph->bitmap.buffer,
    // 	             m_face->glyph->bitmap.width,
    // 	             m_face->glyph->bitmap.rows,
    // 	             m_face->glyph->bitmap.pitch,
    // 	             QImage::Format_Indexed8 );

    // 	qDebug()<<"GSlot"<< m_face->glyph->bitmap.buffer
    // 			<< m_face->glyph->bitmap.width
    // 			<< m_face->glyph->bitmap.rows
    // 			<< m_face->glyph->bitmap.pitch ;

    if ((m_face->glyph->bitmap.num_grays != 256) || (color != QColor(Qt::black))) {
        QVector<QRgb> palette;
        palette.clear();
        int r(color.red());
        int g(color.green());
        int b(color.blue());
        for (int aa = 0; aa < m_face->glyph->bitmap.num_grays; ++aa) {
            palette << qRgba(r, g, b, aa);
        }
        img.setColorTable(palette);
    } else {
        img.setColorTable(gray256Palette);
    }

    unsigned char *cursor(m_face->glyph->bitmap.buffer);
    // 	QString dbs;
    for (int r(0); r < static_cast<int>(m_face->glyph->bitmap.rows); ++r) {
        // 		dbs.clear();
        for (int x(0); x < static_cast<int>(m_face->glyph->bitmap.width); ++x) {
            img.setPixel(x, r, *(cursor + x));
            // 			dbs += (*(cursor + x) > 0) ? "+" : ".";
        }
        // 		qDebug()<<dbs;
        cursor += m_face->glyph->bitmap.pitch;
    }

    return scaledToStrike(img);
}

/// Colour fonts and bitmap-only fonts *******************************************

QImage FontItem::scaledToStrike(const QImage &img) const
{
    if (qFuzzyCompare(m_bitmapScale, 1.0) || img.isNull())
        return img;
    return img.scaled(qMax(1, qRound(img.width() * m_bitmapScale)),
                      qMax(1, qRound(img.height() * m_bitmapScale)),
                      Qt::IgnoreAspectRatio,
                      Qt::SmoothTransformation);
}

double FontItem::unitsPerEm() const
{
    if (m_face && m_face->units_per_EM > 0)
        return m_face->units_per_EM;
    if (m_headUnitsPerEm > 0.0)
        return m_headUnitsPerEm;
    return 1000.0;
}

bool FontItem::selectStrike(double ppem)
{
    if (!m_face || m_face->num_fixed_sizes <= 0 || !m_face->available_sizes)
        return false;
    int best = 0;
    for (int i = 1; i < m_face->num_fixed_sizes; ++i) {
        if (qAbs(m_face->available_sizes[i].y_ppem / 64.0 - ppem) < qAbs(m_face->available_sizes[best].y_ppem / 64.0 - ppem))
            best = i;
    }
    if (FT_Select_Size(m_face, best) != 0)
        return false;
    const double strikePpem = m_face->available_sizes[best].y_ppem / 64.0;
    m_bitmapScale = (ppem > 0.0 && strikePpem > 0.0) ? ppem / strikePpem : 1.0;
    return true;
}

bool FontItem::setSize(double size)
{
    m_bitmapScale = 1.0;
    if (FT_IS_SCALABLE(m_face))
        return FT_Set_Char_Size(m_face, qRound(size * 64), 0, typotek::getInstance()->getDpiX(), typotek::getInstance()->getDpiY()) == 0;
    return selectStrike(size * typotek::getInstance()->getDpiY() / 72.0);
}

bool FontItem::setPixelSize(double pixels)
{
    m_bitmapScale = 1.0;
    if (FT_IS_SCALABLE(m_face))
        return FT_Set_Pixel_Sizes(m_face, 0, FT_UInt(qRound(pixels))) == 0;
    return selectStrike(pixels);
}

FT_Int32 FontItem::loadFlags(FT_Int32 flags) const
{
    // An SVG glyph is drawn here (FMSvgGlyphs), not by FreeType, which would want rendering
    // hooks and fails the load without them: its outline gives the metrics and the fallback.
    return (m_hasColor && !m_svgGlyphs) ? (flags | FT_LOAD_COLOR) : flags;
}

void FontItem::metricsToUnits()
{
    // the strike's 26.6 pixels into the units of the design
    const double ppem = m_face->size ? m_face->size->metrics.y_ppem : 0;
    if (ppem <= 0)
        return;
    const double toUnits = unitsPerEm() / (64.0 * ppem);
    FT_Glyph_Metrics &m = m_glyph->metrics;
    m.width = FT_Pos(std::lround(m.width * toUnits));
    m.height = FT_Pos(std::lround(m.height * toUnits));
    m.horiBearingX = FT_Pos(std::lround(m.horiBearingX * toUnits));
    m.horiBearingY = FT_Pos(std::lround(m.horiBearingY * toUnits));
    m.horiAdvance = FT_Pos(std::lround(m.horiAdvance * toUnits));
    m.vertBearingX = FT_Pos(std::lround(m.vertBearingX * toUnits));
    m.vertBearingY = FT_Pos(std::lround(m.vertBearingY * toUnits));
    m.vertAdvance = FT_Pos(std::lround(m.vertAdvance * toUnits));
}

FT_Error FontItem::loadUnscaled(int index)
{
    if (FT_IS_SCALABLE(m_face))
        return FT_Load_Glyph(m_face, index, FT_LOAD_NO_SCALE);
    // a bitmap-only font: the metrics of a strike, the biggest when none is selected, in font units
    if ((!m_face->size || m_face->size->metrics.y_ppem == 0) && !selectStrike(1e6))
        return FT_Err_Invalid_Pixel_Size;
    const double scale = m_bitmapScale;
    const FT_Error error = FT_Load_Glyph(m_face, index, loadFlags(FT_LOAD_DEFAULT));
    m_bitmapScale = scale;
    if (error)
        return error;
    metricsToUnits();
    return 0;
}

double FontItem::bitmapLeft() const
{
    return m_glyph->bitmap_left * m_bitmapScale;
}

double FontItem::bitmapTop() const
{
    return m_glyph->bitmap_top * m_bitmapScale;
}

double FontItem::bitmapAdvance() const
{
    return m_glyph->advance.x / 64.0 * m_bitmapScale;
}

bool FontItem::paintedGlyph(int index, QImage &img, double &left, double &top)
{
    if (!m_face || !m_face->size || index < 0)
        return false;
    const double ppem = m_face->size->metrics.y_ppem * m_bitmapScale;
    // OpenType-SVG first: where a font has it, it is the richest of its pictures
    if (m_svgGlyphs && m_svgGlyphs->hasGlyph(unsigned(index)))
        return m_svgGlyphs->paint(unsigned(index), ppem, img, left, top);
    if (!m_paintFont)
        return false;
    QImage painted;
    double paintedLeft = 0.0;
    double paintedTop = 0.0;
    if (!FMColorPainter::paint(m_paintFont, FT_UInt(index), ppem, Qt::black, painted, paintedLeft, paintedTop))
        return false;
    img = painted;
    left = paintedLeft;
    top = paintedTop;
    return true;
}

bool FontItem::hasColor()
{
    if (!m_faceSeen) {
        if (!ensureFace())
            return false;
        releaseFace();
    }
    return m_hasColor;
}

QStringList FontItem::colorTables()
{
    QStringList ret;
    if (!hasColor())
        return ret;
    for (const char *name : {"COLR", "CPAL", "CBDT", "sbix", "SVG "}) {
        if (table(QString::fromLatin1(name)) > 0)
            ret << QString::fromLatin1(name).trimmed();
    }
    return ret;
}

QGraphicsPathItem *FontItem::colorItem(int index, double scalefactor)
{
    if (!m_hasColor || !m_face)
        return nullptr;

    // OpenType-SVG: drawn by Qt SVG, as sharp as COLR version 1 below
    if (m_svgGlyphs && m_svgGlyphs->hasGlyph(unsigned(index))) {
        const double ppem = qBound(64.0, scalefactor * unitsPerEm() * 4.0, 1024.0);
        QImage img;
        double left = 0.0;
        double top = 0.0;
        if (m_svgGlyphs->paint(unsigned(index), ppem, img, left, top)) {
            const double perPixel = scalefactor * unitsPerEm() / ppem;
            auto *item = new FMColorGlyphItem;
            item->setImage(img, QRectF(left * perPixel, -top * perPixel, img.width() * perPixel, img.height() * perPixel));
            return item;
        }
    }

    // COLR version 1: painted, sharp enough for the size the item is scaled to
    if (m_paintFont && FMColorPainter::hasPaint(m_paintFont, FT_UInt(index))) {
        const double ppem = qBound(64.0, scalefactor * unitsPerEm() * 4.0, 1024.0);
        QImage img;
        double left = 0.0;
        double top = 0.0;
        if (FMColorPainter::paint(m_paintFont, FT_UInt(index), ppem, Qt::black, img, left, top)) {
            const double perPixel = scalefactor * unitsPerEm() / ppem;
            auto *item = new FMColorGlyphItem;
            item->setImage(img, QRectF(left * perPixel, -top * perPixel, img.width() * perPixel, img.height() * perPixel));
            return item;
        }
    }

    // layers of outlines, each with its colour from the palette (COLR version 0)
    QList<GlyphLayer> layers;
    FT_LayerIterator iterator;
    iterator.p = nullptr;
    FT_UInt layerGlyph = 0;
    FT_UInt colorIndex = 0;
    FT_Color *palette = nullptr;
    FT_Palette_Data paletteData;
    paletteData.num_palette_entries = 0;
    if (FT_Palette_Data_Get(m_face, &paletteData) != 0 || FT_Palette_Select(m_face, 0, &palette) != 0)
        palette = nullptr;
    while (FT_Get_Color_Glyph_Layer(m_face, FT_UInt(index), &layerGlyph, &colorIndex, &iterator)) {
        if (FT_Load_Glyph(m_face, layerGlyph, FT_LOAD_NO_SCALE) != 0)
            continue;
        GlyphLayer layer;
        SizedPath sp{};
        sp.p = &layer.path;
        sp.s = scalefactor;
        FT_Outline_Decompose(&m_glyph->outline, &outline_funcs, &sp);
        layer.path.closeSubpath();
        if (palette && colorIndex < paletteData.num_palette_entries) {
            const FT_Color &c = palette[colorIndex];
            layer.color = QColor(c.red, c.green, c.blue, c.alpha);
        }
        layers << layer;
    }
    if (!layers.isEmpty()) {
        auto *item = new FMColorGlyphItem;
        item->setLayers(layers);
        return item;
    }

    // a bitmap (CBDT, sbix): its biggest strike, in the box the metrics give
    if (!FT_HAS_FIXED_SIZES(m_face) || !selectStrike(1e6))
        return nullptr;
    m_bitmapScale = 1.0;
    if (FT_Load_Glyph(m_face, index, loadFlags(FT_LOAD_DEFAULT)) != 0 || FT_Render_Glyph(m_glyph, FT_RENDER_MODE_NORMAL) != 0)
        return nullptr;
    if (m_glyph->bitmap.pixel_mode != FT_PIXEL_MODE_BGRA || m_glyph->bitmap.width == 0)
        return nullptr;
    const QImage img(glyphImage());
    metricsToUnits();
    const FT_Glyph_Metrics &m = m_glyph->metrics;
    const QRectF box(m.horiBearingX * scalefactor, -m.horiBearingY * scalefactor, m.width * scalefactor, m.height * scalefactor);
    auto *item = new FMColorGlyphItem;
    item->setImage(img, box);
    return item;
}

FontInfoMap FontItem::rawInfo()
{
    return FMFontDb::DB()->getInfoMap(m_path);
}

int FontItem::shaperType() const
{
    return m_shaperType;
}

void FontItem::setShaperType(int theValue)
{
    m_shaperType = theValue;
}

GlyphList FontItem::glyphs(QString spec, double fsize)
{
    // 	qDebug()<<"glyphs ("<< spec.left(24) <<", "<<fsize<<" )";
    FMHyphenator *hyph = typotek::getInstance()->getHyphenator();
    GlyphList ret;
    if (spec.isEmpty() || fsize <= 0.0)
        return ret;
    if (!ensureFace())
        return ret;
    double scalefactor = fsize / unitsPerEm();

    QChar spaceChar(' ');
    int startSpaceCount(0);
    int endSpaceCount(0);
    int specCount(spec.size());
    for (int s(0); s < specCount; ++s) {
        if (spec.at(s) == spaceChar)
            ++startSpaceCount;
        else
            break;
    }
    if (startSpaceCount != specCount) {
        for (int s(specCount - 1); s >= 0; --s) {
            if (spec.at(s) == spaceChar)
                ++endSpaceCount;
            else
                break;
        }
    }

    QStringList stl(spec.split(spaceChar, Qt::SkipEmptyParts));

    QGraphicsPathItem *glyph = itemFromChar(spaceChar.unicode(), fsize);
    RenderedGlyph wSpace(glyph->data(GLYPH_DATA_GLYPH).toInt(), 0, glyph->data(GLYPH_DATA_HADVANCE).toDouble() * scalefactor, 0, 0, 0, ' ', false);
    delete glyph;
    for (int s(0); s < startSpaceCount; ++s) {
        ret << wSpace;
    }
    for (QStringList::const_iterator sIt(stl.constBegin()); sIt != stl.constEnd(); ++sIt) {
        if (sIt != stl.constBegin()) {
            ret << wSpace;
        }
        HyphList hl;
        if (hyph) {
            hl = hyph->hyphenate(*sIt);
            // 			if(hl.size())qDebug()<<"Hyph W C"<<*sIt<<hl.size();
        }

        for (int i(0); i < (*sIt).size(); ++i) {
            glyph = itemFromChar((*sIt).at(i).unicode(), fsize);
            if (!glyph) {
                continue;
            }
            RenderedGlyph rg;
            rg.glyph = glyph->data(GLYPH_DATA_GLYPH).toInt();
            rg.log = i; // We are in a 1/1 relation
            rg.lChar = (*sIt).at(i).unicode();
            if (QChar::hasMirrored(rg.lChar))
                rg.mirrorGlyph = int(FT_Get_Char_Index(m_face, QChar::mirroredChar(char32_t(rg.lChar))));
            rg.xadvance = glyph->data(GLYPH_DATA_HADVANCE).toDouble() * scalefactor;
            rg.yadvance = glyph->data(GLYPH_DATA_VADVANCE).toDouble() * scalefactor;
            rg.xoffset = 0;
            rg.yoffset = 0;
            delete glyph;
            if (hl.contains(i)) {
                // 				qDebug()<<"H B A"<<i<<hl[i].first<<hl[i].second;
                rg.isBreak = true;
                QString addOnFirst;
                QString addOnSecond;
                addOnFirst = hl[i].first.endsWith("-") ? "" : "-";
                addOnSecond = /*(*sIt).endsWith(".")?".":*/ "";
                QString bS(hl[i].first + addOnFirst);
                for (int bI(0); bI < bS.size(); ++bI) {
                    // 					qDebug()<<"i bI a"<<i<<bI<<bS.at ( bI );
                    glyph = itemFromChar(bS.at(bI).unicode(), fsize);
                    if (!glyph) {
                        continue;
                    }
                    RenderedGlyph bg;
                    bg.glyph = glyph->data(GLYPH_DATA_GLYPH).toInt();
                    // 					bg.log = ; // We are in a 1/1 relation
                    bg.lChar = bS.at(bI).unicode();
                    bg.xadvance = glyph->data(GLYPH_DATA_HADVANCE).toDouble() * scalefactor;
                    bg.yadvance = glyph->data(GLYPH_DATA_VADVANCE).toDouble() * scalefactor;
                    bg.xoffset = 0;
                    bg.yoffset = 0;
                    delete glyph;
                    rg.hyphen.first << bg;
                }
                bS = hl[i].second + addOnSecond;
                for (int bI(0); bI < bS.size(); ++bI) {
                    glyph = itemFromChar(bS.at(bI).unicode(), fsize);
                    if (!glyph) {
                        continue;
                    }
                    RenderedGlyph bg;
                    bg.glyph = glyph->data(GLYPH_DATA_GLYPH).toInt();
                    // 					bg.log = i; // We are in a 1/1 relation
                    bg.lChar = bS.at(bI).unicode();
                    bg.xadvance = glyph->data(GLYPH_DATA_HADVANCE).toDouble() * scalefactor;
                    bg.yadvance = glyph->data(GLYPH_DATA_VADVANCE).toDouble() * scalefactor;
                    bg.xoffset = 0;
                    bg.yoffset = 0;
                    delete glyph;
                    rg.hyphen.second << bg;
                }
            }
            ret << rg;
        }
    }

    for (int s(0); s < endSpaceCount; ++s) {
        ret << wSpace;
    }
    releaseFace();
    // 	qDebug()<<"EndOfGlyphs";
    return ret;
}

void FontItem::markCharacters(GlyphList &glyphs, const QString &word)
{
    for (RenderedGlyph &g : glyphs) {
        // log is the index of the character in the word: HarfBuzz gives one cluster per character
        if (g.log >= 0 && g.log < word.size())
            g.lChar = word.at(g.log).unicode();
        if (g.lChar && QChar::hasMirrored(g.lChar))
            g.mirrorGlyph = int(FT_Get_Char_Index(m_face, QChar::mirroredChar(char32_t(g.lChar))));
    }
}

GlyphList FontItem::shapeWords(const QString &spec, double fsize, const std::function<GlyphList(const QString &)> &shapeWord)
{
    FMHyphenator *hyph = typotek::getInstance()->getHyphenator();
    GlyphList Gret;
    if (spec.isEmpty() || fsize <= 0.0 || !m_isOpenType) // enough :-)
        return Gret;
    if (!ensureFace())
        return Gret;
    otf = new FMOtf(m_face);

    // the engine works in font units
    const double scalefactor = fsize / unitsPerEm();
    const auto scaled = [scalefactor](GlyphList gl) {
        for (RenderedGlyph &g : gl) {
            g.xadvance *= scalefactor;
            g.yadvance *= scalefactor;
            g.xoffset *= scalefactor;
            g.yoffset *= scalefactor;
        }
        return gl;
    };

    QGraphicsPathItem *glyph = itemFromChar(QChar(' ').unicode(), fsize);
    RenderedGlyph wSpace(glyph->data(GLYPH_DATA_GLYPH).toInt(), 0, glyph->data(GLYPH_DATA_HADVANCE).toDouble() * scalefactor, 0, 0, 0, ' ', false);
    wSpace.lChar = 0x20;
    delete glyph;

    const QStringList stl(spec.split(' ', Qt::SkipEmptyParts));
    for (QStringList::const_iterator sIt(stl.constBegin()); sIt != stl.constEnd(); ++sIt) {
        if (sIt != stl.constBegin()) {
            Gret << wSpace;
        }
        HyphList hl;
        if (hyph) {
            hl = hyph->hyphenate(*sIt);
        }
        GlyphList ret(scaled(shapeWord(*sIt)));
        markCharacters(ret, *sIt);
        for (int i(0); i < ret.size(); ++i) {
            // the breaks are keyed by character, which is log, not by glyph
            const auto brk(hl.constFind(ret[i].log));
            if (brk != hl.constEnd()) {
                ret[i].isBreak = true;
                const QString addOnFirst(brk.value().first.endsWith("-") ? "" : "-");
                ret[i].hyphen.first = scaled(shapeWord(brk.value().first + addOnFirst));
                markCharacters(ret[i].hyphen.first, brk.value().first + addOnFirst);
                ret[i].hyphen.second = scaled(shapeWord(brk.value().second));
                markCharacters(ret[i].hyphen.second, brk.value().second);
            }
        }
        Gret << ret;
    }

    delete otf;
    otf = nullptr;
    releaseFace();
    return Gret;
}

GlyphList FontItem::glyphs(QString spec, double fsize, OTFSet set)
{
    return shapeWords(spec, fsize, [&](const QString &word) {
        return otf->procstring(word, set);
    });
}

GlyphList FontItem::glyphsShaped(const QString &spec, double fsize)
{
    // no script named: the engine takes it from each word
    return shapeWords(spec, fsize, [&](const QString &word) {
        return otf->shape(word, QString(), true);
    });
}

GlyphList FontItem::glyphs(QString spec, double fsize, QString script)
{
    FMHyphenator *hyph = typotek::getInstance()->getHyphenator();
    GlyphList Gret;
    if (spec.isEmpty() || fsize <= 0.0 || !m_isOpenType) // enough :-)
        return Gret;
    if (!ensureFace())
        return Gret;

    otf = new FMOtf(m_face, 0x10000);
    if (!otf) {
        releaseFace();
        return Gret;
    }
    FMShaperFactory *shaperfactory = nullptr;
    shaperfactory = new FMShaperFactory(otf, script, FMShaperFactory::FONTMATRIX);

    /// HYPHENATION

    QStringList stl(spec.split(' ', Qt::SkipEmptyParts));

    double scalefactor = fsize / unitsPerEm();
    QGraphicsPathItem *glyph = itemFromChar(QChar(' ').unicode(), fsize);
    RenderedGlyph wSpace(glyph->data(GLYPH_DATA_GLYPH).toInt(), 0, glyph->data(GLYPH_DATA_HADVANCE).toDouble() * scalefactor, 0, 0, 0, ' ', false);
    wSpace.lChar = 0x20;
    delete glyph;

    QMap<QString, GlyphList> cache;
    for (QStringList::const_iterator sIt(stl.constBegin()); sIt != stl.constEnd(); ++sIt) {
        if (cache.contains(*sIt)) {
            GlyphList ret(cache.value(*sIt));
            Gret << wSpace << ret;
            continue;
        }
        if (sIt != stl.constBegin()) {
            Gret << wSpace;
        }
        HyphList hl;
        if (hyph) {
            hl = hyph->hyphenate(*sIt);
        }

        GlyphList ret(shaperfactory->doShape(*sIt));
        markCharacters(ret, *sIt);

        for (int i(0); i < ret.size(); ++i) {
            ret[i].xadvance *= scalefactor;
            ret[i].yadvance *= scalefactor;
            ret[i].xoffset *= scalefactor;
            ret[i].yoffset *= scalefactor;

            if (hl.contains(ret[i].log)) {
                ret[i].isBreak = true;
                QString addOnFirst;
                QString addOnSecond;
                addOnFirst = hl[i].first.endsWith("-") ? "" : "-";

                ret[i].hyphen.first = shaperfactory->doShape(hl[ret[i].log].first + addOnFirst);
                markCharacters(ret[i].hyphen.first, hl[ret[i].log].first + addOnFirst);
                for (int f(0); f < ret[i].hyphen.first.size(); ++f) {
                    ret[i].hyphen.first[f].xadvance *= scalefactor;
                    ret[i].hyphen.first[f].yadvance *= scalefactor;
                    ret[i].hyphen.first[f].xoffset *= scalefactor;
                    ret[i].hyphen.first[f].yoffset *= scalefactor;
                }

                ret[i].hyphen.second = shaperfactory->doShape(hl[ret[i].log].second + addOnSecond);
                markCharacters(ret[i].hyphen.second, hl[ret[i].log].second + addOnSecond);
                for (int f(0); f < ret[i].hyphen.second.size(); ++f) {
                    ret[i].hyphen.second[f].xadvance *= scalefactor;
                    ret[i].hyphen.second[f].yadvance *= scalefactor;
                    ret[i].hyphen.second[f].xoffset *= scalefactor;
                    ret[i].hyphen.second[f].yoffset *= scalefactor;
                }
            }
        }

        Gret << ret;
        cache[*sIt] = ret;
    }

    /// END OF HYPHENATION

    delete shaperfactory;
    delete otf;
    otf = nullptr;
    releaseFace();
    // 	for (const auto& g : Gret)
    // 	{
    // 		g.dump();
    // 	}
    return Gret;
}

double FontItem::getUnitPerEm()
{
    if (unitPerEm)
        return unitPerEm;
    if (ensureFace()) {
        releaseFace();
    }
    return unitPerEm;
}

unsigned int FontItem::getFTHintMode() const
{
    return m_FTHintMode;
}

void FontItem::setFTHintMode(unsigned int theValue)
{
    m_FTHintMode = theValue;
}

// here for migration purpose
void FontItem::dumpIntoDB()
{
    if (!m_valid)
        return;

    FMFontDb *db(FMFontDb::DB());
    db->initRecord(m_path);

    QString panString(panose());

    QList<FMFontDb::Field> fl;
    QVariantList vl;

    fl << FMFontDb::Family << FMFontDb::Variant << FMFontDb::Name << FMFontDb::Type << FMFontDb::Panose;

    vl << m_family << m_variant << m_name << m_type << panString;

    db->setValues(m_path, fl, vl);
    db->setInfoMap(m_path, moreInfo());
}

QStringList FontItem::charmaps()
{
    QStringList ret;
    for (const auto &e : std::as_const(m_charsets)) {
        ret << FontStrings::Encoding(e);
    }
    return ret;
}

QString FontItem::renderSVG(const QString &s, const double &size)
{
    if (!ensureFace())
        return QString();

    QString ret;
    QString svg;
    QTransform tf;
    double pifs(size);
    double scaleFactor(pifs / unitPerEm);
    double vertOffset(pifs);
    double horOffset(0);
    tf.translate(horOffset, vertOffset);
    // read now, itemFromChar() takes and releases the face on its own
    const double svgHeight(m_face->height * scaleFactor);

    for (const auto &c : s) {
        {
            QGraphicsPathItem *gpi(itemFromChar(c.unicode(), pifs));
            if (gpi) {
                GlyphToSVGHelper gtsh(gpi->path(), tf);
                svg += gtsh.getSVGPath();
                horOffset += gpi->data(GLYPH_DATA_HADVANCE).toDouble() * scaleFactor;
                tf.translate(gpi->data(GLYPH_DATA_HADVANCE).toDouble() * scaleFactor, 0);
                delete gpi;
            }
        }
    }
    QString openElem(QString("<svg width=\"%1\" height=\"%2\"  xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">").arg(horOffset).arg(svgHeight));
    ret += openElem;
    ret += svg;
    ret += "</svg>";

    releaseFace();
    return ret;
}

unsigned short FontItem::getNamedChar(const QString &name)
{
    if (!ensureFace())
        return 0;
    unsigned short ret(0);

    if (FT_HAS_GLYPH_NAMES(m_face)) {
        int bLen(256);
        auto buffer(new char[bLen]);
        FT_UInt index(1);
        FT_UInt cc = FT_Get_First_Char(m_face, &index);
        QString cname;
        while (index) {
            FT_Get_Glyph_Name(m_face, index, buffer, bLen);
            cname = QString::fromLatin1(buffer);
            // 			qDebug()<<"NC"<<cname<<cc<<index;
            if (0 == name.compare(cname)) {
                ret = cc;
                break;
            }
            cc = FT_Get_Next_Char(m_face, cc, &index);
        }
        delete[] buffer;
    }

    releaseFace();
    return ret;
}

QStringList FontItem::getNames()
{
    QStringList ret;
    if (!ensureFace())
        return ret;
    if (FT_HAS_GLYPH_NAMES(m_face)) {
        int bLen(256);
        auto buffer(new char[bLen]);
        FT_UInt index(1);
        FT_UInt cc = FT_Get_First_Char(m_face, &index);
        while (index) {
            FT_Get_Glyph_Name(m_face, index, buffer, bLen);
            ret << QString::fromLatin1(buffer);
            cc = FT_Get_Next_Char(m_face, cc, &index);
        }
        delete[] buffer;
    }
    releaseFace();
    return ret;
}

void FontItem::exploreKernFeature()
{
    if (!ensureFace())
        return;

    FMKernFeature kf(m_face);

    releaseFace();
}

bool FontItem::getUnicodeBuiltIn() const
{
    return m_unicodeBuiltIn;
}

FT_Encoding FontItem::getCurrentEncoding() const
{
    return m_currentEncoding;
}

double FontItem::getUnitPerEm() const
{
    return unitPerEm;
}

QList<FT_Encoding> FontItem::getCharsets() const
{
    return m_charsets;
}

#include "moc_fontitem.cpp"
