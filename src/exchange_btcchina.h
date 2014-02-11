// Copyright (C) 2014 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form http://qtopentrader.com
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#ifndef EXCHANGE_BTCCHINA_H
#define EXCHANGE_BTCCHINA_H

#include "exchange.h"

class Exchange_BTCChina : public Exchange
{
	Q_OBJECT

public:
	Exchange_BTCChina(QByteArray pRestSign, QByteArray pRestKey);
	~Exchange_BTCChina();

private:
	bool isFirstTicker;
	bool isReplayPending(int);
	bool lastInfoReceived;
	bool tickerOnly;

	int apiDownCounter;
	int secondPart;

	JulyHttp *julyHttpAuth;
	JulyHttp *julyHttpPublic;

	QByteArray lastFetchTid;
	QByteArray getMidData(QString a, QString b,QByteArray *data);
	QByteArray numForBuySellFromDouble(double val, int maxDecimals);

	QList<DepthItem> *depthAsks;
	QList<DepthItem> *depthBids;
	QList<QByteArray> cancelingOrderIDs;

	QMap<double,double> lastDepthAsksMap;
	QMap<double,double> lastDepthBidsMap;

	QString apiLogin;

	QTime authRequestTime;
	QTimer *secondTimer;

	quint32 privateTonce;
	quint32 tonceCounter;

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

#endif // EXCHANGE_BTCCHINA_H
