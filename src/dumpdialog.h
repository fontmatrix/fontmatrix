/*
    SPDX-FileCopyrightText: 2008 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DUMPDIALOG_H
#define DUMPDIALOG_H

#include "ui_dumpdialog.h"

class FontItem;
class FMDumpInfo;

class FMDumpDialog : public QDialog, private Ui::DumpDialog
{
    Q_OBJECT
public:
    FMDumpDialog(FontItem *font, QWidget *parent);
    ~FMDumpDialog() override;

    [[nodiscard]] QString getModel() const;
    [[nodiscard]] QString getFilePath() const;

private:
    FMDumpInfo *m_dumpinfo = nullptr;

private Q_SLOTS:
    void slotDumpIt();
    void browseFile();
    void browseModel();

    void insertSelectedField();
};

#endif // DUMPDIALOG_H
