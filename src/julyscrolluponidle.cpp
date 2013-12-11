// Copyright (C) 2013 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form https://sourceforge.net/projects/bitcointrader/
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#include "julyscrolluponidle.h"

JulyScrollUpOnIdle::JulyScrollUpOnIdle(QScrollBar *parent)
	: QObject(parent)
{
	scrollBar=parent;
	idleTimer=new QTimer(this);
	connect(idleTimer,SIGNAL(timeout()),this,SLOT(timeOut()));
	connect(scrollBar,SIGNAL(valueChanged(int)),this,SLOT(valueChanged(int)));
}

JulyScrollUpOnIdle::~JulyScrollUpOnIdle()
{

}

void JulyScrollUpOnIdle::timeOut()
{
	scrollBar->setValue(0);
}

void JulyScrollUpOnIdle::valueChanged(int val)
{
	if(val>0)idleTimer->start(30000);
	else idleTimer->stop();
}