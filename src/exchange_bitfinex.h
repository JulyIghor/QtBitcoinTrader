// Copyright (C) 2014 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form http://qtopentrader.com
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#ifndef EXCHANGE_BITFINEX_H
#define EXCHANGE_BITFINEX_H

#include "exchange.h"

class Exchange_Bitfinex : public Exchange
{
	Q_OBJECT

public:
	Exchange_Bitfinex(QByteArray pRestSign, QByteArray pRestKey);
	~Exchange_Bitfinex();

private:
	QByteArray historyLastTimestamp;

	bool isFirstAccInfo;
	bool isFirstTicker;
	bool isReplayPending(int);
	bool lastInfoReceived;

	int apiDownCounter;
	int secondPart;

	JulyHttp *julyHttp;

	QByteArray lastTradesDateCache;

	quint64 lastTradesDate;
	quint32 tickerLastDate;

	QList<DepthItem> *depthAsks;
	QList<DepthItem> *depthBids;

	QMap<double,double> lastDepthAsksMap;
	QMap<double,double> lastDepthBidsMap;

	QString apiLogin;

	QTime authRequestTime;

	quint32 privateNonce;

	void clearVariables();
	void depthSubmitOrder(QMap<double,double> *,double,double,bool);
	void depthUpdateOrder(double,double,bool);
	void sendToApi(int reqType, QByteArray method, bool auth=false, bool sendNow=true, QByteArray commands=0);
private slots:
	void secondSlot();
public slots:
	void dataReceivedAuth(QByteArray,int);
	void reloadDepth();
	void clearValues();
	void getHistory(bool);
	void buy(double, double);
	void sell(double, double);
	void cancelOrder(QByteArray);
};

#endif // EXCHANGE_BITFINEX_H
