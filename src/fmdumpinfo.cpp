//
// C++ Implementation: fmdumpinfo
//
// Description: 
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "fmdumpinfo.h"

#include "fontitem.h"
#include "fmfontdb.h"

#include <QFile>
#include <QTextStream>

FMDumpInfo::FMDumpInfo(FontItem * font, const QString & model)
	:m_font(font), m_model(model)
{
	m_info["Family"] = m_font->family();
	m_info["Variant"] = m_font->variant();

	QMap<int, QString> m_name;
	m_name[FMFontDb::Copyright]= "Copyright" ;
	m_name[FMFontDb::FontFamily]=  "Font_Family"  ;
	m_name[FMFontDb::FontSubfamily]=   "Font_Subfamily"  ;
	m_name[FMFontDb::UniqueFontIdentifier]=   "Unique_font_identifier"  ;
	m_name[FMFontDb::FullFontName]=   "Full_font_name"  ;
	m_name[FMFontDb::VersionString]=   "Version_string"  ;
	m_name[FMFontDb::PostscriptName]=   "Postscript_name"  ;
	m_name[FMFontDb::Trademark]=  "Trademark"  ;
	m_name[FMFontDb::ManufacturerName]=   "Manufacturer"  ;
	m_name[FMFontDb::Designer]=   "Designer"  ;
	m_name[FMFontDb::Description]=   "Description"  ;
	m_name[FMFontDb::URLVendor]=   "URL_Vendor"  ;
	m_name[FMFontDb::URLDesigner]=   "URL_Designer"  ;
	m_name[FMFontDb::LicenseDescription]=   "License_Description"  ;
	m_name[FMFontDb::LicenseInfoURL]=   "License_Info_URL"  ;
	m_name[FMFontDb::PreferredFamily]=   "Preferred_Family"  ;
	m_name[FMFontDb::PreferredSubfamily]=   "Preferred_Subfamily" ;
	m_name[FMFontDb::CompatibleMacintosh]=   "Compatible_Full"  ;
	m_name[FMFontDb::SampleText]=   "Sample_text"  ;
	m_name[FMFontDb::PostScriptCIDName]=  "PostScript_CID"  ;
	FontInfoMap fim(m_font->rawInfo());
	
	/*
	hierarchy ;-)
	langIdMap[0x0000]="DEFAULT";
	langIdMap[0x0009]="ENGLISH_GENERAL";
	langIdMap[0x0409]="ENGLISH_UNITED_STATES";
	langIdMap[0x0809]="ENGLISH_UNITED_KINGDOM";
	*/
	QList<int> llist;
	llist << 0x0000 << 0x0009 << 0x0409 << 0x0809;
	foreach(int langid, llist)
	{
		if( fim.contains(langid) ) // DEFAULT - generally means english in fact, which is good for our purpose.
		{
			foreach(int key, fim[langid].keys())
			{
				if( !m_info.contains(m_name[key]) )
					m_info[m_name[key]] = fim[langid][key];
			}
		}
	}

	
}

FMDumpInfo::~ FMDumpInfo()
{
}

bool FMDumpInfo::dumpInfo(const QString& filepath)
{
	QFile file(filepath);
	if(!file.open(QIODevice::WriteOnly))
	{
		return false;
	}
	QTextStream ts(&file);
	QString re(m_model);
	
	foreach(QString key, m_info.keys())
	{
		re.replace("${"+key+"}", m_info[key]);
	}
	ts << re;
	file.close();
        return true;
}















