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

#include "fmmissingfonthelper.h"
#include "fmrepair.h"
#include "typotek.h"
#include <QMessageBox>

FMMissingFontHelper::FMMissingFontHelper(const QString& ff)
{
    typotek * t = typotek::getInstance();
    QMessageBox::warning( t,
                          tr("Missing Font File") ,
                          tr("Fontmatrix has been unable to load the font in file \n%1.\n Please check missing files.").arg(ff),
                          QMessageBox::Ok,
                          QMessageBox::NoButton);
    FmRepair repair(t);
    repair.exec();
}

FMMissingFontHelper::FMMissingFontHelper(const QStringList& ff)
{
    typotek * t = typotek::getInstance();
    QMessageBox::warning( t,
                          tr("Missing Font File") ,
                          tr("Fontmatrix has been unable to load fonts in files \n%1.\n Please check missing files.").arg(ff.join("\n")),
                          QMessageBox::Ok,
                          QMessageBox::NoButton);
    FmRepair repair(t);
    repair.exec();
}
