// Copyright (C) 2013 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form https://sourceforge.net/projects/bitcointrader/
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#ifndef HISTORYITEM_H
#define HISTORYITEM_H

struct HistoryItem 
{
	quint32 date;
	double volume;
	double price;
	QByteArray symbol;
	int type;//0=General, 1=Sell, 2=Buy, 3=Fee, 4=Deposit, 5=Withdraw
	bool isValid(){return date>0&&symbol.size()==6;}
};

#endif // HISTORYITEM_H