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

#include "exchange_mtgox.h"

Exchange_MtGox::Exchange_MtGox(QByteArray pRestSign, QByteArray pRestKey)
	: Exchange()
{
	calculatingFeeMode=0;
	lastTradesDate=0;
	tickerLastDate=0;
	baseValues.exchangeName="Mt.Gox";
	privateRestSign=pRestSign;
	privateRestKey=pRestKey;
	depthAsks=0;
	depthBids=0;
	forceDepthLoad=false;
	julyHttp=0;
	tickerOnly=false;

	currencyMapFile="MtGox";
	baseValues.currentPair.name="BTC/USD";
	baseValues.currentPair.setSymbol("BTCUSD");
	baseValues.currentPair.currRequestPair="BTCUSD";
	baseValues.currentPair.priceDecimals=5;
	minimumRequestIntervalAllowed=500;
	baseValues.currentPair.priceMin=qPow(0.1,baseValues.currentPair.priceDecimals);
	baseValues.currentPair.tradeVolumeMin=0.01;
	baseValues.currentPair.tradePriceMin=0.1;
	defaultCurrencyParams.currADecimals=8;
	defaultCurrencyParams.currBDecimals=5;
	defaultCurrencyParams.currABalanceDecimals=8;
	defaultCurrencyParams.currBBalanceDecimals=5;
	defaultCurrencyParams.priceDecimals=5;
	defaultCurrencyParams.priceMin=qPow(0.1,baseValues.currentPair.priceDecimals);

	supportsLoginIndicator=true;
	supportsAccountVolume=true;
	supportsExchangeLag=true;

	authRequestTime.restart();
	privateNonce=(QDateTime::currentDateTime().toTime_t()-1371854884)*10;
}

Exchange_MtGox::~Exchange_MtGox()
{
}


void Exchange_MtGox::clearVariables()
{
	isFirstTicker=true;
	isFirstAccInfo=true;
	lastTickerHigh=0.0;
	lastTickerLow=0.0;
	lastTickerSell=0.0;
	lastTickerBuy=0.0;
	lastTickerVolume=0.0;
	lastBtcBalance=0.0;
	lastUsdBalance=0.0;
	lastVolume=0.0;
	lastFee=0.0;
	secondPart=0;
	apiDownCounter=0;
	lastHistory.clear();
	lastOrders.clear();
	reloadDepth();
	lastInfoReceived=false;
	tickerLastDate=0;
	lastTradesDate=QDateTime::currentDateTime().addSecs(-600).toTime_t();
	lastTradesDate*=1000000;
	lastTradesDateCache=QByteArray::number(lastTradesDate);
}

void Exchange_MtGox::clearValues()
{
	clearVariables();
	if(julyHttp)julyHttp->clearPendingData();
}

void Exchange_MtGox::secondSlot()
{
	static int infoCounter=0;
	if(userInfoTime.elapsed()>(lastInfoReceived?10000:1000)&&!isReplayPending(202))
	{
		userInfoTime.restart();
		sendToApi(202,baseValues.currentPair.currRequestPair+"/money/info",true,baseValues.httpSplitPackets);
	}

	if(!tickerOnly&&!isReplayPending(204))sendToApi(204,baseValues.currentPair.currRequestPair+"/money/orders",true,baseValues.httpSplitPackets);

	if(depthEnabled&&(forceDepthLoad||/*infoCounter==3&&*/!isReplayPending(111)))
	{
		emit depthRequested();
		sendToApi(111,baseValues.currentPair.currRequestPair+"/money/depth/fetch",false,baseValues.httpSplitPackets);
		forceDepthLoad=false;
	}

	if(!isReplayPending(101))sendToApi(101,baseValues.currentPair.currRequestPair+"/money/order/lag",false,baseValues.httpSplitPackets);
	if((infoCounter==1)&&!isReplayPending(103))sendToApi(103,baseValues.currentPair.currRequestPair+"/money/ticker",false,baseValues.httpSplitPackets);
	if(!isReplayPending(104))sendToApi(104,baseValues.currentPair.currRequestPair+"/money/ticker_fast",false,baseValues.httpSplitPackets);

	if(!isReplayPending(109))sendToApi(109,baseValues.currentPair.currRequestPair+"/money/trades/fetch?since="+lastTradesDateCache,false,baseValues.httpSplitPackets);
	if(lastHistory.isEmpty())
		if(!isReplayPending(208))sendToApi(208,"money/wallet/history",true,baseValues.httpSplitPackets,"&currency=BTC");
	if(!baseValues.httpSplitPackets&&julyHttp)julyHttp->prepareDataSend();

	if(++infoCounter>9)
	{
		infoCounter=0;
		quint32 syncNonce=(QDateTime::currentDateTime().toTime_t()-1371854884)*10;
		if(privateNonce<syncNonce)privateNonce=syncNonce;
	}
	Exchange::secondSlot();
}

