/*
    SPDX-FileCopyrightText: 2009 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "fminfodisplay.h"

#include "fmencdata.h"
#include "fmfontdb.h"
#include "fmfontstrings.h"
#include "fmlicense.h"
#include "fontitem.h"
#include "glyphtosvghelper.h"
#include "typotek.h"

#include <KLocalizedString>
#include <QMap>
#include <QObject>
#include <QRegularExpression>
#include <QStringList>

#include <QBuffer>
#include <QByteArray>
#include <QFile>
#include <QImage>
#include <QPainter>
#include <QSvgRenderer>

FMInfoDisplay::FMInfoDisplay(FontItem *font)
{
    /**
    Selectors are :
#headline
#file
    .infoblock
    .infoname
    .langmatch
    .langundefined
    .langnomatch
    .encodingcurrent
    .encoding
     */
    html = "<html>\n<head>\n";
    html += "<title>" + xhtmlifies(font->fancyName()) + "</title>\n";
    html += "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />\n";
    html += "</head>\n<body>\n";
    html += writeSVGPreview(font);
    html += "<div id=\"file\">" + xhtmlifies(font->path()) + "</div>\n";
    // 	ret += "<div id=\"search\"><a href=\"http://www.myfonts.com/search?search[text]="+ m_family +"\">On myfonts</a>";
    html += "<div id=\"general\">\n";
    html += writeOrderedInfo(font);
    html += writeFsType(font);
    html += "</div>\n"; // general
    html += writeVariations(font);
    html += writePanose(font);
    html += writeLangOS2(font);
    html += "</body>\n </html>\n";

#ifdef BUILD_TYPE_DEBUG
    QFile df("fontmatrix.xhtml");
    if (df.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        df.write(html.toUtf8());
    }
#endif
}

FMInfoDisplay::~FMInfoDisplay() = default;

QString FMInfoDisplay::getHtml()
{
    return html;
}

QString FMInfoDisplay::writeFsType(FontItem *font)
{
    FontItem::FsType OSFsType(font->getFsType());
    QString embedFlags = "<div id=\"fstype\">";
    // 0 - 3 are exclusive
    if (OSFsType == FontItem::NOT_RESTRICTED)
        embedFlags += "<div><div class=\"fsname\">" + FontStrings::FsType(FontItem::NOT_RESTRICTED, true) + "</div><div class=\"fsdesc\">"
            + FontStrings::FsType(FontItem::NOT_RESTRICTED, false) + "</div></div>\n";
    else if (OSFsType & FontItem::RESTRICTED)
        embedFlags += "<div><div class=\"fsname\">" + FontStrings::FsType(FontItem::RESTRICTED, true) + "</div><div class=\"fsdesc\">"
            + FontStrings::FsType(FontItem::RESTRICTED, false) + "</div></div>\n";
    else if (OSFsType & FontItem::PREVIEW_PRINT)
        embedFlags += "<div><div class=\"fsname\">" + FontStrings::FsType(FontItem::PREVIEW_PRINT, true) + "</div><div class=\"fsdesc\">"
            + FontStrings::FsType(FontItem::PREVIEW_PRINT, false) + "</div></div>\n";
    else if (OSFsType & FontItem::EDIT_EMBED)
        embedFlags += "<div><div class=\"fsname\">" + FontStrings::FsType(FontItem::EDIT_EMBED, true) + "</div><div class=\"fsdesc\">"
            + FontStrings::FsType(FontItem::EDIT_EMBED, false) + "</div></div>\n";
    if (OSFsType & FontItem::NOSUBSET)
        embedFlags += "<div><div class=\"fsname\">" + FontStrings::FsType(FontItem::NOSUBSET, true) + "</div><div class=\"fsdesc\">"
            + FontStrings::FsType(FontItem::NOSUBSET, false) + "</div></div>\n";
    if (OSFsType & FontItem::BITMAP_ONLY)
        embedFlags += "<div><div class=\"fsname\">" + FontStrings::FsType(FontItem::BITMAP_ONLY, true) + "</div><div class=\"fsdesc\">"
            + FontStrings::FsType(FontItem::BITMAP_ONLY, false) + "</div></div>\n";
    embedFlags += "</div>";

    return embedFlags;
}

