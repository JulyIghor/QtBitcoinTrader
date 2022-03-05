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

#include "timesync.h"
#include "iniengine.h"
#include "exchange_hitbtc.h"

Exchange_HitBTC::Exchange_HitBTC(const QByteArray &pRestSign, const QByteArray &pRestKey)
    : Exchange(),
      isFirstAccInfo(true),
      lastTickerDate(0),
      lastTradesId(0),
      lastTradesDate(0),
      lastHistoryId(0),
      julyHttp(nullptr),
      depthAsks(nullptr),
      depthBids(nullptr),
      lastDepthAsksMap(),
      lastDepthBidsMap()
{
    clearHistoryOnCurrencyChanged = true;
    calculatingFeeMode = 1;
    baseValues.exchangeName = "HitBTC";
    baseValues.currentPair.name = "BTC/USD";
    baseValues.currentPair.setSymbol("BTC/USD");
    baseValues.currentPair.currRequestPair = "BTCUSD";
    baseValues.currentPair.priceDecimals = 3;
    minimumRequestIntervalAllowed = 500;
    baseValues.currentPair.priceMin = qPow(0.1, baseValues.currentPair.priceDecimals);
    baseValues.currentPair.tradeVolumeMin = 0.01;
    baseValues.currentPair.tradePriceMin = 0.1;
    forceDepthLoad = false;
    tickerOnly = false;
    setApiKeySecret(pRestKey, pRestSign);

    currencyMapFile = "HitBTC";
    defaultCurrencyParams.currADecimals = 8;
    defaultCurrencyParams.currBDecimals = 8;
    defaultCurrencyParams.currABalanceDecimals = 8;
    defaultCurrencyParams.currBBalanceDecimals = 8;
    defaultCurrencyParams.priceDecimals = 3;
    defaultCurrencyParams.priceMin = qPow(0.1, baseValues.currentPair.priceDecimals);

    supportsLoginIndicator = false;
    supportsAccountVolume = false;

    connect(this, &Exchange::threadFinished, this, &Exchange_HitBTC::quitThread, Qt::DirectConnection);
}

Exchange_HitBTC::~Exchange_HitBTC()
{
}

void Exchange_HitBTC::quitThread()
{
    clearValues();

    
        delete depthAsks;

    
        delete depthBids;

    
        delete julyHttp;
}

void Exchange_HitBTC::clearVariables()
{
    isFirstAccInfo = true;
    lastTickerDate = 0;
    lastTradesId = 0;
    lastTradesDate = TimeSync::getMSecs() - 600000;
    lastHistoryId = 0;
    Exchange::clearVariables();
    lastHistory.clear();
    lastOrders.clear();
    reloadDepth();
}

void Exchange_HitBTC::clearValues()
{
    clearVariables();

    if (julyHttp)
        julyHttp->clearPendingData();
}

void Exchange_HitBTC::reloadDepth()
{
    lastDepthBidsMap.clear();
    lastDepthAsksMap.clear();
    lastDepthData.clear();
    Exchange::reloadDepth();
}

