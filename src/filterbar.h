/*
    SPDX-FileCopyrightText: 2010 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FILTERBAR_H
#define FILTERBAR_H

#include <QAbstractListModel>
#include <QHBoxLayout>
#include <QList>
#include <QListView>
#include <QMap>
#include <QMenu>
#include <QMouseEvent>
#include <QStringListModel>
#include <QWidget>

class FiltersDialogItem;
class FilterItem;
class FilterData;

class TagListModel : public QAbstractListModel
{
    Q_OBJECT
    const int specialTagsCount;

    QStringList currentTags;

public:
    enum TagListRole {
        TagType = Qt::UserRole,
        TagString
    };

    explicit TagListModel(QObject *parent);
    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]] int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    [[nodiscard]] Qt::ItemFlags flags(const QModelIndex &index) const override;

    void clearCurrents();
    void addToCurrents(const QString &t);
    void removeFromCurrents(const QString &t);
    void renameCurrent(const QString &from, const QString &to);

public Q_SLOTS:
    void tagsDBChanged();
};

class TagListView : public QListView
{
    Q_OBJECT

    int m_andOrKey;

public:
    explicit TagListView(QWidget *parent)
        : QListView(parent)
        , m_andOrKey(0)
    {
    }

    int getAndKey()
    {
        int ret(m_andOrKey);
        m_andOrKey = 0;
        return ret;
    }

protected:
    void mouseReleaseEvent(QMouseEvent *event) override
    {
        if (event->modifiers().testFlag(Qt::ShiftModifier))
            m_andOrKey = 1;
        else if (event->modifiers().testFlag(Qt::ControlModifier))
            m_andOrKey = 2;
        else
            m_andOrKey = 0;
        QListView::mouseReleaseEvent(event);
    }
};

namespace Ui
{
class FilterBar;
}

class FilterBar : public QWidget
{
    Q_OBJECT

public:
    explicit FilterBar(QWidget *parent = nullptr);
    ~FilterBar() override;

    void setFilterListLayout(QHBoxLayout *l)
    {
        filterListLayout = l;
    }
    void setCurFilterWidget(QWidget *w)
    {
        curFilterWidget = w;
    }

protected:
    void changeEvent(QEvent *e) override;

private:
    Ui::FilterBar *const ui;
    QHBoxLayout *filterListLayout = nullptr;
    QWidget *curFilterWidget = nullptr;

    QList<FilterItem *> filters;
    void addFilterItem(FilterData *f, bool process = true);
    void removeAllFilters();
    TagListModel *tagListModel = nullptr;
    //    QMenu * metaFieldsMenu;
    int metaFieldKey;

    QString filterString(FilterData *d, bool first = false);
    void loadFilters();
    QList<FiltersDialogItem *> items;
    // Lazy-initialised translated strings; static-init i18n() would run before
    // KLocalizedString::setApplicationDomain() and fall back to source text.
    static const QString &andOp();
    static const QString &notOp();
    static const QString &orOp();

    QStringListModel *mModel = nullptr;
    QStringList mList;
    /// the languages and the licences of the collection, read when their section is first opened
    void fillLanguages();
    void fillLicenses();
    void fillDuplicates();
    void fillKinds();
    bool languagesFilled = false;
    bool licensesFilled = false;
    bool duplicatesFilled = false;
    bool kindsFilled = false;

Q_SIGNALS:
    void initSearch(int, QString);
    void filterChanged();

private Q_SLOTS:
    void processFilters();
    void slotPanoFilter();
    void metaFilter();
    void metaSelectField(int idx);

    void slotLoadFilter(const QString &fname);
    void slotRemoveFilter(const QString &fname);

    void slotTagSelect(const QModelIndex &index);
    void slotTagEdit(const QModelIndex &index);

    void slotToggleTags(bool t);
    void slotToggleMeta(bool t);
    void slotTogglePano(bool t);
    void slotToggleFilter(bool t);
    void slotToggleLanguages(bool t);
    void slotToggleLicense(bool t);
    void slotToggleDuplicates(bool t);
    void slotToggleKinds(bool t);

    void slotLangFilter(int index);
    void slotLicenseFilter(int index);
    void slotDuplicateFilter(int index);
    void slotKindFilter(int index);

private:
    void filtersDialog();
    void slotRemoveFilterItem(bool process = true);

public Q_SLOTS:
    void slotClearFilter();
    void slotSaveFilter();
    void slotSaveFilter(const QString &fname);

public:
    /// applies the filters again, or shows every font when there is none: for fonts added to the database
    void refilter();
    /// the languages and the licences are read again, the fonts of the database having changed
    void invalidateCoverage();
};

#endif // FILTERBAR_H
