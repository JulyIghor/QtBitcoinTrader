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
#include "exchange_poloniex.h"

Exchange_Poloniex::Exchange_Poloniex(const QByteArray &pRestSign, const QByteArray &pRestKey)
    : Exchange(),
      isFirstAccInfo(true),
      lastTradesId(0),
      lastTradesDate(0),
      lastHistoryTime(0),
      privateNonce((TimeSync::getTimeT() - 1371854884) * 10),
      julyHttp(nullptr),
      depthAsks(nullptr),
      depthBids(nullptr),
      lastDepthAsksMap(),
      lastDepthBidsMap()
{
    clearHistoryOnCurrencyChanged = true;
    calculatingFeeMode = 1;
    baseValues.exchangeName = "Poloniex";
    baseValues.currentPair.name = "ETH/BTC";
    baseValues.currentPair.setSymbol("ETH/BTC");
    baseValues.currentPair.currRequestPair = "BTC_ETH";
    baseValues.currentPair.priceDecimals = 8;
    minimumRequestIntervalAllowed = 500;
    minimumRequestTimeoutAllowed = 10000;
    baseValues.currentPair.priceMin = qPow(0.1, baseValues.currentPair.priceDecimals);
    baseValues.currentPair.tradeVolumeMin = 0.00000001;
    baseValues.currentPair.tradePriceMin = 0.00000001;
    forceDepthLoad = false;
    tickerOnly = false;
    setApiKeySecret(pRestKey, pRestSign);

    currencyMapFile = "Poloniex";
    defaultCurrencyParams.currADecimals = 8;
    defaultCurrencyParams.currBDecimals = 8;
    defaultCurrencyParams.currABalanceDecimals = 8;
    defaultCurrencyParams.currBBalanceDecimals = 8;
    defaultCurrencyParams.priceDecimals = 8;
    defaultCurrencyParams.priceMin = qPow(0.1, baseValues.currentPair.priceDecimals);

    supportsLoginIndicator = false;
    supportsAccountVolume = false;

    connect(this, &Exchange::threadFinished, this, &Exchange_Poloniex::quitThread, Qt::DirectConnection);
}

Exchange_Poloniex::~Exchange_Poloniex()
{
}

void Exchange_Poloniex::quitThread()
{
    clearValues();

    
        delete depthAsks;

    
        delete depthBids;

    
        delete julyHttp;
}

void Exchange_Poloniex::clearVariables()
{
    isFirstAccInfo = true;
    lastTradesId = 0;
    lastTradesDate = 0;
    lastHistoryTime = 0;
    Exchange::clearVariables();
    lastHistory.clear();
    lastOrders.clear();
    reloadDepth();
}

void Exchange_Poloniex::clearValues()
{
    clearVariables();

    if (julyHttp)
        julyHttp->clearPendingData();
}

void Exchange_Poloniex::reloadDepth()
{
    lastDepthBidsMap.clear();
    lastDepthAsksMap.clear();
    lastDepthData.clear();
    Exchange::reloadDepth();
}

