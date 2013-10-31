// Copyright (C) 2013 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form https://sourceforge.net/projects/bitcointrader/
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#include "exchange_btce.h"
#include <openssl/hmac.h>
#include "main.h"
#include <QDateTime>

Exchange_BTCe::Exchange_BTCe(QByteArray pRestSign, QByteArray pRestKey)
	: QThread()
{
	forceDepthLoad=false;
	lastPriceDate=0;
	julyHttp=0;
	isApiDown=false;
	tickerOnly=false;
	privateRestSign=pRestSign;
	privateRestKey=pRestKey;
	moveToThread(this);

	authRequestTime.restart();
	privateNonce=(QDateTime::currentDateTime().toTime_t()-1371854884)*10;
}

Exchange_BTCe::~Exchange_BTCe()
{
	if(isLogEnabled)logThread->writeLog("BTC-E API Thread Deleted");
}

void Exchange_BTCe::setupApi(QtBitcoinTrader *mainClass, bool tickOnly)
{
	tickerOnly=tickOnly;
	if(!tickerOnly)
	{
		connect(mainClass,SIGNAL(apiBuy(double, double)),this,SLOT(buy(double, double)));
		connect(mainClass,SIGNAL(apiSell(double, double)),this,SLOT(sell(double, double)));
		connect(mainClass,SIGNAL(cancelOrderByOid(QByteArray)),this,SLOT(cancelOrder(QByteArray)));
		connect(this,SIGNAL(ordersChanged(QList<OrderItem> *)),mainClass,SLOT(ordersChanged(QList<OrderItem> *)));
		connect(mainClass,SIGNAL(cancelOrderByOid(QByteArray)),this,SLOT(cancelOrder(QByteArray)));
		connect(mainClass,SIGNAL(getHistory(bool)),this,SLOT(getHistory(bool)));
		connect(this,SIGNAL(historyChanged(QList<HistoryItem>*)),mainClass,SLOT(historyChanged(QList<HistoryItem>*)));
		connect(this,SIGNAL(orderCanceled(QByteArray)),mainClass,SLOT(orderCanceled(QByteArray)));
		connect(this,SIGNAL(ordersIsEmpty()),mainClass,SLOT(ordersIsEmpty()));
	}

	connect(this,SIGNAL(depthFirstOrder(double,double,bool)),mainClass,SLOT(depthFirstOrder(double,double,bool)));
	connect(this,SIGNAL(depthUpdateOrder(double,double,bool)),mainClass,SLOT(depthUpdateOrder(double,double,bool)));
	connect(this,SIGNAL(showErrorMessage(QString)),mainClass,SLOT(showErrorMessage(QString)));

	connect(mainClass,SIGNAL(clearValues()),this,SLOT(clearValues()));
	connect(mainClass,SIGNAL(reloadDepth()),this,SLOT(reloadDepth()));
	connect(this,SIGNAL(firstTicker()),mainClass,SLOT(firstTicker()));
	connect(this,SIGNAL(apiLagChanged(double)),mainClass->ui.lagValue,SLOT(setValue(double)));
	connect(this,SIGNAL(accFeeChanged(double)),mainClass->ui.accountFee,SLOT(setValue(double)));
	connect(this,SIGNAL(accBtcBalanceChanged(double)),mainClass->ui.accountBTC,SLOT(setValue(double)));
	connect(this,SIGNAL(accUsdBalanceChanged(double)),mainClass->ui.accountUSD,SLOT(setValue(double)));

	connect(this,SIGNAL(tickerHighChanged(double)),mainClass->ui.marketHigh,SLOT(setValue(double)));
	connect(this,SIGNAL(tickerLowChanged(double)),mainClass->ui.marketLow,SLOT(setValue(double)));
	connect(this,SIGNAL(tickerSellChanged(double)),mainClass->ui.marketSell,SLOT(setValue(double)));
	connect(this,SIGNAL(tickerLastChanged(double)),mainClass->ui.marketLast,SLOT(setValue(double)));
	connect(this,SIGNAL(tickerBuyChanged(double)),mainClass->ui.marketBuy,SLOT(setValue(double)));
	connect(this,SIGNAL(tickerVolumeChanged(double)),mainClass->ui.marketVolume,SLOT(setValue(double)));

	connect(this,SIGNAL(addLastTrade(double, qint64, double, QByteArray, bool)),mainClass,SLOT(addLastTrade(double, qint64, double, QByteArray, bool)));

	start();
}

