/*
    SPDX-FileCopyrightText: 2007 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FONTBOOK_H
#define FONTBOOK_H

#include <QColor>
#include <QDomDocument>
#include <QHash>
#include <QList>
#include <QMap>
#include <QObject>
#include <QPageSize>
#include <QPainter>
#include <QPrinter>
#include <QRectF>

class ProgressBarDuo;
/**
    @author Pierre Marchand <pierremarc@oep-h.com>

    It’s time to write something more "definitive" :)
*/

struct TextElementStyle {
    QString name;
    QString font; // _FONTMATRIX_ is reserved
    double fontsize = 0.0;
    double lineheight = 0.0;
    QColor color;
    double margin_top = 0.0, margin_left = 0.0, margin_bottom = 0.0, margin_right = 0.0;
    TextElementStyle() = default;
    TextElementStyle(QString n, QString f, double fs, double lh, double mt, double ml, double mb, double mr, QColor co)
        : name(n)
        , font(f)
        , fontsize(fs)
        , lineheight(lh)
        , color(co)
        , margin_top(mt)
        , margin_left(ml)
        , margin_bottom(mb)
        , margin_right(mr)
    {
    }
};

struct TextElement {
    QString e;
    bool valid;
    /**
    Has to be set if "e" must be substituted with a contextual info
    available infos depend of level and are :
    - Family
    - SubFamily
    - Encoding
    - PageNumber
    - ...
    At some point, I’ll provide a reference.
    */
    // 	bool internal; OBSOLETE - substitution will be regexpizated ##KEYWORD##
    TextElement()
        : valid(false)
    {
    }
    explicit TextElement(QString elem)
        : e(elem)
        , valid(true)
    {
    }
};

/**
GraphicElement will be made by
<graphic xpos="0.0" ypos="0.0"><svg></svg></graphic>
where svg comes from regular svg file.
*/
struct GraphicElement {
    QString name;
    double x, y;
    QString svg; // OBSOLETE :)
    bool valid;
    GraphicElement(QString aName, QString svgstring, double xpos, double ypos)
        : name(aName)
        , x(xpos)
        , y(ypos)
        , svg(svgstring.startsWith("<?xml") ? svgstring : ("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n" + svgstring))
        , valid(true)
    {
    }
    GraphicElement()
        : name("noname")
        , x(0.0)
        , y(0.0)
        , svg("")
        , valid(false)
    {
    }
};

struct FontBookContext {
    TextElement textElement;
    TextElementStyle textStyle;
    GraphicElement graphic;
    /*
    enum FBCLevel{PAGE, FAMILY, SUBFAMILY};

    FBCLevel level;*/
};

class FontItem;

/// a style the book shows: a font, or one named instance of a variable font
struct BookStyle {
    FontItem *font = nullptr;
    QString name; ///< the style name, or the name of the instance
    QList<double> coords; ///< the coordinates of the instance; empty for the font as it is
};

class FontBook : public QObject
{
public:
    enum Style {
        Full,
        OneLiner
    };
    FontBook();

    ~FontBook() override;
    void doBook(Style s);

private:
    void doFullBook();
    void doFullBookCover();
    // return true if uses only its page, false if it spreads over the 2 pages
    bool doFullBookPageLeft(const QString &family);
    void doFullBookPageRight(const QString &family);
    void doOneLinerBook();

    /// the styles of the fonts, a variable font contributing every named instance it has
    QList<BookStyle> styles(const QList<FontItem *> &fonts);
    /// puts a variable font at the coordinates of the style before it renders
    static void showStyle(const BookStyle &style);
    /// back to the coordinates the fonts had before the book
    void restoreStyles();
    QHash<FontItem *, QList<double>> savedCoords;

    QPrinter *printer = nullptr;
    QPainter *painter = nullptr;
    QRectF printerRect;
    QStringList stringList;
    ProgressBarDuo *progress = nullptr;

    QString outputFilePath;
    QMap<QString, QPageSize::PageSizeId> mapPSize;
    void doBookFromTemplate(const QDomDocument &aTemplate);
};

#endif
