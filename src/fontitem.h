/*
    SPDX-FileCopyrightText: 2007 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FONTITEM_H
#define FONTITEM_H

#include <functional>

#include <QByteArray>
#include <QFlags>
#include <QGraphicsPathItem>
#include <QIcon>
#include <QMap>
#include <QPainterPath>
#include <QPixmap>
#include <QPointF>
#include <QString>
#include <QStringList>
#include <QUrl>
// #include <QThread>
#include <QGraphicsItem>
#include <QRectF>
#include <QVariant>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "fmsharestruct.h"

struct hb_font_t;
class FMSvgGlyphs;
class QGraphicsPixmapItem;
class QGraphicsScene;
class QGraphicsRectItem;
class QGraphicsTextItem;
struct OTFSet;
class FMOtf;
class QGraphicsView;
class QGraphicsObject;

class QProgressDialog;
class QNetworkReply;
class QFile;

constexpr int PROGRESSION_LTR = 0;
constexpr int PROGRESSION_RTL = 2;
constexpr int PROGRESSION_TTB = 4;
constexpr int PROGRESSION_BTT = 8;

constexpr int GLYPH_DATA_GLYPH = 1;
constexpr int GLYPH_DATA_BITMAPLEFT = 2;
constexpr int GLYPH_DATA_BITMAPTOP = 3;
constexpr int GLYPH_DATA_HADVANCE = 4;
constexpr int GLYPH_DATA_VADVANCE = 5;
constexpr int GLYPH_DATA_HADVANCE_SCALED = 6;
constexpr int GLYPH_DATA_ERROR = 7;
constexpr int GLYPH_DATA_FONTNAME = 100;

/**
    @author Pierre Marchand <pierre@oep-h.com>

    (reminder) glyph data -> 1 = index, 2 = charcode
*/
struct FontLocalInfo {
    QString file;
    QString family;
    QString variant;
    QString type;
    QString panose;
    // 	QMap<int,QMap<QString, QString> > info;
    QStringList tags;
    QPixmap pix;
};

/// an axis of a variable font, in design units
struct FontVariationAxis {
    QString tag; ///< "wght", "wdth", "opsz", "ital", "slnt" or the designer's own
    QString name;
    double minimum = 0.0;
    double def = 0.0;
    double maximum = 0.0;
    bool hidden = false; ///< the designer asks for it to stay out of the interface
};

/// a named instance of a variable font: a style name and one design coordinate per axis
struct FontNamedInstance {
    QString name;
    QList<double> coords;
};

class MetaGlyphItem : public QGraphicsItem
{
    QMap<int, QVariant> m_Data;

public:
    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) override
    {
    }
    [[nodiscard]] QRectF boundingRect() const override
    {
        return QRectF();
    }
    void setMetaData(int key, const QVariant &value)
    {
        m_Data.insert(key, value);
    }
    [[nodiscard]] QVariant metaData(int key) const
    {
        return m_Data.value(key);
    }
};

class FontItem : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString Family READ family)
    Q_PROPERTY(QString Variant READ variant)
    Q_PROPERTY(QString Path READ path)
    Q_PROPERTY(QStringList Tags READ tags WRITE setTags)
    Q_PROPERTY(bool OpenType READ isOpenType)
    Q_PROPERTY(bool Active READ isActivated WRITE setActivated)
    Q_PROPERTY(int GlyphsCount READ glyphsCount)

public:
    enum FsType {
        NOT_RESTRICTED = 0x0000,
        RESTRICTED = 0x0002,
        PREVIEW_PRINT = 0x0004,
        EDIT_EMBED = 0x0008,
        NOSUBSET = 0x0100,
        BITMAP_ONLY = 0x0200
    };

    explicit FontItem(QString path, bool remote = false, bool faststart = false);
    FontItem(QString path, QString family, QString variant, QString type, bool active);
    FontItem *Clone();
    /** Needed when the item has been instantiate with "faststart=true" */
    void updateItem();
    ~FontItem() override;

    static QList<int> legitimateNonPathChars;
    void fillLegitimateSpaces();

