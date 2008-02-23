//
// C++ Interface: fmrepair
//
// Description: 
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef FMREPAIR_H
#define FMREPAIR_H

#include <ui_repair.h>
#include <QDialog>

class FmRepair : public QDialog, private Ui::repairDialog
{
	Q_OBJECT
	public:
		FmRepair(QWidget *parent);
		~FmRepair();
	private:
		void fillDeadLink();
		void fillActNotLinked();
		void fillDeactLinked();
		void fillLists();
		
		void doConnect();
	private slots:
		void slotSelAllDead();
		void slotRemoveDead();
		
		void slotSelAllActNotLinked();
		void slotRelinkActNotLinked();
		void slotDeactivateActNotLinked();
		
		void slotSelAllDeactLinked();
		void slotDelinkDeactLinked();
		void slotActivateDeactLinked();
	
};


#endif
