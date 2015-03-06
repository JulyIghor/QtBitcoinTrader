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

#include "exchange_lakebtc.h"

Exchange_LakeBTC::Exchange_LakeBTC(QByteArray pRestSign, QByteArray pRestKey)
	: Exchange()
{
	exchangeDisplayOnlyCurrentPairOpenOrders=true;
	buySellAmountExcludedFee=true;
	clearHistoryOnCurrencyChanged=false;
    isLastTradesTypeSupported=true;
	balanceDisplayAvailableAmount=false;
	minimumRequestIntervalAllowed=600;
	calculatingFeeMode=1;
	baseValues.exchangeName="LakeBTC";
	depthAsks=0;
	depthBids=0;
	forceDepthLoad=false;
	julyHttpAuth=0;
	julyHttpPublic=0;
	tickerOnly=false;
	setApiKeySecret(pRestKey,pRestSign);

	currencyMapFile="LakeBTC";
	baseValues.currentPair.name="BTC/USD";
	baseValues.currentPair.setSymbol("BTCUSD");
	baseValues.currentPair.currRequestPair="USD";
	baseValues.currentPair.priceDecimals=2;
	baseValues.currentPair.priceMin=qPow(0.1,baseValues.currentPair.priceDecimals);
	baseValues.currentPair.tradeVolumeMin=0.001;
	baseValues.currentPair.tradePriceMin=0.1;

    defaultCurrencyParams.currADecimals=3;
    defaultCurrencyParams.currBDecimals=2;
	defaultCurrencyParams.currABalanceDecimals=8;
	defaultCurrencyParams.currBBalanceDecimals=5;
	defaultCurrencyParams.priceDecimals=2;
	defaultCurrencyParams.currABalanceDecimals=8;
	defaultCurrencyParams.currBBalanceDecimals=5;
	defaultCurrencyParams.priceMin=qPow(0.1,baseValues.currentPair.priceDecimals);

	supportsLoginIndicator=true;
    supportsAccountVolume=false;

	moveToThread(this);
	authRequestTime.restart();
}

Exchange_LakeBTC::~Exchange_LakeBTC()
{
}

void Exchange_LakeBTC::clearVariables()
{
	isFirstTicker=true;
	cancelingOrderIDs.clear();
	Exchange::clearVariables();
	secondPart=0;
	apiDownCounter=0;
    historyLastDate.clear();
	lastHistory.clear();
	lastOrders.clear();
	historyLastTradesRequest="bctrades";
	reloadDepth();
	lastInfoReceived=false;
	lastFetchTid.clear();
}

void Exchange_LakeBTC::clearValues()
{
	if(julyHttpAuth)julyHttpAuth->clearPendingData();
	if(julyHttpPublic)julyHttpPublic->clearPendingData();
	clearVariables();
}

QByteArray Exchange_LakeBTC::getMidData(QString a, QString b,QByteArray *data)
{
	QByteArray rez;
	if(b.isEmpty())b="\",";
	int startPos=data->indexOf(a,0);
	if(startPos>-1)
	{
		int endPos=data->indexOf(b,startPos+a.length());
		if(endPos>-1)rez=data->mid(startPos+a.length(),endPos-startPos-a.length());
	}
	return rez;
}

void Exchange_LakeBTC::secondSlot()
{
	static int infoCounter=0;
	switch(infoCounter)
	{
	case 0: if(!tickerOnly&&!isReplayPending(204))sendToApi(204,"getOrders",true,true/*,"false"*/); break;
	case 1: if(!isReplayPending(202))sendToApi(202,"getAccountInfo",true,true); break;
	case 2: if(lastHistory.isEmpty()&&!isReplayPending(208))sendToApi(208,"getTrades",true,true); break;
	
	default: break;
	}

	if(isDepthEnabled()&&(forceDepthLoad||/*infoCounter==3&&*/!isReplayPending(111)))
	{
		if(!isReplayPending(111))
		{
			emit depthRequested();
			sendToApi(111,"bcorderbook",false,true);
		}
		forceDepthLoad=true;
	}

	if(!isReplayPending(103))sendToApi(103,"ticker",false,true);

	if(infoCounter==3&&!isReplayPending(109))
	{
		if(!lastFetchTid.isEmpty())historyLastTradesRequest="bctrades?since="+lastFetchTid;
		else
		{
			quint32 fetchTid=static_cast<quint32>(time(NULL)) - 600;
			QByteArray tmtonce=QByteArray::number(fetchTid);
			historyLastTradesRequest="bctrades?since="+tmtonce;
		}
		sendToApi(109,historyLastTradesRequest,false,true);
	}

	if(infoCounter++==3)
	{
		infoCounter=0;
	}

	Exchange::secondSlot();
}