void Exchange_HitBTC::dataReceivedAuth(const QByteArray& data, int reqType, int pairChangeCount)
{
    if (pairChangeCount != m_pairChangeCount)
        return;

    if (debugLevel)
        logThread->writeLog("RCV: " + data);

    bool success = !data.startsWith("{\"error\":");
    QString errorString;

    if (!success)
    {
        errorString = getMidData("\"message\":\"", "\"", &data) + "<br>" + getMidData("\"description\":\"", "\"", &data);

        if (debugLevel)
            logThread->writeLog("Invalid data:" + data, 2);
    }
    else switch (reqType)
    {
    case 103: //ticker
        if (data.size() > 10 && getMidData("\"symbol\":\"", "\"", &data) == baseValues.currentPair.currRequestPair)
        {
            double tickerHigh = getMidData("\"high\":\"", "\"", &data).toDouble();

            if (tickerHigh > 0.0 && !qFuzzyCompare(tickerHigh, lastTickerHigh))
            {
                IndicatorEngine::setValue(baseValues.exchangeName, baseValues.currentPair.symbol, "High", tickerHigh);
                lastTickerHigh = tickerHigh;
            }

            double tickerLow = getMidData("\"low\":\"", "\"", &data).toDouble();

            if (tickerLow > 0.0 && !qFuzzyCompare(tickerLow, lastTickerLow))
            {
                IndicatorEngine::setValue(baseValues.exchangeName, baseValues.currentPair.symbol, "Low", tickerLow);
                lastTickerLow = tickerLow;
            }

            double tickerSell = getMidData("\"bid\":\"", "\"", &data).toDouble();

            if (tickerSell > 0.0 && !qFuzzyCompare(tickerSell, lastTickerSell))
            {
                IndicatorEngine::setValue(baseValues.exchangeName, baseValues.currentPair.symbol, "Sell", tickerSell);
                lastTickerSell = tickerSell;
            }

            double tickerBuy = getMidData("\"ask\":\"", "\"", &data).toDouble();

            if (tickerBuy > 0.0 && !qFuzzyCompare(tickerBuy, lastTickerBuy))
            {
                IndicatorEngine::setValue(baseValues.exchangeName, baseValues.currentPair.symbol, "Buy", tickerBuy);
                lastTickerBuy = tickerBuy;
            }

            double tickerVolume = getMidData("\"volume\":\"", "\"", &data).toDouble();

            if (tickerVolume > 0.0 && !qFuzzyCompare(tickerVolume, lastTickerVolume))
            {
                IndicatorEngine::setValue(baseValues.exchangeName, baseValues.currentPair.symbol, "Volume", tickerVolume);
                lastTickerVolume = tickerVolume;
            }

            QDateTime date = QDateTime::fromString(getMidData("\"timestamp\":\"", "\"", &data), Qt::ISODate);
            date.setTimeSpec(Qt::UTC);
            qint64 newTickerDate = date.toMSecsSinceEpoch();

            if (lastTickerDate < newTickerDate)
            {
                lastTickerDate = newTickerDate;
                double tickerLast = getMidData("\"last\":\"", "\"", &data).toDouble();

                if (tickerLast > 0.0 && !qFuzzyCompare(tickerLast, lastTickerLast))
                {
                    IndicatorEngine::setValue(baseValues.exchangeName, baseValues.currentPair.symbol, "Last", tickerLast);
                    lastTickerLast = tickerLast;
                }
            }
        }

        break;//ticker

    case 109: //trades
        if (data.size() > 10)
        {
            qint64 time10Min = TimeSync::getMSecs() - 600000;
            QStringList tradeList = QString(data).split("},{");
            auto* newTradesItems = new QList<TradesItem>;

            for (int n = tradeList.size() - 1; n >= 0; --n)
            {
                QByteArray tradeData = tradeList.at(n).toLatin1();

                QDateTime date = QDateTime::fromString(getMidData("\"timestamp\":\"", "\"", &tradeData), Qt::ISODate);
                date.setTimeSpec(Qt::UTC);
                qint64 dateInt = date.toMSecsSinceEpoch();

                if (dateInt < time10Min)
                    continue;

                lastTradesDate = dateInt;
                qint64 currentTid = getMidData("\"id\":", ",", &tradeData).toLongLong();

                if (currentTid <= lastTradesId)
                    continue;

                lastTradesId = currentTid;

                TradesItem newItem;
                newItem.date      = dateInt / 1000;
                newItem.amount    = getMidData("\"quantity\":\"", "\"", &tradeData).toDouble();
                newItem.price     = getMidData("\"price\":\"",    "\"", &tradeData).toDouble();
                newItem.symbol    = baseValues.currentPair.symbol;
                newItem.orderType = getMidData("\"side\":\"",     "\"", &tradeData) == "buy" ? 1 : -1;

                if (newItem.isValid())
                    (*newTradesItems) << newItem;
                else if (debugLevel)
                    logThread->writeLog("Invalid trades fetch data line:" + tradeData, 2);

                if (n == 0 && dateInt > lastTickerDate && newItem.price > 0.0)
                {
                    lastTickerDate = dateInt;

                    if (!qFuzzyCompare(newItem.price, lastTickerLast))
                    {
                        IndicatorEngine::setValue(baseValues.exchangeName, baseValues.currentPair.symbol, "Last", newItem.price);
                        lastTickerLast = newItem.price;
                    }
                }
            }

            if (!newTradesItems->empty())
                emit addLastTrades(baseValues.currentPair.symbol, newTradesItems);
            else
                delete newTradesItems;
        }
        break;//trades

    case 111: //depth
        if (data.size() > 10)
        {
            emit depthRequestReceived();

            if (data != lastDepthData)
            {
                lastDepthData = data;
                depthAsks = new QList<DepthItem>;
                depthBids = new QList<DepthItem>;

                QMap<double, double> currentAsksMap;
                QStringList asksList = QString(getMidData("\"ask\":[{\"price\":\"", "\"}]", &data)).split("\"},{\"price\":\"");
                double groupedPrice = 0.0;
                double groupedVolume = 0.0;
                int rowCounter = 0;

                for (int n = 0; n < asksList.size(); n++)
                {
                    if (baseValues.depthCountLimit && rowCounter >= baseValues.depthCountLimit)
                        break;

                    QStringList currentPair = asksList.at(n).split("\",\"size\":\"");

                    if (currentPair.size() != 2)
                        continue;

                    double price  = currentPair.first().toDouble();
                    double amount = currentPair.last().toDouble();

                    if (baseValues.groupPriceValue > 0.0)
                    {
                        if (n == 0)
                        {
                            emit depthFirstOrder(baseValues.currentPair.symbol, price, amount, true);
                            groupedPrice = baseValues.groupPriceValue * static_cast<int>(price / baseValues.groupPriceValue);
                            groupedVolume = amount;
                        }
                        else
                        {
                            bool matchCurrentGroup = price < groupedPrice + baseValues.groupPriceValue;

                            if (matchCurrentGroup)
                                groupedVolume += amount;

                            if (!matchCurrentGroup || n == asksList.size() - 1)
                            {
                                depthSubmitOrder(baseValues.currentPair.symbol,
                                                 &currentAsksMap, groupedPrice + baseValues.groupPriceValue, groupedVolume, true);
                                rowCounter++;
                                groupedVolume = amount;
                                groupedPrice += baseValues.groupPriceValue;
                            }
                        }
                    }
                    else
                    {
                        depthSubmitOrder(baseValues.currentPair.symbol, &currentAsksMap, price, amount, true);
                        rowCounter++;
                    }
                }

                QList<double> currentAsksList = lastDepthAsksMap.keys();

                for (int n = 0; n < currentAsksList.size(); n++)
                    if (qFuzzyIsNull(currentAsksMap.value(currentAsksList.at(n), 0)))
                        depthUpdateOrder(baseValues.currentPair.symbol,
                                         currentAsksList.at(n), 0.0, true);

                lastDepthAsksMap = currentAsksMap;

                QMap<double, double> currentBidsMap;
                QStringList bidsList = QString(getMidData("\"bid\":[{\"price\":\"", "\"}]", &data)).split("\"},{\"price\":\"");
                groupedPrice = 0.0;
                groupedVolume = 0.0;
                rowCounter = 0;

                for (int n = 0; n < bidsList.size(); n++)
                {
                    if (baseValues.depthCountLimit && rowCounter >= baseValues.depthCountLimit)
                        break;

                    QStringList currentPair = bidsList.at(n).split("\",\"size\":\"");

                    if (currentPair.size() != 2)
                        continue;

                    double price  = currentPair.first().toDouble();
                    double amount = currentPair.last().toDouble();

                    if (baseValues.groupPriceValue > 0.0)
                    {
                        if (n == 0)
                        {
                            emit depthFirstOrder(baseValues.currentPair.symbol, price, amount, false);
                            groupedPrice = baseValues.groupPriceValue * static_cast<int>(price / baseValues.groupPriceValue);
                            groupedVolume = amount;
                        }
                        else
                        {
                            bool matchCurrentGroup = price > groupedPrice - baseValues.groupPriceValue;

                            if (matchCurrentGroup)
                                groupedVolume += amount;

                            if (!matchCurrentGroup || n == asksList.size() - 1)
                            {
                                depthSubmitOrder(baseValues.currentPair.symbol,
                                                 &currentBidsMap, groupedPrice - baseValues.groupPriceValue, groupedVolume, false);
                                rowCounter++;
                                groupedVolume = amount;
                                groupedPrice -= baseValues.groupPriceValue;
                            }
                        }
                    }
                    else
                    {
                        depthSubmitOrder(baseValues.currentPair.symbol, &currentBidsMap, price, amount, false);
                        rowCounter++;
                    }
                }

                QList<double> currentBidsList = lastDepthBidsMap.keys();

                for (int n = 0; n < currentBidsList.size(); n++)
                    if (qFuzzyIsNull(currentBidsMap.value(currentBidsList.at(n), 0)))
                        depthUpdateOrder(baseValues.currentPair.symbol,
                                         currentBidsList.at(n), 0.0, false);

                lastDepthBidsMap = currentBidsMap;

                emit depthSubmitOrders(baseValues.currentPair.symbol, depthAsks, depthBids);
                depthAsks = nullptr;
                depthBids = nullptr;
            }
        }
        break;

    case 202: //info
        if (data.size() > 10)
        {
            QByteArray btcBalance = getMidData("\"currency\":\"" + baseValues.currentPair.currAStr + "\"", "}", &data);
            QByteArray usdBalance = getMidData("\"currency\":\"" + baseValues.currentPair.currBStr + "\"", "}", &data);
            btcBalance = getMidData("\"available\":\"", "\"", &btcBalance);
            usdBalance = getMidData("\"available\":\"", "\"", &usdBalance);

            if (checkValue(btcBalance, lastBtcBalance))
                emit accBtcBalanceChanged(baseValues.currentPair.symbol, lastBtcBalance);

            if (checkValue(usdBalance, lastUsdBalance))
                emit accUsdBalanceChanged(baseValues.currentPair.symbol, lastUsdBalance);
        }
        break;//info

    case 204://orders
        if (lastOrders != data)
        {
            lastOrders = data;

            if (data == "[]")
            {
                emit ordersIsEmpty();
                break;
            }

            QStringList ordersList = QString(data).split("},{");
            auto* orders = new QList<OrderItem>;

            for (int n = 0; n < ordersList.size(); ++n)
            {
                OrderItem currentOrder;
                QByteArray currentOrderData = ordersList.at(n).toLatin1();

                //0=Canceled, 1=Open, 2=Pending, 3=Post-Pending
                QByteArray status = getMidData("\"status\":\"", "\"", &currentOrderData);

                if (status == "new" || status == "partiallyFilled")
                    currentOrder.status = 1;
                else
                    currentOrder.status = 0;

                QDateTime date = QDateTime::fromString(getMidData("\"createdAt\":\"", "\"", &currentOrderData), Qt::ISODate);
                date.setTimeSpec(Qt::UTC);
                currentOrder.date   = date.toSecsSinceEpoch();
                currentOrder.oid    = getMidData("\"clientOrderId\":\"", "\"", &currentOrderData);
                currentOrder.type   = getMidData("\"side\":\"",          "\"", &currentOrderData) == "sell";
                currentOrder.amount = getMidData("\"quantity\":\"",      "\"", &currentOrderData).toDouble();
                currentOrder.price  = getMidData("\"price\":\"",         "\"", &currentOrderData).toDouble();
                QByteArray request  = getMidData("\"symbol\":\"",        "\"", &currentOrderData);
                QList<CurrencyPairItem>* pairs = IniEngine::getPairs();

                for (int i = 0; i < pairs->size(); ++i)
                {
                    if (pairs->at(i).currRequestPair == request)
                    {
                        currentOrder.symbol = pairs->at(i).symbol;
                        break;
                    }
                }

                if (currentOrder.isValid())
                    (*orders) << currentOrder;
            }

            if (!orders->empty())
                emit orderBookChanged(baseValues.currentPair.symbol, orders);
            else
                delete orders;
        }
        break;//orders

    case 305: //order/cancel
        if (success)
        {
            QByteArray oid = getMidData("\"clientOrderId\":\"", "\"", &data);

            if (!oid.isEmpty())
                emit orderCanceled(baseValues.currentPair.symbol, oid);
        }

        break;//order/cancel

    case 306:
        if (debugLevel)
            logThread->writeLog("Buy OK: " + data, 2);

        break;//order/buy

    case 307:
        if (debugLevel)
            logThread->writeLog("Sell OK: " + data, 2);

        break;//order/sell

    case 208: //history
        if (data.size() > 10)
        {
            if (lastHistory != data)
            {
                lastHistory = data;

                QStringList historyList = QString(data).split("},{");
                qint64 maxId = 0;
                auto* historyItems = new QList<HistoryItem>;

                for (int n = 0; n < historyList.size(); ++n)
                {
                    QByteArray logData(historyList.at(n).toLatin1());

                    qint64 id = getMidData("\"id\":", ",", &logData).toLongLong();

                    if (id <= lastHistoryId)
                        break;

                    if (id > maxId)
                        maxId = id;

                    HistoryItem currentHistoryItem;

                    QByteArray request  = getMidData("\"symbol\":\"", "\"", &logData);
                    QList<CurrencyPairItem>* pairs = IniEngine::getPairs();

                    for (int i = 0; i < pairs->size(); ++i)
                    {
                        if (pairs->at(i).currRequestPair == request)
                        {
                            currentHistoryItem.symbol = pairs->at(i).symbol;
                            break;
                        }
                    }

                    QDateTime date = QDateTime::fromString(getMidData("\"timestamp\":\"", "\"", &logData), Qt::ISODate);
                    date.setTimeSpec(Qt::UTC);
                    currentHistoryItem.dateTimeInt = date.toSecsSinceEpoch();
                    currentHistoryItem.type        = getMidData("\"side\":\"",     "\"", &logData) == "sell" ? 1 : 2;
                    currentHistoryItem.volume      = getMidData("\"quantity\":\"", "\"", &logData).toDouble();
                    currentHistoryItem.price       = getMidData("\"price\":\"",    "\"", &logData).toDouble();

                    if (currentHistoryItem.isValid())
                        (*historyItems) << currentHistoryItem;
                }

                if (maxId > lastHistoryId)
                    lastHistoryId = maxId;

                emit historyChanged(historyItems);
            }
        }
        break;//money/wallet/history

    case 210:
        if (data.size() > 10)
        {
            double fee = qMax(getMidData("\"takeLiquidityRate\":\"",    "\"",  &data).toDouble(),
                              getMidData("\"provideLiquidityRate\":\"", "\"",  &data).toDouble()) * 100;

            if (!qFuzzyIsNull(fee) && !qFuzzyCompare(fee, lastFee))
            {
                emit accFeeChanged(baseValues.currentPair.symbol, fee);
                lastFee = fee;
            }
        }
        break;//order/sell

    default:
        break;
    }

    if (reqType >= 300 && !success)
    {
        if (debugLevel)
            logThread->writeLog("API error: " + errorString.toLatin1() + " ReqType: " + QByteArray::number(reqType), 2);
    }
    else if (reqType >= 200 && reqType < 300)
    {
        static int authErrorCount = 0;

        if (!success)
        {
            authErrorCount++;

            if (authErrorCount < 3)
                return;

            if (debugLevel)
                logThread->writeLog("API error: " + errorString.toLatin1() + " ReqType: " + QByteArray::number(reqType), 2);

            if (!errorString.isEmpty())
                emit showErrorMessage(errorString);
        }
        else
            authErrorCount = 0;
    }
    else if (reqType < 200)
    {
        static int errorCount = 0;

        if (!success)
        {
            errorCount++;

            if (errorCount < 3)
                return;

            if (debugLevel)
                logThread->writeLog("API error: " + errorString.toLatin1() + " ReqType: " + QByteArray::number(reqType), 2);

            if (!errorString.isEmpty())
                emit showErrorMessage("I:>" + errorString);
        }
        else
            errorCount = 0;
    }
}