bool Exchange_MtGox::isReplayPending(int reqType)
{
	if(julyHttp==0)return false;
	return julyHttp->isReqTypePending(reqType);
}

void Exchange_MtGox::getHistory(bool force)
{
	if(tickerOnly)return;
	if(force)lastHistory.clear();
	if(!isReplayPending(208))sendToApi(208,"money/wallet/history",true,true,"&currency=BTC");
}

void Exchange_MtGox::buy(double apiBtcToBuy, double apiPriceToBuy)
{
	if(tickerOnly)return;
	QByteArray params="&type=bid&amount_int="+QByteArray::number(apiBtcToBuy*qPow(10,baseValues.currentPair.currADecimals),'f',0)+"&price_int="+QByteArray::number(apiPriceToBuy*qPow(10,baseValues.currentPair.priceDecimals),'f',0);
	if(debugLevel)logThread->writeLog("Buy: "+params,2);
	sendToApi(306,baseValues.currentPair.currRequestPair+"/money/order/add",true,true,params);
}

void Exchange_MtGox::sell(double apiBtcToSell, double apiPriceToSell)
{
	if(tickerOnly)return;
	QByteArray params="&type=ask&amount_int="+QByteArray::number(apiBtcToSell*qPow(10,baseValues.currentPair.currADecimals),'f',0)+"&price_int="+QByteArray::number(apiPriceToSell*qPow(10,baseValues.currentPair.priceDecimals),'f',0);
	if(debugLevel)logThread->writeLog("Sell: "+params,2);
	sendToApi(307,baseValues.currentPair.currRequestPair+"/money/order/add",true,true,params);
}

void Exchange_MtGox::cancelOrder(QByteArray order)
{
	if(tickerOnly)return;
	if(debugLevel)logThread->writeLog("Cancel order: "+order,2);
	sendToApi(305,baseValues.currentPair.currRequestPair+"/money/order/cancel",true,true,"&oid="+order);
}

void Exchange_MtGox::sendToApi(int reqType, QByteArray method, bool auth, bool sendNow, QByteArray commands)
{
	if(julyHttp==0)
	{
		julyHttp=new JulyHttp("data.mtgox.com","Rest-Key: "+privateRestKey+"\r\n",this);
		connect(julyHttp,SIGNAL(anyDataReceived()),baseValues_->mainWindow_,SLOT(anyDataReceived()));
		connect(julyHttp,SIGNAL(setDataPending(bool)),baseValues_->mainWindow_,SLOT(setDataPending(bool)));
		connect(julyHttp,SIGNAL(apiDown(bool)),baseValues_->mainWindow_,SLOT(setApiDown(bool)));
		connect(julyHttp,SIGNAL(errorSignal(QString)),baseValues_->mainWindow_,SLOT(showErrorMessage(QString)));
		connect(julyHttp,SIGNAL(sslErrorSignal(const QList<QSslError> &)),this,SLOT(sslErrors(const QList<QSslError> &)));
		connect(julyHttp,SIGNAL(dataReceived(QByteArray,int)),this,SLOT(dataReceivedAuth(QByteArray,int)));
	}

	if(auth)
	{
		QByteArray postData="nonce="+QByteArray::number(++privateNonce);
		postData.append("000000");postData.append(commands);
		QByteArray forHash=method+'\0'+postData;
		if(sendNow)
		julyHttp->sendData(reqType, "POST /api/2/"+method,postData, "Rest-Sign: "+hmacSha512(privateRestSign,forHash).toBase64()+"\r\n");
		else
		julyHttp->prepareData(reqType, "POST /api/2/"+method, postData, "Rest-Sign: "+hmacSha512(privateRestSign,forHash).toBase64()+"\r\n");

	}
	else
	{
		if(sendNow)
			julyHttp->sendData(reqType, "GET /api/2/"+method);
		else 
			julyHttp->prepareData(reqType, "GET /api/2/"+method);
	}
}

