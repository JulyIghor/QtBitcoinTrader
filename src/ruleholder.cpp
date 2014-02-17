//  This file is part of Qt Bitcion Trader
//      https://github.com/JulyIGHOR/QtBitcoinTrader
//  Copyright (C) 2013-2014 July IGHOR <julyighor@gmail.com>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  In addition, as a special exception, the copyright holders give
//  permission to link the code of portions of this program with the
//  OpenSSL library under certain conditions as described in each
//  individual source file, and distribute linked combinations including
//  the two.
//
//  You must obey the GNU General Public License in all respects for all
//  of the code used other than OpenSSL. If you modify file(s) with this
//  exception, you may extend this exception to your version of the
//  file(s), but you are not obligated to do so. If you do not wish to do
//  so, delete this exception statement from your version. If you delete
//  this exception statement from all source files in the program, then
//  also delete it here.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "main.h"
#include "ruleholder.h"

RuleHolder::RuleHolder(int moreLessEqual, double price, double bitcoins, bool isBuy, double sellPrice, int rulePriceTp, double _checkPricePercentage, double _rulePricePercentage, double _ruleAmountPercentage, int _ruleGroupId, QString _sound, bool _trailingEnabled,  bool enabled)
{
	trailingEnabled=_trailingEnabled;
	lastRuleCheckPrice=0.0;
	ruleCheckPricePercentage=_checkPricePercentage/100.0;
	ruleGroupId=_ruleGroupId;
	ruleWavFile=_sound;
	ruleExecutePricePercentage=_rulePricePercentage/100.0;
	ruleAmountPercentage=_ruleAmountPercentage/100.0;

	ruleState=enabled?1:0;
	waitingGoodLag=false;
	invalidHolder=false;
	rulePriceType=rulePriceTp;
	ruleExecutePrice=sellPrice;
	ruleCheckPrice=price;
	ruleMoreLessEqualChanged=moreLessEqual;
	ruleAmount=bitcoins;
	buying=isBuy;
	saveRuleCheckPrice();
}

RuleHolder::RuleHolder(QString strData)
{
	lastRuleCheckPrice=0.0;
	ruleState=0;
	invalidHolder=true;
	waitingGoodLag=false;
	QStringList restorableList=strData.split('|');
	if(restorableList.count()<=7)return;
	rulePriceType=restorableList.at(0).toInt();
	waitingGoodLag=restorableList.at(1).toInt();
	buying=restorableList.at(2).toInt();
	ruleAmount=restorableList.at(3).toDouble();
	ruleMoreLessEqualChanged=restorableList.at(4).toInt();
	ruleExecutePrice=restorableList.at(5).toDouble();
	ruleCheckPrice=restorableList.at(6).toDouble();

	if(restorableList.count()>7)ruleCheckPricePercentage=restorableList.at(7).toDouble()/100.0;
	else ruleCheckPricePercentage=0.0;

	if(restorableList.count()>8)ruleExecutePricePercentage=restorableList.at(8).toDouble()/100.0;
	else ruleExecutePricePercentage=0.0;

	if(restorableList.count()>9)ruleAmountPercentage=restorableList.at(9).toDouble()/100.0;
	else ruleAmountPercentage=1.0;

	if(restorableList.count()>10)ruleGroupId=restorableList.at(10).toDouble();
	else ruleGroupId=0;

	if(restorableList.count()>11)ruleWavFile=QByteArray::fromBase64(restorableList.at(11).toAscii());
	if(restorableList.count()>12)trailingEnabled=restorableList.at(12).toInt();
	else ruleWavFile.clear();

	invalidHolder=false;
	saveRuleCheckPrice();
}