void Exchange_HitBTC::depthUpdateOrder(const QString& symbol, double price, double amount, bool isAsk)
{
    if (symbol != baseValues.currentPair.symbol)
        return;

    if (isAsk)
    {
        if (depthAsks == nullptr)
            return;

        DepthItem newItem;
        newItem.price = price;
        newItem.volume = amount;

        if (newItem.isValid())
            (*depthAsks) << newItem;
    }
    else
    {
        if (depthBids == nullptr)
            return;

        DepthItem newItem;
        newItem.price = price;
        newItem.volume = amount;

        if (newItem.isValid())
            (*depthBids) << newItem;
    }
}

void Exchange_HitBTC::depthSubmitOrder(const QString& symbol, QMap<double, double>* currentMap, double priceDouble,
                                    double amount, bool isAsk)
{
    if (symbol != baseValues.currentPair.symbol)
        return;

    if (priceDouble == 0.0 || amount == 0.0)
        return;

    if (isAsk)
    {
        (*currentMap)[priceDouble] = amount;

        if (!qFuzzyCompare(lastDepthAsksMap.value(priceDouble, 0.0), amount))
            depthUpdateOrder(symbol, priceDouble, amount, true);
    }
    else
    {
        (*currentMap)[priceDouble] = amount;

        if (!qFuzzyCompare(lastDepthBidsMap.value(priceDouble, 0.0), amount))
            depthUpdateOrder(symbol, priceDouble, amount, false);
    }
}

