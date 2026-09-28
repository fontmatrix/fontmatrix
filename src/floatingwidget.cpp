/*
    SPDX-FileCopyrightText: 2010 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "floatingwidget.h"
#include "floatingwidgetsregister.h"
#include "fmfontdb.h"
#include "fontitem.h"
#include "typotek.h"

FloatingWidget::FloatingWidget(const QString &f, const QString &typ, QWidget *parent)
    : QWidget(parent)
    , fName(f)
    , fType(typ)
    , printer(nullptr)
    , printDialog(nullptr)
{
    setAttribute(Qt::WA_DeleteOnClose);
    QString fn;
    FontItem *fi(FMFontDb::DB()->Font(fName));
    if (fi != nullptr)
        fn = fi->fancyName();
    else
        fn = f;
    actionName = QStringLiteral("[%1]").arg(fType) + QStringLiteral(" ") + fn;
    wTitle = fn + QStringLiteral(" - Fontmatrix");
    if (nullptr == parent) {
        ddetach();
    }
}

FloatingWidget::~FloatingWidget()
{
    if (printer)
        delete printer;
    if (printDialog)
        delete printDialog;
}

bool FloatingWidget::event(QEvent *e)
{
    //	if(windowTitle().isEmpty())
    //	{
    //		QWidget::setWindowTitle(wTitle);
    //	}
    if ((e->type() == QEvent::Show) || (e->type() == QEvent::Hide))
        Q_EMIT visibilityChange();

    return QWidget::event(e);
}

void FloatingWidget::activate(bool a)
{
    if (a) {
        if (!isVisible())
            setVisible(true);
        raise();
    } else
        hide();
}

void FloatingWidget::ddetach()
{
    if (nullptr != parent())
        setParent(nullptr, Qt::Window);
    setWindowTitle(wTitle);
    FloatingWidgetsRegister::Register(this, fName, fType);
    show();
    Q_EMIT detached();
}

#include "moc_floatingwidget.cpp"
