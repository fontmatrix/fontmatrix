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


#ifndef WRAPLIBOTF
#define WRAPLIBOTF

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_TRUETYPE_TABLES_H

extern "C"
{
#include "harfbuzz.h"
}

#include <QString>
#include <QStringList>
#include <QMap>
#include <QList>


using namespace std;

struct OTFSet
{
	QString script;
	QString lang;
	QStringList gpos_features;
	QStringList gsub_features;
	OTFSet() {};
	OTFSet ( const OTFSet& os )
			: script ( os.script ),
			lang ( os.lang ),
			gpos_features ( os.gpos_features ),
			gsub_features ( os.gsub_features ) {};
	QString dump() {return script + "|" +lang+ "|"+ gpos_features.join ( "|" ) + gsub_features.join ( "|" );}
	bool isEmpty() { return script.isEmpty() && lang.isEmpty() && gpos_features.isEmpty() &&  gsub_features.isEmpty() ;};
};

struct RenderedGlyph
{
	int glyph;
	double xadvance;
	double yadvance;
	double xoffset;
	double yoffset;
	quint16 prop;
	QString dump()
	{
		return QString ( "Rendered Glyph(%1)(%6) : XA=%2 XO=%3 YA=%4 YO=%5" )
		       .arg ( glyph )
		       .arg ( xadvance )
		       .arg ( xoffset )
		       .arg ( yadvance )
		       .arg ( yoffset )
		       .arg ( prop );
	}
};

class FmOtf
{
	public:
		FmOtf ( FT_Face, double scale = 0.0 );
		~FmOtf ();

		enum altPolicy{TAKE_FIRST, ASK_ONCE, ASK_EACH};
		typedef QMap<HB_UShort,QList<HB_UShort> > altGlyphsMap;
		altGlyphsMap altGlyphs;
		QMap<HB_UShort, HB_UShort> regAltGlyphs;
		altPolicy altGlyphsPolicy;
		QString curString;

	private:
		FT_Face _face;
//   ScShaper * shaper;
		bool useShaper;
		HB_FontRec hbFont;
		QByteArray _memgdef,_memgsub,_memgpos;
		HB_StreamRec* gdefstream;
		HB_StreamRec* gsubstream;
		HB_StreamRec* gposstream;
		HB_GDEF _gdef;
		HB_GSUB _gsub;
		HB_GPOS _gpos;

		//OTF_GlyphString mys;
		HB_Buffer _buffer;

		bool glyphAlloc;

		int GDEF, GSUB, GPOS;




	public:

// 	OTF_GlyphString * FmOtfString() {return &mys;}
// 	int unicode(int gid){ return OTF_get_unicode(my, gid);}
		int get_glyph ( int index );//{return _buffer->out_string[index].gindex;}
		QString curTable;
		HB_UShort curScript, curLang, curLangReq;
		QString curScriptName, curLangName;
		QStringList curFeatures;

		/*
		 * These members functions apply features currently set
		 */
	private:
		int procstring ( QString s, QString script, QString lang, QStringList gsub, QStringList gpos );
		int procstring1 ( QString s, QString script, QString lang, QStringList gsub, QStringList gpos );
	public:
		QList<RenderedGlyph> procstring ( QString s, OTFSet set );
		/*
		  * These functions give access to informations contained in the fontfile
		 */
		QStringList   get_tables ();
		QStringList   get_scripts ();
		QStringList   get_langs ();
		QStringList   get_features ( bool required=false );
		/*
		 * These allow to set up the features ( Tab -> Scr -> Lan -> Fea )
		 */
		void set_table ( QString );
		void set_script ( QString );
		void set_lang ( QString );
		void set_features ( QStringList );


//   uint get_position(int,GlyphLayout *);
//   uint presentAlternates(HB_UInt, HB_UShort, QList<HB_UShort>);
		QList<RenderedGlyph> get_position ( HB_Buffer abuffer = 0 );

		friend class FontItem;



};

#endif
