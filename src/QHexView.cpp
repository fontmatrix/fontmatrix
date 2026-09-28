/*
    SPDX-FileCopyrightText: 2006 Evan Teran <eteran@alum.rit.edu>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "QHexView.h"

#include <QApplication>
#include <QClipboard>
#include <QFontDialog>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPalette>
#include <QPixmap>
#include <QScrollBar>
#include <QtGlobal>
#include <cctype>
#include <climits>

// #include "ByteStream.h"
// #include "Util.h"
// #include "CommentServerInterface.h"

//------------------------------------------------------------------------------
// Name: QHexView(QWidget * parent)
// Desc: constructor
//------------------------------------------------------------------------------
QHexView::QHexView(QWidget *parent)
    : QAbstractScrollArea(parent)
    , m_RowWidth(16)
    , m_WordWidth(1)
    , m_AddressColor(Qt::red)
    , m_ShowHex(true)
    , m_ShowAscii(true)
    , m_ShowAddress(true)
    , m_ShowComments(true)
    , m_Origin(0)
    , m_AddressOffset(0)
    , m_SelectionStart(-1)
    , m_SelectionEnd(-1)
    , m_Data(nullptr)
    , m_Highlighting(Highlighting_None)
    , m_EvenWord(Qt::blue)
    , m_NonPrintableText(Qt::red)
    , m_UnprintableChar('.')
    , m_ShowLine1(true)
    , m_ShowLine2(true)
    , m_ShowLine3(true)
    , m_CommentServer(nullptr)
{
    setShowAddressSeparator(true);

    // default to a simple monospace font
    setHexFont(QFont(QStringLiteral("Monospace"), 8));
}

//------------------------------------------------------------------------------
// Name: setShowAddressSeparator(bool value)
// Desc:
//------------------------------------------------------------------------------
void QHexView::setShowAddressSeparator(bool value)
{
    if (value) {
#if QT_POINTER_SIZE == 4
        strncpy(m_AddressFormatString, "%04x:%04x", sizeof(m_AddressFormatString));
#elif QT_POINTER_SIZE == 8
        strncpy(m_AddressFormatString, "%08x:%08x", sizeof(m_AddressFormatString));
#endif
    } else {
#if QT_POINTER_SIZE == 4
        strncpy(m_AddressFormatString, "%04x%04x", sizeof(m_AddressFormatString));
#elif QT_POINTER_SIZE == 8
        strncpy(m_AddressFormatString, "%08x%08x", sizeof(m_AddressFormatString));
#endif
    }

    m_AddressFormatString[sizeof(m_AddressFormatString) - 1] = '\0';

    m_ShowAddressSeparator = value;
}

//------------------------------------------------------------------------------
// Name: ~QHexView()
// Desc: destructor
//------------------------------------------------------------------------------
QString QHexView::formatAddress(address_t address)
{
    QString ret = QString::asprintf(m_AddressFormatString, (address >> 16) & 0xffff, address & 0xffff);
    return ret;
}

//------------------------------------------------------------------------------
// Name:
// Desc:
//------------------------------------------------------------------------------
void QHexView::repaintView()
{
    viewport()->repaint();
}

//------------------------------------------------------------------------------
// Name: dataSize() const
// Desc: returns how much data we are viewing
//------------------------------------------------------------------------------
int QHexView::dataSize() const
{
    return m_Data != nullptr ? m_Data->size() : 0;
}

//------------------------------------------------------------------------------
// Name: setHexFont(const QFont &f)
// Desc: sets the font and calculates font metrics for later. Not named
//       setFont(), which would hide the non-virtual QWidget::setFont()
//------------------------------------------------------------------------------
void QHexView::setHexFont(const QFont &f)
{
    // recalculate all of our metrics/offsets
    const QFontMetrics fm(f);
    m_FontWidth = fm.maxWidth();
    m_FontHeight = fm.height();

    updateScrollbars();

    // TODO: assert that we are using a fixed font & find out if we care?
    QAbstractScrollArea::setFont(f);
}

//------------------------------------------------------------------------------
// Name: createStandardContextMenu()
// Desc: creates the "standard" context menu for the widget
//------------------------------------------------------------------------------
QMenu *QHexView::createStandardContextMenu()
{
    auto const menu = new QMenu(this);

    menu->addAction(QStringLiteral("Set &Font"), this, &QHexView::mnuSetFont);
    menu->addSeparator();
    addToggleActionToMenu(menu, QStringLiteral("Show A&ddress"), m_ShowAddress, this, &QHexView::setShowAddress);
    addToggleActionToMenu(menu, QStringLiteral("Show &Hex"), m_ShowHex, this, &QHexView::setShowHexDump);
    addToggleActionToMenu(menu, QStringLiteral("Show &Ascii"), m_ShowAscii, this, &QHexView::setShowAsciiDump);
    addToggleActionToMenu(menu, QStringLiteral("Show &Comments"), m_ShowComments, this, &QHexView::setShowComments);

    auto const wordMenu = new QMenu(QStringLiteral("Set Word Width"), this);
    for (const int bytes : {1, 2, 4, 8}) {
        const QString caption = (bytes == 1) ? QStringLiteral("1 Byte") : QStringLiteral("%1 Bytes").arg(bytes);
        addToggleActionToMenu(wordMenu, caption, m_WordWidth == bytes, this, [this, bytes]() {
            setWordWidth(bytes);
        });
    }

    auto const rowMenu = new QMenu(QStringLiteral("Set Row Width"), this);
    for (const int words : {1, 2, 4, 8, 16}) {
        const QString caption = (words == 1) ? QStringLiteral("1 Word") : QStringLiteral("%1 Words").arg(words);
        addToggleActionToMenu(rowMenu, caption, m_RowWidth == words, this, [this, words]() {
            setRowWidth(words);
        });
    }

    menu->addSeparator();
    menu->addMenu(wordMenu);
    menu->addMenu(rowMenu);

    menu->addSeparator();
    menu->addAction(QStringLiteral("&Copy Selection To Clipboard"), this, &QHexView::mnuCopy);

    return menu;
}

//------------------------------------------------------------------------------
// Name: contextMenuEvent(QContextMenuEvent *event)
// Desc: default context menu event, simply shows standard menu
//------------------------------------------------------------------------------
void QHexView::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *const menu = createStandardContextMenu();
    menu->exec(event->globalPos());
    delete menu;
}

//------------------------------------------------------------------------------
// Name:
// Desc:
//------------------------------------------------------------------------------
void QHexView::mnuCopy()
{
    if (hasSelectedText()) {
        QApplication::clipboard()->setText(QString::fromUtf8(selectedBytes()));
    }
}

//------------------------------------------------------------------------------
// Name: mnuSetFont()
// Desc: slot used to set the font of the widget based on dialog selector
//------------------------------------------------------------------------------
void QHexView::mnuSetFont()
{
    setHexFont(QFontDialog::getFont(nullptr, font(), this));
}

//------------------------------------------------------------------------------
// Name: clear()
// Desc: clears all data from the view
//------------------------------------------------------------------------------
void QHexView::clear()
{
    if (m_Data != nullptr) {
        m_Data->clear();
    }

    repaintView();
}

//------------------------------------------------------------------------------
// Name: hasSelectedText() const
// Desc: returns true if any text is selected
//------------------------------------------------------------------------------
bool QHexView::hasSelectedText() const
{
    return !(m_SelectionStart == -1 || m_SelectionEnd == -1);
}

//------------------------------------------------------------------------------
// Name: isInViewableArea(int index) const
// Desc: returns true if the word at the given index is in the viewable area
//------------------------------------------------------------------------------
bool QHexView::isInViewableArea(int index) const
{
    const int firstViewableWord = verticalScrollBar()->value() * m_RowWidth;
    const int viewableLines = viewport()->height() / m_FontHeight;
    const int viewableWords = viewableLines * m_RowWidth;
    const int lastViewableWord = firstViewableWord + viewableWords;

    return index >= firstViewableWord && index < lastViewableWord;
}

//------------------------------------------------------------------------------
// Name: keyPressEvent(QKeyEvent *event)
// Desc:
//------------------------------------------------------------------------------
void QHexView::keyPressEvent(QKeyEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        switch (event->key()) {
        case Qt::Key_A:
            selectAll();
            repaintView();
            break;
        case Qt::Key_Home:
            scrollTo(0);
            break;
        case Qt::Key_End:
            scrollTo(dataSize() - bytesPerRow());
            break;
        case Qt::Key_Down:

            do {
                int offset = verticalScrollBar()->value() * bytesPerRow();

                if (m_Origin != 0) {
                    if (offset > 0) {
                        offset += m_Origin;
                        offset -= bytesPerRow();
                    }
                }

                if (offset + 1 < dataSize()) {
                    scrollTo(offset + 1);
                }
            } while (false);

            // return so we don't pass on the key event
            return;
        case Qt::Key_Up:
            do {
                int offset = verticalScrollBar()->value() * bytesPerRow();

                if (m_Origin != 0) {
                    if (offset > 0) {
                        offset += m_Origin;
                        offset -= bytesPerRow();
                    }
                }

                if (offset > 0) {
                    scrollTo(offset - 1);
                }
            } while (false);

            // return so we don't pass on the key event
            return;
        }
    }

    QAbstractScrollArea::keyPressEvent(event);
}

//------------------------------------------------------------------------------
// Name: isPrintable(unsigned int ch)
// Desc: determines if a character has a printable ascii symbol
//------------------------------------------------------------------------------
bool QHexView::isPrintable(unsigned int ch)
{
    // if it's standard ascii use isprint, otherwise go with our observations
    if (ch < 128) {
        // 		return safe_ctype<std::isprint>(ch);
        return std::isprint(ch);
    } else {
        return (ch & 0xff) >= 0xa0;
    }
}

//------------------------------------------------------------------------------
// Name: line3() const
// Desc: returns the x coordinate of the 3rd line
//------------------------------------------------------------------------------
int QHexView::line3() const
{
    if (m_ShowAscii) {
        const int elements = bytesPerRow();
        return asciiDumpLeft() + (elements * m_FontWidth) + (m_FontWidth / 2);
    } else {
        return line2();
    }
}

//------------------------------------------------------------------------------
// Name: line2() const
// Desc: returns the x coordinate of the 2nd line
//------------------------------------------------------------------------------
int QHexView::line2() const
{
    if (m_ShowHex) {
        const int elements = m_RowWidth * (charsPerWord() + 1) - 1;
        return hexDumpLeft() + (elements * m_FontWidth) + (m_FontWidth / 2);
    } else {
        return line1();
    }
}

//------------------------------------------------------------------------------
// Name: line1() const
// Desc: returns the x coordinate of the 1st line
//------------------------------------------------------------------------------
int QHexView::line1() const
{
    if (m_ShowAddress) {
        const int elements = addressLen();
        return (elements * m_FontWidth) + (m_FontWidth / 2);
    } else {
        return 0;
    }
}

//------------------------------------------------------------------------------
// Name: hexDumpLeft() const
// Desc: returns the x coordinate of the hex-dump field left edge
//------------------------------------------------------------------------------
int QHexView::hexDumpLeft() const
{
    return line1() + (m_FontWidth / 2);
}

//------------------------------------------------------------------------------
// Name: asciiDumpLeft() const
// Desc: returns the x coordinate of the ascii-dump field left edge
//------------------------------------------------------------------------------
int QHexView::asciiDumpLeft() const
{
    return line2() + (m_FontWidth / 2);
}

//------------------------------------------------------------------------------
// Name: commentLeft() const
// Desc: returns the x coordinate of the comment field left edge
//------------------------------------------------------------------------------
int QHexView::commentLeft() const
{
    return line3() + (m_FontWidth / 2);
}

//------------------------------------------------------------------------------
// Name: charsPerWord() const
// Desc: returns how many characters each word takes up
//------------------------------------------------------------------------------
unsigned int QHexView::charsPerWord() const
{
    return m_WordWidth * 2;
}

//------------------------------------------------------------------------------
// Name: addressLen() const
// Desc: returns the lenth in characters the address will take up
//------------------------------------------------------------------------------
unsigned int QHexView::addressLen() const
{
    static const unsigned int addressLength = (sizeof(address_t) * CHAR_BIT) / 4;
    return addressLength + (m_ShowAddressSeparator ? 1 : 0);
}

//------------------------------------------------------------------------------
// Name: updateScrollbars()
// Desc: recalculates scrollbar maximum value base on lines total and lines viewable
//------------------------------------------------------------------------------
void QHexView::updateScrollbars()
{
    const unsigned int totalLines = dataSize() / bytesPerRow();
    const unsigned int viewableLines = viewport()->height() / m_FontHeight;

    unsigned int scrollMax = (totalLines > viewableLines) ? totalLines - 1 : 0;

    if (m_Origin != 0) {
        ++scrollMax;
    }

    verticalScrollBar()->setMaximum(scrollMax);
}

//------------------------------------------------------------------------------
// Name: scrollTo(unsigned int offset)
// Desc: scrolls view to given byte offset
//------------------------------------------------------------------------------
void QHexView::scrollTo(unsigned int offset)
{
    const int bpr = bytesPerRow();
    m_Origin = offset % bpr;
    address_t address = offset / bpr;

    updateScrollbars();

    if (m_Origin != 0) {
        ++address;
    }

    verticalScrollBar()->setValue(address);
    repaintView();
}

//------------------------------------------------------------------------------
// Name: setShowAddress(bool show)
// Desc: sets if we are to display the address column
//------------------------------------------------------------------------------
void QHexView::setShowAddress(bool show)
{
    m_ShowAddress = show;
    repaintView();
}

//------------------------------------------------------------------------------
// Name: setShowHexDump(bool show)
// Desc: sets if we are to display the hex-dump column
//------------------------------------------------------------------------------
void QHexView::setShowHexDump(bool show)
{
    m_ShowHex = show;
    repaintView();
}

//------------------------------------------------------------------------------
// Name: setShowComments(bool show)
// Desc: sets if we are to display the comments column
//------------------------------------------------------------------------------
void QHexView::setShowComments(bool show)
{
    m_ShowComments = show;
    repaintView();
}

//------------------------------------------------------------------------------
// Name: setShowAsciiDump(bool show)
// Desc: sets if we are to display the ascii-dump column
//------------------------------------------------------------------------------
void QHexView::setShowAsciiDump(bool show)
{
    m_ShowAscii = show;
    repaintView();
}

//------------------------------------------------------------------------------
// Name: setRowWidth(int rowWidth)
// Desc: sets the row width (units is words)
//------------------------------------------------------------------------------
void QHexView::setRowWidth(int rowWidth)
{
    m_RowWidth = rowWidth;

    updateScrollbars();
    repaintView();
}

//------------------------------------------------------------------------------
// Name: setWordWidth(int wordWidth)
// Desc: sets how many bytes represent a word
//------------------------------------------------------------------------------
void QHexView::setWordWidth(int wordWidth)
{
    m_WordWidth = wordWidth;

    updateScrollbars();
    repaintView();
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
unsigned int QHexView::bytesPerRow() const
{
    return m_RowWidth * m_WordWidth;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
int QHexView::pixelToWord(int x, int y) const
{
    int word = -1;

    switch (m_Highlighting) {
    case Highlighting_Data:
        // the right edge of a box is kinda quirky, so we pretend there is one
        // extra character there
        x = qBound(line1(), x, line2() + m_FontWidth);

        // the selection is in the data view portion
        x -= line1();

        // scale x/y down to character from pixels
        x /= m_FontWidth;
        y /= m_FontHeight;

        // make x relative to rendering mode of the bytes
        x /= (charsPerWord() + 1);
        break;
    case Highlighting_Ascii:
        x = qBound(asciiDumpLeft(), x, line3());

        // the selection is in the ascii view portion
        x -= asciiDumpLeft();

        // scale x/y down to character from pixels
        x /= m_FontWidth;
        y /= m_FontHeight;

        // make x relative to rendering mode of the bytes
        x /= m_WordWidth;
        break;
    default:
        Q_ASSERT(0);
        break;
    }

    // starting offset in bytes
    unsigned int startOffset = verticalScrollBar()->value() * bytesPerRow();

    // take into account the origin
    if (m_Origin != 0) {
        if (startOffset > 0) {
            startOffset += m_Origin;
            startOffset -= bytesPerRow();
        }
    }

    // convert byte offset to word offset, rounding up
    startOffset /= m_WordWidth;

    if ((m_Origin % m_WordWidth) != 0) {
        startOffset += 1;
    }

    word = ((y * m_RowWidth) + x) + startOffset;

    return word;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
void QHexView::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        const int x = event->position().x();
        const int y = event->position().y();
        if (x >= line1() && x < line2()) {
            m_Highlighting = Highlighting_Data;

            const int offset = pixelToWord(x, y);
            int byteOffset = offset * m_WordWidth;
            if (m_Origin) {
                if (m_Origin % m_WordWidth) {
                    byteOffset -= m_WordWidth - (m_Origin % m_WordWidth);
                }
            }

            m_SelectionStart = byteOffset;
            m_SelectionEnd = m_SelectionStart + m_WordWidth;
            repaintView();
        }
    }
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
void QHexView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        const int x = event->position().x();
        const int y = event->position().y();

        if (x < line2()) {
            m_Highlighting = Highlighting_Data;
        } else if (x >= line2()) {
            m_Highlighting = Highlighting_Ascii;
        }

        const int offset = pixelToWord(x, y);
        int byteOffset = offset * m_WordWidth;
        if (m_Origin) {
            if (m_Origin % m_WordWidth) {
                byteOffset -= m_WordWidth - (m_Origin % m_WordWidth);
            }
        }

        if (offset < dataSize()) {
            m_SelectionStart = m_SelectionEnd = byteOffset;
        } else {
            m_SelectionStart = m_SelectionEnd = -1;
        }
        repaintView();
    }
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
void QHexView::mouseMoveEvent(QMouseEvent *event)
{
    if (m_Highlighting != Highlighting_None) {
        const int x = event->position().x();
        const int y = event->position().y();

        const int offset = pixelToWord(x, y);

        if (m_SelectionStart != -1) {
            if (offset == -1) {
                m_SelectionEnd = (m_RowWidth - m_SelectionStart) + m_SelectionStart;
            } else {
                int byteOffset = (offset * m_WordWidth);

                if (m_Origin) {
                    if (m_Origin % m_WordWidth) {
                        byteOffset -= m_WordWidth - (m_Origin % m_WordWidth);
                    }
                }
                m_SelectionEnd = byteOffset;
            }

            if (m_SelectionEnd < 0) {
                m_SelectionEnd = 0;
            }

            if (!isInViewableArea(m_SelectionEnd)) {
                // TODO: scroll to an appropriate location
            }
        }
        repaintView();
    }
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
void QHexView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_Highlighting = Highlighting_None;
    }
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
void QHexView::setData(C *d)
{
    m_Data = d;

    deselect();
    updateScrollbars();
    repaintView();
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
void QHexView::resizeEvent(QResizeEvent *)
{
    updateScrollbars();
}

//------------------------------------------------------------------------------
// Name: setAddressOffset(address_t offset)
//------------------------------------------------------------------------------
void QHexView::setAddressOffset(address_t offset)
{
    m_AddressOffset = offset;
}

//------------------------------------------------------------------------------
// Name: isSelected(int index) const
//------------------------------------------------------------------------------
bool QHexView::isSelected(int index) const
{
    bool ret = false;
    if (index < static_cast<int>(dataSize())) {
        if (m_SelectionStart != m_SelectionEnd) {
            if (m_SelectionStart < m_SelectionEnd) {
                ret = (index >= m_SelectionStart && index < m_SelectionEnd);
            } else {
                ret = (index >= m_SelectionEnd && index < m_SelectionStart);
            }
        }
    }
    return ret;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
void QHexView::drawComments(QPainter &painter, unsigned int /*offset*/, unsigned int row) const
{
    painter.setPen(QPen(palette().text().color()));

    QString comment;

    // 	if(m_CommentServer != 0) {
    // 		comment = m_CommentServer->getComment(address, m_WordWidth);
    // 	}

    painter.drawText(commentLeft(), row, comment.length() * m_FontWidth, m_FontHeight, Qt::AlignTop, comment);
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
void QHexView::drawHexDump(QPainter &painter, unsigned int offset, unsigned int row, int &wordCount) const
{
    const C &dataRef(*m_Data);
    const int size = dataSize();

    // i is the word we are currently rendering
    for (int i = 0; i < m_RowWidth; ++i) {
        // index of first byte of current "word"
        const int index = offset + (i * m_WordWidth);

        // equal <=, not < because we want to test the END of the word we
        // about to render, not the start, it's allowed to end at the very last
        // byte
        if (index + m_WordWidth <= size) {
            union {
                quint64 q;
                quint32 d;
                quint16 w;
                quint8 b;
            } value = {0};

            QString byteBuffer;

            switch (m_WordWidth) {
            case 1:
                value.b |= dataRef[index + 0];
                byteBuffer = QString::asprintf("%02x", value.b);
                break;
            case 2:
                value.w |= dataRef[index + 0];
                value.w |= dataRef[index + 1] << 8;
                byteBuffer = QString::asprintf("%04x", value.w);
                break;
            case 4:
                value.d |= dataRef[index + 0];
                value.d |= dataRef[index + 1] << 8;
                value.d |= dataRef[index + 2] << 16;
                value.d |= dataRef[index + 3] << 24;
                byteBuffer = QString::asprintf("%08x", value.d);
                break;
            case 8:
                // we need the cast to ensure that it won't assume 32-bit
                // and drop bits shifted more that 31
                value.q |= static_cast<quint64>(dataRef[index + 0]);
                value.q |= static_cast<quint64>(dataRef[index + 1]) << 8;
                value.q |= static_cast<quint64>(dataRef[index + 2]) << 16;
                value.q |= static_cast<quint64>(dataRef[index + 3]) << 24;
                value.q |= static_cast<quint64>(dataRef[index + 4]) << 32;
                value.q |= static_cast<quint64>(dataRef[index + 5]) << 40;
                value.q |= static_cast<quint64>(dataRef[index + 6]) << 48;
                value.q |= static_cast<quint64>(dataRef[index + 7]) << 56;
                byteBuffer = QString::asprintf("%016llx", value.q);
                break;
            }

            const int drawLeft = hexDumpLeft() + (i * (charsPerWord() + 1) * m_FontWidth);

            if (isSelected(index)) {
                painter.fillRect(drawLeft, row, charsPerWord() * m_FontWidth, m_FontHeight, palette().highlight());

                // should be highlight the space between us and the next word?
                if (i != (m_RowWidth - 1)) {
                    if (isSelected(index + 1)) {
                        painter.fillRect(drawLeft + m_FontWidth, row, charsPerWord() * m_FontWidth, m_FontHeight, palette().highlight());
                    }
                }

                painter.setPen(QPen(palette().highlightedText().color()));
            } else {
                painter.setPen(QPen((wordCount & 1) ? m_EvenWord : palette().text().color()));
            }

            painter.drawText(drawLeft, row, byteBuffer.length() * m_FontWidth, m_FontHeight, Qt::AlignTop, byteBuffer);

            ++wordCount;
        } else {
            break;
        }
    }
}

//------------------------------------------------------------------------------
// Name: drawAsciiDump(QPainter &painter, unsigned int offset, unsigned int row) const
//------------------------------------------------------------------------------
void QHexView::drawAsciiDump(QPainter &painter, unsigned int offset, unsigned int row) const
{
    const C &dataRef(*m_Data);
    const int size = dataSize();

    // i is the byte index
    const int charsPerRow = bytesPerRow();

    for (int i = 0; i < charsPerRow; ++i) {
        const int index = offset + i;

        if (index < size) {
            const char ch = dataRef[index];
            const int drawLeft = asciiDumpLeft() + i * m_FontWidth;
            const bool printable = isPrintable(ch);

            // drawing a selected character
            if (isSelected(index)) {
                painter.fillRect(drawLeft, row, m_FontWidth, m_FontHeight, palette().highlight());

                painter.setPen(QPen(palette().highlightedText().color()));

            } else {
                painter.setPen(QPen(printable ? palette().text().color() : m_NonPrintableText));
            }

            const QString byteBuffer(QLatin1Char(printable ? ch : m_UnprintableChar));

            painter.drawText(drawLeft, row, m_FontWidth, m_FontHeight, Qt::AlignTop, byteBuffer);
        } else {
            break;
        }
    }
}

//------------------------------------------------------------------------------
// Name: paintEvent(QPaintEvent *)
//------------------------------------------------------------------------------
void QHexView::paintEvent(QPaintEvent *)
{
    QPainter painter(viewport());

    int wordCount = 0;

    // pixel offset of this row
    unsigned int row = 0;

    // current actual offset (in bytes)
    unsigned int offset = verticalScrollBar()->value() * bytesPerRow();

    if (m_Origin != 0) {
        if (offset > 0) {
            offset += m_Origin;
            offset -= bytesPerRow();
        } else {
            m_Origin = 0;
            updateScrollbars();
        }
    }

    while (row + m_FontHeight < static_cast<unsigned int>(height()) && offset < static_cast<unsigned int>(dataSize())) {
        if (m_ShowAddress) {
            const address_t addressRVA = m_AddressOffset + offset;
            const QString addressBuffer = formatAddress(addressRVA);
            painter.setPen(QPen(m_AddressColor));
            painter.drawText(0, row, addressBuffer.length() * m_FontWidth, m_FontHeight, Qt::AlignTop, addressBuffer);
        }

        if (m_ShowHex) {
            drawHexDump(painter, offset, row, wordCount);
        }

        if (m_ShowAscii) {
            drawAsciiDump(painter, offset, row);
        }

        if (m_ShowComments) {
            drawComments(painter, offset, row);
        }

        offset += bytesPerRow();
        row += m_FontHeight;
    }

    painter.setPen(QPen(palette().shadow().color()));

    if (m_ShowAddress && m_ShowLine1) {
        painter.drawLine(line1(), 0, line1(), height());
    }

    if (m_ShowHex && m_ShowLine2) {
        painter.drawLine(line2(), 0, line2(), height());
    }

    if (m_ShowAscii && m_ShowLine3) {
        painter.drawLine(line3(), 0, line3(), height());
    }
}

//------------------------------------------------------------------------------
// Name: selectAll()
//------------------------------------------------------------------------------
void QHexView::selectAll()
{
    m_SelectionStart = 0;
    m_SelectionEnd = dataSize();
}

//------------------------------------------------------------------------------
// Name: deselect()
//------------------------------------------------------------------------------
void QHexView::deselect()
{
    m_SelectionStart = -1;
    m_SelectionEnd = -1;
}

//------------------------------------------------------------------------------
// Name: allBytes() const
//------------------------------------------------------------------------------
QByteArray QHexView::allBytes() const
{
    QByteArray ret;
    const C &dataRef(*m_Data);
    const int size = dataSize();
    for (int i = 0; i < size; ++i) {
        ret.push_back(dataRef[i]);
    }
    return ret;
}

//------------------------------------------------------------------------------
// Name: selectedBytes() const
//------------------------------------------------------------------------------
QByteArray QHexView::selectedBytes() const
{
    QByteArray ret;
    const C &dataRef(*m_Data);
    const int size = dataSize();
    for (int i = 0; i < size; ++i) {
        if (isSelected(i)) {
            ret.push_back(dataRef[i]);
        }
    }

    return ret;
}

//------------------------------------------------------------------------------
// Name: selectedBytesAddress() const
//------------------------------------------------------------------------------
QHexView::address_t QHexView::selectedBytesAddress() const
{
    const address_t selectBase = qMin(m_SelectionStart, m_SelectionEnd);
    return selectBase + m_AddressOffset;
}

//------------------------------------------------------------------------------
// Name: selectedBytesSize() const
//------------------------------------------------------------------------------
unsigned int QHexView::selectedBytesSize() const
{
    unsigned int ret;
    if (m_SelectionEnd > m_SelectionStart) {
        ret = m_SelectionEnd - m_SelectionStart;
    } else {
        ret = m_SelectionStart - m_SelectionEnd;
    }

    return ret;
}

//------------------------------------------------------------------------------
// Name: addressOffset() const
//------------------------------------------------------------------------------
QHexView::address_t QHexView::addressOffset() const
{
    return m_AddressOffset;
}

//------------------------------------------------------------------------------
// Name: setCommentServer(CommentServerInterface *p)
//------------------------------------------------------------------------------
void QHexView::setCommentServer(CommentServerInterface *p)
{
    m_CommentServer = p;
}

//------------------------------------------------------------------------------
// Name: commentServer() const
//------------------------------------------------------------------------------
CommentServerInterface *QHexView::commentServer() const
{
    return m_CommentServer;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
bool QHexView::showHexDump() const
{
    return m_ShowHex;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
bool QHexView::showAddress() const
{
    return m_ShowAddress;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
bool QHexView::showAsciiDump() const
{
    return m_ShowAscii;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
bool QHexView::showComments() const
{
    return m_ShowComments;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
int QHexView::wordWidth() const
{
    return m_WordWidth;
}

//------------------------------------------------------------------------------
// Name:
//------------------------------------------------------------------------------
int QHexView::rowWidth() const
{
    return m_RowWidth;
}

#include "moc_QHexView.cpp"
