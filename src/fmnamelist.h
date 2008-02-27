//
// C++ Interface: fmnamelist
//
// Description: 
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef FMNAMELIST_H
#define FMNAMELIST_H

#include <QTreeWidget>
#include <QString>

class FMNameList : public QTreeWidget
{
	Q_OBJECT
	public:
		FMNameList(QWidget *parent);
		~FMNameList();
	protected:
		void keyPressEvent ( QKeyEvent * e );
// 	private:
// 		QString curString;
};

#endif

