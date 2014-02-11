// Copyright (C) 2014 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form http://qtopentrader.com
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#include "tradesitem.h"
#include "main.h"

TradesItem::TradesItem()
{
	backGray=false;
	displayFullDate=false;
	date=0;
	amount=0.0;
	price=0.0;
	total=0.0;
	orderType=0;
	direction=0;
}

void TradesItem::cacheStrings()
{
	QDateTime itemDate=QDateTime::fromTime_t(date);
	timeStr=itemDate.toString(baseValues.timeFormat);
	dateStr=itemDate.toString(baseValues.dateTimeFormat);
	if(price>0.0)priceStr=mainWindow.numFromDouble(price);
	if(amount>0.0)amountStr=mainWindow.numFromDouble(amount);
	if(amount>0.0&&price>0.0)totalStr=QString::number(price*amount,'f',baseValues.currentPair.currBBalanceDecimals);
}

bool TradesItem::isValid()
{
	bool valid=date>0&&price>0.0&&amount>0.0;
	if(valid)cacheStrings();
	return valid;
}