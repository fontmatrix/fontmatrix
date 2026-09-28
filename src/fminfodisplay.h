/*
    SPDX-FileCopyrightText: 2009 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FMINFODISPLAY_H
#define FMINFODISPLAY_H

#include <QString>

/**
A processing class generating XHTML to be displayed in Info tab.

    @author Pierre Marchand <pierremarc@oep-h.com>
*/

class FontItem;
class FMInfoDisplay
{
    FMInfoDisplay() = default;
    QString html;

    QString writeFsType(FontItem *font);
    QString writeSVGPreview(FontItem *font);
    QString writeOrderedInfo(FontItem *font);
    QString writePanose(FontItem *font);
    QString writeLangOS2(FontItem *font);
    QString writeVariations(FontItem *font);

    QString url2href(QString value);
    QString xhtmlifies(const QString &value);

public:
    explicit FMInfoDisplay(FontItem *font);
    ~FMInfoDisplay();

    QString getHtml();
};

#endif
