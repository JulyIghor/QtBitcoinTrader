// Copyright (C) 2013 July IGHOR.
// I want to create Bitcoin Trader application that can be configured for any rule and strategy.
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
#include <QSslError>

Exchange_BTCe::Exchange_BTCe(QByteArray pRestSign, QByteArray pRestKey)
	: QThread()
{
	sslEnabled=true;
	tickerOnly=false;
	vipRequestCount=0;
	privateRestSign=pRestSign;
	headerNoAuth.setValue("Host","btc-e.com");
	headerNoAuth.setValue("User-Agent","Qt Bitcoin Trader v"+appVerStr);
	headerNoAuth.setContentType("application/x-www-form-urlencoded");
	headerAuth=headerNoAuth;
	headerAuth.setValue("Key",pRestKey);
	moveToThread(this);
	softLagTime.restart();
	privateNonce=(QDateTime::currentDateTime().toTime_t()-1371854884)*10;
}

Exchange_BTCe::~Exchange_BTCe()
{
	if(isLogEnabled)logThread->writeLog("BTC-E API Thread Deleted");
}

void Exchange_BTCe::setupApi(QtBitcoinTrader *mainClass, bool tickOnly, bool sslEn)
{
	sslEnabled=sslEn;
	tickerOnly=tickOnly;
	if(!tickerOnly)
	{
		connect(mainClass,SIGNAL(setSslEnabled(bool)),this,SLOT(setSslEnabled(bool)));
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

	connect(this,SIGNAL(identificationRequired()),mainClass,SLOT(identificationRequired()));
	connect(this,SIGNAL(apiDownChanged(bool)),mainClass,SLOT(setApiDown(bool)));
	connect(this,SIGNAL(accLastSellChanged(QByteArray,double)),mainClass,SLOT(accLastSellChanged(QByteArray,double)));
	connect(this,SIGNAL(accLastBuyChanged(QByteArray,double)),mainClass,SLOT(accLastBuyChanged(QByteArray,double)));

	connect(mainClass,SIGNAL(clearValues()),this,SLOT(clearValues()));
	connect(this,SIGNAL(firstTicker()),mainClass,SLOT(firstTicker()));
	connect(this,SIGNAL(firstAccInfo()),mainClass,SLOT(firstAccInfo()));
	connect(this,SIGNAL(apiLagChanged(double)),mainClass->ui.lagValue,SLOT(setValue(double)));
	connect(this,SIGNAL(softLagChanged(double)),mainClass->ui.lastUpdate,SLOT(setValue(double)));
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

void Exchange_BTCe::setSslEnabled(bool on)
{
	sslEnabled=on;
	clearValues();
	requestIdsNoAuth.clear();
	if(httpNoAuth->hasPendingRequests())httpNoAuth->clearPendingRequests();
	if(httpAuth->hasPendingRequests())httpAuth->clearPendingRequests();
	if(sslEnabled)
	{
		httpAuth->setHost("btc-e.com",QHttp::ConnectionModeHttps);
		httpNoAuth->setHost("btc-e.com",QHttp::ConnectionModeHttps);
	}
	else
	{
		httpAuth->setHost("btc-e.com",QHttp::ConnectionModeHttp);
		httpNoAuth->setHost("btc-e.com",QHttp::ConnectionModeHttp);
	}
}

void Exchange_BTCe::clearValues()
{
	isApiDown=false;
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
	secondPart=0;
	apiDownCounter=0;
	lastHistory.clear();
	lastOrders.clear();
	requestIdsAuth.clear();
	requestIdsNoAuth.clear();
	if(httpAuth->hasPendingRequests())httpAuth->clearPendingRequests();
	if(httpNoAuth->hasPendingRequests())httpNoAuth->clearPendingRequests();
	lastFetchTid=QDateTime::currentDateTime().addSecs(-600).toTime_t();
	lastFetchTid=-lastFetchTid;
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

void Exchange_BTCe::httpDoneNoAuth(int cId, bool error)
{
	int reqType=requestIdsNoAuth.value(cId,0);
	if(reqType>0)requestIdsNoAuth.remove(cId);else return;
	if(error)return;

	QByteArray data=httpNoAuth->readAll();

	{
		bool lastApiDown=isApiDown;
		bool isUnknownRequest=data.size()==0||data.at(0)!='{';
		if(isUnknownRequest)
		{
			if(++apiDownCounter>3||softLagTime.elapsed()>2000)isApiDown=true;
		}
		else {apiDownCounter=0;isApiDown=false;}
		if(lastApiDown!=isApiDown)emit apiDownChanged(isApiDown);
		if(isUnknownRequest)return;
	}

	emit softLagChanged(softLagTime.elapsed()/1000.0);
	softLagTime.restart();

	switch(reqType)
	{
	case 3: //ticker
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

			QByteArray tickerLast=getMidData("\"last\":",",\"",&data);
			if(!tickerLast.isEmpty())
			{
				double newTickerLast=tickerLast.toDouble();
				if(newTickerLast!=lastTickerLast)emit tickerLastChanged(newTickerLast);
				lastTickerLast=newTickerLast;
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
			if(isFirstTicker)
			{
				emit firstTicker();
				isFirstTicker=false;
			}
		}
		break;//ticker
	case 9: //trades
		{
			if(data.size()<10)break;
			QStringList tradeList=QString(data).split("},{");
			for(int n=tradeList.count()-1;n>=0;n--)
			{
				QByteArray tradeData=tradeList.at(n).toAscii();
				if(lastFetchTid<0&&getMidData("date\":",",\"",&tradeData).toLongLong()<-lastFetchTid)continue;
				qint64 currentTid=getMidData("\"tid\":",",\"",&tradeData).toLongLong();
				if(lastFetchTid>=currentTid)continue;
				lastFetchTid=currentTid;
				emit addLastTrade(getMidData("\"amount\":",",\"",&tradeData).toDouble(),getMidData("date\":",",\"",&tradeData).toLongLong(),getMidData("\"price\":",",\"",&tradeData).toDouble(),getMidData("\"price_currency\":\"","\",\"",&tradeData),getMidData("\"trade_type\":\"","\"",&tradeData)=="ask");
			}
		}
		break;//trades
	case 10: //Fee
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
	}
}

void Exchange_BTCe::httpDoneAuth(int cId, bool error)
{
	int reqType=requestIdsAuth.value(cId,0);
	if(vipRequestCount&&(reqType>=5&&reqType<=7))vipRequestCount--;
	if(reqType>0)requestIdsAuth.remove(cId);else return;
	if(error)return;

	QByteArray data=httpAuth->readAll();

	{
		bool lastApiDown=isApiDown;
		bool isUnknownRequest=data.size()==0||data.at(0)!='{';
		if(isUnknownRequest)
		{
			if(++apiDownCounter>3||softLagTime.elapsed()>2000)isApiDown=true;
		}
		else {apiDownCounter=0;isApiDown=false;}
		if(lastApiDown!=isApiDown)emit apiDownChanged(isApiDown);
		if(isUnknownRequest)return;
	}

	emit softLagChanged(softLagTime.elapsed()/1000.0);
	softLagTime.restart();

	bool success=getMidData("success\":",",",&data)=="1";
	
		switch(reqType)
		{
		case 2: //info
			{
				if(!success)return;
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
					if(!isRightsGood)emit identificationRequired();
					emit firstAccInfo();
					isFirstAccInfo=false;
					}
				}
			}
			break;//info
		case 4://orders
			if(!success)return;
			if(data.size()<=30)break;
			if(lastOrders!=data)
			{
				lastOrders=data;
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
					rezultData.append(currencySignMap->value(currencyPair.last().toAscii(),"$")+"\n");
				}
				emit ordersChanged(rezultData);
			}
			break;//orders
		case 5: //order/cancel
			if(success)
			{
				QByteArray oid=getMidData("order_id\":",",\"",&data);
				if(!oid.isEmpty())emit orderCanceled(oid);
			}
			break;//order/cancel
		case 6: if(isLogEnabled)logThread->writeLog("Buy OK: "+data);break;//order/buy
		case 7: if(isLogEnabled)logThread->writeLog("Sell OK: "+data);break;//order/sell
		case 8: ///history
			if(!success)return;
			if(lastHistory!=data)
			{
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
				lastHistory=data;
			}
			break;//money/wallet/history
		default: break;
		}
	
	if(!success)
	{
		QString errorString=getMidData("error\":\"","\"",&data);
		if(isLogEnabled)logThread->writeLog("API error: "+errorString.toAscii());
		if(errorString.isEmpty())return;
		if(errorString=="invalid sign")emit identificationRequired();
	}
}