private:
    bool isUpToDate;
    bool m_valid;
    bool m_active;

    bool m_remote;
    bool remoteCached;
    QString remoteHerePath; ///< the local copy of a remote font, once downloaded
    bool stopperDownload;
    QFile *rFile = nullptr;
    QProgressDialog *rProgressDialog = nullptr;
    QPixmap m_remotePreview; ///< the preview the remote directory gave, shown until the file is here
    QString m_remoteInfo; ///< the information page the remote directory gave

    QString m_path;
    QUrl m_url;
    QString m_afm;
    QString m_name;
    // Basically, we collect all infos that are in an FT_FaceRec
    QString m_faceFlags;
    QString m_type;
    QString m_styleFlags;
    QString m_family;
    QString m_variant;
    QString m_fileSize;
    double m_size = 0.0;
    int m_numGlyphs = 0;
    int m_numFaces = 0;
    // 		QString m_panose;
    double unitPerEm;

    QString getAlternateFamilyName();
    QString getAlternateVariantName();
    QList<FT_Encoding> m_charsets;
    QList<int> spaceIndex;

    bool m_isOpenType;
    FMOtf *otf = nullptr;
    // words, spaces and hyphenation around whatever shapes one word
    GlyphList shapeWords(const QString &spec, double fsize, const std::function<GlyphList(const QString &)> &shapeWord);

    //		FT_Library theLibrary;
    FT_Face m_face;
    FT_Face lastFace;
    FT_Error ft_error = 0;
    //		QMap<FT_Library,FT_Face> faces;
    int facesRef = 0;
    FT_GlyphSlot m_glyph = nullptr;

    QImage glyphImage(QColor color = Qt::black);
    void fill256Palette();
    void fillInvertedPalette();

    bool m_rasterFreetype;
    unsigned int m_FTHintMode;
    // 		unsigned int m_FTRenderMode;not yet implemented

    bool ensureFace();
    void releaseFace();
    /// what the bidi algorithm needs of a shaped glyph: its character, and the glyph of
    /// the mirrored character for a bracket; the face must be open
    void markCharacters(GlyphList &glyphs, const QString &word);
    void encodeFace();

    // colour fonts, and bitmap-only fonts (CBDT, sbix), which FreeType cannot scale
    bool m_faceSeen = false; ///< the face was open once: m_hasColor is known
    bool m_hasColor = false;
    double m_headUnitsPerEm = 0.0; ///< of a bitmap-only font, whose face says 0
    double m_bitmapScale = 1.0; ///< what the bitmaps of the strike selected last are to be scaled by
    /// the units of the design: the face's, the head table's for a bitmap-only font
    [[nodiscard]] double unitsPerEm() const;
    /// FT_Set_Char_Size, or the strike of a bitmap-only font nearest to the size
    bool setSize(double size);
    bool setPixelSize(double pixels);
    bool selectStrike(double ppem);
    /// FT_LOAD_COLOR added for a colour font
    [[nodiscard]] FT_Int32 loadFlags(FT_Int32 flags) const;
    /// the metrics of the glyph in font units into m_glyph->metrics, for a bitmap-only font too
    FT_Error loadUnscaled(int index);
    void metricsToUnits();
    /// the bitmap of a strike brought to the size asked for
    [[nodiscard]] QImage scaledToStrike(const QImage &img) const;
    /// where the bitmap rendered last sits, scaled like the bitmap
    [[nodiscard]] double bitmapLeft() const;
    [[nodiscard]] double bitmapTop() const;
    [[nodiscard]] double bitmapAdvance() const;
    /// the glyph of a colour font for a scene of paths; nullptr when it has no colour
    QGraphicsPathItem *colorItem(int index, double scalefactor);
    /// COLR version 1: the HarfBuzz font that paints, while the face is open
    hb_font_t *m_paintFont = nullptr;
    /// OpenType-SVG: the documents of the glyphs, while the face is open
    FMSvgGlyphs *m_svgGlyphs = nullptr;
    /// the glyph painted at the size set, in place of what FreeType rendered; false when it has no paint
    bool paintedGlyph(int index, QImage &img, double &left, double &top);

    // variable fonts: the axes and the named instances are read once from the
    // face, the coordinates shown are set on it at every open
    bool m_variationRead = false;
    QList<FontVariationAxis> m_axes;
    QList<FontNamedInstance> m_instances;
    QList<double> m_coords; ///< the design coordinates shown, empty for the default
    void readVariation();
    void applyVariation();
    /// sets these coordinates on the open face, empty for the default of the font
    void applyVariation(const QList<double> &coords);
    QString sfntName(unsigned int nameId);

    QList<int> getAlternates(int ccode);
    QString panose();

    QString testFlag(long flag, long against, QString yes, QString no);
    QByteArray pixarray(uchar *b, int len);

    QList<QGraphicsPixmapItem *> pixList;
    QList<QGraphicsPathItem *> glyphList;
    QList<QGraphicsTextItem *> labList;
    QList<QGraphicsRectItem *> selList;
    QMap<int, QGraphicsPixmapItem *> fancyGlyphs;
    QMap<int, QGraphicsTextItem *> fancyTexts;
    QMap<int, QList<QGraphicsPixmapItem *>> fancyAlternates;

    bool allIsRendered;
    bool isDerendered = false;
    int m_glyphsPerRow;
    bool m_isEncoded;
    bool m_unicodeBuiltIn = false;
    FT_Encoding m_currentEncoding = FT_ENCODING_NONE;
    int currentChar;

    // 		bool m_RTL;// Right to Left
    // 		bool m_VertUD;// Vertical Up Down
    int m_progression;

    // 		bool m_lock;

    QMap<int, double> advanceCache;

    //		QIcon theOneLinePreviewIcon;
    //		QPixmap theOneLinePreviewPixmap;

    static QGraphicsScene *theOneLineScene;

    FontInfoMap moreInfo_sfnt();
    FontInfoMap moreInfo_type1();

    // 		FontInfoMap moreInfo;
    // 		QMap<QString, QString> panoseInfo;

    int m_shaperType;

    // if true return width, else return number of _chars_ consumed
    bool renderReturnWidth;

