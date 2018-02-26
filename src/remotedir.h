//
// C++ Interface: remotedir
//
// Description:
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef REMOTEDIR_H
#define REMOTEDIR_H

#include <QString>
#include <QStringList>
#include <QMap>
#include <QPixmap>
// #include <QThread>

class QBuffer;
class QByteArray;
// class QHttp;


/**
	@author Pierre Marchand <pierremarc@oep-h.com>
*/
class RemoteDir : public QObject 
{
		Q_OBJECT
	public:
	struct FontInfo
	{
		QString file;
		QString family;
		QString variant;
		QString type;
		QString info;
		QStringList tags;
		QPixmap pix;
		QString dump();
	};

		RemoteDir ( const QStringList &dirs );
		~RemoteDir();
		
		void run();
		QList<FontInfo> rFonts(){return m_fonts;}
		bool isReady(){return m_ready;}
	private:
		QStringList argDirs;
		bool m_ready;
// 		QList<QHttp*> https; // TODO Replace this code
		QList<QBuffer*> buffers;
		QMap<int, QString> rDirs;
		
		// < GET Id, <state (0 = error; 1 = downloading; 2 = finished) >
		QMap<int,  int > httpRequests;
		// < GET Id, buffer >
		QMap<int, QByteArray*> httpBuffers;
		// < GET Id, http >
// 		QMap<int, QHttp*> reverseHttp; // TODO Replace this code
		// < GET Id, path >
		QMap<int, QString> httpPaths;
		
		//< file, preview>
		QMap<QString, QByteArray*> pixmaps;
		QMap<int, int> pendingPixmaps;
		
		QList<FontInfo> m_fonts;
		
		void getPreviews();
		void eventEndDownload();
		
		bool stopperEndReq;
		bool stopperEndPreviews;
		bool stopper;
	private slots:
		void slotProgress(int done, int total);
		void slotEndReq(int id, bool error);
		void slotEndPreviews(int id, bool error);
	signals:
		void listIsReady();
		
};

#endif
