// Copyright (C) 2013 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form https://sourceforge.net/projects/bitcointrader/
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#include "julyspinboxfix.h"
#include "main.h"

JulySpinBoxFix::JulySpinBoxFix(QDoubleSpinBox *parentSB, int minWid)
	: QObject()
{
	parentSB->setMaximumWidth(110);
	pMinimumWidth=minWid;
	spinMargin=30;
	if(parentSB->buttonSymbols()==QDoubleSpinBox::NoButtons)spinMargin=10;
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
	static QFontMetrics spinBoxFontMetrics(parentSpinBox->font());
	if(pMinimumWidth==0)
		parentSpinBox->setMinimumWidth(spinBoxFontMetrics.width(text)+spinMargin);
	else 
		parentSpinBox->setMinimumWidth(qMax(spinBoxFontMetrics.width(text)+spinMargin,pMinimumWidth));
}
