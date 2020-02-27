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

#include "timesync.h"
#include "exchange_okcoin.h"

Exchange_OKCoin::Exchange_OKCoin(QByteArray pRestSign, QByteArray pRestKey)
    : Exchange()
{
    lastFee = 0;
    calculatingFeeMode = 1;
    clearOpenOrdersOnCurrencyChanged = true;
    clearHistoryOnCurrencyChanged = true;
    baseValues.exchangeName = "OKCoin";
    baseValues.currentPair.name = "BTC/CNY";
    baseValues.currentPair.setSymbol("BTCCNY");
    baseValues.currentPair.currRequestPair = "btc_cny";
    baseValues.currentPair.priceDecimals = 2;
    minimumRequestIntervalAllowed = 700;
    baseValues.currentPair.priceMin = qPow(0.1, baseValues.currentPair.priceDecimals);
    baseValues.currentPair.tradeVolumeMin = 0.001;
    baseValues.currentPair.tradePriceMin = 0.1;
    depthAsks = nullptr;
    depthBids = nullptr;
    forceDepthLoad = false;
    julyHttp = nullptr;
    isApiDown = false;
    tickerOnly = false;
    setApiKeySecret(pRestKey, pRestSign);

    currencyMapFile = "OKCoin";
    defaultCurrencyParams.currADecimals = 8;
    defaultCurrencyParams.currBDecimals = 8;
    defaultCurrencyParams.currABalanceDecimals = 8;
    defaultCurrencyParams.currBBalanceDecimals = 8;
    defaultCurrencyParams.priceDecimals = 3;
    defaultCurrencyParams.priceMin = qPow(0.1, baseValues.currentPair.priceDecimals);

    supportsLoginIndicator = false;
    supportsAccountVolume = false;

    authRequestTime.restart();
    lastHistoryId = 0;

    connect(this, &Exchange::threadFinished, this, &Exchange_OKCoin::quitThread, Qt::DirectConnection);
}

Exchange_OKCoin::~Exchange_OKCoin()
{
}

void Exchange_OKCoin::quitThread()
{
    clearValues();

    if (depthAsks)
        delete depthAsks;

    if (depthBids)
        delete depthBids;

    if (julyHttp)
        delete julyHttp;
}

void Exchange_OKCoin::clearVariables()
{
    isFirstAccInfo = true;
    Exchange::clearVariables();
    lastOpenedOrders = -1;
    apiDownCounter = 0;
    lastHistory.clear();
    lastOrders.clear();
    reloadDepth();
    lastFetchTid = 1;
    lastTickerDate = 0;
    startTradesDate = TimeSync::getTimeT() - 600;

    if (0.2 != lastFee)
        emit accFeeChanged(baseValues.currentPair.symbol, 0.2);

    lastFee = 0.2;
}

void Exchange_OKCoin::clearValues()
{
    clearVariables();

    if (julyHttp)
        julyHttp->clearPendingData();
}

void Exchange_OKCoin::reloadDepth()
{
    lastDepthBidsMap.clear();
    lastDepthAsksMap.clear();
    lastDepthData.clear();
    Exchange::reloadDepth();
}

