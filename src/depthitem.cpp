// Copyright (C) 2014 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form http://qtopentrader.com
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#include "depthitem.h"
#include "main.h"

bool DepthItem::isValid()
{
	bool valid=price>=0.0&&volume>=0.0;
	if(valid)
	{
		priceStr=mainWindow.numFromDouble(price);
		volumeStr=QString::number(volume,'f',baseValues.currentPair.currADecimals);
	}
	return valid;
}