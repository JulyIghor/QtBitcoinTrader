// Copyright (C) 2014 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form http://qtopentrader.com
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#ifndef CURRENCYINFO_H
#define CURRENCYINFO_H

#include <QObject>

struct CurencyInfo
{
	CurencyInfo(QByteArray defS="$"){sign=defS;}
	QByteArray name;
	QByteArray sign;
	double valueStep;
	double valueSmall;
	bool isValid()
	{
		return !name.isEmpty()&&!sign.isEmpty();
	}
};

#endif // CURRENCYINFO_H