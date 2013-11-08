// Copyright (C) 2013 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form https://sourceforge.net/projects/bitcointrader/
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#ifndef EXCHANGE_BITSTAMP_H
#define EXCHANGE_BITSTAMP_H

#include "exchange.h"

class Exchange_Bitstamp : public Exchange
{
	Q_OBJECT

public:
	Exchange_Bitstamp(QByteArray pRestSign, QByteArray pRestKey);
	~Exchange_Bitstamp();

private:
	void depthUpdateOrder(double,double,bool);
	QList<DepthItem> *depthAsks;
	QList<DepthItem> *depthBids;
	QList<QByteArray> cancelingOrderIDs;
	void depthSubmitOrder(QMap<double,double> *,double,double,bool);
	QMap<double,double> lastDepthAsksMap;
	QMap<double,double> lastDepthBidsMap;
	void clearVariables();
	JulyHttp *julyHttp;
	QTime authRequestTime;
	QByteArray lastFetchDate;
	QByteArray lastTickerDate;
	QByteArray lastDepthData;
	QByteArray lastBidAskTimestamp;
	bool tickerOnly;
	QByteArray lastHistory;
	QByteArray lastOrders;
	bool lastInfoReceived;
	bool isFirstTicker;

	double lastTickerHigh;
	double lastTickerLow;
	double lastTickerSell;
	double lastTickerBuy;
	double lastTickerVolume;

	double lastBtcBalance;
	double lastUsdBalance;
	double lastAvUsdBalance;
	double lastVolume;
	double lastFee;

	QString apiLogin;
	int apiDownCounter;
	int secondPart;
	QByteArray privateClientId;
	quint32 privateNonce;

	bool isReplayPending(int);

	void sendToApi(int reqType, QByteArray method, bool auth=false, bool sendNow=true, QByteArray commands=0);
	QTimer *secondTimer;
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

#endif // EXCHANGE_BITSTAMP_H