QString FMInfoDisplay::writeVariations(FontItem *font)
{
    if (!font->isVariable())
        return QString();
    QString ret;
    ret += "<div id=\"variations\">\n";
    ret += "\t<div class=\"langblockname\">" + i18nc("@title:group the axes of a variable font", "Variation Axes") + "</div>\n";
    ret += "\t<ul>\n";
    const QList<FontVariationAxis> axes(font->variationAxes());
    for (const FontVariationAxis &axis : axes) {
        ret += QString("\t\t<li>%1</li>\n")
                   .arg(xhtmlifies(i18nc("@item an axis of a variable font: %1 name, %2 tag, %3 minimum, %4 maximum, %5 default",
                                         "%1 (%2): from %3 to %4, %5 by default",
                                         axis.name,
                                         axis.tag,
                                         QString::number(axis.minimum),
                                         QString::number(axis.maximum),
                                         QString::number(axis.def))));
    }
    ret += "\t</ul>\n";
    const QList<FontNamedInstance> instances(font->namedInstances());
    if (!instances.isEmpty()) {
        ret += "\t<div class=\"langblockname\">" + i18nc("@title:group the named instances of a variable font", "Named Instances") + "</div>\n";
        ret += "\t<ul>\n";
        for (const FontNamedInstance &instance : instances)
            ret += QString("\t\t<li>%1</li>\n").arg(xhtmlifies(instance.name));
        ret += "\t</ul>\n";
    }
    ret += "</div>\n";
    return ret;
}

QString FMInfoDisplay::writeLangOS2(FontItem *font)
{
    QString ret;
    QStringList llist(font->supportedLangDeclaration());
    if (llist.count() > 0) {
        ret += "<div id=\"langblock\">\n";
        ret += "\t<div class=\"langblockname\">" + i18nc("@title:group", "Unicode Ranges") + "</div>\n";
        ret += "\t<ul>\n";
        for (const auto &ln : std::as_const(llist)) {
            ret += QString("\t\t<li>%1</li>\n").arg(ln);
        }
        ret += "\t</ul>\n";
        ret += "</div>\n";
    }
    return ret;
}

