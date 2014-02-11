// Copyright (C) 2014 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form http://qtopentrader.com
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#ifndef TRADESITEM_H
#define TRADESITEM_H

#include <QObject>

struct TradesItem
{
	TradesItem();
	bool backGray;

	bool displayFullDate;
	quint32 date;
	QString dateStr;
	QString timeStr;

	double amount;
	QString amountStr;

	double price;
	QString priceStr;

	double total;
	QString totalStr;

	QByteArray symbol;//Like a "BTCUSD" 6 symbols only

	int orderType;//-1:Bid; 0:None; 1:Ask

	int direction;//-1:Down; 0: None; 1:Up

	void cacheStrings();

	bool isValid();
};

#endif // TRADESITEM_H
