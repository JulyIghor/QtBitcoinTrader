// Copyright (C) 2013 July IGHOR.
// I want to create Bitcoin Trader application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form https://sourceforge.net/projects/bitcointrader/
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#include "exchange_mtgox.h"
#include <openssl/hmac.h>
#include "main.h"
#include <QDateTime>
#include <QSslError>

Exchange_MtGox::Exchange_MtGox(QByteArray pRestSign, QByteArray pRestKey)
	: QThread()
{
	sslEnabled=true;
	tickerOnly=false;
	vipRequestCount=0;
	privateRestSign=pRestSign;
	headerNoAuth.setValue("Host","data.mtgox.com");
	headerNoAuth.setValue("User-Agent","Qt Bitcoin Trader v"+appVerStr);
	headerNoAuth.setContentType("application/x-www-form-urlencoded");
	headerAuth=headerNoAuth;
	headerAuth.setValue("Rest-Key",pRestKey);
	moveToThread(this);
	softLagTime.restart();
	privateNonce=QDateTime::currentDateTime().toTime_t()*1000;
}

Exchange_MtGox::~Exchange_MtGox()
{
	if(isLogEnabled)logThread->writeLog("Mt.Gox API Thread Deleted");
}

void Exchange_MtGox::setupApi(QtBitcoinTrader *mainClass, bool tickOnly, bool sslEn)
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
	connect(this,SIGNAL(accVolumeChanged(double)),mainClass->ui.accountVolume,SLOT(setValue(double)));
	connect(this,SIGNAL(accFeeChanged(double)),mainClass->ui.accountFee,SLOT(setValue(double)));
	connect(this,SIGNAL(accBtcBalanceChanged(double)),mainClass->ui.accountBTC,SLOT(setValue(double)));
	connect(this,SIGNAL(accUsdBalanceChanged(double)),mainClass->ui.accountUSD,SLOT(setValue(double)));
	connect(this,SIGNAL(loginChanged(QString)),mainClass,SLOT(loginChanged(QString)));

	connect(this,SIGNAL(tickerHighChanged(double)),mainClass->ui.marketHigh,SLOT(setValue(double)));
	connect(this,SIGNAL(tickerLowChanged(double)),mainClass->ui.marketLow,SLOT(setValue(double)));
	connect(this,SIGNAL(tickerSellChanged(double)),mainClass->ui.marketSell,SLOT(setValue(double)));
	connect(this,SIGNAL(tickerLastChanged(double)),mainClass->ui.marketLast,SLOT(setValue(double)));
	connect(this,SIGNAL(tickerBuyChanged(double)),mainClass->ui.marketBuy,SLOT(setValue(double)));
	connect(this,SIGNAL(tickerVolumeChanged(double)),mainClass->ui.marketVolume,SLOT(setValue(double)));

	connect(this,SIGNAL(addLastTrade(double, qint64, double, QByteArray, bool)),mainClass,SLOT(addLastTrade(double, qint64, double, QByteArray, bool)));

	start();
}

void Exchange_MtGox::setSslEnabled(bool on)
{
	sslEnabled=on;
	clearValues();
	requestIdsNoAuth.clear();
	if(httpNoAuth->hasPendingRequests())httpNoAuth->clearPendingRequests();
	if(sslEnabled)
	{
		httpAuth->setHost("data.mtgox.com",QHttp::ConnectionModeHttps);
		httpNoAuth->setHost("data.mtgox.com",QHttp::ConnectionModeHttps);
	}
	else
	{
		httpAuth->setHost("data.mtgox.com",QHttp::ConnectionModeHttp);
		httpNoAuth->setHost("data.mtgox.com",QHttp::ConnectionModeHttp);
	}
}

void Exchange_MtGox::clearValues()
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
	lastFee=0.0;
	secondPart=0;
	apiDownCounter=0;
	lastHistory.clear();
	lastOrders.clear();
	requestIdsAuth.clear();
	requestIdsNoAuth.clear();
	if(httpAuth->hasPendingRequests())httpAuth->clearPendingRequests();
	if(httpNoAuth->hasPendingRequests())httpNoAuth->clearPendingRequests();
	lastFetchDate=QByteArray::number(QDateTime::currentDateTime().addSecs(-600).toTime_t())+"000000";
}

QByteArray Exchange_MtGox::getMidData(QString a, QString b,QByteArray *data)
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

