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
#include "importedfontsdialog.h"
#include <QListWidgetItem>

ImportedFontsDialog::ImportedFontsDialog(QStringList fontlist)
 : QDialog()
{
	setupUi(this);
// 	fontList->addItems(fontlist);
	int buggyFonts = 0;
	for(int i=0; i < fontlist.count();++i)
	{
		QString s(fontlist[i]);
		bool success = true;
		if(s.startsWith("__FAILEDTOLOAD__", Qt::CaseSensitive))
		{
			success = false;
			s = s.mid(16) + tr(" (not loaded)");
			++buggyFonts;
		}
		QListWidgetItem *it=new QListWidgetItem(s);
		it->setTextColor(success ? Qt::black : Qt::red);
		fontList->addItem(it);
	}
	label->setText(QString("Number of Imported Fonts ") + QString::number(fontList->count() - buggyFonts));
// 	exec();
}


ImportedFontsDialog::~ImportedFontsDialog()
{
}


