//
// C++ Implementation: fmactivate
//
// Description: 
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "fmactivate.h"

#include "fontitem.h"
#include "typotek.h"

#include <QDebug>
#include <QFile>

FMActivate* FMActivate::instance = 0;

FMActivate * FMActivate::getInstance()
{
	if(!instance)
	{
		instance = new FMActivate;
	}
	Q_ASSERT(instance);
	return instance;
}

#ifdef APPLE
void FMActivate::activate(FontItem * fit, bool act)
{
	qDebug() << "Activation of " << fit->path() << act;
	if ( act )
	{

		if ( !fit->isLocked() )
		{
			if ( !fit->isActivated() )
			{
				fit->setActivated ( true );

// 				QFileInfo fofi ( fit->path() );

				if ( !QFile::copy ( fit->path() , typotek::getInstance()->getManagedDir() + "/" + fit->activationName() ) )
				{
					qDebug() << "unable to copy " << fit->path() ;
				}
				else
				{
					qDebug() << fit->path() << " copied" ;
					if ( !fit->afm().isEmpty() )
					{
						
// 						QFileInfo afm ( fit->afm() );
						if ( !QFile::copy( fit->afm(), typotek::getInstance()->getManagedDir() + "/" + fit->activationAFMName() ) )
						{
							qDebug() << "unable to copy " << fit->afm();
						}
						else
						{
							qDebug() << fit->afm() << "copied"; 
						}
					}
					else
					{
						qDebug()<<"There is no AFM file attached to "<<fit->path();
					}
				}
			}
			else
			{
				qDebug() << "\tYet activated";
			}

		}
		else
		{
			qDebug() << "\tIs Locked";
		}

	}
	else
	{

		if ( !fit->isLocked() )
		{
			if ( fit->isActivated() )
			{
				fit->setActivated ( false );
// 				QFileInfo fofi ( fit->path() );
				if ( !QFile::remove ( typotek::getInstance()->getManagedDir() + "/" + fit->activationName() ) )
				{
					qDebug() << "unable to unlink " << fit->name() ;
				}
				else
				{
					if ( !fit->afm().isEmpty() )
					{
// 						QFileInfo afm ( fit->afm() );
						if ( !QFile::remove ( typotek::getInstance()->getManagedDir() + "/" + fit->activationAFMName() ) )
						{
							qDebug() << "unable to unlink " << fit->afm() ;
						}
					}
// 					typo->adaptator()->private_signal ( 0, fofi.fileName() );
				}
			}

		}
		else
		{
			qDebug() << "\tIs Locked";
		}
	}
	
	emit activationEvent ( fit->path() );
}

#elif _WIN32
void FMActivate::activate(FontItem * fit, bool act)
{
	//TODO implement activation/deactivation for Windows
}

#else // fontconfig
void FMActivate::activate(FontItem * fit, bool act)
{
	qDebug() << "Activation of " << fit->path() << act;
	if ( act )
	{

		if ( !fit->isLocked() )
		{
			if ( !fit->isActivated() )
			{
				fit->setActivated ( true );

// 				QFileInfo fofi ( fit->path() );

				if ( !QFile::link ( fit->path() , typotek::getInstance()->getManagedDir() + "/" + fit->activationName() ) )
				{
					qDebug() << "unable to link " << fit->path() ;
				}
				else
				{
					qDebug() << fit->path() << " linked" ;
					if ( !fit->afm().isEmpty() )
					{
						
// 						QFileInfo afm ( fit->afm() );
						if ( !QFile::link ( fit->afm(), typotek::getInstance()->getManagedDir() + "/" + fit->activationAFMName() ) )
						{
							qDebug() << "unable to link " << fit->afm();
						}
						else
						{
							qDebug() << fit->afm() << " linked"; 
						}
					}
					else
					{
						qDebug()<<"There is no AFM file attached to "<<fit->path();
					}
				}
			}
			else
			{
				qDebug() << "\tYet activated";
			}

		}
		else
		{
			qDebug() << "\tIs Locked";
		}

	}
	else
	{

		if ( !fit->isLocked() )
		{
			if ( fit->isActivated() )
			{
				fit->setActivated ( false );
// 				QFileInfo fofi ( fit->path() );
				if ( !QFile::remove ( typotek::getInstance()->getManagedDir() + "/" + fit->activationName() ) )
				{
					qDebug() << "unable to unlink " << fit->name() ;
				}
				else
				{
					if ( !fit->afm().isEmpty() )
					{
// 						QFileInfo afm ( fit->afm() );
						if ( !QFile::remove ( typotek::getInstance()->getManagedDir() + "/" + fit->activationAFMName() ) )
						{
							qDebug() << "unable to unlink " << fit->afm() ;
						}
					}
// 					typo->adaptator()->private_signal ( 0, fofi.fileName() );
				}
			}

		}
		else
		{
			qDebug() << "\tIs Locked";
		}
	}
	
	emit activationEvent ( fit->path() );
// 	typo->save();
}

#endif
