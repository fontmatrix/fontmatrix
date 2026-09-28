/*
    SPDX-FileCopyrightText: 2010 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FAMILYWIDGET_H
#define FAMILYWIDGET_H

#include <QList>
#include <QModelIndex>
#include <QStringList>
#include <QWidget>

constexpr int FAMILY_VIEW_INFO = 0;
constexpr int FAMILY_VIEW_SAMPLE = 1;
constexpr int FAMILY_VIEW_CHART = 2;
constexpr int FAMILY_VIEW_ACTIVATION = 3;

class FMPreviewModel;
class FontItem;
class TagsWidget;
class FloatingWidget;

namespace Ui
{
class FamilyWidget;
}

class FamilyWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FamilyWidget(QWidget *parent = nullptr);
    ~FamilyWidget() override;

    void setFamily(const QString &f);
    TagsWidget *tagWidget();
    QString family;
    QString curVariant;
    int curInstance = -1; ///< the named instance of curVariant chosen in the list, -1 for none

protected:
    void changeEvent(QEvent *e) override;
    void keyPressEvent(QKeyEvent *event) override;

    void buildList(const QList<FontItem *> &fl);

private:
    Ui::FamilyWidget *const ui;
    FMPreviewModel *previewModel = nullptr;
    FloatingWidget *sample = nullptr;
    FloatingWidget *chart = nullptr;
    FloatingWidget *activation = nullptr;

    unsigned int currentIndex;
    QMetaObject::Connection variationConnection; ///< the list follows the instance the font is shown at
    void followVariation();
    unsigned int currentPage;
    QString uniBlock;

    void updateButtons();

Q_SIGNALS:
    void backToList();
    void fontSelected(const QString &path);
    void familyStateChanged();

private Q_SLOTS:
    void slotPreviewUpdateSize(int);
    void slotPreviewSelected(const QModelIndex &index);
    void slotShowInfo();
    void slotShowSample();
    void slotShowChart();
    void slotShowActivation();
    void slotDetachSample();
    void slotDetachChart();
    void slotStateChange();

private:
    void slotPreviewUpdate();
    //    void slotDetachActivation();
};

#endif // FAMILYWIDGET_H
