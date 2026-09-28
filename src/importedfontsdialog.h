/*
    SPDX-FileCopyrightText: 2007 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef IMPORTEDFONTSDIALOG_H
#define IMPORTEDFONTSDIALOG_H

#include <QDialog>
#include <QStringList>
#include <QWidget>
#include <ui_importedfonts.h>

/**
    @author Pierre Marchand <pierre@oep-h.com>
*/
class ImportedFontsDialog : public QDialog, private Ui::ImportedFonts
{
public:
    ImportedFontsDialog(QWidget *parent, QStringList fontlist);

    ~ImportedFontsDialog() override;
};

#endif
