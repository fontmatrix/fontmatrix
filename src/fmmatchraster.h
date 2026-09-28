/*
    SPDX-FileCopyrightText: 2009 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FMMATCHRASTER_H
#define FMMATCHRASTER_H

#include "ui_matchraster.h"
#include <QMap>
#include <QRect>

class FontItem;

class FMMatchRaster : public QDialog, private Ui::MatchRasterDialog
{
    Q_OBJECT
public:
    explicit FMMatchRaster(QWidget *parent);
    ~FMMatchRaster() override;

private:
    QRect curRect;
    QRgb curCol = 0U;
    // 		QMap<unsigned int, QImage> mItems;
    unsigned int refCodepoint;
    QImage refImage;

    int m_compsize;
    int m_minRefSize;
    bool m_waitingForButton;
    int m_progressValue;
    double m_matchLimit;

    QList<FontItem *> compFonts;
    QList<FontItem *> remainFonts;
    QList<FontItem *> filteredFonts;
    FontItem *waitingFont = nullptr;

    QImage autoCrop(const QImage &img);

protected:
    void moveEvent(QMoveEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private Q_SLOTS:
    void browseImage();
    void enterGrabMode(bool);
    void zoomChanged(int);
    void addImage(const QString &text);
    void search();

    void switchControlRect(bool);

    void slotAcceptFont();
    void slotRefuseFont();

    void slotStop();

    void recordCurrentRect(QRect);
    void recordCurrentColor(QRgb);

private:
    void loadImage();
    void grabScreen();
};

#endif // FMMATCHRASTER_H
