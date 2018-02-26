//
// C++ Implementation: remotedir
//
// Description: 
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "remotedir.h"
#include "typotek.h"

// #include <QHttp>
#include <QByteArray>
#include <QBuffer>
#include <QUrl>
#include <QDomDocument>
#include <QWaitCondition>
#include <QDebug>

extern QWaitCondition remoteDirsCond;

RemoteDir::RemoteDir(const QStringList &dirs)
	: argDirs(dirs), m_ready(false)
{
	qDebug()<<"RemoteDir::RemoteDir("<<dirs.join(";")<<")";
	if (argDirs.isEmpty())
		m_ready = true;
	
	stopper = stopperEndPreviews = stopperEndReq = false;
}

void RemoteDir::run()
{
	qDebug()<<"RemoteDir::run()";
	for(int ridx(0); ridx < argDirs.count(); ++ridx)
	{
		QByteArray *ba = new QByteArray;
		QBuffer *buffer = new QBuffer;
		buffers << buffer;
		buffer->setBuffer(ba);
		buffer->open(QIODevice::WriteOnly);
		
		QUrl url(argDirs[ridx]);
#if 0 // TODO Replace this part of code
		QHttp *rd = new QHttp(url.host());
		rd->setObjectName(argDirs[ridx]);
		https << rd;
		
		connect(rd,SIGNAL(requestFinished( int, bool )),this,SLOT(slotEndReq(int, bool)));
// 		connect(rd,SIGNAL(dataReadProgress( int, int )),this,SLOT(slotProgress(int, int)));
		
		int rdId(rd->get(url.path()+"/fontmatrix.data", buffer));
		typotek::getInstance()->showStatusMessage(tr("Downloading")+" " + url.toString() + "/fontmatrix.data");
		rDirs[rdId] = argDirs[ridx];
		httpRequests[rdId] = 1;
		httpBuffers[rdId] = ba;
		reverseHttp[rdId] = rd;
		httpPaths[rdId] = url.path();
#endif
	}
}


RemoteDir::~RemoteDir()
{
#if 0 // TODO Replace this code
	foreach(QHttp *h, https)
	{
		delete h;
	}
#endif
	foreach(QBuffer *b, buffers)
	{
		delete b;
	}
	for(QMap<int, QByteArray*>::iterator ba = httpBuffers.begin(); ba != httpBuffers.end(); ++ba)
		delete ba.value();
}

void RemoteDir::slotEndPreviews(int id, bool error)
{
// 	qDebug()<<"RemoteDir::slotEndPreviews("<< id<<", "<<error<<")";
	if(stopperEndPreviews)
		return;
	if(error)
		pendingPixmaps[id] = 0;
	else
		pendingPixmaps[id] = 2;
	
	int pendingReqs(0);
#if 0 // TODO Replace this code
	for(int i(0);i < https.count(); ++i)
	{
		if (https[i]->hasPendingRequests())
		{
			++pendingReqs;
		}
		else
		{
			https[i]->close();
		}
	}
#endif
	if(!pendingReqs)
	{
		qDebug() <<"Get all previews";
		stopperEndPreviews = true;
		eventEndDownload();
	}
	
}

void RemoteDir::slotEndReq(int id, bool error)
{
	qDebug()<<"RemoteDir::slotEndReq("<< id<<", "<<error<<")";
	if(stopperEndReq)
		return;
	if(error)
		httpRequests[id] = 0;
	else
		httpRequests[id] = 2;
	
	int ih(0);
	bool hFound = false;
#if 0 // TODO Replace this code
	for(;ih < https.count();++ih)
	{
		if(sender() == https[ih])
		{
			hFound = true;
			break;
		}
	}
#endif
	if(!hFound)
	{
		qDebug()<< "Oops - Can’t determine which Http object called me";
		return;
	}
#if 0 // TODO Replace this code
	disconnect( https[ih],SIGNAL(requestFinished( int, bool )),this,SLOT(slotEndReq(int, bool)));
#endif
	int pendingReqs(0);
#if 0 // TODO Replace this code
	for(int i(0);i < https.count(); ++i)
	{
		if (https[i]->hasPendingRequests())
		{
			++pendingReqs;
		}
// 		else
// 		{
// 			https[i]->close();
// 		}
	}
#endif
	if(!pendingReqs)
	{
		stopperEndReq = true;
		getPreviews();
	}
}

