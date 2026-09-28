/*
    SPDX-FileCopyrightText: 2007 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "fmfloatingpreview.h"
#include "fmactivate.h"
#include "fmfontdb.h"
#include "fontitem.h"
#include "typotek.h"

#include <KLocalizedString>
#include <QCursor>
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPixmap>
#include <QPushButton>
#include <QSize>
#include <QVBoxLayout>

#include <QDebug>

FMFloatingMenu::FMFloatingMenu(QWidget *parent, FontItem *item)
    : QWidget(parent)
    , fontItem(item)
{
    menuLayout = new QGridLayout(this);
    menuLayout->setContentsMargins(3, 0, 3, 0);
    menuLayout->setVerticalSpacing(0);

    QFont f(font());
    double fs(f.pointSizeF());
    f.setPointSizeF(0.6 * fs);

    if (item) {
        QFont f2(f);
        f2.setBold(true);
        fontName = new QLabel(item->fancyName(), this);
        fontName->setFont(f2);
        menuLayout->addWidget(fontName, 0, 0);
    }

    line = new QFrame(this);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    menuLayout->addWidget(line, 0, 1);

    bool act(false);
    if (item && !item->isActivated()) {
        actButton = new QPushButton(i18nc("@action:button", "Activate"), this);
        actButton->setFont(f);
        menuLayout->addWidget(actButton, 0, 2, Qt::AlignRight);
        connect(actButton, &QPushButton::clicked, this, &FMFloatingMenu::activateFont);
        act = true;
    }

    closeButton = new QPushButton(i18nc("@action:button", "close"), this);
    closeButton->setFont(f);
    menuLayout->addWidget(closeButton, 0, act ? 3 : 2, Qt::AlignRight);

    text = new QLineEdit(typotek::getInstance()->word(item), this);
    text->setFont(f);
    menuLayout->addWidget(text, 1, 0, 1, -1);

    connect(text, &QLineEdit::textEdited, reinterpret_cast<FMFloatingPreview *>(parent), &FMFloatingPreview::updatePreview);
    connect(closeButton, &QPushButton::clicked, this, &FMFloatingMenu::forwardCloseClicked);
}

// void FMFloatingMenu::enterEvent(QEvent *e)
//{
//	childrenVisible(true);
//	QWidget::enterEvent(e);
// }
//
// void FMFloatingMenu::leaveEvent(QEvent * e)
//{
//	childrenVisible(false);
//	QWidget::leaveEvent(e);
// }

void FMFloatingMenu::childrenVisible(bool v)
{
    for (const auto range = findChildren<QWidget *>(); auto *w : range) {
        w->setVisible(v);
    }
}

void FMFloatingMenu::forwardCloseClicked()
{
    Q_EMIT closeClicked();
}

void FMFloatingMenu::activateFont()
{
    QList<FontItem *> fl;
    fl << fontItem;
    FMActivate::getInstance()->activate(fl, true);
    menuLayout->removeWidget(actButton);
    delete actButton;
}

FMFloatingPreview::FMFloatingPreview(QWidget *parent, FontItem *item)
    : QWidget(parent)
{
    hasMouseGrab = false;
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    if (canTransparent())
        setAttribute(Qt::WA_TranslucentBackground, true);
    setAttribute(Qt::WA_DeleteOnClose, true);

    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    menuWidget = new FMFloatingMenu(this, item);
    previewLabel = new QLabel(this);
    mainLayout->addWidget(menuWidget, Qt::AlignHCenter);
    mainLayout->addWidget(previewLabel, Qt::AlignHCenter);

    connect(menuWidget, &FMFloatingMenu::closeClicked, this, &FMFloatingPreview::close);
}

FMFloatingPreview::~FMFloatingPreview() = default;

void FMFloatingPreview::create(FontItem *item, QRect rect)
{
    auto p(new FMFloatingPreview(typotek::getInstance(), item));
    p->fontItem = item;
    QColor bgC;
    if (p->canTransparent())
        bgC = QColor(Qt::transparent);
    else
        bgC = QColor(Qt::white);
    QPixmap preview = item->oneLinePreviewPixmap(typotek::getInstance()->word(item), Qt::black, bgC, rect.width());

    QRect r(preview.rect());
    QPoint delta(r.width() / 2, r.height() / 2);
    p->previewLabel->setGeometry(r);
    p->previewLabel->setPixmap(preview);

    QRect r2(QPoint(rect.x() - (r.width() / 2), QCursor::pos().y() - (p->menuWidget->geometry().height() / 2)),
             QSize(r.width(), r.height() + p->menuWidget->geometry().height()));

    p->setGeometry(r2);
    p->show();
    p->setFocus(Qt::OtherFocusReason);
    p->grabMouse();
    QApplication::setOverrideCursor(Qt::SizeAllCursor);
    p->hasMouseGrab = true;
    p->refPoint = QCursor::pos() - p->geometry().topLeft();
}

void FMFloatingPreview::mousePressEvent(QMouseEvent *e)
{
    if (!(e->buttons() & Qt::LeftButton))
        return;
    refPoint = e->globalPosition().toPoint() - geometry().topLeft();
    QApplication::setOverrideCursor(Qt::SizeAllCursor);
}

void FMFloatingPreview::mouseReleaseEvent(QMouseEvent *)
{
    if (hasMouseGrab)
        releaseMouse();
    refPoint = QPoint();
    QApplication::restoreOverrideCursor();
}

void FMFloatingPreview::mouseMoveEvent(QMouseEvent *e)
{
    if (!(e->buttons() & Qt::LeftButton))
        return;
    QRect r(geometry());
    QPoint delta(e->globalPosition().toPoint() - refPoint);
    r.moveTo(delta);
    setGeometry(r);
}

void FMFloatingPreview::enterEvent(QEvent *)
{
    menuWidget->childrenVisible(true);
}

void FMFloatingPreview::leaveEvent(QEvent *)
{
    if (hasMouseGrab)
        releaseMouse();
    menuWidget->childrenVisible(false);
    QApplication::restoreOverrideCursor();
}

bool FMFloatingPreview::canTransparent()
{
    return true;
}

void FMFloatingPreview::updatePreview(const QString &t)
{
    QColor bgC;
    if (canTransparent())
        bgC = QColor(Qt::transparent);
    else
        bgC = QColor(Qt::white);
    QPixmap preview = fontItem->oneLinePreviewPixmap(t, Qt::black, bgC, this->width());
    QRect r(preview.rect());
    previewLabel->setGeometry(r);
    previewLabel->setPixmap(preview);
}

#include "moc_fmfloatingpreview.cpp"
