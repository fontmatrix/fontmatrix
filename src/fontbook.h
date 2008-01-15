//
// C++ Interface: fontbook
//
// Description:
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef FONTBOOK_H
#define FONTBOOK_H

#include <QDomDocument>

/**
	@author Pierre Marchand <pierremarc@oep-h.com>

	It’s time to write something more "definitive" :)
*/

struct TextElementStyle
{
	QString name;
	QString font; // _FONTMATRIX_ is reserved
	double fontsize;
	double lineheight;
	double margin_top,margin_left,margin_bottom,margin_right;
	TextElementStyle () {}
	TextElementStyle ( QString n, QString f, double fs, double lh, double mt, double ml, double mb, double mr ) :
			name(n),
			font ( f ),
			fontsize ( fs ),
			lineheight ( lh ),
			margin_top ( mt ),
			margin_left ( ml ),
			margin_bottom ( mb ),
			margin_right ( mr ) {}
};

struct TextElement
{
	QString e;
	/**
	Has to be set if "e" must be substituted with a contextual info
	available infos depend of level and are :
	- Family
	- SubFamily
	- Encoding
	- PageNumber
	- ...
	*/
	bool internal;
	TextElement(){}
	TextElement ( QString elem, bool i, bool f ) :e ( elem ), internal ( i ){}
};

struct FontBookContext
{
	TextElement textElement;
	TextElementStyle textStyle;
/*	
	enum FBCLevel{PAGE, FAMILY, SUBFAMILY};
	
	FBCLevel level;*/
	
};

class FontBook
{
	public:
		FontBook();

		~FontBook();
		void doBook();
	private:
		
		QString outputFilePath;
		void doBookFromTemplate ( const QDomDocument &aTemplate );
};

#endif