bool Exchange_LakeBTC::isReplayPending(int reqType)
{
	if(reqType<200)
	{
		if(julyHttpPublic==0)return false;
		return julyHttpPublic->isReqTypePending(reqType);
	}

	if(julyHttpAuth==0)return false;
	return julyHttpAuth->isReqTypePending(reqType);
}

void Exchange_LakeBTC::getHistory(bool force)
{
	if(tickerOnly)return;
	if(force)lastHistory.clear();
	if(!isReplayPending(208))sendToApi(208,"getTrades",true,true);
}

void Exchange_LakeBTC::buy(QString symbol, double apiBtcToBuy, double apiPriceToBuy)
{
	if(tickerOnly)return;

    CurrencyPairItem pairItem;
    pairItem=baseValues.currencyPairMap.value(symbol,pairItem);
    if(pairItem.symbol.isEmpty())return;

    QByteArray data=byteArrayFromDouble(apiPriceToBuy,pairItem.priceDecimals,0)+","+byteArrayFromDouble(apiBtcToBuy,pairItem.currADecimals,0)+",\""+pairItem.currRequestPair+"\"";
	if(debugLevel)logThread->writeLog("Buy: "+data,2);
	sendToApi(306,"buyOrder",true,true,data);
}

void Exchange_LakeBTC::sell(QString symbol, double apiBtcToSell, double apiPriceToSell)
{
	if(tickerOnly)return;

    CurrencyPairItem pairItem;
    pairItem=baseValues.currencyPairMap.value(symbol,pairItem);
    if(pairItem.symbol.isEmpty())return;

    QByteArray data=byteArrayFromDouble(apiPriceToSell,pairItem.priceDecimals,0)+","+byteArrayFromDouble(apiBtcToSell,baseValues.currentPair.currADecimals,0)+",\""+pairItem.currRequestPair+"\"";
	if(debugLevel)logThread->writeLog("Sell: "+data,2);
	sendToApi(307,"sellOrder",true,true,data);
}

void Exchange_LakeBTC::cancelOrder(QString symbol, QByteArray order)
{
	if(tickerOnly)return;

    if(symbol.isEmpty())symbol=baseValues.currentPair.symbol;

    CurrencyPairItem pairItem;
    pairItem=baseValues.currencyPairMap.value(symbol,pairItem);
    if(pairItem.symbol.isEmpty())return;

	cancelingOrderIDs<<order;
	if(debugLevel)logThread->writeLog("Cancel order: "+order,2);
	sendToApi(305,"cancelOrder",true,true,order);
}