void Exchange_BTCe::clearVariables()
{
	isFirstTicker=true;
	isFirstAccInfo=true;
	lastTickerHigh=0.0;
	lastTickerLow=0.0;
	lastTickerSell=0.0;
	lastTickerLast=0.0;
	lastTickerBuy=0.0;
	lastTickerVolume=0.0;
	lastBtcBalance=0.0;
	lastUsdBalance=0.0;
	lastVolume=0.0;
	lastOpenedOrders=-1;
	lastFee=0.0;
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

QByteArray Exchange_BTCe::getMidData(QString a, QString b,QByteArray *data)
{
	QByteArray rez;
	if(b.isEmpty())b=QLatin1String("\",");
	int startPos=data->indexOf(a,0);
	if(startPos>-1)
	{
		int endPos=data->indexOf(b,startPos+a.length());
		if(endPos>-1)rez=data->mid(startPos+a.length(),endPos-startPos-a.length());
	}
	return rez;
}

void Exchange_BTCe::reloadDepth()
{
	lastDepthBidsMap.clear();
	lastDepthAsksMap.clear();
	lastDepthData.clear();
	forceDepthLoad=true;
}

void Exchange_BTCe::dataReceivedAuth(QByteArray data, int reqType)
{
	bool success=!data.startsWith("{\"success\":0");
	QString errorString;
	if(!success)errorString=getMidData("error\":\"","\"",&data);

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

			QByteArray tickerUpdated=getMidData("\"updated\":",",\"",&data);
			if(!tickerUpdated.isEmpty())
			{
				qint64 newTickerUpdated=tickerUpdated.toLongLong();
				if(newTickerUpdated>lastFetchTid)
				{
					QByteArray tickerLast=getMidData("\"last\":",",\"",&data);
					if(!tickerLast.isEmpty())
					{
						double newTickerLast=tickerLast.toDouble();
						if(newTickerLast!=lastTickerLast)emit tickerLastChanged(newTickerLast);
						lastTickerLast=newTickerLast;
					}
				}
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
			QStringList tradeList=QString(data).split("},{");
			for(int n=tradeList.count()-1;n>=0;n--)
			{
				QByteArray tradeData=tradeList.at(n).toAscii();
				qint64 tradeDate=getMidData("date\":",",\"",&tradeData).toLongLong();
				if(lastFetchTid<0&&tradeDate<-lastFetchTid)continue;
				qint64 currentTid=getMidData("\"tid\":",",\"",&tradeData).toLongLong();
				if(currentTid<1000||lastFetchTid>=currentTid)continue;
				lastFetchTid=currentTid;
				if(tradeDate>lastPriceDate)lastPriceDate=tradeDate;
				emit addLastTrade(getMidData("\"amount\":",",\"",&tradeData).toDouble(),tradeDate,getMidData("\"price\":",",\"",&tradeData).toDouble(),getMidData("\"item\":\"","\",\"",&tradeData)+getMidData("\"price_currency\":\"","\",\"",&tradeData),getMidData("\"trade_type\":\"","\"",&tradeData)=="ask");
			}
		}
		break;//trades
	case 110: //Fee
		{
			QByteArray tradeFee=getMidData("trade\":","}",&data);
			if(!tradeFee.isEmpty())
			{
				double newFee=tradeFee.toDouble();
				if(newFee!=lastFee)emit accFeeChanged(newFee);
				lastFee=newFee;
			}
		}
		break;// Fee
	case 111: //depth
		if(data.startsWith("{\"asks\":["))
		{
			emit depthUpdateOrder(0.0,0.0,true);
			if(lastDepthData!=data)
			{
				lastDepthData=data;
				QMap<double,double> currentAsksMap;
				QStringList asksList=QString(getMidData("asks\":[[","]]",&data)).split("],[");
				double groupedPrice=0.0;
				double groupedVolume=0.0;
				int rowCounter=0;

				for(int n=0;n<asksList.count();n++)
				{
					if(depthCountLimit&&rowCounter>=depthCountLimit)break;
					QStringList currentPair=asksList.at(n).split(",");
					if(currentPair.count()!=2)continue;
					double priceDouble=currentPair.first().toDouble();
					double amount=currentPair.last().toDouble();

					if(groupPriceValue>0.0)
					{
						if(n==0)
						{
							emit depthFirstOrder(priceDouble,amount,true);
							groupedPrice=groupPriceValue*(int)(priceDouble/groupPriceValue);
							groupedVolume=amount;
						}
						else
						{
							bool matchCurrentGroup=priceDouble<groupedPrice+groupPriceValue;
							if(matchCurrentGroup)groupedVolume+=amount;
							if(!matchCurrentGroup||n==asksList.count()-1)
							{
								depthSubmitOrder(&currentAsksMap,groupedPrice+groupPriceValue,groupedVolume,true);
								rowCounter++;
								groupedVolume=amount;
								groupedPrice+=groupPriceValue;
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
					if(currentAsksMap.value(currentAsksList.at(n),0)==0)emit depthUpdateOrder(currentAsksList.at(n),0.0,true);
				lastDepthAsksMap=currentAsksMap;

				QMap<double,double> currentBidsMap;
				QStringList bidsList=QString(getMidData("bids\":[[","]]",&data)).split("],[");
				groupedPrice=0.0;
				groupedVolume=0.0;
				rowCounter=0;

				for(int n=0;n<bidsList.count();n++)
				{
					if(depthCountLimit&&rowCounter>=depthCountLimit)break;
					QStringList currentPair=bidsList.at(n).split(",");
					if(currentPair.count()!=2)continue;
					double priceDouble=currentPair.first().toDouble();
					double amount=currentPair.last().toDouble();
					if(groupPriceValue>0.0)
					{
						if(n==0)
						{
							emit depthFirstOrder(priceDouble,amount,false);
							groupedPrice=groupPriceValue*(int)(priceDouble/groupPriceValue);
							groupedVolume=amount;
						}
						else
						{
							bool matchCurrentGroup=priceDouble>groupedPrice-groupPriceValue;
							if(matchCurrentGroup)groupedVolume+=amount;
							if(!matchCurrentGroup||n==0)
							{
								depthSubmitOrder(&currentBidsMap,groupedPrice-groupPriceValue,groupedVolume,false);
								rowCounter++;
								groupedVolume=amount;
								groupedPrice-=groupPriceValue;
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
					if(currentBidsMap.value(currentBidsList.at(n),0)==0)emit depthUpdateOrder(currentBidsList.at(n),0.0,false);
				lastDepthBidsMap=currentBidsMap;
			}
		}
		else if(isLogEnabled)logThread->writeLog("Invalid depth data:"+data);
		break;
	case 202: //info
		{
			if(!success)break;
			QByteArray fundsData=getMidData("funds\":{","}",&data)+",";
			QByteArray btcBalance=getMidData(currencyAStrLow+"\":",",",&fundsData);
			if(!btcBalance.isEmpty())
			{
				double newBtcBalance=btcBalance.toDouble();
				if(lastBtcBalance!=newBtcBalance)emit accBtcBalanceChanged(newBtcBalance);
				lastBtcBalance=newBtcBalance;
			}

			QByteArray usdBalance=getMidData("\""+currencyBStrLow+"\":",",",&fundsData);
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
	case 306: if(isLogEnabled)logThread->writeLog("Buy OK: "+data);break;//order/buy
	case 307: if(isLogEnabled)logThread->writeLog("Sell OK: "+data);break;//order/sell
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
				currentHistoryItem.type=0;
				currentHistoryItem.price=0.0;
				currentHistoryItem.volume=0.0;
				currentHistoryItem.date=0;

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
					currentHistoryItem.date=getMidData("timestamp\":","}",&curLog).toUInt();
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
		if(isLogEnabled)logThread->writeLog("API error: "+errorString.toAscii());
		if(errorString.isEmpty())return;
		if(errorString==QLatin1String("no orders"))return;
		if(reqType<300)emit showErrorMessage("I:>"+errorString);
	}
	else errorCount=0;
}

void Exchange_BTCe::depthSubmitOrder(QMap<double,double> *currentMap ,double priceDouble, double amount, bool isAsk)
{
	if(priceDouble==0.0||amount==0.0)return;

	if(isAsk)
	{
		(*currentMap)[priceDouble]=amount;
		if(lastDepthAsksMap.value(priceDouble,0.0)!=amount)
			emit depthUpdateOrder(priceDouble,amount,true);
	}
	else
	{
		(*currentMap)[priceDouble]=amount;
		if(lastDepthBidsMap.value(priceDouble,0.0)!=amount)
			emit depthUpdateOrder(priceDouble,amount,false);
	}
}

void Exchange_BTCe::run()
{
	if(isLogEnabled)logThread->writeLog("BTC-e API Thread Started");

	clearVariables();

	secondTimer=new QTimer;
	secondTimer->setSingleShot(true);
	connect(secondTimer,SIGNAL(timeout()),this,SLOT(secondSlot()));
	secondSlot();
	exec();
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

	if(!isReplayPending(202))sendToApi(202,"",true,httpSplitPackets,"method=getInfo&");

	if(!tickerOnly&&!isReplayPending(204))sendToApi(204,"",true,httpSplitPackets,"method=OrderList&");
	
	if(!isReplayPending(103))sendToApi(103,currencyRequestPair+"/ticker",false,httpSplitPackets);
	if(!isReplayPending(109))sendToApi(109,currencyRequestPair+"/trades",false,httpSplitPackets);
	if(!depthRefreshBlocked&&(forceDepthLoad||infoCounter==3&&!isReplayPending(111)))
	{
		sendToApi(111,currencyRequestPair+"/depth",false,httpSplitPackets);
		forceDepthLoad=false;
	}

	if(!httpSplitPackets&&julyHttp)julyHttp->prepareDataSend();

	if(++infoCounter>9)
	{
		infoCounter=0;
		quint32 syncNonce=(QDateTime::currentDateTime().toTime_t()-1371854884)*10;
		if(privateNonce<syncNonce)privateNonce=syncNonce;
	}
	secondTimer->start(httpRequestInterval);
}

void Exchange_BTCe::getHistory(bool force)
{
	if(tickerOnly)return;
	if(force)lastHistory.clear();
	if(!isReplayPending(208))sendToApi(208,"",true,httpSplitPackets,"method=TradeHistory&");
	if(!isReplayPending(110))sendToApi(110,currencyRequestPair+"/fee",false,httpSplitPackets);
	if(!httpSplitPackets&&julyHttp)julyHttp->prepareDataSend();
}

void Exchange_BTCe::buy(double apiBtcToBuy, double apiPriceToBuy)
{
	if(tickerOnly)return;
	QByteArray btcToBuy=QByteArray::number(apiBtcToBuy,'f',8);
	int digitsToCut=btcDecimals;
	if(digitsToCut>6)digitsToCut--;
	int dotPos=btcToBuy.indexOf('.');
	if(dotPos>0)
	{
		int toCut=(btcToBuy.size()-dotPos-1)-digitsToCut;
		if(toCut>0)btcToBuy.remove(btcToBuy.size()-toCut-1,toCut);
	}
	sendToApi(306,"",true,true,"method=Trade&pair="+currencyRequestPair+"&type=buy&rate="+QByteArray::number(apiPriceToBuy)+"&amount="+btcToBuy+"&");
	secondSlot();
}

void Exchange_BTCe::sell(double apiBtcToSell, double apiPriceToSell)
{
	if(tickerOnly)return;

	QByteArray btcToSell=QByteArray::number(apiBtcToSell,'f',8);
	int dotPos=btcToSell.indexOf('.');
	if(dotPos>0)
	{
		int toCut=(btcToSell.size()-dotPos-1)-btcDecimals;
		if(toCut>0)btcToSell.remove(btcToSell.size()-toCut-1,toCut);
	}
	sendToApi(307,"",true,true,"method=Trade&pair="+currencyRequestPair+"&type=sell&rate="+QByteArray::number(apiPriceToSell)+"&amount="+btcToSell+"&");
}

void Exchange_BTCe::cancelOrder(QByteArray order)
{
	if(tickerOnly)return;

	order.prepend("method=CancelOrder&order_id=");
	order.append("&");
	sendToApi(305,"",true,true,order);
}

void Exchange_BTCe::sendToApi(int reqType, QByteArray method, bool auth, bool sendNow, QByteArray commands)
{
	if(julyHttp==0)
	{ 
		julyHttp=new JulyHttp("btc-e.com","Key: "+privateRestKey+"\r\n",this);
		connect(julyHttp,SIGNAL(anyDataReceived()),mainWindow_,SLOT(anyDataReceived()));
		connect(julyHttp,SIGNAL(apiDown(bool)),mainWindow_,SLOT(setApiDown(bool)));
		connect(julyHttp,SIGNAL(setDataPending(bool)),mainWindow_,SLOT(setDataPending(bool)));
		connect(julyHttp,SIGNAL(errorSignal(QString)),mainWindow_,SLOT(showErrorMessage(QString)));
		connect(julyHttp,SIGNAL(sslErrorSignal(const QList<QSslError> &)),this,SLOT(sslErrors(const QList<QSslError> &)));
		connect(julyHttp,SIGNAL(dataReceived(QByteArray,int)),this,SLOT(dataReceivedAuth(QByteArray,int)));
	}

	if(auth)
	{
		QByteArray postData=commands+"nonce="+QByteArray::number(++privateNonce);
		if(isLogEnabled)logThread->writeLog("/tapi"+method+"?"+postData);

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
				julyHttp->sendData(reqType, "GET /api/2/"+method);
			else 
				julyHttp->prepareData(reqType, "GET /api/2/"+method);
		}
		else
		{
			if(sendNow)
				julyHttp->sendData(reqType, "POST /api/2/"+method, commands);
			else 
				julyHttp->prepareData(reqType, "POST /api/2/"+method, commands);
		}
	}
}

void Exchange_BTCe::sslErrors(const QList<QSslError> &errors)
{
	QStringList errorList;
	for(int n=0;n<errors.count();n++)errorList<<errors.at(n).errorString();
	if(isLogEnabled)logThread->writeLog(errorList.join(" ").toAscii());
	emit showErrorMessage("SSL Error: "+errorList.join(" "));
}