void Exchange_BTCe::run()
{
	if(isLogEnabled)logThread->writeLog("BTC-e API Thread Started");
	if(sslEnabled)
	{
		httpAuth=new QHttp("btc-e.com",QHttp::ConnectionModeHttps);
		httpNoAuth=new QHttp("btc-e.com",QHttp::ConnectionModeHttps);
	}
	else
	{
		httpAuth=new QHttp("btc-e.com",QHttp::ConnectionModeHttp);
		httpNoAuth=new QHttp("btc-e.com",QHttp::ConnectionModeHttp);
	}

	clearValues();

	secondTimer=new QTimer;
	connect(httpAuth,SIGNAL(requestFinished(int,bool)),this,SLOT(httpDoneAuth(int,bool)));
	connect(httpNoAuth,SIGNAL(requestFinished(int,bool)),this,SLOT(httpDoneNoAuth(int,bool)));
	connect(httpAuth,SIGNAL(sslErrors(const QList<QSslError> &)),this,SLOT(sslErrors(const QList<QSslError> &)));
	connect(httpNoAuth,SIGNAL(sslErrors(const QList<QSslError> &)),this,SLOT(sslErrors(const QList<QSslError> &)));

	connect(secondTimer,SIGNAL(timeout()),this,SLOT(secondSlot()));
	secondTimer->start(500);
	exec();
}

