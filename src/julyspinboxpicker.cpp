// Copyright (C) 2014 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form http://qtopentrader.com
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#include "julyspinboxpicker.h"
#include <QDesktopWidget>
#include <QApplication>
#include <QDoubleSpinBox>
#include <QMouseEvent>
#include <QtCore/qmath.h>
#include "main.h"

JulySpinBoxPicker::JulySpinBoxPicker(QDoubleSpinBox *parent, double *forceMinValue, double intMinV)
	: QLabel()
{
	internalMinimumValue=intMinV;
	if(internalMinimumValue>0.0&&forceMinValue==0)forceMinimumValue=&internalMinimumValue;
	else
	forceMinimumValue=forceMinValue;
	scrollDirection=0;
	parentSpinBox=parent;
	setScaledContents(true);
	setFixedSize(11,16);
	j_debugMode=false;
	setIcon(0);
	setCursor(QCursor(Qt::OpenHandCursor));
}

JulySpinBoxPicker::~JulySpinBoxPicker()
{

}

void JulySpinBoxPicker::setIcon(int state)
{
	static QPixmap mouse("://Resources/Mouse.png");
	static QPixmap mouseDrag("://Resources/MouseDrag.png");
	static QPixmap mouseDragLeftRight("://Resources/MouseDragLeftRight.png");
	static QPixmap mouseDragUpDown("://Resources/MouseDragUpDown.png");

	switch(state)
	{
	case -1: setPixmap(mouseDragUpDown); break;
	case 0: setPixmap(mouse); break;
	case 1: setPixmap(mouseDragLeftRight); break;
	case 2: setPixmap(mouseDrag); break;
	}
}

void JulySpinBoxPicker::mousePressEvent(QMouseEvent *event)
{
	event->accept();
	if(event->button()==Qt::LeftButton)
	{
		if(!j_debugMode)setCursor(QCursor(Qt::BlankCursor));
		j_cursorLastPos=QCursor::pos();
		j_cursorLastMove=QCursor::pos();
		j_isPressing=true;
		scrollDirection=0;
		setIcon(2);

		for(int n=0;n<QApplication::desktop()->screenCount();n++)
			if(QApplication::desktop()->screenGeometry(n).contains(j_cursorLastPos))
			{
				currentScreenRect=QApplication::desktop()->screenGeometry(n);
				break;
			}
		maximumValue=10.0;
		if(parentSpinBox->accessibleName()=="USD"||parentSpinBox->accessibleName()=="PRICE")maximumValue=baseValues.currentPair.currBInfo.valueSmall;
		if(parentSpinBox->accessibleName()=="BTC")maximumValue=baseValues.currentPair.currAInfo.valueSmall;
		if(forceMinimumValue)minimumValue=*forceMinimumValue;
		else minimumValue=qPow(0.1,parentSpinBox->decimals());
	}
}

void JulySpinBoxPicker::mouseReleaseEvent(QMouseEvent *event)
{
	if(event->button()==Qt::LeftButton)
	{
		j_isPressing=false;
		setIcon(0);
		QCursor::setPos(j_cursorLastPos);
		if(!j_debugMode)setCursor(QCursor(Qt::OpenHandCursor));
	}
	event->accept();
}

void JulySpinBoxPicker::mouseMoveEvent(QMouseEvent *event)
{
	if(j_isPressing)
	{
		int t_x=QCursor::pos().x();
		int t_y=QCursor::pos().y();
		int t_deltaX=t_x-j_cursorLastMove.x();
		int t_deltaY=t_y-j_cursorLastMove.y();
		j_cursorLastMove.setX(t_x);
		j_cursorLastMove.setY(t_y);
		if((qAbs(t_deltaX)<100)&&(qAbs(t_deltaY)<100))
		{
			if(scrollDirection==0)
			{
				if(qAbs(t_deltaX)>qAbs(t_deltaY))scrollDirection=1;
				else scrollDirection=-1;

				setIcon(scrollDirection);
			}
			if(scrollDirection==-1)
			{
				int tempValue=t_deltaX;
				t_deltaX=-t_deltaY;
				t_deltaY=tempValue;
			}

			double valueToChange=t_deltaX;
			if(QApplication::keyboardModifiers()==Qt::NoModifier)valueToChange/=100.0;
			valueToChange*=maximumValue;

			parentSpinBox->setValue(parentSpinBox->value()+valueToChange);
		}
		int t_deltaXY=100;
		if((t_x<t_deltaXY)||
			(t_y<t_deltaXY)||
			(t_x>currentScreenRect.right()-t_deltaXY)||
			(t_y>currentScreenRect.bottom()-t_deltaXY))
			QCursor::setPos(currentScreenRect.center());
	}
	event->accept();
}