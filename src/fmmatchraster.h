//
// C++ Interface: fmmatchraster
//
// Description: 
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef FMMATCHRASTER_H
#define FMMATCHRASTER_H

#include "ui_matchraster.h"
#include <QRect>
#include <QMap>

class FontItem;

class FMMatchRaster : public QDialog , private Ui::MatchRasterDialog
{
	Q_OBJECT
	public:
		FMMatchRaster(QWidget * parent);
		~FMMatchRaster();
	private:
		QRect curRect;
		QRgb curCol;
// 		QMap<unsigned int, QImage> mItems;
		unsigned int refCodepoint;
		QImage refImage;
		
		int m_compsize;
		int m_minRefSize;
		bool m_waitingForButton;
		int m_progressValue;
		double m_matchLimit;
		
		QList<FontItem*> compFonts;
		QList<FontItem*> remainFonts;
		QList<FontItem*> filteredFonts;
		FontItem* waitingFont;
		
		QImage autoCrop(const QImage& img);
		
	private slots:
		void browseImage();
		void loadImage();
		void addImage(const QString & text);
		void search();
		
		void slotAcceptFont();
		void slotRefuseFont();
		
		void slotStop();
		
		void recordCurrentRect(QRect);
		void recordCurrentColor(QRgb);
		
};

#endif // FMMATCHRASTER_H

