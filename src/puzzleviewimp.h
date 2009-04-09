//
// C++ Interface: puzzleviewimp
//
// Description:
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef PUZZLEVIEWIMP_H
#define PUZZLEVIEWIMP_H

#include <QList>
#include <QString>
#include <QImage>

class SVect
{
	public:
		SVect() :x ( 0.0 ),y ( 0.0 ) {}
		SVect ( double xx, double yy ) :x ( xx ),y ( yy ) {}
		SVect ( const SVect& other ) {this->x = other.x;this->y = other.y;}
		SVect& operator= ( const SVect& other ) {this->x = other.x;this->y = other.y; return *this;}

		inline double squareDistance ( const SVect& other ) const
		{
			double C1 ( other.x - x );
			double C2 ( other.y - y );

			return ( C1 * C1 ) + ( C2 * C2 );
		}

	private:
		double x;
		double y;
};

class PuzzleViewImp : public QList<SVect>
{
		PuzzleViewImp() {}
		
	public:
		PuzzleViewImp ( const QString& iPath, QRgb patternColor  );
		PuzzleViewImp ( const QImage& qimg,  QRgb patternColor  );
		~PuzzleViewImp();

		QList<double> 	CompList ( const PuzzleViewImp& other );
		double		CompSum ( const PuzzleViewImp& other );
		double		CompMean ( const PuzzleViewImp& other );

	private:
		QRgb pc;
		void init ( const QImage& qimg );

};

#endif // PUZZLEVIEWIMP_H