QString FMInfoDisplay::writeSVGPreview(FontItem *font)
{
    // Rasterise the assembled SVG once: QTextBrowser can resolve
    // <img src=data:...> but not inline <svg> elements.
    QString svgPaths;
    QTransform tf;
    double pifs(typotek::getInstance()->getPreviewInfoFontSize());
    double scaleFactor(pifs / font->getUnitPerEm());
    double maxHeight(0);
    double vertOffset(pifs);
    double horOffset(0);
    tf.translate(horOffset, vertOffset);

    for (const auto fancyNameList = font->fancyName(); const auto &c : fancyNameList) {
        QGraphicsPathItem *gpi(font->itemFromChar(c.unicode(), pifs));
        if (gpi) {
            GlyphToSVGHelper gtsh(gpi->path(), tf);
            svgPaths += gtsh.getSVGPath() + "\n";
            horOffset += gpi->data(GLYPH_DATA_HADVANCE).toDouble() * scaleFactor;
            /* cast gtsh.height from qreal (float on ARM) to double so qMax can be done */
            maxHeight = qMax((double)gtsh.getRect().height(), maxHeight);
            tf.translate(gpi->data(GLYPH_DATA_HADVANCE).toDouble() * scaleFactor, 0);
            delete gpi;
        }
    }

    const int w = qRound(horOffset);
    const int h = qRound(maxHeight * 1.6);
    if (svgPaths.isEmpty() || w <= 0 || h <= 0)
        return QString();

    const QString svgDoc =
        QStringLiteral("<svg width=\"%1px\" height=\"%2px\" xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n%3</svg>").arg(w).arg(h).arg(svgPaths);

    QSvgRenderer renderer(svgDoc.toUtf8());
    QImage image(w, h, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    {
        QPainter painter(&image);
        renderer.render(&painter);
    }

    QByteArray pngBytes;
    QBuffer buffer(&pngBytes);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");

    return QStringLiteral("<div id=\"previewblock\"><img src=\"data:image/png;base64,%1\"/></div>\n").arg(QString::fromLatin1(pngBytes.toBase64()));
}

QString FMInfoDisplay::writeOrderedInfo(FontItem *font)
{
    QString ret;
    QMap<int, QStringList> orderedInfo;
    QString modelItem("<div class=\"infoblock\"><div class=\"infoname\"> %1 </div><div class=\"langundefined\"> %2 </div></div>\n");
    QString fontType(font->type());
    if (fontType == QString("CFF"))
        fontType = QString("OpenType");

    // one arg() call each: a path or a name may hold a "%2" of its own
    ret += modelItem.arg(i18nc("@label", "File"), font->path().replace("/", "/&shy;"));
    ret += modelItem.arg(i18nc("@label", "Glyphs count"), QString::number(font->glyphsCount()));
    ret += modelItem.arg(i18nc("@label", "Font Type"), fontType);
    if (font->hasColor()) {
        // CBDT and sbix hold bitmaps, COLR layers of outlines with the colours of CPAL, SVG documents
        ret += modelItem.arg(i18nc("@label the tables a colour font has its colours in", "Color"), font->colorTables().join(QLatin1String(", ")));
    }
    // what the licence fields of the name table below say, in one word
    ret += modelItem.arg(i18nc("@label the licence of a font, recognised from its wording", "License"), FMLicense::name(FMLicense::of(font)));

    QStringList cmapStrings;
    for (const auto charsets = font->getCharsets(); const auto &c : charsets) {
        QString encString(FontStrings::Encoding(c));
        if ((c == FT_ENCODING_UNICODE) && (!font->getUnicodeBuiltIn()))
            encString += "*";
        if (c == font->getCurrentEncoding())
            cmapStrings << "<span class=\"encodingcurrent\">" + encString + "</span>\n";
        else
            cmapStrings << "<span class=\"encoding\">" + encString + "</span>\n";
    }
    ret += "<div class=\"infoblock\"><div class=\"infoname\">" + i18nc("@title:group", "Charmaps List") + "</div><div class=\"langundefined\">"
        + font->charmaps().join(", ") + "</div></div>\n";

    // 	if ( !moreInfo.isEmpty() ) // moreInfo.isNotEmpty
    {
        QString sysLang = QLocale::languageToString(QLocale::system().language()).toUpper();

        // We must iter once to find localized strings and ensure default ones are _not_ shown in these cases
        QList<int> localizedKeys;
        FontInfoMap moreInfo(FMFontDb::DB()->getInfoMap(font->path()));
        for (QMap<int, QMap<int, QString>>::const_iterator lit = moreInfo.constBegin(); lit != moreInfo.constEnd(); ++lit) {
            for (QMap<int, QString>::const_iterator mit = lit.value().begin(); mit != lit.value().end(); ++mit) {
                if (FMEncData::LangIdMap()[lit.key()].contains(sysLang)) {
                    localizedKeys << mit.key();
                }
            }
        }

        //		QString styleLangMatch("\"langundefined\"");
        for (QMap<int, QMap<int, QString>>::const_iterator lit = moreInfo.constBegin(); lit != moreInfo.constEnd(); ++lit) {
            //			if ( FMEncData::LangIdMap()[ lit.key() ].contains ( sysLang ) ) // lang match
            //			{
            //				styleLangMatch = "\"langmatch\"";
            //			}
            //			else if ( FMEncData::LangIdMap()[ lit.key() ] == "DEFAULT" ) // lang does not match but it’s international name
            //			{
            //				styleLangMatch = "\"langundefined\"";
            //			}
            //			else // lang does not match at all
            //			{
            //				styleLangMatch = "\"langnomatch\"";
            //			}
            for (QMap<int, QString>::const_iterator mit = lit.value().begin(); mit != lit.value().end(); ++mit) {
                //				if ( FMEncData::LangIdMap()[ lit.key() ].contains ( sysLang ) )
                {
                    QString name_value(url2href(xhtmlifies(mit.value()))); // compact coding :)
                    name_value.replace("\n", "<br/>");
                    QString dcname(name_value);
                    //					if ( !orderedInfo[ mit.key() ].contains ( dcname ) )
                    if (!orderedInfo.contains(mit.key()))
                        orderedInfo[mit.key()] << dcname;
                }
                //				else if ( FMEncData::LangIdMap()[ lit.key() ] == "DEFAULT" && !localizedKeys.contains ( mit.key() ) )
                //				{
                //					QString name_value(url2href(xhtmlifies(mit.value())));
                //					name_value.replace ( "\n","<br/>" );
                //					QString dcname ( "<div class="+ styleLangMatch +">" +  name_value +"</div>\n" );
                //					if ( !orderedInfo[ mit.key() ].contains ( dcname ) )
                //						orderedInfo[ mit.key() ] << dcname;
                //				}
            }
        }
    }

    /// Times to manually order presentation!

    QList<FMFontDb::InfoItem> order;
    order << FMFontDb::FontFamily << FMFontDb::FontSubfamily << FMFontDb::FullFontName << FMFontDb::PostscriptName << FMFontDb::Designer
          << FMFontDb::Description << FMFontDb::LicenseDescription << FMFontDb::Copyright << FMFontDb::PostScriptCIDName << FMFontDb::SampleText
          << FMFontDb::VersionString << FMFontDb::Trademark << FMFontDb::ManufacturerName << FMFontDb::URLVendor << FMFontDb::URLDesigner
          << FMFontDb::LicenseInfoURL << FMFontDb::PreferredFamily << FMFontDb::PreferredSubfamily << FMFontDb::CompatibleMacintosh
          << FMFontDb::UniqueFontIdentifier;

    QMap<FMFontDb::InfoItem, QString> tNames(FontStrings::Names());
    for (const auto &key : std::as_const(order)) {
        if (orderedInfo.contains(key))
            ret += modelItem.arg(tNames.value(key), orderedInfo[key].join(" "));
    }

    return ret;
}

QString FMInfoDisplay::writePanose(FontItem *font)
{
    QString panBlockOut;
    QString panoseLabel("<div id=\"panoselabel\">Panose</div>");
    QString pN(FMFontDb::DB()->getValue(font->path(), FMFontDb::Panose, false).toString());
    if (!pN.isEmpty()) {
        QStringList pl(pN.split(":"));
        if (pl.count() == 10) {
            const auto panoseKeys(FontStrings::Panose().keys());
            for (int i(0); i < panoseKeys.count(); ++i) {
                FontStrings::PanoseKey k(panoseKeys.at(i));
                int pValue(pl[i].toInt());
                panBlockOut += "<div class=\"panose_name\">" + FontStrings::PanoseKeyName(k) + "</div>\n";
                panBlockOut += "<div class=\"panose_desc\">" + FontStrings::Panose().value(k).value(pValue) /* + " - "+ pl[i]*/ + "</div>\n";
            }
        }
    }

    return "<div id=\"panose_block\">" + panoseLabel + panBlockOut + "</div>\n";
}

/**
 * Make HTTP links out of url in a text string
 */
QString FMInfoDisplay::url2href(QString value)
{
    static const QString punctuationAfter = "\\.\\,;:!?)\"'";
    static const QRegularExpression rxWww("([^/])(www\\.[\\w\\d])");
    static const QRegularExpression rx("(http[s]?://\\S+?)([" + punctuationAfter + "](?:\\s|$))"); // non-greedy
    static const QRegularExpression rxLink("(http[s]?://\\S+)[\\.]?");
    static const QRegularExpression rxSpace("(</a>)\\s([" + punctuationAfter + "])");
    value.replace(rxWww, "\\1http://\\2"); // add an http to www. without it
    value.replace(rx, "\\1 \\2"); // add a space before  punctuation "attached" to url
    value.replace(rx, "\\1 \\2"); // run the prepared regexp twice for ")."
    value.replace(rxLink, "<a href=\"\\1\">\\1</a>"); // Make HTTP links
    value.replace(rxSpace, "\\1\\2"); // remove extra space after </a>
    return value;
} // url2href

QString FMInfoDisplay::xhtmlifies(const QString &value)
{
    QMap<int, QString> pattern;
    pattern[0x22] = "&#34;"; // "
    pattern[0x26] = "&#38;"; // &
    pattern[0x27] = "&#39;"; // '
    pattern[0x3c] = "&#60;"; // <
    pattern[0x3e] = "&#62;"; // >

    QString ret;
    int len(value.length());
    for (int i(0); i < len; ++i) {
        if (pattern.contains(value[i].unicode())) {
            ret += pattern[value[i].unicode()];
        } else
            ret += value[i];
    }
    return ret;
}
