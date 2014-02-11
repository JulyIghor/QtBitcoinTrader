// Copyright (C) 2014 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form http://qtopentrader.com
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#include "exchange_btce.h"
#include <openssl/hmac.h>
#include "main.h"
#include <QDateTime>

Exchange_BTCe::Exchange_BTCe(QByteArray pRestSign, QByteArray pRestKey) 
	: Exchange()
{
	calculatingFeeMode=0;
	baseValues.exchangeName="BTC-e";
	baseValues.currentPair.name="BTC/USD";
	baseValues.currentPair.setSymbol("BTCUSD");
	baseValues.currentPair.currRequestPair="btc_usd";
	baseValues.currentPair.priceDecimals=3;
	minimumRequestIntervalAllowed=600;
	baseValues.currentPair.priceMin=qPow(0.1,baseValues.currentPair.priceDecimals);
	baseValues.currentPair.tradeVolumeMin=0.01;
	baseValues.currentPair.tradePriceMin=0.1;
	lastTickerDate=0;
	lastFetchTid=0;
	depthAsks=0;
	depthBids=0;
	forceDepthLoad=false;
	julyHttp=0;
	isApiDown=false;
	tickerOnly=false;
	privateRestSign=pRestSign;
	privateRestKey=pRestKey;
	moveToThread(this);

	currencyMapFile="BTCe";
	defaultCurrencyParams.currADecimals=8;
	defaultCurrencyParams.currBDecimals=8;
	defaultCurrencyParams.currABalanceDecimals=8;
	defaultCurrencyParams.currBBalanceDecimals=8;
	defaultCurrencyParams.priceDecimals=3;
	defaultCurrencyParams.priceMin=qPow(0.1,baseValues.currentPair.priceDecimals);

	supportsLoginIndicator=false;
	supportsAccountVolume=false;
	supportsExchangeLag=false;

	authRequestTime.restart();
	privateNonce=(QDateTime::currentDateTime().toTime_t()-1371854884)*10;
}

Exchange_BTCe::~Exchange_BTCe()
{
}

void Exchange_BTCe::clearVariables()
{
	isFirstTicker=true;
	isFirstAccInfo=true;
	Exchange::clearVariables();
	lastOpenedOrders=-1;
	apiDownCounter=0;
	lastHistory.clear();
	lastOrders.clear();
	reloadDepth();
	lastFetchTid=QDateTime::currentDateTime().addSecs(-600).toTime_t();
	lastFetchTid=-lastFetchTid;
}

void Exchange_BTCe::clearValues()
{
	clearVariables();
	if(julyHttp)julyHttp->clearPendingData();
}

void Exchange_BTCe::reloadDepth()
{
	lastDepthBidsMap.clear();
	lastDepthAsksMap.clear();
	lastDepthData.clear();
	Exchange::reloadDepth();
}