void Exchange_MtGox::translateUnicodeStr(QString *str)
{
	const QRegExp rx("(\\\\u[0-9a-fA-F]{4})");
	int pos=0;
	while((pos=rx.indexIn(*str,pos))!=-1)str->replace(pos++, 6, QChar(rx.cap(1).right(4).toUShort(0, 16)));
}

void Exchange_MtGox::httpDoneNoAuth(int cId, bool error)
{
	int reqType=requestIdsNoAuth.value(cId,0);
	if(reqType>0)requestIdsNoAuth.remove(cId);else return;
	if(error)return;

	QByteArray data=httpNoAuth->readAll();

	bool lastApiDown=isApiDown;
	if(data.size()&&data.at(0)!='{')apiDownCounter++;else {apiDownCounter=0;isApiDown=false;}
	if(apiDownCounter==3)isApiDown=true;
	if(lastApiDown!=isApiDown)emit apiDownChanged(isApiDown);
	if(isApiDown)return;

	emit softLagChanged(softLagTime.elapsed()/1000.0);
	softLagTime.restart();

	bool success=getMidData("{\"result\":\"","\",",&data)=="success";

	if(success)
	{
		switch(reqType)
		{
			case 1: apiLagChanged(getMidData("lag_secs\":",",\"",&data).toDouble()); break;//lag
			case 3: //ticker
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

					QByteArray tickerSell=getMidData("sell\":{\"value\":\"","",&data);
					if(!tickerSell.isEmpty())
					{
						double newTickerSell=tickerSell.toDouble();
						if(newTickerSell!=lastTickerSell)emit tickerSellChanged(newTickerSell);
						lastTickerSell=newTickerSell;
					}

					QByteArray tickerLast=getMidData("last\":{\"value\":\"","",&data);
					if(!tickerLast.isEmpty())
					{
						double newTickerLast=tickerLast.toDouble();
						if(newTickerLast!=lastTickerLast)emit tickerLastChanged(newTickerLast);
						lastTickerLast=newTickerLast;
					}

					QByteArray tickerBuy=getMidData("buy\":{\"value\":\"","",&data);
					if(!tickerBuy.isEmpty())
					{
						double newTickerBuy=tickerBuy.toDouble();
						if(newTickerBuy!=lastTickerBuy)emit tickerBuyChanged(newTickerBuy);
						lastTickerBuy=newTickerBuy;
					}

					QByteArray tickerVolume=getMidData("vol\":{\"value\":\"","",&data);
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
			case 9: //money/trades/fetch
				if(data.size()<32)break;
				QStringList tradeList=QString(data).split("\"},{\"");
				for(int n=0;n<tradeList.count();n++)
				{
					QByteArray tradeData=tradeList.at(n).toAscii();
					emit addLastTrade(getMidData("\"amount\":\"","\",",&tradeData).toDouble(),getMidData("date\":",",\"",&tradeData).toLongLong(),getMidData("\"price\":\"","\",",&tradeData).toDouble(),getMidData("\"price_currency\":\"","\",\"",&tradeData),getMidData("\"trade_type\":\"","\"",&tradeData)=="ask");
					if(n==tradeList.count()-1)
						lastFetchDate=getMidData("\"tid\":\"","\",\"",&tradeData);
				}
				break;
		}
	}
}

