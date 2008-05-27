/***************************************************************************
 *   Copyright (C) 2007 by Pierre Marchand   *
 *   pierre@oep-h.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef FONTITEM_H
#define FONTITEM_H

#include <QString>
#include <QStringList>
#include <QByteArray>
#include <QMap>
#include <QPointF>
#include <QPainterPath>
#include <QGraphicsPathItem>
#include <QIcon>
#include <QPixmap>
#include <QUrl>
// #include <QThread>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "fmsharestruct.h"

class QGraphicsPixmapItem;
class QGraphicsScene;
class QGraphicsRectItem;
class QGraphicsTextItem;
struct OTFSet;
class FMOtf;
class QGraphicsView;

class QProgressDialog;
class QHttp;
class QFile;

#define PROGRESSION_LTR 0
#define PROGRESSION_RTL 2
#define PROGRESSION_TTB 4
#define PROGRESSION_BTT 8

#define GLYPH_DATA_GLYPH 1
#define GLYPH_DATA_BITMAPLEFT 2
#define GLYPH_DATA_BITMAPTOP 3
#define GLYPH_DATA_HADVANCE 4
#define GLYPH_DATA_VADVANCE 5
#define GLYPH_DATA_FONTNAME 100

/**
	@author Pierre Marchand <pierre@oep-h.com>

	(reminder) glyph data -> 1 = index, 2 = charcode
*/
struct FontLocalInfo
{
	QString file;
	QString family;
	QString variant;
	QString type;
	QString panose;
	QMap<int,QMap<QString, QString> > info;
	QStringList tags;
	QPixmap pix;
};

class FontItem : public QObject
{

		Q_OBJECT
	public:
		FontItem ( QString path , bool remote = false, bool faststart = false);
		/** Needed when the item has been instantiate with "faststart=true" */
		void updateItem();

		~FontItem();
	private:
		bool m_valid;
		
		bool m_remote;
		bool remoteCached;
		QString remoteHerePath;
		bool stopperDownload;
		QHttp *rHttp;
		QFile *rFile;
		int remoteId;
		QProgressDialog *rProgressDialog;
		
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
		double m_size;
		int m_numGlyphs;
		int m_numFaces;
		QString m_panose;
		
		void fillCharsetMap();
		void fillLegitimateSpaces();
		void fillLangIdMap();
		void fill256Palette();
		void fillInvertedPalette();
		
		QString getAlternateFamilyName();
		QString getAlternateVariantName();
		QStringList m_charsets;
		QList<int> spaceIndex;
		
		bool m_isOpenType;
		FMOtf *otf;

// 		QString m_author;
// 		QString m_foundry;

		QStringList m_tags;
		QString m_cacheInfo;
		static FT_Library theLibrary;
		FT_Error      ft_error;
		FT_Face m_face;
		int facesRef;
		FT_GlyphSlot m_glyph;
		
		QGraphicsPathItem* itemFromChar ( int charcode, double size );
		QGraphicsPathItem* itemFromGindex ( int index, double size );
		
		QGraphicsPixmapItem* itemFromCharPix ( int charcode, double size );
		QGraphicsPixmapItem* itemFromGindexPix ( int index, double size );
		
		QImage glyphImage();
		
		bool m_rasterFreetype;
		
		bool ensureLibrary();
		bool ensureFace();
		void releaseFace();
		
		QList<int> getAlternates(int ccode);

		QString testFlag ( long flag , long against, QString yes, QString no );
		QByteArray pixarray ( uchar *b, int len );


		QList<QGraphicsPixmapItem *> pixList;
		QList<QGraphicsPathItem*> glyphList;
		QList<QGraphicsTextItem*> labList;
		QList<QGraphicsRectItem*> selList;
		QList<QGraphicsScene *> sceneList;
		QMap<int, QGraphicsPixmapItem*> fancyGlyphs;
		QMap<int, QGraphicsTextItem*> fancyTexts;
		QMap<int, QList<QGraphicsPixmapItem*> > fancyAlternates;

		QPixmap fixedPixmap;
		
		bool allIsRendered;
		bool isDerendered;
		int m_glyphsPerRow;
		bool hasUnicode;
		int currentChar;
		
// 		bool m_RTL;// Right to Left
// 		bool m_VertUD;// Vertical Up Down
		int m_progression;
		
		bool m_lock;
		
		QMap<int,double> advanceCache;

		QIcon theOneLinePreviewIcon;
		QPixmap theOneLinePreviewPixmap;

		static QGraphicsScene *theOneLineScene;

		void moreInfo_sfnt();
		void moreInfo_type1();

