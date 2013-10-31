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

#include <QThread>
#include <QTimer>
#include <QTime>
#include "qtbitcointrader.h"
#include "julyhttp.h"

class Exchange_BTCChina : public QThread
{
	Q_OBJECT

public:
	void setupApi(QtBitcoinTrader *, bool tickerOnly=false);
	Exchange_BTCChina(QByteArray pRestSign, QByteArray pRestKey);
	~Exchange_BTCChina();

private:
	QByteArray numForSellFromDouble(double val, int maxDecimals);
	void translateUnicodeStr(QString *str);
	QList<QByteArray> cancelingOrderIDs;
	bool forceDepthLoad;
	void depthSubmitOrder(QMap<double,double> *,double,double,bool);
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
	QByteArray privateRestSign;
	QByteArray privateRestKey;
	quint32 privateTonce;
	quint32 tonceCounter;

	bool isReplayPending(int);

	void sendToApi(int reqType, QByteArray method, bool auth=false, bool sendNow=true, QByteArray commands=0);
	QTimer *secondTimer;
	void run();
signals:
	void depthFirstOrder(double,double,bool);
	void depthUpdateOrder(double,double,bool);

	void addLastTrade(double,qint64,double,QByteArray,bool);

	void ordersChanged(QList<OrderItem> *);
	void showErrorMessage(QString);

	void historyChanged(QList<HistoryItem>*);

	void ordersIsEmpty();
	void orderCanceled(QByteArray);

	void firstTicker();
	void tickerHighChanged(double);
	void tickerLowChanged(double);
	void tickerSellChanged(double);
	void tickerLastChanged(double);
	void tickerBuyChanged(double);
	void tickerVolumeChanged(double);

	void accVolumeChanged(double);
	void accFeeChanged(double);
	void accBtcBalanceChanged(double);
	void accUsdBalanceChanged(double);
	void loginChanged(QString);
	void apiDownChanged(bool);
	void apiLagChanged(double);
	void softLagChanged(int);
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
