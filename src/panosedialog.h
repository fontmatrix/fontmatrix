/*
    SPDX-FileCopyrightText: 2008 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PANOSEDIALOG_H
#define PANOSEDIALOG_H

#include "ui_panosedialog.h"

class FontItem;
class QComboBox;
class QLabel;

class FMPanoseDialog : public QDialog, private Ui::PanoseDialog
{
    Q_OBJECT
public:
    FMPanoseDialog(FontItem *font, QWidget *parent);
    ~FMPanoseDialog() override;

    [[nodiscard]] QString getSourcePanose() const
    {
        return m_sourcepanose;
    }
    [[nodiscard]] QString getTargetPanose() const
    {
        return m_targetpanose;
    }
    [[nodiscard]] bool getOk() const
    {
        return m_ok;
    }

private:
    FontItem *m_font = nullptr;
    QString m_sourcepanose;
    QString m_targetpanose;
    QMap<QString, QComboBox *> m_box;
    QMap<QString, QLabel *> m_label;
    bool m_ok;

    void populateDialog();

private Q_SLOTS:
    void panoseChange(int index);
    void closeOk();
    void closeCancel();
};

#endif // PANOSEDIALOG_H
