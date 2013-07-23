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

#include <QThread>
#include "julyhttp.h"
#include <QTimer>
#include <QTime>
#include "qtbitcointrader.h"

class Exchange_BTCe : public QThread
{
	Q_OBJECT

public:
	void setupApi(QtBitcoinTrader *, bool tickerOnly=false);
	Exchange_BTCe(QByteArray pRestSign, QByteArray pRestKey);
	~Exchange_BTCe();

private:
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
	double lastTickerLast;
	double lastTickerBuy;
	double lastTickerVolume;

	double lastBtcBalance;
	double lastUsdBalance;
	double lastVolume;
	double lastFee;

	QByteArray getMidData(QString a, QString b,QByteArray *data);

	qint64 lastPriceDate;

	QTime authRequestTime;
	int apiDownCounter;
	QByteArray privateRestSign;
	QByteArray privateRestKey;
	quint32 privateNonce;

	bool isReplayPending(int);

	void sendToApi(int reqType, QByteArray method, bool auth=false, bool sendNow=true, QByteArray commands=0);
	QTimer *secondTimer;
	int lastOpenedOrders;
	void run();
signals:
	void depthUpdateOrder(double,double,bool);
	void showErrorMessage(QString);
	void addLastTrade(double,qint64,double,QByteArray,bool);

	void ordersChanged(QString);
	void identificationRequired(QString);

	void ordersLogChanged(QString);

	void accLastSellChanged(QByteArray,double);
	void accLastBuyChanged(QByteArray,double);

	void ordersIsEmpty();
	void orderCanceled(QByteArray);

	void firstTicker();
	void tickerHighChanged(double);
	void tickerLowChanged(double);
	void tickerSellChanged(double);
	void tickerLastChanged(double);
	void tickerBuyChanged(double);
	void tickerVolumeChanged(double);

	void firstAccInfo();
	void accFeeChanged(double);
	void accBtcBalanceChanged(double);
	void accUsdBalanceChanged(double);
	void apiDownChanged(bool);
	void apiLagChanged(double);
	void softLagChanged(int);
private slots:
	void sslErrors(const QList<QSslError> &);
	void dataReceivedAuth(QByteArray,int);
	void secondSlot();
public slots:
	void clearValues();
	void reloadOrders();
	void getHistory(bool);
	void buy(double, double);
	void sell(double, double);
	void cancelOrder(QByteArray);
};

#endif // EXCHANGE_BTCE_H
