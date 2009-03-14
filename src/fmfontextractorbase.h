//
// C++ Interface: fmfontextractorbase
//
// Description:
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef FMFONTEXTRACTORBASE_H
#define FMFONTEXTRACTORBASE_H

#include <QIODevice>
#include <QString>
#include <QStringList>

class FMFontExtractorBase
{
	public:
		FMFontExtractorBase(){}
		virtual ~FMFontExtractorBase(){}
		
		
		/**
		 *        Indicate on which the extractor must operate
		 * @param  filePath file path of the resources container
		 * @return false if it failed to load the file
		 */
		virtual bool loadFile(const QString& filePath) = 0;
		
		/**
		 *        Expose which extensions an implementation can handle
		 * @return a list of file suffixes
		 */
		virtual QStringList extensions() = 0;
		
		/**
		 * 	Extract names of resources contained in the file to be extracted
		 * @return a list of names (identifiers)
		 */
		virtual QStringList list() = 0;
		
		/**
		 * 	Indicates type of resource
		 * @param name an identifier as returned by list()
		 * @return a suffix reflecting the type of the font
		 */
		virtual QString fontType ( const QString& name ) = 0;
		
		/**
		 *       Write one of the font to a device
		 * @param name an identifier as returned by list()
		 * @param openedDevice an opened QIODevice
		 * @return result of write operation
		 */
		virtual bool write ( const QString& name, QIODevice * openedDevice ) = 0;
		

};

#endif // FMFONTEXTRACTORBASE_H


