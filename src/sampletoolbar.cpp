/*
    SPDX-FileCopyrightText: 2010 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "sampletoolbar.h"
#include "fmfontstrings.h"
#include "ui_sampletoolbar.h"
#include <KLocalizedString>

SampleToolBar::SampleToolBar(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SampleToolBar)
{
    ui->setupUi(this);
    //    setAutoFillBackground(true);

    connect(ui->liveSize, &QDoubleSpinBox::valueChanged, this, &SampleToolBar::SizeChanged);
    connect(ui->sampleButton, &QToolButton::toggled, this, &SampleToolBar::SampleToggled);
    connect(ui->opentypeButton, &QToolButton::toggled, this, &SampleToolBar::OpenTypeToggled);
    connect(ui->variationsButton, &QToolButton::toggled, this, &SampleToolBar::VariationsToggled);
    connect(ui->languageCombo, &QComboBox::currentIndexChanged, this, &SampleToolBar::ScriptSelected);
}

SampleToolBar::~SampleToolBar()
{
    delete ui;
}

void SampleToolBar::changeEvent(QEvent *e)
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

double SampleToolBar::getFontSize() const
{
    return ui->liveSize->value();
}

void SampleToolBar::setFontSize(double fs)
{
    ui->liveSize->setValue(fs);
}

namespace
{
QToolButton *buttonOf(Ui::SampleToolBar *ui, SampleToolBar::Button b)
{
    switch (b) {
    case SampleToolBar::SampleButton:
        return ui->sampleButton;
    case SampleToolBar::OpenTypeButton:
        return ui->opentypeButton;
    case SampleToolBar::VariationsButton:
        return ui->variationsButton;
    }
    return nullptr;
}
}

bool SampleToolBar::isChecked(Button b)
{
    QToolButton *button = buttonOf(ui, b);
    return button && button->isChecked();
}

void SampleToolBar::toggle(Button b, bool c)
{
    if (QToolButton *button = buttonOf(ui, b))
        button->setChecked(c);
}

void SampleToolBar::enableButton(Button b, bool c)
{
    if (QToolButton *button = buttonOf(ui, b))
        button->setEnabled(c);
}

void SampleToolBar::showButton(Button b, bool s)
{
    if (QToolButton *button = buttonOf(ui, b))
        button->setVisible(s);
}

void SampleToolBar::setScripts(const QStringList &ll)
{
    // Called from SampleWidget::fillOTTree() on every font selection, so the
    // combo has to be emptied first: without this each font appends another
    // "Select language" entry plus a duplicate of every script, and the
    // selection is left pointing into the previous font's entries. A stale
    // non-default selection matters, because SampleWidget::doRender() gives
    // the script shaper precedence over the OpenType feature checkboxes —
    // they then silently stop having any effect.
    ui->languageCombo->clear();
    ui->languageCombo->addItem(i18nc("@item:inlistbox", "Select language"), QString("NOSHAPER"));
    for (const auto &l : ll) {
        ui->languageCombo->addItem(FontStrings::scriptTagName(l), l);
    }
    // Re-enable as well as disable: a font with no scripts used to leave the
    // combo disabled for every font selected afterwards.
    ui->languageCombo->setEnabled(!ll.isEmpty());
}

QString SampleToolBar::getScript()
{
    QString ret(ui->languageCombo->itemData(ui->languageCombo->currentIndex()).toString());
    if (ret != QString("NOSHAPER"))
        return ret;
    return QString();
}

#include "moc_sampletoolbar.cpp"
