/*
    SPDX-FileCopyrightText: 2009 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "fmmissingfonthelper.h"
#include "fmrepair.h"
#include "typotek.h"
#include <KLocalizedString>
#include <KMessageBox>

FMMissingFontHelper::FMMissingFontHelper(const QString &ff)
{
    typotek *t = typotek::getInstance();
    KMessageBox::error(t,
                       i18nc("@info", "Fontmatrix has been unable to load the font in file \n%1.\n Please check missing files.", ff),
                       i18nc("@title:window", "Missing Font File"));
    FmRepair repair(t);
    repair.exec();
}

FMMissingFontHelper::FMMissingFontHelper(const QStringList &ff)
{
    typotek *t = typotek::getInstance();
    KMessageBox::error(t,
                       i18nc("@info", "Fontmatrix has been unable to load fonts in files \n%1.\n Please check missing files.", ff.join("\n")),
                       i18nc("@title:window", "Missing Font File"));
    FmRepair repair(t);
    repair.exec();
}
