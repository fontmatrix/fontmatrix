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
#include "harfbuzz-external.h"

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
///
HB_UShort  AltFunc ( HB_UInt    pos,
                     HB_UShort   glyphID,
                     HB_UShort   num_alternates,
                     HB_UShort*  alternates,
                     void*       data )
{
	// todo
// 	qDebug(QString("%1 alternate glyphs available for glyph %2").arg(num_alternates).arg(glyphID));
// 	FmOtf* ot = (FmOtf*) data;
// 	if(ot->altGlyphsPolicy == FmOtf::TAKE_FIRST)
// 	{
// 		return (HB_UShort)0;
// 	}
// 	if(ot->altGlyphsPolicy == FmOtf::ASK_ONCE)
// 	{
// 		if(ot->regAltGlyphs.contains(glyphID))
// 			return ot->regAltGlyphs[glyphID];
// 		else
// 		{
// 			QList<HB_UShort> tmp;
// 			for(int i=0;i < num_alternates; ++i)
// 			{
// 				tmp.append(alternates[i]);
// 			}
// 			ot->regAltGlyphs[glyphID] = ot->presentAlternates(pos, glyphID, tmp);
// 			return ot->regAltGlyphs[glyphID];
// 		}
// 	}
// 	if(ot->altGlyphsPolicy == FmOtf::ASK_EACH)
// 	{
// 		QList<HB_UShort> tmp;
// 		for(int i=0;i < num_alternates; ++i)
// 		{
// 			tmp.append(alternates[i]);
// 		}
// 		return ot->presentAlternates(pos, glyphID, tmp);
// 	}
	return ( HB_UShort ) 0;
}


QString
OTF_tag_name ( HB_UInt tag )
{
	QString name;
	name[0] = ( char ) ( tag >> 24 );
	name[1] = ( char ) ( ( tag >> 16 ) & 0xFF );
	name[2] = ( char ) ( ( tag >> 8 ) & 0xFF );
	name[3] = ( char ) ( tag & 0xFF );
//   qDebug(QString("OTF_tag_name (%1) -> %2").arg(tag).arg(name));
	return name;
}

HB_UInt
OTF_name_tag ( QString s )
{

	HB_UInt ret = FT_MAKE_TAG ( s[0].unicode (), s[1].unicode (), ( s[2].isNull() ? ' ' :s[2].unicode () ), ( s[3].isNull() ? ' ' :s[3].unicode () ) );
//   qDebug(QString("OTF_name_tag (%1) -> %2").arg(s).arg(ret));
	return ret;
}

//#define DFLT 0xFFFF


int FmOtf::get_glyph ( int index )
{
	return _buffer->in_string[index].gindex;
}