QString RuleHolder::generateSavableData()
{
	QStringList savableList;
	savableList<<QString::number(rulePriceType);
	savableList<<QString::number(waitingGoodLag);
	savableList<<QString::number(buying);
	savableList<<mainWindow.numFromDouble(ruleAmount);
	savableList<<QString::number(ruleMoreLessEqualChanged);
	savableList<<mainWindow.numFromDouble(ruleExecutePrice);
	savableList<<mainWindow.numFromDouble(ruleCheckPrice);
	savableList<<mainWindow.numFromDouble(ruleCheckPricePercentage*100.0);
	savableList<<mainWindow.numFromDouble(ruleExecutePricePercentage*100.0);
	savableList<<mainWindow.numFromDouble(ruleAmountPercentage*100.0);
	savableList<<QString::number(ruleGroupId);
	savableList<<ruleWavFile.toAscii().toBase64();
	savableList<<(trailingEnabled?"1":"0");
	return savableList.join("|");
}

bool RuleHolder::isTrading()
{
	return ruleAmount>-5.0;//Important value
}

double RuleHolder::getCurrentExecPrice()
{
	if(ruleExecutePrice>-1.0)return ruleExecutePrice;
	double returnValue=0.0;
	if(ruleExecutePrice==-1.0)returnValue=mainWindow.ui.marketLast->value();else
	if(ruleExecutePrice==-2.0)returnValue=mainWindow.ui.marketBid->value();else
	if(ruleExecutePrice==-3.0)returnValue=mainWindow.ui.marketAsk->value();else
	if(ruleExecutePrice==-4.0)returnValue=mainWindow.ui.marketHigh->value();else
	if(ruleExecutePrice==-5.0)returnValue=mainWindow.ui.marketLow->value();else
	if(ruleExecutePrice==-6.0)returnValue=mainWindow.ui.ordersLastBuyPrice->value();else
	if(ruleExecutePrice==-7.0)returnValue=mainWindow.ui.ordersLastSellPrice->value();
	return returnValue+returnValue*ruleExecutePricePercentage;
}

double RuleHolder::getCurrentCheckPrice()
{
	switch(rulePriceType)
	{
	case 1: return mainWindow.ui.marketLast->value(); break;
	case 2: return mainWindow.ui.marketBid->value(); break;
	case 3: return mainWindow.ui.marketAsk->value(); break;
	case 4: return mainWindow.ui.marketHigh->value(); break;
	case 5: return mainWindow.ui.marketLow->value(); break;
	case 6: return mainWindow.ui.ordersLastBuyPrice->value(); break;
	case 7: return mainWindow.ui.ordersLastSellPrice->value(); break;
	case 8: return mainWindow.ui.accountBTC->value(); break;
	case 9: return mainWindow.ui.accountUSD->value(); break;
	case 10: return mainWindow.ui.ruleTotalToBuyValue->value(); break;
	case 12: return mainWindow.ui.ruleTotalToBuyBSValue->value(); break;
	case 13: return mainWindow.ui.ruleAmountToReceiveBSValue->value(); break;
	case 14: return mainWindow.ui.tradesVolume5m->value(); break;
	case 15: return mainWindow.ui.tradesBidsPrecent->value(); break;
	case 16: return -1.0; break;
	default: return 0.0; break;
	}
	return 0.0;
}

void RuleHolder::saveRuleCheckPrice()
{
	if(ruleState==0)return;
	if(ruleCheckPricePercentage!=0.0||ruleMoreLessEqualChanged==-2)
		lastRuleCheckPrice=getCurrentCheckPrice();
}

int RuleHolder::getRuleState()
{
	return ruleState;
}

void RuleHolder::setRuleState(int newState)//0: Disabled; 1:Enabled
{
	ruleState=newState;
	saveRuleCheckPrice();
}

#include <QDebug>

