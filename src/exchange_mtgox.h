// Copyright (C) 2013 July IGHOR.
// I want to create Bitcoin Trader application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form https://sourceforge.net/projects/bitcointrader/
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#ifndef EXCHANGE_MTGOX_H
#define EXCHANGE_MTGOX_H

#include <QThread>
#include <QNetworkAccessManager>
#include <QTimer>
#include <QTime>
#include "qtbitcointrader.h"

class Exchange_MtGox : public QThread
{
	Q_OBJECT

public:
	void setupApi(QtBitcoinTrader *, bool tickerOnly=false);
	Exchange_MtGox(QByteArray pRestSign, QByteArray pRestKey);
	~Exchange_MtGox();

private:
	QTime authRequestTime;
	QByteArray lastFetchDate;
	bool tickerOnly;
	bool isApiDown;
	void translateUnicodeStr(QString *str);
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

	QString apiLogin;
	QByteArray getMidData(QString a, QString b,QByteArray *data);
	QTime softLagTime;
	int apiDownCounter;
	int secondPart;
	QByteArray privateRestSign;
	QByteArray privateRestKey;
	quint32 privateNonce;

	QMap<QNetworkReply*,int> requestsMap;
	QMap<QNetworkReply*,qint32> timeStampMap;
	void removeReplay(QNetworkReply*);
	bool isReplayPending(int);

	void sendToApi(int reqType, QByteArray method, bool auth=false, QByteArray commands=0, bool incNonce=true);
	QTimer *secondTimer;
	void run();
signals:
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
	void accVolumeChanged(double);
	void accFeeChanged(double);
	void accBtcBalanceChanged(double);
	void accUsdBalanceChanged(double);
	void loginChanged(QString);
	void apiDownChanged(bool);
	void apiLagChanged(double);
	void softLagChanged(int);
private slots:
	void requestFinished(QNetworkReply *);
	void sslErrorsSlot(QNetworkReply *, const QList<QSslError> &);
	void secondSlot();
public slots:
	void clearValues();
	void reloadOrders();
	void getHistory(bool);
	void buy(double, double);
	void sell(double, double);
	void cancelOrder(QByteArray);
};

#endif // EXCHANGE_MTGOX_H
