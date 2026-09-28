/*
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FMVARIATIONSWIDGET_H
#define FMVARIATIONSWIDGET_H

#include <QList>
#include <QWidget>

class FontItem;
class QComboBox;
class QDoubleSpinBox;
class QGridLayout;
class QPushButton;
class QSlider;

/**
 * The axes of a variable font: a named instance to pick, a slider and a
 * spin box per axis, a button back to the default. Every change goes to
 * FontItem::setVariationCoordinates(), so that whatever renders the font
 * next shows it there, and coordinatesChanged() tells the views to do so.
 */
class FMVariationsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FMVariationsWidget(QWidget *parent = nullptr);
    ~FMVariationsWidget() override;

    /// the font whose axes are shown; nullptr, or a font without axes, empties the widget
    void setFont(FontItem *font);
    [[nodiscard]] FontItem *font() const
    {
        return m_font;
    }

Q_SIGNALS:
    void coordinatesChanged();

private:
    struct AxisRow {
        QSlider *slider = nullptr;
        QDoubleSpinBox *spin = nullptr;
        double scale = 1.0; ///< slider steps per design unit
    };

    void rebuild();
    void slotInstancePicked(int index);
    void slotSliderMoved(int axis, int value);
    void slotSpinChanged(int axis, double value);
    void slotReset();
    void showCoordinates(const QList<double> &coords);
    void applyCoordinates();
    [[nodiscard]] QList<double> coordinates() const;

    FontItem *m_font = nullptr;
    QComboBox *m_instances = nullptr;
    QGridLayout *m_axesLayout = nullptr;
    QWidget *m_axesBox = nullptr;
    QPushButton *m_reset = nullptr;
    QList<AxisRow> m_rows;
    bool m_updating = false;
};

#endif
