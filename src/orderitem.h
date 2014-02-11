// Copyright (C) 2014 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form http://qtopentrader.com
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#ifndef ORDERITEM_H
#define ORDERITEM_H
#include <QByteArray>
#include <QString>

struct OrderItem 
{
	QByteArray oid;
	quint32 date;
	QString dateStr;
	bool type;//true=Ask, false=Bid
	int status;//0=Canceled, 1=Open, 2=Pending, 3=Post-Pending
	double amount;
	QString amountStr;
	double price;
	QString priceStr;
	double total;
	QString totalStr;
	QByteArray symbol;
	bool isValid();
};

#endif // ORDERITEM_H