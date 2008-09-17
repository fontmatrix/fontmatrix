//
// C++ Interface: fmactivate
//
// Description: 
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
// 

#ifndef FMACTIVATE_H
#define FMACTIVATE_H

#include <QObject>
#include <QString>

class FontItem;

class FMActivate : public QObject
{
	Q_OBJECT
			
	FMActivate(){};
	static FMActivate *instance;
	
	public:
		static FMActivate* getInstance();
		
		void activate(FontItem* fit , bool act );
		void activate(QList<FontItem*> fitList , bool act );
		
	signals:
		void activationEvent(QString);
	
};

#endif
