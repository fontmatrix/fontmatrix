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
#include <QMap>
#include <QHash>

class FontItem;

class FMActivate : public QObject
{
	Q_OBJECT
			
	FMActivate();
	static FMActivate *instance;
	
	enum Error
	{
		NO_LINK = 0,
		ALREADY_ACTIVE,
		NO_UNLINK,
		ALREADY_UNACTIVE,
		MISSING_AFM,
		ERROR
	};

	QHash<Error, QString> errorStrings;
	void setErrorStrings();

	public:
		static FMActivate* getInstance();
		
//		void activate(FontItem* fit , bool act );
		void activate(QList<FontItem*> fitList , bool act );
		QMap<QString,QString> errors();
		
	signals:
		void activationEvent(const QStringList&);
		
	private:
		/*
		Add and Remove fonts in ~/.config/fontconfig/fonts.conf
		with <selecfont><rejectfont><glob> sequence
		*/
		bool addFcReject(const QString& path);
		bool remFcReject(const QString& path);

		QMap<QString,QString> m_errors;
	
};

#endif
