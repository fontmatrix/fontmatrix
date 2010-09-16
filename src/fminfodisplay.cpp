//
// C++ Implementation: fminfodisplay
//
// Description:
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "fminfodisplay.h"

#include "fmencdata.h"
#include "fontitem.h"
#include "fmfontdb.h"
#include "fmfontstrings.h"
#include "fmpaths.h"
#include "glyphtosvghelper.h"
#include "typotek.h"

#include <QMap>
#include <QObject>
#include <QRegExp>
#include <QStringList>

#include <QFile>


FMInfoDisplay::FMInfoDisplay(FontItem * font)
{

	/**
	Selectors are :
#headline
#file
	.infoblock
	.infoname
	.langmatch
	.langundefined
	.langnomatch
	.encodingcurrent
	.encoding
	 */
	html = "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n";
	html += "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n";
	html += "<head>\n";
	html += "<title>" + xhtmlifies( font->fancyName() ) + "</title>\n";
	html += "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />\n";
	html += "<link rel=\"stylesheet\" href=\"" + QUrl::fromLocalFile(typotek::getInstance()->getInfoStyle()).toString() + "\" type=\"text/css\" />\n";
	html += "<script type=\"text/javascript\" src=\""+ QUrl::fromLocalFile(FMPaths::ResourcesDir() + "fontmatrix.js" ).toString() +" \" />\n";
	html += "</head>\n<body>\n";
	html += writeSVGPreview(font);
	html += "<div id=\"file\">" + xhtmlifies( font->path() ) + "</div>\n" ;
// 	ret += "<div id=\"search\"><a href=\"http://www.myfonts.com/search?search[text]="+ m_family +"\">On myfonts</a>";
	html += "<div id=\"general\">\n";
	html += writeOrderedInfo(font);
	html += writeFsType(font);
	html += "</div>\n"; // general
	html += writePanose(font);
	html += writeLangOS2(font);
	html += "</body>\n </html>\n";

#ifdef BUILD_TYPE_DEBUG
		QFile df("fontmatrix.xhtml" );
		if(df.open(QIODevice::WriteOnly | QIODevice::Truncate))
		{
			df.write(html.toUtf8());
		}
#endif
}


FMInfoDisplay::~FMInfoDisplay()
{
}

QString FMInfoDisplay::getHtml()
{
	return html;
}

QString FMInfoDisplay::writeFsType(FontItem * font)
{
	FontItem::FsType OSFsType(font->getFsType());
	QString embedFlags = "<div id=\"fstype\">";
	// 0 - 3 are exclusive
	if ( OSFsType == FontItem::NOT_RESTRICTED  )
		embedFlags+="<div><div class=\"fsname\">" + FontStrings::FsType( FontItem::NOT_RESTRICTED , true) 
				+ "</div><div class=\"fsdesc\">"+ FontStrings::FsType( FontItem::NOT_RESTRICTED , false) + "</div></div>\n";
	else if ( OSFsType & FontItem::RESTRICTED )
		embedFlags+="<div><div class=\"fsname\">" + FontStrings::FsType( FontItem::RESTRICTED , true) 
				+ "</div><div class=\"fsdesc\">"+ FontStrings::FsType( FontItem::RESTRICTED , false) + "</div></div>\n";
	else if ( OSFsType & FontItem::PREVIEW_PRINT  )
		embedFlags+="<div><div class=\"fsname\">" + FontStrings::FsType(FontItem::PREVIEW_PRINT , true) 
				+ "</div><div class=\"fsdesc\">" + FontStrings::FsType(FontItem::PREVIEW_PRINT , false) + "</div></div>\n";
	else if ( OSFsType & FontItem::EDIT_EMBED )
		embedFlags+="<div><div class=\"fsname\">" + FontStrings::FsType(FontItem::EDIT_EMBED , true) 
				+ "</div><div class=\"fsdesc\">" + FontStrings::FsType(FontItem::EDIT_EMBED , false) + "</div></div>\n";
	if ( OSFsType & FontItem::NOSUBSET  )
		embedFlags+="<div><div class=\"fsname\">" + FontStrings::FsType(FontItem::NOSUBSET , true) 
				+ "</div><div class=\"fsdesc\">" + FontStrings::FsType(FontItem::NOSUBSET , false) + "</div></div>\n";
	if ( OSFsType & FontItem::BITMAP_ONLY )
		embedFlags+="<div><div class=\"fsname\">" + FontStrings::FsType(FontItem::BITMAP_ONLY , true) 
				+ "</div><div class=\"fsdesc\">" + FontStrings::FsType(FontItem::BITMAP_ONLY , false) + "</div></div>\n";
	embedFlags +="</div>";
	
	return embedFlags;

}

QString FMInfoDisplay::writeLangOS2(FontItem * font)
{
	QString ret;
	QStringList llist(font->supportedLangDeclaration());
	if(llist.count() > 0)
	{
		ret += "<div id=\"langblock\">\n";
		ret += "\t<div class=\"langblockname\">" + QObject::tr("Unicode Ranges") + "</div>\n";
		ret += "\t<ul>\n";
		foreach(QString ln, llist)
		{
			ret += QString("\t\t<li>%1</li>\n").arg(ln);
		}
		ret += "\t</ul>\n";
		ret += "</div>\n";
		
	}
	return ret;
}