void Exchange_MtGox::depthUpdateOrder(double price, double amount, bool isAsk)
{
	if(isAsk)
	{
		if(depthAsks==0)return;
		DepthItem newItem;
		newItem.price=price;
		newItem.volume=amount;
		if(newItem.isValid())
			(*depthAsks)<<newItem;
	}
	else
	{
		if(depthBids==0)return;
		DepthItem newItem;
		newItem.price=price;
		newItem.volume=amount;
		if(newItem.isValid())
			(*depthBids)<<newItem;
	}
}

void Exchange_MtGox::depthSubmitOrder(QMap<double,double> *currentMap ,double priceDouble, double amount, bool isAsk)
{
	if(priceDouble==0.0||amount==0.0)return;

	if(isAsk)
	{
		(*currentMap)[priceDouble]=amount;
		if(lastDepthAsksMap.value(priceDouble,0.0)!=amount)
			depthUpdateOrder(priceDouble,amount,true);
	}
	else
	{
		(*currentMap)[priceDouble]=amount;
		if(lastDepthBidsMap.value(priceDouble,0.0)!=amount)
			depthUpdateOrder(priceDouble,amount,false);
	}
}

void Exchange_MtGox::reloadDepth()
{
	lastDepthBidsMap.clear();
	lastDepthAsksMap.clear();
	lastDepthData.clear();
	Exchange::reloadDepth();
}

