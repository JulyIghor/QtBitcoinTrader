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
		connect(mainClass,SIGNAL(reloadOrders()),this,SLOT(reloadOrders()));
		connect(mainClass,SIGNAL(apiBuy(double, double)),this,SLOT(buy(double, double)));
		connect(mainClass,SIGNAL(apiSell(double, double)),this,SLOT(sell(double, double)));
		connect(mainClass,SIGNAL(cancelOrderByOid(QByteArray)),this,SLOT(cancelOrder(QByteArray)));
		connect(this,SIGNAL(ordersChanged(QString)),mainClass,SLOT(ordersChanged(QString)));
		connect(mainClass,SIGNAL(cancelOrderByOid(QByteArray)),this,SLOT(cancelOrder(QByteArray)));
		connect(mainClass,SIGNAL(getHistory(bool)),this,SLOT(getHistory(bool)));
		connect(this,SIGNAL(ordersLogChanged(QString)),mainClass,SLOT(ordersLogChanged(QString)));
		connect(this,SIGNAL(orderCanceled(QByteArray)),mainClass,SLOT(orderCanceled(QByteArray)));
		connect(this,SIGNAL(ordersIsEmpty()),mainClass,SLOT(ordersIsEmpty()));
	}

	connect(this,SIGNAL(depthUpdateOrder(double,double,bool)),mainClass,SLOT(depthUpdateOrder(double,double,bool)));
	connect(this,SIGNAL(showErrorMessage(QString)),mainClass,SLOT(showErrorMessage(QString)));
	connect(this,SIGNAL(accLastSellChanged(QByteArray,double)),mainClass,SLOT(accLastSellChanged(QByteArray,double)));
	connect(this,SIGNAL(accLastBuyChanged(QByteArray,double)),mainClass,SLOT(accLastBuyChanged(QByteArray,double)));

	connect(mainClass,SIGNAL(clearValues()),this,SLOT(clearValues()));
	connect(this,SIGNAL(firstTicker()),mainClass,SLOT(firstTicker()));
	connect(this,SIGNAL(firstAccInfo()),mainClass,SLOT(firstAccInfo()));
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
	lastDepthBidsMap.clear();
	lastDepthAsksMap.clear();
	lastDepthData.clear();
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
	if(b.isEmpty())b="\",";
	int startPos=data->indexOf(a,0);
	if(startPos>-1)
	{
		int endPos=data->indexOf(b,startPos+a.length());
		if(endPos>-1)rez=data->mid(startPos+a.length(),endPos-startPos-a.length());
	}
	return rez;
}