QString FMInfoDisplay::writeSVGPreview(FontItem * font)
{
	QString svg;
	QTransform tf;
	double pifs ( typotek::getInstance()->getPreviewInfoFontSize() );
	double scaleFactor( pifs / font->getUnitPerEm() );
	double maxHeight ( 0 );
	double vertOffset ( pifs );
	double horOffset ( 0 );
	tf.translate ( horOffset , vertOffset );

	foreach ( QChar c, font->fancyName() )
	{
		{
			QGraphicsPathItem * gpi ( font->itemFromChar ( c.unicode(), pifs ) );
			if ( gpi )
			{
				GlyphToSVGHelper gtsh ( gpi->path(), tf );
				svg += gtsh.getSVGPath() + "\n";
				horOffset += gpi->data(GLYPH_DATA_HADVANCE).toDouble() * scaleFactor;
				maxHeight = qMax ( gtsh.getRect().height(), maxHeight );
				tf.translate( gpi->data(GLYPH_DATA_HADVANCE).toDouble()  * scaleFactor,0 );
				delete gpi;
			}
		}
	}
	QString openElem ( QString ( "<div id=\"previewblock\"><svg width=\"%1px\" height=\"%2px\"  xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n" )
			.arg ( qRound(horOffset) )
			.arg ( qRound(maxHeight*1.6) ));
	
	
	return openElem + svg + "</svg></div>\n";
}

QString FMInfoDisplay::writeOrderedInfo(FontItem * font)
{
	QString ret;
	QMap<int, QStringList> orderedInfo;
	QString modelItem ( "<div class=\"infoblock\"><div class=\"infoname\"> %1 </div><div class=\"langundefined\"> %2 </div></div>\n" );
	QString fontType(font->type());
	if(fontType == QString("CFF"))
		fontType = QString("OpenType");

	ret += modelItem.arg(QObject::tr("File"))
	       .arg(font->path().replace("/","/&shy;"));
	ret += modelItem.arg( QObject::tr ( "Glyphs count" ))
	       .arg(QString::number ( font->glyphsCount() ));
	ret += modelItem.arg(QObject::tr ( "Font Type" ) )
	       .arg(fontType );


	QStringList cmapStrings;
	foreach ( FT_Encoding c, font->getCharsets() )
	{
		QString encString ( FontStrings::Encoding ( c ) );
		if ( ( c == FT_ENCODING_UNICODE ) && ( !font->getUnicodeBuiltIn() ) )
			encString +="*";
		if ( c == font->getCurrentEncoding() )
			cmapStrings << "<span class=\"encodingcurrent\">" + encString + "</span>\n";
		else
			cmapStrings << "<span class=\"encoding\">" + encString + "</span>\n";
	}
	ret += "<div class=\"infoblock\"><div class=\"infoname\">"+ QObject::tr ( "Charmaps List" ) +"</div><div class=\"langundefined\">"+ font->charmaps().join( ", " ) +"</div></div>\n";

	
// 	if ( !moreInfo.isEmpty() ) // moreInfo.isNotEmpty
	{
		QString sysLang = QLocale::languageToString ( QLocale::system ().language() ).toUpper();
		QString sysCountry = QLocale::countryToString ( QLocale::system ().country() ).toUpper();
		QString sysLoc = sysLang + "_"+ sysCountry;

		//We must iter once to find localized strings and ensure default ones are _not_ shown in these cases
		QList<int> localizedKeys;
		FontInfoMap moreInfo ( FMFontDb::DB()->getInfoMap ( font->path() ) );
		for ( QMap<int, QMap<int, QString> >::const_iterator lit = moreInfo.begin(); lit != moreInfo.end(); ++lit )
		{
			for ( QMap<int, QString>::const_iterator mit = lit.value().begin(); mit != lit.value().end(); ++mit )
			{
				if ( FMEncData::LangIdMap()[ lit.key() ].contains ( sysLang ) )
				{
					localizedKeys << mit.key();
				}
			}
		}

//		QString styleLangMatch("\"langundefined\"");
		for ( QMap<int, QMap<int, QString> >::const_iterator lit = moreInfo.begin(); lit != moreInfo.end(); ++lit )
		{
//			if ( FMEncData::LangIdMap()[ lit.key() ].contains ( sysLang ) ) // lang match
//			{
//				styleLangMatch = "\"langmatch\"";
//			}
//			else if ( FMEncData::LangIdMap()[ lit.key() ] == "DEFAULT" ) // lang does not match but it’s international name
//			{
//				styleLangMatch = "\"langundefined\"";
//			}
//			else // lang does not match at all
//			{
//				styleLangMatch = "\"langnomatch\"";
//			}
			for ( QMap<int, QString>::const_iterator mit = lit.value().begin(); mit != lit.value().end(); ++mit )
			{
//				if ( FMEncData::LangIdMap()[ lit.key() ].contains ( sysLang ) )
				{
					QString name_value(url2href(xhtmlifies(mit.value()))); // compact coding :)
					name_value.replace ( "\n","<br/>" );
					QString dcname (name_value);
//					if ( !orderedInfo[ mit.key() ].contains ( dcname ) )
					if(!orderedInfo.contains(mit.key()))
						orderedInfo[ mit.key() ] << dcname;
				}
//				else if ( FMEncData::LangIdMap()[ lit.key() ] == "DEFAULT" && !localizedKeys.contains ( mit.key() ) )
//				{
//					QString name_value(url2href(xhtmlifies(mit.value())));
//					name_value.replace ( "\n","<br/>" );
//					QString dcname ( "<div class="+ styleLangMatch +">" +  name_value +"</div>\n" );
//					if ( !orderedInfo[ mit.key() ].contains ( dcname ) )
//						orderedInfo[ mit.key() ] << dcname;
//				}
			}
		}



	}
	
	/// Times to manually order presentation!
	
	QList<FMFontDb::InfoItem> order;
	order 	<< FMFontDb::FontFamily
			<< FMFontDb::FontSubfamily
			<< FMFontDb::FullFontName
			<< FMFontDb::PostscriptName
			<< FMFontDb::Designer
			<< FMFontDb::Description
			<< FMFontDb::LicenseDescription
			<< FMFontDb::Copyright
			<< FMFontDb::PostScriptCIDName
			<< FMFontDb::SampleText
			<< FMFontDb::VersionString
			<< FMFontDb::Trademark
			<< FMFontDb::ManufacturerName
			<< FMFontDb::URLVendor
			<< FMFontDb::URLDesigner
			<< FMFontDb::LicenseInfoURL
			<< FMFontDb::PreferredFamily
			<< FMFontDb::PreferredSubfamily
			<< FMFontDb::CompatibleMacintosh
			<< FMFontDb::UniqueFontIdentifier;
	
	QMap<FMFontDb::InfoItem, QString> tNames(FontStrings::Names());
	foreach(FMFontDb::InfoItem key, order)
	{
		if (orderedInfo.contains(key))
			ret += modelItem.arg(tNames.value(key))
					.arg(orderedInfo[key].join(" "));
	}
	
	return ret;
}

