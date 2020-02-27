//  This file is part of Qt Bitcoin Trader
//      https://github.com/JulyIGHOR/QtBitcoinTrader
//  Copyright (C) 2013-2020 July Ighor <julyighor@gmail.com>
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
#include "julymath.h"
#include "indicatorengine.h"

struct DepthItem;

class Exchange : public QObject
{
    Q_OBJECT

public:
    bool exchangeDisplayOnlyCurrentPairOpenOrders;
    bool clearOpenOrdersOnCurrencyChanged;
    bool clearHistoryOnCurrencyChanged;
    bool exchangeTickerSupportsHiLowPrices;
    bool isDepthEnabled();
    std::atomic_bool depthEnabledFlag;
    virtual void filterAvailableUSDAmountValue(double* amount);

    CurrencyPairItem defaultCurrencyParams;
    bool balanceDisplayAvailableAmount;
    int minimumRequestIntervalAllowed;
    int minimumRequestTimeoutAllowed;
    double decAmountFromOpenOrder;
    int calculatingFeeMode;//0: direct multiply; 1: rounded by decimals; 3: real fee
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
    bool multiCurrencyTradeSupport;
    bool isLastTradesTypeSupported;
    bool exchangeSupportsAvailableAmount;
    bool checkDuplicatedOID;
    bool forceDepthLoad;
    bool tickerOnly;
    bool supportsLoginIndicator;
    bool supportsAccountVolume;
    bool supportsExchangeFee;
    bool supportsExchangeVolume;

    bool orderBookItemIsDedicatedOrder;

    QScopedPointer<QTimer> secondTimer;

    QString domain;
    quint16 port;
    bool    useSsl;

    void setApiKeySecret(QByteArray key, QByteArray secret);

    QByteArray& getApiKey();
    QByteArray getApiSign();

    virtual void clearVariables();
    void translateUnicodeStr(QString* str);
    void translateUnicodeOne(QByteArray* str);
    static QByteArray getMidData(QString a, QString b, QByteArray* data);
    QByteArray getMidVal(QString a, QString b, QByteArray* data);
    void setupApi(QtBitcoinTrader*, bool tickerOnly = false);
    Exchange();
    ~Exchange();

private:
    QByteArray privateKey;

    QList<char*> apiKeyChars;
    QList<char*> apiSignChars;

signals:
    void started();
    void threadFinished();

    void availableAmountChanged(QString, double);
    void depthRequested();
    void depthRequestReceived();
    void depthFirstOrder(QString, double, double, bool);

    void depthSubmitOrders(QString, QList<DepthItem>* asks, QList<DepthItem>* bids);

    void addLastTrades(QString, QList<TradesItem>* trades);

    void orderBookChanged(QString, QList<OrderItem>* orders);
    void showErrorMessage(QString);

    void historyChanged(QList<HistoryItem>*);

    void ordersIsEmpty();
    void orderCanceled(QString, QByteArray);

    void accVolumeChanged(double);
    void accFeeChanged(QString, double);
    void accBtcBalanceChanged(QString, double);
    void accUsdBalanceChanged(QString, double);
    void loginChanged(QString);
    void apiDownChanged(bool);
    void softLagChanged(int);
private slots:
    void sslErrors(const QList<QSslError>&);
    void quitExchange();
public slots:
    virtual void secondSlot();
    virtual void dataReceivedAuth(QByteArray, int);
    virtual void reloadDepth();
    virtual void clearValues();
    virtual void getHistory(bool);
    virtual void buy(QString, double, double);
    virtual void sell(QString, double, double);
    virtual void cancelOrder(QString, QByteArray);

    void run();

protected:
    bool checkValue(QByteArray& valueStr, double& lastValue);
};

#endif // EXCHANGE_H
