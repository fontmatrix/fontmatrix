/*
fmpython_w.h
PythonQt wrapper

author - Pierre marchand

*/

#ifndef FMPYTHON_W_H
#define FMPYTHON_W_H

#include "PythonQt.h"

class typotek;
class FMPythonW : public QObject
{
		Q_OBJECT
		FMPythonW();
		~FMPythonW() {}
		static FMPythonW * instance;
	public:
		static FMPythonW * getInstance();
		
		void run(const QString& pyScript);

	public slots:
		void nextFace();
		void previousFace();
		void nextFamily();
		void previousFamily();

		QString currentFontPath();
		QString currentFontFamily();
		QString currentFontStyle();

	signals:
		void currentChanged();

	private:
		typotek* tk;
		PythonQtObjectPtr mainContext;
		void doConnect();
		void init();

};

#endif //FMPYTHON_W_H

