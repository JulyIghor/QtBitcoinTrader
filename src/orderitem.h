// Copyright (C) 2013 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form https://sourceforge.net/projects/bitcointrader/
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#ifndef ORDERITEM_H
#define ORDERITEM_H

struct OrderItem 
{
	QByteArray oid;
	qint64 date;
	bool type;//true=Ask, false=Bid
	int status;//0=Canceled, 1=Open, 2=Pending, 3=Post-Pending
	double amount;
	double price;
	QByteArray symbol;
	bool isValid(){return date>0&&price>0.0&&symbol.size()==6;}
};

#endif // ORDERITEM_H