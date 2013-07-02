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
#include <QNetworkReply>
#include <QMutex>

Exchange_MtGox::Exchange_MtGox(QByteArray pRestSign, QByteArray pRestKey)
	: QThread()
{
	isApiDown=false;
	tickerOnly=false;
	privateRestSign=pRestSign;
	privateRestKey=pRestKey;
	moveToThread(this);
	authRequestTime.restart();
	softLagTime.restart();
	privateNonce=(QDateTime::currentDateTime().toTime_t()-1371854884)*10;
}

Exchange_MtGox::~Exchange_MtGox()
{
	if(isLogEnabled)logThread->writeLog("Mt.Gox API Thread Deleted");
}

void Exchange_MtGox::setupApi(QtBitcoinTrader *mainClass, bool tickOnly)
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

	connect(this,SIGNAL(identificationRequired(QString)),mainClass,SLOT(identificationRequired(QString)));
	connect(this,SIGNAL(apiDownChanged(bool)),mainClass,SLOT(setApiDown(bool)));
	connect(this,SIGNAL(accLastSellChanged(QByteArray,double)),mainClass,SLOT(accLastSellChanged(QByteArray,double)));
	connect(this,SIGNAL(accLastBuyChanged(QByteArray,double)),mainClass,SLOT(accLastBuyChanged(QByteArray,double)));

	connect(mainClass,SIGNAL(clearValues()),this,SLOT(clearValues()));
	connect(this,SIGNAL(firstTicker()),mainClass,SLOT(firstTicker()));
	connect(this,SIGNAL(firstAccInfo()),mainClass,SLOT(firstAccInfo()));
	connect(this,SIGNAL(apiLagChanged(double)),mainClass->ui.lagValue,SLOT(setValue(double)));
	connect(this,SIGNAL(softLagChanged(int)),mainClass,SLOT(setSoftLagValue(int)));
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

void Exchange_MtGox::clearValues()
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
	lastFee=0.0;
	secondPart=0;
	apiDownCounter=0;
	lastHistory.clear();
	lastOrders.clear();
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

