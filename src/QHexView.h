/*
Copyright (C) 2006 Evan Teran
                   eteran@alum.rit.edu

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/


#ifndef QHEXVIEW_20060506_H_
#define QHEXVIEW_20060506_H_

#include <QAbstractScrollArea>
#include <QByteArray>
#include <QMap>
#include <QString>
// #include "Types.h"

#if defined(_MSC_VER)
  typedef __int8 int8_t;
  typedef unsigned __int8 uint8_t;
  typedef __int16 int16_t;
  typedef unsigned __int16 uint16_t;
  typedef __int32 int32_t;
  typedef unsigned __int32 uint32_t;
  typedef __int64 int64_t;
  typedef unsigned __int64 uint64_t;
#endif

class QMenu;
class ByteStream;
class CommentServerInterface;

class QHexView : public QAbstractScrollArea {
	Q_OBJECT
	
public:
	typedef QVector<uint8_t> C;
	typedef uint32_t address_t;
	
public:
	QHexView(QWidget * parent = 0);
	
public:
	void setCommentServer(CommentServerInterface *p);
	CommentServerInterface * commentServer() const;
		
protected:
	virtual void paintEvent(QPaintEvent * event);
	virtual void resizeEvent(QResizeEvent * event);
	virtual void mousePressEvent(QMouseEvent * event);
	virtual void mouseMoveEvent(QMouseEvent * event);
	virtual void mouseReleaseEvent(QMouseEvent * event);
	virtual void keyPressEvent(QKeyEvent * event);
	virtual void mouseDoubleClickEvent(QMouseEvent * event);
	virtual void contextMenuEvent(QContextMenuEvent * event);

public slots:
	void setShowAddress(bool);
	void setShowAsciiDump(bool);
	void setShowHexDump(bool);
	void setShowComments(bool);
	//void setLineColor(QColor);
	//void setAddressColor(QColor);
	void setWordWidth(int);
	void setRowWidth(int);
	void setFont(const QFont &font);
	void setShowAddressSeparator(bool value);
	void repaint();
		
public:
	address_t addressOffset() const;
	bool showHexDump() const;
	bool showAddress() const;
	bool showAsciiDump() const;
	bool showComments() const;
	QColor lineColor() const;
	QColor addressColor() const;
	int wordWidth() const;
	int rowWidth() const;
	
private:
	int m_RowWidth;			// amount of "words" per row
	int m_WordWidth;		// size of a "word" in bytes
	QColor m_AddressColor;	// colour of the address in display
	bool m_ShowHex;			// should we show the hex display?
	bool m_ShowAscii;		// should we show the ascii display?
	bool m_ShowAddress;		// should we show the address display?
	bool m_ShowComments;

public:
	void setData(C *d);
	void setAddressOffset(address_t offset);
	void scrollTo(unsigned int offset);
	
	address_t selectedBytesAddress() const;
	unsigned int selectedBytesSize() const;
	QByteArray selectedBytes() const;
	QByteArray allBytes() const;
	QMenu *createStandardContextMenu();

public slots:
	void clear();
	void selectAll();
	void deselect();
	bool hasSelectedText() const;
	void mnuSetFont();
	void mnuCopy();

private:
	void updateScrollbars();
	
	bool isSelected(int index) const;
	bool isInViewableArea(int index) const;
	
	int pixelToWord(int x, int y) const;
	
	unsigned int charsPerWord() const;
	int hexDumpLeft() const;
	int asciiDumpLeft() const;
	int commentLeft() const;
	unsigned int addressLen() const;
	int line1() const;
	int line2() const;
	int line3() const;

	unsigned int bytesPerRow() const;
	
	int dataSize() const;
	
	void drawAsciiDump(QPainter &painter, unsigned int offset, unsigned int row) const;
	void drawHexDump(QPainter &painter, unsigned int offset, unsigned int row, int &wordCount) const;
	void drawComments(QPainter &painter, unsigned int offset, unsigned int row) const;
	
	QString formatAddress(address_t address);
	
private:
	static bool isPrintable(unsigned int ch);
	static QAction *addToggleActionToMenu(QMenu *menu, const QString &caption, bool checked, QObject *reciever, const char *slot);
	
private:
	address_t m_Origin;
	address_t m_AddressOffset;		// this is the offset that our base address is relative to
	int m_SelectionStart;			// index of first selected word (or -1)
	int m_SelectionEnd;				// index of last selected word (or -1)
	int m_FontWidth;				// width of a character in this font
	int m_FontHeight;				// height of a character in this font
	C *m_Data;						// the current data
	
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
	bool m_ShowAddressSeparator;	// should we show ':' character in address to seperate high/low portions
	char m_AddressFormatString[32];
	
	CommentServerInterface *m_CommentServer;
};

#endif
