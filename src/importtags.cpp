/*
    SPDX-FileCopyrightText: 2008 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "importtags.h"
#include "fmfontdb.h"
#include "fontmatrix_debug.h"

#include <QDebug>

ImportTags::ImportTags(QWidget *parent, QStringList tags)
    : QDialog(parent)
{
    qCDebug(FONTMATRIX_LOG) << "ImportTags(" << tags << ")";
    setupUi(this);
    m_tags = tags;
    // 	m_tags.removeAll("Activated_On");
    // 	m_tags.removeAll("Activated_Off");
    for (int i = 0; i < m_tags.count(); ++i) {
        auto it = new QListWidgetItem(m_tags.at(i), tagsList);
        it->setCheckState(Qt::Unchecked);
    }

    connect(tagNewButton, &QPushButton::released, this, &ImportTags::slotNewTag);
    connect(tagText, &QLineEdit::editingFinished, this, &ImportTags::slotNewTag);
    connect(okButton, &QPushButton::released, this, &ImportTags::slotEnd);
}

ImportTags::~ImportTags()
{
    for (int i = 0; i < tagsList->count(); ++i) {
        delete tagsList->item(i);
    }
}

void ImportTags::slotNewTag()
{
    QString nTag(tagText->text());
    if (m_tags.contains(nTag))
        return;
    if (nTag.simplified().isEmpty())
        return;
    m_tags << nTag;
    auto it = new QListWidgetItem(nTag, tagsList);
    it->setCheckState(Qt::Checked);
    tagText->clear();

    FMFontDb::DB()->addTagToDB(nTag);
}

void ImportTags::slotEnd()
{
    m_tags.clear();
    for (int i = 0; i < tagsList->count(); ++i) {
        if (tagsList->item(i)->checkState() == Qt::Checked)
            m_tags << tagsList->item(i)->text();
    }
    close();
}

#include "moc_importtags.cpp"