void Exchange_MtGox::requestFinished(QNetworkReply *replay)
{
	QByteArray data=replay->readAll();
	int reqType=requestsMap.value(replay,0);
	bool isAuthRequest=reqType>199;

	bool isUnknownRequest=data.size()==0||data.at(0)=='<';

	if(isAuthRequest)
	{
		bool lastApiDown=isApiDown;
		if(isUnknownRequest)
		{
			if(++apiDownCounter>3||softLagTime.elapsed()>2000)isApiDown=true;
		}
		else
		{
			if(isLogEnabled)logThread->writeLog("AuthData: "+data);
			authRequestTime.restart();
			apiDownCounter=0;
			isApiDown=false;
		}
		if(lastApiDown!=isApiDown)emit apiDownChanged(isApiDown);
	}
	else
	{
		if(isUnknownRequest||!isApiDown&&authRequestTime.elapsed()>15000)
		{
			if(++apiDownCounter>3)
			{
				isApiDown=true;
				emit apiDownChanged(isApiDown);
			}
		}
	}

	if(isUnknownRequest)return;


	emit softLagChanged(softLagTime.elapsed());
	softLagTime.restart();

	bool success=getMidData("{\"result\":\"","\",",&data)=="success";

	switch(reqType)
	{
		case 101: if(success)apiLagChanged(getMidData("lag_secs\":",",\"",&data).toDouble()); break;//lag
		case 103: //ticker
			if(success)
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

				QByteArray tickerVolume=getMidData("vol\":{\"value\":\"","",&data);
				if(!tickerVolume.isEmpty())
				{
					double newTickerVolume=tickerVolume.toDouble();
					if(newTickerVolume!=lastTickerVolume)emit tickerVolumeChanged(newTickerVolume);
					lastTickerVolume=newTickerVolume;
				}
				if(isFirstTicker)
				{
					QByteArray tickerLast=getMidData("last\":{\"value\":\"","",&data);
					if(!tickerLast.isEmpty())
					{
						double newTickerLast=tickerLast.toDouble();
						if(newTickerLast!=lastTickerLast)emit tickerLastChanged(newTickerLast);
						lastTickerLast=newTickerLast;
					}
					emit firstTicker();
					isFirstTicker=false;
				}
			}
			break;//ticker
		case 109: //money/trades/fetch
			if(success&&data.size()>32)
			{
				if(data.size()<32)break;
				QStringList tradeList=QString(data).split("\"},{\"");
				for(int n=0;n<tradeList.count();n++)
				{
					QByteArray tradeData=tradeList.at(n).toAscii();
					emit addLastTrade(getMidData("\"amount\":\"","\",",&tradeData).toDouble(),getMidData("date\":",",\"",&tradeData).toLongLong(),getMidData("\"price\":\"","\",",&tradeData).toDouble(),getMidData("\"price_currency\":\"","\",\"",&tradeData),getMidData("\"trade_type\":\"","\"",&tradeData)=="ask");
					if(n==tradeList.count()-1)
					{
						QByteArray nextFetchDate=getMidData("\"tid\":\"","\",\"",&tradeData);
						if(!nextFetchDate.isEmpty())lastFetchDate=nextFetchDate;
					}
				}
			}
			break;
		case 202: //info
			{
				if(!success)break;
				if(isLogEnabled)logThread->writeLog("Info: "+data);
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
					if(!rights.isEmpty())
					{
						bool isRightsGood=rights.contains("get_info")&&rights.contains("trade");
						if(!isRightsGood)emit identificationRequired("invalid_rights");
						emit firstAccInfo();
						isFirstAccInfo=false;
					}
				}
			}
			break;//info
		case 204://orders
			if(!success)break;
			if(data.size()<=30){lastOrders.clear();emit ordersIsEmpty();break;}
			if(lastOrders!=data)
			{
				lastOrders=data;
				QString rezultData;
				QByteArray currentOrder=getMidData("oid","\"actions\":",&data);
				while(currentOrder.size())
				{
					rezultData.append(getMidData("\":\"","\",\"",&currentOrder)+";"+getMidData(",\"date\":",",",&currentOrder)+";"+getMidData("\"type\":\"","\",\"",&currentOrder)+";"+getMidData("\"status\":\"","\",\"",&currentOrder)+";"+getMidData("\"amount\":{\"value\":\"","\",\"",&currentOrder)+";"+getMidData("\"price\":{\"value\":\"","\",\"",&currentOrder)+";"+currencySignMap->value(getMidData("\"currency\":\"","\",\"",&currentOrder),"$")+";"+currencySignMap->value(getMidData("\"item\":\"","\",\"",&currentOrder),"$")+"\n");
					if(data.size()>currentOrder.size())data.remove(0,currentOrder.size());
					currentOrder=getMidData("oid","\"actions\"",&data);
				}
				emit ordersChanged(rezultData);
			}
			break;//orders
		case 305: //order/cancel
			{
				if(!success)break;
				QByteArray oid=getMidData("oid\":\"","\",\"",&data);
				if(!oid.isEmpty())emit orderCanceled(oid);
			}
			break;//order/cancel
		case 306: if(isLogEnabled)logThread->writeLog("Buy OK: "+data);break;//order/buy
		case 307: if(isLogEnabled)logThread->writeLog("Sell OK: "+data);break;//order/sell
		case 208: //money/wallet/history 
			if(!success)break;
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
										QByteArray currencyA="USD";
										QByteArray currencyB="USD";
										QByteArray logValue=getMidData("\"Value\":{\"value\":\"","\",\"",&curLog);
										QByteArray logDate=getMidData("\"Date\":",",\"",&curLog);
										QByteArray logText=getMidData(" at ","\",\"",&curLog);
										currencyA=getMidData("\"currency\":\"","\"",&curLog);
										if((logTypeInt==1||logTypeInt==2)&&(lastSellPrice==0.0||lastBuyPrice==0.0))
										{
											QByteArray priceValue;
											QByteArray priceSign;
											for(int n=0;n<logText.size();n++)
												if(QChar(logText.at(n)).isDigit()||logText.at(n)=='.')priceValue.append(logText.at(n));
												else priceSign.append(logText.at(n));
												if(priceSign.isEmpty())priceSign="$";

												currencyB=currencySignMap->key(priceSign,"$");
												if(lastSellPrice==0.0&&logTypeInt==1)
												{
													lastSellPrice=priceValue.toDouble();
													emit accLastSellChanged(currencyB,lastSellPrice);
												}
												if(lastBuyPrice==0.0&&logTypeInt==2)
												{
													lastBuyPrice=priceValue.toDouble();
													emit accLastBuyChanged(currencyB,lastBuyPrice);
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

										newLog.append("<font color=\"gray\">"+QDateTime::fromTime_t(logDate.toUInt()).toString(localDateTimeFormat)+"</font>&nbsp;<font color=\"#996515\">"+currencySignMap->value(currencyA,"USD")+logValue+"</font>"+logText+" "+logType+"<br>");
									}
				}
				emit ordersLogChanged(newLog);
				lastHistory=data;
			}
			break;//money/wallet/history
		default: break;
	}

	if(isAuthRequest&&!success)
	{
		QString errorString=getMidData("error\":\"","\"",&data);
		QString tokenString=getMidData("token\":\"","\"}",&data);
		if(isLogEnabled)logThread->writeLog(errorString.toAscii()+" "+tokenString.toAscii());
		if(errorString.isEmpty())return;
		if(errorString=="Order not found")return;
		if(reqType<300)
		{
			static int invalidRequestCount=0;
			if(invalidRequestCount++>5)
				emit identificationRequired(errorString+"<br>"+tokenString+"<br>"+replay->url().toString().toAscii());
		}
	}
	removeReplay(replay);
}