void Exchange_LakeBTC::sendToApi(int reqType, QByteArray method, bool auth, bool sendNow, QByteArray commands)
{
	if(auth)
	{
		if(julyHttpAuth==0)
		{
			julyHttpAuth=new JulyHttp("www.lakebtc.com","",this,true,true,"application/json-rpc");
			connect(julyHttpAuth,SIGNAL(anyDataReceived()),baseValues_->mainWindow_,SLOT(anyDataReceived()));
			connect(julyHttpAuth,SIGNAL(setDataPending(bool)),baseValues_->mainWindow_,SLOT(setDataPending(bool)));
			connect(julyHttpAuth,SIGNAL(apiDown(bool)),baseValues_->mainWindow_,SLOT(setApiDown(bool)));
			connect(julyHttpAuth,SIGNAL(errorSignal(QString)),baseValues_->mainWindow_,SLOT(showErrorMessage(QString)));
			connect(julyHttpAuth,SIGNAL(sslErrorSignal(const QList<QSslError> &)),this,SLOT(sslErrors(const QList<QSslError> &)));
			connect(julyHttpAuth,SIGNAL(dataReceived(QByteArray,int)),this,SLOT(dataReceivedAuth(QByteArray,int)));
		}
		QByteArray signatureParams;
		signatureParams=commands;
		signatureParams.replace("\"","");
		signatureParams.replace("true","1");
		signatureParams.replace("false","");

		QByteArray postData;
		QByteArray appendedHeader;

		static int tonceCounter=0;		
		static quint32 lastTonce=static_cast<quint32>(time(NULL));
		quint32 newTonce=static_cast<quint32>(time(NULL));

		if(lastTonce!=newTonce)
		{
			tonceCounter=0;
			lastTonce=newTonce;
		}
		else
		{
			tonceCounter+=10;
			if(tonceCounter>99)tonceCounter=0;
		}

		QByteArray tonceCounterData=QByteArray::number(tonceCounter);
		if(tonceCounter>9)tonceCounterData.append("0000");
		else tonceCounterData.append("00000");

		QByteArray tonce=QByteArray::number(newTonce)+tonceCounterData;

		QByteArray signatureString="tonce="+tonce+"&accesskey="+getApiKey()+"&requestmethod=post&id=1&method="+method+"&params="+signatureParams;
		signatureString=getApiKey()+":"+hmacSha1(getApiSign(),signatureString).toHex();
		if(debugLevel&&reqType>299)logThread->writeLog(postData);

		postData="{\"method\":\""+method+"\",\"params\":["+commands+"],\"id\":1}";
		appendedHeader="Authorization: Basic "+signatureString.toBase64()+"\r\n";
		appendedHeader+="Json-Rpc-Tonce: "+tonce+"\r\n";

		if(sendNow)
			julyHttpAuth->sendData(reqType, "POST /api_v1",postData,appendedHeader);
		else
			julyHttpAuth->prepareData(reqType, "POST /api_v1",postData,appendedHeader);
	}
	else
	{
		if(julyHttpPublic==0)
		{
			julyHttpPublic=new JulyHttp("www.lakebtc.com","",this,true,true,"application/json-rpc");
			connect(julyHttpPublic,SIGNAL(anyDataReceived()),baseValues_->mainWindow_,SLOT(anyDataReceived()));
			connect(julyHttpPublic,SIGNAL(setDataPending(bool)),baseValues_->mainWindow_,SLOT(setDataPending(bool)));
			connect(julyHttpPublic,SIGNAL(apiDown(bool)),baseValues_->mainWindow_,SLOT(setApiDown(bool)));
			connect(julyHttpPublic,SIGNAL(errorSignal(QString)),baseValues_->mainWindow_,SLOT(showErrorMessage(QString)));
			connect(julyHttpPublic,SIGNAL(sslErrorSignal(const QList<QSslError> &)),this,SLOT(sslErrors(const QList<QSslError> &)));
			connect(julyHttpPublic,SIGNAL(dataReceived(QByteArray,int)),this,SLOT(dataReceivedAuth(QByteArray,int)));
		}
		if(sendNow)
			julyHttpPublic->sendData(reqType, "GET /api_v1/"+method);
		else
			julyHttpPublic->prepareData(reqType, "GET /api_v1/"+method);
	}
}
void Exchange_LakeBTC::depthUpdateOrder(QString symbol, double price, double amount, bool isAsk)
{
    if(symbol!=baseValues.currentPair.symbol)return;

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

void Exchange_LakeBTC::depthSubmitOrder(QString symbol, QMap<double,double> *currentMap ,double priceDouble, double amount, bool isAsk)
{
    if(symbol!=baseValues.currentPair.symbol)return;

	if(priceDouble==0.0||amount==0.0)return;
	if(isAsk)
	{
		(*currentMap)[priceDouble]=amount;
		if(lastDepthAsksMap.value(priceDouble,0.0)!=amount)
            depthUpdateOrder(symbol,priceDouble,amount,true);
	}
	else
	{
		(*currentMap)[priceDouble]=amount;
		if(lastDepthBidsMap.value(priceDouble,0.0)!=amount)
            depthUpdateOrder(symbol,priceDouble,amount,false);
	}
}

void Exchange_LakeBTC::reloadDepth()
{
	lastDepthBidsMap.clear();
	lastDepthAsksMap.clear();
	lastDepthData.clear();
	Exchange::reloadDepth();
}

void Exchange_LakeBTC::dataReceivedAuth(QByteArray data, int reqType)
{
	if(debugLevel)logThread->writeLog("RCV: "+data);
    if(data.size()&&data.at(0)==QLatin1Char('<'))return;

    bool success=(data.startsWith("{")&&!data.startsWith("{\"error\":"))||data.startsWith("[{");
	if(success&&data.startsWith("401"))success=false;

	switch(reqType)
	{
	case 103: //ticker
		if(!success)break;
		if(data.startsWith("{\"USD\":{"))
		{
			QByteArray tickerHigh=getMidData("\"high\":",",",&data);
			if(!tickerHigh.isEmpty())
			{
                double newTickerHigh=tickerHigh.toDouble();
                if(newTickerHigh!=lastTickerHigh)emit tickerHighChanged(baseValues.currentPair.symbol,newTickerHigh);
				lastTickerHigh=newTickerHigh;
			}

			QByteArray tickerLow=getMidData("\"low\":",",",&data);
			if(!tickerLow.isEmpty())
			{
                double newTickerLow=tickerLow.toDouble();
                if(newTickerLow!=lastTickerLow)emit tickerLowChanged(baseValues.currentPair.symbol,newTickerLow);
				lastTickerLow=newTickerLow;
			}

			QByteArray tickerVolume=getMidData("\"volume\":",",",&data);
			if(!tickerVolume.isEmpty())
			{
                double newTickerVolume=tickerVolume.toDouble();
                if(newTickerVolume!=lastTickerVolume)emit tickerVolumeChanged(baseValues.currentPair.symbol,newTickerVolume);
				lastTickerVolume=newTickerVolume;
			}

			QByteArray tickerLast=getMidData("\"last\":",",",&data);
			if(!tickerLast.isEmpty())
			{
                double newTickerLast=tickerLast.toDouble();
                if(newTickerLast!=lastTickerLast)emit tickerLastChanged(baseValues.currentPair.symbol,newTickerLast);
				lastTickerLast=newTickerLast;
			}

			QByteArray tickerSell=getMidData("\"bid\":",",",&data);
			if(!tickerSell.isEmpty())
			{
                double newTickerSell=tickerSell.toDouble();
                if(newTickerSell!=lastTickerSell)emit tickerSellChanged(baseValues.currentPair.symbol,newTickerSell);
				lastTickerSell=newTickerSell;
			}

			QByteArray tickerBuy=getMidData("\"ask\":","}",&data);
			if(!tickerBuy.isEmpty())
			{
                double newTickerBuy=tickerBuy.toDouble();
                if(newTickerBuy!=lastTickerBuy)emit tickerBuyChanged(baseValues.currentPair.symbol,newTickerBuy);
				lastTickerBuy=newTickerBuy;
			}

			if(isFirstTicker)
			{
				emit firstTicker();
				isFirstTicker=false;
			}
		}
		else if(debugLevel)logThread->writeLog("Invalid ticker data:"+data,2);
		break;//ticker
	case 109: //trades
		if(success&&data.size()>32)
		{
			if(data.startsWith("[{\"date\":"))
			{
				QStringList tradeList=QString(data).split("},{");
				for(int i = 0; i < (tradeList.size()/2); i++) tradeList.swap(i,tradeList.size()-(1+i));
				QList<TradesItem> *newTradesItems=new QList<TradesItem>;
				for(int n=0;n<tradeList.count();n++)
				{
					QByteArray tradeData=tradeList.at(n).toLatin1();
					if(!tradeData.lastIndexOf('}'))tradeData.append("}");
					QByteArray nextFetchTid=getMidData("\"tid\":","}",&tradeData);
					if(nextFetchTid<=lastFetchTid)continue;
					TradesItem newItem;
					newItem.amount=getMidData("\"amount\":",",",&tradeData).toDouble();
					newItem.price=getMidData("\"price\":",",",&tradeData).toDouble();
					newItem.date=getMidData("\"date\":",",",&tradeData).toUInt();
                    newItem.symbol=baseValues.currentPair.symbol;
					newItem.orderType=100;

					if(newItem.isValid())(*newTradesItems)<<newItem;
					else if(debugLevel)logThread->writeLog("Invalid trades fetch data line:"+tradeData,2);

                    if(n==tradeList.count()-1&&!nextFetchTid.isEmpty())lastFetchTid=nextFetchTid;
				}
                if(newTradesItems->count())emit addLastTrades(baseValues.currentPair.symbol,newTradesItems);
				else delete newTradesItems;
			}
            else if(debugLevel)logThread->writeLog("Invalid trades fetch data:"+data,2);
		}
		break;
case 111: //api/order_book
		if(data.startsWith("{\"asks\":"))
		{
			emit depthRequestReceived();

			if(lastDepthData!=data)
			{
				lastDepthData=data;
				depthAsks=new QList<DepthItem>;
				depthBids=new QList<DepthItem>;

				QMap<double,double> currentBidsMap;
				
				QStringList bidsList=QString(getMidData("\"bids\":[[","]]",&data)).split("],[");
				double groupedPrice=0.0;
				double groupedVolume=0.0;
				int rowCounter=0;
				for(int n=0;n<bidsList.count();n++)
				{
					if(baseValues.depthCountLimit&&rowCounter>=baseValues.depthCountLimit)break;
					QByteArray currentRow=bidsList.at(n).toLatin1().append("}");
					double priceDouble=getMidData("",",",&currentRow).toDouble();
					double amount=getMidData(",","}",&currentRow).toDouble();
					//if(n==0)emit tickerSellChanged(baseValues.currentPair.symbol,priceDouble);
					if(baseValues.groupPriceValue>0.0)
					{
						if(n==0)
						{
							emit depthFirstOrder(baseValues.currentPair.symbol,priceDouble,amount,false);
							groupedPrice=baseValues.groupPriceValue*(int)(priceDouble/baseValues.groupPriceValue);
							groupedVolume=amount;
						}
						else
						{
							bool matchCurrentGroup=priceDouble>groupedPrice-baseValues.groupPriceValue;
							if(matchCurrentGroup)groupedVolume+=amount;
							if(!matchCurrentGroup||n==bidsList.count()-1)
							{
								depthSubmitOrder(baseValues.currentPair.symbol,
								&currentBidsMap,groupedPrice-baseValues.groupPriceValue,groupedVolume,false);
								rowCounter++;
								groupedVolume=amount;
								groupedPrice-=baseValues.groupPriceValue;
							}
						}
					}
					else
					{
						depthSubmitOrder(baseValues.currentPair.symbol,
						&currentBidsMap,priceDouble,amount,false);
						rowCounter++;
					}
				}
				QList<double> currentBidsList=lastDepthBidsMap.keys();
				for(int n=0;n<currentBidsList.count();n++)
					if(currentBidsMap.value(currentBidsList.at(n),0)==0)depthUpdateOrder(baseValues.currentPair.symbol,currentBidsList.at(n),0.0,false);//Remove price
				lastDepthBidsMap=currentBidsMap;

				QMap<double,double> currentAsksMap;
				QStringList asksList=QString(getMidData("\"asks\":[[","]]",&data)).split("],[");
				groupedPrice=0.0;
				groupedVolume=0.0;
				rowCounter=0;

				for(int n=0;n<bidsList.count();n++)
				{
					if(baseValues.depthCountLimit&&rowCounter>=baseValues.depthCountLimit)break;
					QByteArray currentRow=asksList.at(n).toLatin1().append("}");
					double priceDouble=getMidData("",",",&currentRow).toDouble();
					double amount=getMidData(",","}",&currentRow).toDouble();
					if(n==0)emit tickerBuyChanged(baseValues.currentPair.symbol,priceDouble);

					if(priceDouble>99999)break;

					if(baseValues.groupPriceValue>0.0)
					{
						if(n==0)
						{
							emit depthFirstOrder(baseValues.currentPair.symbol,priceDouble,amount,true);
							groupedPrice=baseValues.groupPriceValue*(int)(priceDouble/baseValues.groupPriceValue);
							groupedVolume=amount;
						}
						else
						{
							bool matchCurrentGroup=priceDouble<groupedPrice+baseValues.groupPriceValue;
							if(matchCurrentGroup)groupedVolume+=amount;
							if(!matchCurrentGroup||n==asksList.count()-1)
							{
								depthSubmitOrder(baseValues.currentPair.symbol,
									&currentAsksMap,groupedPrice+baseValues.groupPriceValue,groupedVolume,true);
								rowCounter++;
								groupedVolume=amount;
								groupedPrice+=baseValues.groupPriceValue;
							}
						}
					}
					else
					{
						depthSubmitOrder(baseValues.currentPair.symbol,
						 &currentAsksMap,priceDouble,amount,true);
						rowCounter++;
					}
				}
				QList<double> currentAsksList=lastDepthAsksMap.keys();
				for(int n=0;n<currentAsksList.count();n++)
				if(currentAsksMap.value(currentAsksList.at(n),0)==0)depthUpdateOrder(baseValues.currentPair.symbol,
				currentAsksList.at(n),0.0,true);//Remove price
				lastDepthAsksMap=currentAsksMap;
				emit depthSubmitOrders(baseValues.currentPair.symbol,depthAsks, depthBids);
				depthAsks=0;
				depthBids=0;
			}
		}
		else if(debugLevel)logThread->writeLog("Invalid depth data:"+data,2);
		break;
	case 202: //AccountInfo
		{
			if(!success)break;
			if(data.startsWith("{\"profile\""))
			{
				lastInfoReceived=true;
				if(debugLevel)logThread->writeLog("Info: "+data);

				if(apiLogin.isEmpty())
				{
					QByteArray login=getMidData("email\":\"","\"",&data);
					if(!login.isEmpty())
					{
						apiLogin=login;
						translateUnicodeStr(&apiLogin);
						emit loginChanged(apiLogin);
					}
				}

				QByteArray feeData="0.002";
                if(!feeData.isEmpty())emit accFeeChanged(baseValues.currentPair.symbol,feeData.toDouble());

				QByteArray balanceData="0.00";
				QByteArray btcBalance=getMidData("BTC\":","}",&data);
				if(!btcBalance.isEmpty())
				{
                    double newBtcBalance=btcBalance.toDouble();
                    if(lastBtcBalance!=newBtcBalance)emit accBtcBalanceChanged(baseValues.currentPair.symbol,newBtcBalance);
					lastBtcBalance=newBtcBalance;
				}

				QByteArray usdBalance=getMidData("USD\":",",",&data);
				if(!usdBalance.isEmpty())
				{
                    double newUsdBalance=usdBalance.toDouble();
                    if(newUsdBalance!=lastUsdBalance)emit accUsdBalanceChanged(baseValues.currentPair.symbol,newUsdBalance);
					lastUsdBalance=newUsdBalance;
				}
			}
			else if(debugLevel)logThread->writeLog("Invalid Info data:"+data,2);
		}
		break;//balance
	case 204://open_orders
		if(!success)break;
		if(data.startsWith("[{\"id\":"))
		{
			if(lastOrders!=data)
			{
				if(data.startsWith("[]")){lastOrders.clear();emit ordersIsEmpty();break;}
				lastOrders=data;
				QStringList ordersList=QString(data).split("},{");

				QList<OrderItem> *orders=new QList<OrderItem>;
				for(int n=0;n<ordersList.count();n++)
				{	
					OrderItem currentOrder;
					
					QByteArray currentOrderData=ordersList.at(n).toAscii();
					currentOrderData.append("}");
					currentOrder.oid=getMidData("\"id\":",",",&currentOrderData);
					currentOrder.date=getMidData("\"at\":","}",&currentOrderData).toUInt();
					currentOrder.type=getMidData("\"category\":\"","\"",&currentOrderData)=="sell";
					QByteArray status=getMidData("\"active\":",",",&currentOrderData);
					//0=Canceled, 1=Open, 2=Pending
					if(status=="true")currentOrder.status=1;
					else
					if(status=="false")currentOrder.status=0;
					else currentOrder.status=2;
					currentOrder.amount=getMidData("\"amount\":",",",&currentOrderData).toDouble();
					currentOrder.price=getMidData("\"ppc\":",",",&currentOrderData).toDouble();
					currentOrder.symbol=baseValues.currentPair.symbol;
					if(currentOrder.isValid())(*orders)<<currentOrder;
				}
				lastOrders=data;
				emit orderBookChanged(baseValues.currentPair.symbol,orders);
				lastInfoReceived=false;
			}
		}
		else if(debugLevel)logThread->writeLog("Invalid Orders data:"+data,2);
		break;//open_orders
	case 305: //cancelOrder
		{
			if(!success)break;
			if(!cancelingOrderIDs.isEmpty())
			{
                if(data.startsWith("{\"result\":true"))emit orderCanceled(baseValues.currentPair.symbol,cancelingOrderIDs.first());
				if(debugLevel)logThread->writeLog("Order canceled:"+cancelingOrderIDs.first(),2);
				cancelingOrderIDs.removeFirst();
			}
		}
		break;//cancelOrder
	case 306: //buyOrder
		if(!success||!debugLevel)break;
		if(data.startsWith("{\"id\":"))logThread->writeLog("Buy OK: "+data);
		else logThread->writeLog("Invalid Order Buy Data:"+data);
		break;//buyOrder
	case 307: //sellOrder
		if(!success||!debugLevel)break;
		if(data.startsWith("{\"id\":"))logThread->writeLog("Sell OK: "+data);
		else logThread->writeLog("Invalid Order Sell Data:"+data);
		break;//sellOrder
	case 208: //money/wallet/history 
		if(!success)break;
		if(data.startsWith("[{\"type\":"))
		{
            if(lastHistory!=data)
            {
                lastHistory=data;

                QList<HistoryItem> *historyItems=new QList<HistoryItem>;
                QStringList dataList=QString(data).split("},{");
                QByteArray newHistoryLastDate;
                QByteArray newHistoryLastId;
                for(int i = 0; i < (dataList.size()/2); i++) dataList.swap(i,dataList.size()-(1+i));
                for(int n=0;n<dataList.count();n++)
                {
                    QByteArray curLog(dataList.at(n).toLatin1());
                    curLog.append("}");

                    QByteArray currentOrderDate=getMidData("date\":",",",&curLog);
                    if(!historyLastDate.isEmpty()&&currentOrderDate<historyLastDate)break;
                    if(n==0)newHistoryLastDate=QByteArray::number(currentOrderDate.toUInt()-1);

                    QByteArray currentOrderID=getMidData("id\":",",",&curLog);
                    if(!historyLastID.isEmpty()&&currentOrderID<=historyLastID)break;
                    if(n==0)newHistoryLastId=currentOrderID;

                    QByteArray transactionType=getMidData("type\":\"","\"",&curLog);
					
					bool isAsk=false;
					if(transactionType=="sell")isAsk=true;
					else if(transactionType!="buy")continue;

					HistoryItem currentHistoryItem;
					currentHistoryItem.symbol="BTCUSD";

					if(isAsk)currentHistoryItem.type=1;
					else currentHistoryItem.type=2;

					double btcAmount=getMidData("\"amount\":",",",&curLog).toDouble();
					if(btcAmount<0.0)btcAmount=-btcAmount;
					currentHistoryItem.volume=btcAmount;

					double usdAmount=getMidData("total\":",",",&curLog).toDouble();
					if(usdAmount<0.0)usdAmount=-usdAmount;
					currentHistoryItem.price=usdAmount/btcAmount;

					currentHistoryItem.dateTimeInt=getMidData("at\":","}",&curLog).toUInt();

					if(currentHistoryItem.isValid())(*historyItems)<<currentHistoryItem;
				}
				emit historyChanged(historyItems);
			}
		}
	default: break;
	}

	if(reqType<200||reqType==204||reqType==305)return;
	static int errorCount=0;
	if(!success)
	{
		errorCount++;
		QString errorString;
		bool invalidMessage=!data.startsWith("{");
		if(!invalidMessage)
			errorString=getMidData("message\":\"","\",",&data)+" Code:"+getMidData("code\":",",",&data);
		else errorString=data;
		if(debugLevel)logThread->writeLog("API Error: "+errorString.toLatin1()+" ReqType:"+QByteArray::number(reqType),2);
		if(errorCount<3&&reqType<300&&!errorString.endsWith("Unauthorized"))return;
		if(errorString.isEmpty())return;
		errorString.append("<br>"+QString::number(reqType));
		if(invalidMessage||reqType<300)emit showErrorMessage("I:>"+errorString);
	}
	else errorCount=0;
}

void Exchange_LakeBTC::sslErrors(const QList<QSslError> &errors)
{
	QStringList errorList;
	for(int n=0;n<errors.count();n++)errorList<<errors.at(n).errorString();
	if(debugLevel)logThread->writeLog(errorList.join(" ").toLatin1(),2);
	emit showErrorMessage("SSL Error: "+errorList.join(" "));
}
