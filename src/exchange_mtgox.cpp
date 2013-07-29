// Copyright (C) 2013 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
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

Exchange_MtGox::Exchange_MtGox(QByteArray pRestSign, QByteArray pRestKey)
	: QThread()
{
	julyHttp=0;
	tickerOnly=false;
	privateRestSign=pRestSign;
	privateRestKey=pRestKey;
	moveToThread(this);
	authRequestTime.restart();
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

	connect(this,SIGNAL(depthUpdateOrder(double,double,bool)),mainClass,SLOT(depthUpdateOrder(double,double,bool)));
	connect(this,SIGNAL(showErrorMessage(QString)),mainClass,SLOT(showErrorMessage(QString)));
	connect(this,SIGNAL(accLastSellChanged(QByteArray,double)),mainClass,SLOT(accLastSellChanged(QByteArray,double)));
	connect(this,SIGNAL(accLastBuyChanged(QByteArray,double)),mainClass,SLOT(accLastBuyChanged(QByteArray,double)));

	connect(mainClass,SIGNAL(clearValues()),this,SLOT(clearValues()));
	connect(this,SIGNAL(firstTicker()),mainClass,SLOT(firstTicker()));
	connect(this,SIGNAL(firstAccInfo()),mainClass,SLOT(firstAccInfo()));
	connect(this,SIGNAL(apiLagChanged(double)),mainClass->ui.lagValue,SLOT(setValue(double)));
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

void Exchange_MtGox::clearVariables()
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
	lastDepthBidsMap.clear();
	lastDepthAsksMap.clear();
	lastDepthData.clear();
	lastInfoReceived=false;
	lastFetchDate=QByteArray::number(QDateTime::currentDateTime().addSecs(-600).toTime_t())+"000000";
}

