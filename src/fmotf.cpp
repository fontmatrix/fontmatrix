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


#include "fmotf.h"

#include <QDebug>

namespace Harfbuzz
{

	HB_LineBreakClass HB_GetLineBreakClass ( HB_UChar32 ch )
	{
		return ( HB_LineBreakClass ) 0;
	}

	void HB_GetUnicodeCharProperties ( HB_UChar32 ch, HB_CharCategory *category, int *combiningClass )
	{
		*category = ( HB_CharCategory ) QChar::Category ( ch );
		*combiningClass = QChar::CombiningClass ( ch );
	}

	HB_CharCategory HB_GetUnicodeCharCategory ( HB_UChar32 ch )
	{
		return ( HB_CharCategory ) QChar::Category ( ch );
	}

	int HB_GetUnicodeCharCombiningClass ( HB_UChar32 ch )
	{
		return QChar::CombiningClass ( ch );
	}

	HB_UChar16 HB_GetMirroredChar ( HB_UChar16 ch )
	{
		return QChar ( ch ).mirroredChar().unicode();
	}
}

// Harfbuzz::HB_LineBreakClass HB_GetLineBreakClass ( Harfbuzz::HB_UChar32 ch )
// {
// 	return ( Harfbuzz::HB_LineBreakClass ) 0;
// }
//
// void HB_GetUnicodeCharProperties ( Harfbuzz::HB_UChar32 ch, Harfbuzz::HB_CharCategory *category, int *combiningClass )
// {
// 	*category = ( Harfbuzz::HB_CharCategory ) QChar::Category ( ch );
// 	*combiningClass = QChar::CombiningClass ( ch );
// }
//
// Harfbuzz::HB_CharCategory HB_GetUnicodeCharCategory ( Harfbuzz::HB_UChar32 ch )
// {
// 	return ( Harfbuzz::HB_CharCategory ) QChar::Category ( ch );
// }
//
// int HB_GetUnicodeCharCombiningClass ( Harfbuzz::HB_UChar32 ch )
// {
// 	return QChar::CombiningClass ( ch );
// }
//
// Harfbuzz::HB_UChar16 HB_GetMirroredChar ( Harfbuzz::HB_UChar16 ch )
// {
// 	return QChar ( ch ).mirroredChar().unicode();
// }

Harfbuzz::HB_UShort  AltFunc ( Harfbuzz::HB_UInt    pos,
                               Harfbuzz::HB_UShort   glyphID,
                               Harfbuzz::HB_UShort   num_alternates,
                               Harfbuzz::HB_UShort*  alternates,
                               void*       data )
{

	return ( Harfbuzz::HB_UShort ) 0;
}

Harfbuzz::HB_UChar32 getChar ( const Harfbuzz::HB_UChar16 *string, Harfbuzz::hb_uint32 length, Harfbuzz::hb_uint32 &i )
{
	qDebug() << "HB_UChar32 getChar";
	Harfbuzz::HB_UChar32 ch;
// HB_SurrogateToUcs4 expands in HB_UChar32 without prefix, i have to expand manually
	if ( ( ( string[i] & 0xfc00 ) == 0xd800 ) //  HB_IsHighSurrogate
	        && i < length - 1
	        && ( ( string[i + 1] & 0xfc00 ) == 0xdc00 ) ) // HB_IsLowSurrogate
	{
		ch = ( ( ( Harfbuzz::HB_UChar32 ) string[i] ) <<10 ) + ( string[i + 1] ) - 0x35fdc00;// HB_SurrogateToUcs4
		++i;
	}
	else
	{
		ch = string[i];
	}
	return ch;
}

Harfbuzz::HB_Bool hb_stringToGlyphs ( Harfbuzz::HB_Font font, const Harfbuzz::HB_UChar16 *string, Harfbuzz::hb_uint32 length, Harfbuzz::HB_Glyph *glyphs, Harfbuzz::hb_uint32 *numGlyphs, Harfbuzz::HB_Bool /*rightToLeft*/ )
{
	qDebug() << "HB_Bool hb_stringToGlyphs";
	FT_Face face = ( FT_Face ) font->userData;
	if ( length > *numGlyphs )
		return false;

	int glyph_pos = 0;
	for ( Harfbuzz::hb_uint32 i = 0; i < length; ++i )
	{
		glyphs[glyph_pos] = FT_Get_Char_Index ( face, getChar ( string, length, i ) );
		++glyph_pos;
	}

	*numGlyphs = glyph_pos;

	return true;
}

