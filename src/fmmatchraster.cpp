/*
    SPDX-FileCopyrightText: 2009 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "fmmatchraster.h"
#include "fmfontdb.h"
#include "fontitem.h"
#include "mainviewwidget.h"
#include "puzzleviewimp.h"
#include "typotek.h"

#include "fmconfig.h"
#include <KLocalizedString>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QImage>
#include <QScreen>
#include <QString>

#include <KMessageBox>
#include <QDebug>

FMMatchRaster::FMMatchRaster(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);
    m_compsize = FMConfig::value(QStringLiteral("MatchRaster/CompareSize"), 120).toInt();
    m_matchLimit = FMConfig::value(QStringLiteral("MatchRaster/Limit"), 800.0).toDouble();
    m_minRefSize = FMConfig::value(QStringLiteral("MatchRaster/ReferenceSize"), 160).toInt();

    m_progressValue = 0;
    m_waitingForButton = false;
    waitingFont = nullptr;
    refCodepoint = 0;

    connect(browseButton, &QPushButton::clicked, this, &FMMatchRaster::browseImage);
    connect(grabZoom, &QSlider::valueChanged, this, &FMMatchRaster::zoomChanged);
    connect(grabModeBox, &QCheckBox::toggled, this, &FMMatchRaster::enterGrabMode);
    connect(tweakRectBox, &QCheckBox::toggled, this, &FMMatchRaster::switchControlRect);
    connect(letter, &QLineEdit::textChanged, this, &FMMatchRaster::addImage);
    connect(searchButton, &QPushButton::clicked, this, &FMMatchRaster::search);

    connect(iView, &IView::rectChange, this, &FMMatchRaster::recordCurrentRect);
    connect(iView, &IView::selColorChanged, this, &FMMatchRaster::recordCurrentColor);

    connect(buttonBox, &QDialogButtonBox::rejected, this, &FMMatchRaster::slotRefuseFont);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &FMMatchRaster::slotAcceptFont);

    connect(stopButton, &QPushButton::clicked, this, &FMMatchRaster::slotStop);
}

FMMatchRaster::~FMMatchRaster() = default;

void FMMatchRaster::browseImage()
{
    QString ifile(QFileDialog::getOpenFileName(this, "Fontmatrix - Browse Image", QDir::homePath()));
    imagePath->setText(ifile);
    loadImage();
}

void FMMatchRaster::loadImage()
{
    QString ifile(imagePath->text());
    if (QFile::exists(ifile))
        iView->setImage(ifile);
}

void FMMatchRaster::addImage(const QString &)
{
    if (letter->text().isEmpty())
        return;

    bool ok;
    refCodepoint = (letter->text().size() != 4) ? letter->text().at(0).unicode() : letter->text().toUInt(&ok, 16);
    refImage = iView->getPixmap().toImage().copy(curRect);
    const unsigned int iw(refImage.width());
    const unsigned int ih(refImage.height());
    if ((iw < static_cast<unsigned int>(m_minRefSize)) && (ih < static_cast<unsigned int>(m_minRefSize))) {
        double dw(iw);
        double dh(ih);
        double minS(m_minRefSize);
        double ratio(minS / qMin(dw, dh));
        dw *= ratio;
        dh *= ratio;
        refImage = refImage.scaled(qRound(dw), qRound(dh), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    } else if (iw < static_cast<unsigned int>(m_minRefSize)) {
        refImage = refImage.scaledToWidth(m_minRefSize, Qt::SmoothTransformation);
    } else if (ih < static_cast<unsigned int>(m_minRefSize)) {
        refImage = refImage.scaledToHeight(m_minRefSize, Qt::SmoothTransformation);
    }

    // 	refImage.save("REFIMAGE.png");
}

void FMMatchRaster::search()
{
    if ((refImage.isNull()) || (!refCodepoint)) {
        return;
    }
    buttonBox->setEnabled(false);
    if (!m_waitingForButton) {
        remainFonts = compFonts = FMFontDb::DB()->getFilteredFonts();
        progressBar->setRange(0, compFonts.size());
        stackedWidget->setCurrentIndex(1);

        if (!checkInteractive->isChecked())
            questionWidget->setVisible(false);
    } else {
        compFonts = remainFonts;
    }

    PuzzleViewImp ref(refImage, curCol);

    for (auto *fit : std::as_const(compFonts)) {
        progressBar->setValue(++m_progressValue);
        remainFonts.removeAll(fit);

        fontName->setText(fit->fancyName());
        QImage adjustedImg(autoCrop(fit->charImage(refCodepoint, m_compsize)));
        if (!adjustedImg.isNull()) {
            PuzzleViewImp comp(adjustedImg, QColor(Qt::black).rgb());
            double compResult(ref.CompMean(comp));
            if (compResult >= 0.0) {
                // 				qDebug()<<((compResult < m_matchLimit)? "****":"\t")<<compResult<<fit->fancyName();
                if (compResult < m_matchLimit) {
                    if (!filteredFonts.contains(fit)) {
                        if (checkInteractive->isChecked()) {
                            compView->setEnabled(true);
                            compView->setImage(QPixmap::fromImage(adjustedImg));
                            compView->setEnabled(false);
                            scoreLabel->setText(
                                i18nc("@info", "The font %1 scores %2.\nDo you want to add it to the filtered fonts?", fit->fancyName(), compResult));
                            buttonBox->setEnabled(true);
                            waitingFont = fit;
                            m_waitingForButton = true;
                            return;
                        } else {
                            if (!filteredFonts.contains(fit))
                                filteredFonts << fit;
                        }
                    }
                }
            }
        }
    }

    slotStop();
}

void FMMatchRaster::recordCurrentRect(QRect r)
{
    curRect = r;
}

void FMMatchRaster::recordCurrentColor(QRgb c)
{
    curCol = c;
}

void FMMatchRaster::slotAcceptFont()
{
    if (!filteredFonts.contains(waitingFont))
        filteredFonts << waitingFont;
    waitingFont = nullptr;
    search();
}

void FMMatchRaster::slotRefuseFont()
{
    waitingFont = nullptr;
    search();
}

void FMMatchRaster::slotStop()
{
    if ((waitingFont != nullptr) && (!filteredFonts.contains(waitingFont)))
        filteredFonts << waitingFont;
    if (!filteredFonts.isEmpty()) {
        typotek::getInstance()->getTheMainView()->setCurFonts(filteredFonts);
    } else {
        KMessageBox::information(this, i18nc("@info", "No font match the submitted image"));
    }
    close();
}

QImage FMMatchRaster::autoCrop(const QImage &cImg)
{
    const int cw = cImg.width();
    const int ch = cImg.height();
    if ((!cImg.isNull()) && (cw > 0) && (ch > 0)) {
        QRect r;
        const QRgb wp = cImg.pixel(0, 0);
        bool topReached(false);
        for (int y(0); y < ch; ++y) {
            for (int x(0); x < cw; ++x) {
                if (cImg.pixel(x, y) != wp) {
                    topReached = true;
                    r.setTop(y);
                    break;
                }
            }
            if (topReached)
                break;
        }
        bool bottomReached(false);
        for (int y(ch - 1); y >= 0; --y) {
            for (int x(cw - 1); x >= 0; --x) {
                if (cImg.pixel(x, y) != wp) {
                    bottomReached = true;
                    r.setBottom(y);
                    break;
                }
            }
            if (bottomReached)
                break;
        }
        r.setLeft(cw);
        for (int y(0); y < ch; ++y) {
            for (int x(0); x < cw; ++x) {
                if (cImg.pixel(x, y) != wp) {
                    r.setLeft(qMin(x, r.left()));
                    break;
                }
            }
        }
        r.setRight(0);
        for (int y(0); y < ch; ++y) {
            for (int x(cw - 1); x >= 0; --x) {
                if (cImg.pixel(x, y) != wp) {
                    r.setRight(qMax(x, r.right()));
                    break;
                }
            }
        }

        if ((!r.isNull()) && (r.width() > 0) && (r.height() > 0))
            return cImg.copy(r).scaled(refImage.width(), refImage.height());
    }
    return QImage();
}

void FMMatchRaster::switchControlRect(bool cr)
{
    if (cr && grabModeBox->isChecked()) {
        grabModeBox->setChecked(false);
    }
    iView->setControlRect(cr);
}

void FMMatchRaster::grabScreen()
{
    int ratio(grabZoom->value());
    QRect wr(geometry());
    QRect vr(iView->geometry());
    QRect absr(wr.x() - (vr.width() / ratio), wr.y() + vr.y() + sampleBox->geometry().y(), vr.width() / ratio, vr.height() / ratio);
    iView->setImage(QApplication::primaryScreen()->grabWindow(0, absr.x(), absr.y(), absr.width(), absr.height()));
    //		qDebug()<<"iView"<<iView->geometry()<<"this"<<geometry();
}

void FMMatchRaster::moveEvent(QMoveEvent *event)
{
    if (grabModeBox->isChecked()) {
        grabScreen();
    }
    QDialog::moveEvent(event);
}

void FMMatchRaster::resizeEvent(QResizeEvent *event)
{
    if (grabModeBox->isChecked()) {
        grabScreen();
    }
    QDialog::resizeEvent(event);
}

void FMMatchRaster::enterGrabMode(bool e)
{
    if (e)
        grabScreen();
}

void FMMatchRaster::zoomChanged(int)
{
    grabScreen();
}

#include "moc_fmmatchraster.cpp"