QString FMInfoDisplay::writePanose(FontItem * font)
{
	QString panBlockOut;
	QString panoseLabel("<div id=\"panoselabel\">Panose</div>");
	QString pN ( FMFontDb::DB()->getValue ( font->path(), FMFontDb::Panose, false ).toString() );
	if ( !pN.isEmpty() )
	{
		QStringList pl ( pN.split ( ":" ) );
		if ( pl.count() == 10 )
		{
			for ( int i ( 0 );  i < FontStrings::Panose().keys().count(); ++i )
			{
				FontStrings::PanoseKey k ( FontStrings::Panose().keys() [i] );
				int pValue ( pl[i].toInt() );
				panBlockOut += "<div class=\"panose_name\">" + FontStrings::PanoseKeyName ( k ) + "</div>\n";
				panBlockOut += "<div class=\"panose_desc\">" + FontStrings::Panose().value ( k ).value ( pValue )/* + " - "+ pl[i]*/ +"</div>\n";
			}
		}
	}
	
	return "<div id=\"panose_block\">" + panoseLabel + panBlockOut + "</div>\n";
}



/**
 * Make HTTP links out of url in a text string
 */
QString FMInfoDisplay::url2href (QString value )
{
	QString punctuationAfter = "\\.\\,;:!?)�\"'";
	value.replace ( QRegExp ( "([^/])(www\\.[\\w\\d])" ), "\\1http://\\2" ); // add an http to www. without it
	QRegExp rx = QRegExp("(http[s]?://\\S+)(["+punctuationAfter+"](?:\\s|$))"); // prepare a regexp to 
	rx.setMinimal(true);
	value.replace(rx, "\\1 \\2"); // add a space before  punctuation "attached" to url
	value.replace(rx, "\\1 \\2"); // run the prepared regexp twice for ")."
	value.replace ( QRegExp ( "(http[s]?://\\S+)[\\.]?" ), "<a href=\"\\1\">\\1</a>" ); // Make HTTP links
	value.replace ( QRegExp ( "(</a>)\\s(["+punctuationAfter+"])" ), "\\1\\2" ); // remove extra space after </a>
	return value;
} // url2href

QString FMInfoDisplay::xhtmlifies(const QString& value)
{
	QMap<int, QString> pattern;
	pattern[0x22] = "&#34;"; // "
	pattern[0x26] = "&#38;"; // &
	pattern[0x27] = "&#39;"; // '
	pattern[0x3c] = "&#60;"; // <
	pattern[0x3e] = "&#62;"; // >
	
	QString ret;
	int len(value.length());
	for(int i(0); i<len; ++i)
	{
		if( pattern.contains(value[i].unicode()) )
		{
			ret += pattern[value[i].unicode()];
		}
		else
			ret += value[i];
	}
	return ret;
}

