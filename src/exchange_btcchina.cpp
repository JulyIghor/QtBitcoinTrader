// Copyright (C) 2013 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form https://sourceforge.net/projects/bitcointrader/
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#include "exchange_btcchina.h"
#include <openssl/hmac.h>
#include "main.h"
#include <QDateTime>

Exchange_BTCChina::Exchange_BTCChina(QByteArray pRestSign, QByteArray pRestKey)
	: QThread()
{
	forceDepthLoad=false;
	julyHttpAuth=0;
	julyHttpPublic=0;
	tickerOnly=false;
	privateRestSign=pRestSign;
	privateRestKey=pRestKey;
	moveToThread(this);
	authRequestTime.restart();
	tonceCounter=0;
	privateTonce=QDateTime::currentDateTime().toTime_t();
}

Exchange_BTCChina::~Exchange_BTCChina()
{
	if(isLogEnabled)logThread->writeLog("BTCChina API Thread Deleted");
}

void Exchange_BTCChina::translateUnicodeStr(QString *str)
{
	const QRegExp rx("(\\\\u[0-9a-fA-F]{4})");
	int pos=0;
	while((pos=rx.indexIn(*str,pos))!=-1)str->replace(pos++, 6, QChar(rx.cap(1).right(4).toUShort(0, 16)));
}