bool RuleHolder::isAchieved(double price)
{
	if(ruleState!=1)return false;
	if(ruleExecutePrice>-1)price+=price*ruleExecutePricePercentage;
	if(rulePriceType<8&&price<baseValues.currentPair.tradePriceMin)return false;
	if(waitingGoodLag)return true;
	if(rulePriceType==16)return true;
	if(ruleCheckPricePercentage!=0.0)
	{
		if(ruleMoreLessEqualChanged==-1)
		{
			if(lastRuleCheckPrice-lastRuleCheckPrice*ruleCheckPricePercentage>price)return true;
			if(trailingEnabled&&lastRuleCheckPrice<price)lastRuleCheckPrice=price;
		}
		if(ruleMoreLessEqualChanged==1)
		{
			if(lastRuleCheckPrice+lastRuleCheckPrice*ruleCheckPricePercentage<price)return true;
			if(trailingEnabled&&lastRuleCheckPrice>price)lastRuleCheckPrice=price;
		}
		if(ruleMoreLessEqualChanged==0)
		{
			if(lastRuleCheckPrice+lastRuleCheckPrice*ruleCheckPricePercentage==price)return true;
		}
	}
	else
	{
		if(ruleMoreLessEqualChanged==-1&&ruleCheckPrice>price)return true;
		if(ruleMoreLessEqualChanged==1&&ruleCheckPrice<price)return true;
		if(ruleMoreLessEqualChanged==0&&ruleCheckPrice==price)return true;
	}

	if(ruleMoreLessEqualChanged==-2)
	{
		if(ruleCheckPricePercentage!=0.0)
		{
			if(lastRuleCheckPrice+lastRuleCheckPrice*ruleCheckPricePercentage<price)
			{
				lastRuleCheckPrice=price;
				return true;
			}
			if(lastRuleCheckPrice-lastRuleCheckPrice*ruleCheckPricePercentage>price)
			{
				lastRuleCheckPrice=price;
				return true;
			}
		}
		else
		{
			if(lastRuleCheckPrice+ruleCheckPrice<price)
			{
				lastRuleCheckPrice=price;
				return true;
			}
			if(lastRuleCheckPrice-ruleCheckPrice>price)
			{
				lastRuleCheckPrice=price;
				return true;
			}
		}
	}
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
	QString trailingText;
	if(trailingEnabled&&rulePriceType!=16)trailingText=" "+julyTr("TRAILING_ENABLED_DESCRIPTION","(trailing)");

	QString priceStr;
	if(ruleCheckPricePercentage!=0.0)
	{
		priceStr="%"+mainWindow.numFromDouble(ruleCheckPricePercentage*100.0);
	}
	else
	{
		priceStr=mainWindow.numFromDouble(ruleCheckPrice);
		if(rulePriceType==15)priceStr.prepend("%");
		else
		if(rulePriceType==8||rulePriceType==10||rulePriceType==12||rulePriceType==14)priceStr.prepend(baseValues.currentPair.currASign);
		else priceStr.prepend(baseValues.currentPair.currBSign);
	}
		if(ruleMoreLessEqualChanged==-2)
		{
			if(rulePriceType==1)return julyTr("IF_MARKET_LAST_CHANGED","If market last price changed %1").arg(priceStr)+trailingText;
			if(rulePriceType==2)return julyTr("IF_MARKET_BUY_CHANGED","If market bid price changed %1").arg(priceStr)+trailingText;
			if(rulePriceType==3)return julyTr("IF_MARKET_SELL_CHANGED","If market ask price changed %1").arg(priceStr)+trailingText;
			if(rulePriceType==4)return julyTr("IF_MARKET_HIGH_CHANGED","If market high price changed %1").arg(priceStr)+trailingText;
			if(rulePriceType==5)return julyTr("IF_MARKET_LOW_CHANGED","If market low price changed %1").arg(priceStr)+trailingText;
			if(rulePriceType==6)return julyTr("IF_MARKET_LAST_BUY_CHANGED","If my orders last buy price changed %1").arg(priceStr)+trailingText;
			if(rulePriceType==7)return julyTr("IF_MARKET_LAST_SELL_CHANGED","If my orders last sell price changed %1").arg(priceStr);
			if(rulePriceType==8)return julyTr("IF_BALANCE_GOES_CHANGED","If %1 balance changed %2").arg(QString(baseValues.currentPair.currAStr)).arg(priceStr)+trailingText;
			if(rulePriceType==9)return julyTr("IF_BALANCE_GOES_CHANGED","If %1 balance changed %2").arg(QString(baseValues.currentPair.currBStr)).arg(priceStr)+trailingText;
			if(rulePriceType==10)return julyTr("IF_TOTAL_TO_BUY_AT_LAST_CHANGED","If Total to Buy at Last price changed %1").arg(priceStr)+trailingText;
			if(rulePriceType==11)return julyTr("IF_AMOUNT_TO_RECEIVE_AT_LAST_CHANGED","If Amount to Receive at Last price changed %1").arg(priceStr)+trailingText;
			if(rulePriceType==12)return julyTr("IF_TOTAL_TO_BUY_AT_BUY_PRICE_CHANGED","If Total to Buy at Buy price changed %1").arg(priceStr)+trailingText;
			if(rulePriceType==13)return julyTr("IF_AMOUNT_TO_RECEIVE_AT_SELL_PRICE_CHANGED","If Amount to Receive at Sell price changed %1").arg(priceStr)+trailingText;

			if(rulePriceType==14)return julyTr("IF_10MIN_VOLUME_CHANGED","If trades 10 min. volume changed %1").arg(priceStr)+trailingText;
			if(rulePriceType==15)return julyTr("IF_10MIN_PERCENTAGE_CHANGED","If trades 10 min. Buy/Sell changed %1").arg(priceStr)+trailingText;
		}
		else
		if(ruleMoreLessEqualChanged==1)
		{
			if(rulePriceType==1)return julyTr("IF_MARKET_LAST_MORE","If market last price goes more than %1").arg(priceStr)+trailingText;
			if(rulePriceType==2)return julyTr("IF_MARKET_BUY_MORE","If market bid price goes more than %1").arg(priceStr)+trailingText;
			if(rulePriceType==3)return julyTr("IF_MARKET_SELL_MORE","If market ask price goes more than %1").arg(priceStr)+trailingText;
			if(rulePriceType==4)return julyTr("IF_MARKET_HIGH_MORE","If market high price goes more than %1").arg(priceStr)+trailingText;
			if(rulePriceType==5)return julyTr("IF_MARKET_LOW_MORE","If market low price goes more than %1").arg(priceStr)+trailingText;
			if(rulePriceType==6)return julyTr("IF_MARKET_LAST_BUY_MORE","If my orders last buy price goes more than %1").arg(priceStr)+trailingText;
			if(rulePriceType==7)return julyTr("IF_MARKET_LAST_SELL_MORE","If my orders last sell price goes more than %1").arg(priceStr)+trailingText;
			if(rulePriceType==8)return julyTr("IF_BALANCE_GOES_MORE","If %1 balance goes more than %2").arg(QString(baseValues.currentPair.currAStr)).arg(priceStr)+trailingText;
			if(rulePriceType==9)return julyTr("IF_BALANCE_GOES_MORE","If %1 balance goes more than %2").arg(QString(baseValues.currentPair.currBStr)).arg(priceStr)+trailingText;
			if(rulePriceType==10)return julyTr("IF_TOTAL_TO_BUY_AT_LAST_MORE","If Total to Buy at Last price goes more than %1").arg(priceStr)+trailingText;
			if(rulePriceType==11)return julyTr("IF_AMOUNT_TO_RECEIVE_AT_LAST_MORE","If Amount to Receive at Last price goes more than %1").arg(priceStr)+trailingText;
			if(rulePriceType==12)return julyTr("IF_TOTAL_TO_BUY_AT_BUY_PRICE_MORE","If Total to Buy at Buy price goes more than %1").arg(priceStr)+trailingText;
			if(rulePriceType==13)return julyTr("IF_AMOUNT_TO_RECEIVE_AT_SELL_PRICE_MORE","If Amount to Receive at Sell price goes more than %1").arg(priceStr)+trailingText;

			if(rulePriceType==14)return julyTr("IF_10MIN_VOLUME_MORE","If trades 10 min. volume goes more than %1").arg(priceStr)+trailingText;
			if(rulePriceType==15)return julyTr("IF_10MIN_PERCENTAGE_MORE","If trades 10 min. Buy/Sell goes more than %1").arg(priceStr)+trailingText;
		}
		else
		if(ruleMoreLessEqualChanged==-1)
		{
			if(rulePriceType==1)return julyTr("IF_MARKET_LAST_LESS","If market last price goes less than %1").arg(priceStr)+trailingText;
			if(rulePriceType==2)return julyTr("IF_MARKET_BUY_LESS","If market bid price goes less than %1").arg(priceStr)+trailingText;
			if(rulePriceType==3)return julyTr("IF_MARKET_SELL_LESS","If market ask price goes less than %1").arg(priceStr)+trailingText;
			if(rulePriceType==4)return julyTr("IF_MARKET_HIGH_LESS","If market high price goes less than %1").arg(priceStr)+trailingText;
			if(rulePriceType==5)return julyTr("IF_MARKET_LOW_LESS","If market low price goes less than %1").arg(priceStr)+trailingText;
			if(rulePriceType==6)return julyTr("IF_MARKET_LAST_BUY_LESS","If my orders last buy price goes less than %1").arg(priceStr)+trailingText;
			if(rulePriceType==7)return julyTr("IF_MARKET_LAST_SELL_LESS","If my orders last sell price goes less than %1").arg(priceStr)+trailingText;
			if(rulePriceType==8)return julyTr("IF_BALANCE_GOES_LESS","If %1 balance goes less than %2").arg(QString(baseValues.currentPair.currAStr)).arg(priceStr)+trailingText;
			if(rulePriceType==9)return julyTr("IF_BALANCE_GOES_LESS","If %1 balance goes less than %2").arg(QString(baseValues.currentPair.currBStr)).arg(priceStr)+trailingText;
			if(rulePriceType==10)return julyTr("IF_TOTAL_TO_BUY_AT_LAST_LESS","If Total to Buy at Last price goes less than %1").arg(priceStr)+trailingText;
			if(rulePriceType==11)return julyTr("IF_AMOUNT_TO_RECEIVE_AT_LAST_LESS","If Amount to Receive at Last price goes less than %1").arg(priceStr)+trailingText;
			if(rulePriceType==12)return julyTr("IF_TOTAL_TO_BUY_AT_BUY_PRICE_LESS","If Total to Buy at Buy price goes less than %1").arg(priceStr)+trailingText;
			if(rulePriceType==13)return julyTr("IF_AMOUNT_TO_RECEIVE_AT_SELL_PRICE_LESS","If Amount to Receive at Sell price goes less than %1").arg(priceStr)+trailingText;

			if(rulePriceType==14)return julyTr("IF_10MIN_VOLUME_LESS","If trades 10 min. volume goes less than %1").arg(priceStr)+trailingText;
			if(rulePriceType==15)return julyTr("IF_10MIN_PERCENTAGE_LESS","If trades 10 min. Buy/Sell goes less than %1").arg(priceStr)+trailingText;
		}
		else
		if(ruleMoreLessEqualChanged==0)
		{
			if(rulePriceType==1)return julyTr("IF_MARKET_LAST_EQUAL","If market last price equal to %1").arg(priceStr)+trailingText;
			if(rulePriceType==2)return julyTr("IF_MARKET_BUY_EQUAL","If market bid price equal to %1").arg(priceStr)+trailingText;
			if(rulePriceType==3)return julyTr("IF_MARKET_SELL_EQUAL","If market ask price equal to %1").arg(priceStr)+trailingText;
			if(rulePriceType==4)return julyTr("IF_MARKET_HIGH_EQUAL","If market high price equal to %1").arg(priceStr)+trailingText;
			if(rulePriceType==5)return julyTr("IF_MARKET_LOW_EQUAL","If market low price equal to %1").arg(priceStr)+trailingText;
			if(rulePriceType==6)return julyTr("IF_MARKET_LAST_BUY_EQUAL","If my orders last buy price equal to %1").arg(priceStr)+trailingText;
			if(rulePriceType==7)return julyTr("IF_MARKET_LAST_SELL_EQUAL","If my orders last sell price equal to %1").arg(priceStr)+trailingText;
			if(rulePriceType==8)return julyTr("IF_BALANCE_GOES_EQUAL","If %1 balance equal to %2").arg(QString(baseValues.currentPair.currAStr)).arg(priceStr)+trailingText;
			if(rulePriceType==9)return julyTr("IF_BALANCE_GOES_EQUAL","If %1 balance equal to %2").arg(QString(baseValues.currentPair.currBStr)).arg(priceStr)+trailingText;
		}
		if(rulePriceType==16)return julyTranslator.translateCheckBox("IF_IMMEDIATELY_EXECUTION","Immediately execution")+trailingText;
		return priceStr;
}

