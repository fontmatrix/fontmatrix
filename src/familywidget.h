/***************************************************************************
 *   Copyright (C) 2010 by Pierre Marchand   *
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

#ifndef FAMILYWIDGET_H
#define FAMILYWIDGET_H

#include <QWidget>
#include <QList>
#include <QModelIndex>
#include <QStringList>

class FMPreviewModel;
class FontItem;
class TagsWidget;
class QWebView;
//class SampleWidget;
//class ChartWidget;
class FloatingWidget;

namespace Ui {
    class FamilyWidget;
}

class FamilyWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FamilyWidget(QWidget *parent = 0);
    ~FamilyWidget();

    void setFamily(const QString& f, unsigned int curIdx = 0);
    TagsWidget* tagWidget();
    QWebView * info();
    QString family;
    QString curVariant;

protected:
    void changeEvent(QEvent *e);

    void buildList(const QList<FontItem*>& fl);

private:
    Ui::FamilyWidget *ui;
    FMPreviewModel * previewModel;
    FloatingWidget *sample;
    FloatingWidget *chart;

    QList<QStringList> variants;
    void appendVariants(const QString& w, const QString& s, const QString& wi, const QString& o);
    QList<FontItem*> orderVariants(QList<FontItem*> ul);
    inline bool compareVariants(const QStringList& a, const QStringList& b);

signals:
    void backToList();
    void fontSelected(const QString& path);
    void familyStateChanged();

private slots:
    void slotPreviewUpdate();
    void slotPreviewUpdateSize(int);
    void slotPreviewSelected(const QModelIndex & index);
    void slotShowInfo();
    void slotShowSample();
    void slotShowChart();
    void slotDetachSample();
    void slotDetachChart();
    void slotActivate(bool c);
    void slotDeactivate(bool c);

};

#endif // FAMILYWIDGET_H
