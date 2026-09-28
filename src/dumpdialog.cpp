/*
    SPDX-FileCopyrightText: 2008 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "dumpdialog.h"
#include "fmdumpinfo.h"
#include "fontitem.h"

#include <QFile>
#include <QFileDialog>
#include <QTextStream>

FMDumpDialog::FMDumpDialog(FontItem *font, QWidget *parent)
    : QDialog(parent)
    , m_dumpinfo(nullptr)
{
    setupUi(this);
    fontName->setText(font->fancyName());
    m_dumpinfo = new FMDumpInfo(font);
    nameList->addItems(m_dumpinfo->infos());

    connect(browseButton, &QPushButton::clicked, this, &FMDumpDialog::browseFile);
    connect(loadButton, &QPushButton::clicked, this, &FMDumpDialog::browseModel);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &FMDumpDialog::slotDumpIt);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &FMDumpDialog::reject);

    connect(modelText, &ModelText::insertContent, this, &FMDumpDialog::insertSelectedField);
}

FMDumpDialog::~FMDumpDialog()
{
    if (m_dumpinfo)
        delete m_dumpinfo;
}

QString FMDumpDialog::getModel() const
{
    return modelText->toPlainText();
}

QString FMDumpDialog::getFilePath() const
{
    return filePath->text();
}

void FMDumpDialog::slotDumpIt()
{
    m_dumpinfo->setModel(getModel());

    if (m_dumpinfo->dumpInfo(getFilePath()))
        accept();
    else
        reject();
}

void FMDumpDialog::browseFile()
{
    QString s(QFileDialog::getSaveFileName(this, QStringLiteral("Fontmatrix"), QDir::homePath()));
    if (!s.isEmpty()) {
        filePath->setText(s);
    }
}

void FMDumpDialog::browseModel()
{
    QString s(QFileDialog::getOpenFileName(this, QStringLiteral("Fontmatrix"), QDir::homePath()));
    if (!s.isEmpty()) {
        QFile file(s);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream ts(&file);
            modelText->setPlainText(ts.readAll());
        }
        file.close();
    } else
        modelText->setPlainText(QLatin1String(""));
}

void FMDumpDialog::insertSelectedField()
{
    if (!nameList->selectedItems().isEmpty()) {
        QTextCursor cursor = modelText->textCursor();
        cursor.insertText(m_dumpinfo->info(nameList->selectedItems().first()->text()));
    }
}

#include "moc_dumpdialog.cpp"
