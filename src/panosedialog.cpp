//
// C++ Implementation: panosedialog
//
// Description: 
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "panosedialog.h"
#include "fontitem.h"
#include "fmfontstrings.h"
#include "fmfontdb.h"

#include <QComboBox>
#include <QDebug>

FMPanoseDialog::FMPanoseDialog(FontItem * font, QWidget * parent)
	:QDialog(parent), m_font(font), m_ok(false)
{
	setupUi(this);
	Q_ASSERT(m_font);
	fontName->setText(m_font->fancyName());
	m_sourcepanose = FMFontDb::DB()->getValue(m_font->path(), FMFontDb::Panose).toString();
	populateDialog();
	
	connect(buttonBox, SIGNAL(accepted()), this, SLOT(closeOk()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(closeCancel()));
}

FMPanoseDialog::~ FMPanoseDialog()
{
	foreach(QString s, m_box.keys())
	{
		delete m_box[s];
	}
	foreach(QString s, m_label.keys())
	{
		delete m_label[s];
	}
}

void FMPanoseDialog::populateDialog()
{
	
	QStringList pl ( m_sourcepanose.split ( ":" ) );
	QList<int> l;
	if ( pl.count() == 10 )
	{
		foreach ( QString s, pl )
		{
			l << s.toInt();
		}
	}
	
	const QMap< FontStrings::PanoseKey, QMap<int, QString> >& pmap( FontStrings::Panose() ); 
	FontStrings::PanoseKey pk(FontStrings::firstPanoseKey());
	int pValue(0);
	while(pk != FontStrings::InvalidPK)
	{
		QString sk( FontStrings::PanoseKeyName(pk) );
		pValue = l.takeFirst();	
		m_label[sk] = new QLabel(sk, this);
		m_box[sk] = new QComboBox(this);
		
		
		int cIndex(0);
		foreach(int ik, pmap[pk].keys())
		{
			m_box[sk]->addItem( pmap[pk][ik], ik );
			if(ik == pValue)
				m_box[sk]->setCurrentIndex(cIndex);
			++cIndex;
			
		}
		vLayout->addWidget(m_label[sk]);
		vLayout->addWidget(m_box[sk]);
		
		pk = FontStrings::nextPanoseKey(pk);
	}
	
	foreach(QString sk, m_box.keys())
	{
		connect(m_box[sk], SIGNAL(currentIndexChanged (int)), this, SLOT(panoseChange(int)));
	}
}

void FMPanoseDialog::panoseChange(int index)
{
	QStringList l;
	const QMap< FontStrings::PanoseKey, QMap<int, QString> >& pmap( FontStrings::Panose() ); 
	FontStrings::PanoseKey pk(FontStrings::firstPanoseKey());
	
	while(pk != FontStrings::InvalidPK)
	{
		QString sk( FontStrings::PanoseKeyName(pk) );
		
		l << QString::number(m_box[sk]->itemData( m_box[sk]->currentIndex() ).toInt());
		
		pk = FontStrings::nextPanoseKey(pk);
	}
	m_targetpanose = l.join(":");
}

void FMPanoseDialog::closeOk()
{
	m_ok = true;
	close();
}

void FMPanoseDialog::closeCancel()
{
	close();
}


