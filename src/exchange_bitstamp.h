// Copyright (C) 2014 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form http://qtopentrader.com
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
	void filterAvailableUSDAmountValue(double *amount);
	Exchange_Bitstamp(QByteArray pRestSign, QByteArray pRestKey);
	~Exchange_Bitstamp();

private:
	double accountFee;
	bool isFirstTicker;
	bool isReplayPending(int);
	bool lastInfoReceived;
	bool tickerOnly;

	int apiDownCounter;
	int secondPart;

	JulyHttp *julyHttp;

	QByteArray privateClientId;

	QList<DepthItem> *depthAsks;
	QList<DepthItem> *depthBids;
	QList<QByteArray> cancelingOrderIDs;

	QMap<double,double> lastDepthAsksMap;
	QMap<double,double> lastDepthBidsMap;
	QString apiLogin;

	QTime authRequestTime;
	QTimer *secondTimer;

	quint32 lastBidAskTimestamp;
	quint32 lastTickerDate;
	quint32 lastTradesDate;
	quint32 privateNonce;

	void clearVariables();
	void depthSubmitOrder(QMap<double,double> *,double,double,bool);
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

#endif // EXCHANGE_BITSTAMP_H
