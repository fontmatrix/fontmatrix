#include "openclosearrow.h"

OpenCloseArrow::OpenCloseArrow(QWidget *parent, bool open) :
    QLabel(parent),
    openingState(open)
{
	setOpText();
}

OpenCloseArrow::~OpenCloseArrow()
{

}


void OpenCloseArrow::mouseReleaseEvent(QMouseEvent *ev)
{
	openingState = !openingState;
	setOpText();
	emit openChanged(openingState);

}


void OpenCloseArrow::changeOpen(bool t)
{
	openingState = t;
	setOpText();
	emit openChanged(openingState);
}

void OpenCloseArrow::setOpText()
{
	if(openingState)
		setText("-");
	else
		setText("+");
}
