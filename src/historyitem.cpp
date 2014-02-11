// Copyright (C) 2014 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form http://qtopentrader.com
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

	QString usdSign=baseValues.currencyMap.value(symbol.right(3),CurencyInfo("$")).sign;
	if(price>0.0)priceStr=usdSign+mainWindow.numFromDouble(price);
	if(volume>0.0)
	{
		volumeStr=baseValues.currencyMap.value(symbol.left(3),CurencyInfo("BTC")).sign+mainWindow.numFromDouble(volume);
	}
	if(volume>0.0&&price>0.0)
	{
		QString totalStrDown=mainWindow.numFromDouble(mainWindow.getValidDoubleForPercision(price*volume,8,false));
		QString totalStrUp=mainWindow.numFromDouble(mainWindow.getValidDoubleForPercision(price*volume,8,true));
		totalStr=usdSign+(totalStrDown.length()>totalStrUp.length()?totalStrUp:totalStrDown);

		if(!baseValues.forceDotInSpinBoxes)
		{
			priceStr.replace(".",",");
			volumeStr.replace(".",",");
			totalStr.replace(".",",");
		}
	}
}

bool HistoryItem::isValid()
{
	bool valid=dateTimeInt>0&&symbol.size()==6;
	if(valid)cacheStrings();
	return valid;
}