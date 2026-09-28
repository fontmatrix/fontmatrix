/*
    SPDX-FileCopyrightText: 2010 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "metawidget.h"
#include "fmfontstrings.h"
#include "ui_metawidget.h"

#include <KLocalizedString>
#include <QComboBox>
#include <QCompleter>
#include <QDebug>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QStringListModel>
#include <QVariant>

QStringListModel *MetaWidget::mModel = nullptr;
QStringList MetaWidget::mList = QStringList();

MetaWidget::MetaWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MetaWidget)
{
    ui->setupUi(this);
    if (mModel == nullptr) {
        mModel = new QStringListModel;
        mModel->setStringList(mList);
    }
    //	QGridLayout * ui->grid(new QGridLayout(this));
    // line edits do not take ownership of a completer
    auto completer(new QCompleter(mModel, this));

    //	dont know why but it doesn't want to be placed in the ui->grid ###
    //	QLabel *lab(new QLabel(i18n("<div style=\"font-weight:bold;\">Fill-in a text field and press enter.</div>"), this));
    //	ui->grid->addWidget(lab,0,0,0,-1);

    QList<FMFontDb::InfoItem> ln;
    ln << FMFontDb::FontFamily << FMFontDb::FontSubfamily << FMFontDb::Designer << FMFontDb::Description << FMFontDb::Copyright << FMFontDb::Trademark
       << FMFontDb::ManufacturerName << FMFontDb::LicenseDescription << FMFontDb::AllInfo;

    int limit = ((ln.count() + 1) / 2) - 1;

    for (int gIdx(0); gIdx < ln.count(); ++gIdx) {
        FMFontDb::InfoItem k(ln[gIdx]);
        //		if((k !=  FMFontDb::AllInfo))
        {
            QString fieldname(FontStrings::Names().value(k));
            if (k == FMFontDb::AllInfo)
                fieldname = QString("<div style=\"font-weight:bold\">%1</div>").arg(fieldname);
            auto label(new QLabel(fieldname, this));
            auto line(new QLineEdit(this));
            metFields[line] = k;
            line->setCompleter(completer);
            label->setBuddy(line);
            connect(line, &QLineEdit::returnPressed, this, &MetaWidget::addFilter);
            if ((gIdx) < limit) {
                ui->grid->addWidget(label, gIdx, 0);
                ui->grid->addWidget(line, gIdx, 1);
            } else {
                int row(gIdx - limit);
                ui->grid->addWidget(label, row, 3);
                ui->grid->addWidget(line, row, 4);
            }
        }
    }

    connect(ui->cancelButton, &QPushButton::clicked, this, &MetaWidget::Close);
    connect(ui->filterButton, &QPushButton::clicked, this, &MetaWidget::addFilter);
}

MetaWidget::~MetaWidget()
{
    delete ui;
}

void MetaWidget::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MetaWidget::addFilter()
{
    //	if(QString(sender()->metaObject()->className()) == QString("QLineEdit"))
    //	{
    //		QLineEdit *l(reinterpret_cast<QLineEdit*>(sender()));
    //		FMFontDb::InfoItem it(metFields[l]);
    //		QString t(l->text());
    //		if(!mList.contains(t))
    //		{
    //			mList.append(t);
    //			mModel->setStringList(mList);
    //		}
    //		resultMap[it] = t;
    //	}
    //	else
    {
        for (const auto metFieldsKeys = metFields.keys(); auto *l : metFieldsKeys) {
            QString t(l->text());
            FMFontDb::InfoItem it(metFields.value(l));
            if (!t.isEmpty()) {
                if (!mList.contains(t)) {
                    mList.append(t);
                    mModel->setStringList(mList);
                }
                resultMap[it] = t;
            }
        }
    }
    Q_EMIT filterAdded();
}

#include "moc_metawidget.cpp"