void Exchange_MtGox::run()
{
	if(isLogEnabled)logThread->writeLog("Mt.Gox API Thread Started");

	clearValues();

	secondTimer=new QTimer;
	secondTimer->setSingleShot(true);
	connect(secondTimer,SIGNAL(timeout()),this,SLOT(secondSlot()));
	secondSlot();
	exec();
}

void Exchange_MtGox::reloadOrders()
{
	lastOrders.clear();
}

void Exchange_MtGox::secondSlot()
{
	emit softLagChanged(softLagTime.elapsed());
	static int requestCounter=1;
	switch(requestCounter)
	{
	case 1: if(!isReplayPending(101)){sendToApi(101,currencyRequestPair+"/money/order/lag",false);break;}
	case 2: if(!isReplayPending(202)){sendToApi(202,currencyRequestPair+"/money/info",true);break;}
	case 3: if(!isReplayPending(103)){sendToApi(103,currencyRequestPair+"/money/ticker",false);break;}
	case 4: if(!tickerOnly&&!isReplayPending(204)){sendToApi(204,currencyRequestPair+"/money/orders",true);break;}
	case 5:
		if(lastHistory.isEmpty())getHistory(false);
		if(!isReplayPending(109)){sendToApi(109,currencyRequestPair+"/money/trades/fetch?since="+lastFetchDate,false);break;}
	default: requestCounter=1; break;
	}
	requestCounter++;
	if(requestCounter>5)requestCounter=1;

	secondTimer->start(httpRequestInterval);
}

bool Exchange_MtGox::isReplayPending(int reqType)
{
	QNetworkReply *pendingReplay=requestsMap.key(reqType,0);
	if(pendingReplay==0)return false;
	if(timeStampMap.value(pendingReplay,0)+5>currentTimeStamp)return true;
	removeReplay(pendingReplay);
	return false;
}

void Exchange_MtGox::getHistory(bool force)
{
	if(tickerOnly)return;
	if(force)lastHistory.clear();
	if(!isReplayPending(208))sendToApi(208,"money/wallet/history",true,"&currency=BTC");
}

void Exchange_MtGox::buy(double apiBtcToBuy, double apiPriceToBuy)
{
	if(tickerOnly)return;
	QByteArray params="&type=bid&amount_int="+QByteArray::number(apiBtcToBuy*100000000,'f',0)+"&price_int="+QByteArray::number(apiPriceToBuy*100000,'f',0);
	isReplayPending(306);
	sendToApi(306,currencyRequestPair+"/money/order/add",true,params);
	sendToApi(306,currencyRequestPair+"/money/order/add",true,params,false);
	sendToApi(306,currencyRequestPair+"/money/order/add",true,params,false);
	secondPart=3;
	secondSlot();
}

