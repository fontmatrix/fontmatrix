/*
    SPDX-FileCopyrightText: 2008 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "fmlayoptwidget.h"
#include <QDebug>

FMLayOptWidget::FMLayOptWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);

    connect(beforeSlid, &QSlider::valueChanged, this, &FMLayOptWidget::bChanged);
    connect(fitSlid, &QSlider::valueChanged, this, &FMLayOptWidget::exChanged);
    connect(afterSlid, &QSlider::valueChanged, this, &FMLayOptWidget::aChanged);
    connect(endSlid, &QSlider::valueChanged, this, &FMLayOptWidget::enChanged);
    connect(hyphSlid, &QSlider::valueChanged, this, &FMLayOptWidget::hChanged);
    connect(spaceSlid, &QSlider::valueChanged, this, &FMLayOptWidget::sChanged);

    connect(beforeSlid, &QSlider::sliderReleased, this, &FMLayOptWidget::bEdited);
    connect(fitSlid, &QSlider::sliderReleased, this, &FMLayOptWidget::exEdited);
    connect(afterSlid, &QSlider::sliderReleased, this, &FMLayOptWidget::aEdited);
    connect(endSlid, &QSlider::sliderReleased, this, &FMLayOptWidget::enEdited);
    connect(hyphSlid, &QSlider::sliderReleased, this, &FMLayOptWidget::hEdited);
    connect(spaceSlid, &QSlider::sliderReleased, this, &FMLayOptWidget::sEdited);
}

void FMLayOptWidget::bChanged(int cv)
{
    bScore->setText(QString::number(cv));
    if (!beforeSlid->isSliderDown())
        bEdited();
}

void FMLayOptWidget::exChanged(int cv)
{
    eScore->setText(QString::number(cv));
    if (!fitSlid->isSliderDown())
        exEdited();
}

void FMLayOptWidget::aChanged(int cv)
{
    aScore->setText(QString::number(cv));
    if (!afterSlid->isSliderDown())
        aEdited();
}

void FMLayOptWidget::enChanged(int cv)
{
    endScore->setText(QString::number(cv));
    if (!endSlid->isSliderDown())
        enEdited();
}

void FMLayOptWidget::hChanged(int cv)
{
    hPenalty->setText(QString::number((double)cv / 10.0));
    if (!hyphSlid->isSliderDown())
        hEdited();
}

void FMLayOptWidget::sChanged(int cv)
{
    minSpace->setText(QString::number(cv));
    if (!spaceSlid->isSliderDown())
        sEdited();
}

void FMLayOptWidget::bEdited()
{
    Q_EMIT valueChanged(BEFORE);
}

void FMLayOptWidget::exEdited()
{
    Q_EMIT valueChanged(EXACT);
}

void FMLayOptWidget::aEdited()
{
    Q_EMIT valueChanged(AFTER);
}

void FMLayOptWidget::enEdited()
{
    Q_EMIT valueChanged(END);
}

void FMLayOptWidget::hEdited()
{
    Q_EMIT valueChanged(HYPHEN);
}

void FMLayOptWidget::sEdited()
{
    Q_EMIT valueChanged(SPACE);
}

void FMLayOptWidget::setRange(V v, int min, int max)
{
    switch (v) {
    case BEFORE:
        beforeSlid->setRange(min, max);
        break;
    case EXACT:
        fitSlid->setRange(min, max);
        break;
    case AFTER:
        afterSlid->setRange(min, max);
        break;
    case END:
        endSlid->setRange(min, max);
        break;
    case HYPHEN:
        hyphSlid->setRange(min, max);
        break;
    case SPACE:
        spaceSlid->setRange(min, max);
        break;
    default:
        break;
    }
}

void FMLayOptWidget::setValue(V v, int value)
{
    switch (v) {
    case BEFORE:
        beforeSlid->setValue(value);
        bScore->setText(QString::number(value));
        break;
    case EXACT:
        fitSlid->setValue(value);
        eScore->setText(QString::number(value));
        break;
    case AFTER:
        afterSlid->setValue(value);
        aScore->setText(QString::number(value));
        break;
    case END:
        endSlid->setValue(value);
        endScore->setText(QString::number(value));
        break;
    case HYPHEN:
        hyphSlid->setValue(value);
        hPenalty->setText(QString::number(((double)value) / 10.0));
        break;
    case SPACE:
        spaceSlid->setValue(value);
        minSpace->setText(QString::number(value) + QLatin1String(" %"));
        break;
    default:
        break;
    }
}

void FMLayOptWidget::setValue(V v, double value)
{
    setValue(v, qRound(value));
}

double FMLayOptWidget::getValue(V v)
{
    switch (v) {
    case BEFORE:
        return beforeSlid->value();
        break;
    case EXACT:
        return fitSlid->value();
        break;
    case AFTER:
        return afterSlid->value();
        break;
    case END:
        return endSlid->value();
        break;
    case HYPHEN:
        return hyphSlid->value();
        break;
    case SPACE:
        return spaceSlid->value();
        break;
    default:
        break;
    }
    return 0.0;
}

#include "moc_fmlayoptwidget.cpp"
