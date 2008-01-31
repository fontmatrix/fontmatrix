//
// C++ Interface: importtags
//
// Description:
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef IMPORTTAGS_H
#define IMPORTTAGS_H

#include <QDialog>
#include <ui_importtags.h>

/**
	@author Pierre Marchand <pierremarc@oep-h.com>
*/
class ImportTags : public QDialog, private Ui::ImportTagsDialog
{
	Q_OBJECT
	public:
		ImportTags ( QWidget * parent, QStringList tags );
		~ImportTags();
		
		
		QStringList tags(){return m_tags;}
	private:
		QStringList m_tags;
		
	private slots:
		void slotNewTag();
		void slotEnd();
};

#endif
