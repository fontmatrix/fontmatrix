/*
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "fmsvgglyphs.h"
#include "fontmatrix_debug.h"

#include <QImage>
#include <QPainter>
#include <QRegularExpression>
#include <QSvgRenderer>
#include <QXmlStreamReader>

#include FT_COLOR_H
#include FT_TRUETYPE_TABLES_H

#include <zlib.h>

namespace
{
quint16 u16(const QByteArray &b, quint32 at)
{
    if (at + 2 > quint32(b.size()))
        return 0;
    return quint16((uchar(b.at(at)) << 8) | uchar(b.at(at + 1)));
}

quint32 u32(const QByteArray &b, quint32 at)
{
    if (at + 4 > quint32(b.size()))
        return 0;
    return (quint32(uchar(b.at(at))) << 24) | (quint32(uchar(b.at(at + 1))) << 16) | (quint32(uchar(b.at(at + 2))) << 8) | quint32(uchar(b.at(at + 3)));
}

/// a gzip member inflated; empty when it is not one or is damaged. A glyph is a few
/// kilobytes: past 32 MiB it is not a glyph any more
QByteArray gunzip(const QByteArray &in)
{
    constexpr qsizetype limit = 32 * 1024 * 1024;
    z_stream stream{};
    stream.next_in = reinterpret_cast<Bytef *>(const_cast<char *>(in.constData()));
    stream.avail_in = uInt(in.size());
    if (inflateInit2(&stream, 16 + MAX_WBITS) != Z_OK)
        return QByteArray();
    QByteArray out;
    char buffer[16384];
    int status(Z_OK);
    do {
        stream.next_out = reinterpret_cast<Bytef *>(buffer);
        stream.avail_out = sizeof buffer;
        status = inflate(&stream, Z_NO_FLUSH);
        if (status != Z_OK && status != Z_STREAM_END) {
            inflateEnd(&stream);
            return QByteArray();
        }
        out.append(buffer, qsizetype(sizeof buffer - stream.avail_out));
    } while (status != Z_STREAM_END && out.size() < limit);
    inflateEnd(&stream);
    return status == Z_STREAM_END ? out : QByteArray();
}

/// the smallest rectangle of the image that holds every pixel that is not transparent
QRect inkOf(const QImage &img)
{
    int left(img.width()), right(-1), top(img.height()), bottom(-1);
    for (int y(0); y < img.height(); ++y) {
        const QRgb *line(reinterpret_cast<const QRgb *>(img.constScanLine(y)));
        for (int x(0); x < img.width(); ++x) {
            if (qAlpha(line[x])) {
                left = std::min(left, x);
                right = std::max(right, x);
                top = std::min(top, y);
                bottom = std::max(bottom, y);
            }
        }
    }
    return right < 0 ? QRect() : QRect(QPoint(left, top), QPoint(right, bottom));
}
}

FMSvgGlyphs *FMSvgGlyphs::create(FT_Face face)
{
    // the table itself, not FT_HAS_SVG(): that one only exists since FreeType 2.12
    if (!face)
        return nullptr;
    const FT_ULong tag(FT_MAKE_TAG('S', 'V', 'G', ' '));
    FT_ULong length(0);
    if (FT_Load_Sfnt_Table(face, tag, 0, nullptr, &length) || !length)
        return nullptr;
    auto svg = new FMSvgGlyphs;
    svg->m_table.resize(qsizetype(length));
    if (FT_Load_Sfnt_Table(face, tag, 0, reinterpret_cast<FT_Byte *>(svg->m_table.data()), &length)) {
        delete svg;
        return nullptr;
    }
    svg->m_unitsPerEm = face->units_per_EM ? face->units_per_EM : 1000;

    // header: version, offset to the document list, reserved
    svg->m_listOffset = u32(svg->m_table, 2);
    const quint16 count(u16(svg->m_table, svg->m_listOffset));
    for (quint16 i(0); i < count; ++i) {
        const quint32 at(svg->m_listOffset + 2 + quint32(i) * 12);
        Record r;
        r.first = u16(svg->m_table, at);
        r.last = u16(svg->m_table, at + 2);
        r.offset = u32(svg->m_table, at + 4);
        r.length = u32(svg->m_table, at + 8);
        if (r.first <= r.last && r.length && svg->m_listOffset + r.offset + r.length <= quint32(svg->m_table.size()))
            svg->m_records << r;
    }
    if (svg->m_records.isEmpty()) {
        delete svg;
        return nullptr;
    }

    // the first palette, for the colour variables of the documents
    FT_Palette_Data paletteData;
    FT_Color *colors = nullptr;
    if (!FT_Palette_Data_Get(face, &paletteData) && paletteData.num_palettes && !FT_Palette_Select(face, 0, &colors) && colors) {
        for (FT_UShort c(0); c < paletteData.num_palette_entries; ++c)
            svg->m_palette << QColor(colors[c].red, colors[c].green, colors[c].blue).name();
    }
    qCDebug(FONTMATRIX_LOG) << "SVG table:" << svg->m_records.size() << "document records," << svg->m_palette.size() << "palette entries";
    return svg;
}

bool FMSvgGlyphs::hasGlyph(unsigned int glyph) const
{
    // the records are sorted by glyph
    auto it = std::lower_bound(m_records.cbegin(), m_records.cend(), glyph, [](const Record &r, unsigned int g) {
        return r.last < g;
    });
    return it != m_records.cend() && it->first <= glyph;
}

QByteArray FMSvgGlyphs::document(const Record &record)
{
    auto cached = m_documents.constFind(record.offset);
    if (cached != m_documents.constEnd())
        return cached.value();
    QByteArray doc(m_table.mid(qsizetype(m_listOffset + record.offset), qsizetype(record.length)));
    if (doc.startsWith("\x1f\x8b"))
        doc = gunzip(doc);
    m_documents.insert(record.offset, doc);
    return doc;
}

QByteArray FMSvgGlyphs::glyphDocument(unsigned int glyph)
{
    auto cached = m_glyphDocuments.constFind(glyph);
    if (cached != m_glyphDocuments.constEnd())
        return cached.value();

    auto it = std::lower_bound(m_records.cbegin(), m_records.cend(), glyph, [](const Record &r, unsigned int g) {
        return r.last < g;
    });
    if (it == m_records.cend() || it->first > glyph)
        return QByteArray();
    QString text(QString::fromUtf8(document(*it)));
    if (text.isEmpty())
        return QByteArray();

    // colour variables: the first palette of the font, else the fallback the font gives
    static const QRegularExpression var(QStringLiteral("var\\(\\s*--color(\\d+)\\s*(?:,\\s*([^)]*))?\\)"));
    QString resolved;
    qsizetype last(0);
    for (auto match = var.globalMatch(text); match.hasNext();) {
        const QRegularExpressionMatch m(match.next());
        const int entry(m.captured(1).toInt());
        const QString fallback(m.captured(2).trimmed());
        resolved += QStringView(text).mid(last, m.capturedStart() - last);
        resolved += entry < m_palette.size() ? m_palette.at(entry) : (fallback.isEmpty() ? QStringLiteral("black") : fallback);
        last = m.capturedEnd();
    }
    resolved += QStringView(text).mid(last);
    text = resolved;

    // the root element: its viewBox, and whether it is the glyph itself
    const QString id(QStringLiteral("glyph%1").arg(glyph));
    QXmlStreamReader reader(text);
    while (!reader.atEnd() && !reader.isStartElement())
        reader.readNext();
    if (!reader.isStartElement() || reader.name() != QLatin1String("svg"))
        return QByteArray();
    const QString viewBox(reader.attributes().value(QLatin1String("viewBox")).toString());
    const bool rootIsGlyph(reader.attributes().value(QLatin1String("id")) == id);
    const qsizetype innerStart(reader.characterOffset());
    const qsizetype innerEnd(text.lastIndexOf(QLatin1String("</svg")));

    QString doc;
    if (rootIsGlyph || innerEnd <= innerStart) {
        doc = text;
    } else {
        // "as though the element and its content were specified in a <defs> and then referenced
        // as the graphic content of the document" (the OpenType specification, 'SVG ' table)
        doc = QStringLiteral(
                  "<svg version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\"%1><defs>%2</defs>"
                  "<use xlink:href=\"#%3\"/></svg>")
                  .arg(viewBox.isEmpty() ? QString() : QStringLiteral(" viewBox=\"%1\"").arg(viewBox), text.mid(innerStart, innerEnd - innerStart), id);
    }
    const QByteArray bytes(doc.toUtf8());
    m_glyphDocuments.insert(glyph, bytes);
    return bytes;
}

bool FMSvgGlyphs::paint(unsigned int glyph, double ppem, QImage &img, double &left, double &top)
{
    if (!hasGlyph(glyph) || ppem <= 0.0)
        return false;
    const QByteArray doc(glyphDocument(glyph));
    if (doc.isEmpty())
        return false;
    QSvgRenderer renderer(doc);
    if (!renderer.isValid()) {
        qCDebug(FONTMATRIX_LOG) << "SVG glyph" << glyph << "cannot be read";
        return false;
    }
    // Without a viewBox, one unit is one font unit and the viewport is the em square;
    // with one, the viewBox is mapped onto that square. Its top left corner is the
    // glyph origin: content above the baseline has a negative y. Nothing is clipped.
    const QRectF box(renderer.viewBoxF().isEmpty() || !doc.contains("viewBox") ? QRectF(0, 0, m_unitsPerEm, m_unitsPerEm) : renderer.viewBoxF());
    renderer.setViewBox(box);
    renderer.setAspectRatioMode(Qt::IgnoreAspectRatio);

    // room enough for any glyph around its origin: half an em left of it, one and a
    // half above the baseline, one below; the ink is cut out afterwards
    const int side(qCeil(ppem * 2.5));
    const QPointF origin(ppem * 0.5, ppem * 1.5);
    QImage canvas(side, side, QImage::Format_ARGB32_Premultiplied);
    canvas.fill(Qt::transparent);
    {
        QPainter p(&canvas);
        p.setRenderHint(QPainter::Antialiasing);
        p.setRenderHint(QPainter::SmoothPixmapTransform);
        renderer.render(&p, QRectF(origin, QSizeF(ppem, ppem)));
    }
    const QRect ink(inkOf(canvas));
    if (ink.isEmpty())
        return false;
    img = canvas.copy(ink);
    left = ink.left() - origin.x();
    top = origin.y() - ink.top();
    return true;
}