bool Exchange_HitBTC::isReplayPending(int reqType)
{
    if (julyHttp == nullptr)
        return false;

    return julyHttp->isReqTypePending(reqType);
}

void Exchange_HitBTC::secondSlot()
{
    static int sendCounter = 0;

    switch (sendCounter)
    {
    case 0:
        if (!isReplayPending(103))
            sendToApi(103, "ticker/" + baseValues.currentPair.currRequestPair);

        break;

    case 1:
        if (!isReplayPending(202))
            sendToApi(202, "GET /api/2/trading/balance", true);

        break;

    case 2:
        if (!isReplayPending(109))
            sendToApi(109, "trades/" + baseValues.currentPair.currRequestPair + "?from=" + QByteArray::number(lastTradesDate + 1));

        break;

    case 3:
            if (!tickerOnly && !isReplayPending(204))
                sendToApi(204, "GET /api/2/order", true);

        break;

    case 4:
        if (isDepthEnabled() && (forceDepthLoad || !isReplayPending(111)))
        {
            emit depthRequested();
            sendToApi(111, "orderbook/" + baseValues.currentPair.currRequestPair + "?limit=" + baseValues.depthCountLimitStr);
            forceDepthLoad = false;
        }
        break;

    case 5:
        if (lastHistory.isEmpty())
            getHistory(false);

        break;

    default:
        break;
    }

    if (sendCounter++ >= 5)
        sendCounter = 0;

    Exchange::secondSlot();
}

