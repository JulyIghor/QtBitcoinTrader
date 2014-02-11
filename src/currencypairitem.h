// Copyright (C) 2014 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form http://qtopentrader.com
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#ifndef CURRENCYPAIRITEM_H
#define CURRENCYPAIRITEM_H

#include "currencyinfo.h"

struct CurrencyPairItem
{
	CurrencyPairItem();

	void setSymbol(QByteArray symb);

	QString currAName;
	QString name;

	CurencyInfo currAInfo;
	CurencyInfo currBInfo;

	QByteArray currSymbol;
	QByteArray currRequestPair;
	QByteArray currRequestSecond;

	QByteArray currAStr;
	QByteArray currAStrLow;
	QByteArray currASign;

	QByteArray currBStr;
	QByteArray currBStrLow;
	QByteArray currBSign;

	double priceMin;//qPow(0.1,priceDecimals);
	//double priceMax;
	double tradePriceMin;
	double tradeVolumeMin;

	int priceDecimals;
	int currADecimals;
	int currBDecimals;
	int currABalanceDecimals;
	int currBBalanceDecimals;
};

#endif // CURRENCYPAIRITEM_H
