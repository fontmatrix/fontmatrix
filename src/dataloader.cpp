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
#include "dataloader.h"
#include "fmpaths.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QLocale>

DataLoader::DataLoader ()
{
	load();
}

void DataLoader::reload()
{
	load();
}

void DataLoader::load()
{
	sm.clear();
	pm.clear();
	// First we load system samples
	QDir samplesDir(FMPaths::ResourcesDir() + "Samples" );
	foreach(QString ld,
		samplesDir.entryList(QDir::NoDotAndDotDot | QDir::AllDirs) )
	{
		QDir lang(samplesDir.absoluteFilePath(ld));
		QLocale locale(ld);
		QString loclang(QLocale::languageToString(locale.language()));
		qDebug()<<ld<<loclang;
		foreach(QString st,
			lang.entryList(QDir::NoDotAndDotDot | QDir::NoSymLinks | QDir::Files) )
		{
			QFile fp(lang.absoluteFilePath(st));
			if(fp.open(QIODevice::ReadOnly))
			{
				sm[loclang][st] = QString::fromUtf8(fp.readAll());
			}
		}
	}

	// Then personals
	QDir uDir(FMPaths::SamplesDir());
	if(!uDir.exists())
	{
		qDebug()<<"Create Directory:"<<uDir.absolutePath();
		uDir.mkpath(uDir.absolutePath());
	}
	else
	{
		foreach(QString ld, uDir.entryList(QDir::NoDotAndDotDot | QDir::NoSymLinks | QDir::Files) )
		{
			QFile fp(uDir.absoluteFilePath(ld));
			if(fp.open(QIODevice::ReadOnly))
			{
				pm[ld] = QString::fromUtf8(fp.readAll());
			}
		}
	}

	// Emergency !!
	if(sm.isEmpty() && pm.isEmpty())
	{
		sm["Emergency"]["Text"] = QString("Emergency Text");
	}
}

// TODO
bool DataLoader::update(const QString& name, const QString& sample)
{
	qDebug()<<"DataLoader::update"<<name<<sample;
	QDir uDir(FMPaths::SamplesDir());
	QFile fp(uDir.absoluteFilePath(name));
	if(fp.open(QIODevice::WriteOnly | QIODevice::Truncate))
	{
		if(fp.write(sample.toUtf8()) == sample.toUtf8().count())
		{
			pm[name] = sample;
			return true;
		}
	}
	return false;

}

bool DataLoader::remove(const QString& name)
{
	QDir uDir(FMPaths::SamplesDir());
	QFile fp(uDir.absoluteFilePath(name));
	if(fp.exists())
	{
		if(fp.remove())
		{
			pm.remove(name);
			return true;
		}
	}
	return false;
}
