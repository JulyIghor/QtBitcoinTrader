// Copyright (C) 2013 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form https://sourceforge.net/projects/bitcointrader/
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#include "historyitem.h"
#include "main.h"

HistoryItem::HistoryItem()
{
	displayFullDate=false;
	dateTimeInt=0;
	volume=0.0;
	price=0.0;
	total=0.0;
	type=0;
}

void HistoryItem::cacheStrings()
{
	QDateTime cachedDateTime=QDateTime::fromTime_t(dateTimeInt);
	dateTimeStr=cachedDateTime.toString(baseValues.dateTimeFormat);
	timeStr=cachedDateTime.toString(baseValues.timeFormat);
	cachedDateTime.setTime(QTime(0,0,0,0));
	dateInt=cachedDateTime.toTime_t();

	QString usdSign=baseValues.currencySignMap.value(symbol.right(3),"USD");
	if(price>0.0)priceStr=usdSign+mainWindow.numFromDouble(price);
	if(volume>0.0)volumeStr=baseValues.currencySignMap.value(symbol.left(3),"BTC")+mainWindow.numFromDouble(volume);
	if(volume>0.0&&price>0.0)totalStr=usdSign+mainWindow.numFromDouble(mainWindow.getValidDoubleForPercision(price*volume,8));
}

bool HistoryItem::isValid()
{
	bool valid=dateTimeInt>0&&symbol.size()==6;
	if(valid)cacheStrings();
	return valid;
}