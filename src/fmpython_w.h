/*
fmpython_w.h
PythonQt wrapper

author - Pierre marchand

*/

#ifndef FMPYTHON_W_H
#define FMPYTHON_W_H

#include "PythonQt.h"

class typotek;
class FMFontDb;
class FMPythonW : public QObject
{
		Q_OBJECT
		FMPythonW();
		~FMPythonW() {}
		static FMPythonW * instance;
		static const QStringList exposedClasses;
	public:
		static FMPythonW * getInstance();
		void runFile ( const QString& pyScript );
		void runString( const QString& pyScript );

	public slots:
		void nextFace();
		void previousFace();
		void nextFamily();
		void previousFamily();

		QString currentFontPath();
		QString currentFontFamily();
		QString currentFontStyle();

		FMFontDb*  DB();

		void Debug ( QVariant var );

	signals:
		void currentChanged();

	private:
		typotek* tk;
		void doConnect();
		void init();
		
	private slots:
		void catchStdOut(const QString& s);
		void catchStdErr(const QString& s);

};

#endif //FMPYTHON_W_H

