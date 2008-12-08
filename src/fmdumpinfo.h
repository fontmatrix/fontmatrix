//
// C++ Interface: fmdumpinfo
//
// Description: 
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef FMDUMPINFO_H
#define FMDUMPINFO_H

#include <QString>
#include <QStringList>
#include <QMap>

class FontItem;

class FMDumpInfo
{
	public:
		FMDumpInfo(FontItem* font, const QString& model = QString());
		~FMDumpInfo();
		
		QStringList infos(){return m_info.keys();}
		QString info(const QString& k){return m_info[k];}
		void setModel(const QString& model){m_model = model;}
		bool dumpInfo(const QString& filepath);
		
		
	private:
		FontItem * m_font;
		QString m_model;
		
		QMap<QString, QString> m_info;
};

#endif // FMDUMPINFO_H
