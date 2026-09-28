/*
    SPDX-FileCopyrightText: 2009 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FMFONTEXTRACT_H
#define FMFONTEXTRACT_H

#include <QMap>

#include "fmfontextractorbase.h"
#include "ui_fontextractordialog.h"

class FMFontExtract : public QDialog, private Ui::FontExtractorDialog
{
    Q_OBJECT

public:
    explicit FMFontExtract(QWidget *parent);
    ~FMFontExtract() override;

private:
    QMap<QString, FMFontExtractorBase *> extractors;
    FMFontExtractorBase *currentExtractor = nullptr;

    void loadDoc(const QString &path);
    QString lastPath;
    QString lastDir;

private Q_SLOTS:
    void slotBrowseDoc();
    void slotBrowseDir();
    void slotExtract();
};

#endif // FMFONTEXTRACT_H