void hb_getAdvances ( Harfbuzz::HB_Font font, const Harfbuzz::HB_Glyph * glyphs, Harfbuzz::hb_uint32 numGlyphs, Harfbuzz::HB_Fixed *advances, int flags )
{
	qDebug() << "void hb_getAdvances with flag("<<QString::number ( flags, 16 ) <<")";
	FT_Face face = ( FT_Face ) font->userData;
	for ( Harfbuzz::hb_uint32 i = 0; i < numGlyphs; ++i )
	{
		qDebug() << "\tLoad index "<< i;
		qDebug() << "\tWhich is glyph "<<glyphs[i];
		FT_Load_Glyph ( face, glyphs[i],FT_LOAD_NO_SCALE );
		qDebug() << "ADV("<< glyphs[i] <<")("<< face->glyph->metrics.horiAdvance <<")";
		advances[i] = face->glyph->metrics.horiAdvance;
	}
}

Harfbuzz::HB_Bool hb_canRender ( Harfbuzz::HB_Font font, const Harfbuzz::HB_UChar16 *string, Harfbuzz::hb_uint32 length )
{
	qDebug() << "HB_Bool hb_canRender";
	FT_Face face = ( FT_Face ) font->userData;

	for ( Harfbuzz::hb_uint32 i = 0; i < length; ++i )
		if ( !FT_Get_Char_Index ( face, getChar ( string, length, i ) ) )
			return false;

	return true;
}

Harfbuzz::HB_Error hb_getSFntTable ( void *font, Harfbuzz::HB_Tag tableTag, Harfbuzz::HB_Byte *buffer, Harfbuzz::HB_UInt *length )
{
	qDebug() << "HB_Error hb_getSFntTable";
	FT_Face face = ( FT_Face ) font;
	FT_ULong ftlen = *length;
	FT_Error error = 0;

	if ( !FT_IS_SFNT ( face ) )
		return Harfbuzz::HB_Err_Invalid_Argument;

	error = FT_Load_Sfnt_Table ( face, tableTag, 0, buffer, &ftlen );
	*length = ftlen;
	return ( Harfbuzz::HB_Error ) error;
}

Harfbuzz::HB_Error hb_getPointInOutline ( Harfbuzz::HB_Font font, Harfbuzz::HB_Glyph glyph, int flags, Harfbuzz::hb_uint32 point, Harfbuzz::HB_Fixed *xpos, Harfbuzz::HB_Fixed *ypos, Harfbuzz::hb_uint32 *nPoints )
{
	qDebug() << "HB_Error hb_getPointInOutline";
	Harfbuzz::HB_Error error = Harfbuzz::HB_Err_Ok;
	FT_Face face = ( FT_Face ) font->userData;

	int load_flags = ( flags & Harfbuzz::HB_ShaperFlag_UseDesignMetrics ) ? FT_LOAD_NO_HINTING : FT_LOAD_DEFAULT;

	if ( ( error = ( Harfbuzz::HB_Error ) FT_Load_Glyph ( face, glyph, load_flags ) ) )
		return error;

	if ( face->glyph->format != ft_glyph_format_outline )
		return ( Harfbuzz::HB_Error ) Harfbuzz::HB_Err_Invalid_SubTable;//HB_Err_Invalid_GPOS_SubTable;

	*nPoints = face->glyph->outline.n_points;
	if ( ! ( *nPoints ) )
		return Harfbuzz::HB_Err_Ok;

	if ( point > *nPoints )
		return ( Harfbuzz::HB_Error ) Harfbuzz::HB_Err_Invalid_SubTable;//HB_Err_Invalid_GPOS_SubTable;

	*xpos = face->glyph->outline.points[point].x;
	*ypos = face->glyph->outline.points[point].y;

	return Harfbuzz::HB_Err_Ok;
}

