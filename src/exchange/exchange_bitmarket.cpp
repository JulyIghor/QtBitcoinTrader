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
#include "exchange_bitmarket.h"

Exchange_BitMarket::Exchange_BitMarket(QByteArray pRestSign, QByteArray pRestKey)
    : Exchange()
{
    calculatingFeeMode = 1;
    clearOpenOrdersOnCurrencyChanged = true;
    clearHistoryOnCurrencyChanged = true;
    baseValues.exchangeName = "BitMarket";
    baseValues.currentPair.name = "BTC/PLN";
    baseValues.currentPair.setSymbol("BTCPLN");
    baseValues.currentPair.currRequestPair = "btc_pln";
    baseValues.currentPair.priceDecimals = 3;
    minimumRequestIntervalAllowed = 700;
    baseValues.currentPair.priceMin = qPow(0.1, baseValues.currentPair.priceDecimals);
    baseValues.currentPair.tradeVolumeMin = 0.01;
    baseValues.currentPair.tradePriceMin = 0.1;
    depthAsks = nullptr;
    depthBids = nullptr;
    forceDepthLoad = false;
    julyHttp = nullptr;
    isApiDown = false;
    tickerOnly = false;
    setApiKeySecret(pRestKey, pRestSign);

    currencyMapFile = "BitMarket";
    defaultCurrencyParams.currADecimals = 8;
    defaultCurrencyParams.currBDecimals = 8;
    defaultCurrencyParams.currABalanceDecimals = 8;
    defaultCurrencyParams.currBBalanceDecimals = 8;
    defaultCurrencyParams.priceDecimals = 3;
    defaultCurrencyParams.priceMin = qPow(0.1, baseValues.currentPair.priceDecimals);

    supportsLoginIndicator = false;
    supportsAccountVolume = false;

    authRequestTime.restart();
    privateNonce = (TimeSync::getTimeT() - 1371854884) * 10;

    connect(this, &Exchange::threadFinished, this, &Exchange_BitMarket::quitThread, Qt::DirectConnection);
}

Exchange_BitMarket::~Exchange_BitMarket()
{
}

void Exchange_BitMarket::quitThread()
{
    clearValues();

    if (depthAsks)
        delete depthAsks;

    if (depthBids)
        delete depthBids;

    if (julyHttp)
        delete julyHttp;
}

void Exchange_BitMarket::clearVariables()
{
    cancelingOrderIDs.clear();
    Exchange::clearVariables();
    lastOpenedOrders = -1;
    apiDownCounter = 0;
    lastHistoryId = 0;
    lastHistoryCount = 0;
    lastHistory.clear();
    lastOrders.clear();
    reloadDepth();
    lastFetchTid = TimeSync::getTimeT() - 600;
    lastFetchTid = -lastFetchTid;
    lastTickerDate = 0;
    lastTradesTid.clear();
}

void Exchange_BitMarket::clearValues()
{
    clearVariables();

    if (julyHttp)
        julyHttp->clearPendingData();
}

void Exchange_BitMarket::reloadDepth()
{
    lastDepthBidsMap.clear();
    lastDepthAsksMap.clear();
    lastDepthData.clear();
    Exchange::reloadDepth();
}