void Exchange_BTCChina::setupApi(QtBitcoinTrader *mainClass, bool tickOnly)
{
	tickerOnly=tickOnly;
	if(!tickerOnly)
	{
		connect(mainClass,SIGNAL(apiBuy(double, double)),this,SLOT(buy(double, double)));
		connect(mainClass,SIGNAL(apiSell(double, double)),this,SLOT(sell(double, double)));
		connect(mainClass,SIGNAL(cancelOrderByOid(QByteArray)),this,SLOT(cancelOrder(QByteArray)));
		connect(this,SIGNAL(ordersChanged(QList<OrderItem> *)),mainClass,SLOT(ordersChanged(QList<OrderItem> *)));
		connect(mainClass,SIGNAL(cancelOrderByOid(QByteArray)),this,SLOT(cancelOrder(QByteArray)));
		//connect(mainClass,SIGNAL(getHistory(bool)),this,SLOT(getHistory(bool)));
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

void Exchange_BTCChina::clearVariables()
{
	isFirstTicker=true;
	cancelingOrderIDs.clear();
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
	reloadDepth();
	lastInfoReceived=false;
	lastFetchDate=QByteArray::number(QDateTime::currentDateTime().addSecs(-600).toTime_t())+"0000000";
}

void Exchange_BTCChina::clearValues()
{
	clearVariables();
	if(julyHttpAuth)julyHttpAuth->clearPendingData();
}

QByteArray Exchange_BTCChina::getMidData(QString a, QString b,QByteArray *data)
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

void Exchange_BTCChina::run()
{
	if(isLogEnabled)logThread->writeLog("BTCChina API Thread Started");
	clearVariables();

	secondTimer=new QTimer;
	secondTimer->setSingleShot(true);
	connect(secondTimer,SIGNAL(timeout()),this,SLOT(secondSlot()));
	secondSlot();
	exec();
}

void Exchange_BTCChina::secondSlot()
{
	static int infoCounter=0;
	switch(infoCounter)
	{
	case 0: if(!tickerOnly&&!isReplayPending(204))sendToApi(204,"getOrders",true,true/*,"false"*/); break;
	case 1: if(!isReplayPending(202))sendToApi(202,"getAccountInfo",true,true); break;
	//case 2: if(lastHistory.isEmpty()&&!isReplayPending(208))sendToApi(208,"getTransactions",true,true); break;
	default: break;
	}

	if(!isReplayPending(103))sendToApi(103,"ticker",false,false);
	if(!isReplayPending(109))sendToApi(109,"trades",false,false);

	if(!depthRefreshBlocked&&(forceDepthLoad||infoCounter==2&&!isReplayPending(111)))
	{
	    if(!isReplayPending(111))sendToApi(111,"orderbook",false,false);
		forceDepthLoad=false;
	}
	if(julyHttpPublic)julyHttpPublic->prepareDataSend();

	if(++infoCounter>2)
	{
		infoCounter=0;
		quint32 syncTonce=QDateTime::currentDateTime().toTime_t();
		if(privateTonce<syncTonce)
		{
			privateTonce=syncTonce;
			tonceCounter=0;
		}
	}

	secondTimer->start(httpRequestInterval);
}

bool Exchange_BTCChina::isReplayPending(int reqType)
{
	if(reqType<200)
	{
	if(julyHttpPublic==0)return false;
	return julyHttpPublic->isReqTypePending(reqType);
	}

	if(julyHttpAuth==0)return false;
	return julyHttpAuth->isReqTypePending(reqType);
}

void Exchange_BTCChina::getHistory(bool force)
{
	if(tickerOnly)return;
	if(force)lastHistory.clear();
	//if(!isReplayPending(208))sendToApi(208,"user_transactions/",true,true);
}

void Exchange_BTCChina::buy(double apiBtcToBuy, double apiPriceToBuy)
{
	if(tickerOnly)return;
	sendToApi(306,"buyOrder",true,true,numForSellFromDouble(apiPriceToBuy,priceDecimals)+","+numForSellFromDouble(apiBtcToBuy,btcDecimals));
}

void Exchange_BTCChina::sell(double apiBtcToSell, double apiPriceToSell)
{
	if(tickerOnly)return;
	sendToApi(307,"sellOrder",true,true,numForSellFromDouble(apiPriceToSell,priceDecimals)+","+numForSellFromDouble(apiBtcToSell,btcDecimals));
}

void Exchange_BTCChina::cancelOrder(QByteArray order)
{
	if(tickerOnly)return;
	cancelingOrderIDs<<order;
	sendToApi(305,"cancelOrder",true,true,order);
}

QByteArray Exchange_BTCChina::numForSellFromDouble(double val, int maxDecimals)
{
	val=mainWindow.getValidDoubleForPercision(val,3,false);
	QByteArray numberText=QByteArray::number(val,'f',maxDecimals);
	int curPos=numberText.size()-1;
	while(curPos>0&&numberText.at(curPos)=='0')numberText.remove(curPos--,1);
	if(numberText.size()&&numberText.at(numberText.size()-1)=='.')numberText.remove(numberText.size()-1,1);
	return numberText;
}

void Exchange_BTCChina::sendToApi(int reqType, QByteArray method, bool auth, bool sendNow, QByteArray commands)
{
	if(auth)
	{
		if(julyHttpAuth==0)
		{
			julyHttpAuth=new JulyHttp("api.btcchina.com","",this,true,true,"application/json-rpc");
			connect(julyHttpAuth,SIGNAL(anyDataReceived()),mainWindow_,SLOT(anyDataReceived()));
			connect(julyHttpAuth,SIGNAL(setDataPending(bool)),mainWindow_,SLOT(setDataPending(bool)));
			connect(julyHttpAuth,SIGNAL(apiDown(bool)),mainWindow_,SLOT(setApiDown(bool)));
			connect(julyHttpAuth,SIGNAL(errorSignal(QString)),mainWindow_,SLOT(showErrorMessage(QString)));
			connect(julyHttpAuth,SIGNAL(sslErrorSignal(const QList<QSslError> &)),this,SLOT(sslErrors(const QList<QSslError> &)));
			connect(julyHttpAuth,SIGNAL(dataReceived(QByteArray,int)),this,SLOT(dataReceivedAuth(QByteArray,int)));
		}

		QByteArray signatureParams;

		if(reqType!=204)
		{
			signatureParams=commands;
		}

		QByteArray postData;
		QByteArray appendedHeader;
		QByteArray tonceCounterData=QByteArray::number(tonceCounter++);

		tonceCounterData.prepend(QByteArray("0").repeated(6-tonceCounterData.size()));


		QByteArray tonce=QByteArray::number(QDateTime::currentDateTime().toTime_t())+tonceCounterData;
		QByteArray signatureString="tonce="+tonce+"&accesskey="+privateRestKey+"&requestmethod=post&id=1&method="+method+"&params="+signatureParams;

		signatureString=privateRestKey+":"+hmacSha1(privateRestSign,signatureString).toHex();

		//if(reqType==208)commands="\""+commands+"\"";

		postData="{\"method\":\""+method+"\",\"params\":["+commands+"],\"id\":1}";

		appendedHeader="Authorization: Basic "+signatureString.toBase64()+"\r\n";
		appendedHeader+="Json-Rpc-Tonce: "+tonce+"\r\n";

		if(sendNow)
			julyHttpAuth->sendData(reqType, "POST /api_trade_v1.php",postData,appendedHeader);
		else
			julyHttpAuth->prepareData(reqType, "POST /api_trade_v1.php",postData,appendedHeader);
	}
	else
	{
		if(julyHttpPublic==0)
		{
			julyHttpPublic=new JulyHttp("vip.btcchina.com","",this,true,true,"application/json-rpc");
			connect(julyHttpPublic,SIGNAL(anyDataReceived()),mainWindow_,SLOT(anyDataReceived()));
			connect(julyHttpPublic,SIGNAL(setDataPending(bool)),mainWindow_,SLOT(setDataPending(bool)));
			connect(julyHttpPublic,SIGNAL(apiDown(bool)),mainWindow_,SLOT(setApiDown(bool)));
			connect(julyHttpPublic,SIGNAL(errorSignal(QString)),mainWindow_,SLOT(showErrorMessage(QString)));
			connect(julyHttpPublic,SIGNAL(sslErrorSignal(const QList<QSslError> &)),this,SLOT(sslErrors(const QList<QSslError> &)));
			connect(julyHttpPublic,SIGNAL(dataReceived(QByteArray,int)),this,SLOT(dataReceivedAuth(QByteArray,int)));
		}
		if(sendNow)
			julyHttpPublic->sendData(reqType, "GET /bc/"+method);
		else
			julyHttpPublic->prepareData(reqType, "GET /bc/"+method);
	}
}

void Exchange_BTCChina::depthSubmitOrder(QMap<double,double> *currentMap ,double priceDouble, double amount, bool isAsk)
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

void Exchange_BTCChina::reloadDepth()
{
	lastDepthBidsMap.clear();
	lastDepthAsksMap.clear();
	lastDepthData.clear();
	forceDepthLoad=true;
}

void Exchange_BTCChina::dataReceivedAuth(QByteArray data, int reqType)
{
	bool success=data.startsWith("{")&&!data.startsWith("{\"error\":")||data.startsWith("[{");
	if(success&&data.startsWith("401"))success=false;
		
	switch(reqType)
	{
	case 103: //ticker
		if(!success)break;
		if(data.startsWith("{\"ticker\":{"))
		{
			QByteArray tickerHigh=getMidData("\"high\":\"","\"",&data);
			if(!tickerHigh.isEmpty())
			{
				double newTickerHigh=tickerHigh.toDouble();
				if(newTickerHigh!=lastTickerHigh)emit tickerHighChanged(newTickerHigh);
				lastTickerHigh=newTickerHigh;
			}

			QByteArray tickerLow=getMidData("\"low\":\"","\"",&data);
			if(!tickerLow.isEmpty())
			{
				double newTickerLow=tickerLow.toDouble();
				if(newTickerLow!=lastTickerLow)emit tickerLowChanged(newTickerLow);
				lastTickerLow=newTickerLow;
			}

			QByteArray tickerVolume=getMidData("\"vol\":\"","\"",&data);
			if(!tickerVolume.isEmpty())
			{
				double newTickerVolume=tickerVolume.toDouble();
				if(newTickerVolume!=lastTickerVolume)emit tickerVolumeChanged(newTickerVolume);
				lastTickerVolume=newTickerVolume;
			}

			QByteArray tickerLast=getMidData("\"last\":\"","\"",&data);
			if(!tickerLast.isEmpty())
			{
				double newTickerLast=tickerLast.toDouble();
				if(newTickerLast!=lastTickerLast)emit tickerLastChanged(newTickerLast);
				lastTickerLast=newTickerLast;
			}

			QByteArray tickerSell=getMidData("\"buy\":\"","\"",&data);
			if(!tickerSell.isEmpty())
			{
				double newTickerSell=tickerSell.toDouble();
				if(newTickerSell!=lastTickerSell)emit tickerSellChanged(newTickerSell);
				lastTickerSell=newTickerSell;
			}

			QByteArray tickerBuy=getMidData("\"sell\":\"","\"",&data);
			if(!tickerBuy.isEmpty())
			{
				double newTickerBuy=tickerBuy.toDouble();
				if(newTickerBuy!=lastTickerBuy)emit tickerBuyChanged(newTickerBuy);
				lastTickerBuy=newTickerBuy;
			}

			if(isFirstTicker)
			{
				emit firstTicker();
				isFirstTicker=false;
			}
		}
		else if(isLogEnabled)logThread->writeLog("Invalid ticker data:"+data);
		break;//ticker
	case 109: //trades
		if(success&&data.size()>32)
		{
			if(data.startsWith("[{"))
			{
				QStringList tradeList=QString(data).split("},{");
				for(int n=0;n<tradeList.count();n++)
				{
					QByteArray tradeData=tradeList.at(n).toAscii();
					QByteArray tradeDate=getMidData("\"date\":\"","\"",&tradeData);
					QByteArray nextFetchDate=tradeDate+getMidData("\"tid\":\"","\"",&tradeData);
					if(nextFetchDate<=lastFetchDate)continue;
					double doubleAmount=getMidData("\"amount\":",",",&tradeData).toDouble();
					double doublePrice=getMidData("\"price\":",",",&tradeData).toDouble();
					if(doubleAmount>0.0&&doublePrice>0.0)
					{
						emit addLastTrade(doubleAmount,tradeDate.toLongLong(),doublePrice,currencySymbol,true);
						if(n==tradeList.count()-1&&!nextFetchDate.isEmpty())lastFetchDate=nextFetchDate;
					}
					else if(isLogEnabled)logThread->writeLog("Invalid trades fetch data line:"+tradeData);
				}
			}
			else if(isLogEnabled)logThread->writeLog("Invalid trades fetch data:"+data);
		}
		break;
	case 111: //bc/orderbook
		if(data.startsWith("{\"asks\":["))
		{
			emit depthUpdateOrder(0.0,0.0,true);
			if(lastDepthData!=data)
			{
				lastDepthData=data;
				
				int bidsStart=data.indexOf("],\"bids\":[");
				if(bidsStart==-1)return;
				QByteArray asksData=data.mid(10,bidsStart-11);
				data.remove(0,bidsStart+11);
				data.remove(data.size()-5,5);

				QMap<double,double> currentAsksMap;
				QStringList asksList=QString(asksData).split("],[");
				double groupedPrice=0.0;
				double groupedVolume=0.0;
				int rowCounter=0;

				for(int n=asksList.count()-1;n>=0;n--)
				{
					if(depthCountLimit&&rowCounter>=depthCountLimit)break;
					QStringList currentRow=asksList.at(n).split(',');
					if(currentRow.count()!=2)continue;
					double priceDouble=currentRow.first().toDouble();
					double amount=currentRow.last().toDouble();

					if(priceDouble>99999)break;

					if(groupPriceValue>0.0)
					{
						if(n==asksList.count()-1)
						{
							emit depthFirstOrder(priceDouble,amount,true);
							groupedPrice=groupPriceValue*(int)(priceDouble/groupPriceValue);
							groupedVolume=amount;
						}
						else
						{
							bool matchCurrentGroup=priceDouble<groupedPrice+groupPriceValue;
							if(matchCurrentGroup)groupedVolume+=amount;
							if(!matchCurrentGroup||n==0)
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
					if(currentAsksMap.value(currentAsksList.at(n),0)==0)emit depthUpdateOrder(currentAsksList.at(n),0.0,true);//Remove price
				lastDepthAsksMap=currentAsksMap;

				QMap<double,double> currentBidsMap;
				QStringList bidsList=QString(data).split("],[");
				groupedPrice=0.0;
				groupedVolume=0.0;
				rowCounter=0;
				for(int n=0;n<bidsList.count();n++)
				{
					if(depthCountLimit&&rowCounter>=depthCountLimit)break;
					QStringList currentRow=bidsList.at(n).split(',');
					if(currentRow.count()!=2)continue;
					double priceDouble=currentRow.first().toDouble();
					double amount=currentRow.last().toDouble();

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
							if(!matchCurrentGroup||n==bidsList.count()-1)
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
					if(currentBidsMap.value(currentBidsList.at(n),0)==0)emit depthUpdateOrder(currentBidsList.at(n),0.0,false);//Remove price
				lastDepthBidsMap=currentBidsMap;
			}
		}
		else if(isLogEnabled)logThread->writeLog("Invalid depth data:"+data);
		break;
	case 202: //AccountInfo
		{
			if(!success)break;
			if(data.startsWith("{\"result\":{\"profile\""))
			{
				lastInfoReceived=true;
				if(isLogEnabled)logThread->writeLog("Info: "+data);

				if(apiLogin.isEmpty())
				{
					QByteArray login=getMidData("username\":\"","\"",&data);
					if(!login.isEmpty())
					{
						apiLogin=login;
						translateUnicodeStr(&apiLogin);
						emit loginChanged(apiLogin);
					}
				}

				QByteArray feeData=getMidData("trade_fee\":\"","\"",&data);
				if(!feeData.isEmpty())emit accFeeChanged(feeData.toDouble());

				QByteArray frozenData=getMidData("\"frozen\":","}}}",&data);

				QByteArray btcBalance=getMidData("symbol\":\"\\u0e3f\",\"amount\":\"","\"",&data);
				if(!btcBalance.isEmpty())
				{
					double newBtcBalance=btcBalance.toDouble()+getMidData("symbol\":\"\\u0e3f\",\"amount\":\"","\"",&frozenData).toDouble();
					if(lastBtcBalance!=newBtcBalance)emit accBtcBalanceChanged(newBtcBalance);
					lastBtcBalance=newBtcBalance;
				}

				QByteArray usdBalance=getMidData("symbol\":\"\\u00a5\",\"amount\":\"","\"",&data);
				if(!usdBalance.isEmpty())
				{
					double newUsdBalance=usdBalance.toDouble()+getMidData("symbol\":\"\\u00a5\",\"amount\":\"","\"",&frozenData).toDouble();
					if(newUsdBalance!=lastUsdBalance)emit accUsdBalanceChanged(newUsdBalance);
					lastUsdBalance=newUsdBalance;
				}
			}
			else if(isLogEnabled)logThread->writeLog("Invalid Info data:"+data);
		}
		break;//balance
	case 204://open_orders
		if(!success)break;
		if(data.startsWith("{\"result\":{\"order\":["))
		{
			if(lastOrders!=data)
			{
				if(data.startsWith("{\"result\":{\"order\":[]}")){lastOrders.clear();emit ordersIsEmpty();break;}

				lastOrders=data;

				if(data.size()>20)
				{
					data.remove(0,20);
					int endOfOrders=data.lastIndexOf(']');
					if(endOfOrders>-1)data.remove(endOfOrders-1,data.size()-endOfOrders-1);
				}

				QStringList ordersList=QString(data).split("},{");

				QList<OrderItem> *orders=new QList<OrderItem>;
				for(int n=0;n<ordersList.count();n++)
				{	
					OrderItem currentOrder;
					QByteArray currentOrderData=ordersList.at(n).toAscii();
					currentOrder.oid=getMidData("\"id\":",",",&currentOrderData);
					currentOrder.date=getMidData("\"date\":",",",&currentOrderData).toUInt();
					currentOrder.type=getMidData("\"type\":\"","\"",&currentOrderData)=="ask";
					QByteArray status=getMidData("\"status\":\"","\"",&currentOrderData);
					//0=Canceled, 1=Open, 2=Pending, 3=Post-Pending
					if(status=="open")currentOrder.status=1;
					else
					if(status=="cancelled")currentOrder.status=0;
					else currentOrder.status=2;

					currentOrder.amount=getMidData("\"amount\":\"","\"",&currentOrderData).toDouble();
					currentOrder.price=getMidData("\"price\":\"","\"",&currentOrderData).toDouble();
					currentOrder.symbol=currencyAStr+getMidData("currency\":\"","\"",&currentOrderData);
					if(currentOrder.isValid())(*orders)<<currentOrder;
				}
				emit ordersChanged(orders);
				lastInfoReceived=false;
			}
		}
		else if(isLogEnabled)logThread->writeLog("Invalid Orders data:"+data);
		break;//open_orders
	case 305: //cancelOrder
		{
			if(!success)break;
			if(!cancelingOrderIDs.isEmpty())
			{
				if(data.startsWith("{\"result\":true"))emit orderCanceled(cancelingOrderIDs.first());
				if(isLogEnabled)logThread->writeLog("Order canceled:"+cancelingOrderIDs.first());
				cancelingOrderIDs.removeFirst();
			}
		}
		break;//cancelOrder
	case 306: //buyOrder
		if(!success||!isLogEnabled)break;
			if(data.startsWith("{\"result\":"))logThread->writeLog("Buy OK: "+data);
			else logThread->writeLog("Invalid Order Buy Data:"+data);
		break;//buyOrder
	case 307: //sellOrder
		if(!success||!isLogEnabled)break;
			if(data.startsWith("{\"result\":"))logThread->writeLog("Sell OK: "+data);
			else logThread->writeLog("Invalid Order Sell Data:"+data);
		break;//sellOrder
	default: break;
	}

	static int errorCount=0;
	if(!success&&reqType!=305)
	{
		errorCount++;
		QString errorString;
		bool invalidMessage=!data.startsWith("{");
		if(!invalidMessage)
			errorString=getMidData("message\":\"","\",",&data)+" Code:"+getMidData("code\":",",",&data);
		else errorString=data;
		if(isLogEnabled)logThread->writeLog("API Error: "+errorString.toAscii());
		if(errorCount<3&&reqType<300&&!errorString.endsWith("Unauthorized"))return;
		if(errorString.isEmpty())return;
		errorString.append("<br>"+QString::number(reqType));
		if(invalidMessage||reqType<300)emit showErrorMessage("I:>"+errorString);
	}
	else errorCount=0;
}

void Exchange_BTCChina::sslErrors(const QList<QSslError> &errors)
{
	QStringList errorList;
	for(int n=0;n<errors.count();n++)errorList<<errors.at(n).errorString();
	if(isLogEnabled)logThread->writeLog(errorList.join(" ").toAscii());
	emit showErrorMessage("SSL Error: "+errorList.join(" "));
}