// Copyright (C) 2014 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form http://qtopentrader.com
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
#include "tradesitem.h"

struct DepthItem;

class Exchange : public QThread
{
	Q_OBJECT

public:
	bool clearHistoryOnCurrencyChanged;
	bool exchangeTickerSupportsHiLowPrices;
	bool depthEnabled;
	virtual void filterAvailableUSDAmountValue(double *amount);

	CurrencyPairItem defaultCurrencyParams;
	bool balanceDisplayAvailableAmount;
	int minimumRequestIntervalAllowed;
	double decAmountFromOpenOrder;
	int calculatingFeeMode;//0: direct multiply; 1: rounded by decimals
	bool buySellAmountExcludedFee;

	CurrencyPairItem currencyPairInfo;

	double lastTickerLast;
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

	QByteArray lastDepthData;
	QByteArray lastHistory;
	QByteArray lastOrders;

	QString currencyMapFile;
	bool isLastTradesTypeSupported;
	bool exchangeSupportsAvailableAmount;
	bool checkDuplicatedOID;
	bool forceDepthLoad;
	bool tickerOnly;
	bool supportsLoginIndicator;
	bool supportsAccountVolume;
	bool supportsExchangeLag;
	bool supportsExchangeFee;
	bool supportsExchangeVolume;

	bool orderBookItemIsDedicatedOrder;

	QTimer *secondTimer;
	QByteArray privateRestSign;
	QByteArray privateRestKey;
	virtual void clearVariables();
	void translateUnicodeStr(QString *str);
	void translateUnicodeOne(QByteArray *str);
	QByteArray getMidData(QString a, QString b,QByteArray *data);
	void setupApi(QtBitcoinTrader *, bool tickerOnly=false);
	Exchange();
	~Exchange();

private:
	void run();

signals:
	void setCurrencyPairsList(QList<CurrencyPairItem>*);

	void availableAmountChanged(double);
	void depthRequested();
	void depthRequestReceived();
	void depthFirstOrder(double,double,bool);

	void depthSubmitOrders(QList<DepthItem> *asks, QList<DepthItem> *bids);

	void addLastTrades(QList<TradesItem> *trades);

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