void Exchange_MtGox::httpDoneAuth(int cId, bool error)
{
	int reqType=requestIdsAuth.value(cId,0);
	if(vipRequestCount&&(reqType>=5&&reqType<=7))vipRequestCount--;
	if(reqType>0)requestIdsAuth.remove(cId);else return;
	if(error)return;

	QByteArray data=httpAuth->readAll();

	bool lastApiDown=isApiDown;
	if(data.size()&&data.at(0)!='{')apiDownCounter++;else {apiDownCounter=0;isApiDown=false;}
	if(apiDownCounter==3)isApiDown=true;
	if(lastApiDown!=isApiDown)emit apiDownChanged(isApiDown);
	if(isApiDown)return;

	emit softLagChanged(softLagTime.elapsed()/1000.0);
	softLagTime.restart();

	bool success=getMidData("{\"result\":\"","\",",&data)=="success";

	if(success)
	{
		switch(reqType)
		{
		case 2: //info
			{
				if(apiLogin.isEmpty())
				{
					QByteArray login=getMidData("Login\":\"","\",",&data);
					if(!login.isEmpty()){apiLogin=login;emit loginChanged(login);}
				}

				QByteArray btcBalance=getMidData(currencyAStr+"\":{\"Balance\":{\"value\":\"","",&data);
				if(!btcBalance.isEmpty())
				{
					double newBtcBalance=btcBalance.toDouble();
					if(lastBtcBalance!=newBtcBalance)emit accBtcBalanceChanged(newBtcBalance);
					lastBtcBalance=newBtcBalance;
				}

				QByteArray usdBalance=getMidData(currencyBStr+"\":{\"Balance\":{\"value\":\"","",&data);
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
					bool isRightsGood=rights.contains("get_info")&&rights.contains("trade");
					if(!isRightsGood)emit identificationRequired();
					emit firstAccInfo();
					isFirstAccInfo=false;
				}
			}
			break;//info
		case 4://orders
			if(data.size()<=30){lastOrders.clear();emit ordersIsEmpty();break;}
			if(lastOrders!=data)
			{
				lastOrders=data;
				QString rezultData;
				QByteArray currentOrder=getMidData("oid","\"actions\":",&data);
				while(currentOrder.size())
				{
				rezultData.append(getMidData("\":\"","\",\"",&currentOrder)+";"+getMidData(",\"date\":",",",&currentOrder)+";"+getMidData("\"type\":\"","\",\"",&currentOrder)+";"+getMidData("\"status\":\"","\",\"",&currentOrder)+";"+getMidData("\"amount\":{\"value\":\"","\",\"",&currentOrder)+";"+getMidData("\"price\":{\"value\":\"","\",\"",&currentOrder)+";"+currencySignMap->value(getMidData("\"currency\":\"","\",\"",&currentOrder),"$")+"\n");
				if(data.size()>currentOrder.size())data.remove(0,currentOrder.size());
				currentOrder=getMidData("oid","\"actions\"",&data);
				}
				emit ordersChanged(rezultData);
			}
			break;//orders
		case 5: //order/cancel
			{
			QByteArray oid=getMidData("oid\":\"","\",\"",&data);
			if(!oid.isEmpty())emit orderCanceled(oid);
			}
			break;//order/cancel
		case 6: if(isLogEnabled)logThread->writeLog("Buy OK: "+data);break;//order/buy
		case 7: if(isLogEnabled)logThread->writeLog("Sell OK: "+data);break;//order/sell
		case 8: //money/wallet/history 
			if(lastHistory!=data)
			{
			double lastBuyPrice=0.0;
			double lastSellPrice=0.0;
			QString newLog(data);
			translateUnicodeStr(&newLog);
			QStringList dataList=newLog.split("\"Index\"");
			newLog.clear();
			for(int n=0;n<dataList.count();n++)
			{
				QByteArray curLog(dataList.at(n).toAscii());
				QByteArray logType=getMidData("\"Type\":\"","\",\"",&curLog);
				int logTypeInt=0;
				if(logType=="out"){logTypeInt=1;logType="<font color=\"red\">("+julyTr("LOG_SOLD","Sold").toAscii()+")</font>";}
				else 
					if(logType=="in"){logTypeInt=2;logType="<font color=\"blue\">("+julyTr("LOG_BOUGHT","Bought").toAscii()+")</font>";}
					else 
						if(logType=="fee"){logTypeInt=3;logType.clear();}
						else 
							if(logType=="deposit"){logTypeInt=4;logType.clear();logType="<font color=\"green\">("+julyTr("LOG_DEPOSIT","Deposit").toAscii()+")</font>";}
							else
								if(logType=="withdraw"){logTypeInt=5;logType.clear();logType="<font color=\"brown\">("+julyTr("LOG_WITHDRAW","Withdraw").toAscii()+")</font>";}
							if(logTypeInt)
							{
								QByteArray logValue=getMidData("\"Value\":{\"value\":\"","\",\"",&curLog);
								QByteArray logDate=getMidData("\"Date\":",",\"",&curLog);
								QByteArray logText=getMidData(" at ","\",\"",&curLog);
								if((logTypeInt==1||logTypeInt==2)&&(lastSellPrice==0.0||lastBuyPrice==0.0))
								{
									QByteArray priceValue;
									for(int n=0;n<logText.size();n++)
										if(QChar(logText.at(n)).isDigit()||logText.at(n)=='.')priceValue.append(logText.at(n));
									QByteArray priceCurrency=logText.left(logText.size()-priceValue.size());
									if(lastSellPrice==0.0&&logTypeInt==1)
									{
										lastSellPrice=priceValue.toDouble();
										emit accLastSellChanged(priceCurrency,lastSellPrice);
									}
									if(lastBuyPrice==0.0&&logTypeInt==2)
									{
										lastBuyPrice=priceValue.toDouble();
										emit accLastBuyChanged(priceCurrency,lastBuyPrice);
									}
								}

								if(logTypeInt==3&&logText.isEmpty())
								{
									logText="<font color=\"darkgreen\">";
									logText.append(" ("+julyTr("LOG_FEE","fee")+")");
									logText.append("</font>");
								}
								else
								if(!logText.isEmpty())logText=" "+julyTr("AT"," at %1").arg(QString("<font color=\"darkgreen\">"+logText+"</font>").replace("fee",julyTr("LOG_FEE","fee"))).toAscii();

								newLog.append("<font color=\"gray\">"+QDateTime::fromTime_t(logDate.toUInt()).toString(localDateTimeFormat)+"</font>&nbsp;<font color=\"#996515\">"+currencyASign+logValue+"</font>"+logText+" "+logType+"<br>");
							}
			}
			emit ordersLogChanged(newLog);
			lastHistory=data;
			}
			break;//money/wallet/history
		default: break;
		}
	}//success
	else
	{
		QString errorString=getMidData("error\":\"","\"",&data);
		QString tokenString=getMidData("token\":\"","\"}",&data);
		if(isLogEnabled)logThread->writeLog(errorString.toAscii()+" "+tokenString.toAscii());
		if(errorString.isEmpty())return;
		if(errorString=="Order not found")return;
		if(errorString.startsWith("Identification required"))
			if(tokenString!="login_error_invalid_nonce")emit identificationRequired();
	}
}

