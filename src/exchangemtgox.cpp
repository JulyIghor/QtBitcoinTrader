// Copyright (C) 2013 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form https://sourceforge.net/projects/bitcointrader/
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#include "exchangemtgox.h"
#include "main.h"

ExchangeMtGox::ExchangeMtGox(QObject *parent)
	: QThread(parent)
{
	http=0;
	moveToThread(this);
	start();
}

ExchangeMtGox::~ExchangeMtGox()
{
	if(http)delete http;
}

void ExchangeMtGox::run()
{
	http=new QHttp(this);
	connect(http,SIGNAL(done(bool)),this,SLOT(httpDone(bool)));
	exec();
}

void ExchangeMtGox::httpDone(bool error)
{

}