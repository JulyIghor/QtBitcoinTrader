// Copyright (C) 2014 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form http://qtopentrader.com
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#ifndef HISTORYITEM_H
#define HISTORYITEM_H

#include <QObject>

class HistoryItem
{
public:
	HistoryItem();

	bool displayFullDate;
	quint32 dateTimeInt;
	quint32 dateInt;
	QString dateTimeStr;
	QString timeStr;
	QString description;

	double volume;
	QString volumeStr;

	double price;
	QString priceStr;

	double total;
	QString totalStr;

	QByteArray symbol;

	int type; //0=General, 1=Sell, 2=Buy, 3=Fee, 4=Deposit, 5=Withdraw

	void cacheStrings();

	bool isValid();
};

#endif // HISTORYITEM_H