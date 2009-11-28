/***************************************************************************
 *   Copyright (C) 2009 by Pierre Marchand   *
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

#ifndef FMMISSINGFONTHELPER_H
#define FMMISSINGFONTHELPER_H

#include <QObject>
#include <QString>
#include <QStringList>

/**
  Let's try to nicely handle missing font files.
  At first, just popup the "check database" dialog.
*/
class FMMissingFontHelper : public QObject
{
    FMMissingFontHelper(){}
public:
    FMMissingFontHelper(const QString& ff);
    FMMissingFontHelper(const QStringList& ff);

};

#endif // FMMISSINGFONTHELPER_H
