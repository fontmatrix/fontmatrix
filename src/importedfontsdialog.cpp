/*
    SPDX-FileCopyrightText: 2007 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "importedfontsdialog.h"
#include "typotek.h"
#include <KLocalizedString>
#include <QListWidgetItem>

ImportedFontsDialog::ImportedFontsDialog(QWidget *parent, QStringList fontlist)
    : QDialog(parent)
{
    setupUi(this);
    // 	fontList->addItems(fontlist);
    int buggyFonts = 0;
    for (int i = 0; i < fontlist.count(); ++i) {
        QString s(fontlist[i]);
        bool success = true;
        if (s.startsWith("__FAILEDTOLOAD__", Qt::CaseSensitive)) {
            success = false;
            s = s.mid(16) + i18nc("@item:inlistbox appended to the name of a font that could not be loaded", " (not loaded)");
            ++buggyFonts;
        }
        auto it = new QListWidgetItem(s);
        it->setForeground(success ? Qt::black : Qt::red);
        fontList->addItem(it);
    }
    label->setText(i18nc("@label", "Number of Imported Fonts ") + QString::number(fontList->count() - buggyFonts));
    dontShowBox->setChecked(false);

    connect(dontShowBox, &QCheckBox::toggled, typotek::getInstance(), &typotek::setImportedFontsHidden);
}

ImportedFontsDialog::~ImportedFontsDialog()
{
    disconnect(dontShowBox, &QCheckBox::toggled, typotek::getInstance(), &typotek::setImportedFontsHidden);
}
