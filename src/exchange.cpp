// Copyright (C) 2014 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form http://qtopentrader.com
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#include "exchange.h"
#include <QDateTime>
#include "main.h"
#include "depthitem.h"
#include <QFile>

Exchange::Exchange()
	: QThread()
{
	orderBookItemIsDedicatedOrder=false;
	supportsExchangeFee=true;
	supportsExchangeVolume=true;
	clearHistoryOnCurrencyChanged=false;
	exchangeTickerSupportsHiLowPrices=true;
	depthEnabled=true;
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
	QSettings settingsParams(":/Resources/Exchanges/"+currencyMapFile+".ini",QSettings::IniFormat);
	QStringList symbolList=settingsParams.childGroups();
	QList<CurrencyPairItem> *newCurrPairs=new QList<CurrencyPairItem>;

	for(int n=0;n<symbolList.count();n++)
	{
		CurrencyPairItem currentPair=defaultCurrencyParams;
		currentPair.name=settingsParams.value(symbolList.at(n)+"/Symbol","").toByteArray();
		if(currentPair.name.length()!=6)continue;
		currentPair.setSymbol(currentPair.name.toAscii());
		currentPair.name.insert(3,"/");
		currentPair.currRequestSecond=settingsParams.value(symbolList.at(n)+"/RequestSecond","").toByteArray();
		if(!currentPair.currRequestSecond.isEmpty())
			currentPair.name.append(" ["+currentPair.currRequestSecond+"]");
		currentPair.currRequestPair=settingsParams.value(symbolList.at(n)+"/Request","").toByteArray();
		if(currentPair.currRequestPair.isEmpty())continue;
		currentPair.priceDecimals=settingsParams.value(symbolList.at(n)+"/PriceDecimals","").toInt();
		currentPair.priceMin=settingsParams.value(symbolList.at(n)+"/PriceMin","").toDouble();
		currentPair.tradeVolumeMin=settingsParams.value(symbolList.at(n)+"/TradeVolumeMin","").toDouble();
		currentPair.tradePriceMin=settingsParams.value(symbolList.at(n)+"/TradePriceMin","").toDouble();
		(*newCurrPairs)<<currentPair;
	}

	connect(this,SIGNAL(setCurrencyPairsList(QList<CurrencyPairItem>*)),mainClass,SLOT(setCurrencyPairsList(QList<CurrencyPairItem>*)));

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
	connect(this,SIGNAL(tickerSellChanged(double)),mainClass->ui.marketAsk,SLOT(setValue(double)));
	connect(this,SIGNAL(tickerLastChanged(double)),mainClass->ui.marketLast,SLOT(setValue(double)));
	connect(this,SIGNAL(tickerBuyChanged(double)),mainClass->ui.marketBid,SLOT(setValue(double)));
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