void Exchange_BTCe::cancelPendingAuthRequests()
{
	if(vipRequestCount)return;
	if(httpAuth->hasPendingRequests())httpAuth->clearPendingRequests();
	requestIdsAuth.clear();
}

void Exchange_BTCe::cancelPendingNoAuthRequests()
{
	if(httpNoAuth->hasPendingRequests())httpNoAuth->clearPendingRequests();
	requestIdsNoAuth.clear();
}

void Exchange_BTCe::reloadOrders()
{
	lastOrders.clear();
}

void Exchange_BTCe::secondSlot()
{
	emit softLagChanged(softLagTime.elapsed()/1000.0);

	if(requestIdsAuth.count()<10&&!vipRequestCount)//Max pending requests at time
	{
		if(requestIdsAuth.key(2,0)==0)requestIdsAuth[sendToApi("",true,"method=getInfo&")]=2;
		if(!tickerOnly&&requestIdsAuth.key(4,0)==0)requestIdsAuth[sendToApi("",true,"method=OrderList&")]=4;
	} else cancelPendingAuthRequests();

	if(requestIdsNoAuth.count()<10)//Max pending requests at time
	{
		if(requestIdsNoAuth.key(3,0)==0)requestIdsNoAuth[sendToApi(currencyRequestPair+"/ticker",false)]=3;
		static int tradesCounter=0;
		if(tradesCounter++==0)
			if(requestIdsNoAuth.key(9,0)==0)requestIdsNoAuth[sendToApi(currencyRequestPair+"/trades",false)]=9;
		if(tradesCounter>3)tradesCounter=0;
	} else cancelPendingNoAuthRequests();

	if(lastHistory.isEmpty())getHistory(false);
}