void Exchange_BTCe::dataReceivedAuth(QByteArray data, int reqType)
{
	bool success=data.startsWith("{\"success\":1");
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
				emit addLastTrade(getMidData("\"amount\":",",\"",&tradeData).toDouble(),tradeDate,getMidData("\"price\":",",\"",&tradeData).toDouble(),getMidData("\"price_currency\":\"","\",\"",&tradeData),getMidData("\"trade_type\":\"","\"",&tradeData)=="ask");
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
				if(depthCountLimit)while(asksList.count()>depthCountLimit)asksList.removeLast();
				for(int n=0;n<asksList.count();n++)
				{
					QStringList currentPair=asksList.at(n).split(",");
					if(currentPair.count()!=2)continue;
					double priceDouble=currentPair.first().toDouble();
					double amount=currentPair.last().toDouble();
					if(priceDouble>0.0&&amount>0.0)
					{
						currentAsksMap[priceDouble]=amount;
						if(lastDepthAsksMap.value(priceDouble,0.0)!=amount)emit depthUpdateOrder(priceDouble,amount,true);
					}
				}
				QList<double> currentAsksList=lastDepthAsksMap.keys();
				for(int n=0;n<currentAsksList.count();n++)
					if(currentAsksMap.value(currentAsksList.at(n),0)==0)emit depthUpdateOrder(currentAsksList.at(n),0.0,true);
				lastDepthAsksMap=currentAsksMap;

				QMap<double,double> currentBidsMap;
				QStringList bidsList=QString(getMidData("bids\":[[","]]",&data)).split("],[");
				if(depthCountLimit)while(bidsList.count()>depthCountLimit)bidsList.removeLast();
				for(int n=0;n<bidsList.count();n++)
				{
					QStringList currentPair=bidsList.at(n).split(",");
					if(currentPair.count()!=2)continue;
					double priceDouble=currentPair.first().toDouble();
					double amount=currentPair.last().toDouble();
					if(priceDouble>0.0&&amount>0.0)
					{
						currentBidsMap[priceDouble]=amount;
						if(lastDepthBidsMap.value(priceDouble,0.0)!=amount)emit depthUpdateOrder(priceDouble,amount,false);
					}
					if(priceDouble>0.0&&amount>0.0)emit depthUpdateOrder(priceDouble,amount,false);
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
			QByteArray btcBalance=getMidData(currencyAStrLow+"\":",",\"",&data);
			if(!btcBalance.isEmpty())
			{
				double newBtcBalance=btcBalance.toDouble();
				if(lastBtcBalance!=newBtcBalance)emit accBtcBalanceChanged(newBtcBalance);
				lastBtcBalance=newBtcBalance;
			}

			QByteArray usdBalance=getMidData("\""+currencyBStrLow+"\":",",\"",&data);
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
					emit firstAccInfo();
					isFirstAccInfo=false;
				}
			}
		}
		break;//info
	case 204://orders
		{
		if(data.size()<=30)break;
		bool isEmptyOrders=!success&&errorString=="no orders";if(isEmptyOrders)success=true;
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

			for(int n=0;n<ordersList.count();n++)
			{//itemDate+";"+itemType+";"+itemStatus+";"+itemAmount+";"+itemPrice+";"+orderSign
				QByteArray currentOrder="{"+ordersList.at(n).toAscii()+"}";
				QByteArray itemStatus;
				int itemStatusInt=getMidData("status\":","}",&currentOrder).toInt();
				switch(itemStatusInt)
				{
				case 0: itemStatus="open"; break;
				case 1: itemStatus="PENDING1"; break;
				case 2: itemStatus="INVALID2"; break;
				case 3: itemStatus="CANCELED3"; break;
				default: itemStatus="INVALID4";
				}
				QByteArray tradeType=getMidData("type\":\"","\",\"",&currentOrder);
				if(tradeType=="buy")tradeType="bid";
				else if(tradeType=="sell")tradeType="ask";
				QStringList currencyPair=QString(getMidData("pair\":\"","\",\"",&currentOrder)).toUpper().split("_");
				if(currencyPair.count()!=2)continue;
				rezultData.append(getMidData("{","\":{",&currentOrder)+";");
				rezultData.append(getMidData("timestamp_created\":",",\"",&currentOrder)+";");
				rezultData.append(tradeType+";");
				rezultData.append(itemStatus+";");
				rezultData.append(getMidData("amount\":",",\"",&currentOrder)+";");
				rezultData.append(getMidData("rate\":",",\"",&currentOrder)+";");
				rezultData.append(currencySignMap->value(currencyPair.last().toAscii(),"$")+";");
				rezultData.append(currencySignMap->value(currencyPair.first().toAscii(),"$")+"\n");
			}
			emit ordersChanged(rezultData);
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
		bool isEmptyOrders=!success&&errorString=="no trades";if(isEmptyOrders)success=true;
		if(lastHistory!=data)
		{
			lastHistory=data;
			if(!success)break;
			double lastBuyPrice=0.0;
			double lastSellPrice=0.0;
			QString newLog(data);
			QStringList dataList=newLog.split("\":{\"");
			if(dataList.count())dataList.removeFirst();
			if(dataList.count())dataList.removeFirst();
			if(dataList.count()==0)return;
			newLog.clear();
			for(int n=0;n<dataList.count();n++)
			{
				QByteArray curLog(dataList.at(n).toAscii());
				QByteArray logType=getMidData("type\":\"","\",\"",&curLog);
				int logTypeInt=0;
				if(logType=="sell"){logTypeInt=1;logType="<font color=\"red\">("+julyTr("LOG_SOLD","Sold").toAscii()+")</font>";}
				else 
					if(logType=="buy"){logTypeInt=2;logType="<font color=\"blue\">("+julyTr("LOG_BOUGHT","Bought").toAscii()+")</font>";}
					if(logTypeInt)
					{
						QByteArray logValue=getMidData("amount\":",",\"",&curLog);
						QByteArray logDate=getMidData("timestamp\":","}",&curLog);
						QByteArray priceValue=getMidData("rate\":",",\"",&curLog);
						QStringList currencyPair;
						if((logTypeInt==1||logTypeInt==2)&&(lastSellPrice==0.0||lastBuyPrice==0.0))
						{
							currencyPair=QString(getMidData("pair\":\"","\",\"",&curLog)).toUpper().split("_");
							if(currencyPair.count()!=2||currencyPair.first().isEmpty()||currencyPair.last().isEmpty())continue;
							if(lastSellPrice==0.0&&logTypeInt==1)
							{
								lastSellPrice=priceValue.toDouble();
								emit accLastSellChanged(currencyPair.last().toAscii(),lastSellPrice);
							}
							if(lastBuyPrice==0.0&&logTypeInt==2)
							{
								lastBuyPrice=priceValue.toDouble();
								emit accLastBuyChanged(currencyPair.last().toAscii(),lastBuyPrice);
							}
						}
						if(currencyPair.count()!=2)continue;
						QString priceText="<font color=\"darkgreen\">"+currencySignMap->value(currencyPair.last().toAscii(),"USD")+" "+priceValue+"</font>";
						newLog.append("<font color=\"gray\">"+QDateTime::fromTime_t(logDate.toUInt()).toString(localDateTimeFormat).toAscii()+"</font>&nbsp;");
						newLog.append("<font color=\"#996515\">");
						newLog.append(currencySignMap->value(currencyPair.first().toAscii(),"BTC"));
						newLog.append(" "+logValue+"</font> "+julyTr("AT"," at %1").arg(priceText).toAscii()+" "+logType+"<br>");
					}
			}
			emit ordersLogChanged(newLog);
		}
		break;//money/wallet/history
		}
	default: break;
	}

	if(!success)
	{
		if(isLogEnabled)logThread->writeLog("API error: "+errorString.toAscii());
		if(errorString.isEmpty())return;
		if(errorString=="no orders")return;
		if(!errorString.contains("nonce"))//Temporary disabled this
		if(reqType<300)emit showErrorMessage("I:>"+errorString);
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

void Exchange_BTCe::reloadOrders()
{
	lastOrders.clear();
}

void Exchange_BTCe::secondSlot()
{
	static int infoCounter=0;
	if(lastHistory.isEmpty())getHistory(false);

	if(!isReplayPending(202))sendToApi(202,"",true,httpSplitPackets,"method=getInfo&");

	if(!tickerOnly&&!isReplayPending(204))sendToApi(204,"",true,httpSplitPackets,"method=OrderList&");
	
	if(!isReplayPending(103))sendToApi(103,currencyRequestPair+"/ticker",false,httpSplitPackets);
	if(!isReplayPending(109))sendToApi(109,currencyRequestPair+"/trades",false,httpSplitPackets);
	if(infoCounter==3&&!isReplayPending(111))sendToApi(111,currencyRequestPair+"/depth",false,httpSplitPackets);

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

		bool isVipRequest=reqType>300;
		if(sendNow)
			julyHttp->sendData(reqType, isVipRequest, "POST /tapi/"+method, (isVipRequest?300:-1), postData, "Sign: "+hmacSha512(privateRestSign,postData).toHex()+"\r\n");
		else
			julyHttp->prepareData(reqType, "POST /tapi/"+method, postData, "Sign: "+hmacSha512(privateRestSign,postData).toHex()+"\r\n");
	}
	else
	{
		if(commands.isEmpty())
		{
			if(sendNow)
				julyHttp->sendData(reqType, false, "GET /api/2/"+method);
			else 
				julyHttp->prepareData(reqType, "GET /api/2/"+method);
		}
		else
		{
			if(sendNow)
				julyHttp->sendData(reqType, false, "POST /api/2/"+method, -1, commands);
			else 
				julyHttp->prepareData(reqType, "POST /api/2/"+method,commands);
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