		QMap<int,QMap<QString, QString> > moreInfo;
		QMap<QString, QString> panoseInfo;
		
		int m_shaperType;
		
		// if true return width, else return number of _chars_ consumed
		bool renderReturnWidth;
		
	private slots:
		void slotDownloadStart(int id);
		void slotDowloadProgress(int done, int total );
		void slotDownloadEnd(int id, bool error );
		void slotDownloadDone(bool error);
		
		void slotDownloadState(int state);
		
		
	signals:
		void dowloadFinished();

	public:

		QString path() {return m_path;}
		QString afm() {return m_afm;}
		void setAfm ( QString apath ) {m_afm = apath;}
		QString faceFlags() {return m_faceFlags;}
		QString family() {return m_family;}
		QString variant() {return m_variant;}
		QStringList tags() {return m_tags;}
		int glyphsCount() {return m_numGlyphs;}
		QString type(){return m_type;}
		QStringList charmaps(){return m_charsets;}
		void setTags ( QStringList l );
		void addTag(const QString &t){if(!m_tags.contains(t) && !t.isEmpty())m_tags.append(t);}
		QString name();
		QString fancyName() {return m_family + " " + m_variant;}
		QString infoText ( bool fromcache = true );
		QString infoGlyph ( int index, int code = 0 );
		QString glyphName(int codepoint);
		QMap<int,QMap<QString, QString> >& rawInfo();
		QString panose(){return m_panose;}

		QString value ( QString k );
		QString panose( QString k );

		double renderLine ( QGraphicsScene *scene, QString spec,  QPointF origine, double lineWidth, double fsize, double zindex = 100.0 ,bool record = true );
		double renderLine ( OTFSet set, QGraphicsScene *scene, QString spec,  QPointF origine, double lineWidth,double fsize, bool record = true );
		double renderLine ( QString script, QGraphicsScene *scene, QString spec,  QPointF origine, double lineWidth,double fsize, bool record = true );
		/// We prepare ejection of renderLine methods
		GlyphList glyphs(QString spec, double fsize);
		GlyphList glyphs(QString spec, double fsize, OTFSet set);
		GlyphList glyphs(QString spec, double fsize, QString script);
		
		
		void renderAll ( QGraphicsScene *scene, int begin_code, int end_code );
		//return count codes that remain
		int renderChart(QGraphicsScene *scene, int begin_code, int end_code ,double pwidth, double pheight);
		int countCoverage ( int begin_code, int end_code );
		void deRender ( QGraphicsScene *scene );
		void deRenderAll();
		
		//Return a ref that will be asked for destroy the element. -1 if failed
		int showFancyGlyph(QGraphicsView *view, int charcode, bool charcodeIsAGlyphIndex = false);
		void hideFancyGlyph(int ref);
		
		QString toElement();
		
		QGraphicsPathItem* hasCodepoint ( int code );
		
		void trimSpacesIndex();
		
		QString activationName();
		QString activationAFMName();

// 		QIcon oneLinePreviewIcon ( QString oneline );
		QPixmap oneLinePreviewPixmap ( QString oneline , QColor bg_color, int size_w = 0);
		void clearPreview();

		bool isActivated();
		void setActivated ( bool act );
		
		bool isLocal();

		// Relative to fontactionwidget
		void lock() {m_lock=true;};
		void unLock() {m_lock=false;};
		bool isLocked() {return m_lock;};

// 		int debug_size();

		void adjustGlyphsPerRow ( int width );
		bool isOpenType(){return m_isOpenType;};
		FMOtf *takeOTFInstance();
		void releaseOTFInstance(FMOtf * rotf);
		
		void setFTRaster(bool f){m_rasterFreetype = f;};
		bool rasterFreetype(){return m_rasterFreetype;};
		
		void setProgression(int p){m_progression = p;}
		int progression(){return m_progression;}
		
		// sfnt names
		static void fillNamesMeaning();
		static void fillPanoseMap();
		
		bool isValid(){return m_valid;}
		
		bool isRemote(){return m_remote;}
		bool isCached(){return remoteCached;}		
		void  fileRemote(QString family, QString variant, QString type, QString info, QPixmap pixmap);
		void  fileLocal(QString family, QString variant, QString type, QString p, QMap<int, QMap< QString, QString > > info);
		void  fileLocal(FontLocalInfo);
		// retval : 1 => Ready; 2 => Wait ; ...
		int getFromNetwork();

	void setShaperType ( int theValue );
	int shaperType() const;
	
	void setRenderReturnWidth ( bool theValue )
	{
		renderReturnWidth = theValue;
	}
	
	
	
};

#endif