void Exchange_OKCoin::dataReceivedAuth(QByteArray data, int reqType)
{
    if (debugLevel)
        logThread->writeLog("RCV: " + data);

    if (data.size() && data.at(0) == QLatin1Char('<'))
        return;

    bool success = !data.startsWith("{\"error_code");

    if (success)
        switch (reqType)
        {
            case 103: //ticker
                if (data.startsWith("{\"date\":\""))
                {
                    QByteArray tickerHigh = getMidData("high\":\"", "\"", &data);

                    if (!tickerHigh.isEmpty())
                    {
                        double newTickerHigh = tickerHigh.toDouble();

                        if (newTickerHigh != lastTickerHigh)
                            IndicatorEngine::setValue(baseValues.exchangeName, baseValues.currentPair.symbol, "High", newTickerHigh);

                        lastTickerHigh = newTickerHigh;
                    }

                    QByteArray tickerLow = getMidData("low\":\"", "\"", &data);

                    if (!tickerLow.isEmpty())
                    {
                        double newTickerLow = tickerLow.toDouble();

                        if (newTickerLow != lastTickerLow)
                            IndicatorEngine::setValue(baseValues.exchangeName, baseValues.currentPair.symbol, "Low", newTickerLow);

                        lastTickerLow = newTickerLow;
                    }

                    QByteArray tickerSell = getMidData("sell\":\"", "\"", &data);

                    if (!tickerSell.isEmpty())
                    {
                        double newTickerSell = tickerSell.toDouble();

                        if (newTickerSell != lastTickerSell)
                            IndicatorEngine::setValue(baseValues.exchangeName, baseValues.currentPair.symbol, "Buy", newTickerSell);

                        lastTickerSell = newTickerSell;
                    }

                    QByteArray tickerBuy = getMidData("buy\":\"", "\"", &data);

                    if (!tickerBuy.isEmpty())
                    {
                        double newTickerBuy = tickerBuy.toDouble();

                        if (newTickerBuy != lastTickerBuy)
                            IndicatorEngine::setValue(baseValues.exchangeName, baseValues.currentPair.symbol, "Sell", newTickerBuy);

                        lastTickerBuy = newTickerBuy;
                    }

                    QByteArray tickerVolume = getMidData("vol\":\"", "\"", &data);

                    if (!tickerVolume.isEmpty())
                    {
                        double newTickerVolume = tickerVolume.toDouble();

                        if (newTickerVolume != lastTickerVolume)
                            IndicatorEngine::setValue(baseValues.exchangeName, baseValues.currentPair.symbol, "Volume", newTickerVolume);

                        lastTickerVolume = newTickerVolume;
                    }

                    qint64 newTickerDate = getMidData("date\":\"", "\"", &data).toUInt();

                    if (lastTickerDate < newTickerDate)
                    {
                        lastTickerDate = newTickerDate;
                        QByteArray tickerLast = getMidData("last\":\"", "\"", &data);
                        double tickerLastDouble = tickerLast.toDouble();

                        if (tickerLastDouble > 0.0)
                            IndicatorEngine::setValue(baseValues.exchangeName, baseValues.currentPair.symbol, "Last", tickerLastDouble);
                    }
                }
                else
                    success = false;

                break;//ticker

            case 109: //trades
                if (data.startsWith("["))
                {
                    if (!data.startsWith("[{\"amount\":\""))
                        break;

                    QStringList tradeList = QString(data).split("},{");
                    QList<TradesItem>* newTradesItems = new QList<TradesItem>;

                    TradesItem newItem;

                    for (int n = 0; n < tradeList.count(); n++)
                    {
                        QByteArray tradeData = tradeList.at(n).toLatin1();
                        newItem.date = getMidData("\"date\":", ",", &tradeData).toLongLong();

                        if (newItem.date < startTradesDate)
                            continue;

                        qint64 currentTid = getMidData("\"tid\":", ",", &tradeData).toULongLong();

                        if (lastFetchTid >= currentTid)
                            continue;

                        lastFetchTid = currentTid;
                        newItem.price = getMidData("\"price\":\"", "\"", &tradeData).toDouble();
                        newItem.amount = getMidData("\"amount\":\"", "\"", &tradeData).toDouble();

                        if (newItem.amount == 0.0)
                            continue;

                        newItem.symbol = baseValues.currentPair.symbol;
                        newItem.orderType = getMidData("\"type\":\"", "\"", &tradeData) == "sell" ? 1 : -1;

                        if (newItem.isValid())
                        {
                            int pos = -1;

                            for (int i = newTradesItems->size() - 1; i >= 0; i--)
                            {
                                if (newTradesItems->at(i).date != newItem.date)
                                    break;

                                if (newTradesItems->at(i).orderType == newItem.orderType && newTradesItems->at(i).price == newItem.price)
                                {
                                    pos = i;
                                    break;
                                }
                            }

                            if (pos == -1)
                                (*newTradesItems) << newItem;
                            else
                            {
                                (*newTradesItems)[pos].amount += newItem.amount;
                            }

                            //(*newTradesItems)<<newItem;
                        }
                        else
                        {
                            if (debugLevel)
                                logThread->writeLog("Invalid trades fetch data line:" + tradeData, 2);
                        }
                    }

                    if (newTradesItems->count())
                    {
                        if (lastTickerDate < newItem.date)
                        {
                            lastTickerDate = newItem.date;
                            IndicatorEngine::setValue(baseValues.exchangeName, baseValues.currentPair.symbol, "Last", newTradesItems->last().price);
                        }

                        emit addLastTrades(baseValues.currentPair.symbol, newTradesItems);
                    }
                    else
                        delete newTradesItems;
                }
                else
                    success = false;

                break;//trades

            case 111: //depth
                if (data.startsWith("{\"asks\":["))
                {
                    emit depthRequestReceived();

                    if (lastDepthData != data)
                    {
                        lastDepthData = data;
                        depthAsks = new QList<DepthItem>;
                        depthBids = new QList<DepthItem>;

                        QMap<double, double> currentAsksMap;
                        QStringList asksList = QString(getMidData("asks\":[[", "]]", &data)).split("],[");
                        double groupedPrice = 0.0;
                        double groupedVolume = 0.0;
                        int rowCounter = 0;

                        for (int n = 0; n < asksList.count(); n++)
                        {
                            if (baseValues.depthCountLimit && rowCounter >= baseValues.depthCountLimit)
                                break;

                            QStringList currentPair = asksList.at(n).split(",");

                            if (currentPair.count() != 2)
                                continue;

                            double priceDouble = currentPair.first().toDouble();
                            double amount = currentPair.last().toDouble();

                            if (baseValues.groupPriceValue > 0.0)
                            {
                                if (n == 0)
                                {
                                    emit depthFirstOrder(baseValues.currentPair.symbol, priceDouble, amount, true);
                                    groupedPrice = baseValues.groupPriceValue * (int)(priceDouble / baseValues.groupPriceValue);
                                    groupedVolume = amount;
                                }
                                else
                                {
                                    bool matchCurrentGroup = priceDouble < groupedPrice + baseValues.groupPriceValue;

                                    if (matchCurrentGroup)
                                        groupedVolume += amount;

                                    if (!matchCurrentGroup || n == asksList.count() - 1)
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

                        for (int n = 0; n < currentAsksList.count(); n++)
                            if (currentAsksMap.value(currentAsksList.at(n), 0) == 0)
                                depthUpdateOrder(baseValues.currentPair.symbol,
                                                 currentAsksList.at(n), 0.0, true);

                        lastDepthAsksMap = currentAsksMap;

                        QMap<double, double> currentBidsMap;
                        QStringList bidsList = QString(getMidData("bids\":[[", "]]", &data)).split("],[");
                        groupedPrice = 0.0;
                        groupedVolume = 0.0;
                        rowCounter = 0;

                        for (int n = 0; n < bidsList.count(); n++)
                        {
                            if (baseValues.depthCountLimit && rowCounter >= baseValues.depthCountLimit)
                                break;

                            QStringList currentPair = bidsList.at(n).split(",");

                            if (currentPair.count() != 2)
                                continue;

                            double priceDouble = currentPair.first().toDouble();
                            double amount = currentPair.last().toDouble();

                            if (baseValues.groupPriceValue > 0.0)
                            {
                                if (n == 0)
                                {
                                    emit depthFirstOrder(baseValues.currentPair.symbol, priceDouble, amount, false);
                                    groupedPrice = baseValues.groupPriceValue * (int)(priceDouble / baseValues.groupPriceValue);
                                    groupedVolume = amount;
                                }
                                else
                                {
                                    bool matchCurrentGroup = priceDouble > groupedPrice - baseValues.groupPriceValue;

                                    if (matchCurrentGroup)
                                        groupedVolume += amount;

                                    if (!matchCurrentGroup || n == asksList.count() - 1)
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

                        for (int n = 0; n < currentBidsList.count(); n++)
                            if (currentBidsMap.value(currentBidsList.at(n), 0) == 0)
                                depthUpdateOrder(baseValues.currentPair.symbol,
                                                 currentBidsList.at(n), 0.0, false);

                        lastDepthBidsMap = currentBidsMap;

                        emit depthSubmitOrders(baseValues.currentPair.symbol, depthAsks, depthBids);
                        depthAsks = nullptr;
                        depthBids = nullptr;
                    }
                }
                else
                    success = false;

                break;

            case 202: //info
                if (data.startsWith("{\"result\":true,\"info\":{"))
                {
                    QByteArray fundsData  = getMidData("free\":{", "}", &data);
                    QByteArray btcBalance = getMidData(baseValues.currentPair.currAStrLow + "\":\"", "\"", &fundsData);
                    QByteArray usdBalance = getMidData(baseValues.currentPair.currBStrLow + "\":\"", "\"", &fundsData);

                    if (checkValue(btcBalance, lastBtcBalance))
                        emit accBtcBalanceChanged(baseValues.currentPair.symbol, lastBtcBalance);

                    if (checkValue(usdBalance, lastUsdBalance))
                        emit accUsdBalanceChanged(baseValues.currentPair.symbol, lastUsdBalance);
                }
                else
                    success = false;

                break;//info

            case 204://orders
                if (data.startsWith("{\"orders\":["))
                {
                    if (lastOrders != data)
                    {
                        lastOrders = data;

                        if (data.startsWith("{\"orders\":[]"))
                        {
                            emit ordersIsEmpty();
                            break;
                        }

                        QString ordersData = getMidData("{\"orders\":[", "]", &data);
                        QStringList ordersList = ordersData.split("},{");

                        if (ordersList.count() == 0)
                            return;

                        QList<OrderItem>* orders = new QList<OrderItem>;

                        for (int n = 0; n < ordersList.count(); n++)
                        {
                            OrderItem currentOrder;
                            QByteArray currentOrderData = ordersList.at(n).toLatin1();

                            currentOrder.oid = getMidData("\"order_id\":", ",", &currentOrderData);
                            currentOrder.date = getMidData("\"create_date\":", "000,", &currentOrderData).toUInt();
                            currentOrder.type = getMidData("\"type\":\"", "\"", &currentOrderData) == "sell";
                            currentOrder.status = getMidData("\"status\":", ",", &currentOrderData).toInt() + 1;
                            currentOrder.amount = getMidData("\"amount\":", ",", &currentOrderData).toDouble();
                            currentOrder.price = getMidData("\"price\":", ",", &currentOrderData).toDouble();
                            currentOrder.symbol = getMidData("\"symbol\":\"", "\"", &currentOrderData).toUpper().replace("_", "");

                            if (currentOrder.isValid())
                                (*orders) << currentOrder;
                        }

                        emit orderBookChanged(baseValues.currentPair.symbol, orders);
                    }
                }
                else
                    success = false;

                break;//orders

            case 305: //order/cancel
                if (data.startsWith("{\"order_id\":"))
                {
                    QByteArray oid = getMidData("order_id\":", ",", &data);

                    if (!oid.isEmpty())
                        emit orderCanceled(baseValues.currentPair.symbol, oid);
                }
                else
                    success = false;

                break;//order/cancel

            case 306:
                if (debugLevel)
                    logThread->writeLog("Buy OK: " + data, 2);

                break;//order/buy

            case 307:
                if (debugLevel)
                    logThread->writeLog("Sell OK: " + data, 2);

                break;//order/sell

            case 208: ///history
                if (data.startsWith("{\"current_page\":"))
                {
                    if (lastHistory != data)
                    {
                        lastHistory = data;

                        QStringList dataList = QString(data).split("},{");

                        if (dataList.count() == 0)
                            return;

                        qint64 currentId;
                        qint64 maxId = 0;
                        QList<HistoryItem>* historyItems = new QList<HistoryItem>;

                        for (int n = 0; n < dataList.count(); n++)
                        {
                            QByteArray curLog(dataList.at(n).toLatin1());

                            if (getMidData("\"status\":", ",", &curLog) != "2")
                                continue;

                            currentId = getMidData("\"order_id\":", ",", &curLog).toULongLong();

                            if (currentId <= lastHistoryId)
                                break;

                            if (n == 0)
                                maxId = currentId;

                            HistoryItem currentHistoryItem;
                            QByteArray logType = getMidData("\"type\":\"", "\"", &curLog);

                            if (logType == "sell")
                                currentHistoryItem.type = 1;
                            else if (logType == "buy")
                                currentHistoryItem.type = 2;

                            if (currentHistoryItem.type)
                            {
                                if (currentHistoryItem.type == 1 || currentHistoryItem.type == 2)
                                    currentHistoryItem.symbol = getMidData("\"symbol\":\"", "\"", &curLog).toUpper().replace("_", "");

                                currentHistoryItem.dateTimeInt = getMidData("\"create_date\":", "000,", &curLog).toUInt();
                                currentHistoryItem.price = getMidData("\"price\":", ",", &curLog).toDouble();
                                currentHistoryItem.volume = getMidData("\"amount\":", ",", &curLog).toDouble();

                                if (currentHistoryItem.isValid())
                                    (*historyItems) << currentHistoryItem;
                            }
                        }

                        if (maxId > lastHistoryId)
                            lastHistoryId = maxId;

                        emit historyChanged(historyItems);
                    }
                }
                else
                    success = false;

                break;//money/wallet/history

            default:
                break;
        }

    static int errorCount = 0;

    if (!success)
    {
        errorCount++;

        if (errorCount > 2)
        {
            if (debugLevel)
                logThread->writeLog("API error: invalid data (ReqType: " + QByteArray::number(reqType) + "):\n" + data, 2);

            if (reqType < 300)
                emit showErrorMessage("I:>API error: invalid data (ReqType: " + QByteArray::number(reqType) + ")");
        }
    }
    else
        errorCount = 0;

    if (reqType >= 200 && reqType < 300)
    {
        static int authErrorCount = 0;

        if (!success && data.startsWith("{\"error_code\":"))
        {
            authErrorCount++;

            if (authErrorCount > 2)
            {
                qint64 errorCode = getMidData("error_code\":", ",", &data).toLongLong();
                QString authError = "Error code: " + QByteArray::number(errorCode) + ".";

                if (debugLevel)
                    logThread->writeLog("API error: " + authError.toLatin1() + " ReqType: " + QByteArray::number(reqType), 2);

                if (errorCode == 10005 || errorCode == 10007 || errorCode == 10017)
                    authError.prepend(julyTr("TRUNAUTHORIZED", "Invalid API key.") + "<br>");

                emit showErrorMessage(authError);
            }
        }
        else
            authErrorCount = 0;
    }
}

void Exchange_OKCoin::depthUpdateOrder(QString symbol, double price, double amount, bool isAsk)
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

void Exchange_OKCoin::depthSubmitOrder(QString symbol, QMap<double, double>* currentMap, double priceDouble,
                                       double amount, bool isAsk)
{
    if (symbol != baseValues.currentPair.symbol)
        return;

    if (priceDouble == 0.0 || amount == 0.0)
        return;

    if (isAsk)
    {
        (*currentMap)[priceDouble] = amount;

        if (lastDepthAsksMap.value(priceDouble, 0.0) != amount)
            depthUpdateOrder(symbol, priceDouble, amount, true);
    }
    else
    {
        (*currentMap)[priceDouble] = amount;

        if (lastDepthBidsMap.value(priceDouble, 0.0) != amount)
            depthUpdateOrder(symbol, priceDouble, amount, false);
    }
}

bool Exchange_OKCoin::isReplayPending(int reqType)
{
    if (julyHttp == nullptr)
        return false;

    return julyHttp->isReqTypePending(reqType);
}

void Exchange_OKCoin::secondSlot()
{
    static int sendCounter = 0;

    switch (sendCounter)
    {
        case 0:
            if (!isReplayPending(103))
                sendToApi(103, "ticker.do?symbol=" + baseValues.currentPair.currRequestPair);

            break;

        case 1:
            if (!isReplayPending(202))
                sendToApi(202, "userinfo.do", true, "api_key=" + getApiKey());

            break;

        case 2:
            if (!isReplayPending(109))
                sendToApi(109, "trades.do?symbol=" + baseValues.currentPair.currRequestPair + "&since=" + QByteArray::number(
                              lastFetchTid));

            break;

        case 3:
            if (!tickerOnly && !isReplayPending(204))
                sendToApi(204, "order_info.do", true,
                          "api_key=" + getApiKey() + "&order_id=-1&symbol=" + baseValues.currentPair.currRequestPair);

            break;

        case 4:
            if (isDepthEnabled() && (forceDepthLoad || !isReplayPending(111)))
            {
                emit depthRequested();
                sendToApi(111, "depth.do?symbol=" + baseValues.currentPair.currRequestPair + "&size=" + baseValues.depthCountLimitStr);
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

void Exchange_OKCoin::getHistory(bool force)
{
    if (tickerOnly)
        return;

    if (force)
        lastHistory.clear();

    if (!isReplayPending(208))
        sendToApi(208, "order_history.do", true,
                  "api_key=" + getApiKey() + "&current_page=1&page_length=200&status=1&symbol=" + baseValues.currentPair.currRequestPair);
}

void Exchange_OKCoin::buy(QString symbol, double apiBtcToBuy, double apiPriceToBuy)
{
    if (tickerOnly)
        return;

    CurrencyPairItem pairItem;
    pairItem = baseValues.currencyPairMap.value(symbol, pairItem);

    if (pairItem.symbol.isEmpty())
        return;

    QByteArray data = "amount=" + JulyMath::byteArrayFromDouble(apiBtcToBuy, pairItem.currADecimals, 0) + "&api_key=" + getApiKey();
    data.append("&price=" + JulyMath::byteArrayFromDouble(apiPriceToBuy, pairItem.priceDecimals, 0));
    data.append("&symbol=" + pairItem.currRequestPair + "&type=buy");

    if (debugLevel)
        logThread->writeLog("Buy: " + data, 2);

    sendToApi(306, "trade.do", true, data);
}

void Exchange_OKCoin::sell(QString symbol, double apiBtcToSell, double apiPriceToSell)
{
    if (tickerOnly)
        return;

    CurrencyPairItem pairItem;
    pairItem = baseValues.currencyPairMap.value(symbol, pairItem);

    if (pairItem.symbol.isEmpty())
        return;

    QByteArray data = "amount=" + JulyMath::byteArrayFromDouble(apiBtcToSell, pairItem.currADecimals, 0) + "&api_key=" + getApiKey();
    data.append("&price=" + JulyMath::byteArrayFromDouble(apiPriceToSell, pairItem.priceDecimals, 0));
    data.append("&symbol=" + pairItem.currRequestPair + "&type=sell");

    if (debugLevel)
        logThread->writeLog("Sell: " + data, 2);

    sendToApi(307, "trade.do", true, data);
}

void Exchange_OKCoin::cancelOrder(QString, QByteArray order)
{
    if (tickerOnly)
        return;

    order.prepend("api_key=" + getApiKey() + "&order_id=");
    order.append("&symbol=" + baseValues.currentPair.currRequestPair);

    if (debugLevel)
        logThread->writeLog("Cancel order: " + order, 2);

    sendToApi(305, "cancel_order.do", true, order);
}

void Exchange_OKCoin::sendToApi(int reqType, QByteArray method, bool auth, QByteArray commands)
{
    if (julyHttp == nullptr)
    {
        if (domain.isEmpty() || port == 0)
            julyHttp = new JulyHttp("www.okcoin.cn", "Key: " + getApiKey() + "\r\n", this);
        else
        {
            julyHttp = new JulyHttp(domain, "Key: " + getApiKey() + "\r\n", this, useSsl);
            julyHttp->setPortForced(port);
        }

        connect(julyHttp, SIGNAL(anyDataReceived()), baseValues_->mainWindow_, SLOT(anyDataReceived()));
        connect(julyHttp, SIGNAL(apiDown(bool)), baseValues_->mainWindow_, SLOT(setApiDown(bool)));
        connect(julyHttp, SIGNAL(setDataPending(bool)), baseValues_->mainWindow_, SLOT(setDataPending(bool)));
        connect(julyHttp, SIGNAL(errorSignal(QString)), baseValues_->mainWindow_, SLOT(showErrorMessage(QString)));
        connect(julyHttp, SIGNAL(sslErrorSignal(const QList<QSslError>&)), this, SLOT(sslErrors(const QList<QSslError>&)));
        connect(julyHttp, SIGNAL(dataReceived(QByteArray, int)), this, SLOT(dataReceivedAuth(QByteArray, int)));
    }

    if (auth)
    {
        commands += "&sign=" + QCryptographicHash::hash(commands + "&secret_key=" + getApiSign(),
                    QCryptographicHash::Md5).toHex().toUpper();
        julyHttp->sendData(reqType, "POST /api/v1/" + method, commands);
    }
    else
    {
        julyHttp->sendData(reqType, "GET /api/v1/" + method);
    }
}

void Exchange_OKCoin::sslErrors(const QList<QSslError>& errors)
{
    QStringList errorList;

    for (int n = 0; n < errors.count(); n++)
        errorList << errors.at(n).errorString();

    if (debugLevel)
        logThread->writeLog(errorList.join(" ").toLatin1(), 2);

    emit showErrorMessage("SSL Error: " + errorList.join(" "));
}
