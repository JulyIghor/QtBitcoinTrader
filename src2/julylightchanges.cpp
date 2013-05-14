//Created by July IGHOR
//http://trader.uax.co
//Bitcoin Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc

#include "julylightchanges.h"

JulyLightChanges::JulyLightChanges(QDoubleSpinBox *parent, QString colL, QString colH)
	: QObject()
{
	lastValue=0.0;
	colorL=colL;
	colorH=colH;
	parentSheet=parent;
	setParent(parent);
	changeTimer=new QTimer;
	connect(changeTimer,SIGNAL(timeout()),this,SLOT(changeTimerSlot()));
	changeTimer->setSingleShot(true);
	connect(parent,SIGNAL(valueChanged(double)),this,SLOT(valueChanged(double)));
}

JulyLightChanges::~JulyLightChanges()
{
	if(changeTimer)delete changeTimer;
}

void JulyLightChanges::changeTimerSlot()
{
	parentSheet->setStyleSheet("");
}//Please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc

void JulyLightChanges::valueChanged(double val)
{
	changeTimer->stop();
	if(lastValue<=val)
		parentSheet->setStyleSheet("background: \""+colorH+"\"");
	else
		parentSheet->setStyleSheet("background: \""+colorL+"\"");
	lastValue=val;
	changeTimer->start(2000);
}