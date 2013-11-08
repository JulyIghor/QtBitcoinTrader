// Copyright (C) 2013 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form https://sourceforge.net/projects/bitcointrader/
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#ifndef EXCHANGE_H
#define EXCHANGE_H

#include <QThread>
#include <QTimer>
#include <QTime>
#include <openssl/hmac.h>
#include "main.h"
#include <QtCore/qmath.h>
#include "qtbitcointrader.h"
#include "julyhttp.h"
#include "orderitem.h"
#include "depthitem.h"

class Exchange : public QThread
{
	Q_OBJECT

public:
	QByteArray exchangeID;
	bool forceDepthLoad;
	bool tickerOnly;
	QTimer *secondTimer;
	QByteArray privateRestSign;
	QByteArray privateRestKey;
	virtual void clearVariables();
	void translateUnicodeStr(QString *str);
	QByteArray getMidData(QString a, QString b,QByteArray *data);
	void setupApi(QtBitcoinTrader *, bool tickerOnly=false);
	Exchange();
	~Exchange();

private:
	void run();

signals:
	void availableAmountChanged(double);
	void depthRequested();
	void depthRequestReceived();
	void depthFirstOrder(double,double,bool);

	void depthSubmitOrders(QList<DepthItem> *asks, QList<DepthItem> *bids);

	void addLastTrade(double,qint64,double,QByteArray,bool);

	void ordersChanged(QList<OrderItem> *orders);
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
	void sslErrors(const QList<QSslError> &);
public slots:
	virtual void secondSlot();
	virtual void dataReceivedAuth(QByteArray,int);
	virtual void reloadDepth();
	virtual void clearValues();
	virtual void getHistory(bool);
	virtual void buy(double, double);
	virtual void sell(double, double);
	virtual void cancelOrder(QByteArray);
};

#endif // EXCHANGE_H
