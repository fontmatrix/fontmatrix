/*
    SPDX-FileCopyrightText: 2010 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "playwidget.h"
#include "ui_playwidget.h"

#include <KLocalizedString>
#include <QDialog>
#include <QPainter>
#include <QPrintDialog>
#include <QPrinter>
#include <QRectF>

PlayWidget *PlayWidget::instance = nullptr;
PlayWidget::PlayWidget()
    : ui(new Ui::PlayWidget)
{
    ui->setupUi(this);
    setWindowTitle(i18nc("@title:window", "Playground"));
    ui->toolbar->setDetached();
    ui->toolbar->setNoClose(true);
    playScene = new QGraphicsScene;
    playScene->setSceneRect(0, 0, 10000, 10000);
    ui->playView->setScene(playScene);

    connect(ui->playView, &FMPlayGround::pleaseZoom, this, &PlayWidget::slotZoom);
    connect(ui->toolbar, &FloatingWidgetToolBar::Hide, this, &PlayWidget::hide);
    connect(ui->toolbar, &FloatingWidgetToolBar::Print, this, &PlayWidget::print);
}

PlayWidget::~PlayWidget()
{
    delete ui;
    instance = nullptr;
}

PlayWidget *PlayWidget::getInstance()
{
    if (instance == nullptr) {
        instance = new PlayWidget;
        Q_ASSERT(instance);
    }
    return instance;
}

void PlayWidget::changeEvent(QEvent *e)
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

void PlayWidget::closeEvent(QCloseEvent *)
{
    hide();
}

void PlayWidget::hideEvent(QHideEvent *e)
{
    QWidget::hideEvent(e);
    Q_EMIT visibilityChanged();
}

void PlayWidget::showEvent(QShowEvent *e)
{
    QWidget::showEvent(e);
    Q_EMIT visibilityChanged();
}

void PlayWidget::slotZoom(int z)
{
    double delta = 1.0 + (z / 1000.0);
    QTransform trans;
    trans.scale(delta, delta);
    ui->playView->setTransform(trans, (z == 0) ? false : true);
}

double PlayWidget::playFontSize()
{
    return ui->playFontSize->value();
}

QRectF PlayWidget::getMaxRect()
{
    return ui->playView->getMaxRect();
}

void PlayWidget::clearSelection()
{
    ui->playView->deselectAll();
}

void PlayWidget::print()
{
    QPrinter thePrinter(QPrinter::HighResolution);
    QPrintDialog dialog(&thePrinter, this);
    dialog.setWindowTitle("Fontmatrix - " + i18nc("@title:window", "Print Playground"));

    if (dialog.exec() != QDialog::Accepted)
        return;
    thePrinter.setFullPage(true);
    QPainter aPainter(&thePrinter);

    double pWidth(thePrinter.pageRect(QPrinter::DevicePixel).width());
    double pHeight(thePrinter.pageRect(QPrinter::DevicePixel).height());

    QRectF targetR(pWidth * 0.1, pHeight * 0.1, pWidth * 0.8, pHeight * 0.8);
    QRectF sourceR(PlayWidget::getInstance()->getMaxRect());
    PlayWidget::getInstance()->clearSelection();
    PlayWidget::getInstance()->getPlayScene()->render(&aPainter, targetR, sourceR, Qt::KeepAspectRatio);
}

#include "moc_playwidget.cpp"
