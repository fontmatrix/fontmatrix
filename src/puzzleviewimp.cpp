//
// C++ Implementation: puzzleviewimp
//
// Description:
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "puzzleviewimp.h"

#include <QDebug>

PuzzleViewImp::PuzzleViewImp ( const QString & iPath , QRgb patternColor)
	:pc(patternColor)
{
	QImage qimage( iPath );
	if ( !qimage.isNull() )
	{
		init(qimage);
	}
}

PuzzleViewImp::PuzzleViewImp ( const QImage & qimg , QRgb patternColor)
	:pc(patternColor)
{
	init(qimg);
}

PuzzleViewImp::~ PuzzleViewImp()
{
}

void PuzzleViewImp::init(const QImage & qimg)
{
// 	qDebug()<<"PuzzleViewImp::init"<<qimg.rect();
	const unsigned int iw(qimg.width());
	const unsigned int ih(qimg.height());
	clear();
	
// 	if(iw != ih)
// 	{
// 		qWarning()<<"Image is not a square, cannot process";
// 		return;
// 	}
	
	QList<double> sumrow;
	QList<double> sumcol;
	sumrow.clear();
	sumcol.clear();
	double sum(0);
	// sums of patterns by row
	for(unsigned int y(0);y < ih;++y)
	{
		sum = 0;
		for(int x(0);x < iw;++x)
		{
			if(qimg.pixel(x,y) == pc)
				sum += 1.0;
		}
		sumrow << sum;
	}
	// sums of patterns by column
	for(unsigned int x(0);x < iw;++x)
	{
		sum = 0;
		for(int y(0);y < ih;++y)
		{
			if(qimg.pixel(x,y) == pc)
				sum += 1.0;
		}
		sumcol << sum;
	}
	
	const unsigned int maxIdx(qMax(iw,ih));
	for(unsigned int i(0); i < maxIdx;++i)
	{
		if(i >= iw)
			append(SVect(sumrow[i], 0.0));
		else if(i >= ih)
			append(SVect(0.0 , sumcol[i]));
		else
			append(SVect(sumrow[i], sumcol[i]));
	}
}

QList< double > PuzzleViewImp::CompList(const PuzzleViewImp & other)
{
// 	qDebug()<<"PuzzleViewImp::CompList";
	QList<double> ret;
	if((!count()) || (count() != other.count()))
	{
		return ret;
	}
	const unsigned int c(count());
	for(unsigned int i(0);i < c;++i)
	{
		ret << at(i).squareDistance(other.at(i));
	}
	return ret;
}

double PuzzleViewImp::CompSum(const PuzzleViewImp & other)
{
// 	qDebug()<<"PuzzleViewImp::CompSum";
	double ret(0);
	if((!count()) || (count() != other.count()))
	{
		return -1.0;
	}
	const unsigned int c(count());
	for(unsigned int i(0);i < c;++i)
	{
		ret += at(i).squareDistance(other.at(i));
	}
	return ret;
}

double PuzzleViewImp::CompMean(const PuzzleViewImp & other)
{
// 	qDebug()<<"PuzzleViewImp::CompMean";
	double ret(0);
	if((!count()) || (count() != other.count()))
	{
// 		qDebug()<<"C"<< count()<<"O.C"<<other.count();
		return -1.0;
	}
	const unsigned int c(count());
	for(unsigned int i(0);i < c;++i)
	{
		ret += at(i).squareDistance(other.at(i));
	}
	return ret / static_cast<double>(c);
}

