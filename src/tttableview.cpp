/*
    SPDX-FileCopyrightText: 2008 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <KLocalizedString>
#include <QDebug>
#include <QFile>
#include <QFileDialog>

#include "tttableview.h"

#include "fmfontstrings.h"
#include "fontitem.h"

TTTableView::TTTableView(FontItem *font, QWidget *parent)
    : QWidget(parent)
    , m_font(font)
{
    setupUi(this);
    // 	tView->setColumnCount (3);
    bool hasTable(false);
    QTreeWidgetItem *first;
    for (const auto keysList = FontStrings::Tables().keys(); const auto &tname : keysList) {
        int len(font->table(tname));
        if (len > 0) {
            auto twi(new QTreeWidgetItem);
            twi->setText(NAME, tname);
            twi->setText(DESCRIPTION, FontStrings::Tables()[tname]);
            twi->setText(SIZE, QString::number(len));
            twiList << twi;
            tView->addTopLevelItem(twi);
            if (!hasTable) {
                hasTable = true;
                first = twi;
            }
        }
        // 		else
        // 			qDebug()<<tname<<len;
    }
    tView->resizeColumnToContents(DESCRIPTION);

    connect(tView, &QTreeWidget::itemSelectionChanged, this, &TTTableView::updateHexView);
    connect(exportButton, &QPushButton::clicked, this, &TTTableView::exportHex);

    if (hasTable) {
        first->setSelected(true);
        updateHexView();
    }
}

TTTableView::~TTTableView()
{
    for (auto *twi : std::as_const(twiList)) {
        delete twi;
    }
}

void TTTableView::updateHexView()
{
    if (tView->selectedItems().isEmpty())
        return;

    QString table(tView->selectedItems()[0]->text(NAME));
    curTable = m_font->tableData(table);

    m_data.clear();
    for (int i(0); i < curTable.size(); ++i) {
        m_data << curTable.at(i);
    }
    hexView->setData(&m_data);
}

void TTTableView::exportHex()
{
    if (curTable.isEmpty())
        return;

    QString fileName = QFileDialog::getSaveFileName(this, i18nc("@title:window", "Save File"));
    if (fileName.isEmpty())
        return;
    QFile f(fileName);
    if (f.open(QIODevice::WriteOnly)) {
        f.seek(0);

        f.write(curTable);
    }
}

#include "moc_tttableview.cpp"
