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

#include "exchange_binance.h"
#include "iniengine.h"
#include "timesync.h"

Exchange_Binance::Exchange_Binance(const QByteArray& pRestSign, const QByteArray& pRestKey) :
    Exchange(),
    isFirstAccInfo(true),
    isValidApiKey(true),
    sslErrorCounter(0),
    lastTickerId(0),
    lastTradesId(0),
    lastHistoryId(0),
    julyHttp(nullptr),
    depthAsks(nullptr),
    depthBids(nullptr),
    lastDepthAsksMap(),
    lastDepthBidsMap()
{
    clearOpenOrdersOnCurrencyChanged = true;
    clearHistoryOnCurrencyChanged = true;
    baseValues.feeDecimals = 3;
    calculatingFeeMode = 1;
    baseValues.exchangeName = "Binance";
    baseValues.currentPair.name = "BTC/USD";
    baseValues.currentPair.setSymbol("BTCUSD");
    baseValues.currentPair.currRequestPair = "btc_usd";
    baseValues.currentPair.priceDecimals = 3;
    minimumRequestIntervalAllowed = 600;
    baseValues.currentPair.priceMin = qPow(0.1, baseValues.currentPair.priceDecimals);
    baseValues.currentPair.tradeVolumeMin = 0.01;
    baseValues.currentPair.tradePriceMin = 0.1;
    forceDepthLoad = false;
    tickerOnly = false;
    setApiKeySecret(pRestKey, pRestSign);

    currencyMapFile = "Binance";
    defaultCurrencyParams.currADecimals = 8;
    defaultCurrencyParams.currBDecimals = 8;
    defaultCurrencyParams.currABalanceDecimals = 8;
    defaultCurrencyParams.currBBalanceDecimals = 8;
    defaultCurrencyParams.priceDecimals = 3;
    defaultCurrencyParams.priceMin = qPow(0.1, baseValues.currentPair.priceDecimals);

    supportsLoginIndicator = false;
    supportsAccountVolume = false;

    connect(this, &Exchange::threadFinished, this, &Exchange_Binance::quitThread, Qt::DirectConnection);
}

Exchange_Binance::~Exchange_Binance()
{
}

void Exchange_Binance::quitThread()
{
    clearValues();

    delete depthAsks;

    delete depthBids;

    delete julyHttp;
}

void Exchange_Binance::clearVariables()
{
    isFirstAccInfo = true;
    isValidApiKey = true;
    lastTickerId = 0;
    lastTradesId = 0;
    lastHistoryId = 0;
    Exchange::clearVariables();
    lastHistory.clear();
    lastOrders.clear();
    reloadDepth();
}

void Exchange_Binance::clearValues()
{
    clearVariables();

    if (julyHttp)
        julyHttp->clearPendingData();
}

void Exchange_Binance::reloadDepth()
{
    lastDepthBidsMap.clear();
    lastDepthAsksMap.clear();
    lastDepthData.clear();
    Exchange::reloadDepth();
}