QString RuleHolder::getSellOrBuyString()
{
	if(ruleAmount==-5.0)return julyTr("CANCEL_ALL_ORDERS","Cancel All Orders");
	if(ruleAmount==-6.0)return julyTr("ENABLE_ALL_RULES","Enable All Rules");
	if(ruleAmount==-7.0)return julyTr("DISABLE_ALL_RULES","Disable All Rules");
	if(ruleAmount==-8.0)return julyTr("ENABLE_ALL_RULES_GROUP","Enable Group #%1").arg(ruleGroupId);
	if(ruleAmount==-9.0)return julyTr("DISABLE_ALL_RULES_GROUP","Disable Group #%1").arg(ruleGroupId);
	if(ruleAmount==-10.0)return julyTr("PLAY_BEEP","Beep");
	if(ruleAmount==-11.0)return julyTr("PLAY_SOUND","Play Sound");
	if(buying)return julyTr("ORDER_TYPE_BID","Buy");
	return julyTr("ORDER_TYPE_ASK","Sell");
}

QString RuleHolder::getBitcoinsString()
{
	if(ruleAmount==-1.0)return julyTr("AMOUNT_SELL_OF_MY","Sell %1% of my %2").arg(mainWindow.numFromDouble(ruleAmountPercentage*100.0)).arg(QString(baseValues.currentPair.currAStr));
	if(ruleAmount==-2.0)return julyTr("AMOUNT_SELL_OF_MY","Sell %1% of my %2").arg("50").arg(QString(baseValues.currentPair.currAStr));
	if(ruleAmount==-3.0)return julyTr("AMOUNT_SPEND_OF_MY","Spend %1% of my %2").arg(mainWindow.numFromDouble(ruleAmountPercentage*100.0)).arg(QString(baseValues.currentPair.currBStr));
	if(ruleAmount==-4.0)return julyTr("AMOUNT_SPEND_OF_MY","Spend %1% of my %2").arg("50").arg(QString(baseValues.currentPair.currBStr));

	if(ruleAmount<0.0)return julyTr("NOT_USED","Not Used");
	return baseValues.currentPair.currASign+" "+mainWindow.numFromDouble(ruleAmount);
}

