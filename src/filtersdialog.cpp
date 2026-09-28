/*
    SPDX-FileCopyrightText: 2010 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "filtersdialog.h"
#include "ui_filtersdialog.h"

#include "filteritem.h"
#include "filtermeta.h"
#include "filterpanose.h"
#include "filtersdialogitem.h"
#include "filtertag.h"
#include "fmpaths.h"

#include <KLocalizedString>
#include <QDir>
#include <QFile>

const QString &FiltersDialog::andOp()
{
    static const QString s = i18nc("@item:intext filter operator", "And");
    return s;
}
const QString &FiltersDialog::notOp()
{
    static const QString s = i18nc("@item:intext filter operator", "Not");
    return s;
}
const QString &FiltersDialog::orOp()
{
    static const QString s = i18nc("@item:intext filter operator", "Or");
    return s;
}

FiltersDialog::FiltersDialog(const QList<FilterItem *> &currentFilters, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FiltersDialog)
{
    setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);

    loadFilters();

    connect(ui->addButton, &QPushButton::clicked, this, &FiltersDialog::slotAddFilter);
    //	connect(ui->moreButton, SIGNAL(clicked(bool)), this, SLOT(showAdd(bool)));

    if (!currentFilters.isEmpty()) {
        QString fs;
        bool first(true);
        for (auto *f : currentFilters) {
            FilterData *d(f->filter());
            if (first) {
                first = false;
                fs += filterString(d, true);
            } else
                fs += filterString(d);
        }
        ui->curFilter->setText(fs);
        ui->messageStack->setCurrentWidget(ui->pagePossible);
    } else {
        ui->messageStack->setCurrentWidget(ui->pageImpossible);
    }
}

FiltersDialog::~FiltersDialog()
{
    delete ui;
}

QString FiltersDialog::filterString(FilterData *d, bool first)
{
    QString fs;
    if (first) {
        if (d->data(FilterData::Not).toBool())
            fs += notOp() + QString(" [%1] ").arg(d->getText());
        else
            fs += QString("[%1] ").arg(d->getText());
    } else {
        if (d->data(FilterData::Or).toBool())
            fs += orOp();
        else
            fs += andOp();

        if (d->data(FilterData::Not).toBool())
            fs += QString(" %1").arg(notOp());
        fs += QString(" [%1] ").arg(d->getText());
    }
    return fs;
}

void FiltersDialog::loadFilters()
{
    for (auto *i : std::as_const(items))
        delete i;
    items.clear();

    QDir fbasedir(FMPaths::FiltersDir());
    QStringList fbaselist(fbasedir.entryList(QDir::NoDotAndDotDot | QDir::Dirs, QDir::Name));
    for (const auto &fname : std::as_const(fbaselist)) {
        QDir fdir(FMPaths::FiltersDir() + fname);
        QStringList flist(fdir.entryList(QDir::NoDotAndDotDot | QDir::Files, QDir::Name));
        QString fString;
        bool first(true);
        for (const auto &fn : std::as_const(flist)) {
            QStringList l(fn.split(QString("-")));
            if (l.count() == 2) {
                QString type(l.at(1));
                QFile file(fdir.absoluteFilePath(fn));
                if (file.open(QIODevice::ReadOnly)) {
                    FilterData *f = nullptr;
                    if (type == QString("Meta")) {
                        f = new FilterMeta;
                    } else if (type == QString("Panose")) {
                        f = new FilterPanose;
                    } else if (type == QString("Tag")) {
                        f = new FilterTag;
                    }
                    if (!f)
                        continue;
                    f->fromByteArray(file.readAll());
                    if (first) {
                        first = false;
                        fString += filterString(f, true);
                    } else
                        fString += filterString(f);
                    delete f;
                }
            }
        }
        auto fdi(new FiltersDialogItem(fname, fString, this));
        items.append(fdi);
        ui->filtersLayout->addWidget(fdi);
        connect(fdi, &FiltersDialogItem::Filter, this, &FiltersDialog::Filter);
        connect(fdi, &FiltersDialogItem::Filter, this, &FiltersDialog::close);
        connect(fdi, &FiltersDialogItem::Remove, this, &FiltersDialog::slotRemoveFilter);
    }
}

void FiltersDialog::slotAddFilter()
{
    QString fname(ui->newName->text());
    if (!fname.isEmpty())
        Q_EMIT AddFilter(fname);

    loadFilters();
}

void FiltersDialog::slotRemoveFilter(QString fs)
{
    Q_EMIT RemoveFilter(fs);
    loadFilters();
}

#include "moc_filtersdialog.cpp"