void hb_getGlyphMetrics ( Harfbuzz::HB_Font font, Harfbuzz::HB_Glyph glyph, Harfbuzz::HB_GlyphMetrics *metrics )
{
	qDebug() << "void hb_getGlyphMetrics";
	// ###
	metrics->x = metrics->y = metrics->width = metrics->height = metrics->xOffset = metrics->yOffset = 0;
}

Harfbuzz::HB_Fixed hb_getFontMetric ( Harfbuzz::HB_Font font, Harfbuzz::HB_FontMetric metric )
{
	qDebug() << "HB_Fixed hb_getFontMetric";
	return 0; // ####
}

const Harfbuzz::HB_FontClass hb_fontClass =
{
	hb_stringToGlyphs,
	hb_getAdvances,
	hb_canRender,
	hb_getPointInOutline,
	hb_getGlyphMetrics,
	hb_getFontMetric
};

QString
OTF_tag_name ( Harfbuzz::HB_UInt tag )
{
	QString name;
	name[0] = ( char ) ( tag >> 24 );
	name[1] = ( char ) ( ( tag >> 16 ) & 0xFF );
	name[2] = ( char ) ( ( tag >> 8 ) & 0xFF );
	name[3] = ( char ) ( tag & 0xFF );
//   qDebug(QString("OTF_tag_name (%1) -> %2").arg(tag).arg(name));
	return name;
}

Harfbuzz::HB_UInt
OTF_name_tag ( QString s )
{

	Harfbuzz::HB_UInt ret = FT_MAKE_TAG ( s[0].unicode (), s[1].unicode (), ( s[2].isNull() ? ' ' :s[2].unicode () ), ( s[3].isNull() ? ' ' :s[3].unicode () ) );
//   qDebug(QString("OTF_name_tag (%1) -> %2").arg(s).arg(ret));
	return ret;
}

//#define DFLT 0xFFFF


int FmOtf::get_glyph ( int index )
{
	return _buffer->in_string[index].gindex;
}


FmOtf::FmOtf ( FT_Face f , double scale )
{

	_face = f;
	_buffer = 0;

	hbFont.klass = &hb_fontClass;
	hbFont.userData = _face ;
	hbFont.x_ppem = _face->size->metrics.x_ppem;
	hbFont.y_ppem = _face->size->metrics.y_ppem;
// 	if(scale == 0.0)
// 	{
// 		hbFont.x_scale = _face->size->metrics.x_scale;
// 		hbFont.y_scale = _face->size->metrics.y_scale;
// 	}
// 	else
	{
		hbFont.x_scale = scale;
		hbFont.y_scale = scale;
	}

	glyphAlloc = false;
	FT_ULong length = 0;

	if ( !FT_Load_Sfnt_Table ( _face, OTF_name_tag ( "GDEF" ), 0, NULL, &length ) )
	{
// 		qDebug() << QString ( "length of GDEF table is %1" ).arg ( length ) ;
		if ( length > 0 )
		{
			_memgdef.resize ( length );
			FT_Load_Sfnt_Table ( _face, OTF_name_tag ( "GDEF" ), 0,
			                     ( FT_Byte * ) _memgdef.data (), &length );
			gdefstream = new ( Harfbuzz::HB_StreamRec );
			gdefstream->base = ( Harfbuzz::HB_Byte * ) _memgdef.data ();
			gdefstream->size = _memgdef.size ();
			gdefstream->pos = 0;


			Harfbuzz::HB_New_GDEF_Table ( &_gdef );
			if ( !Harfbuzz::HB_Load_GDEF_Table ( gdefstream, &_gdef ) )
				GDEF = 1;
			else
				GDEF = 0;
		}

		else
			GDEF = 0;
	}
	else
		GDEF = 0;
	length = 0;
	if ( !FT_Load_Sfnt_Table ( _face, OTF_name_tag ( "GSUB" ), 0, NULL, &length ) )
	{
// 		qDebug()<< QString ( "length of GSUB table is %1" ).arg ( length ) ;
		if ( length > 32 ) //Some font files seem to have a fake table that is just 32 words long and make harbuzz confused
		{
			_memgsub.resize ( length );
			FT_Load_Sfnt_Table ( _face, OTF_name_tag ( "GSUB" ), 0,
			                     ( FT_Byte * ) _memgsub.data (), &length );
			gsubstream = new ( Harfbuzz::HB_StreamRec );
			gsubstream->base = ( Harfbuzz::HB_Byte * ) _memgsub.data ();
			gsubstream->size = _memgsub.size ();
			gsubstream->pos = 0;

			if ( GDEF ? !Harfbuzz::HB_Load_GSUB_Table ( gsubstream, &_gsub, _gdef, gdefstream ) :
			        !Harfbuzz::HB_Load_GSUB_Table ( gsubstream, &_gsub, NULL, NULL ) )
			{
				GSUB = 1;
				//HB_GSUB_Register_Alternate_Function( _gsub, AltFunc, &altGlyphs);
			}
			else
				GSUB = 0;
		}
		else
			GSUB = 0;
	}
	else
		GSUB = 0;

	length = 0;
	if ( !FT_Load_Sfnt_Table ( _face, OTF_name_tag ( "GPOS" ), 0, NULL, &length ) )
	{
// 		qDebug () << QString ( "length of GPOS table is %1" ).arg ( length  );
		if ( length > 32 )
		{
			_memgpos.resize ( length );
			FT_Load_Sfnt_Table ( _face, OTF_name_tag ( "GPOS" ), 0,
			                     ( FT_Byte * ) _memgpos.data (), &length );
			gposstream = new ( Harfbuzz::HB_StreamRec );
			gposstream->base = ( Harfbuzz::HB_Byte * ) _memgpos.data ();
			gposstream->size = _memgpos.size ();
			gposstream->pos = 0;

			if ( GDEF ? !Harfbuzz::HB_Load_GPOS_Table ( gposstream, &_gpos, _gdef, gdefstream ) :
			        !Harfbuzz::HB_Load_GPOS_Table ( gposstream, &_gpos, NULL, NULL ) )
				GPOS = 1;
			else
				GPOS = 0;
		}
		else
			GPOS = 0;
	}
	else
		GPOS = 0;
// 	if ( Harfbuzz::hb_buffer_new ( &_buffer ) )
// 		qDebug ( "unable to get _buffer" );
}