void Exchange_MtGox::dataReceivedAuth(QByteArray data, int reqType)
{
	if(debugLevel)logThread->writeLog("RCV: "+data);
	bool success=getMidData("{\"result\":\"","\",",&data)=="success";

	switch(reqType)
	{
	case 101:
		if(!success)break;
		if(data.startsWith("{\"result\":\"success\",\"data\":{\"lag"))
		{
			double currentLag=getMidData("lag_secs\":",",\"",&data).toDouble();
			static double lastLag=0.0001;
			if(lastLag!=currentLag&&currentLag<9999.0)emit apiLagChanged(currentLag);//lag
			lastLag=currentLag;
		}
		else if(debugLevel)logThread->writeLog("Invalid lag data:"+data,2);
		break;
	case 103: //ticker
		if(!success)break;
			if(data.startsWith("{\"result\":\"success\",\"data\":{\"high"))
			{
				QByteArray tickerHigh=getMidData("high\":{\"value\":\"","",&data);
				if(!tickerHigh.isEmpty())
				{
					double newTickerHigh=tickerHigh.toDouble();
					if(newTickerHigh!=lastTickerHigh)emit tickerHighChanged(newTickerHigh);
					lastTickerHigh=newTickerHigh;
				}

				QByteArray tickerLow=getMidData("low\":{\"value\":\"","",&data);
				if(!tickerLow.isEmpty())
				{
					double newTickerLow=tickerLow.toDouble();
					if(newTickerLow!=lastTickerLow)emit tickerLowChanged(newTickerLow);
					lastTickerLow=newTickerLow;
				}

				QByteArray tickerVolume=getMidData("vol\":{\"value\":\"","",&data);
				if(!tickerVolume.isEmpty())
				{
					double newTickerVolume=tickerVolume.toDouble();
					if(newTickerVolume!=lastTickerVolume)emit tickerVolumeChanged(newTickerVolume);
					lastTickerVolume=newTickerVolume;
				}
			}
			else if(debugLevel)logThread->writeLog("Invalid ticker data:"+data,2);
		break;//ticker
	case 104: //ticker fast
		if(data.startsWith("{\"result\":\"success\",\"data\":{\"last_local"))
		{

			QByteArray tickerSell=getMidData("buy\":{\"value\":\"","",&data);
			if(!tickerSell.isEmpty())
			{
				double newTickerSell=tickerSell.toDouble();
				if(newTickerSell!=lastTickerSell)emit tickerSellChanged(newTickerSell);
				lastTickerSell=newTickerSell;
			}

			QByteArray tickerBuy=getMidData("sell\":{\"value\":\"","",&data);
			if(!tickerBuy.isEmpty())
			{
				double newTickerBuy=tickerBuy.toDouble();
				if(newTickerBuy!=lastTickerBuy)emit tickerBuyChanged(newTickerBuy);
				lastTickerBuy=newTickerBuy;
			}
			qint64 tickerNow=getMidData("now\":\"","\"}",&data).toLongLong();
			if(tickerLastDate<tickerNow)
			{
				QByteArray tickerLast=getMidData("last\":{\"value\":\"","",&data);
				double newTickerLast=tickerLast.toDouble();
				if(newTickerLast>0.0)
				{
					emit tickerLastChanged(newTickerLast);
					tickerLastDate=tickerNow;
				}
			}
			if(isFirstTicker)
			{
				emit firstTicker();
				isFirstTicker=false;
			}
		}
		else if(debugLevel)logThread->writeLog("Invalid ticker fast data:"+data,2);
			break; //ticker fast
	case 109: //money/trades/fetch
		if(success&&data.size()>32)
		{
			if(data.startsWith("{\"result\":\"success\",\"data\":[{\"date"))
			{
				QStringList tradeList=QString(data).split("\"},{\"");
				QList<TradesItem> *newTradesItems=new QList<TradesItem>;
				for(int n=0;n<tradeList.count();n++)
				{
					QByteArray tradeData=tradeList.at(n).toAscii();
					qint64 currentTid=getMidData("\"tid\":\"","\",\"",&tradeData).toLongLong();
					if(lastTradesDate>=currentTid||currentTid==0)continue;

					TradesItem newItem;
					newItem.amount=getMidData("\"amount\":\"","\",",&tradeData).toDouble();
					newItem.price=getMidData("\"price\":\"","\",",&tradeData).toDouble();

					newItem.symbol=getMidData("\"item\":\"","\",\"",&tradeData)+getMidData("\"price_currency\":\"","\",\"",&tradeData);
					newItem.orderType=getMidData("\"trade_type\":\"","\"",&tradeData)=="ask"?1:-1;
					newItem.date=getMidData("date\":",",",&tradeData).toUInt();

					if(newItem.isValid())(*newTradesItems)<<newItem;
					else if(debugLevel)logThread->writeLog("Invalid trades fetch data line:"+tradeData,2);

					if(n==tradeList.count()-1)
					{
						emit tickerLastChanged(newItem.price);
						tickerLastDate=currentTid;
						lastTradesDate=currentTid;
						lastTradesDateCache=QByteArray::number(tickerLastDate+1);
					}
				}
				if(newTradesItems->count())emit addLastTrades(newTradesItems);
				else delete newTradesItems;
			}
			else if(debugLevel)logThread->writeLog("Invalid trades fetch data:"+data,2);
		}
		break;
	case 111: //depth
		if(data.startsWith("{\"result\":\"success\",\"data\":{\"now\""))
		{
			emit depthRequestReceived();

			if(lastDepthData!=data)
			{
				lastDepthData=data;

				depthAsks=new QList<DepthItem>;
				depthBids=new QList<DepthItem>;

				QMap<double,double> currentAsksMap;
				QStringList asksList=QString(getMidData("\"asks\":[{","}]",&data)).split("},{");
				double groupedPrice=0.0;
				double groupedVolume=0.0;
				int rowCounter=0;
				for(int n=0;n<asksList.count();n++)
				{
					if(baseValues.depthCountLimit&&rowCounter>=baseValues.depthCountLimit)break;
					QByteArray currentRow=asksList.at(n).toAscii();
					double priceDouble=getMidData("price\":",",\"",&currentRow).toDouble();
					double amount=getMidData("amount\":",",\"",&currentRow).toDouble();

					if(baseValues.groupPriceValue>0.0)
					{
						if(n==0)
						{
							emit depthFirstOrder(priceDouble,amount,true);
							groupedPrice=baseValues.groupPriceValue*(int)(priceDouble/baseValues.groupPriceValue);
							groupedVolume=amount;
						}
						else
						{
							bool matchCurrentGroup=priceDouble<groupedPrice+baseValues.groupPriceValue;
							if(matchCurrentGroup)groupedVolume+=amount;
							if(!matchCurrentGroup||n==asksList.count()-1)
							{
								depthSubmitOrder(&currentAsksMap,groupedPrice+baseValues.groupPriceValue,groupedVolume,true);
								rowCounter++;
								groupedVolume=amount;
								groupedPrice+=baseValues.groupPriceValue;
							}
						}
					}
					else
					{
					depthSubmitOrder(&currentAsksMap,priceDouble,amount,true);
					rowCounter++;
					}
				}
				QList<double> currentAsksList=lastDepthAsksMap.keys();
				for(int n=0;n<currentAsksList.count();n++)
					if(currentAsksMap.value(currentAsksList.at(n),0)==0)depthUpdateOrder(currentAsksList.at(n),0.0,true);//Remove price
				lastDepthAsksMap=currentAsksMap;

				QMap<double,double> currentBidsMap;
				QStringList bidsList=QString(getMidData("\"bids\":[{","}]",&data)).split("},{");
				groupedPrice=0.0;
				groupedVolume=0.0;
				rowCounter=0;
				for(int n=bidsList.count()-1;n>=0;n--)
				{
					if(baseValues.depthCountLimit&&rowCounter>=baseValues.depthCountLimit)break;
					QByteArray currentRow=bidsList.at(n).toAscii();
					double priceDouble=getMidData("price\":",",\"",&currentRow).toDouble();
					double amount=getMidData("amount\":",",\"",&currentRow).toDouble();

					if(baseValues.groupPriceValue>0.0)
					{
						if(n==bidsList.count()-1)
						{
							emit depthFirstOrder(priceDouble,amount,false);
							groupedPrice=baseValues.groupPriceValue*(int)(priceDouble/baseValues.groupPriceValue);
							groupedVolume=amount;
						}
						else
						{
							bool matchCurrentGroup=priceDouble>groupedPrice+baseValues.groupPriceValue;
							if(matchCurrentGroup)groupedVolume+=amount;
							if(!matchCurrentGroup||n==0)
							{
								depthSubmitOrder(&currentBidsMap,groupedPrice-baseValues.groupPriceValue,groupedVolume,false);
								rowCounter++;
								groupedVolume=amount;
								groupedPrice-=baseValues.groupPriceValue;
							}
						}
					}
					else
					{
						depthSubmitOrder(&currentBidsMap,priceDouble,amount,false);
						rowCounter++;
					}
				}
				QList<double> currentBidsList=lastDepthBidsMap.keys();
				for(int n=0;n<currentBidsList.count();n++)
					if(currentBidsMap.value(currentBidsList.at(n),0)==0)depthUpdateOrder(currentBidsList.at(n),0.0,false);//Remove price
				lastDepthBidsMap=currentBidsMap;

				emit depthSubmitOrders(depthAsks, depthBids);
				depthAsks=0;
				depthBids=0;
			}
		}
		else if(debugLevel)logThread->writeLog("Invalid depth data:"+data,2);
		break;
	case 202: //info
		{
			if(!success)break;
			if(data.startsWith("{\"result\":\"success\",\"data\":{\"Login"))
			{
				lastInfoReceived=true;
				if(debugLevel)logThread->writeLog("Info: "+data);
				if(apiLogin.isEmpty())
				{
					QByteArray login=getMidData("Login\":\"","\",",&data);
					if(!login.isEmpty()){apiLogin=login;emit loginChanged(login);}
				}

				QByteArray btcBalance=getMidData(baseValues.currentPair.currAStr+"\":{\"Balance\":{\"value\":\"","",&data);
				if(!btcBalance.isEmpty())
				{
					double newBtcBalance=btcBalance.toDouble();
					if(lastBtcBalance!=newBtcBalance)emit accBtcBalanceChanged(newBtcBalance);
					lastBtcBalance=newBtcBalance;
				}

				QByteArray usdBalance=getMidData(baseValues.currentPair.currBStr+"\":{\"Balance\":{\"value\":\"","",&data);
				if(!usdBalance.isEmpty())
				{
					double newUsdBalance=usdBalance.toDouble();
					if(newUsdBalance!=lastUsdBalance)emit accUsdBalanceChanged(newUsdBalance);
					lastUsdBalance=newUsdBalance;
				}

				QByteArray monthValue=getMidData("Monthly_Volume\":{\"value\":\"","\",",&data);
				if(!monthValue.isEmpty())
				{
					double newVolume=monthValue.toDouble();
					if(newVolume!=lastVolume)emit accVolumeChanged(newVolume);
					lastVolume=newVolume;
				}

				QByteArray tradeFee=getMidData("Trade_Fee\":","}",&data);
				if(!tradeFee.isEmpty())
				{
					double newFee=tradeFee.toDouble();
					if(newFee!=lastFee)emit accFeeChanged(newFee);
					lastFee=newFee;
				}
				if(isFirstAccInfo)
				{
					QByteArray rights=getMidData("Rights\":","]",&data);
					if(!rights.isEmpty())
					{
						bool isRightsGood=rights.contains("get_info")&&rights.contains("trade");
						if(!isRightsGood)emit showErrorMessage("I:>invalid_rights");
						isFirstAccInfo=false;
					}
				}
			}
			else if(debugLevel)logThread->writeLog("Invalid Info data:"+data,2);
		}
		break;//info
	case 204://orders
		if(!success)break;
		if(data.size()<=30){lastOrders.clear();emit ordersIsEmpty();break;}
		if(data.startsWith("{\"result\":\"success\",\"data\":[{\"oid"))
		{
			if(lastOrders!=data)
			{
				lastOrders=data;
				QByteArray currentOrderData=getMidData("oid","\"actions\":",&data);
				QList<OrderItem> *orders=new QList<OrderItem>;
				while(currentOrderData.size())
				{
					OrderItem currentOrder;
					currentOrder.oid=getMidData("\":\"","\",\"",&currentOrderData);
					currentOrder.date=getMidData(",\"date\":",",",&currentOrderData).toUInt();
					currentOrder.type=getMidData("\"type\":\"","\",\"",&currentOrderData).toLower()=="ask";

					QByteArray statusBytes=getMidData("\"status\":\"","\",\"",&currentOrderData).toLower();
					//0=Canceled, 1=Open, 2=Pending, 3=Post-Pending
					if(statusBytes=="canceled")currentOrder.status=0;
					else
					if(statusBytes=="open")currentOrder.status=1;
					else
					if(statusBytes=="pending")currentOrder.status=2;
					else
					if(statusBytes=="post-pending")currentOrder.status=3;

					currentOrder.amount=getMidData("\"amount\":{\"value\":\"","\",\"",&currentOrderData).toDouble();
					currentOrder.price=getMidData("\"price\":{\"value\":\"","\",\"",&currentOrderData).toDouble();
					currentOrder.symbol=getMidData("\"item\":\"","\",\"",&currentOrderData)+getMidData("\"currency\":\"","\",\"",&currentOrderData);
					if(currentOrder.isValid())(*orders)<<currentOrder;
					if(data.size()>currentOrderData.size())data.remove(0,currentOrderData.size());
					currentOrderData=getMidData("oid","\"actions\"",&data);
				}
				emit ordersChanged(orders);

				lastInfoReceived=false;
			}
		}
		else if(debugLevel)logThread->writeLog("Invalid Orders data:"+data,2);
		break;//orders
	case 305: //order/cancel
		{
			if(!success)break;
			QByteArray oid=getMidData("oid\":\"","\",\"",&data);
			if(!oid.isEmpty())emit orderCanceled(oid);
			else if(debugLevel)logThread->writeLog("Invalid Order/Cancel data:"+data,2);
		}
		break;//order/cancel
	case 306: //order/buy
			if(!success||!debugLevel)break;
			   if(data.startsWith("{\"result\":\"success\",\"data\":\""))logThread->writeLog("Buy OK: "+data);
			  else logThread->writeLog("Invalid Order Buy Data:"+data);
			break;//order/buy
	case 307: //order/sell
			if(!success||!debugLevel)break;
			  if(data.startsWith("{\"result\":\"success\",\"data\":\""))logThread->writeLog("Sell OK: "+data);
			  else logThread->writeLog("Invalid Order Sell Data:"+data);
			 break;//order/sell
	case 208: //money/wallet/history 
		if(!success)break;
		if(data.startsWith("{\"result\":\"success\",\"data\":{\"records"))
		{
			if(lastHistory!=data)
			{
				lastHistory=data;
				QList<HistoryItem> *historyItems=new QList<HistoryItem>;

				QStringList dataList=QString(data).split("},{");
				for(int n=0;n<dataList.count();n++)
				{
					HistoryItem currentHistoryItem;

					QByteArray curLog(dataList.at(n).toAscii());
					QByteArray logType=getMidData("\"Type\":\"","\",\"",&curLog);

					if(logType=="out")currentHistoryItem.type=1;
					else 
					if(logType=="in")currentHistoryItem.type=2;
					else 
					if(logType=="fee")currentHistoryItem.type=3;
					else 
					if(logType=="deposit")currentHistoryItem.type=4;
					else
					if(logType=="withdraw")currentHistoryItem.type=5;
					if(currentHistoryItem.type)
					{
						QByteArray currencyA("USD");
						currentHistoryItem.volume=getMidData("\"Value\":{\"value\":\"","\",\"",&curLog).toDouble();
						currentHistoryItem.dateTimeInt=getMidData("\"Date\":",",\"",&curLog).toUInt();
						QByteArray logText=getMidData(" at ","\",\"",&curLog);

						QByteArray priceSign;
						static QList<QPair<QByteArray,QByteArray> > utfSignList;
						if(utfSignList.count()==0)
						{
							QPair<QByteArray,QByteArray> curPair;
							curPair.first="$";curPair.second="USD";utfSignList<<curPair;
							curPair.first="\\u00a0\\u20ac";curPair.second="EUR";utfSignList<<curPair;
							curPair.first="\\u00a0RUB";curPair.second="RUB";utfSignList<<curPair;
							curPair.first="AU$";curPair.second="AUD";utfSignList<<curPair;
							curPair.first="CA$";curPair.second="CAD";utfSignList<<curPair;
							curPair.first="\\u00a0CHF";curPair.second="CHF";utfSignList<<curPair;
							curPair.first="\\u00a0\\u5143";curPair.second="CNY";utfSignList<<curPair;
							curPair.first="\\u00a0CZK";curPair.second="CZK";utfSignList<<curPair;
							curPair.first="\\u00a0Kr";curPair.second="DKK";utfSignList<<curPair;
							curPair.first="\\u00a3";curPair.second="GBP";utfSignList<<curPair;
							curPair.first="HK$";curPair.second="HKD";utfSignList<<curPair;
							curPair.first="\\u00a5";curPair.second="JPY";utfSignList<<curPair;
							curPair.first="\\u00a0Kr";curPair.second="NOK";utfSignList<<curPair;
							curPair.first="NZ$";curPair.second="NZD";utfSignList<<curPair;
							curPair.first="\\u00a0z\\u0142";curPair.second="PLN";utfSignList<<curPair;
							curPair.first="\\u00a0Kr";curPair.second="SEK";utfSignList<<curPair;
							curPair.first="SG$";curPair.second="SGD";utfSignList<<curPair;
							curPair.first=" \\u0e3f";curPair.second="THB";utfSignList<<curPair;
						}

						for(int n=0;n<utfSignList.count();n++)
						{
							if(logText.contains(utfSignList.at(n).first))
							{
								logText.replace(utfSignList.at(n).first,"");
								priceSign=baseValues.currencyMap.value(utfSignList.at(n).second,CurencyInfo("$")).sign;
								break;
							}
						}
						if(priceSign.isEmpty())priceSign="$";

						translateUnicodeOne(&logText);
						QByteArray priceValue;
						for(int n=0;n<logText.size();n++)
						{
							if(QChar(logText.at(n)).isSpace())break;
							if(QChar(logText.at(n)).isDigit()||logText.at(n)=='.')priceValue.append(logText.at(n));
						}

						currentHistoryItem.price=priceValue.toDouble();
						currentHistoryItem.symbol=getMidData("\"currency\":\"","\"",&curLog)+baseValues.currencyMapSign.value(priceSign,"USD");
						if(currentHistoryItem.isValid())(*historyItems)<<currentHistoryItem;
					}
				}
				emit historyChanged(historyItems);
			}
		}
		else if(debugLevel)logThread->writeLog("Invalid History data:"+data.left(200),2);
		break;//money/wallet/history
	default: break;
	}

	static int errorCount=0;
	if(!success)
	{
		errorCount++;
		if(errorCount<3)return;
		QString errorString=getMidData("error\":\"","\"",&data);
		QString tokenString=getMidData("token\":\"","\"}",&data);
		if(debugLevel)logThread->writeLog(errorString.toAscii()+" "+tokenString.toAscii(),2);
		if(errorString.isEmpty())return;
		if(errorString==QLatin1String("Order not found"))return;
		errorString.append("<br>"+tokenString);
		errorString.append("<br>"+QString::number(reqType));
		if(reqType<300)emit showErrorMessage("I:>"+errorString);
	}
	else errorCount=0;
}