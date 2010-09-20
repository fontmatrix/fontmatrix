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

#include "sampletoolbar.h"
#include "ui_sampletoolbar.h"
#include "fmfontstrings.h"

SampleToolBar::SampleToolBar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SampleToolBar)
{
    ui->setupUi(this);
//    setAutoFillBackground(true);

    connect(ui->liveSize, SIGNAL(valueChanged(double)), this, SIGNAL(SizeChanged(double)));
    connect(ui->sampleButton, SIGNAL(toggled(bool)), this, SIGNAL(SampleToggled(bool)));
    connect(ui->opentypeButton, SIGNAL(toggled(bool)), this, SIGNAL(OpenTypeToggled(bool)));
    connect(ui->languageCombo, SIGNAL(currentIndexChanged(int)), this, SIGNAL(ScriptSelected()));
}

SampleToolBar::~SampleToolBar()
{
    delete ui;
}

void SampleToolBar::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

double SampleToolBar::getFontSize() const
{
	return ui->liveSize->value();
}

void SampleToolBar::setFontSize(double fs)
{
	ui->liveSize->setValue(fs);
}

bool SampleToolBar::isChecked(Button b)
{
	if(b == SampleButton)
		return ui->sampleButton->isChecked();
	else if(b == OpenTypeButton)
		return ui->opentypeButton->isChecked();
	return false;
}

void SampleToolBar::toggle(Button b, bool c)
{
	if(b == SampleButton)
		ui->sampleButton->setChecked(c);
	else if(b == OpenTypeButton)
		ui->opentypeButton->setChecked(c);
}

void SampleToolBar::enableButton(Button b, bool c)
{
	if(b == SampleButton)
		ui->sampleButton->setEnabled(c);
	else if(b == OpenTypeButton)
		ui->opentypeButton->setEnabled(c);
}

void SampleToolBar::setScripts(const QStringList &ll)
{
	ui->languageCombo->addItem(tr("Select language"), QString("NOSHAPER"));
	foreach(QString l, ll)
	{
		ui->languageCombo->addItem(FontStrings::scriptTagName(l), l);
	}
	if(ll.isEmpty())
		ui->languageCombo->setEnabled(false);
}

QString SampleToolBar::getScript()
{
	QString ret(ui->languageCombo->itemData(ui->languageCombo->currentIndex()).toString());
	if(ret != QString("NOSHAPER"))
		return ret;
	return QString();
}
