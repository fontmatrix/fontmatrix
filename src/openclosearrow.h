/*
    SPDX-FileCopyrightText: 2010 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef OPENCLOSEARROW_H
#define OPENCLOSEARROW_H

#include <QLabel>

class OpenCloseArrow : public QLabel
{
    Q_OBJECT

    QString baseText;

public:
    explicit OpenCloseArrow(QWidget *parent = nullptr, bool open = true);
    ~OpenCloseArrow() override;

    [[nodiscard]] bool isOpen() const
    {
        return openingState;
    }

private:
    bool openingState;
    void setOpText();

protected:
    void mouseReleaseEvent(QMouseEvent *ev) override;

Q_SIGNALS:
    void openChanged(bool);

public:
    void changeOpen(bool t);
};

#endif // OPENCLOSEARROW_H