FmOtf::~FmOtf ()
{

	/// All those free lead to segfault in Harfbuzz, we’ll see later,
	/// now, we really want to be able to remove a font.
	if ( _buffer )
		Harfbuzz::hb_buffer_free ( _buffer );
	if ( GDEF )
		Harfbuzz::HB_Done_GDEF_Table ( _gdef );
	if ( GSUB )
		Harfbuzz::HB_Done_GSUB_Table ( _gsub );
	if ( GPOS )
		Harfbuzz::HB_Done_GPOS_Table ( _gpos );

}

int FmOtf::procstring ( QString s, QString script, QString lang, QStringList gsub, QStringList gpos )
{
	curString = s;
	regAltGlyphs.clear();
	return procstring1 ( s,script,lang,gsub,gpos );

}

QList<RenderedGlyph> FmOtf::procstring ( QString s, OTFSet set )
{
	curString = s;
	regAltGlyphs.clear();
	if ( Harfbuzz::hb_buffer_new ( &_buffer ) != Harfbuzz::HB_Err_Ok)
	{
		qDebug ( ) << "Unable to get _buffer("<< _buffer <<")";
		return QList<RenderedGlyph>();
	}
	int numR = procstring1 ( s, set.script, set.lang, set.gsub_features, set.gpos_features );
	
	QList<RenderedGlyph> ret = get_position();
	
	Harfbuzz::hb_buffer_free ( _buffer );
	_buffer = 0;
	
	return ret;
}

