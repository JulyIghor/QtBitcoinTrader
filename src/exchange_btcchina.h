// Copyright (C) 2013 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form https://sourceforge.net/projects/bitcointrader/
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
	QByteArray numForBuySellFromDouble(double val, int maxDecimals);
	void depthUpdateOrder(double,double,bool);
	QList<DepthItem> *depthAsks;
	QList<DepthItem> *depthBids;
	void depthSubmitOrder(QMap<double,double> *,double,double,bool);
	QList<QByteArray> cancelingOrderIDs;
	QMap<double,double> lastDepthAsksMap;
	QMap<double,double> lastDepthBidsMap;
	void clearVariables();
	JulyHttp *julyHttpAuth;
	JulyHttp *julyHttpPublic;
	QTime authRequestTime;
	QByteArray lastFetchDate;
	QByteArray lastDepthData;
	bool tickerOnly;
	QByteArray lastHistory;
	QByteArray lastOrders;
	bool lastInfoReceived;
	bool isFirstTicker;

	double lastTickerHigh;
	double lastTickerLow;
	double lastTickerSell;
	double lastTickerLast;
	double lastTickerBuy;
	double lastTickerVolume;

	double lastBtcBalance;
	double lastUsdBalance;
	double lastVolume;
	double lastFee;

	QString apiLogin;
	QByteArray getMidData(QString a, QString b,QByteArray *data);
	int apiDownCounter;
	int secondPart;
	quint32 privateTonce;
	quint32 tonceCounter;

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

#endif // EXCHANGE_BTCCHINA_H
