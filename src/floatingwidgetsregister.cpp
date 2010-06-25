/***************************************************************************
 *   Copyright (C) 2010 by Pierre Marchand   *
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

#include "floatingwidgetsregister.h"

#include "floatingwidget.h"

FloatingWidgetsRegister * FloatingWidgetsRegister::instance = 0;

FloatingWidgetsRegister::FloatingWidgetsRegister()
{
}

FloatingWidgetsRegister * FloatingWidgetsRegister::that()
{
	if(instance == 0)
		instance = new FloatingWidgetsRegister;
	return instance;
}


void FloatingWidgetsRegister::Register(FloatingWidget * f, const QString &fid, const QString &typ)
{
	FloatingWidgetsRegister *fwr(that());
	fwr->fwMap[typ][fid] = f;
}

FloatingWidget * FloatingWidgetsRegister::Widget(const QString &fid, const QString &typ)
{
	FloatingWidgetsRegister *fwr(that());
	if(fwr->fwMap.contains(typ))
	{
		if(fwr->fwMap[typ].contains(fid))
		{
			if(fwr->fwMap[typ][fid].isNull())
				fwr->fwMap[typ].remove(fid);
			else
				return fwr->fwMap[typ][fid];
		}
	}
	return 0;
}

QList<FloatingWidget*> FloatingWidgetsRegister::AllWidgets()
{
	QList<FloatingWidget*> ret;
	FloatingWidgetsRegister *fwr(that());
	ret.clear();
	foreach(QString t, fwr->fwMap.keys())
	{
		foreach(QString f, fwr->fwMap[t].keys())
		{
			if(fwr->fwMap[t][f].isNull())
				fwr->fwMap[t].remove(f);
			else
				ret << fwr->fwMap[t][f].data();
		}
	}
	return ret;
}