FmOtf::FmOtf ( FT_Face f )
{

	_face = f;
	altGlyphsPolicy = ASK_ONCE;
	// Give a try to the shaper;
// 	shaper = new ScShaper();
// 	useShaper = shaper->setFace ( _face );
//	qDebug(QString("shaper->setFace returns : %1").arg(useShaper ? "TRUE" : "FALSE"));

	hbFont.klass = NULL;		/* Hope it will work without more code */
	hbFont.userData = _face ;
	hbFont.x_ppem = _face->size->metrics.x_ppem;
	hbFont.y_ppem = _face->size->metrics.y_ppem;
	hbFont.x_scale = 0x10000;//_face->size->metrics.x_scale;
	hbFont.y_scale = 0x10000;//_face->size->metrics.y_scale;

	glyphAlloc = false;
	FT_ULong length = 0;

	if ( !FT_Load_Sfnt_Table ( _face, OTF_name_tag ( "GDEF" ), 0, NULL, &length ) )
	{
// 		qDebug ( QString ( "length of GDEF table is %1" ).arg ( length ) );
		if ( length > 0 )
		{
			_memgdef.resize ( length );
			FT_Load_Sfnt_Table ( _face, OTF_name_tag ( "GDEF" ), 0,
			                     ( FT_Byte * ) _memgdef.data (), &length );
			gdefstream = new ( HB_StreamRec );
			gdefstream->base = ( HB_Byte * ) _memgdef.data ();
			gdefstream->size = _memgdef.size ();
			gdefstream->pos = 0;


			HB_New_GDEF_Table ( &_gdef );
			if ( !HB_Load_GDEF_Table ( gdefstream, &_gdef ) )
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
// 		qDebug ( QString ( "length of GSUB table is %1" ).arg ( length ) );
		if ( length > 32 ) //Some font files seem to have a fake table that is just 32 words long and make harbuzz confused
		{
			_memgsub.resize ( length );
			FT_Load_Sfnt_Table ( _face, OTF_name_tag ( "GSUB" ), 0,
			                     ( FT_Byte * ) _memgsub.data (), &length );
			gsubstream = new ( HB_StreamRec );
			gsubstream->base = ( HB_Byte * ) _memgsub.data ();
			gsubstream->size = _memgsub.size ();
			gsubstream->pos = 0;

			if ( GDEF ? !HB_Load_GSUB_Table ( gsubstream, &_gsub, _gdef, gdefstream ) :
			        !HB_Load_GSUB_Table ( gsubstream, &_gsub, NULL, NULL ) )
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
// 		qDebug ( QString ( "length of GPOS table is %1" ).arg ( length ) );
		if ( length > 32 )
		{
			_memgpos.resize ( length );
			FT_Load_Sfnt_Table ( _face, OTF_name_tag ( "GPOS" ), 0,
			                     ( FT_Byte * ) _memgpos.data (), &length );
			gposstream = new ( HB_StreamRec );
			gposstream->base = ( HB_Byte * ) _memgpos.data ();
			gposstream->size = _memgpos.size ();
			gposstream->pos = 0;

			if ( GDEF ? !HB_Load_GPOS_Table ( gposstream, &_gpos, _gdef, gdefstream ) :
			        !HB_Load_GPOS_Table ( gposstream, &_gpos, NULL, NULL ) )
				GPOS = 1;
			else
				GPOS = 0;
		}
		else
			GPOS = 0;
	}
	else
		GPOS = 0;
	if ( hb_buffer_new ( &_buffer ) )
		qDebug ( "unable to get _buffer" );
}


FmOtf::~FmOtf ()
{

	if ( _buffer )
		hb_buffer_free ( _buffer );
	if ( GDEF )
		HB_Done_GDEF_Table ( _gdef );
	if ( GSUB )
		HB_Done_GSUB_Table ( _gsub );
	if ( GPOS )
		HB_Done_GPOS_Table ( _gpos );

}

int FmOtf::procstring ( QString s, QString script, QString lang, QStringList gsub, QStringList gpos )
{
	curString = s;
	regAltGlyphs.clear();
// 	if ( lang == "dflt" )
// 		useShaper = shaper->setScript ( script );
// 	else
// 		useShaper = shaper->setLang ( lang );
// 	if ( !useShaper )
		return procstring1 ( s,script,lang,gsub,gpos );
// 	else
// 	{
// 		return shaper->doShape ( s );
// 	}
// 	return 0;
}

int
FmOtf::procstring1 ( QString s, QString script, QString lang, QStringList gsub,
                     QStringList gpos )
{

	hb_buffer_clear ( _buffer );
	int n = s.length ();
	HB_Error           error;
	uint all = 0x1;
	uint init = 0x2;
	uint fina = 0x8;
	uint prop;
	for ( int i = 0; i < n; i++ )
	{
		prop = 0;
		if ( i== 0 )
			prop = init;
		if ( i > 0 )
		{

			if ( !s[i-1].isLetterOrNumber() )
				prop = init;
		}
		if ( i < n-1 )
		{
			if ( !s[i+1].isLetterOrNumber() )
				prop = fina;
		}

		prop |= all;
		error = hb_buffer_add_glyph ( _buffer,
		                              FT_Get_Char_Index ( _face, s[i].unicode() ),
		                              prop,
		                              i );
//       qDebug(QString("adding char [%1] gives glyph [%2]  properties [%3] cluster [%4]").arg(s[i].unicode()).arg(_buffer->in_string[i].gindex).arg(_buffer->in_string[i].properties,8,2).arg(_buffer->in_string[i].cluster));

	}


	if ( gsub.count () )
	{
		HB_GSUB_Clear_Features ( _gsub );
		set_table ( "GSUB" );
		set_script ( script );
		set_lang ( lang );

		for ( QStringList::iterator ife = gsub.begin (); ife != gsub.end ();
		        ife++ )
		{
			uint fprop = all;
			if ( *ife == "init" )
				fprop |= init;
			else if ( *ife == "fina" )
				fprop |= fina;

// 			qDebug ( QString ( "fprop = %1, feature = %2" ).arg ( ~fprop,8,2 ).arg ( *ife ) );
			HB_UShort fidx;
			error = HB_GSUB_Select_Feature ( _gsub,
			                                 OTF_name_tag ( *ife ),
			                                 curScript, curLang, &fidx );
			if ( !error )
			{
				HB_GSUB_Add_Feature ( _gsub, fidx, ~fprop );
				//qDebug(QString("GSUB [%2] feature.lookupcount = %1").arg(_gsub->FeatureList.FeatureRecord[fidx].Feature.LookupListCount).arg(*ife));
			}
			//  else
			//qDebug(QString("adding gsub feature [%1] failed : %2").arg(*ife).arg(error));
		}

		error = HB_GSUB_Apply_String ( _gsub, _buffer );
// 		if ( error && error != HB_Err_Not_Covered ) qDebug ( QString ( "applying gsub features to string \"%1\" returned %2" ).arg ( s ).arg ( error ) );

	}
	if ( gpos.count () )
	{
		HB_GPOS_Clear_Features ( _gpos );
		set_table ( "GPOS" );
		set_script ( script );
		set_lang ( lang );

		for ( QStringList::iterator ife = gpos.begin (); ife != gpos.end ();
		        ife++ )
		{
			uint fprop = 0xffff;
			fprop |= all | init | fina;
			HB_UShort fidx;
			error = HB_GPOS_Select_Feature ( _gpos,
			                                 OTF_name_tag ( *ife ),
			                                 curScript, curLang, &fidx );
			if ( !error )
			{
				HB_GPOS_Add_Feature ( _gpos, fidx,fprop );
				// qDebug(QString("GPOS [%2] feature.lookupcount = %1").arg(_gpos->FeatureList.FeatureRecord[fidx].Feature.LookupListCount).arg(*ife));
			}
// 			else
// 				qDebug ( QString ( "adding gsub feature [%1] failed : %2" ).arg ( *ife ).arg ( error ) );
		}
		if ( _buffer->in_length > 0 ) memset ( _buffer->positions, 0, _buffer->in_length*sizeof ( HB_PositionRec ) );
		error = HB_GPOS_Apply_String ( &hbFont, _gpos, FT_LOAD_NO_SCALE, _buffer,
		                               /*while dvi is true font klass is not used */ true,
		                               /*r2l */ true );
// 		if ( error && error != HB_Err_Not_Covered ) qDebug ( QString ( "applying gpos features to string \"%1\" returned %2" ).arg ( s ).arg ( error ) );

	}


	return _buffer->in_length;
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
		HB_UInt *taglist;
		if ( HB_GSUB_Query_Scripts ( _gsub, &taglist ) )
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

		HB_UInt *taglist;
		if ( HB_GPOS_Query_Scripts ( _gpos, &taglist ) )
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
		if ( HB_GSUB_Select_Script
		        ( _gsub, OTF_name_tag ( curScriptName ), &curScript ) )
			qDebug (  "Unable to set script index "  );
	}
	if ( curTable == "GPOS" && GPOS )
	{
		if ( HB_GPOS_Select_Script
		        ( _gpos, OTF_name_tag ( curScriptName ), &curScript ) )
			qDebug ( "Unable to set script index" );
	}
}


QStringList
FmOtf::get_langs ()
{
	QStringList ret;

	if ( curTable == "GSUB" && GSUB )
	{

		HB_UInt *taglist;
		if ( HB_GSUB_Query_Languages ( _gsub, curScript, &taglist ) )
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

		HB_UInt *taglist;
		if ( HB_GPOS_Query_Languages ( _gpos, curScript, &taglist ) )
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
		curLang = HB_DEFAULT_LANGUAGE;
		return;
	}
	HB_Error           error;
	curLangName = s;
	if ( curTable == "GSUB" && GSUB )
	{
		error = HB_GSUB_Select_Language ( _gsub,
		                                  OTF_name_tag ( curLangName ),
		                                  curScript,
		                                  &curLang,
		                                  &curLangReq );
// 		if ( error )
// 			qDebug ( QString ( "Unable to set lang index due to error %1" ).arg ( error ) );
	}
	if ( curTable == "GPOS" && GPOS )
	{
		error = HB_GPOS_Select_Language ( _gpos, OTF_name_tag ( curLangName ),curScript, &curLang, &curLangReq );
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

		HB_UInt *taglist;
		required ? HB_GSUB_Query_Features ( _gsub, curScript, curLangReq,
		                                    &taglist ) :
		HB_GSUB_Query_Features ( _gsub, curScript, curLang, &taglist );
		while ( *taglist )
		{
			ret.append ( OTF_tag_name ( *taglist ) );
			++taglist;
		}


	}
	if ( curTable == "GPOS" && GPOS )
	{

		HB_UInt *taglist;
		required ? HB_GPOS_Query_Features ( _gpos, curScript, curLangReq,
		                                    &taglist ) :
		HB_GPOS_Query_Features ( _gpos, curScript, curLang, &taglist );
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
/*
uint FmOtf::get_position (int g, GlyphLayout * gl)
{
// 	qDebug(QString("%1").arg(g));
	if(useShaper)
	{
		return shaper->shapedChar(g,gl);
	}
	//ok, g is now the position in the charstring
	// so, we must have a look at clusters.
	QList<uint> where;
	for(uint i = 0 ; i < _buffer->in_length; ++i)
	{
		if(_buffer->in_string[i].cluster == g)
		{
			where.append(i);
		}
	}
	if(where.isEmpty())
	{
		gl->glyph = NULL_GLYPH;
		gl->xadvance = gl->yadvance = gl->xoffset = gl->yoffset = 0;
	}
	if(where.count() == 1)
	{
		int here = where[0];
		HB_Position p= &(_buffer->positions[here]);
		gl->glyph = _buffer->in_string[here].gindex;
		if(p->new_advance)
		{
			gl->xadvance = p->x_advance;
			gl->yadvance = p->y_advance;
			gl->xoffset = p->x_pos;
			gl->yoffset = p->y_pos;
		}
		else
		{
			FT_GlyphSlot
			slot = _face->glyph;
      			if (!FT_Load_Glyph(_face, gl->glyph , FT_LOAD_NO_SCALE))
			{
				gl->xadvance = (double) (p->x_advance + slot->advance.x);
				gl->yadvance = (double) (p->y_advance + slot->advance.y);
				gl->xoffset = p->x_pos;
				gl->yoffset = p->y_pos;
// 				qDebug(QString("advance of %1 : %2 =  %3 + %4").arg(g).arg(gl->xadvance).arg(p->x_advance).arg(slot->advance.x));
			}
		}

	}
	if(where.count() > 1)
	{
		int here = where[0];
		HB_Position p= &(_buffer->positions[here]);
		gl->glyph = _buffer->in_string[here].gindex;
		if(p->new_advance)
		{
			gl->xadvance = p->x_advance;
			gl->yadvance = p->y_advance;
			gl->xoffset = p->x_pos;
			gl->yoffset = p->y_pos;
		}
		else
		{
			FT_GlyphSlot
					slot = _face->glyph;
			if (!FT_Load_Glyph(_face, gl->glyph, FT_LOAD_NO_SCALE))
			{
				gl->xadvance = (double) (p->x_advance + slot->advance.x);
				gl->yadvance = (double) (p->y_advance + slot->advance.y);
				gl->xoffset = p->x_pos;
				gl->yoffset = p->y_pos;
			}
		}
		GlyphLayout * glt = gl;
		for(int j = 1 ; j < where.count(); ++j)
		{
			glt->grow();
			glt = glt->more;
			here = where[j];
			p = &(_buffer->positions[here]);
			glt->glyph = _buffer->in_string[here].gindex;

			glt->xadvance = p->x_advance;
			glt->yadvance = p->y_advance;
			if(p->new_advance)
			{
				glt->xadvance = p->x_advance;
				glt->yadvance = p->y_advance;
			}
			else
			{
				FT_GlyphSlot
						slot = _face->glyph;
				if (!FT_Load_Glyph(_face, glt->glyph, FT_LOAD_NO_SCALE))
				{
					glt->xadvance = (double) (p->x_advance + slot->advance.x);
					glt->yadvance = (double) (p->y_advance + slot->advance.y);
				}
			}
			if(p->back > 0) // we believe is equal to j, and pray :-)
				glt->xoffset = p->x_pos - gl->xadvance;
			else
				glt->xoffset = p->x_pos;
			glt->yoffset = p->y_pos;
		}
	}

	return where.count();
}

*/


// uint FmOtf::presentAlternates(HB_UInt pos , HB_UShort glyph, QValueList<HB_UShort> altglyphs)
// {
// 	QString contextString ;
// 	for(uint i=0; i < _buffer->in_length; ++i)
// 	{
// 		if(i != pos)
// 			contextString += curString[_buffer->in_string[i].cluster];
// 		else
// 		{
// 			contextString += " [";
// 			contextString += curString[_buffer->in_string[i].cluster];
// 			contextString += "] ";
// 		}
// 	}
//
//
// 	QValueList<QPixmap*> pix;
// 	QPainter p;
// 	int row;
// 	for(uint i = 0; i < altglyphs.count(); ++i)
// 	{
// 		qDebug(QString("render glyph %1").arg(altglyphs[i]));
// 		if(!FT_Load_Glyph(_face, altglyphs[i], FT_LOAD_RENDER))
// 		{
// 			qDebug(QString("append pixmap w = %1, h = %2").arg(_face->glyph->bitmap.pitch).arg(_face->glyph->bitmap.rows));
// 			pix.append(new QPixmap());
// 			pix.last()->resize(_face->glyph->bitmap.pitch / 10 ,_face->glyph->bitmap.rows / 10);
// 			row = _face->glyph->bitmap.pitch;
// 			p.begin(pix.last());
// 			for( int y=0; y < _face->glyph->bitmap.rows; y += 10)
// 			{
// 				for(int x= 0; x < _face->glyph->bitmap.pitch; x += 10)
// 				{
// 					p.setPen(QColor (_face->glyph->bitmap.buffer[(y * row) + x],
// 							_face->glyph->bitmap.buffer[(y * row) + x],
// 							_face->glyph->bitmap.buffer[(y * row) + x]));
// 					p.drawPoint(x/10,y/10);
// 				}
// 			}
// 			p.end();
//
// 			//pix.last().loadFromData((const uchar *)(_face->glyph->bitmap.buffer),_face->glyph->bitmap.width *  _face->glyph->bitmap.pitch);
// 		}
// 	}
// 	if(pix.count())
// 	{
// 		altBox ab(contextString, pix);
// 		ab.exec();
// 		return ab.value;
// 	}
// 	return 0;
// }
