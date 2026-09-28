/*
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "fmvariationswidget.h"
#include "fontitem.h"

#include <KLocalizedString>

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QVBoxLayout>

#include <cmath>

FMVariationsWidget::FMVariationsWidget(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);

    auto *instanceRow = new QHBoxLayout;
    instanceRow->addWidget(new QLabel(i18nc("@label the named instances of a variable font", "Instance:"), this));
    m_instances = new QComboBox(this);
    m_instances->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    instanceRow->addWidget(m_instances, 1);
    layout->addLayout(instanceRow);

    m_axesBox = new QWidget(this);
    m_axesLayout = new QGridLayout(m_axesBox);
    m_axesLayout->setContentsMargins(0, 0, 0, 0);
    m_axesLayout->setColumnStretch(1, 1);
    layout->addWidget(m_axesBox);

    auto *buttons = new QHBoxLayout;
    m_reset = new QPushButton(i18nc("@action:button back to the default coordinates of a variable font", "Default"), this);
    buttons->addWidget(m_reset);
    buttons->addStretch(1);
    layout->addLayout(buttons);
    layout->addStretch(1);

    connect(m_instances, &QComboBox::activated, this, &FMVariationsWidget::slotInstancePicked);
    connect(m_reset, &QPushButton::clicked, this, &FMVariationsWidget::slotReset);
}

FMVariationsWidget::~FMVariationsWidget() = default;

void FMVariationsWidget::setFont(FontItem *font)
{
    m_font = (font && font->isVariable()) ? font : nullptr;
    rebuild();
}

void FMVariationsWidget::rebuild()
{
    m_updating = true;
    m_rows.clear();
    m_instances->clear();
    // the rows of the last font
    while (QLayoutItem *item = m_axesLayout->takeAt(0)) {
        delete item->widget();
        delete item;
    }
    if (!m_font) {
        m_updating = false;
        setEnabled(false);
        return;
    }
    setEnabled(true);

    m_instances->addItem(i18nc("@item:inlistbox the coordinates of a variable font match no named instance", "Custom"));
    const QList<FontNamedInstance> instances(m_font->namedInstances());
    for (const FontNamedInstance &instance : instances)
        m_instances->addItem(instance.name);

    const QList<FontVariationAxis> axes(m_font->variationAxes());
    int row = 0;
    for (int a = 0; a < axes.size(); ++a) {
        const FontVariationAxis &axis = axes.at(a);
        AxisRow ar;
        if (!axis.hidden) {
            // whole numbers on a wide axis (weight 100–900), hundredths on a narrow one (italic 0–1)
            ar.scale = (axis.maximum - axis.minimum) >= 50.0 ? 1.0 : 100.0;
            auto *name = new QLabel(axis.name, m_axesBox);
            name->setToolTip(i18nc("@info:tooltip the tag and the range of a variation axis, %1 tag, %2 minimum, %3 default, %4 maximum",
                                   "%1: from %2 to %4, %3 by default",
                                   axis.tag,
                                   QString::number(axis.minimum),
                                   QString::number(axis.def),
                                   QString::number(axis.maximum)));
            ar.slider = new QSlider(Qt::Horizontal, m_axesBox);
            ar.slider->setRange(int(std::lround(axis.minimum * ar.scale)), int(std::lround(axis.maximum * ar.scale)));
            ar.spin = new QDoubleSpinBox(m_axesBox);
            ar.spin->setDecimals(ar.scale > 1.0 ? 2 : 0);
            ar.spin->setSingleStep(1.0 / ar.scale);
            ar.spin->setRange(axis.minimum, axis.maximum);
            ar.spin->setKeyboardTracking(false);
            m_axesLayout->addWidget(name, row, 0);
            m_axesLayout->addWidget(ar.slider, row, 1);
            m_axesLayout->addWidget(ar.spin, row, 2);
            ++row;
            connect(ar.slider, &QSlider::valueChanged, this, [this, a](int value) {
                slotSliderMoved(a, value);
            });
            connect(ar.spin, &QDoubleSpinBox::valueChanged, this, [this, a](double value) {
                slotSpinChanged(a, value);
            });
        }
        m_rows << ar;
    }
    m_updating = false;

    QList<double> coords(m_font->variationCoordinates());
    if (coords.isEmpty()) {
        for (const FontVariationAxis &axis : axes)
            coords << axis.def;
    }
    showCoordinates(coords);
}

void FMVariationsWidget::showCoordinates(const QList<double> &coords)
{
    m_updating = true;
    for (int a = 0; a < m_rows.size() && a < coords.size(); ++a) {
        const AxisRow &ar = m_rows.at(a);
        if (!ar.slider)
            continue;
        ar.slider->setValue(int(std::lround(coords.at(a) * ar.scale)));
        ar.spin->setValue(coords.at(a));
    }
    m_instances->setCurrentIndex(m_font ? m_font->namedInstance() + 1 : 0);
    m_updating = false;
}

QList<double> FMVariationsWidget::coordinates() const
{
    QList<double> coords;
    if (!m_font)
        return coords;
    // a hidden axis keeps the coordinate it has
    QList<double> current(m_font->variationCoordinates());
    const QList<FontVariationAxis> axes(m_font->variationAxes());
    for (int a = 0; a < m_rows.size(); ++a) {
        const AxisRow &ar = m_rows.at(a);
        if (ar.spin)
            coords << ar.spin->value();
        else
            coords << (a < current.size() ? current.at(a) : axes.at(a).def);
    }
    return coords;
}

void FMVariationsWidget::applyCoordinates()
{
    if (!m_font || m_updating)
        return;
    m_font->setVariationCoordinates(coordinates());
    m_updating = true;
    m_instances->setCurrentIndex(m_font->namedInstance() + 1);
    m_updating = false;
    // what the user chose here is the font's look from now on, in the next session too
    m_font->rememberVariation();
    Q_EMIT coordinatesChanged();
}

void FMVariationsWidget::slotInstancePicked(int index)
{
    if (!m_font || m_updating || index <= 0)
        return;
    const QList<FontNamedInstance> instances(m_font->namedInstances());
    if (index > instances.size())
        return;
    m_font->setVariationCoordinates(instances.at(index - 1).coords);
    showCoordinates(m_font->variationCoordinates());
    // what the user chose here is the font's look from now on, in the next session too
    m_font->rememberVariation();
    Q_EMIT coordinatesChanged();
}

void FMVariationsWidget::slotSliderMoved(int axis, int value)
{
    if (m_updating || axis >= m_rows.size())
        return;
    const AxisRow &ar = m_rows.at(axis);
    m_updating = true;
    ar.spin->setValue(value / ar.scale);
    m_updating = false;
    applyCoordinates();
}

void FMVariationsWidget::slotSpinChanged(int axis, double value)
{
    if (m_updating || axis >= m_rows.size())
        return;
    const AxisRow &ar = m_rows.at(axis);
    m_updating = true;
    ar.slider->setValue(int(std::lround(value * ar.scale)));
    m_updating = false;
    applyCoordinates();
}

void FMVariationsWidget::slotReset()
{
    if (!m_font)
        return;
    m_font->setVariationCoordinates(QList<double>());
    QList<double> defaults;
    const QList<FontVariationAxis> axes(m_font->variationAxes());
    for (const FontVariationAxis &axis : axes)
        defaults << axis.def;
    showCoordinates(defaults);
    // what the user chose here is the font's look from now on, in the next session too
    m_font->rememberVariation();
    Q_EMIT coordinatesChanged();
}

#include "moc_fmvariationswidget.cpp"
