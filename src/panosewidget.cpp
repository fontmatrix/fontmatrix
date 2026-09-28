/*
    SPDX-FileCopyrightText: 2009 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "panosewidget.h"
#include "fmfontstrings.h"
#include "fmpaths.h"
#include "panosemodel.h"
#include "ui_panosewidget.h"

#include "fmconfig.h"

#include <QColor>
#include <QDir>
#include <QIcon>
#include <QPalette>
#include <QTreeWidgetItem>

PanoseWidget::PanoseWidget(QWidget *parent)
    : QWidget(parent)
    , m_ui(new Ui::PanoseWidget)
{
    m_ui->setupUi(this);

    m_filter.clear();
    m_filterKey = 0;

    QPalette palette(m_ui->pTree->palette());
    palette.setBrush(QPalette::Base, Qt::transparent);
    m_ui->pTree->setPalette(palette);

    const QMap<FontStrings::PanoseKey, QMap<int, QString>> &p(FontStrings::Panose());
    QString defaultDir(FMPaths::ResourcesDir() + QLatin1String("Panose/Icons"));
    QString pDir(FMConfig::value(QStringLiteral("Panose/IconDir"), defaultDir).toString() + QDir::separator());

    for (const auto pKeys = p.keys(); const auto &k : pKeys) {
        QString fn(pDir + QString::number(k) + QDir::separator() + QLatin1String("attribute.png"));
        auto pItem(new QTreeWidgetItem(m_ui->pTree));

        pItem->setText(0, FontStrings::PanoseKeyName(k));
        pItem->setData(0, Qt::UserRole, k);
        if (QFile::exists(fn))
            pItem->setIcon(0, QIcon(fn));

        for (const auto pKeysList = p[k].keys(); const auto &v : pKeysList) {
            if (v > 1) // We do not want "Any" and "No Fit"
            {
                QString fn2(pDir + QString::number(k) + QDir::separator() + QString::number(v) + QLatin1String(".png"));

                auto item(new QTreeWidgetItem(pItem));
                item->setText(0, p.value(k).value(v));
                item->setData(0, Qt::UserRole, v);
                //				item->setForeground(0, QColor(qrand() % 255, qrand() % 255, qrand() % 255));
                if (QFile::exists(fn2))
                    item->setIcon(0, QIcon(fn2));
            }
        }
    }

    connect(m_ui->pTree, &QTreeWidget::itemClicked, this, &PanoseWidget::slotSelect);
}

PanoseWidget::~PanoseWidget()
{
    delete m_ui;
}

void PanoseWidget::doConnect(const bool &c)
{
    if (c) {
        connect(m_ui->pTree, &QTreeWidget::itemClicked, this, &PanoseWidget::slotSelect);
    } else {
        disconnect(m_ui->pTree, &QTreeWidget::itemClicked, this, &PanoseWidget::slotSelect);
    }
}

void PanoseWidget::slotSelect(QTreeWidgetItem *item, int column)
{
    if (column > 0)
        return;
    if (item->childCount() > 0)
        return;
    int pValue(item->parent()->data(0, Qt::UserRole).toInt());
    int cValue(item->data(0, Qt::UserRole).toInt());

    m_filter.clear();
    m_filter.insert(pValue, QList<int>() << cValue);

    Q_EMIT filterChanged();
}

void PanoseWidget::setFilter(const QMap<int, QList<int>> &filter)
{
    m_filter = filter;
}

// void PanoseWidget::changeEvent(QEvent *e)
//{
//     QWidget::changeEvent(e);
//     switch (e->type()) {
//     case QEvent::LanguageChange:
//         m_ui->retranslateUi(this);
//         break;
//     default:
//         break;
//     }
// }

void PanoseWidget::closeEvent(QCloseEvent *)
{
    hide();
}

#include "moc_panosewidget.cpp"