void Exchange_HitBTC::getHistory(bool force)
{
    if (tickerOnly)
        return;

    if (force)
        lastHistory.clear();

    if (!isReplayPending(208))
        sendToApi(208, "GET /api/2/history/trades?symbol=" + baseValues.currentPair.currRequestPair +
                  "&from=" + QByteArray::number(lastHistoryId + 1), true);

    if (!isReplayPending(210))
        sendToApi(210, "GET /api/2/trading/fee/" + baseValues.currentPair.currRequestPair, true);
}

void Exchange_HitBTC::buy(const QString& symbol, double apiBtcToBuy, double apiPriceToBuy)
{
    if (tickerOnly)
        return;

    CurrencyPairItem pairItem;
    pairItem = baseValues.currencyPairMap.value(symbol, pairItem);

    if (pairItem.symbol.isEmpty())
        return;

    QByteArray data = "symbol=" + pairItem.currRequestPair + "&side=buy&quantity=" +
            JulyMath::byteArrayFromDouble(apiBtcToBuy, pairItem.currADecimals, 0) + "&price=" +
            JulyMath::byteArrayFromDouble(apiPriceToBuy, pairItem.priceDecimals, 0);

    if (debugLevel)
        logThread->writeLog("Buy: " + data, 2);

    sendToApi(306, "POST /api/2/order", true, data);
}

