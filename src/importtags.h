/*
    SPDX-FileCopyrightText: 2008 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef IMPORTTAGS_H
#define IMPORTTAGS_H

#include <QDialog>
#include <ui_importtags.h>

/**
    @author Pierre Marchand <pierremarc@oep-h.com>
*/
class ImportTags : public QDialog, private Ui::ImportTagsDialog
{
    Q_OBJECT
public:
    ImportTags(QWidget *parent, QStringList tags);
    ~ImportTags() override;

    QStringList tags()
    {
        return m_tags;
    }

private:
    QStringList m_tags;

private Q_SLOTS:
    void slotNewTag();
    void slotEnd();
};

#endif
