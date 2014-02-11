// Copyright (C) 2014 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form http://qtopentrader.com
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#include "julyspinboxfix.h"
#include "main.h"

JulySpinBoxFix::JulySpinBoxFix(QDoubleSpinBox *parentSB, int minWid)
	: QObject()
{
	if(baseValues.forceDotInSpinBoxes)parentSB->setLocale(QLocale::English);
	else parentSB->setLocale(QLocale::Ukrainian);
	parentSB->setMaximumWidth(110);
	pMinimumWidth=minWid;
	spinMargin=30;
	if(parentSB->buttonSymbols()==QDoubleSpinBox::NoButtons)spinMargin=6;
	parentSpinBox=parentSB;
	valueChanged(parentSB->text());
	if(!parentSB->suffix().isEmpty())
	{
		pMinimumWidth=parentSB->minimumWidth();
		parentSB->setMinimumWidth(pMinimumWidth);
		valueChanged(parentSB->text());
    }
	connect(parentSB,SIGNAL(valueChanged(QString)),this,SLOT(valueChanged(QString)));
}

JulySpinBoxFix::~JulySpinBoxFix()
{
}

void JulySpinBoxFix::valueChanged(QString text)
{
	if(pMinimumWidth==0)
		parentSpinBox->setMinimumWidth(textFontWidth(text)+spinMargin);
	else 
		parentSpinBox->setMinimumWidth(qMax(textFontWidth(text)+spinMargin,pMinimumWidth));
}
