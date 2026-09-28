/*
    SPDX-FileCopyrightText: 2007 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FONTBOOKDIALOG_H
#define FONTBOOKDIALOG_H

#include <QDialog>
#include <QDomDocument>
#include <QPrinter>
#include <QRectF>
#include <QSizeF>
#include <QString>
#include <ui_bookexport.h>

class QGraphicsScene;
class QGraphicsPixmapItem;
class FontItem;
/**
    @author Pierre Marchand <pierre@oep-h.com>
*/
class FontBookDialog : public QDialog, private Ui::BookExportDialog
{
    Q_OBJECT
public:
    explicit FontBookDialog(QWidget *parent = nullptr);

    ~FontBookDialog() override;

    // 		void setSampleText ( QString s);
    // 		void setSampleHeadline(QString s);
    // 		QString getSampleText();
    // 		QString getSampleHeadline();
    // 		double getTabFamily();
    // 		double getTabStyle();
    // 		double getTabSampleText();
    QString getFileName();
    bool isOk;
    bool isTemplate()
    {
        return m_isTemplate;
    }
    QDomDocument getTemplate()
    {
        return m_template;
    }

private Q_SLOTS:
    void slotAccept();
    void slotCancel();
    void slotFileDialog();
    void slotPreviewTemplate(const QString &key);

private:
    // 		void slotPageSize(int index);
    // 		void slotPreview();
    void slotLoadTemplate(const QString &theTemplate);
    // 	signals:
    // 		void updateView();
private:
    QDomDocument m_template;
    bool m_isTemplate;
    // 		void fillSizeList();
    void fillFontsList();
    void fillTemplates();
    // 		QSizeF m_pageSize;
    // 		QPrinter::PageSize m_pageSizeConstant;
    // 		QRectF m_pageRect;
    // 		QGraphicsScene *preScene;
    // 		QList<FontItem*> renderedFont;
    QGraphicsScene *templateScene = nullptr;
    QMap<QString, QString> templatesMap;
    QMap<QString, QPixmap> templatesPreviewMap;
    QGraphicsPixmapItem *curTemplatePreview = nullptr;
};

#endif
