// Copyright (C) 2013 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form https://sourceforge.net/projects/bitcointrader/
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#include "julylightchanges.h"
#include <QApplication>
#include "main.h"

JulyLightChanges::JulyLightChanges(QDoubleSpinBox *parent)
	: QObject()
{
	lastValue=0.0;
	parentSpinBox=parent;
	setParent(parentSpinBox);
	changeTimer=new QTimer;
	connect(changeTimer,SIGNAL(timeout()),this,SLOT(changeTimerSlot()));
	changeTimer->setSingleShot(true);
	valueChanged(parentSpinBox->value());
	connect(parent,SIGNAL(valueChanged(double)),this,SLOT(valueChanged(double)));
}

JulyLightChanges::~JulyLightChanges()
{
	if(changeTimer)delete changeTimer;
}

void JulyLightChanges::changeTimerSlot()
{
	parentSpinBox->setStyleSheet("QDoubleSpinBox:disabled{color:"+baseValues.appTheme.black.name()+"; background: "+baseValues.appTheme.white.name()+";} QDoubleSpinBox {color:"+baseValues.appTheme.black.name()+";background: "+baseValues.appTheme.white.name()+";}");
}

void JulyLightChanges::valueChanged(double val)
{
	changeTimer->stop();
	if(lastValue<=val)
		parentSpinBox->setStyleSheet("QDoubleSpinBox:disabled{color:"+baseValues.appTheme.black.name()+"; background: \""+baseValues.appTheme.lightGreen.name()+"\";} QDoubleSpinBox {color:"+baseValues.appTheme.black.name()+";background: \""+baseValues.appTheme.lightGreen.name()+"\";}");
	else
		parentSpinBox->setStyleSheet("QDoubleSpinBox:disabled{color:"+baseValues.appTheme.black.name()+"; background: \""+baseValues.appTheme.lightRed.name()+"\";} QDoubleSpinBox {color:"+baseValues.appTheme.black.name()+";background: \""+baseValues.appTheme.lightRed.name()+"\";}");
	lastValue=val;
	changeTimer->start(2000);
}