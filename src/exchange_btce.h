// Copyright (C) 2013 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form https://sourceforge.net/projects/bitcointrader/
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#ifndef EXCHANGE_BTCE_H
#define EXCHANGE_BTCE_H

#include "exchange.h"

class Exchange_BTCe : public Exchange
{
	Q_OBJECT

public:
	Exchange_BTCe(QByteArray pRestSign, QByteArray pRestKey);
	~Exchange_BTCe();

private:
	void depthUpdateOrder(double,double,bool);
	QList<DepthItem> *depthAsks;
	QList<DepthItem> *depthBids;
	void depthSubmitOrder(QMap<double,double> *currentMap ,double priceDouble, double amount, bool isAsk);
	QMap<double,double> lastDepthAsksMap;
	QMap<double,double> lastDepthBidsMap;
	QByteArray lastDepthData;
	void clearVariables();
	JulyHttp *julyHttp;

	qint64 lastFetchTid;
	bool tickerOnly;
	bool isApiDown;
	QByteArray lastHistory;
	QByteArray lastOrders;
	bool isFirstTicker;
	bool isFirstAccInfo;

	double lastTickerHigh;
	double lastTickerLow;
	double lastTickerSell;
	QByteArray lastTickerDate;
	double lastTickerBuy;
	double lastTickerVolume;

	double lastBtcBalance;
	double lastUsdBalance;
	double lastVolume;
	double lastFee;

	qint64 lastPriceDate;

	QTime authRequestTime;
	int apiDownCounter;
	quint32 privateNonce;

	bool isReplayPending(int);

	void sendToApi(int reqType, QByteArray method, bool auth=false, bool sendNow=true, QByteArray commands=0);
	int lastOpenedOrders;
private slots:
	void reloadDepth();
	void sslErrors(const QList<QSslError> &);
	void dataReceivedAuth(QByteArray,int);
	void secondSlot();
public slots:
	void clearValues();
	void getHistory(bool);
	void buy(double, double);
	void sell(double, double);
	void cancelOrder(QByteArray);
};

#endif // EXCHANGE_BTCE_H
