/*
    SPDX-FileCopyrightText: 2008 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "panosedialog.h"
#include "fmfontdb.h"
#include "fmfontstrings.h"
#include "fontitem.h"

#include <QComboBox>
#include <QDebug>

FMPanoseDialog::FMPanoseDialog(FontItem *font, QWidget *parent)
    : QDialog(parent)
    , m_font(font)
    , m_ok(false)
{
    setupUi(this);
    Q_ASSERT(m_font);
    fontName->setText(m_font->fancyName());
    m_sourcepanose = FMFontDb::DB()->getValue(m_font->path(), FMFontDb::Panose).toString();
    if (!m_sourcepanose.isEmpty())
        populateDialog();

    connect(buttonBox, &QDialogButtonBox::accepted, this, &FMPanoseDialog::closeOk);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &FMPanoseDialog::closeCancel);
}

FMPanoseDialog::~FMPanoseDialog()
{
    for (const auto m_boxKeys = m_box.keys(); const auto &s : m_boxKeys) {
        delete m_box.value(s);
    }
    for (const auto m_labelKeys = m_label.keys(); const auto &s : m_labelKeys) {
        delete m_label.value(s);
    }
}

void FMPanoseDialog::populateDialog()
{
    QStringList pl(m_sourcepanose.split(QStringLiteral(":")));
    QList<int> l;
    if (pl.count() == 10) {
        for (const auto &s : std::as_const(pl)) {
            l << s.toInt();
        }
    }

    const QMap<FontStrings::PanoseKey, QMap<int, QString>> &pmap(FontStrings::Panose());
    FontStrings::PanoseKey pk(FontStrings::firstPanoseKey());
    int pValue(0);
    while (pk != FontStrings::InvalidPK) {
        QString sk(FontStrings::PanoseKeyName(pk));
        pValue = l.takeFirst();
        m_label[sk] = new QLabel(sk, this);
        m_box[sk] = new QComboBox(this);

        int cIndex(0);
        for (const auto pmapKeys = pmap[pk].keys(); const auto &ik : pmapKeys) {
            m_box.value(sk)->addItem(pmap.value(pk).value(ik), ik);
            if (ik == pValue)
                m_box.value(sk)->setCurrentIndex(cIndex);
            ++cIndex;
        }
        vLayout->addWidget(m_label.value(sk));
        vLayout->addWidget(m_box.value(sk));

        pk = FontStrings::nextPanoseKey(pk);
    }

    for (const auto m_boxKeysList = m_box.keys(); const auto &sk : m_boxKeysList) {
        connect(m_box.value(sk), &QComboBox::currentIndexChanged, this, &FMPanoseDialog::panoseChange);
    }
}

void FMPanoseDialog::panoseChange(int)
{
    QStringList l;
    FontStrings::PanoseKey pk(FontStrings::firstPanoseKey());

    while (pk != FontStrings::InvalidPK) {
        QString sk(FontStrings::PanoseKeyName(pk));

        l << QString::number(m_box[sk]->itemData(m_box[sk]->currentIndex()).toInt());

        pk = FontStrings::nextPanoseKey(pk);
    }
    m_targetpanose = l.join(QStringLiteral(":"));
}

void FMPanoseDialog::closeOk()
{
    m_ok = true;
    close();
}

void FMPanoseDialog::closeCancel()
{
    close();
}

#include "moc_panosedialog.cpp"