private:
    void downloadEnd(QNetworkReply *reply);

Q_SIGNALS:
    /// the file of a remote font has arrived (ok), or the download failed
    void downloadFinished(bool ok);
    /// the coordinates of a variable font moved: what shows the font has to render it again
    void variationChanged();

public:
    QString renderSVG(const QString &s, const double &size);
    int countCoverage(int begin_code, int end_code);
    bool hasCharcode(int cc);
    bool hasChars(const QString &s);
    int firstChar();
    int lastChar();
    int countChars();
    int nextChar(int from, int offset = 1);
    unsigned short getNamedChar(const QString &name);
    QStringList getNames();

    /// We prepare ejection of renderLine methods
    GlyphList glyphs(QString spec, double fsize);
    GlyphList glyphs(QString spec, double fsize, OTFSet set);
    // Shaped the way applications do, with the features the script asks for
    GlyphList glyphsShaped(const QString &spec, double fsize);
    GlyphList glyphs(QString spec, double fsize, QString script);

    // experiences go there
    void exploreKernFeature();

public:
    [[nodiscard]] QString path() const
    {
        return m_path;
    }
    [[nodiscard]] QString afm() const
    {
        return m_afm;
    }
    void setAfm(QString apath)
    {
        m_afm = apath;
    }
    [[nodiscard]] QString faceFlags() const
    {
        return m_faceFlags;
    }
    [[nodiscard]] QString family() const
    {
        return m_family;
    }
    [[nodiscard]] QString variant() const
    {
        return m_variant;
    }
    [[nodiscard]] QStringList tags() const;
    [[nodiscard]] int glyphsCount() const;
    QString type()
    {
        return m_type;
    }
    QStringList charmaps();
    void setTags(QStringList l);
    void addTag(const QString &t);
    QString name();
    QString fancyName()
    {
        return m_family + QLatin1Char(' ') + m_variant;
    }
    QString infoGlyph(int index, int code = 0);
    QString glyphName(int codepoint, bool codeIsChar = true);
    FontInfoMap rawInfo();
    FontInfoMap moreInfo();
    FsType getFsType();
    QStringList supportedLangDeclaration();
    double italicAngle();

    // Return the length of a TT table
    int table(const QString &tableName);
    QByteArray tableData(const QString &tableName);

    // 		QString value ( QString k );
    // 		QString panose( QString k );

    double renderLine(QGraphicsScene *scene, QString spec, QPointF origine, double lineWidth, double fsize, double zindex = 100.0);
    double renderLine(OTFSet set, QGraphicsScene *scene, QString spec, QPointF origine, double lineWidth, double fsize);
    double renderLine(QString script, QGraphicsScene *scene, QString spec, QPointF origine, double lineWidth, double fsize);
    QGraphicsPathItem *itemFromChar(int charcode, double size);
    QGraphicsPathItem *itemFromGindex(int index, double size);

    QGraphicsPixmapItem *itemFromCharPix(int charcode, double size);
    QGraphicsPixmapItem *itemFromGindexPix(int index, double size);
    // cant have qpixmap outside main thread and QGraphicsPixmapItem create at least  a null one when instantiated
    MetaGlyphItem *itemFromGindexPix_mt(int index, double size);

    QImage charImage(int charcode, double size);
    QImage glyphImage(int index, double size);

    void renderAll(QGraphicsScene *scene, int begin_code, int end_code);
    // return count codes that remain
    int renderChart(QGraphicsScene *scene, int begin_code, int end_code, double pwidth, double pheight);

    void deRenderAll();

    // Return a ref that will be asked for destroy the element. -1 if failed
    int showFancyGlyph(QGraphicsView *view, int charcode, bool charcodeIsAGlyphIndex = false);
    void hideFancyGlyph(int ref);

    QString toElement();

    QGraphicsPathItem *hasCodepointLoaded(int code);

    void trimSpacesIndex();

    QString activationName();
    QString activationAFMName();

    // 		QIcon oneLinePreviewIcon ( QString oneline );
    /// the line drawn with the font, at `coords` when they are given (a named instance of a
    /// variable font) and else at the coordinates the font is shown with
    QPixmap oneLinePreviewPixmap(QString oneline, QColor fg_color, QColor bg_color, int size_w = 0, int fsize = 0, const QList<double> &coords = {});
    void clearPreview();

    [[nodiscard]] bool isActivated() const;
    void setActivated(bool act);

    bool isLocal();

    // Relative to fontactionwidget
    // 		void lock() {m_lock=true;};
    // 		void unLock() {m_lock=false;};
    // 		bool isLocked() {return m_lock;};

    // 		int debug_size();

    void adjustGlyphsPerRow(int width);
    bool isOpenType()
    {
        return m_isOpenType;
    }
    FMOtf *takeOTFInstance();
    void releaseOTFInstance(FMOtf *rotf);
    // Returns a flat list of OT features
    QStringList features();

    void setFTRaster(bool f)
    {
        m_rasterFreetype = f;
    }
    [[nodiscard]] bool rasterFreetype() const
    {
        return m_rasterFreetype;
    }

    void setProgression(int p)
    {
        m_progression = p;
    }
    [[nodiscard]] int progression() const
    {
        return m_progression;
    }

    // sfnt names
    // 		static void fillNamesMeaning();
    // // 		static void fillPanoseMap();
    static void fillFSftypeMap();

    bool isValid()
    {
        return m_valid;
    }

    [[nodiscard]] bool isRemote() const
    {
        return m_remote;
    }
    /// a remote font whose file has been downloaded
    [[nodiscard]] bool isCached() const
    {
        return remoteCached;
    }
    /// the file to open: the download of a remote font, the path of any other
    [[nodiscard]] QString localPath() const
    {
        return m_remote ? remoteHerePath : m_path;
    }
    /// what a remote directory knows about the font, before its file is here
    void fileRemote(const QString &family, const QString &variant, const QString &type, const QString &info, const QPixmap &pixmap);
    void fileLocal(QString family, QString variant, QString type, QString p);
    void fileLocal(FontLocalInfo);
    /**
     * Starts the download of a remote font into typotek::remoteTmpDir().
     * @return 1 when the file is already here, 2 when the download runs or was
     * started (downloadFinished() tells the end), 0 when it cannot start.
     */
    int getFromNetwork();
    /// a path is a remote font when it is a URL
    static bool isRemotePath(const QString &path);

    void setShaperType(int theValue);
    [[nodiscard]] int shaperType() const;

    void setRenderReturnWidth(bool theValue)
    {
        renderReturnWidth = theValue;
    }

    double getUnitPerEm();
    void setFTHintMode(unsigned int theValue);
    [[nodiscard]] unsigned int getFTHintMode() const;

    void dumpIntoDB();

    [[nodiscard]] bool getUnicodeBuiltIn() const;

    /// a font with colour glyphs: bitmaps (CBDT, sbix) or layered outlines (COLR)
    bool hasColor();
    /// the tables the colour comes from, for the information page
    QStringList colorTables();

    /// a font with variation axes (OpenType Font Variations)
    bool isVariable();
    QList<FontVariationAxis> variationAxes();
    QList<FontNamedInstance> namedInstances();
    /// the design coordinates the font is shown with, one per axis; empty for the default
    [[nodiscard]] QList<double> variationCoordinates() const
    {
        return m_coords;
    }
    /**
     * Sets the design coordinates every rendering of the font uses from now
     * on, one per axis, empty for the default of the font. A value outside the
     * range of its axis is clamped by FreeType.
     */
    void setVariationCoordinates(const QList<double> &coords);
    /// the named instance the coordinates shown are those of, -1 when they are not
    int namedInstance();
    /**
     * Keeps the coordinates shown now for the next session: the font is shown with
     * them again, everywhere, the first time it is opened. The default of the font
     * forgets them. What the user chose, not what a view sets for a moment.
     */
    void rememberVariation();

    [[nodiscard]] FT_Encoding getCurrentEncoding() const;

    [[nodiscard]] double getUnitPerEm() const;

    [[nodiscard]] QList<FT_Encoding> getCharsets() const;
};

#endif
