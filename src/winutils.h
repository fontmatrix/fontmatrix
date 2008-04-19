// Thanks to jghali
#ifdef _WIN32

#include <windows.h>
#include <shlobj.h>

QString getWin32SystemFontDir()
{
	QString qstr;
	WCHAR dir[512];
	if ( SHGetSpecialFolderPathW(NULL, dir, CSIDL_FONTS, false) )
	{
		qstr = QString::fromUtf16((const unsigned short*) dir);
		if( !qstr.endsWith("\\") )
			qstr += "\\";
		qstr.replace( '\\', '/' );
	}
	return qstr;
}


#endif
