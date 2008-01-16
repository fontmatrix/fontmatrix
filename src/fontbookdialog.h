/***************************************************************************
 *   Copyright (C) 2007 by Pierre Marchand   *
 *   pierre@oep-h.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef FONTBOOKDIALOG_H
#define FONTBOOKDIALOG_H

#include <ui_bookexport.h>
#include <QDialog>
#include <QString>
#include <QSizeF>
#include <QRectF>
#include <QPrinter>
#include <QDomDocument>

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
		FontBookDialog ( QWidget *parent = 0 );

		~FontBookDialog();

// 		void setSampleText ( QString s);
// 		void setSampleHeadline(QString s);
// 		QString getSampleText();
// 		QString getSampleHeadline();
// 		double getTabFamily();
// 		double getTabStyle();
// 		double getTabSampleText();
		QString getFileName();
// 		QSizeF getPageSize();
// 		QPrinter::PageSize getPageSizeConstant();
// 		double getFontSize(QString s);
		bool isOk;
		bool isTemplate(){return m_isTemplate;}
		QDomDocument getTemplate(){return m_template;}
		
	private slots:
		void slotAccept();
		void slotCancel();
		void slotFileDialog();
// 		void slotPageSize(int index);
// 		void slotPreview();
		void slotLoadTemplate(const QString &theTemplate);
		void slotPreviewTemplate(const QString &key);
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
		QGraphicsScene *templateScene;
		QMap<QString,QString> templatesMap;
		QMap<QString,QPixmap> templatesPreviewMap;
		QGraphicsPixmapItem *curTemplatePreview;
		
};

#endif
