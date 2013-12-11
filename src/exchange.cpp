// Copyright (C) 2013 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form https://sourceforge.net/projects/bitcointrader/
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#include "exchange.h"
#include <QDateTime>
#include "main.h"
#include "depthitem.h"

Exchange::Exchange()
	: QThread()
{
	balanceDisplayAvailableAmount=true;
	minimumRequestIntervalAllowed=100;
	decAmountFromOpenOrder=0.0;
	buySellAmountExcludedFee=false;
	calculatingFeeMode=0;
	supportsLoginIndicator=true;
	supportsAccountVolume=true;
	supportsExchangeLag=true;
	exchangeSupportsAvailableAmount=false;
	checkDuplicatedOID=false;
	isLastTradesTypeSupported=true;
	forceDepthLoad=false;

	clearVariables();
	moveToThread(this);
}

Exchange::~Exchange()
{
	if(debugLevel)logThread->writeLogB(baseValues.exchangeName+" API Thread Deleted",2);
}

QByteArray Exchange::getMidData(QString a, QString b,QByteArray *data)
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

void Exchange::translateUnicodeStr(QString *str)
{
	const QRegExp rx("(\\\\u[0-9a-fA-F]{4})");
	int pos=0;
	while((pos=rx.indexIn(*str,pos))!=-1)str->replace(pos++, 6, QChar(rx.cap(1).right(4).toUShort(0, 16)));
}

void Exchange::translateUnicodeOne(QByteArray *str)
{
	if(!str->contains("\\u"))return;
	QStringList bytesList=QString(*str).split("\\u");
	if(bytesList.count())bytesList.removeFirst();
	else return;
	QString strToReturn;
	for(int n=0;n<bytesList.count();n++)
		if(bytesList.at(n).length()>3)
			strToReturn+="\\u"+bytesList.at(n).left(4);
	translateUnicodeStr(&strToReturn);
	*str=strToReturn.toAscii();
}

void Exchange::run()
{
	if(debugLevel)logThread->writeLogB(baseValues.exchangeName+" API Thread Started",2);
	clearVariables();

	secondTimer=new QTimer;
	secondTimer->setSingleShot(true);
	connect(secondTimer,SIGNAL(timeout()),this,SLOT(secondSlot()));
	secondSlot();
	exec();
}

void Exchange::secondSlot()
{
	if(secondTimer)secondTimer->start(baseValues.httpRequestInterval);
}

void Exchange::dataReceivedAuth(QByteArray, int)
{
}

void Exchange::reloadDepth()
{
	forceDepthLoad=true;
}

void Exchange::clearVariables()
{
	lastTickerLast=0.0;
	lastTickerHigh=0.0;
	lastTickerLow=0.0;
	lastTickerSell=0.0;
	lastTickerBuy=0.0;
	lastTickerVolume=0.0;

	lastBtcBalance=0.0;
	lastUsdBalance=0.0;
	lastAvUsdBalance=0.0;
	lastVolume=0.0;
	lastFee=0.0;
}

void Exchange::filterAvailableUSDAmountValue(double *)
{

}

void Exchange::setupApi(QtBitcoinTrader *mainClass, bool tickOnly)//Execute only once
{
	QFile curMap(":/Resources/"+currencyMapFile);
	curMap.open(QIODevice::ReadOnly);
	QStringList curencyList=QString(curMap.readAll().replace("\r","")).split("\n");
	curMap.close();
	QList<CurrencyPairItem> newCurrPairs;

	for(int n=0;n<curencyList.count();n++)
	{
		QStringList curDataList=curencyList.at(n).split("=");
		if(curDataList.count()!=5)continue;
		CurrencyPairItem currentPair=defaultCurrencyParams;
		currentPair.name=curDataList.first();
		currentPair.setSymbol(curDataList.first().replace("/","").toAscii());
		curDataList.removeFirst();
		currentPair.currRequestPair=curDataList.first().toAscii();
		currentPair.priceDecimals=curDataList.at(1).toInt();
		currentPair.priceMin=qPow(0.1,baseValues.currentPair.priceDecimals);
		currentPair.tradeVolumeMin=curDataList.at(2).toDouble();
		currentPair.tradePriceMin=curDataList.at(3).toDouble();
		newCurrPairs<<currentPair;
	}

	connect(this,SIGNAL(setCurrencyPairsList(QList<CurrencyPairItem>)),mainClass,SLOT(setCurrencyPairsList(QList<CurrencyPairItem>)));

	emit setCurrencyPairsList(newCurrPairs);

	tickerOnly=tickOnly;
	if(!tickerOnly)
	{
		connect(mainClass,SIGNAL(apiBuy(double, double)),this,SLOT(buy(double, double)));
		connect(mainClass,SIGNAL(apiSell(double, double)),this,SLOT(sell(double, double)));
		connect(mainClass,SIGNAL(cancelOrderByOid(QByteArray)),this,SLOT(cancelOrder(QByteArray)));
		connect(this,SIGNAL(ordersChanged(QList<OrderItem> *)),mainClass,SLOT(ordersChanged(QList<OrderItem> *)));
		connect(mainClass,SIGNAL(getHistory(bool)),this,SLOT(getHistory(bool)));
		connect(this,SIGNAL(historyChanged(QList<HistoryItem>*)),mainClass,SLOT(historyChanged(QList<HistoryItem>*)));
		connect(this,SIGNAL(orderCanceled(QByteArray)),mainClass,SLOT(orderCanceled(QByteArray)));
		connect(this,SIGNAL(ordersIsEmpty()),mainClass,SLOT(ordersIsEmpty()));
	}

	connect(this,SIGNAL(depthRequested()),mainClass,SLOT(depthRequested()));
	connect(this,SIGNAL(depthRequestReceived()),mainClass,SLOT(depthRequestReceived()));
	connect(this,SIGNAL(depthSubmitOrders(QList<DepthItem> *, QList<DepthItem> *)),mainClass,SLOT(depthSubmitOrders(QList<DepthItem> *, QList<DepthItem> *)));
	connect(this,SIGNAL(depthFirstOrder(double,double,bool)),mainClass,SLOT(depthFirstOrder(double,double,bool)));
	connect(this,SIGNAL(showErrorMessage(QString)),mainClass,SLOT(showErrorMessage(QString)));

	connect(this,SIGNAL(availableAmountChanged(double)),mainClass,SLOT(availableAmountChanged(double)));
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

	connect(this,SIGNAL(addLastTrades(QList<TradesItem> *)),mainClass,SLOT(addLastTrades(QList<TradesItem> *)));

	start();
}

void Exchange::clearValues()
{

}


void Exchange::getHistory(bool)
{
}

void Exchange::buy(double, double)
{
}

void Exchange::sell(double, double)
{
}

void Exchange::cancelOrder(QByteArray)
{
}

void Exchange::sslErrors(const QList<QSslError> &errors)
{
	QStringList errorList;
	for(int n=0;n<errors.count();n++)errorList<<errors.at(n).errorString();
	if(debugLevel)logThread->writeLog(errorList.join(" ").toAscii(),2);
	emit showErrorMessage("SSL Error: "+errorList.join(" "));
}