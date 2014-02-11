// Copyright (C) 2014 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form http://qtopentrader.com
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
	bool isApiDown;
	bool isFirstAccInfo;
	bool isFirstTicker;
	bool isReplayPending(int);
	bool tickerOnly;

	int apiDownCounter;
	int lastOpenedOrders;

	JulyHttp *julyHttp;

	qint64 lastFetchTid;

	QList<DepthItem> *depthAsks;
	QList<DepthItem> *depthBids;

	QMap<double,double> lastDepthAsksMap;
	QMap<double,double> lastDepthBidsMap;

	QTime authRequestTime;

	quint32 lastPriceDate;
	quint32 lastTickerDate;
	quint32 privateNonce;

	void clearVariables();
	void depthSubmitOrder(QMap<double,double> *currentMap ,double priceDouble, double amount, bool isAsk);
	void depthUpdateOrder(double,double,bool);
	void sendToApi(int reqType, QByteArray method, bool auth=false, bool sendNow=true, QByteArray commands=0);
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
