// Copyright (C) 2013 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form https://sourceforge.net/projects/bitcointrader/
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#include "main.h"
#include "ruleholder.h"

RuleHolder::RuleHolder(int moreLessEqual, double price, double bitcoins, bool isBuy, double sellPrice, int rulePriceTp,  bool enabled)
{
	ruleState=enabled?1:0;
	waitingGoodLag=false;
	invalidHolder=false;
	rulePriceType=rulePriceTp;
	rulePrice=sellPrice;
	ruleCheckPrice=price;
	ruleMoreLessEqual=moreLessEqual;
	ruleBtc=bitcoins;
	buying=isBuy;
}

RuleHolder::RuleHolder(QString strData)
{
	ruleState=0;
	invalidHolder=true;
	waitingGoodLag=false;
	QStringList restorableList=strData.split('|');
	if(restorableList.count()!=7)return;
	rulePriceType=restorableList.at(0).toInt();
	waitingGoodLag=restorableList.at(1).toInt();
	buying=restorableList.at(2).toInt();
	ruleBtc=restorableList.at(3).toDouble();
	ruleMoreLessEqual=restorableList.at(4).toInt();
	rulePrice=restorableList.at(5).toDouble();
	ruleCheckPrice=restorableList.at(6).toDouble();
	invalidHolder=false;
}

QString RuleHolder::generateSavableData()
{
	QStringList savableList;
	savableList<<QString::number(rulePriceType);
	savableList<<QString::number(waitingGoodLag);
	savableList<<QString::number(buying);
	savableList<<mainWindow.numFromDouble(ruleBtc);
	savableList<<QString::number(ruleMoreLessEqual);
	savableList<<mainWindow.numFromDouble(rulePrice);
	savableList<<mainWindow.numFromDouble(ruleCheckPrice);
	return savableList.join("|");
}

int RuleHolder::getRuleState()
{
	return ruleState;
}

void RuleHolder::setRuleState(int newState)
{
	ruleState=newState;
}

bool RuleHolder::isAchieved(double price)
{
	if(ruleState!=1)return false;
	if(rulePriceType!=8&&rulePriceType!=9&&price<baseValues.currentPair.tradePriceMin)return false;
	if(waitingGoodLag)return true;
	if(ruleMoreLessEqual==-1&&ruleCheckPrice>price)return true;
	if(ruleMoreLessEqual==1&&ruleCheckPrice<price)return true;
	if(ruleMoreLessEqual==0&&ruleCheckPrice==price)return true;
	return false;
}

void RuleHolder::startWaitingLowLag()
{
	waitingGoodLag=true;
}

bool RuleHolder::isBuying()
{
	return buying;
}