#ifdef FM_OWNSHAPER
QList< RenderedGlyph > FmOtf::procstring( QList<Character> shaped , QString script )
{
// 	QString script = "latn";
	QString lang = "dflt";
	regAltGlyphs.clear();
	if ( Harfbuzz::hb_buffer_new ( &_buffer ) != Harfbuzz::HB_Err_Ok)
	{
		qDebug ( ) << "Unable to get _buffer("<< _buffer <<")";
		return QList<RenderedGlyph>();
	}
	Harfbuzz::hb_buffer_clear ( _buffer );
	int n = shaped.count ();
	Harfbuzz::HB_Error           error;
	uint all = 0x1;
	QMap<QString,uint> props;
	
	//First we collect properties
	for( int i = 0; i < n; i++ )
	{
		foreach(QString cProp, shaped[i].CustomProperties)
		{
			if(!props.contains(cProp))
				props[cProp] = ++all ;
		}
	}
	for ( int i = 0; i < n; i++ )
	{
		uint prop = 0;
// 		prop |= all;
		foreach(QString cProp, shaped[i].CustomProperties)
		{
			prop |= ~(props[cProp]);
		}
		if(!prop)
		{
			prop = 0xFFFF;
		}
		error = Harfbuzz::hb_buffer_add_glyph ( _buffer, FT_Get_Char_Index ( _face, shaped[i].unicode() ), prop, i );
		qDebug() << "Adding "<< QString::number(shaped[i].unicode(),16) << "["<<prop<<"]";
		if ( error !=  Harfbuzz::HB_Err_Ok )
			qDebug() << "hb_buffer_add_glyph () failed";

	}

// 	if ( ! gsub.isEmpty() )
	{

		Harfbuzz::HB_GSUB_Clear_Features ( _gsub );

		set_table ( "GSUB" );
		set_script ( script );
		set_lang ( lang );

		for ( QMap<QString,uint>::iterator ife = props.begin (); ife != props.end (); ife++ )
		{
			Harfbuzz::HB_UShort fidx;
			error = Harfbuzz::HB_GSUB_Select_Feature ( _gsub, OTF_name_tag ( ife.key() ), curScript, curLang, &fidx );
			if ( !error )
			{
				Harfbuzz::HB_GSUB_Add_Feature ( _gsub, fidx, ife.value() );
			}
			else
				qDebug() << QString ( "adding gsub feature [%1] failed : %2" ).arg ( *ife ).arg ( error );
		}

		error = Harfbuzz::HB_GSUB_Apply_String ( _gsub, _buffer );
		if ( error && error != Harfbuzz::HB_Err_Not_Covered )
			qDebug () << QString ( "applying gsub features to string  returned %2" ).arg ( error );

	}
// 	if ( !gpos.isEmpty() )
	{
		Harfbuzz::HB_GPOS_Clear_Features ( _gpos );
		set_table ( "GPOS" );
		set_script ( script );
		set_lang ( lang );
		for ( QMap<QString,uint>::iterator ife = props.begin (); ife != props.end (); ife++ )
		{
			Harfbuzz::HB_UShort fidx;
			error = Harfbuzz::HB_GPOS_Select_Feature ( _gpos, OTF_name_tag ( ife.key() ), curScript, curLang, &fidx );
			if ( !error )
			{
				Harfbuzz::HB_GPOS_Add_Feature ( _gpos, fidx, ife.value() );
			}
			else
				qDebug() << QString ( "adding gpos feature [%1] failed : %2" ).arg ( *ife ).arg ( error );
		}

		error = Harfbuzz::HB_GPOS_Apply_String ( &hbFont, _gpos, FT_LOAD_NO_SCALE, _buffer,
		        /*while dvi is true font klass is not used */ true,
		        /*r2l */ true );
		if ( error && error != Harfbuzz::HB_Err_Not_Covered )
			qDebug () << QString ( "applying gpos features to string returned %2" ).arg ( error ) ;

	}
	
	
	QList<RenderedGlyph> ret = get_position();
	Harfbuzz::hb_buffer_free ( _buffer );
	_buffer = 0;
	return ret;
	
}
#endif