void Exchange_BTCe::dataReceivedAuth(QByteArray data, int reqType)
{
	bool success=!data.startsWith("{\"success\":0");
	QString errorString;
	if(!success)errorString=getMidData("error\":\"","\"",&data);

	if(debugLevel)logThread->writeLog("RCV: "+data);

	switch(reqType)
	{
	case 103: //ticker
		{
			QByteArray tickerHigh=getMidData("high\":",",\"",&data);
			if(!tickerHigh.isEmpty())
			{
				double newTickerHigh=tickerHigh.toDouble();
				if(newTickerHigh!=lastTickerHigh)emit tickerHighChanged(newTickerHigh);
				lastTickerHigh=newTickerHigh;
			}

			QByteArray tickerLow=getMidData("\"low\":",",\"",&data);
			if(!tickerLow.isEmpty())
			{
				double newTickerLow=tickerLow.toDouble();
				if(newTickerLow!=lastTickerLow)emit tickerLowChanged(newTickerLow);
				lastTickerLow=newTickerLow;
			}

			QByteArray tickerSell=getMidData("\"sell\":",",\"",&data);
			if(!tickerSell.isEmpty())
			{
				double newTickerSell=tickerSell.toDouble();
				if(newTickerSell!=lastTickerSell)emit tickerSellChanged(newTickerSell);
				lastTickerSell=newTickerSell;
			}

			QByteArray tickerBuy=getMidData("\"buy\":",",\"",&data);
			if(!tickerBuy.isEmpty())
			{
				double newTickerBuy=tickerBuy.toDouble();
				if(newTickerBuy!=lastTickerBuy)emit tickerBuyChanged(newTickerBuy);
				lastTickerBuy=newTickerBuy;
			}

			QByteArray tickerVolume=getMidData("\"vol_cur\":",",\"",&data);
			if(!tickerVolume.isEmpty())
			{
				double newTickerVolume=tickerVolume.toDouble();
				if(newTickerVolume!=lastTickerVolume)emit tickerVolumeChanged(newTickerVolume);
				lastTickerVolume=newTickerVolume;
			}

			quint32 newTickerDate=getMidData("\"updated\":","}",&data).toUInt();
			if(lastTickerDate<newTickerDate)
			{
				lastTickerDate=newTickerDate;
				QByteArray tickerLast=getMidData("\"last\":",",\"",&data);
				double tickerLastDouble=tickerLast.toDouble();
				if(tickerLastDouble>0.0)emit tickerLastChanged(tickerLastDouble);
			}

			if(isFirstTicker)
			{
				emit firstTicker();
				isFirstTicker=false;
			}
		}
		break;//ticker
	case 109: //trades
		{
			if(data.size()<10)break;
			QByteArray currentRequestSymbol=getMidData("\"","\":[{",&data).toUpper().replace("_","");
			QStringList tradeList=QString(data).split("},{");
			QList<TradesItem> *newTradesItems=new QList<TradesItem>;

			for(int n=tradeList.count()-1;n>=0;n--)
			{
				QByteArray tradeData=tradeList.at(n).toAscii()+"}";
				TradesItem newItem;
				newItem.date=getMidData("timestamp\":","}",&tradeData).toUInt();
				newItem.price=getMidData("\"price\":",",\"",&tradeData).toDouble();
				if(lastFetchTid<0&&newItem.date<-lastFetchTid)continue;
				quint32 currentTid=getMidData("\"tid\":",",\"",&tradeData).toUInt();
				if(currentTid<1000||lastFetchTid>=currentTid)continue;
				lastFetchTid=currentTid;
				if(n==0&&lastTickerDate<newItem.date)
				{
					lastTickerDate=newItem.date;
					emit tickerLastChanged(newItem.price);
				}
				newItem.amount=getMidData("\"amount\":",",\"",&tradeData).toDouble();
				newItem.symbol=currentRequestSymbol;
				newItem.orderType=getMidData("\"type\":\"","\"",&tradeData)=="ask"?1:-1;

				if(newItem.isValid())(*newTradesItems)<<newItem;
				else if(debugLevel)logThread->writeLog("Invalid trades fetch data line:"+tradeData,2);
			}
			if(newTradesItems->count())emit addLastTrades(newTradesItems);
			else delete newTradesItems;
		}
		break;//trades
	case 110: //Fee
		{
			QStringList feeList=QString(getMidData("pairs\":{\"","}}}",&data)).split("},\"");
			for(int n=0;n<feeList.count();n++)
			{
				if(!feeList.at(n).startsWith(baseValues.currentPair.currRequestPair))continue;
				QByteArray currentFeeData=feeList.at(n).toAscii()+",";
				double newFee=getMidData("fee\":",",",&currentFeeData).toDouble();
				if(newFee!=lastFee)emit accFeeChanged(newFee);
				lastFee=newFee;
			}
		}
		break;// Fee
	case 111: //depth
		if(data.startsWith("{\""+baseValues.currentPair.currRequestPair+"\":{\"asks"))
		{
			emit depthRequestReceived();

			if(lastDepthData!=data)
			{
				lastDepthData=data;
				depthAsks=new QList<DepthItem>;
				depthBids=new QList<DepthItem>;

				QMap<double,double> currentAsksMap;
				QStringList asksList=QString(getMidData("asks\":[[","]]",&data)).split("],[");
				double groupedPrice=0.0;
				double groupedVolume=0.0;
				int rowCounter=0;

				for(int n=0;n<asksList.count();n++)
				{
					if(baseValues.depthCountLimit&&rowCounter>=baseValues.depthCountLimit)break;
					QStringList currentPair=asksList.at(n).split(",");
					if(currentPair.count()!=2)continue;
					double priceDouble=currentPair.first().toDouble();
					double amount=currentPair.last().toDouble();

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
					if(currentAsksMap.value(currentAsksList.at(n),0)==0)depthUpdateOrder(currentAsksList.at(n),0.0,true);
				lastDepthAsksMap=currentAsksMap;

				QMap<double,double> currentBidsMap;
				QStringList bidsList=QString(getMidData("bids\":[[","]]",&data)).split("],[");
				groupedPrice=0.0;
				groupedVolume=0.0;
				rowCounter=0;

				for(int n=0;n<bidsList.count();n++)
				{
					if(baseValues.depthCountLimit&&rowCounter>=baseValues.depthCountLimit)break;
					QStringList currentPair=bidsList.at(n).split(",");
					if(currentPair.count()!=2)continue;
					double priceDouble=currentPair.first().toDouble();
					double amount=currentPair.last().toDouble();
					if(baseValues.groupPriceValue>0.0)
					{
						if(n==0)
						{
							emit depthFirstOrder(priceDouble,amount,false);
							groupedPrice=baseValues.groupPriceValue*(int)(priceDouble/baseValues.groupPriceValue);
							groupedVolume=amount;
						}
						else
						{
							bool matchCurrentGroup=priceDouble>groupedPrice-baseValues.groupPriceValue;
							if(matchCurrentGroup)groupedVolume+=amount;
							if(!matchCurrentGroup||n==asksList.count()-1)
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
					if(currentBidsMap.value(currentBidsList.at(n),0)==0)depthUpdateOrder(currentBidsList.at(n),0.0,false);
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
			QByteArray fundsData=getMidData("funds\":{","}",&data)+",";
			QByteArray btcBalance=getMidData(baseValues.currentPair.currAStrLow+"\":",",",&fundsData);
			if(!btcBalance.isEmpty())
			{
				double newBtcBalance=btcBalance.toDouble();
				if(lastBtcBalance!=newBtcBalance)emit accBtcBalanceChanged(newBtcBalance);
				lastBtcBalance=newBtcBalance;
			}

			QByteArray usdBalance=getMidData("\""+baseValues.currentPair.currBStrLow+"\":",",",&fundsData);
			if(!usdBalance.isEmpty())
			{
				double newUsdBalance=usdBalance.toDouble();
				if(newUsdBalance!=lastUsdBalance)emit accUsdBalanceChanged(newUsdBalance);
				lastUsdBalance=newUsdBalance;
			}

			int openedOrders=getMidData("open_orders\":",",\"",&data).toInt();
			if(openedOrders==0&&lastOpenedOrders){lastOrders.clear(); emit ordersIsEmpty();}
			lastOpenedOrders=openedOrders;

			if(isFirstAccInfo)
			{
				QByteArray rights=getMidData("rights\":{","}",&data);
				if(!rights.isEmpty())
				{
					bool isRightsGood=rights.contains("info\":1")&&rights.contains("trade\":1");
					if(!isRightsGood)emit showErrorMessage("I:>invalid_rights");
					isFirstAccInfo=false;
				}
			}
		}
		break;//info
	case 204://orders
		{
		if(data.size()<=30)break;
		bool isEmptyOrders=!success&&errorString==QLatin1String("no orders");if(isEmptyOrders)success=true;
		if(lastOrders!=data)
		{
			lastOrders=data;
			if(isEmptyOrders)
			{
				emit ordersIsEmpty();
				break;
			}
			data.replace("return\":{\"","},\"");
			QString rezultData;
			QStringList ordersList=QString(data).split("},\"");
			if(ordersList.count())ordersList.removeFirst();
			if(ordersList.count()==0)return;

			QList<OrderItem> *orders=new QList<OrderItem>;
			for(int n=0;n<ordersList.count();n++)
			{
				OrderItem currentOrder;
				QByteArray currentOrderData="{"+ordersList.at(n).toAscii()+"}";

				currentOrder.oid=getMidData("{","\":{",&currentOrderData);
				currentOrder.date=getMidData("timestamp_created\":",",\"",&currentOrderData).toUInt();
				currentOrder.type=getMidData("type\":\"","\",\"",&currentOrderData)=="sell";
				currentOrder.status=getMidData("status\":","}",&currentOrderData).toInt()+1;
				currentOrder.amount=getMidData("amount\":",",\"",&currentOrderData).toDouble();
				currentOrder.price=getMidData("rate\":",",\"",&currentOrderData).toDouble();
				currentOrder.symbol=getMidData("pair\":\"","\",\"",&currentOrderData).toUpper().replace("_","");
				if(currentOrder.isValid())(*orders)<<currentOrder;
			}
			emit ordersChanged(orders);
		}
		break;//orders
		}
	case 305: //order/cancel
		if(success)
		{
			QByteArray oid=getMidData("order_id\":",",\"",&data);
			if(!oid.isEmpty())emit orderCanceled(oid);
		}
		break;//order/cancel
	case 306: if(debugLevel)logThread->writeLog("Buy OK: "+data,2);break;//order/buy
	case 307: if(debugLevel)logThread->writeLog("Sell OK: "+data,2);break;//order/sell
	case 208: ///history
		{
		bool isEmptyOrders=!success&&errorString==QLatin1String("no trades");if(isEmptyOrders)success=true;
		if(lastHistory!=data)
		{
			lastHistory=data;
			if(!success)break;
			QList<HistoryItem> *historyItems=new QList<HistoryItem>;

			QString newLog(data);
			QStringList dataList=newLog.split("\":{\"");
			if(dataList.count())dataList.removeFirst();
			if(dataList.count())dataList.removeFirst();
			if(dataList.count()==0)return;
			newLog.clear();
			for(int n=0;n<dataList.count();n++)
			{
				HistoryItem currentHistoryItem;

				QByteArray curLog(dataList.at(n).toAscii());
				QByteArray logType=getMidData("type\":\"","\",\"",&curLog);

				if(logType=="sell")currentHistoryItem.type=1;
				else 
				if(logType=="buy")currentHistoryItem.type=2;
				
				if(currentHistoryItem.type)
				{
					QStringList currencyPair;
					if(currentHistoryItem.type==1||currentHistoryItem.type==2)
						currentHistoryItem.symbol=getMidData("pair\":\"","\",\"",&curLog).toUpper().replace("_","");
					currentHistoryItem.dateTimeInt=getMidData("timestamp\":","}",&curLog).toUInt();
					currentHistoryItem.price=getMidData("rate\":",",\"",&curLog).toDouble();
					currentHistoryItem.volume=getMidData("amount\":",",\"",&curLog).toDouble();
					if(currentHistoryItem.isValid())(*historyItems)<<currentHistoryItem;
				}
			}
			emit historyChanged(historyItems);
		}
		break;//money/wallet/history
		}
	default: break;
	}

	static int errorCount=0;
	if(!success)
	{
		errorCount++;
		if(errorCount<3)return;
		if(debugLevel)logThread->writeLog("API error: "+errorString.toAscii()+" ReqType:"+QByteArray::number(reqType),2);
		if(errorString.isEmpty())return;
		if(errorString==QLatin1String("no orders"))return;
		if(reqType<300)emit showErrorMessage("I:>"+errorString);
	}
	else errorCount=0;
}

void Exchange_BTCe::depthUpdateOrder(double price, double amount, bool isAsk)
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

void Exchange_BTCe::depthSubmitOrder(QMap<double,double> *currentMap ,double priceDouble, double amount, bool isAsk)
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

bool Exchange_BTCe::isReplayPending(int reqType)
{
	if(julyHttp==0)return false;
	return julyHttp->isReqTypePending(reqType);
}

void Exchange_BTCe::secondSlot()
{
	static int infoCounter=0;
	if(lastHistory.isEmpty())getHistory(false);

	if(!isReplayPending(202))sendToApi(202,"",true,baseValues.httpSplitPackets,"method=getInfo&");

	if(!tickerOnly&&!isReplayPending(204))sendToApi(204,"",true,baseValues.httpSplitPackets,"method=ActiveOrders&");
	if(!isReplayPending(103))sendToApi(103,"ticker/"+baseValues.currentPair.currRequestPair,false,baseValues.httpSplitPackets);
	if(!isReplayPending(109))sendToApi(109,"trades/"+baseValues.currentPair.currRequestPair,false,baseValues.httpSplitPackets);
	if(depthEnabled&&(forceDepthLoad||/*infoCounter==3&&*/!isReplayPending(111)))
	{
		emit depthRequested();
		sendToApi(111,"depth/"+baseValues.currentPair.currRequestPair+"?limit="+baseValues.depthCountLimitStr,false,baseValues.httpSplitPackets);
		forceDepthLoad=false;
	}

	if(!baseValues.httpSplitPackets&&julyHttp)julyHttp->prepareDataSend();

	if(++infoCounter>9)
	{
		infoCounter=0;
		quint32 syncNonce=(QDateTime::currentDateTime().toTime_t()-1371854884)*10;
		if(privateNonce<syncNonce)privateNonce=syncNonce;
	}
	Exchange::secondSlot();
}

void Exchange_BTCe::getHistory(bool force)
{
	if(tickerOnly)return;
	if(force)lastHistory.clear();
	if(!isReplayPending(208))sendToApi(208,"",true,baseValues.httpSplitPackets,"method=TradeHistory&");
	if(!isReplayPending(110))sendToApi(110,"info",false,baseValues.httpSplitPackets);
	if(!baseValues.httpSplitPackets&&julyHttp)julyHttp->prepareDataSend();
}

void Exchange_BTCe::buy(double apiBtcToBuy, double apiPriceToBuy)
{
	if(tickerOnly)return;
	QByteArray btcToBuy=QByteArray::number(apiBtcToBuy,'f',8);
	int digitsToCut=baseValues.currentPair.currADecimals;
	int dotPos=btcToBuy.indexOf('.');
	if(dotPos>0)
	{
		int toCut=(btcToBuy.size()-dotPos-1)-digitsToCut;
		if(toCut>0)btcToBuy.remove(btcToBuy.size()-toCut-1,toCut);
	}
	QByteArray data="method=Trade&pair="+baseValues.currentPair.currRequestPair+"&type=buy&rate="+QByteArray::number(apiPriceToBuy,'f',baseValues.currentPair.priceDecimals)+"&amount="+btcToBuy+"&";
	if(debugLevel)logThread->writeLog("Buy: "+data,2);
	sendToApi(306,"",true,true,data);
}

void Exchange_BTCe::sell(double apiBtcToSell, double apiPriceToSell)
{
	if(tickerOnly)return;

	QByteArray btcToSell=QByteArray::number(apiBtcToSell,'f',8);
	int dotPos=btcToSell.indexOf('.');
	if(dotPos>0)
	{
		int toCut=(btcToSell.size()-dotPos-1)-baseValues.currentPair.currADecimals;
		if(toCut>0)btcToSell.remove(btcToSell.size()-toCut-1,toCut);
	}
	QByteArray data="method=Trade&pair="+baseValues.currentPair.currRequestPair+"&type=sell&rate="+QByteArray::number(apiPriceToSell)+"&amount="+btcToSell+"&";
	if(debugLevel)logThread->writeLog("Sell: "+data,2);
	sendToApi(307,"",true,true,data);
}

void Exchange_BTCe::cancelOrder(QByteArray order)
{
	if(tickerOnly)return;

	order.prepend("method=CancelOrder&order_id=");
	order.append("&");
	if(debugLevel)logThread->writeLog("Cancel order: "+order,2);
	sendToApi(305,"",true,true,order);
}

void Exchange_BTCe::sendToApi(int reqType, QByteArray method, bool auth, bool sendNow, QByteArray commands)
{
	if(julyHttp==0)
	{ 
		julyHttp=new JulyHttp("btc-e.com","Key: "+privateRestKey+"\r\n",this);
		connect(julyHttp,SIGNAL(anyDataReceived()),baseValues_->mainWindow_,SLOT(anyDataReceived()));
		connect(julyHttp,SIGNAL(apiDown(bool)),baseValues_->mainWindow_,SLOT(setApiDown(bool)));
		connect(julyHttp,SIGNAL(setDataPending(bool)),baseValues_->mainWindow_,SLOT(setDataPending(bool)));
		connect(julyHttp,SIGNAL(errorSignal(QString)),baseValues_->mainWindow_,SLOT(showErrorMessage(QString)));
		connect(julyHttp,SIGNAL(sslErrorSignal(const QList<QSslError> &)),this,SLOT(sslErrors(const QList<QSslError> &)));
		connect(julyHttp,SIGNAL(dataReceived(QByteArray,int)),this,SLOT(dataReceivedAuth(QByteArray,int)));
	}

	if(auth)
	{
		QByteArray postData=commands+"nonce="+QByteArray::number(++privateNonce);
		if(sendNow)
			julyHttp->sendData(reqType, "POST /tapi/"+method, postData, "Sign: "+hmacSha512(privateRestSign,postData).toHex()+"\r\n");
		else
			julyHttp->prepareData(reqType, "POST /tapi/"+method, postData, "Sign: "+hmacSha512(privateRestSign,postData).toHex()+"\r\n");
	}
	else
	{
		if(commands.isEmpty())
		{
			if(sendNow)
				julyHttp->sendData(reqType, "GET /api/3/"+method);
			else 
				julyHttp->prepareData(reqType, "GET /api/3/"+method);
		}
		else
		{
			if(sendNow)
				julyHttp->sendData(reqType, "POST /api/3/"+method, commands);
			else 
				julyHttp->prepareData(reqType, "POST /api/3/"+method, commands);
		}
	}
}

void Exchange_BTCe::sslErrors(const QList<QSslError> &errors)
{
	QStringList errorList;
	for(int n=0;n<errors.count();n++)errorList<<errors.at(n).errorString();
	if(debugLevel)logThread->writeLog(errorList.join(" ").toAscii(),2);
	emit showErrorMessage("SSL Error: "+errorList.join(" "));
}