void RemoteDir::eventEndDownload()
{
	if(stopper)
		return;
	QMap<int, QByteArray*>::const_iterator bIt;
	for(bIt = httpBuffers.begin(); bIt != httpBuffers.end(); ++bIt)
	{
		if(httpRequests[bIt.key()] == 0)
			continue;
		QString path(rDirs[bIt.key()]);
		qDebug()<< "Path("<<bIt.key()<<")->"<< path;
		QDomDocument doc ( "fontdata" );
		doc.setContent(*(bIt.value()));
		//loading fonts
		QDomNodeList colList = doc.elementsByTagName ( "fontfile" );
		for ( uint i = 0; i < colList.length(); ++i )
		{
			QDomNode col = colList.item ( i );
			
			FontInfo fi;

			fi.family = col.toElement().attributeNode("family").value();
			fi.variant = col.toElement().attributeNode("variant").value();
			fi.type = col.toElement().attributeNode("type").value();
			QString basename(col.namedItem ( "file" ).toElement().text());
			fi.file  = path + basename;
			fi.info = col.namedItem ( "info" ).toElement().text();
			if(pixmaps.contains(basename))
			{
				fi.pix = QPixmap::fromImage (QImage::fromData( (const uchar*)pixmaps[basename]->data(), pixmaps[basename]->count() ));
			}
			else
			{
				qDebug() << "No pixmap for " + fi.file;
				fi.pix = QPixmap();
			}

			QDomNodeList taglist = col.toElement().elementsByTagName ( "tag" );
			fi.tags.clear();
			for(int ti = 0; ti < taglist.count(); ++ti)
			{
				if(!fi.tags.contains(taglist.at(ti).toElement().text()))
					fi.tags  << taglist.at(ti).toElement().text();
			}
			
			m_fonts << fi;
		}
	}
	m_ready = true;
	stopper = true;
	emit listIsReady();
}

void RemoteDir::getPreviews()
{
	QMap<int, QByteArray*>::const_iterator bIt;
	for(bIt = httpBuffers.begin(); bIt != httpBuffers.end(); ++bIt)
	{
		if(httpRequests[bIt.key()] == 0)
			continue;
		
		QDomDocument doc ( "fontdata" );
		doc.setContent(*(bIt.value()));
		QDomNodeList colList = doc.elementsByTagName ( "fontfile" );
		for ( uint i = 0; i < colList.length(); ++i )
		{
			QDomNode col = colList.item ( i );
			QString p = col.namedItem ( "file" ).toElement().text();
			
			QByteArray *ba = new QByteArray;
			QBuffer *buffer = new QBuffer;
			buffers << buffer;
			buffer->setBuffer(ba);
			buffer->open(QIODevice::WriteOnly);
			pixmaps[p] = ba;
			
#if 0 // TODO Replace this code
			connect(reverseHttp[bIt.key()],SIGNAL(requestFinished( int, bool )),this,SLOT(slotEndPreviews(int, bool)));
			int rdId(reverseHttp[bIt.key()]->get(httpPaths[bIt.key()]+"/"+ p + ".png", buffer));
			pendingPixmaps[rdId] = 1; 
// 			qDebug() << "Started download of " << httpPaths[bIt.key()]+"/"+ p + ".png";
			typotek::getInstance()->showStatusMessage(tr("Downloading") +" "+ httpPaths[bIt.key()]+"/"+ p + ".png");
#endif
		}
	}
}


void RemoteDir::slotProgress(int done, int total)
{
// 	qDebug()<<"RemoteDir::slotProgress(int done, int total)";
	int ih(0);
	bool hFound = false;
#if 0 // TODO Replace this code
	for(;ih < https.count();++ih)
	{
		if(sender() == https[ih])
		{
			hFound = true;
			break;
		}
	}
#endif
	if(!hFound)
	{
		qDebug()<< "Oops - Can’t determine which Http object called me";
		return;
	}
#if 0
	QString file(https[ih]->objectName()); // TODO Replace this code
	qDebug()<< file <<" [" <<done << "/"<< total<<"]";
#endif
}

/// FontInfo **********************************************
QString RemoteDir::FontInfo::dump()
{
	QString sep(" | ");
	return file + sep + family + sep + variant + sep + type + sep +tags.join(sep);
}



