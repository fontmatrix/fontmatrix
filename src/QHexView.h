/*
    SPDX-FileCopyrightText: 2006 Evan Teran <eteran@alum.rit.edu>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef QHEXVIEW_20060506_H_
#define QHEXVIEW_20060506_H_

#include <QAbstractScrollArea>
#include <QAction>
#include <QByteArray>
#include <QMap>
#include <QMenu>
#include <QString>
// added to pacify gcc44
#include <ctype.h>
#include <stdint.h>
// #include "Types.h"

class QMenu;
class ByteStream;
class CommentServerInterface;

class QHexView : public QAbstractScrollArea
{
    Q_OBJECT

public:
    using C = QVector<uint8_t>;
    using address_t = uint32_t;

public:
    explicit QHexView(QWidget *parent = nullptr);

public:
    void setCommentServer(CommentServerInterface *p);
    [[nodiscard]] CommentServerInterface *commentServer() const;

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

public Q_SLOTS:
    void setShowAddress(bool);
    void setShowAsciiDump(bool);
    void setShowHexDump(bool);
    void setShowComments(bool);

public:
    // void setLineColor(QColor);
    // void setAddressColor(QColor);
    void setWordWidth(int);
    void setRowWidth(int);
    void setHexFont(const QFont &font);
    void setShowAddressSeparator(bool value);
    void repaintView();

public:
    [[nodiscard]] address_t addressOffset() const;
    [[nodiscard]] bool showHexDump() const;
    [[nodiscard]] bool showAddress() const;
    [[nodiscard]] bool showAsciiDump() const;
    [[nodiscard]] bool showComments() const;
    [[nodiscard]] QColor lineColor() const;
    [[nodiscard]] QColor addressColor() const;
    [[nodiscard]] int wordWidth() const;
    [[nodiscard]] int rowWidth() const;

private:
    int m_RowWidth; // amount of "words" per row
    int m_WordWidth; // size of a "word" in bytes
    QColor m_AddressColor; // colour of the address in display
    bool m_ShowHex; // should we show the hex display?
    bool m_ShowAscii; // should we show the ascii display?
    bool m_ShowAddress; // should we show the address display?
    bool m_ShowComments;

public:
    void setData(C *d);
    void setAddressOffset(address_t offset);
    void scrollTo(unsigned int offset);

    [[nodiscard]] address_t selectedBytesAddress() const;
    [[nodiscard]] unsigned int selectedBytesSize() const;
    [[nodiscard]] QByteArray selectedBytes() const;
    [[nodiscard]] QByteArray allBytes() const;
    QMenu *createStandardContextMenu();

public Q_SLOTS:
    void mnuSetFont();
    void mnuCopy();

public:
    void clear();
    void selectAll();
    void deselect();
    [[nodiscard]] bool hasSelectedText() const;

private:
    void updateScrollbars();

    [[nodiscard]] bool isSelected(int index) const;
    [[nodiscard]] bool isInViewableArea(int index) const;

    [[nodiscard]] int pixelToWord(int x, int y) const;

    [[nodiscard]] unsigned int charsPerWord() const;
    [[nodiscard]] int hexDumpLeft() const;
    [[nodiscard]] int asciiDumpLeft() const;
    [[nodiscard]] int commentLeft() const;
    [[nodiscard]] unsigned int addressLen() const;
    [[nodiscard]] int line1() const;
    [[nodiscard]] int line2() const;
    [[nodiscard]] int line3() const;

    [[nodiscard]] unsigned int bytesPerRow() const;

    [[nodiscard]] int dataSize() const;

    void drawAsciiDump(QPainter &painter, unsigned int offset, unsigned int row) const;
    void drawHexDump(QPainter &painter, unsigned int offset, unsigned int row, int &wordCount) const;
    void drawComments(QPainter &painter, unsigned int offset, unsigned int row) const;

    QString formatAddress(address_t address);

private:
    static bool isPrintable(unsigned int ch);
    // convenience: a checkable menu entry whose toggled(bool) goes to slot
    template<typename Receiver, typename Slot>
    static QAction *addToggleActionToMenu(QMenu *menu, const QString &caption, bool checked, Receiver *receiver, Slot slot)
    {
        auto *const action = new QAction(caption, menu);
        action->setCheckable(true);
        action->setChecked(checked);
        menu->addAction(action);
        connect(action, &QAction::toggled, receiver, slot);
        return action;
    }

private:
    address_t m_Origin;
    address_t m_AddressOffset; // this is the offset that our base address is relative to
    int m_SelectionStart; // index of first selected word (or -1)
    int m_SelectionEnd; // index of last selected word (or -1)
    int m_FontWidth = 0; // width of a character in this font
    int m_FontHeight = 0; // height of a character in this font
    C *m_Data = nullptr; // the current data

    enum {
        Highlighting_None,
        Highlighting_Data,
        Highlighting_Ascii
    } m_Highlighting;

    QColor m_EvenWord;
    QColor m_NonPrintableText;
    char m_UnprintableChar;

    bool m_ShowLine1;
    bool m_ShowLine2;
    bool m_ShowLine3;
    bool m_ShowAddressSeparator = false; // should we show ':' character in address to seperate high/low portions
    char m_AddressFormatString[32] = {};

    CommentServerInterface *m_CommentServer = nullptr;
};

#endif