int
FmOtf::procstring1 ( QString s, QString script, QString lang, QStringList gsub, QStringList gpos )
{
	Harfbuzz::hb_buffer_clear ( _buffer );
	int n = s.length ();
	Harfbuzz::HB_Error           error;
	uint all = 0x1;
	uint prop;
	for ( int i = 0; i < n; i++ )
	{
		prop = 0;
		prop |= all;
		error = Harfbuzz::hb_buffer_add_glyph ( _buffer,
		                                        FT_Get_Char_Index ( _face, s[i].unicode() ),
		                                        prop,
		                                        i );
		if ( error !=  Harfbuzz::HB_Err_Ok )
			qDebug() << "hb_buffer_add_glyph ("<< s[i] <<") failed";
// 		else
// 			qDebug() << "hb_buffer_add_glyph ("<< s[i] <<") success";

	}

// 	if ( _buffer->in_length > 0 )
// 	{
// 		qDebug() << "_buffer->in_length = " <<_buffer->in_length;
//
// 	}
// 	else
// 		qDebug() << "_buffer->in_length = " <<_buffer->in_length;
//


	if ( ! gsub.isEmpty() )
	{
// 		qDebug() <<"Process GSUB";

		Harfbuzz::HB_GSUB_Clear_Features ( _gsub );

// 		qDebug() <<"Set GSUB";
		set_table ( "GSUB" );
		set_script ( script );
		set_lang ( lang );
// 		qDebug() <<"GSUB set";

		for ( QStringList::iterator ife = gsub.begin (); ife != gsub.end (); ife++ )
		{
			Harfbuzz::HB_UShort fidx;
			error = Harfbuzz::HB_GSUB_Select_Feature ( _gsub,
			        OTF_name_tag ( *ife ),
			        curScript, curLang, &fidx );
			if ( !error )
			{
				Harfbuzz::HB_GSUB_Add_Feature ( _gsub, fidx, ~all );
// 				qDebug()<< QString("adding gsub feature [%1] success : %2").arg(*ife).arg(fidx );
			}
			else
				qDebug() << QString ( "adding gsub feature [%1] failed : %2" ).arg ( *ife ).arg ( error );
		}

// 		qDebug() << "APPLY";
		error = Harfbuzz::HB_GSUB_Apply_String ( _gsub, _buffer );
// 		qDebug() << "YLPPA";
//
		if ( error && error != Harfbuzz::HB_Err_Not_Covered )
			qDebug () << QString ( "applying gsub features to string \"%1\" returned %2" ).arg ( s ).arg ( error );

	}
	if ( !gpos.isEmpty() )
	{
// 		qDebug() <<"Process GPOS";

		Harfbuzz::HB_GPOS_Clear_Features ( _gpos );
		set_table ( "GPOS" );
		set_script ( script );
		set_lang ( lang );

		for ( QStringList::iterator ife = gpos.begin (); ife != gpos.end ();
		        ife++ )
		{
			uint fprop = 0xffff;
// 			fprop |= all | init | fina;
			Harfbuzz::HB_UShort fidx;
			error = Harfbuzz::HB_GPOS_Select_Feature ( _gpos,
			        OTF_name_tag ( *ife ),
			        curScript, curLang, &fidx );
			if ( !error )
			{
				Harfbuzz::HB_GPOS_Add_Feature ( _gpos, fidx,fprop );
// 				qDebug()<< QString("GPOS [%2] feature.lookupcount = %1").arg(_gpos->FeatureList.FeatureRecord[fidx].Feature.LookupListCount).arg(*ife);
			}
			else
				qDebug () << QString ( "adding gsub feature [%1] failed : %2" ).arg ( *ife ).arg ( error ) ;
		}
		error = Harfbuzz::HB_GPOS_Apply_String ( &hbFont, _gpos, FT_LOAD_NO_SCALE, _buffer,
		        /*while dvi is true font klass is not used */ true,
		        /*r2l */ true );
		if ( error && error != Harfbuzz::HB_Err_Not_Covered )
			qDebug () << QString ( "applying gpos features to string \"%1\" returned %2" ).arg ( s ).arg ( error ) ;

	}

//  	qDebug() << "END_PROCSTRING";
	int nret = _buffer->in_length;
// 	Harfbuzz::hb_buffer_free ( _buffer );
	return nret;
}






QStringList
FmOtf::get_tables ()
{
	QStringList ret;

	//   if (GDEF)scName()
	//     ret.insert (ret.end (), "GDEF");
	if ( GPOS )
		ret.insert ( ret.end (), "GPOS" );
	if ( GSUB )
		ret.insert ( ret.end (), "GSUB" );

	return ret;
}

void
FmOtf::set_table ( QString s )
{
	curTable = s;
}