void Exchange_MtGox::clearValues()
{
	clearVariables();
	if(julyHttp)julyHttp->clearPendingData();
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

void Exchange_MtGox::run()
{
	if(isLogEnabled)logThread->writeLog("Mt.Gox API Thread Started");
	clearVariables();

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
	static int infoCounter=0;
	if(userInfoTime.elapsed()>(lastInfoReceived?10000:1000)&&!isReplayPending(202))
	{
		userInfoTime.restart();
		sendToApi(202,currencyRequestPair+"/money/info",true,httpSplitPackets);
	}

	if(!tickerOnly&&!isReplayPending(204))sendToApi(204,currencyRequestPair+"/money/orders",true,httpSplitPackets);

	if(infoCounter==3&&!isReplayPending(111))sendToApi(111,currencyRequestPair+"/money/depth/fetch",false,httpSplitPackets);

	if(!isReplayPending(101))sendToApi(101,currencyRequestPair+"/money/order/lag",false,httpSplitPackets);
	if((infoCounter==0)&&!isReplayPending(103))sendToApi(103,currencyRequestPair+"/money/ticker",false,httpSplitPackets);
	if(!isReplayPending(104))sendToApi(104,currencyRequestPair+"/money/ticker_fast",false,httpSplitPackets);

	if(!isReplayPending(109))sendToApi(109,currencyRequestPair+"/money/trades/fetch?since="+lastFetchDate,false,httpSplitPackets);
	if(lastHistory.isEmpty())
		if(!isReplayPending(208))sendToApi(208,"money/wallet/history",true,httpSplitPackets,"&currency=BTC");
	if(!httpSplitPackets&&julyHttp)julyHttp->prepareDataSend();

	if(++infoCounter>9)
	{
		infoCounter=0;
		quint32 syncNonce=(QDateTime::currentDateTime().toTime_t()-1371854884)*10;
		if(privateNonce<syncNonce)privateNonce=syncNonce;
	}
	secondTimer->start(httpRequestInterval);
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
	QByteArray params="&type=bid&amount_int="+QByteArray::number(apiBtcToBuy*100000000,'f',0)+"&price_int="+QByteArray::number(apiPriceToBuy*100000,'f',0);
	sendToApi(306,currencyRequestPair+"/money/order/add",true,true,params);
}

void Exchange_MtGox::sell(double apiBtcToSell, double apiPriceToSell)
{
	if(tickerOnly)return;
	QByteArray params="&type=ask&amount_int="+QByteArray::number(apiBtcToSell*100000000,'f',0)+"&price_int="+QByteArray::number(apiPriceToSell*100000,'f',0);
	sendToApi(307,currencyRequestPair+"/money/order/add",true,true,params);
}

void Exchange_MtGox::cancelOrder(QByteArray order)
{
	if(tickerOnly)return;
	sendToApi(305,currencyRequestPair+"/money/order/cancel",true,true,"&oid="+order);
}

void Exchange_MtGox::sendToApi(int reqType, QByteArray method, bool auth, bool sendNow, QByteArray commands)
{
	if(julyHttp==0)
	{ 
		julyHttp=new JulyHttp("data.mtgox.com","Rest-Key: "+privateRestKey+"\r\n",this);
		connect(julyHttp,SIGNAL(anyDataReceived()),mainWindow_,SLOT(anyDataReceived()));
		connect(julyHttp,SIGNAL(setDataPending(bool)),mainWindow_,SLOT(setDataPending(bool)));
		connect(julyHttp,SIGNAL(apiDown(bool)),mainWindow_,SLOT(setApiDown(bool)));
		connect(julyHttp,SIGNAL(errorSignal(QString)),mainWindow_,SLOT(showErrorMessage(QString)));
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

void Exchange_MtGox::dataReceivedAuth(QByteArray data, int reqType)
{
	bool success=getMidData("{\"result\":\"","\",",&data)=="success";

	switch(reqType)
	{
	case 101:
		if(!success)break;
		if(data.startsWith("{\"result\":\"success\",\"data\":{\"lag"))emit apiLagChanged(getMidData("lag_secs\":",",\"",&data).toDouble());//lag
		else if(isLogEnabled)logThread->writeLog("Invalid lag data:"+data);
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
			else if(isLogEnabled)logThread->writeLog("Invalid ticker data:"+data);
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
			QByteArray tickerNow=getMidData("now\":\"","\"}",&data);
			if(lastFetchDate<tickerNow)
			{
				QByteArray tickerLast=getMidData("last\":{\"value\":\"","",&data);
				if(!tickerLast.isEmpty())
				{
					double newTickerLast=tickerLast.toDouble();
					if(newTickerLast!=lastTickerLast)emit tickerLastChanged(newTickerLast);
					lastTickerLast=newTickerLast;
				}
			}
			if(isFirstTicker)
			{
				emit firstTicker();
				isFirstTicker=false;
			}
		}
		else if(isLogEnabled)logThread->writeLog("Invalid ticker fast data:"+data);
			break; //ticker fast
	case 109: //money/trades/fetch
		if(success&&data.size()>32)
		{
			if(data.startsWith("{\"result\":\"success\",\"data\":[{\"date"))
			{
				QStringList tradeList=QString(data).split("\"},{\"");
				for(int n=0;n<tradeList.count();n++)
				{
					QByteArray tradeData=tradeList.at(n).toAscii();
					double doubleAmount=getMidData("\"amount\":\"","\",",&tradeData).toDouble();
					double doublePrice=getMidData("\"price\":\"","\",",&tradeData).toDouble();
					QByteArray priceCurrency=getMidData("\"price_currency\":\"","\",\"",&tradeData);
					QByteArray tradeType=getMidData("\"trade_type\":\"","\"",&tradeData);
					if(doubleAmount>0.0&&doublePrice>0.0&&!priceCurrency.isEmpty()&&!tradeType.isEmpty())
					{
						emit addLastTrade(doubleAmount,getMidData("date\":",",\"",&tradeData).toLongLong(),doublePrice,priceCurrency,tradeType=="ask");
						if(n==tradeList.count()-1)
						{
							QByteArray nextFetchDate=getMidData("\"tid\":\"","\",\"",&tradeData);
							if(!nextFetchDate.isEmpty())lastFetchDate=nextFetchDate;
						}
					}
					else if(isLogEnabled)logThread->writeLog("Invalid trades fetch data line:"+tradeData);
				}
			}
			else if(isLogEnabled)logThread->writeLog("Invalid trades fetch data:"+data);
		}
		break;
	case 111: //depth
		if(data.startsWith("{\"result\":\"success\",\"data\":{\"now\""))
		{
			emit depthUpdateOrder(0.0,0.0,true);
			if(lastDepthData!=data)
			{
				lastDepthData=data;
				QMap<double,double> currentAsksMap;
				QStringList asksList=QString(getMidData("\"asks\":[{","}]",&data)).split("},{");
				if(depthCountLimit)while(asksList.count()>depthCountLimit)asksList.removeLast();
				for(int n=0;n<asksList.count();n++)
				{
					QByteArray currentRow=asksList.at(n).toAscii();
					double priceDouble=getMidData("price\":",",\"",&currentRow).toDouble();
					double amount=getMidData("amount\":",",\"",&currentRow).toDouble();
					if(priceDouble>0.0&&amount>0.0)
					{
						currentAsksMap[priceDouble]=amount;
						if(lastDepthAsksMap.value(priceDouble,0.0)!=amount)emit depthUpdateOrder(priceDouble,amount,true);
					}
				}
				QList<double> currentAsksList=lastDepthAsksMap.keys();
				for(int n=0;n<currentAsksList.count();n++)
					if(currentAsksMap.value(currentAsksList.at(n),0)==0)emit depthUpdateOrder(currentAsksList.at(n),0.0,true);//Remove price
				lastDepthAsksMap=currentAsksMap;

				QMap<double,double> currentBidsMap;
				QStringList bidsList=QString(getMidData("\"bids\":[{","}]",&data)).split("},{");
				if(depthCountLimit)while(bidsList.count()>depthCountLimit)bidsList.removeFirst();
				for(int n=0;n<bidsList.count();n++)
				{
					QByteArray currentRow=bidsList.at(n).toAscii();
					double priceDouble=getMidData("price\":",",\"",&currentRow).toDouble();
					double amount=getMidData("amount\":",",\"",&currentRow).toDouble();
					if(priceDouble>0.0&&amount>0.0)
					{
						currentBidsMap[priceDouble]=amount;
						if(lastDepthBidsMap.value(priceDouble,0.0)!=amount)emit depthUpdateOrder(priceDouble,amount,false);
					}
					if(priceDouble>0.0&&amount>0.0)emit depthUpdateOrder(priceDouble,amount,false);
				}
				QList<double> currentBidsList=lastDepthBidsMap.keys();
				for(int n=0;n<currentBidsList.count();n++)
					if(currentBidsMap.value(currentBidsList.at(n),0)==0)emit depthUpdateOrder(currentBidsList.at(n),0.0,false);//Remove price
				lastDepthBidsMap=currentBidsMap;
			}
		}
		else if(isLogEnabled)logThread->writeLog("Invalid depth data:"+data);
		break;
	case 202: //info
		{
			if(!success)break;
			if(data.startsWith("{\"result\":\"success\",\"data\":{\"Login"))
			{
				lastInfoReceived=true;
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
						if(!isRightsGood)emit showErrorMessage("I:>invalid_rights");
						emit firstAccInfo();
						isFirstAccInfo=false;
					}
				}
			}
			else if(isLogEnabled)logThread->writeLog("Invalid Info data:"+data);
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
				QString rezultData;
				QByteArray currentOrder=getMidData("oid","\"actions\":",&data);
				while(currentOrder.size())
				{
					rezultData.append(getMidData("\":\"","\",\"",&currentOrder)+";"+getMidData(",\"date\":",",",&currentOrder)+";"+getMidData("\"type\":\"","\",\"",&currentOrder)+";"+getMidData("\"status\":\"","\",\"",&currentOrder)+";"+getMidData("\"amount\":{\"value\":\"","\",\"",&currentOrder)+";"+getMidData("\"price\":{\"value\":\"","\",\"",&currentOrder)+";"+currencySignMap->value(getMidData("\"currency\":\"","\",\"",&currentOrder),"$")+";"+currencySignMap->value(getMidData("\"item\":\"","\",\"",&currentOrder),"$")+"\n");
					if(data.size()>currentOrder.size())data.remove(0,currentOrder.size());
					currentOrder=getMidData("oid","\"actions\"",&data);
				}
				emit ordersChanged(rezultData);

				lastInfoReceived=false;
			}
		}
		else if(isLogEnabled)logThread->writeLog("Invalid Orders data:"+data);
		break;//orders
	case 305: //order/cancel
		{
			if(!success)break;
			QByteArray oid=getMidData("oid\":\"","\",\"",&data);
			if(!oid.isEmpty())emit orderCanceled(oid);
			else if(isLogEnabled)logThread->writeLog("Invalid Order/Cancel data:"+data);
		}
		break;//order/cancel
	case 306: //order/buy
			  if(!success)break;
			  if(data.startsWith("{\"result\":\"success\",\"data\":\""))
			  {
				 if(isLogEnabled)logThread->writeLog("Buy OK: "+data);
			  }
			  else if(isLogEnabled)logThread->writeLog("Invalid Order Buy Data:"+data);
			  break;//order/buy
	case 307: //order/sell
			  if(!success)break;
			  if(data.startsWith("{\"result\":\"success\",\"data\":\""))
			  {
				 if(isLogEnabled)logThread->writeLog("Sell OK: "+data);
			  }
			  else if(isLogEnabled)logThread->writeLog("Invalid Order Sell Data:"+data);
			  break;//order/sell
	case 208: //money/wallet/history 
		if(!success)break;
		if(data.startsWith("{\"result\":\"success\",\"data\":{\"records"))
		{
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
										QByteArray currencyA("USD");
										QByteArray currencyB("USD");
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
		}
		else if(isLogEnabled)logThread->writeLog("Invalid History data:"+data.left(200));
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
		if(isLogEnabled)logThread->writeLog(errorString.toAscii()+" "+tokenString.toAscii());
		if(errorString.isEmpty())return;
		if(errorString==QLatin1String("Order not found"))return;
		errorString.append("<br>"+tokenString);
		errorString.append("<br>"+QString::number(reqType));
		if(reqType<300)emit showErrorMessage("I:>"+errorString);
	}
	else errorCount=0;
}

void Exchange_MtGox::sslErrors(const QList<QSslError> &errors)
{
	QStringList errorList;
	for(int n=0;n<errors.count();n++)errorList<<errors.at(n).errorString();
	if(isLogEnabled)logThread->writeLog(errorList.join(" ").toAscii());
	emit showErrorMessage("SSL Error: "+errorList.join(" "));
}