void Exchange_HitBTC::sell(const QString& symbol, double apiBtcToSell, double apiPriceToSell)
{
    if (tickerOnly)
        return;

    CurrencyPairItem pairItem;
    pairItem = baseValues.currencyPairMap.value(symbol, pairItem);

    if (pairItem.symbol.isEmpty())
        return;

    QByteArray data = "symbol=" + pairItem.currRequestPair + "&side=sell&quantity=" +
            JulyMath::byteArrayFromDouble(apiBtcToSell, pairItem.currADecimals, 0) + "&price=" +
            JulyMath::byteArrayFromDouble(apiPriceToSell, pairItem.priceDecimals, 0);

    if (debugLevel)
        logThread->writeLog("Sell: " + data, 2);

    sendToApi(307, "POST /api/2/order", true, data);
}

void Exchange_HitBTC::cancelOrder(const QString& /*unused*/, const QByteArray& order)
{
    if (tickerOnly)
        return;

    if (debugLevel)
        logThread->writeLog("Cancel order: " + order, 2);

    sendToApi(305, "DELETE /api/2/order/" + order, true);
}

void Exchange_HitBTC::sendToApi(int reqType, const QByteArray &method, bool auth, QByteArray commands)
{
    if (julyHttp == nullptr)
    {
        if (domain.isEmpty() || port == 0)
            julyHttp = new JulyHttp("api.hitbtc.com", "", this);
        else
        {
            julyHttp = new JulyHttp(domain, "", this, useSsl);
            julyHttp->setPortForced(port);
        }

        connect(julyHttp, &JulyHttp::anyDataReceived, baseValues_->mainWindow_, &QtBitcoinTrader::anyDataReceived);
        connect(julyHttp, &JulyHttp::apiDown, baseValues_->mainWindow_, &QtBitcoinTrader::setApiDown);
        connect(julyHttp, &JulyHttp::setDataPending, baseValues_->mainWindow_, &QtBitcoinTrader::setDataPending);
        connect(julyHttp, &JulyHttp::errorSignal, baseValues_->mainWindow_, &QtBitcoinTrader::showErrorMessage);
        connect(julyHttp, &JulyHttp::sslErrorSignal, this, &Exchange::sslErrors);
        connect(julyHttp, &JulyHttp::dataReceived, this, &Exchange::dataReceivedAuth);
    }

    if (auth)
        julyHttp->sendData(reqType, m_pairChangeCount, method, commands,
                           "Authorization: Basic " + (getApiKey() + ':' + getApiSign()).toBase64() + "\r\n");
    else
        julyHttp->sendData(reqType, m_pairChangeCount, "GET /api/2/public/" + method);
}

void Exchange_HitBTC::sslErrors(const QList<QSslError>& errors)
{
    QStringList errorList;

    for (int n = 0; n < errors.size(); n++)
        errorList << errors.at(n).errorString();

    if (debugLevel)
        logThread->writeLog(errorList.join(" ").toLatin1(), 2);

    emit showErrorMessage("SSL Error: " + errorList.join(" "));
}