void Exchange_BitMarket::dataReceivedAuth(QByteArray data, int reqType)
{
    if (debugLevel)
        logThread->writeLog("RCV: " + data);

    if (data.size() && data.at(0) == QLatin1Char('<'))
        return;

    bool success = true;
    QString errorString = "";

    switch (reqType)
    {
        case 103: //ticker
            if (data.startsWith("{\"ask\":"))
            {
                QByteArray tickerHigh = getMidData("\"high\":", ",\"", &data);

                if (!tickerHigh.isEmpty())
                {
                    double newTickerHigh = tickerHigh.toDouble();

                    if (newTickerHigh != lastTickerHigh)
                        IndicatorEngine::setValue(baseValues.exchangeName, baseValues.currentPair.symbol, "High", newTickerHigh);

                    lastTickerHigh = newTickerHigh;
                }

                QByteArray tickerLow = getMidData("\"low\":", ",\"", &data);

                if (!tickerLow.isEmpty())
                {
                    double newTickerLow = tickerLow.toDouble();

                    if (newTickerLow != lastTickerLow)
                        IndicatorEngine::setValue(baseValues.exchangeName, baseValues.currentPair.symbol, "Low", newTickerLow);

                    lastTickerLow = newTickerLow;
                }

                QByteArray tickerSell = getMidData("\"bid\":", ",\"", &data);

                if (!tickerSell.isEmpty())
                {
                    double newTickerSell = tickerSell.toDouble();

                    if (newTickerSell != lastTickerSell)
                        IndicatorEngine::setValue(baseValues.exchangeName, baseValues.currentPair.symbol, "Sell", newTickerSell);

                    lastTickerSell = newTickerSell;
                }

                QByteArray tickerBuy = getMidData("\"ask\":", ",\"", &data);

                if (!tickerBuy.isEmpty())
                {
                    double newTickerBuy = tickerBuy.toDouble();

                    if (newTickerBuy != lastTickerBuy)
                        IndicatorEngine::setValue(baseValues.exchangeName, baseValues.currentPair.symbol, "Buy", newTickerBuy);

                    lastTickerBuy = newTickerBuy;
                }

                QByteArray tickerVolume = getMidData("\"volume\":", "}", &data);

                if (!tickerVolume.isEmpty())
                {
                    double newTickerVolume = tickerVolume.toDouble();

                    if (newTickerVolume != lastTickerVolume)
                        IndicatorEngine::setValue(baseValues.exchangeName, baseValues.currentPair.symbol, "Volume", newTickerVolume);

                    lastTickerVolume = newTickerVolume;
                }

                QByteArray tickerLast = getMidData("\"last\":", ",\"", &data);

                if (!tickerLast.isEmpty())
                {
                    double newTickerLast = tickerLast.toDouble();

                    if (newTickerLast != lastTickerLast)
                        IndicatorEngine::setValue(baseValues.exchangeName, baseValues.currentPair.symbol, "Last", newTickerLast);

                    lastTickerLast = newTickerLast;
                }
            }
            else
            {
                success = false;
                errorString += "Invalid ticker data. ";
            }

            break;//ticker

        case 109: //trades
            if (data.startsWith("[{\"amount\":"))
            {
                QStringList tradeList = QString(data).split("},{");
                QList<TradesItem>* newTradesItems = new QList<TradesItem>;

                qint64 currentTid = 0;

                for (int n = tradeList.count() - 1; n >= 0; n--)
                {
                    QByteArray tradeData = tradeList.at(n).toLatin1() + "}";
                    TradesItem newItem;

                    newItem.date = getMidData("\"date\":", ",\"", &tradeData).toLongLong();
                    currentTid = getMidData("\"tid\":", ",\"", &tradeData).toUInt();

                    if (lastFetchTid < 0 && newItem.date < -lastFetchTid)
                        continue;

                    if (currentTid < 1000 || lastFetchTid >= currentTid)
                        continue;

                    lastFetchTid = currentTid;
                    newItem.price = getMidData("\"price\":", ",\"", &tradeData).toDouble();
                    newItem.amount = getMidData("\"amount\":", ",\"", &tradeData).toDouble();
                    newItem.orderType = getMidData("\"type\":\"", "\"", &tradeData) == "ask" ? 1 : -1;
                    newItem.symbol = baseValues.currentPair.symbol;

                    if (newItem.isValid())
                        (*newTradesItems) << newItem;
                    else if (debugLevel)
                        logThread->writeLog("Invalid trades fetch data line:" + tradeData, 2);

                    if (n == 0 && lastTickerDate < newItem.date)
                    {
                        lastTickerDate = newItem.date;
                        IndicatorEngine::setValue(baseValues.exchangeName, baseValues.currentPair.symbol, "Last", newItem.price);
                    }
                }

                if (newTradesItems->count())
                    emit addLastTrades(baseValues.currentPair.symbol, newTradesItems);
                else
                    delete newTradesItems;

                if (currentTid > 1000)
                    lastTradesTid = QByteArray::number(currentTid);
            }
            else if (data != "[]")
            {
                success = false;
                errorString += "Invalid trades data. ";
            }

            break;//trades

        case 111: //depth
            if (data.startsWith("{\"asks\":[["))
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
            {
                success = false;
                errorString += "Invalid depth data. ";

                if (debugLevel)
                    logThread->writeLog("Invalid depth data:" + data, 2);
            }

            break;

        case 202: //info
            if (data.startsWith("{\"success\":true,\"data\":{\"balances\":{\"available\":"))
            {
                QByteArray fundsData = getMidData("available\":{", "}", &data) + ",";
                QByteArray btcBalance = getMidData("\"" + baseValues.currentPair.currAStr + "\":", ",", &fundsData);
                QByteArray usdBalance = getMidData("\"" + baseValues.currentPair.currBStr + "\":", ",", &fundsData);

                if (checkValue(btcBalance, lastBtcBalance))
                    emit accBtcBalanceChanged(baseValues.currentPair.symbol, lastBtcBalance);

                if (checkValue(usdBalance, lastUsdBalance))
                    emit accUsdBalanceChanged(baseValues.currentPair.symbol, lastUsdBalance);
            }
            else
            {
                success = false;
                errorString += "Invalid info data. ";
            }

            break;//info

        case 204://orders
            if (data.startsWith("{\"success\":true,\"data\":{\"buy\":["))
            {
                if (lastOrders != data)
                {
                    lastOrders = data;

                    if (data.startsWith("{\"success\":true,\"data\":{\"buy\":[],\"sell\":[]}"))
                    {
                        emit ordersIsEmpty();
                        break;
                    }

                    QByteArray dataBuy = getMidData("\"buy\":[{", "}],", &data) + "},{" + getMidData("\"sell\":[{", "}]", &data);
                    QStringList ordersList = QString(dataBuy).split("},{");

                    if (ordersList.count() == 0)
                        return;

                    QList<OrderItem>* orders = new QList<OrderItem>;

                    for (int n = 0; n < ordersList.count(); n++)
                    {
                        OrderItem currentOrder;
                        QByteArray currentOrderData = ordersList.at(n).toLatin1() + "}";

                        currentOrder.oid = getMidData("\"id\":", ",", &currentOrderData);
                        currentOrder.date = getMidData("\"time\":", "}", &currentOrderData).toUInt();
                        currentOrder.type = getMidData("\"type\":\"", "\"", &currentOrderData) == "sell";
                        currentOrder.amount = getMidData("\"amount\":", ",", &currentOrderData).toDouble();
                        currentOrder.price = getMidData("\"rate\":", ",", &currentOrderData).toDouble();
                        currentOrder.symbol = getMidData("\"market\":\"", "\"", &currentOrderData);
                        currentOrder.status = 1;

                        if (currentOrder.isValid())
                            (*orders) << currentOrder;
                    }

                    emit orderBookChanged(baseValues.currentPair.symbol, orders);
                }
            }
            else
            {
                success = false;
                errorString += "Invalid orders data. ";
            }

            break;//orders

        case 305: //order/cancel
            {
                if (!cancelingOrderIDs.isEmpty())
                {
                    if (data.startsWith("{\"success\":true"))
                        emit orderCanceled(baseValues.currentPair.symbol, cancelingOrderIDs.first());

                    if (debugLevel)
                        logThread->writeLog("Order canceled:" + cancelingOrderIDs.first(), 2);

                    cancelingOrderIDs.removeFirst();
                }

                dataReceivedAuth(data, 202);
            }
            break;//order/cancel

        case 306:
            {
                if (debugLevel)
                    logThread->writeLog("Buy OK: " + data, 2);

                if (data.startsWith("{\"error\""))
                {
                    success = false;
                    errorString += "Invalid order/buy data. ";
                }
            }
            break;//order/buy

        case 307:
            {
                if (debugLevel)
                    logThread->writeLog("Sell OK: " + data, 2);

                if (data.startsWith("{\"error\""))
                {
                    success = false;
                    errorString += "Invalid order/sell data. ";
                }
            }
            break;//order/sell

        case 208: ///history
            if (data.startsWith("{\"success\":true,\"data\":{\"total\":"))
            {
                QByteArray historyData = getMidData("\"results\":[{", "}]}", &data) + "^";

                if (lastHistory != historyData)
                {
                    lastHistory = historyData;

                    if (historyData == "^")
                        break;

                    qint64 count = getMidData("total\":", ",", &data).toInt();

                    if (count <= lastHistoryCount)
                        break;

                    lastHistoryCount = count;

                    QString newLog(historyData);
                    QStringList dataList = newLog.split("},{");

                    if (dataList.count() == 0)
                        return;

                    qint64 currentId;
                    qint64 maxId = 0;
                    QList<HistoryItem>* historyItems = new QList<HistoryItem>;

                    for (int n = 0; n < dataList.count(); n++)
                    {
                        QByteArray curLog(dataList.at(n).toLatin1());

                        currentId = getMidData("id\":", ",", &curLog).toUInt();

                        if (currentId <= lastHistoryId)
                            break;

                        if (n == 0)
                            maxId = currentId;

                        HistoryItem currentHistoryItem;
                        QByteArray logType = getMidData("type\":\"", "\"", &curLog);

                        if (logType == "sell")
                            currentHistoryItem.type = 1;
                        else if (logType == "buy")
                            currentHistoryItem.type = 2;

                        if (currentHistoryItem.type)
                        {
                            currentHistoryItem.symbol = getMidData("currencyCrypto\":\"", "\"", &curLog) + getMidData("currencyFiat\":\"", "\"",
                                                        &curLog);
                            currentHistoryItem.dateTimeInt = getMidData("time\":", ",", &curLog).toUInt();
                            currentHistoryItem.price = getMidData("rate\":", ",", &curLog).toDouble();
                            currentHistoryItem.volume = getMidData("amountCrypto\":", ",", &curLog).toDouble();

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
            {
                success = false;
                errorString += "Invalid history data. ";
            }

            break;//money/wallet/history

        default:
            break;
    }

    if (reqType >= 200 && reqType < 300)
    {
        static int authErrorCount = 0;

        if (!success)
        {
            authErrorCount++;

            if (authErrorCount > 2)
            {
                QString authErrorString = getMidData("errorMsg\":\"", "\"", &data);

                if (debugLevel)
                    logThread->writeLog("API error: " + authErrorString.toLatin1() + " ReqType: " + QByteArray::number(reqType), 2);

                if (authErrorString == "Invalid API key")
                    authErrorString = julyTr("TRUNAUTHORIZED", "Invalid API key.");
                else if (authErrorString == "Invalid nonce value")
                    authErrorString = julyTr("THIS_PROFILE_ALREADY_USED", "Invalid nonce parameter.");

                if (!authErrorString.isEmpty())
                    emit showErrorMessage(authErrorString);
            }
        }
        else
            authErrorCount = 0;
    }

    static int errorCount = 0;

    if (!success)
    {
        errorCount++;

        if (errorCount < 2)
            return;

        QByteArray errorMsg = getMidData("errorMsg\":\"", "\"", &data);
        errorString += "Error message: " + errorMsg;

        if (debugLevel)
            logThread->writeLog("API error: " + errorString.toLatin1() + " ReqType:" + QByteArray::number(reqType), 2);

        if (errorMsg == "Invalid nonce value" || errorMsg == "")
            return;

        emit showErrorMessage("I:>" + errorString);
    }
    else
        errorCount = 0;
}

void Exchange_BitMarket::depthUpdateOrder(QString symbol, double price, double amount, bool isAsk)
{
    if (symbol != baseValues.currentPair.symbol)
        return;

    if (isAsk)
    {
        if (depthAsks == 0)
            return;

        DepthItem newItem;
        newItem.price = price;
        newItem.volume = amount;

        if (newItem.isValid())
            (*depthAsks) << newItem;
    }
    else
    {
        if (depthBids == 0)
            return;

        DepthItem newItem;
        newItem.price = price;
        newItem.volume = amount;

        if (newItem.isValid())
            (*depthBids) << newItem;
    }
}

void Exchange_BitMarket::depthSubmitOrder(QString symbol, QMap<double, double>* currentMap, double priceDouble,
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

bool Exchange_BitMarket::isReplayPending(int reqType)
{
    if (julyHttp == 0)
        return false;

    return julyHttp->isReqTypePending(reqType);
}

void Exchange_BitMarket::secondSlot()
{
    static int sendCounter = 0;

    switch (sendCounter)
    {
        case 0:
            if (!isReplayPending(103))
                sendToApi(103, "ticker.json", false, true);

            break;

        case 1:
            if (!isReplayPending(202))
                sendToApi(202, "info", true, true);

            break;

        case 2:
            if (!isReplayPending(109))
                sendToApi(109, "trades.json?since=" + lastTradesTid, false, true);

            break;

        case 3:
            if (!tickerOnly && !isReplayPending(204))
                sendToApi(204, "orders&market=" + baseValues.currentPair.symbol.toLatin1(), true, true);

            break;

        case 4:
            if (isDepthEnabled() && (forceDepthLoad || !isReplayPending(111)))
            {
                emit depthRequested();
                sendToApi(111, "orderbook.json", false, true);
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

void Exchange_BitMarket::getHistory(bool force)
{
    if (tickerOnly)
        return;

    if (force)
        lastHistory.clear();

    if (!isReplayPending(208))
        sendToApi(208, "trades&market=" +
                  baseValues.currentPair.symbol.toLatin1()/*+"&start="+QByteArray::number(lastHistoryCount)*/, true, true);
}

void Exchange_BitMarket::buy(QString symbol, double apiBtcToBuy, double apiPriceToBuy)
{
    if (tickerOnly)
        return;

    CurrencyPairItem pairItem;
    pairItem = baseValues.currencyPairMap.value(symbol, pairItem);

    if (pairItem.symbol.isEmpty())
        return;

    QByteArray data = "trade&market=" + symbol.toLatin1() + "&type=buy&amount=" + JulyMath::byteArrayFromDouble(apiBtcToBuy,
                      pairItem.currADecimals, 0) + "&rate=" + JulyMath::byteArrayFromDouble(apiPriceToBuy, pairItem.priceDecimals, 0);

    if (debugLevel)
        logThread->writeLog("Buy: " + data, 2);

    sendToApi(306, data, true, true);
}

void Exchange_BitMarket::sell(QString symbol, double apiBtcToSell, double apiPriceToSell)
{
    if (tickerOnly)
        return;

    CurrencyPairItem pairItem;
    pairItem = baseValues.currencyPairMap.value(symbol, pairItem);

    if (pairItem.symbol.isEmpty())
        return;

    QByteArray data = "trade&market=" + symbol.toLatin1() + "&type=sell&amount=" + JulyMath::byteArrayFromDouble(apiBtcToSell,
                      pairItem.currADecimals, 0) + "&rate=" + JulyMath::byteArrayFromDouble(apiPriceToSell, pairItem.priceDecimals, 0);

    if (debugLevel)
        logThread->writeLog("Sell: " + data, 2);

    sendToApi(307, data, true, true);
}

void Exchange_BitMarket::cancelOrder(QString, QByteArray order)
{
    if (tickerOnly)
        return;

    cancelingOrderIDs << order;

    order.prepend("cancel&id=");

    if (debugLevel)
        logThread->writeLog("Cancel order: " + order, 2);

    sendToApi(305, order, true, true);
}

void Exchange_BitMarket::sendToApi(int reqType, QByteArray method, bool auth, bool sendNow)
{
    if (julyHttp == 0)
    {
        if (domain.isEmpty() || port == 0)
            julyHttp = new JulyHttp("www.bitmarket.pl", "API-Key: " + getApiKey() + "\r\n", this);
        else
        {
            julyHttp = new JulyHttp(domain, "API-Key: " + getApiKey() + "\r\n", this, useSsl);
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
        QByteArray postData = "method=" + method + "&tonce=" + QByteArray::number(TimeSync::getTimeT()) +
                              "&nonce=" + QByteArray::number(++privateNonce);

        if (sendNow)
            julyHttp->sendData(reqType, "POST /api2/", postData, "API-Hash: " + hmacSha512(getApiSign(),
                               postData).toHex() + "\r\n");
        else
            julyHttp->prepareData(reqType, "POST /api2/", postData, "API-Hash: " + hmacSha512(getApiSign(),
                                  postData).toHex() + "\r\n");
    }
    else
    {
        QByteArray data = "GET /json/" + baseValues.currentPair.symbol.toLatin1() + "/" + method;

        if (sendNow)
            julyHttp->sendData(reqType, data);
        else
            julyHttp->prepareData(reqType, data);
    }
}

void Exchange_BitMarket::sslErrors(const QList<QSslError>& errors)
{
    QStringList errorList;

    for (int n = 0; n < errors.count(); n++)
        errorList << errors.at(n).errorString();

    if (debugLevel)
        logThread->writeLog(errorList.join(" ").toLatin1(), 2);

    emit showErrorMessage("SSL Error: " + errorList.join(" "));
}
