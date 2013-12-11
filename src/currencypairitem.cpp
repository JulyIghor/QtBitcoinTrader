// Copyright (C) 2013 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form https://sourceforge.net/projects/bitcointrader/
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#include "main.h"

CurrencyPairItem::CurrencyPairItem()
{
	priceMin=0.0;
	tradePriceMin=0.0;
	tradeVolumeMin=0.0;
	priceDecimals=5;
	currADecimals=8;
	currBDecimals=5;
	currABalanceDecimals=8;
	currBBalanceDecimals=5;

	currASign="BTC";
	currAStr="BTC";
	currAStrLow="btc";

	currBStr="USD";
	currBStrLow="usd";
	currBSign="USD";

	currSymbol="BTCUSD";
	currRequestPair="BTCUSD";
}

void CurrencyPairItem::setSymbol(QByteArray symb)
{
	currSymbol=symb.toUpper();
	if(currSymbol.size()!=6){currSymbol.clear();return;}

	currAStr=currSymbol.left(3);
	currAStrLow=currAStr.toLower();

	currBStr=currSymbol.right(3);
	currBStrLow=currBStr.toLower();

	currASign=baseValues_->currencySignMap.value(currAStr,"$");
	currBSign=baseValues_->currencySignMap.value(currBStr,"$");

	currAName=baseValues_->currencyNamesMap.value(currAStr,"BITCOINS");
}