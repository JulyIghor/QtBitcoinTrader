// Copyright (C) 2013 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form https://sourceforge.net/projects/bitcointrader/
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#ifndef EXCHANGE_MTGOX_H
#define EXCHANGE_MTGOX_H

#include "exchange.h"

class Exchange_MtGox : public Exchange
{
	Q_OBJECT

public:
	Exchange_MtGox(QByteArray pRestSign, QByteArray pRestKey);
	~Exchange_MtGox();

private:
	void depthSubmitOrder(QMap<double,double> *,double,double,bool);
	QTime userInfoTime;
	QMap<double,double> lastDepthAsksMap;
	QMap<double,double> lastDepthBidsMap;
	void clearVariables();
	JulyHttp *julyHttp;
	QTime authRequestTime;
	qint64 tickerLastDate;
	QByteArray lastTradesDateCache;
	qint64 lastTradesDate;
	QByteArray lastDepthData;
	QByteArray lastHistory;
	QByteArray lastOrders;
	bool lastInfoReceived;
	bool isFirstTicker;
	bool isFirstAccInfo;

	double lastTickerHigh;
	double lastTickerLow;
	double lastTickerSell;
	double lastTickerBuy;
	double lastTickerVolume;

	double lastBtcBalance;
	double lastUsdBalance;
	double lastVolume;
	double lastFee;

	QString apiLogin;
	int apiDownCounter;
	int secondPart;
	quint32 privateNonce;

	bool isReplayPending(int);

	void sendToApi(int reqType, QByteArray method, bool auth=false, bool sendNow=true, QByteArray commands=0);

	void depthUpdateOrder(double,double,bool);
	QList<DepthItem> *depthAsks;
	QList<DepthItem> *depthBids;

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

#endif // EXCHANGE_MTGOX_H
