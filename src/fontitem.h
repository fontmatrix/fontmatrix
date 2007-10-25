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

#include <ft2build.h>
#include FT_FREETYPE_H

class QGraphicsPixmapItem;
class QGraphicsScene;

/**
	@author Pierre Marchand <pierre@oep-h.com>
	
	(reminder) glyph data -> 1 = index, 2 = charcode
*/
class FontItem{
public:
    FontItem(QString path);

    ~FontItem();
	private:
		QString m_path;
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
		QStringList m_charsets;
		QList<int> m_charLess;
		
		QString m_author;
		QString m_foundry;
		
		QStringList m_tags;
		
		FT_Error      ft_error;
		FT_Face m_face;
		FT_GlyphSlot m_glyph;
		QGraphicsPathItem* itemFromChar(int charcode, double size);
		bool ensureLibrary();
		QString testFlag(long flag , long against, QString yes, QString no);
		QByteArray pixarray(uchar *b, int len);
		
		
		QList<QGraphicsPixmapItem *> pixList;
		QList<QGraphicsPathItem*> glyphList;
		QList<QGraphicsScene *> sceneList;
		bool allIsRendered;
// 		QGraphicsPixmapItem *loremPixmap;
		
		bool m_lock;
		QMap<int,QPainterPath> contourCache;
		QMap<int,double> advanceCache;
		
	public:
		static FT_Library theLibrary;
		static QMap<FT_Encoding, QString> charsetMap;
		
		QString path(){return m_path;};
		QString afm(){return m_afm;};
		QString faceFlags(){return m_faceFlags;};
		QString family(){return m_family;};
		QString variant(){return m_variant;};
		QStringList tags(){return m_tags;};
		void setTags(QStringList l){m_tags = l;};
		QString name();
		QString infoText();
		QString infoGlyph(int index, int code = 0);
		
		QString value(QString k);
		
		void renderLine(QGraphicsScene *scene, QString spec,  QPointF origine, double fsize);
		void renderAll(QGraphicsScene *scene);
		void deRender(QGraphicsScene *scene);
		void deRenderAll();
		
		
		// Relative to fontactionwidget
		void lock(){m_lock=true;};
		void unLock(){m_lock=false;};
		bool isLocked(){return m_lock;};
		
		
		
		
};

#endif