void Exchange_Binance::dataReceivedAuth(const QByteArray& data, int reqType, int pairChangeCount)
{
    if (pairChangeCount != m_pairChangeCount)
        return;

    sslErrorCounter = 0;

    if (debugLevel)
        logThread->writeLog("RCV: " + data);

    if (data.size() && data.at(0) == QLatin1Char('<'))
        return;

    bool success = !data.startsWith("{\"success\":0");
    QString errorString;

    if (!success)
        errorString = getMidData("msg\":\"", "\"", &data);

    if (isValidApiKey)
        if (data == "{\"code\":-2015,\"msg\":\"Invalid API-key, IP, or permissions for action.\"}" ||
            data == "{\"code\":-2014,\"msg\":\"API-key format invalid.\"}")
            isValidApiKey = false;

    switch (reqType)
    {
    case 103: // ticker
        {
            double tickerHigh = getMidData("\"highPrice\":\"", "\"", &data).toDouble();

            if (tickerHigh > 0.0 && !qFuzzyCompare(tickerHigh, lastTickerHigh))
            {
                IndicatorEngine::setValue(baseValues.exchangeName, baseValues.currentPair.symbol, "High", tickerHigh);
                lastTickerHigh = tickerHigh;
            }

            double tickerLow = getMidData("\"lowPrice\":\"", "\"", &data).toDouble();

            if (tickerLow > 0.0 && !qFuzzyCompare(tickerLow, lastTickerLow))
            {
                IndicatorEngine::setValue(baseValues.exchangeName, baseValues.currentPair.symbol, "Low", tickerLow);
                lastTickerLow = tickerLow;
            }

            double tickerSell = getMidData("\"bidPrice\":\"", "\"", &data).toDouble();

            if (tickerSell > 0.0 && !qFuzzyCompare(tickerSell, lastTickerSell))
            {
                IndicatorEngine::setValue(baseValues.exchangeName, baseValues.currentPair.symbol, "Sell", tickerSell);
                lastTickerSell = tickerSell;
            }

            double tickerBuy = getMidData("\"askPrice\":\"", "\"", &data).toDouble();

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

            qint64 tickerId = getMidData("\"lastId\":", ",", &data).toLongLong();

            if (tickerId > lastTickerId)
            {
                lastTickerId = tickerId;
                double tickerLastDouble = getMidData("\"lastPrice\":\"", "\"", &data).toDouble();

                if (tickerLastDouble > 0.0 && !qFuzzyCompare(tickerLastDouble, lastTickerLast))
                {
                    IndicatorEngine::setValue(baseValues.exchangeName, baseValues.currentPair.symbol, "Last", tickerLastDouble);
                    lastTickerLast = tickerLastDouble;
                }
            }
        }
        break; // ticker

    case 109: // trades
        {
            if (data.size() < 10)
                break;

            qint64 time10Min = TimeSync::getTimeT() - 600;
            QStringList tradeList = QString(data).split("},{");
            auto* newTradesItems = new QList<TradesItem>;
            int lastIndex = tradeList.size() - 1;

            for (int n = 0; n < tradeList.size(); ++n)
            {
                QByteArray tradeData = tradeList.at(n).toLatin1() + "}";
                TradesItem newItem;
                qint64 currentTid = getMidData("\"id\":", ",", &tradeData).toLongLong();

                if (currentTid <= lastTradesId)
                    continue;

                lastTradesId = currentTid;
                newItem.date = getMidData("\"time\":", ",", &tradeData).toLongLong() / 1000;

                if (newItem.date < time10Min)
                    continue;

                newItem.price = getMidData("\"price\":\"", "\"", &tradeData).toDouble();

                if (n == lastIndex && currentTid > lastTickerId)
                {
                    lastTickerId = currentTid;

                    if (newItem.price > 0.0 && !qFuzzyCompare(newItem.price, lastTickerLast))
                    {
                        IndicatorEngine::setValue(baseValues.exchangeName, baseValues.currentPair.symbol, "Last", newItem.price);
                        lastTickerLast = newItem.price;
                    }
                }

                newItem.amount = getMidData("\"qty\":\"", "\"", &tradeData).toDouble();
                newItem.symbol = baseValues.currentPair.symbol;
                newItem.orderType = getMidData("\"isBuyerMaker\":", ",", &tradeData) == "true" ? 1 : -1;

                if (newItem.isValid())
                    (*newTradesItems) << newItem;
                else if (debugLevel)
                    logThread->writeLog("Invalid trades fetch data line:" + tradeData, 2);
            }

            if (!newTradesItems->empty())
                emit addLastTrades(baseValues.currentPair.symbol, newTradesItems);
            else
                delete newTradesItems;
        }
        break; // trades

    case 111: // depth
        if (data.startsWith("{\"lastUpdateId\":"))
        {
            emit depthRequestReceived();
            QByteArray lastUpdateId = getMidData("\"lastUpdateId\":", ",", &data);

            if (lastUpdateId != lastDepthData)
            {
                lastDepthData = lastUpdateId;
                depthAsks = new QList<DepthItem>;
                depthBids = new QList<DepthItem>;

                QMap<double, double> currentAsksMap;
                QStringList asksList = QString(getMidData("\"asks\":[[\"", "\"]]", &data)).split("\"],[\"");
                double groupedPrice = 0.0;
                double groupedVolume = 0.0;
                int rowCounter = 0;

                for (int n = 0; n < asksList.size(); n++)
                {
                    if (baseValues.depthCountLimit && rowCounter >= baseValues.depthCountLimit)
                        break;

                    QStringList currentPair = asksList.at(n).split("\",\"");

                    if (currentPair.size() != 2)
                        continue;

                    double priceDouble = currentPair.first().toDouble();
                    double amount = currentPair.last().toDouble();

                    if (baseValues.groupPriceValue > 0.0)
                    {
                        if (n == 0)
                        {
                            emit depthFirstOrder(baseValues.currentPair.symbol, priceDouble, amount, true);
                            groupedPrice = baseValues.groupPriceValue * static_cast<int>(priceDouble / baseValues.groupPriceValue);
                            groupedVolume = amount;
                        }
                        else
                        {
                            bool matchCurrentGroup = priceDouble < groupedPrice + baseValues.groupPriceValue;

                            if (matchCurrentGroup)
                                groupedVolume += amount;

                            if (!matchCurrentGroup || n == asksList.size() - 1)
                            {
                                depthSubmitOrder(baseValues.currentPair.symbol,
                                                 &currentAsksMap,
                                                 groupedPrice + baseValues.groupPriceValue,
                                                 groupedVolume,
                                                 true);
                                rowCounter++;
                                groupedVolume = amount;
                                groupedPrice += baseValues.groupPriceValue;
                            }
                        }
                    }
                    else
                    {
                        depthSubmitOrder(baseValues.currentPair.symbol, &currentAsksMap, priceDouble, amount, true);
                        rowCounter++;
                    }
                }

                QList<double> currentAsksList = lastDepthAsksMap.keys();

                for (int n = 0; n < currentAsksList.size(); n++)
                    if (qFuzzyIsNull(currentAsksMap.value(currentAsksList.at(n), 0)))
                        depthUpdateOrder(baseValues.currentPair.symbol, currentAsksList.at(n), 0.0, true);

                lastDepthAsksMap = currentAsksMap;

                QMap<double, double> currentBidsMap;
                QStringList bidsList = QString(getMidData("\"bids\":[[\"", "\"]]", &data)).split("\"],[\"");
                groupedPrice = 0.0;
                groupedVolume = 0.0;
                rowCounter = 0;

                for (int n = 0; n < bidsList.size(); n++)
                {
                    if (baseValues.depthCountLimit && rowCounter >= baseValues.depthCountLimit)
                        break;

                    QStringList currentPair = bidsList.at(n).split("\",\"");

                    if (currentPair.size() != 2)
                        continue;

                    double priceDouble = currentPair.first().toDouble();
                    double amount = currentPair.last().toDouble();

                    if (baseValues.groupPriceValue > 0.0)
                    {
                        if (n == 0)
                        {
                            emit depthFirstOrder(baseValues.currentPair.symbol, priceDouble, amount, false);
                            groupedPrice = baseValues.groupPriceValue * static_cast<int>(priceDouble / baseValues.groupPriceValue);
                            groupedVolume = amount;
                        }
                        else
                        {
                            bool matchCurrentGroup = priceDouble > groupedPrice - baseValues.groupPriceValue;

                            if (matchCurrentGroup)
                                groupedVolume += amount;

                            if (!matchCurrentGroup || n == asksList.size() - 1)
                            {
                                depthSubmitOrder(baseValues.currentPair.symbol,
                                                 &currentBidsMap,
                                                 groupedPrice - baseValues.groupPriceValue,
                                                 groupedVolume,
                                                 false);
                                rowCounter++;
                                groupedVolume = amount;
                                groupedPrice -= baseValues.groupPriceValue;
                            }
                        }
                    }
                    else
                    {
                        depthSubmitOrder(baseValues.currentPair.symbol, &currentBidsMap, priceDouble, amount, false);
                        rowCounter++;
                    }
                }

                QList<double> currentBidsList = lastDepthBidsMap.keys();

                for (int n = 0; n < currentBidsList.size(); n++)
                    if (qFuzzyIsNull(currentBidsMap.value(currentBidsList.at(n), 0)))
                        depthUpdateOrder(baseValues.currentPair.symbol, currentBidsList.at(n), 0.0, false);

                lastDepthBidsMap = currentBidsMap;

                emit depthSubmitOrders(baseValues.currentPair.symbol, depthAsks, depthBids);
                depthAsks = nullptr;
                depthBids = nullptr;
            }
        }
        else if (debugLevel)
            logThread->writeLog("Invalid depth data:" + data, 2);

        break;

    case 202: // info
        {
            if (!success)
                break;

            QByteArray fundsData = getMidData("\"balances\":[{", "}],\"", &data);
            QByteArray btcBalance = getMidData("\"" + baseValues.currentPair.currAStr + "\",\"free\":\"", "\"", &fundsData);
            QByteArray usdBalance = getMidData("\"" + baseValues.currentPair.currBStr + "\",\"free\":\"", "\"", &fundsData);

            if (checkValue(btcBalance, lastBtcBalance))
                emit accBtcBalanceChanged(baseValues.currentPair.symbol, lastBtcBalance);

            if (checkValue(usdBalance, lastUsdBalance))
                emit accUsdBalanceChanged(baseValues.currentPair.symbol, lastUsdBalance);

            QByteArray makerCommission = getMidData("\"makerCommission\":", ",", &data);
            QByteArray takerCommission = getMidData("\"takerCommission\":", ",", &data);

            if (makerCommission.size() && makerCommission.at(0) == '"')
                makerCommission.remove(0, 1);

            if (takerCommission.size() && takerCommission.at(0) == '"')
                takerCommission.remove(0, 1);

            if (makerCommission.size() && makerCommission.at(makerCommission.size() - 1) == '"')
                makerCommission.chop(1);

            if (takerCommission.size() && takerCommission.at(makerCommission.size() - 1) == '"')
                takerCommission.chop(1);

            double fee = qMax(makerCommission.toDouble(), takerCommission.toDouble()) / 100;

            if (!qFuzzyCompare(fee + 1.0, lastFee + 1.0))
            {
                emit accFeeChanged(baseValues.currentPair.symbol, fee);
                lastFee = fee;
            }

            if (isFirstAccInfo)
            {
                QByteArray rights = getMidData("\"canTrade\":", ",", &data);

                if (!rights.isEmpty())
                {
                    if (rights != "true")
                        emit showErrorMessage("I:>invalid_rights");

                    isFirstAccInfo = false;
                }
            }
        }
        break; // info

    case 204: // orders
        {
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
                    QByteArray status = getMidData("status\":\"", "\"", &currentOrderData);

                    // 0=Canceled, 1=Open, 2=Pending, 3=Post-Pending
                    if (status == "CANCELED" || status == "REJECTED" || status == "EXPIRED")
                        currentOrder.status = 0;
                    else if (status == "NEW" || status == "PARTIALLY_FILLED")
                        currentOrder.status = 1;
                    else
                        currentOrder.status = 2;

                    QByteArray date = getMidData("\"time\":", ",", &currentOrderData);
                    date.chop(3);
                    currentOrder.date = date.toUInt();
                    currentOrder.oid = getMidData("\"orderId\":", ",", &currentOrderData);
                    currentOrder.type = getMidData("\"side\":\"", "\"", &currentOrderData) == "SELL";
                    currentOrder.amount = getMidData("\"origQty\":\"", "\"", &currentOrderData).toDouble();
                    currentOrder.price = getMidData("\"price\":\"", "\"", &currentOrderData).toDouble();
                    QByteArray request = getMidData("\"symbol\":\"", "\"", &currentOrderData);
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

            break; // orders
        }

    case 305: // order/cancel
        if (success)
        {
            QByteArray oid = getMidData("\"orderId\":", ",", &data);

            if (!oid.isEmpty())
                emit orderCanceled(baseValues.currentPair.symbol, oid);
        }

        break; // order/cancel

    case 306:
        if (debugLevel)
            logThread->writeLog("Buy OK: " + data, 2);

        break; // order/buy

    case 307:
        if (debugLevel)
            logThread->writeLog("Sell OK: " + data, 2);

        break; // order/sell

    case 208: // history
        {
            if (data.size() < 10)
                break;

            if (lastHistory != data)
            {
                lastHistory = data;

                QStringList historyList = QString(data).split("},{");
                qint64 maxId = 0;
                auto* historyItems = new QList<HistoryItem>;

                for (int n = historyList.size() - 1; n >= 0; --n)
                {
                    QByteArray logData(historyList.at(n).toLatin1());
                    qint64 id = getMidData("\"id\":", ",", &logData).toLongLong();

                    if (id <= lastHistoryId)
                        break;

                    if (id > maxId)
                        maxId = id;

                    HistoryItem currentHistoryItem;

                    if (getMidData("\"isBuyer\":", ",", &logData) == "true")
                        currentHistoryItem.type = 2;
                    else
                        currentHistoryItem.type = 1;

                    QByteArray request = getMidData("\"symbol\":\"", "\"", &logData);
                    QList<CurrencyPairItem>* pairs = IniEngine::getPairs();

                    for (int i = 0; i < pairs->size(); ++i)
                    {
                        if (pairs->at(i).currRequestPair == request)
                        {
                            currentHistoryItem.symbol = pairs->at(i).symbol;
                            break;
                        }
                    }

                    QByteArray data = getMidData("\"time\":", ",", &logData);
                    data.chop(3);
                    currentHistoryItem.dateTimeInt = data.toLongLong();
                    currentHistoryItem.price = getMidData("\"price\":\"", "\"", &logData).toDouble();
                    currentHistoryItem.volume = getMidData("\"qty\":\"", "\"", &logData).toDouble();

                    if (currentHistoryItem.isValid())
                        (*historyItems) << currentHistoryItem;
                }

                if (maxId > lastHistoryId)
                    lastHistoryId = maxId;

                emit historyChanged(historyItems);
            }

            break; // money/wallet/history
        }

    default:
        break;
    }

    if (reqType >= 200 && reqType < 300)
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

void Exchange_Binance::depthUpdateOrder(const QString& symbol, double price, double amount, bool isAsk)
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

void Exchange_Binance::depthSubmitOrder(const QString& symbol,
                                        QMap<double, double>* currentMap,
                                        double priceDouble,
                                        double amount,
                                        bool isAsk)
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

bool Exchange_Binance::isReplayPending(int reqType)
{
    if (julyHttp == nullptr)
        return false;

    return julyHttp->isReqTypePending(reqType);
}

void Exchange_Binance::secondSlot()
{
    static int sendCounter = 0;

    switch (sendCounter)
    {
    case 0:
        if (!isReplayPending(103))
            sendToApi(103, "v1/ticker/24hr?symbol=" + baseValues.currentPair.currRequestPair, false, true);

        break;

    case 1:
        if (!isReplayPending(202))
            sendToApi(202, "GET /api/v3/account?", true, true);

        break;

    case 2:
        if (!isReplayPending(109))
        {
            if (isValidApiKey)
            {
                QByteArray fromId = lastTradesId ? "&fromId=" + QByteArray::number(lastTradesId + 1) : "";
                sendToApi(109, "v1/historicalTrades?symbol=" + baseValues.currentPair.currRequestPair + fromId, false, false);
            }
            else
                sendToApi(109, "v3/trades?symbol=" + baseValues.currentPair.currRequestPair, false, true);
        }

        break;

    case 3:
        if (!tickerOnly && !isReplayPending(204))
            sendToApi(204, "GET /api/v3/openOrders?", true, true, "symbol=" + baseValues.currentPair.currRequestPair + "&");

        break;

    case 4:
        if (isDepthEnabled() && (forceDepthLoad || !isReplayPending(111)))
        {
            emit depthRequested();
            sendToApi(
                111, "v1/depth?symbol=" + baseValues.currentPair.currRequestPair + "&limit=" + baseValues.depthCountLimitStr, false, true);
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

void Exchange_Binance::getHistory(bool force)
{
    if (tickerOnly)
        return;

    if (force)
        lastHistory.clear();

    if (!isReplayPending(208))
    {
        QByteArray fromId = lastHistoryId ? "fromId=" + QByteArray::number(lastHistoryId + 1) + "&" : "";
        sendToApi(208, "GET /api/v3/myTrades?", true, true, "symbol=" + baseValues.currentPair.currRequestPair + "&" + fromId);
    }
}

void Exchange_Binance::buy(const QString& symbol, double apiBtcToBuy, double apiPriceToBuy)
{
    if (tickerOnly)
        return;

    CurrencyPairItem pairItem;
    pairItem = baseValues.currencyPairMap.value(symbol, pairItem);

    if (pairItem.symbol.isEmpty())
        return;

    QByteArray data = "symbol=" + pairItem.currRequestPair + "&side=BUY&type=LIMIT&timeInForce=GTC&quantity=" +
                      JulyMath::byteArrayFromDouble(apiBtcToBuy, pairItem.currADecimals, 0) +
                      "&price=" + JulyMath::byteArrayFromDouble(apiPriceToBuy, pairItem.priceDecimals, 0) + "&";

    if (debugLevel)
        logThread->writeLog("Buy: " + data, 2);

    sendToApi(306, "POST /api/v3/order?", true, true, data);
}

void Exchange_Binance::sell(const QString& symbol, double apiBtcToSell, double apiPriceToSell)
{
    if (tickerOnly)
        return;

    CurrencyPairItem pairItem;
    pairItem = baseValues.currencyPairMap.value(symbol, pairItem);

    if (pairItem.symbol.isEmpty())
        return;

    QByteArray data = "symbol=" + pairItem.currRequestPair + "&side=SELL&type=LIMIT&timeInForce=GTC&quantity=" +
                      JulyMath::byteArrayFromDouble(apiBtcToSell, pairItem.currADecimals, 0) +
                      "&price=" + JulyMath::byteArrayFromDouble(apiPriceToSell, pairItem.priceDecimals, 0) + "&";

    if (debugLevel)
        logThread->writeLog("Sell: " + data, 2);

    sendToApi(307, "POST /api/v3/order?", true, true, data);
}

void Exchange_Binance::cancelOrder(const QString& symbol, const QByteArray& order)
{
    if (tickerOnly)
        return;

    CurrencyPairItem pairItem;
    pairItem = baseValues.currencyPairMap.value(symbol, pairItem);

    if (pairItem.symbol.isEmpty())
        return;

    QByteArray data = "symbol=" + pairItem.currRequestPair + "&orderId=" + order + "&";

    if (debugLevel)
        logThread->writeLog("Cancel order: " + data, 2);

    sendToApi(305, "DELETE /api/v3/order?", true, true, data);
}

void Exchange_Binance::sendToApi(int reqType, const QByteArray& method, bool auth, bool simple, const QByteArray& commands)
{
    if (julyHttp == nullptr)
    {
        if (domain.isEmpty() || port == 0)
            julyHttp = new JulyHttp("api.binance.com", "X-MBX-APIKEY: " + getApiKey() + "\r", this);
        else
        {
            julyHttp = new JulyHttp(domain, "X-MBX-APIKEY: " + getApiKey() + "\r", this, useSsl);
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
    {
        QByteArray data = commands + "recvWindow=59000&timestamp=" + QByteArray::number(TimeSync::getMSecs() - 23000);
        julyHttp->sendData(
            reqType, m_pairChangeCount, method + data + "&signature=" + hmacSha256(getApiSign(), data).toHex(), "", "\n\r\n");
    }
    else
    {
        if (simple)
            julyHttp->sendData(reqType, m_pairChangeCount, "GET /api/" + method);
        else
            julyHttp->sendData(reqType, m_pairChangeCount, "GET /api/" + method, "", "\n\r\n");
    }
}

void Exchange_Binance::sslErrors(const QList<QSslError>& errors)
{
    if (++sslErrorCounter < 3)
        return;

    QStringList errorList;

    for (int n = 0; n < errors.size(); n++)
        errorList << errors.at(n).errorString();

    if (debugLevel)
        logThread->writeLog(errorList.join(" ").toLatin1(), 2);

    emit showErrorMessage("SSL Error: " + errorList.join(" "));
}
