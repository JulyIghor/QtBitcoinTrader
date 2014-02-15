//  This file is part of Qt Bitcion Trader
//      https://github.com/JulyIGHOR/QtBitcoinTrader
//  Copyright (C) 2013-2014 July IGHOR <julyighor@gmail.com>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  In addition, as a special exception, the copyright holders give
//  permission to link the code of portions of this program with the
//  OpenSSL library under certain conditions as described in each
//  individual source file, and distribute linked combinations including
//  the two.
//
//  You must obey the GNU General Public License in all respects for all
//  of the code used other than OpenSSL. If you modify file(s) with this
//  exception, you may extend this exception to your version of the
//  file(s), but you are not obligated to do so. If you do not wish to do
//  so, delete this exception statement from your version. If you delete
//  this exception statement from all source files in the program, then
//  also delete it here.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

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