void Exchange_BTCe::getHistory(bool force)
{
	if(tickerOnly)return;
	if(force)lastHistory.clear();
	if(requestIdsNoAuth.key(8,0)==0)requestIdsAuth[sendToApi("",true,"method=TradeHistory&count=30&")]=8;
	if(requestIdsNoAuth.key(10,0)==0)requestIdsNoAuth[sendToApi(currencyRequestPair+"/fee",false)]=10;
}

void Exchange_BTCe::buy(double apiBtcToBuy, double apiPriceToBuy)
{
	if(tickerOnly)return;
	cancelPendingAuthRequests();
	vipRequestCount++;
	apiBtcToBuy=((int)(apiBtcToBuy*100000))/100000.0;
	QByteArray btcToBuy=QByteArray::number(apiBtcToBuy);
	QByteArray params="method=Trade&pair="+currencyRequestPair+"&type=buy&rate="+QByteArray::number(apiPriceToBuy)+"&amount="+btcToBuy+"&";
	requestIdsAuth[sendToApi("",true,params)]=6;
	requestIdsAuth[sendToApi("",true,params,false)]=6;
	requestIdsAuth[sendToApi("",true,params,false)]=6;
	secondPart=3;
	secondSlot();
}

void Exchange_BTCe::sell(double apiBtcToSell, double apiPriceToSell)
{
	if(tickerOnly)return;
	cancelPendingAuthRequests();
	vipRequestCount++;
	apiBtcToSell=((int)(apiBtcToSell*100000))/100000.0;
	QByteArray params="method=Trade&pair="+currencyRequestPair+"&type=sell&rate="+QByteArray::number(apiPriceToSell)+"&amount="+QByteArray::number(apiBtcToSell)+"&";
	requestIdsAuth[sendToApi("",true,params)]=7;
	requestIdsAuth[sendToApi("",true,params,false)]=7;
	requestIdsAuth[sendToApi("",true,params,false)]=7;
	secondPart=3;
	secondSlot();
}

void Exchange_BTCe::cancelOrder(QByteArray order)
{
	if(tickerOnly)return;
	cancelPendingAuthRequests();
	vipRequestCount++;
	order.prepend("method=CancelOrder&order_id=");
	order.append("&");
	requestIdsAuth[sendToApi("",true,order)]=5;
	requestIdsAuth[sendToApi("",true,order,false)]=5;
	requestIdsAuth[sendToApi("",true,order,false)]=5;
	secondPart=3;
	secondSlot();
}

int Exchange_BTCe::sendToApi(QByteArray method, bool auth, QByteArray commands, bool incNonce)
{
	if(auth)
	{
		if(incNonce)privateNonce++;
		QByteArray postData=QString(commands+"nonce="+QByteArray::number(privateNonce)).toUtf8();
		headerAuth.setRequest("POST","/tapi"+method);
		headerAuth.setValue("Sign",hmacSha512(privateRestSign,postData).toHex());
		headerAuth.setContentLength(postData.size());
		if(isLogEnabled)logThread->writeLog("/tapi?"+method+"?"+postData);
		return httpAuth->request(headerAuth,postData);
	}
	else
	{
		if(commands.isEmpty())
		{
			headerNoAuth.setRequest("GET","/api/2/"+method);
			return httpNoAuth->request(headerNoAuth);
		}
		headerNoAuth.setRequest("POST","/api/2/"+method);
		return httpNoAuth->request(headerNoAuth,commands);
	}
	return 0;
}

void Exchange_BTCe::sslErrors(const QList<QSslError> &errors)
{
	if(!isLogEnabled)return;
	for(int n=0;n<errors.count();n++)logThread->writeLog("SSL Error: "+errors.at(n).errorString().toAscii());
}