void Exchange_MtGox::run()
{
	if(isLogEnabled)logThread->writeLog("Mt.Gox API Thread Started");
	if(sslEnabled)
	{
		httpAuth=new QHttp("data.mtgox.com",QHttp::ConnectionModeHttps);
		httpNoAuth=new QHttp("data.mtgox.com",QHttp::ConnectionModeHttps);
	}
	else
	{
		httpAuth=new QHttp("data.mtgox.com",QHttp::ConnectionModeHttp);
		httpNoAuth=new QHttp("data.mtgox.com",QHttp::ConnectionModeHttp);
	}

	clearValues();

	secondTimer=new QTimer;
	connect(httpAuth,SIGNAL(requestFinished(int,bool)),this,SLOT(httpDoneAuth(int,bool)));
	connect(httpNoAuth,SIGNAL(requestFinished(int,bool)),this,SLOT(httpDoneNoAuth(int,bool)));
	connect(httpAuth,SIGNAL(sslErrors(const QList<QSslError> &)),this,SLOT(sslErrors(const QList<QSslError> &)));
	connect(httpNoAuth,SIGNAL(sslErrors(const QList<QSslError> &)),this,SLOT(sslErrors(const QList<QSslError> &)));

	connect(secondTimer,SIGNAL(timeout()),this,SLOT(secondSlot()));
	secondTimer->start(200);
	exec();
}

void Exchange_MtGox::cancelPendingAuthRequests()
{
	if(vipRequestCount)return;
	if(httpAuth->hasPendingRequests())httpAuth->clearPendingRequests();
	requestIdsAuth.clear();
}

void Exchange_MtGox::cancelPendingNoAuthRequests()
{
	if(httpNoAuth->hasPendingRequests())httpNoAuth->clearPendingRequests();
	requestIdsNoAuth.clear();
}

void Exchange_MtGox::reloadOrders()
{
	lastOrders.clear();
}

