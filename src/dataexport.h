/*
    SPDX-FileCopyrightText: 2008 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DATAEXPORT_H
#define DATAEXPORT_H

#include <QDir>
#include <QString>
#include <QWidget>

class FontItem;

namespace Ui
{
class DataExport;
}

/**
    @author Pierre Marchand <pierremarc@oep-h.com>

    This class is supposed to export a set of fonts in a
    directory and build a usefull index of the exported font files.
*/
class DataExport : public QWidget
{
    Q_OBJECT
public:
    explicit DataExport(QWidget *parent);
    //	DataExport(const QString &dirPath, const QString &filterTag);
    ~DataExport() override;

private:
    // data
    Ui::DataExport *const ui;
    QDir exDir;
    QString filter;
    QList<FontItem *> fonts;
    // methods
    int copyFiles();
    int buildIndex();
    int buildHtml();
    int buildTemplate(const QString &templateDirPath);

private Q_SLOTS:
    void doExport();
};

#endif
