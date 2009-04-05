/*
fmpython_w.h
PythonQt wrapper

author - Pierre marchand

*/

#ifndef FMPYTHON_W_H
#define FMPYTHON_W_H

#include "PythonQt.h"
#include "fmsharestruct.h"

#include <QThread>

class typotek;
class FMFontDb;
class FontItem;
class FMPythonW : public QThread
{
		Q_OBJECT
		FMPythonW();
		~FMPythonW() {}
		static FMPythonW * instance;
		static const QStringList exposedClassesQOBJECT;
		static const QStringList exposedClassesCPP;
	public:
		static FMPythonW * getInstance();
		void runFile ( const QString& pyScript );
		void runString( const QString& pyScript );
		
	protected:
		void run();
		
	public slots:
		void nextFace();
		void previousFace();
		void nextFamily();
		void previousFamily();
		
		void updateTree();

		QString currentFontPath();
		QString currentFontFamily();
		QString currentFontStyle();
		QStringList currentFontNames();
		QList<FontItem*> currentFonts();

		FMFontDb*  DB();

		void Debug ( QVariant var );

	signals:
		void currentChanged();

	private:
		typotek* tk;
		void doConnect();
		void init();
		
		QString m_scriptAsString;
		
	private slots:
		void catchStdOut(const QString& s);
		void catchStdErr(const QString& s);

};


class FMPythonDecorator : public QObject
{
	Q_OBJECT
	public slots:
		/// RenderGlyph
		RenderedGlyph* new_RenderedGlyph(int g,int l,double xa,double ya,double xo,double yo,unsigned short c,bool b)
		{
			return new RenderedGlyph(g,l,xa,ya,xo,yo,c,b);
		}
		RenderedGlyph* new_RenderedGlyph()
		{
			return new RenderedGlyph();
		}
		void delete_RenderedGlyph(RenderedGlyph* rg)
		{
			delete rg;
		}
		
		/// GlyphList
		GlyphList* new_GlyphList()
		{
			return new GlyphList;
		}
		void delete_GlyphList(GlyphList *gl)
		{
			delete gl;
		}
		int Count(GlyphList* l)
		{
			return l->count();
		}
		const RenderedGlyph* At(GlyphList* l, int idx)
		{
			return &(l->at(idx));
		}
};

#endif //FMPYTHON_W_H