QStringList
FmOtf::get_scripts ()
{
//	qDebug("FmOtf::get_scripts ()");
	QStringList ret;

	if ( curTable == "GSUB" && GSUB )
	{
		Harfbuzz::HB_UInt *taglist;
		if ( Harfbuzz::HB_GSUB_Query_Scripts ( _gsub, &taglist ) )
			qDebug ( "error HB_GSUB_Query_Scripts" );
		while ( *taglist )
		{
// 			qDebug ( QString ( "script [%1]" ).arg ( OTF_tag_name ( *taglist ) ) );
			ret.append ( OTF_tag_name ( *taglist ) );
			++taglist;
		}

	}
	if ( curTable == "GPOS" && GPOS )
	{

		Harfbuzz::HB_UInt *taglist;
		if ( Harfbuzz::HB_GPOS_Query_Scripts ( _gpos, &taglist ) )
			qDebug ( "error HB_GPOS_Query_Scripts" );
		while ( *taglist )
		{
			ret.append ( OTF_tag_name ( *taglist ) );
			++taglist;
		}
	}
	return ret;

}

void
FmOtf::set_script ( QString s )
{
//	qDebug(QString("set_script (%1)").arg(s));
	curScriptName = s;
	if ( curTable == "GSUB" && GSUB )
	{
		if ( Harfbuzz::HB_GSUB_Select_Script
		        ( _gsub, OTF_name_tag ( curScriptName ), &curScript ) )
			qDebug ( "Unable to set script index " );
	}
	if ( curTable == "GPOS" && GPOS )
	{
		if ( Harfbuzz::HB_GPOS_Select_Script
		        ( _gpos, OTF_name_tag ( curScriptName ), &curScript ) )
			qDebug ( "Unable to set script index" );
	}
}


QStringList
FmOtf::get_langs ()
{
	QStringList ret;

	ret << "default";
	if ( curTable == "GSUB" && GSUB )
	{

		Harfbuzz::HB_UInt *taglist;
		if ( Harfbuzz::HB_GSUB_Query_Languages ( _gsub, curScript, &taglist ) )
			qDebug ( "error HB_GSUB_Query_Langs" );
		while ( *taglist )
		{
// 			qDebug ( QString ( "lang [%1]" ).arg ( OTF_tag_name ( *taglist ) ) );
			ret.append ( OTF_tag_name ( *taglist ) );
			++taglist;
		}

	}
	if ( curTable == "GPOS" && GPOS )
	{

		Harfbuzz::HB_UInt *taglist;
		if ( Harfbuzz::HB_GPOS_Query_Languages ( _gpos, curScript, &taglist ) )
			qDebug ( "error HB_GPOS_Query_Langs" );
		while ( *taglist )
		{
			ret.append ( OTF_tag_name ( *taglist ) );
			++taglist;
		}

	}

	return ret;
}

void
FmOtf::set_lang ( QString s )
{
//	qDebug(QString("set_lang (%1)").arg(s));
	if ( s == "default" || s == "dflt" || s.isEmpty () )
	{
		curLangName = "dflt";
		curLang = 0xFFFF;// Harfbuzz::HB_DEFAULT_LANGUAGE;
		return;
	}
	Harfbuzz::HB_Error           error;
	curLangName = s;
	if ( curTable == "GSUB" && GSUB )
	{
		error = Harfbuzz::HB_GSUB_Select_Language ( _gsub,
		        OTF_name_tag ( curLangName ),
		        curScript,
		        &curLang,
		        &curLangReq );
// 		if ( error )
// 			qDebug ( QString ( "Unable to set lang index due to error %1" ).arg ( error ) );
	}
	if ( curTable == "GPOS" && GPOS )
	{
		error = Harfbuzz::HB_GPOS_Select_Language ( _gpos, OTF_name_tag ( curLangName ),curScript, &curLang, &curLangReq );
// 		if ( error )
// 			qDebug ( QString ( "Unable to set lang index due to error %1" ).arg ( error ) );
	}

}


