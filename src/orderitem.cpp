// Copyright (C) 2014 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form http://qtopentrader.com
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#include "orderitem.h"
#include "main.h"

bool OrderItem::isValid()
{
	bool isVal=date>0&&price>0.0&&symbol.size()==6;
	if(isVal)
	{
		dateStr=QDateTime::fromTime_t(date).toString(baseValues.dateTimeFormat);
		QString priceSign=baseValues.currencyMap.value(symbol.right(3),CurencyInfo("$")).sign;
		amountStr=baseValues.currencyMap.value(symbol.left(3),CurencyInfo("$")).sign+mainWindow.numFromDouble(amount);
		priceStr=priceSign+mainWindow.numFromDouble(price);
		total=price*amount;
		totalStr=priceSign+mainWindow.numFromDouble(total,baseValues.currentPair.currBDecimals);
	}
	return isVal;
}