void Exchange_Poloniex::dataReceivedAuth(const QByteArray& data, int reqType, int pairChangeCount)
{
    if (pairChangeCount != m_pairChangeCount)
        return;

    if (debugLevel)
        logThread->writeLog("RCV: " + data);

    if (data.size() && data.at(0) == QLatin1Char('<'))
        return;

    bool success = !data.startsWith("{\"error\":");//{"error":"Invalid command."}
    QString errorString;

    if (!success)
    {
        errorString = getMidData("{\"error\":\"", "\"", &data);

        if (debugLevel)
            logThread->writeLog("Invalid data:" + data, 2);
    }
    else switch (reqType)
    {
    case 103: //ticker
    {
        QByteArray dataPair = getMidData("\"" + baseValues.currentPair.currRequestPair + "\":{", "}", &data);

        if (dataPair.size() < 10)
            break;

        double tickerHigh = getMidData("\"high24hr\":\"", "\"", &dataPair).toDouble();

        if (tickerHigh > 0.0 && !qFuzzyCompare(tickerHigh, lastTickerHigh))
        {
            IndicatorEngine::setValue(baseValues.exchangeName, baseValues.currentPair.symbol, "High", tickerHigh);
            lastTickerHigh = tickerHigh;
        }

        double tickerLow = getMidData("\"low24hr\":\"", "\"", &dataPair).toDouble();

        if (tickerLow > 0.0 && !qFuzzyCompare(tickerLow, lastTickerLow))
        {
            IndicatorEngine::setValue(baseValues.exchangeName, baseValues.currentPair.symbol, "Low", tickerLow);
            lastTickerLow = tickerLow;
        }

        double tickerSell = getMidData("\"highestBid\":\"", "\"", &dataPair).toDouble();

        if (tickerSell > 0.0 && !qFuzzyCompare(tickerSell, lastTickerSell))
        {
            IndicatorEngine::setValue(baseValues.exchangeName, baseValues.currentPair.symbol, "Sell", tickerSell);
            lastTickerSell = tickerSell;
        }

        double tickerBuy = getMidData("\"lowestAsk\":\"", "\"", &dataPair).toDouble();

        if (tickerBuy > 0.0 && !qFuzzyCompare(tickerBuy, lastTickerBuy))
        {
            IndicatorEngine::setValue(baseValues.exchangeName, baseValues.currentPair.symbol, "Buy", tickerBuy);
            lastTickerBuy = tickerBuy;
        }

        double tickerVolume = getMidData("\"quoteVolume\":\"", "\"", &dataPair).toDouble();

        if (tickerVolume > 0.0 && !qFuzzyCompare(tickerVolume, lastTickerVolume))
        {
            IndicatorEngine::setValue(baseValues.exchangeName, baseValues.currentPair.symbol, "Volume", tickerVolume);
            lastTickerVolume = tickerVolume;
        }

        double tickerLast = getMidData("\"last\":\"", "\"", &dataPair).toDouble();

        if (tickerLast > 0.0 && !qFuzzyCompare(tickerLast, lastTickerLast))
        {
            IndicatorEngine::setValue(baseValues.exchangeName, baseValues.currentPair.symbol, "Last", tickerLast);
            lastTickerLast = tickerLast;
        }
    }
    break;//ticker

    case 109: //trades
        if (data.size() > 10)
        {
            qint64 time10Min = TimeSync::getTimeT() - 600;
            QStringList tradeList = QString(data).split("},{");
            auto* newTradesItems = new QList<TradesItem>;

            for (int n = tradeList.size() - 1; n >= 0; --n)
            {
                QByteArray tradeData = tradeList.at(n).toLatin1();
                TradesItem newItem;

                QDateTime date = QDateTime::fromString(getMidData("\"date\":\"", "\"", &tradeData), "yyyy-MM-dd HH:mm:ss");
                date.setTimeSpec(Qt::UTC);
                newItem.date   = date.toLocalTime().toSecsSinceEpoch();

                if (newItem.date < 0)
                    break;

                lastTradesDate = newItem.date;

                if (newItem.date < time10Min)
                    continue;

                qint64 currentTid = getMidData("\"tradeID\":", ",", &tradeData).toLongLong();

                if (currentTid <= lastTradesId)
                    continue;

                lastTradesId = currentTid;

                newItem.amount    = getMidData("\"amount\":\"", "\"", &tradeData).toDouble();
                newItem.price     = getMidData("\"rate\":\"",   "\"", &tradeData).toDouble();
                newItem.symbol    = baseValues.currentPair.symbol;
                newItem.orderType = getMidData("\"type\":\"",   "\"", &tradeData) == "buy" ? 1 : -1;

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
        break;//trades

    case 111: //depth
    {
        emit depthRequestReceived();

        if (data != lastDepthData)
        {
            lastDepthData = data;
            depthAsks = new QList<DepthItem>;
            depthBids = new QList<DepthItem>;

            QMap<double, double> currentAsksMap;
            QStringList asksList = QString(getMidData("\"asks\":[[\"", "]]", &data)).split("],[\"");
            double groupedPrice = 0.0;
            double groupedVolume = 0.0;
            int rowCounter = 0;

            for (int n = 0; n < asksList.size(); n++)
            {
                if (baseValues.depthCountLimit && rowCounter >= baseValues.depthCountLimit)
                    break;

                QStringList currentPair = asksList.at(n).split("\",");

                if (currentPair.size() != 2)
                    continue;

                double priceDouble = currentPair.first().toDouble();
                double amount      = currentPair.last().toDouble();

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
                                             &currentAsksMap, groupedPrice + baseValues.groupPriceValue, groupedVolume, true);
                            rowCounter++;
                            groupedVolume = amount;
                            groupedPrice += baseValues.groupPriceValue;
                        }
                    }
                }
                else
                {
                    depthSubmitOrder(baseValues.currentPair.symbol,
                                     &currentAsksMap, priceDouble, amount, true);
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
            QStringList bidsList = QString(getMidData("\"bids\":[[\"", "]]", &data)).split("],[\"");
            groupedPrice = 0.0;
            groupedVolume = 0.0;
            rowCounter = 0;

            for (int n = 0; n < bidsList.size(); n++)
            {
                if (baseValues.depthCountLimit && rowCounter >= baseValues.depthCountLimit)
                    break;

                QStringList currentPair = bidsList.at(n).split("\",");

                if (currentPair.size() != 2)
                    continue;

                double priceDouble = currentPair.first().toDouble();
                double amount      = currentPair.last().toDouble();

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
                                             &currentBidsMap, groupedPrice - baseValues.groupPriceValue, groupedVolume, false);
                            rowCounter++;
                            groupedVolume = amount;
                            groupedPrice -= baseValues.groupPriceValue;
                        }
                    }
                }
                else
                {
                    depthSubmitOrder(baseValues.currentPair.symbol,
                                     &currentBidsMap, priceDouble, amount, false);
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
        {
            QByteArray btcBalance = getMidData("\"" + baseValues.currentPair.currAStr + "\":\"", "\"", &data);
            QByteArray usdBalance = getMidData("\"" + baseValues.currentPair.currBStr + "\":\"", "\"", &data);

            if (btcBalance.isEmpty())
                btcBalance = "0";

            if (usdBalance.isEmpty())
                usdBalance = "0";

            if (checkValue(btcBalance, lastBtcBalance))
                emit accBtcBalanceChanged(baseValues.currentPair.symbol, lastBtcBalance);

            if (checkValue(usdBalance, lastUsdBalance))
                emit accUsdBalanceChanged(baseValues.currentPair.symbol, lastUsdBalance);
        }
        break;//info

    case 203: //fee
    {
        QByteArray makerFee = getMidData("\"makerFee\":\"", "\"", &data);
        QByteArray takerFee = getMidData("\"takerFee\":\"", "\"", &data);

        if (!makerFee.isEmpty() && !takerFee.isEmpty())
        {
            double fee = qMax(makerFee.toDouble(), takerFee.toDouble()) * 100;

            if (!qFuzzyCompare(fee + 1.0, lastFee + 1.0))
            {
                emit accFeeChanged(baseValues.currentPair.symbol, fee);
                lastFee = fee;
            }
        }

        break;//fee
    }

    case 204://orders
        {
            if (lastOrders != data)
            {
                lastOrders = data;
                QStringList ordersPairList = QString(data).split("],");
                auto* orders = new QList<OrderItem>;

                for (int n = 0; n < ordersPairList.size(); ++n)
                {
                    QByteArray pairData = ordersPairList.at(n).toLatin1();

                    if (!pairData.contains("rate"))
                        continue;

                    QList<QByteArray> pair = getMidData("\"", "\":[{", &pairData).split('_');

                    if (pair.size() != 2)
                        continue;

                    QByteArray symbol = pair.last() + '/' + pair.first();
                    QStringList ordersList = QString(pairData).split("},{");

                    for (int m = 0; m < ordersList.size(); ++m)
                    {
                        QByteArray orderData = ordersList.at(m).toLatin1();
                        OrderItem currentOrder;

                        QDateTime date = QDateTime::fromString(getMidData("\"date\":\"", "\"", &orderData), Qt::ISODate);
                        date.setTimeSpec(Qt::UTC);
                        currentOrder.date   = date.toLocalTime().toSecsSinceEpoch();
                        currentOrder.oid    = getMidData("\"orderNumber\":\"", "\"", &orderData);
                        currentOrder.type   = getMidData("\"type\":\"",        "\"", &orderData) == "sell";
                        currentOrder.amount = getMidData("\"amount\":\"",      "\"", &orderData).toDouble();
                        currentOrder.price  = getMidData("\"rate\":\"",        "\"", &orderData).toDouble();
                        currentOrder.symbol = symbol;
                        currentOrder.status = 1;

                        if (currentOrder.isValid())
                            (*orders) << currentOrder;
                    }
                }

                if (!orders->empty())
                    emit orderBookChanged(baseValues.currentPair.symbol, orders);
                else
                {
                    delete orders;
                    emit ordersIsEmpty();
                }
            }

            break;//orders
        }

    case 305: //order/cancel
        if (data.startsWith("{\"success\":1,"))
        {
            QByteArray orderId = getMidData("Order #", " canceled", &data);
            emit orderCanceled(baseValues.currentPair.symbol, orderId);
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
        {
            if (data.size() < 50)
                break;

            if (lastHistory != data)
            {
                lastHistory = data;

                QStringList historyList = QString(data).split("},{");
                qint64 maxTime = 0;
                auto* historyItems = new QList<HistoryItem>;

                for (int n = 0; n < historyList.size(); ++n)
                {
                    QByteArray logData(historyList.at(n).toLatin1());

                    QDateTime date = QDateTime::fromString(getMidData("\"date\":\"", "\"", &logData), Qt::ISODate);
                    date.setTimeSpec(Qt::UTC);
                    qint64 dateInt = date.toLocalTime().toSecsSinceEpoch();

                    if (dateInt <= lastHistoryTime)
                        break;

                    if (dateInt > maxTime)
                        maxTime = dateInt;

                    HistoryItem currentHistoryItem;

                    if (getMidData("\"category\":\"", "\"", &logData) == "settlement")
                    {
                        if (getMidData("\"type\":\"", "\"", &logData) == "sell")
                            currentHistoryItem.type = 4;
                        else
                            currentHistoryItem.type = 5;
                    }
                    else
                    {
                        if (getMidData("\"type\":\"", "\"", &logData) == "sell")
                            currentHistoryItem.type = 1;
                        else
                            currentHistoryItem.type = 2;
                    }

                    currentHistoryItem.symbol      = baseValues.currentPair.symbol;
                    currentHistoryItem.price       = getMidData("\"rate\":\"",   "\"", &logData).toDouble();
                    currentHistoryItem.volume      = getMidData("\"amount\":\"", "\"", &logData).toDouble();
                    currentHistoryItem.dateTimeInt = dateInt;

                    if (currentHistoryItem.isValid())
                        (*historyItems) << currentHistoryItem;
                }

                if (maxTime > lastHistoryTime)
                    lastHistoryTime = maxTime;

                emit historyChanged(historyItems);
            }

            break;//money/wallet/history
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

void Exchange_Poloniex::depthUpdateOrder(const QString& symbol, double price, double amount, bool isAsk)
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

void Exchange_Poloniex::depthSubmitOrder(const QString& symbol, QMap<double, double>* currentMap, double priceDouble,
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

bool Exchange_Poloniex::isReplayPending(int reqType)
{
    if (julyHttp == nullptr)
        return false;

    return julyHttp->isReqTypePending(reqType);
}

void Exchange_Poloniex::secondSlot()
{
    static int sendCounter = 0;

    switch (sendCounter)
    {
    case 0:
        if (!isReplayPending(103))
            sendToApi(103, "returnTicker");

        break;

    case 1:
        if (!isReplayPending(202))
            sendToApi(202, "returnAvailableAccountBalances", true);

        break;

    case 2:
        if (!isReplayPending(109))
        {
            QByteArray start;

            if (lastTradesDate)
                start = "&start=" + QByteArray::number(lastTradesDate + 1);

            sendToApi(109, "returnTradeHistory&currencyPair=" + baseValues.currentPair.currRequestPair + start);
        }
        break;

    case 3:
        if (!tickerOnly && !isReplayPending(204))
            sendToApi(204, "returnOpenOrders&currencyPair=all", true);

        break;

    case 4:
        if (isDepthEnabled() && (forceDepthLoad || !isReplayPending(111)))
        {
            emit depthRequested();
            sendToApi(111, "returnOrderBook&currencyPair=" + baseValues.currentPair.currRequestPair + "&depth=" + baseValues.depthCountLimitStr);
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

    if (baseValues.httpRequestInterval < 150)
        baseValues.httpRequestInterval = 150;

    Exchange::secondSlot();
}

void Exchange_Poloniex::getHistory(bool force)
{
    if (tickerOnly)
        return;

    if (force)
        lastHistory.clear();

    if (!isReplayPending(208))
        sendToApi(208, "returnTradeHistory&currencyPair=" + baseValues.currentPair.currRequestPair +
                  "&start=" + QByteArray::number(lastHistoryTime + 1) + "&limit=10000", true);

    if (!isReplayPending(210))
        sendToApi(203, "returnFeeInfo", true);
}

void Exchange_Poloniex::buy(const QString& symbol, double apiBtcToBuy, double apiPriceToBuy)
{
    if (tickerOnly)
        return;

    CurrencyPairItem pairItem;
    pairItem = baseValues.currencyPairMap.value(symbol, pairItem);

    if (pairItem.symbol.isEmpty())
        return;

    QByteArray data = "currencyPair=" + pairItem.currRequestPair + "&rate=" +
            JulyMath::byteArrayFromDouble(apiPriceToBuy, pairItem.priceDecimals, 0) + "&amount=" +
            JulyMath::byteArrayFromDouble(apiBtcToBuy, pairItem.currADecimals, 0) + "&";

    if (debugLevel)
        logThread->writeLog("Buy: " + data, 2);

    sendToApi(306, "buy&" + data, true);
}

void Exchange_Poloniex::sell(const QString& symbol, double apiBtcToSell, double apiPriceToSell)
{
    if (tickerOnly)
        return;

    CurrencyPairItem pairItem;
    pairItem = baseValues.currencyPairMap.value(symbol, pairItem);

    if (pairItem.symbol.isEmpty())
        return;

    QByteArray data = "currencyPair=" + pairItem.currRequestPair + "&rate=" +
            JulyMath::byteArrayFromDouble(apiPriceToSell, pairItem.priceDecimals, 0) + "&amount=" +
            JulyMath::byteArrayFromDouble(apiBtcToSell, pairItem.currADecimals, 0) + "&";

    if (debugLevel)
        logThread->writeLog("Sell: " + data, 2);

    sendToApi(306, "sell&" + data, true);
}

void Exchange_Poloniex::cancelOrder(const QString& /*unused*/, const QByteArray& order)
{
    if (tickerOnly)
        return;

    QByteArray data = "orderNumber=" + order + "&";

    if (debugLevel)
        logThread->writeLog("Cancel order: " + data, 2);

    sendToApi(305, "cancelOrder&" + data, true);
}

void Exchange_Poloniex::sendToApi(int reqType, const QByteArray &method, bool auth)
{
    if (julyHttp == nullptr)
    {
        if (domain.isEmpty() || port == 0)
            julyHttp = new JulyHttp("poloniex.com", "Key: " + getApiKey() + "\r\n", this);
        else
        {
            julyHttp = new JulyHttp(domain, "Key: " + getApiKey() + "\r\n", this, useSsl);
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
        QByteArray postData = "command=" + method + "&nonce=" + QByteArray::number(++privateNonce);

        julyHttp->sendData(reqType, m_pairChangeCount, "POST /tradingApi", postData,
                           "Sign: " + hmacSha512(getApiSign(), postData).toHex() + "\r\n");
    }
    else
    {
        julyHttp->sendData(reqType, m_pairChangeCount, "GET /public?command=" + method);
    }
}

void Exchange_Poloniex::sslErrors(const QList<QSslError>& errors)
{
    QStringList errorList;

    for (int n = 0; n < errors.size(); n++)
        errorList << errors.at(n).errorString();

    if (debugLevel)
        logThread->writeLog(errorList.join(" ").toLatin1(), 2);

    emit showErrorMessage("SSL Error: " + errorList.join(" "));
}