QString RuleHolder::getDescriptionString()
{
		QString priceStr=mainWindow.numFromDouble(ruleCheckPrice);
		if(rulePriceType==8||rulePriceType==10||rulePriceType==12)priceStr.prepend(baseValues.currentPair.currASign+" ");
		else priceStr.prepend(baseValues.currentPair.currBSign+" ");

		if(ruleMoreLessEqual==1)
		{
			if(rulePriceType==1)return julyTr("IF_MARKET_LAST_MORE","If market last price goes more than %1").arg(priceStr);
			if(rulePriceType==2)return julyTr("IF_MARKET_BUY_MORE","If market buy price goes more than %1").arg(priceStr);
			if(rulePriceType==3)return julyTr("IF_MARKET_SELL_MORE","If market sell price goes more than %1").arg(priceStr);
			if(rulePriceType==4)return julyTr("IF_MARKET_HIGH_MORE","If market high price goes more than %1").arg(priceStr);
			if(rulePriceType==5)return julyTr("IF_MARKET_LOW_MORE","If market low price goes more than %1").arg(priceStr);
			if(rulePriceType==6)return julyTr("IF_MARKET_LAST_BUY_MORE","If orders last buy price goes more than %1").arg(priceStr);
			if(rulePriceType==7)return julyTr("IF_MARKET_LAST_SELL_MORE","If orders last sell price goes more than %1").arg(priceStr);
			if(rulePriceType==8)return julyTr("IF_BALANCE_GOES_MORE","If %1 balance goes more than %2").arg(QString(baseValues.currentPair.currAStr)).arg(priceStr);
			if(rulePriceType==9)return julyTr("IF_BALANCE_GOES_MORE","If %1 balance goes more than %2").arg(QString(baseValues.currentPair.currBStr)).arg(priceStr);
			if(rulePriceType==10)return julyTr("IF_TOTAL_TO_BUY_AT_LAST_MORE","If Total to Buy at Last price goes more than %1").arg(priceStr);
			if(rulePriceType==11)return julyTr("IF_AMOUNT_TO_RECEIVE_AT_LAST_MORE","If Amount to Receive at Last price goes more than %1").arg(priceStr);
			if(rulePriceType==12)return julyTr("IF_TOTAL_TO_BUY_AT_BUY_PRICE_MORE","If Total to Buy at Buy price goes more than %1").arg(priceStr);
			if(rulePriceType==13)return julyTr("IF_AMOUNT_TO_RECEIVE_AT_SELL_PRICE_MORE","If Amount to Receive at Sell price goes more than %1").arg(priceStr);
		}
		if(ruleMoreLessEqual==-1)
		{
			if(rulePriceType==1)return julyTr("IF_MARKET_LAST_LESS","If market last price goes less than %1").arg(priceStr);
			if(rulePriceType==2)return julyTr("IF_MARKET_BUY_LESS","If market buy price goes less than %1").arg(priceStr);
			if(rulePriceType==3)return julyTr("IF_MARKET_SELL_LESS","If market sell price goes less than %1").arg(priceStr);
			if(rulePriceType==4)return julyTr("IF_MARKET_HIGH_LESS","If market high price goes less than %1").arg(priceStr);
			if(rulePriceType==5)return julyTr("IF_MARKET_LOW_LESS","If market low price goes less than %1").arg(priceStr);
			if(rulePriceType==6)return julyTr("IF_MARKET_LAST_BUY_LESS","If orders last buy price goes less than %1").arg(priceStr);
			if(rulePriceType==7)return julyTr("IF_MARKET_LAST_SELL_LESS","If orders last sell price goes less than %1").arg(priceStr);
			if(rulePriceType==8)return julyTr("IF_BALANCE_GOES_LESS","If %1 balance goes less than %2").arg(QString(baseValues.currentPair.currAStr)).arg(priceStr);
			if(rulePriceType==9)return julyTr("IF_BALANCE_GOES_LESS","If %1 balance goes less than %2").arg(QString(baseValues.currentPair.currBStr)).arg(priceStr);
			if(rulePriceType==10)return julyTr("IF_TOTAL_TO_BUY_AT_LAST_LESS","If Total to Buy at Last price goes less than %1").arg(priceStr);
			if(rulePriceType==11)return julyTr("IF_AMOUNT_TO_RECEIVE_AT_LAST_LESS","If Amount to Receive at Last price goes less than %1").arg(priceStr);
			if(rulePriceType==12)return julyTr("IF_TOTAL_TO_BUY_AT_BUY_PRICE_LESS","If Total to Buy at Buy price goes less than %1").arg(priceStr);
			if(rulePriceType==13)return julyTr("IF_AMOUNT_TO_RECEIVE_AT_SELL_PRICE_LESS","If Amount to Receive at Sell price goes less than %1").arg(priceStr);
		}
		if(ruleMoreLessEqual==0)
		{
			if(rulePriceType==1)return julyTr("IF_MARKET_LAST_EQUAL","If market last price equal to %1").arg(priceStr);
			if(rulePriceType==2)return julyTr("IF_MARKET_BUY_EQUAL","If market buy price equal to %1").arg(priceStr);
			if(rulePriceType==3)return julyTr("IF_MARKET_SELL_EQUAL","If market sell price equal to %1").arg(priceStr);
			if(rulePriceType==4)return julyTr("IF_MARKET_HIGH_EQUAL","If market high price equal to %1").arg(priceStr);
			if(rulePriceType==5)return julyTr("IF_MARKET_LOW_EQUAL","If market low price equal to %1").arg(priceStr);
			if(rulePriceType==6)return julyTr("IF_MARKET_LAST_BUY_EQUAL","If orders last buy price equal to %1").arg(priceStr);
			if(rulePriceType==7)return julyTr("IF_MARKET_LAST_SELL_EQUAL","If orders last sell price equal to %1").arg(priceStr);
			if(rulePriceType==8)return julyTr("IF_BALANCE_GOES_EQUAL","If %1 balance equal to %2").arg(QString(baseValues.currentPair.currAStr)).arg(priceStr);
			if(rulePriceType==9)return julyTr("IF_BALANCE_GOES_EQUAL","If %1 balance equal to %2").arg(QString(baseValues.currentPair.currBStr)).arg(priceStr);
		}
		return priceStr;
}

QString RuleHolder::getSellOrBuyString()
{
	if(ruleBtc==-5.0)return julyTr("CANCEL_ALL_ORDERS","Cancel All Orders");
	if(ruleBtc==-6.0)return julyTr("ENABLE_ALL_RULES","Enable All Rules");
	if(ruleBtc==-7.0)return julyTr("DISABLE_ALL_RULES","Disable All Rules");
	if(buying)return julyTr("ORDER_TYPE_BID","Buy");
	return julyTr("ORDER_TYPE_ASK","Sell");
}

QString RuleHolder::getBitcoinsString()
{
	if(ruleBtc==-1.0)return julyTr("SELL_ALL_BTC","Sell All my BTC");
	if(ruleBtc==-2.0)return julyTr("SELL_HALF_BTC","Sell Half my BTC");
	if(ruleBtc==-3.0)return julyTr("SPEND_ALL_FUNDS","Spend All my Funds");
	if(ruleBtc==-4.0)return julyTr("SPEND_HALF_FUNDS","Spend Half my Funds");
	if(ruleBtc==-5.0)return julyTr("NOT_USED","Not Used");
	if(ruleBtc==-6.0)return julyTr("NOT_USED","Not Used");
	if(ruleBtc==-7.0)return julyTr("NOT_USED","Not Used");

	return baseValues.currentPair.currASign+" "+mainWindow.numFromDouble(ruleBtc);
}

QString RuleHolder::getPriceText()
{
	if(ruleBtc==-5.0)return julyTr("NOT_USED","Not Used");;

	if(rulePrice==-1.0)return julyTr("AS_MARKET_LAST","Market Last");
	if(rulePrice==-2.0)return julyTr("AS_MARKET_BUY","Market Buy");
	if(rulePrice==-3.0)return julyTr("AS_MARKET_SELL","Market Sell");
	if(rulePrice==-4.0)return julyTr("AS_MARKET_HIGH","Market High");
	if(rulePrice==-5.0)return julyTr("AS_MARKET_LOW","Market Low");
	if(rulePrice==-6.0)return julyTr("AS_ORDERS_LAST_BUY","Orders Last Buy");
	if(rulePrice==-7.0)return julyTr("AS_ORDERS_LAST_SELL","Orders Last Sell");
	return baseValues.currentPair.currBSign+" "+mainWindow.numFromDouble(rulePrice);
}

