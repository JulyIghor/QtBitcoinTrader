// Copyright (C) 2013 July IGHOR.
// I want to create Bitcoin Trader application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form https://sourceforge.net/projects/bitcointrader/
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#include "julylightchanges.h"

JulyLightChanges::JulyLightChanges(QDoubleSpinBox *parent, QString colL, QString colH)
	: QObject()
{
	lastValue=0.0;
	colorL=colL;
	colorH=colH;
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
	parentSpinBox->setStyleSheet("QDoubleSpinBox:disabled{color:black; background: \"white\";} QDoubleSpinBox {color:black;background: \"white\";}");
}

void JulyLightChanges::valueChanged(double val)
{
	changeTimer->stop();
	if(lastValue<=val)
		parentSpinBox->setStyleSheet("QDoubleSpinBox:disabled{color:black; background: \""+colorH+"\";} QDoubleSpinBox {color:black;background: \""+colorH+"\";}");
	else
		parentSpinBox->setStyleSheet("QDoubleSpinBox:disabled{color:black; background: \""+colorL+"\";} QDoubleSpinBox {color:black;background: \""+colorL+"\";}");
	lastValue=val;
	changeTimer->start(2000);
}