void Exchange_MtGox::secondSlot()
{
	emit softLagChanged(softLagTime.elapsed()/1000.0);

	if(requestIdsAuth.count()<10&&!vipRequestCount)//Max pending requests at time
	{
	if(requestIdsAuth.key(2,0)==0)requestIdsAuth[sendToApi(currencyRequestPair+"/money/info",true)]=2;
	if(!tickerOnly&&requestIdsAuth.key(4,0)==0)requestIdsAuth[sendToApi(currencyRequestPair+"/money/orders",true)]=4;
	} else cancelPendingAuthRequests();
	if(requestIdsNoAuth.count()<10)//Max pending requests at time
	{
	if(requestIdsNoAuth.key(1,0)==0)requestIdsNoAuth[sendToApi(currencyRequestPair+"/money/order/lag",false)]=1;
	if(requestIdsNoAuth.key(3,0)==0)requestIdsNoAuth[sendToApi(currencyRequestPair+"/money/ticker",false)]=3;
	if(requestIdsNoAuth.key(9,0)==0)requestIdsNoAuth[sendToApi(currencyRequestPair+"/money/trades/fetch?since="+lastFetchDate,false)]=9;
	} else cancelPendingNoAuthRequests();
	if(lastHistory.isEmpty())getHistory(false);
}

void Exchange_MtGox::getHistory(bool force)
{
	if(tickerOnly)return;
	if(force)lastHistory.clear();
	if(requestIdsNoAuth.key(8,0)==0)requestIdsAuth[sendToApi("money/wallet/history",true,"&currency=BTC")]=8;
}

void Exchange_MtGox::buy(double apiBtcToBuy, double apiPriceToBuy)
{
	if(tickerOnly)return;
	cancelPendingAuthRequests();
	vipRequestCount++;
	QByteArray params="&type=bid&amount_int="+QByteArray::number(apiBtcToBuy*100000000,'f',0)+"&price_int="+QByteArray::number(apiPriceToBuy*100000,'f',0);
	requestIdsAuth[sendToApi(currencyRequestPair+"/money/order/add",true,params)]=6;
	requestIdsAuth[sendToApi(currencyRequestPair+"/money/order/add",true,params,false)]=6;
	requestIdsAuth[sendToApi(currencyRequestPair+"/money/order/add",true,params,false)]=6;
	secondPart=3;
	secondSlot();
}

void Exchange_MtGox::sell(double apiBtcToSell, double apiPriceToSell)
{
	if(tickerOnly)return;
	cancelPendingAuthRequests();
	vipRequestCount++;
	QByteArray params="&type=ask&amount_int="+QByteArray::number(apiBtcToSell*100000000,'f',0)+"&price_int="+QByteArray::number(apiPriceToSell*100000,'f',0);
	requestIdsAuth[sendToApi(currencyRequestPair+"/money/order/add",true,params)]=7;
	requestIdsAuth[sendToApi(currencyRequestPair+"/money/order/add",true,params,false)]=7;
	requestIdsAuth[sendToApi(currencyRequestPair+"/money/order/add",true,params,false)]=7;
	secondPart=3;
	secondSlot();
}

void Exchange_MtGox::cancelOrder(QByteArray order)
{
	if(tickerOnly)return;
	cancelPendingAuthRequests();
	vipRequestCount++;
	requestIdsAuth[sendToApi(currencyRequestPair+"/money/order/cancel",true,"&oid="+order)]=5;
	requestIdsAuth[sendToApi(currencyRequestPair+"/money/order/cancel",true,"&oid="+order,false)]=5;
	requestIdsAuth[sendToApi(currencyRequestPair+"/money/order/cancel",true,"&oid="+order,false)]=5;
	secondPart=3;
	secondSlot();
}

int Exchange_MtGox::sendToApi(QByteArray method, bool auth, QByteArray commands, bool incNonce)
{
	if(auth)
	{
		if(incNonce)privateNonce++;
		QByteArray postData="nonce="+QByteArray::number(privateNonce)+"000000"+commands;
		QByteArray forHash=method+"0"+postData;forHash[method.size()]=0;
		headerAuth.setValue("Rest-Sign",hmacSha512(privateRestSign,forHash).toBase64());
		headerAuth.setRequest("POST","/api/2/"+method);
		headerAuth.setContentLength(postData.size());
		if(isLogEnabled)logThread->writeLog("/api/2/"+method+"?"+postData);
		return httpAuth->request(headerAuth,postData);
	}
	else
	{
		headerNoAuth.setRequest("GET","/api/2/"+method);
		return httpNoAuth->request(headerNoAuth);
	}
	return 0;
}

void Exchange_MtGox::sslErrors(const QList<QSslError> &errors)
{
	if(!isLogEnabled)return;
	for(int n=0;n<errors.count();n++)logThread->writeLog("SSL Error: "+errors.at(n).errorString().toAscii());
}