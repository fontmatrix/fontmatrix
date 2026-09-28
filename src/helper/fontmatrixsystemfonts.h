/*
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FONTMATRIXSYSTEMFONTS_H
#define FONTMATRIXSYSTEMFONTS_H

#include <QByteArray>
#include <QLatin1String>
#include <QStringList>
#include <QXmlStreamWriter>

/**
 * What the application and its root helper agree on for the fonts activated
 * for all users: where they go, and the fontconfig file that hides the system
 * fonts switched off, in the form both write.
 */
namespace FontmatrixSystemFonts
{
/// the directory of the copies, one fontconfig scans on every distribution and Flatpak hands to every sandbox
constexpr QLatin1String Directory("/usr/local/share/fonts/fontmatrix");
/// the rejects file of the whole system
constexpr QLatin1String RejectsFile("/etc/fonts/conf.d/60-fontmatrix-rejects.conf");
/// the helper of KAuth and the prefix of its actions
constexpr QLatin1String HelperId("org.fontmatrix");

/// the fontconfig document that hides the files: a rejectfont pattern on each, matched as it is
inline QByteArray rejectsDocument(const QStringList &files)
{
    QByteArray content;
    QXmlStreamWriter xml(&content);
    xml.setAutoFormatting(true);
    xml.writeStartDocument();
    xml.writeDTD(QStringLiteral("<!DOCTYPE fontconfig SYSTEM \"urn:fontconfig:fonts.dtd\">"));
    xml.writeComment(QStringLiteral(" Written by Fontmatrix: the system fonts switched off in it. Fontmatrix rewrites this file; edit the fonts there. "));
    xml.writeStartElement(QStringLiteral("fontconfig"));
    xml.writeStartElement(QStringLiteral("selectfont"));
    xml.writeStartElement(QStringLiteral("rejectfont"));
    for (const QString &file : files) {
        xml.writeStartElement(QStringLiteral("pattern"));
        xml.writeStartElement(QStringLiteral("patelt"));
        xml.writeAttribute(QStringLiteral("name"), QStringLiteral("file"));
        xml.writeTextElement(QStringLiteral("string"), file);
        xml.writeEndElement(); // patelt
        xml.writeEndElement(); // pattern
    }
    xml.writeEndElement(); // rejectfont
    xml.writeEndElement(); // selectfont
    xml.writeEndElement(); // fontconfig
    xml.writeEndDocument();
    return content;
}
}

#endif