QString RuleHolder::getPriceText()
{
	if(ruleAmount<=-5.0)return julyTr("NOT_USED","Not Used");

	QString percentStr;

	if(ruleExecutePricePercentage!=0.0)
	{
		percentStr.append(" ");
		if(ruleExecutePricePercentage>0.0)percentStr.append("+");
		percentStr.append(mainWindow.numFromDouble(ruleExecutePricePercentage*100.0)+"%");
	}

	if(ruleExecutePrice==-1.0)return julyTr("AS_MARKET_LAST","Last Price")+percentStr;
	if(ruleExecutePrice==-2.0)return julyTr("AS_MARKET_BUY","Bid Price")+percentStr;
	if(ruleExecutePrice==-3.0)return julyTr("AS_MARKET_SELL","Ask Price")+percentStr;
	if(ruleExecutePrice==-4.0)return julyTr("AS_MARKET_HIGH","High Price")+percentStr;
	if(ruleExecutePrice==-5.0)return julyTr("AS_MARKET_LOW","Low Price")+percentStr;
	if(ruleExecutePrice==-6.0)return julyTr("AS_ORDERS_LAST_BUY","My Orders Last Buy")+percentStr;
	if(ruleExecutePrice==-7.0)return julyTr("AS_ORDERS_LAST_SELL","My Orders Last Sell")+percentStr;

	return baseValues.currentPair.currBSign+" "+mainWindow.numFromDouble(ruleExecutePrice);
}

