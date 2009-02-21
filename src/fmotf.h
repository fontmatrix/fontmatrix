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


#include <QString>
#include <QStringList>
#include <QMap>
#include <QList>

#include "fmshaper_own.h"
#include "fmsharestruct.h"


#include <harfbuzz.h>


// using namespace std;

class FMOtf
{
	public:
		FMOtf ( FT_Face, double scale = 0.0 );
		~FMOtf ();

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

// 	OTF_GlyphString * FMOtfString() {return &mys;}
// 	int unicode(int gid){ return OTF_get_unicode(my, gid);}
		int get_glyph ( int index );//{return _buffer->out_string[index].gindex;}
		QString curTable;
		HB_UShort curScript, curLang, curLangReq;
		QString curScriptName, curLangName;
		QStringList curFeatures;

		static HB_UShort manageAlternates ( HB_UInt    pos,HB_UShort   glyphID,HB_UShort   num_alternates,HB_UShort*  alternates, void*       data );
		static QList<int> altGlyphs;
		/*
		 * These members functions apply features currently set
		 */
	public:
		// Yes there are a lot, doubtless too much.
		int procstring ( QString s, QString script, QString lang, QStringList gsub, QStringList gpos );
		QList<RenderedGlyph> procstring ( QString s, OTFSet set );
		QList<RenderedGlyph> procstring ( QList<Character> shaped , QString script );
		QList<RenderedGlyph> procstring ( QList<unsigned int> glyList , QString script, QString lang, QStringList gsub, QStringList gpos );

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
		GlyphList get_position ( HB_Buffer abuffer = 0 );

	FT_Face face() const
	{
		return _face;
	}
	

		friend class FontItem;
		friend class FMShaper;



};

#endif
