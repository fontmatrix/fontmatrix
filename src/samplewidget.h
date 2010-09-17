/***************************************************************************
 *   Copyright (C) 2010 by Pierre Marchand   *
 *   pierre@oep-h.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef SAMPLEWIDGET_H
#define SAMPLEWIDGET_H

#include "floatingwidget.h"
#include <QString>
#include <QByteArray>
#include "fmotf.h"

class QGraphicsScene;
class FMLayout;
class QButtonGroup;
class QAbstractButton;
class QFileSystemWatcher;
class QTimer;
class SampleToolBar;
class QTreeWidgetItem;
class QStyledItemDelegate;

namespace Ui {
	class SampleWidget;
}

#define VIEW_PAGE_FREETYPE 0
#define VIEW_PAGE_ABSOLUTE 1
#define VIEW_PAGE_OPENTYPE 3
//#define VIEW_PAGE_SETTINGS 1
#define VIEW_PAGE_SAMPLES  4

class SampleWidget : public FloatingWidget
{
	Q_OBJECT

public:
	struct State
	{
		bool set;
		State() : set(false) {}
		State(const QString& sn, double fs, unsigned int rh, const QString& sh, const QString& sc):
				set(true),
				sampleName(sn),
				fontSize(fs),
				renderHinting(rh),
				shaper(sh),
				script(sc)
		{}
		State(const State& other)
			: set(true)
		{
			sampleName = other.sampleName;
			fontSize = other.fontSize;
			renderHinting = other.renderHinting;
			shaper = other.shaper;
			script = other.script;
		}
		QString sampleName;
		double fontSize;
		unsigned int renderHinting; // 0 = No; 1 = Normal; 2 = Light
		QString shaper;
		QString script;
		QByteArray toByteArray() const;
		State fromByteArray(QByteArray b);

	private:
		State operator= (const State&){}
	};

	static const QString Name;
	explicit SampleWidget(const QString& fid, QWidget *parent = 0);
	~SampleWidget();

	QGraphicsScene* textScene() const;
	State state() const;
	void setState(const State& s);

protected:
	void changeEvent(QEvent *e);

	void refillSampleList();
	unsigned int hinting();

private:
	Ui::SampleWidget *ui;
	SampleToolBar * sampleToolBar;
	QTreeWidgetItem * uRoot;
	QTreeWidgetItem * newSampleName;
	QStyledItemDelegate * sampleNameEditor;

	const QString fontIdentifier;
	QGraphicsScene *loremScene;
	QGraphicsScene *ftScene;
	FMLayout *textLayoutVect;
	FMLayout *textLayoutFT;
	//    QButtonGroup *radioRenderGroup;
	QButtonGroup *radioFTHintingGroup;
	double sampleFontSize;
	double sampleInterSize;
	double sampleRatio;
	int toolPanelWidth;
	QFileSystemWatcher *sysWatcher;
	QTimer *reloadTimer;

	void createConnections();
	void removeConnections();

	void fillOTTree();
	OTFSet deFillOTTree();

	bool layoutForPrint;

	void reSize(double fSize, double lSize){sampleFontSize = fSize; sampleInterSize = lSize;}

private slots:
	void slotView(bool needDeRendering = false);
	//    void slotChangeViewPage(QAbstractButton* );
	//    void slotHintChanged(int);
	//    void slotChangeViewPageSetting(bool);
	void slotUpdateSView();
	void slotZoom(int z);
	void slotUpdateRView();
	void slotSampleChanged();
	void slotLiveFontSize(double);
	void slotFeatureChanged();
	void slotDefaultOTF();
	void slotResetOTF();
	void slotChangeScript();
	void slotProgressionChanged();
	void slotWantShape();
	void slotFileChanged(const QString&);
	void slotReload();
	void slotScriptChange();

	void slotAddSample();
	void slotSampleNameEdited(QWidget* w);
	void slotRemoveSample();
	void slotEditSample();
	void slotUpdateSample();

	void slotShowSamples(bool);
	void slotShowOpenType(bool);

	void slotPrint();

	void saveState();

signals:
	void stopLayout();
	void stateChanged();



};

#endif // SAMPLEWIDGET_H
