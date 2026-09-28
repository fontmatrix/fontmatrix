/*
    SPDX-FileCopyrightText: 2007 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FMFLOATINGPREVIEW_H
#define FMFLOATINGPREVIEW_H

#include <QRect>
#include <QWidget>

class QPushButton;
class QGridLayout;
class QVBoxLayout;
class QLabel;
class QFrame;
class QLineEdit;

class FontItem;

class FMFloatingMenu : public QWidget
{
    Q_OBJECT
public:
    FMFloatingMenu(QWidget *parent, FontItem *item);
    void childrenVisible(bool v);

    // protected:
    //	void enterEvent(QEvent * e);
    //	void leaveEvent(QEvent *e);

private:
    FontItem *fontItem = nullptr;
    QPushButton *closeButton = nullptr;
    QPushButton *actButton = nullptr;
    QFrame *line = nullptr;
    QGridLayout *menuLayout = nullptr;
    QLabel *fontName = nullptr;
    QLineEdit *text = nullptr;

private Q_SLOTS:
    void forwardCloseClicked();
    void activateFont();

Q_SIGNALS:
    void closeClicked();
};

class FontItem;
class FMFloatingPreview : public QWidget
{
    Q_OBJECT

    FMFloatingPreview(QWidget *parent, FontItem *item);

public:
    ~FMFloatingPreview() override;
    static void create(FontItem *item, QRect pos = QRect());

protected:
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void enterEvent(QEvent *e);
    void leaveEvent(QEvent *e) override;

private:
    FontItem *fontItem = nullptr;
    bool hasMouseGrab;
    QPoint refPoint;
    FMFloatingMenu *menuWidget = nullptr;
    QVBoxLayout *mainLayout = nullptr;
    QLabel *previewLabel = nullptr;

    bool canTransparent();

public Q_SLOTS:
    void updatePreview(const QString &t);
};

#endif // FMFLOATINGPREVIEW_H