QStringList
FmOtf::get_features ( bool required )
{
	QStringList ret;

	if ( curTable == "GSUB" && GSUB )
	{

		Harfbuzz::HB_UInt *taglist;
		required ? Harfbuzz::HB_GSUB_Query_Features ( _gsub, curScript, curLangReq,
		        &taglist ) :
		Harfbuzz::HB_GSUB_Query_Features ( _gsub, curScript, curLang, &taglist );
		while ( *taglist )
		{
			ret.append ( OTF_tag_name ( *taglist ) );
			++taglist;
		}


	}
	if ( curTable == "GPOS" && GPOS )
	{

		Harfbuzz::HB_UInt *taglist;
		required ? Harfbuzz::HB_GPOS_Query_Features ( _gpos, curScript, curLangReq,
		        &taglist ) :
		Harfbuzz::HB_GPOS_Query_Features ( _gpos, curScript, curLang, &taglist );
		while ( *taglist )
		{
			ret.append ( OTF_tag_name ( *taglist ) );
			++taglist;
		}

	}
	return ret;
}

void
FmOtf::set_features ( QStringList ls )
{
	curFeatures = ls;
}

// void dump_pos(HB_PositionRec p)
// {
// 	qDebug(QString("xpos = %1 | ypos = %2 | xadv = %3 | yadv = %4 | %5 | back = %6 ").arg(p.x_pos).arg(p.y_pos).arg(p.x_advance).arg(p.y_advance).arg(p.new_advance ? "NEW" : "NOT_NEW").arg(p.back));
// }

/**
Dump all the "uneasy" HB_Buffer into a user-friendy QList :)
*/
QList<RenderedGlyph> FmOtf::get_position ( Harfbuzz::HB_Buffer abuffer )
{
// 	qDebug() << "FmOtf::get_position () for buffer : " << _buffer;
	Harfbuzz::HB_Buffer bak_buf = _buffer ;
	if ( abuffer )
	{
		_buffer = abuffer;
	}
	QList<RenderedGlyph> renderedString;
	bool wantPos = true;
	for ( uint bIndex = 0 ; bIndex < _buffer->in_length; ++bIndex )
	{
// 		qDebug() << "bIndex = "<< bIndex;
		RenderedGlyph gl;

		gl.glyph = _buffer->in_string[bIndex].gindex;
		if ( gl.glyph == 0 )
		{
// 			qDebug() << "glyph skipped";
			continue;
		}

		Harfbuzz::HB_Position p = 0;
		if ( wantPos )
		{
			p = &_buffer->positions[bIndex] ;
			qDebug() << "p = "<< p;
			if ( !p )
				wantPos = false;
		}
		if ( !p ) // applyGPOS has not been called?
		{
			FT_GlyphSlot slot = _face->glyph;
			if ( !FT_Load_Glyph ( _face, gl.glyph , FT_LOAD_NO_SCALE ) )
			{
				gl.xadvance = ( double ) slot->metrics.horiAdvance/* ( slot->advance.x )*/;
				gl.yadvance = ( double ) slot->metrics.vertAdvance/* ( slot->advance.y )*/;
				gl.xoffset = 0;
				gl.yoffset = 0;
			}
		}
		else
		{
// 			qDebug() << p->back;
			double backBonus = 0.0;
			for ( int bb = 0; bb < p->back; ++bb )
			{
				backBonus -= renderedString[bIndex - ( bb + 1 ) ].xadvance;
			}
			if ( p->new_advance )
			{
				gl.xadvance = p->x_advance + backBonus;
				gl.yadvance = p->y_advance;
				gl.xoffset = p->x_pos;
				gl.yoffset = p->y_pos;
			}
			else
			{
				FT_GlyphSlot slot = _face->glyph;
				if ( !FT_Load_Glyph ( _face, gl.glyph , FT_LOAD_NO_SCALE ) )
				{
					gl.xadvance = ( double ) slot->metrics.horiAdvance/* ( slot->advance.x )*/ + backBonus ;
					gl.yadvance = ( double ) slot->metrics.vertAdvance/* ( slot->advance.y )*/;
					gl.xoffset = p->x_pos;
					gl.yoffset = p->y_pos;
				}
			}
		}


		renderedString << gl;
// 		qDebug() << gl.dump();
	}

	_buffer = bak_buf;
	return renderedString;
}