void Exchange_MtGox::sell(double apiBtcToSell, double apiPriceToSell)
{
	if(tickerOnly)return;
	QByteArray params="&type=ask&amount_int="+QByteArray::number(apiBtcToSell*100000000,'f',0)+"&price_int="+QByteArray::number(apiPriceToSell*100000,'f',0);
	isReplayPending(307);
	sendToApi(307,currencyRequestPair+"/money/order/add",true,params);
	sendToApi(307,currencyRequestPair+"/money/order/add",true,params,false);
	sendToApi(307,currencyRequestPair+"/money/order/add",true,params,false);
	secondPart=3;
	secondSlot();
}

void Exchange_MtGox::cancelOrder(QByteArray order)
{
	if(tickerOnly)return;
	isReplayPending(305);
	sendToApi(305,currencyRequestPair+"/money/order/cancel",true,"&oid="+order);
	sendToApi(305,currencyRequestPair+"/money/order/cancel",true,"&oid="+order,false);
	sendToApi(305,currencyRequestPair+"/money/order/cancel",true,"&oid="+order,false);
	secondPart=3;
	secondSlot();
}

void Exchange_MtGox::removeReplay(QNetworkReply* replay)
{
	replay->abort();
	requestsMap.remove(replay);
	timeStampMap.remove(replay);
	replay->deleteLater();
}

void Exchange_MtGox::sendToApi(int reqType, QByteArray method, bool auth, QByteArray commands, bool incNonce)
{
	static QMutex mutex;
	mutex.lock();
	static QUrl httpsUrl("https://data.mtgox.com");

	static QNetworkAccessManager networkManager(this);

	static QNetworkRequest requestAuth;
	static QNetworkRequest requestNoAuth;

	static bool firstSetup=true;
	if(firstSetup)
	{ 
	firstSetup=false;
	connect(&networkManager,SIGNAL(finished(QNetworkReply *)),this,SLOT(requestFinished(QNetworkReply *)));
	connect(&networkManager,SIGNAL(sslErrors(QNetworkReply *, const QList<QSslError> &)),this,SLOT(sslErrorsSlot(QNetworkReply *, const QList<QSslError> &)));

	requestNoAuth.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");
	requestNoAuth.setRawHeader("User-Agent","Qt Bitcoin Trader v"+appVerStr);

	requestAuth.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");
	requestAuth.setRawHeader("User-Agent","Qt Bitcoin Trader v"+appVerStr);
	requestAuth.setRawHeader("Rest-Key",privateRestKey);
	requestAuth.setPriority(QNetworkRequest::HighPriority);
	requestNoAuth.setPriority(QNetworkRequest::LowPriority);
	}

	if(auth)
	{
		if(incNonce)privateNonce++;
		QByteArray postData="nonce="+QByteArray::number(privateNonce);
		postData.append(postData.right(6));postData.append(commands);
		QByteArray forHash=method+"0"+postData;forHash[method.size()]=0;
		requestAuth.setRawHeader("Rest-Sign",hmacSha512(privateRestSign,forHash).toBase64());
		requestAuth.setHeader(QNetworkRequest::ContentLengthHeader,postData.size());
		if(isLogEnabled)logThread->writeLog("/api/2/"+method+"?"+postData);

		httpsUrl.setEncodedPath("api/2/"+method);
		requestAuth.setUrl(httpsUrl);
		QNetworkReply *replay=networkManager.post(requestAuth,postData);
		requestsMap[replay]=reqType;
		timeStampMap[replay]=currentTimeStamp;
	}
	else
	{
		httpsUrl.setEncodedPath("api/2/"+method);
		requestNoAuth.setUrl(httpsUrl);

		QNetworkReply *replay=0;
		if(commands.isEmpty())replay=networkManager.get(requestNoAuth);
		else replay=networkManager.post(requestNoAuth,commands);

		requestsMap[replay]=reqType;
		timeStampMap[replay]=currentTimeStamp;
	}
	mutex.unlock();
}

void Exchange_MtGox::sslErrorsSlot(QNetworkReply *replay, const QList<QSslError> &errors)
{
	removeReplay(replay);
	QStringList errorList;
	for(int n=0;n<errors.count();n++)errorList<<errors.at(n).errorString();
	if(isLogEnabled)logThread->writeLog(errorList.join(" ").toAscii());
	emit identificationRequired("SSL Error: "+errorList.join(" ")+replay->errorString());
}