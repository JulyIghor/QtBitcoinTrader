//  This file is part of Qt Bitcoin Trader
//      https://github.com/JulyIGHOR/QtBitcoinTrader
//  Copyright (C) 2013-2022 July Ighor <julyighor@gmail.com>
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

#include "indicatorengine.h"
#include "julyhttp.h"
#include "julymath.h"
#include "main.h"
#include "orderitem.h"
#include "qtbitcointrader.h"
#include "tradesitem.h"
#include <QThread>
#include <QTime>
#include <QTimer>
#include <QtCore/qmath.h>
#include <openssl/hmac.h>

struct DepthItem;

class Exchange : public QObject
{
    Q_OBJECT

public:
    bool exchangeDisplayOnlyCurrentPairOpenOrders;
    bool clearOpenOrdersOnCurrencyChanged;
    bool clearHistoryOnCurrencyChanged;
    bool exchangeTickerSupportsHiLowPrices;
    bool isDepthEnabled() const;
    std::atomic_bool depthEnabledFlag{};
    virtual void filterAvailableUSDAmountValue(double* amount);

    CurrencyPairItem defaultCurrencyParams;
    bool balanceDisplayAvailableAmount;
    int minimumRequestIntervalAllowed;
    int minimumRequestTimeoutAllowed;
    double decAmountFromOpenOrder;
    int calculatingFeeMode; // 0: direct multiply; 1: rounded by decimals; 3: real fee
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
    bool useSsl;

    int m_pairChangeCount;

    void setApiKeySecret(const QByteArray& key, const QByteArray& secret);

    QByteArray& getApiKey();
    QByteArray getApiSign();

    virtual void clearVariables();
    static QByteArray getMidData(const QString& a, const QString& b, const QByteArray* data);
    QByteArray getMidVal(const QString& a, const QString& b, const QByteArray* data);
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
    void quitExchange();
public slots:
    void sslErrors(const QList<QSslError>&);
    virtual void secondSlot();
    virtual void dataReceivedAuth(const QByteArray&, int, int);
    virtual void reloadDepth();
    virtual void clearValues();
    virtual void getHistory(bool);
    virtual void buy(const QString&, double, double);
    virtual void sell(const QString&, double, double);
    virtual void cancelOrder(const QString&, const QByteArray&);

    void run();

protected:
    bool checkValue(QByteArray& valueStr, double& lastValue);
};

#endif // EXCHANGE_H
