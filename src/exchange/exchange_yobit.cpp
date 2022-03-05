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
#include "exchange_yobit.h"

Exchange_YObit::Exchange_YObit(const QByteArray& pRestSign, const QByteArray& pRestKey)
    : Exchange()
{
    calculatingFeeMode = 1;
    buySellAmountExcludedFee = true;
    clearOpenOrdersOnCurrencyChanged = true;
    clearHistoryOnCurrencyChanged = true;
    baseValues.exchangeName = "YObit";
    baseValues.currentPair.name = "BTC/USD";
    baseValues.currentPair.setSymbol("BTC/USD");
    baseValues.currentPair.currRequestPair = "btc_usd";
    baseValues.currentPair.priceDecimals = 3;
    minimumRequestIntervalAllowed = 500;
    baseValues.currentPair.priceMin = qPow(0.1, baseValues.currentPair.priceDecimals);
    baseValues.currentPair.tradeVolumeMin = 0.0001;
    baseValues.currentPair.tradePriceMin = 0.00000001;
    depthAsks = nullptr;
    depthBids = nullptr;
    forceDepthLoad = false;
    julyHttp = nullptr;
    isApiDown = false;
    tickerOnly = false;
    setApiKeySecret(pRestKey, pRestSign);

    currencyMapFile = "YObit";
    defaultCurrencyParams.currADecimals = 8;
    defaultCurrencyParams.currBDecimals = 8;
    defaultCurrencyParams.currABalanceDecimals = 8;
    defaultCurrencyParams.currBBalanceDecimals = 8;
    defaultCurrencyParams.priceDecimals = 8;
    defaultCurrencyParams.priceMin = qPow(0.1, baseValues.currentPair.priceDecimals);

    supportsLoginIndicator = false;
    supportsAccountVolume = false;

    privateNonce = (TimeSync::getTimeT() - 1371854884) * 10;

    while (privateNonce > 2140000000)
        privateNonce -= 2140000000;

    lastHistoryId = 0;

    QSettings mainSettings(appDataDir + "/QtBitcoinTrader.cfg", QSettings::IniFormat);
    useAltDomain = mainSettings.value("UseAlternateDomainForYobit", false).toBool();

    connect(this, &Exchange::threadFinished, this, &Exchange_YObit::quitThread, Qt::DirectConnection);
}

Exchange_YObit::~Exchange_YObit()
{
}

void Exchange_YObit::quitThread()
{
    clearValues();

    
        delete depthAsks;

    
        delete depthBids;

    
        delete julyHttp;
}

void Exchange_YObit::clearVariables()
{
    isFirstAccInfo = true;
    Exchange::clearVariables();
    lastOpenedOrders = -1;
    apiDownCounter = 0;
    lastHistoryId = 0;
    lastHistory.clear();
    lastOrders.clear();
    reloadDepth();
    lastFetchTid = TimeSync::getTimeT() - 600;
    lastFetchTid = -lastFetchTid;
    lastTickerDate = 0;
}

void Exchange_YObit::clearValues()
{
    clearVariables();

    if (julyHttp)
        julyHttp->clearPendingData();
}

void Exchange_YObit::reloadDepth()
{
    lastDepthBidsMap.clear();
    lastDepthAsksMap.clear();
    lastDepthData.clear();
    Exchange::reloadDepth();
}

void Exchange_YObit::dataReceivedAuth(const QByteArray& data, int reqType, int pairChangeCount)
{
    if (pairChangeCount != m_pairChangeCount)
        return;

    if (debugLevel)
        logThread->writeLog("RCV: " + data);

    if (data.size() && data.at(0) == QLatin1Char('<'))
        return;

    bool success = !data.startsWith("{\"success\":0");
    QString errorString;

    if (!success)
        errorString = getMidData("error\":\"", "\"", &data);


    switch (reqType)
    {
    case 103: //ticker
        {
            QByteArray tickerHigh = getMidData("high\":", ",\"", &data);

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

            QByteArray tickerSell = getMidData("\"buy\":", ",\"", &data);

            if (!tickerSell.isEmpty())
            {
                double newTickerSell = tickerSell.toDouble();

                if (newTickerSell != lastTickerSell)
                    IndicatorEngine::setValue(baseValues.exchangeName, baseValues.currentPair.symbol, "Sell", newTickerSell);

                lastTickerSell = newTickerSell;
            }

            QByteArray tickerBuy = getMidData("\"sell\":", ",\"", &data);

            if (!tickerBuy.isEmpty())
            {
                double newTickerBuy = tickerBuy.toDouble();

                if (newTickerBuy != lastTickerBuy)
                    IndicatorEngine::setValue(baseValues.exchangeName, baseValues.currentPair.symbol, "Buy", newTickerBuy);

                lastTickerBuy = newTickerBuy;
            }

            QByteArray tickerVolume = getMidData("\"vol_cur\":", ",\"", &data);

            if (!tickerVolume.isEmpty())
            {
                double newTickerVolume = tickerVolume.toDouble();

                if (newTickerVolume != lastTickerVolume)
                    IndicatorEngine::setValue(baseValues.exchangeName, baseValues.currentPair.symbol, "Volume", newTickerVolume);

                lastTickerVolume = newTickerVolume;
            }

            qint64 newTickerDate = getMidData("\"updated\":", "}", &data).toUInt();

            if (lastTickerDate < newTickerDate)
            {
                lastTickerDate = newTickerDate;
                QByteArray tickerLast = getMidData("\"last\":", ",\"", &data);
                double tickerLastDouble = tickerLast.toDouble();

                if (tickerLastDouble > 0.0)
                    IndicatorEngine::setValue(baseValues.exchangeName, baseValues.currentPair.symbol, "Last", tickerLastDouble);
            }
        }
        break;//ticker

    case 109: //trades
        {
            if (data.size() < 10)
                break;

            QByteArray currentRequestSymbol = getMidData("\"", "\":[{", &data).toUpper().replace("_", "/");
            QStringList tradeList = QString(data).split("},{");
            auto* newTradesItems = new QList<TradesItem>;

            for (int n = tradeList.size() - 1; n >= 0; n--)
            {
                QByteArray tradeData = tradeList.at(n).toLatin1() + "}";
                TradesItem newItem;
                newItem.date = getMidData("timestamp\":", "}", &tradeData).toLongLong();
                newItem.price = getMidData("\"price\":", ",\"", &tradeData).toDouble();

                if (lastFetchTid < 0 && newItem.date < -lastFetchTid)
                    continue;

                qint64 currentTid = getMidData("\"tid\":", ",\"", &tradeData).toUInt();

                if (currentTid < 1000 || lastFetchTid >= currentTid)
                    continue;

                lastFetchTid = currentTid;

                if (n == 0 && lastTickerDate < newItem.date)
                {
                    lastTickerDate = newItem.date;
                    IndicatorEngine::setValue(baseValues.exchangeName, baseValues.currentPair.symbol, "Last", newItem.price);
                }

                newItem.amount = getMidData("\"amount\":", ",\"", &tradeData).toDouble();
                newItem.symbol = currentRequestSymbol;
                newItem.orderType = getMidData("\"type\":\"", "\"", &tradeData) == "ask" ? 1 : -1;

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

    case 110: //Fee
        {
            QStringList feeList = QString(getMidData("pairs\":{\"", "}}}", &data)).split("},\"");

            for (int n = 0; n < feeList.size(); n++)
            {
                if (!feeList.at(n).startsWith(baseValues.currentPair.currRequestPair))
                    continue;

                QByteArray currentFeeData = feeList.at(n).toLatin1() + ",";
                double newFee = getMidData("fee\":", ",", &currentFeeData).toDouble();

                if (newFee != lastFee)
                    emit accFeeChanged(baseValues.currentPair.symbol, newFee);

                lastFee = newFee;
            }
        }
        break;// Fee

    case 111: //depth
        if (data.startsWith("{\"" + baseValues.currentPair.currRequestPair + "\":{\"asks"))
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

                for (int n = 0; n < asksList.size(); n++)
                {
                    if (baseValues.depthCountLimit && rowCounter >= baseValues.depthCountLimit)
                        break;

                    QStringList currentPair = asksList.at(n).split(",");

                    if (currentPair.size() != 2)
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
                    if (currentAsksMap.value(currentAsksList.at(n), 0) == 0)
                        depthUpdateOrder(baseValues.currentPair.symbol,
                                         currentAsksList.at(n), 0.0, true);

                lastDepthAsksMap = currentAsksMap;

                QMap<double, double> currentBidsMap;
                QStringList bidsList = QString(getMidData("bids\":[[", "]]", &data)).split("],[");
                groupedPrice = 0.0;
                groupedVolume = 0.0;
                rowCounter = 0;

                for (int n = 0; n < bidsList.size(); n++)
                {
                    if (baseValues.depthCountLimit && rowCounter >= baseValues.depthCountLimit)
                        break;

                    QStringList currentPair = bidsList.at(n).split(",");

                    if (currentPair.size() != 2)
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
                    if (currentBidsMap.value(currentBidsList.at(n), 0) == 0)
                        depthUpdateOrder(baseValues.currentPair.symbol,
                                         currentBidsList.at(n), 0.0, false);

                lastDepthBidsMap = currentBidsMap;

                emit depthSubmitOrders(baseValues.currentPair.symbol, depthAsks, depthBids);
                depthAsks = nullptr;
                depthBids = nullptr;
            }
        }
        else if (debugLevel)
            logThread->writeLog("Invalid depth data:" + data, 2);

        break;

    case 202: //info
        {
            if (!success)
                break;

            QByteArray fundsData = getMidData("funds\":{", "}", &data) + ",";
            QByteArray btcBalance = getMidData("\"" + baseValues.currentPair.currAStrLow + "\":", ",", &fundsData);
            QByteArray usdBalance = getMidData("\"" + baseValues.currentPair.currBStrLow + "\":", ",", &fundsData);

            if (checkValue(btcBalance, lastBtcBalance))
                emit accBtcBalanceChanged(baseValues.currentPair.symbol, lastBtcBalance);

            if (checkValue(usdBalance, lastUsdBalance))
                emit accUsdBalanceChanged(baseValues.currentPair.symbol, lastUsdBalance);

            int openedOrders = getMidData("open_orders\":", ",\"", &data).toInt();

            if (openedOrders == 0 && lastOpenedOrders)
            {
                lastOrders.clear();
                emit ordersIsEmpty();
            }

            lastOpenedOrders = openedOrders;

            if (isFirstAccInfo)
            {
                QByteArray rights = getMidData("rights\":{", "}", &data);

                if (!rights.isEmpty())
                {
                    bool isRightsGood = rights.contains("info\":1") && rights.contains("trade\":1");

                    if (!isRightsGood)
                        emit showErrorMessage("I:>invalid_rights");

                    isFirstAccInfo = false;
                }
            }
        }
        break;//info

    case 204://orders
        {
            if (data.size() <= 5)
                break;

            bool isEmptyOrders = success && data.size() < 30;

            if (lastOrders != data)
            {
                lastOrders = data;

                if (isEmptyOrders)
                {
                    emit ordersIsEmpty();
                    break;
                }

                QStringList ordersList = QString(data).replace("return\":{\"", "},\"").split("},\"");

                if (!ordersList.empty())
                    ordersList.removeFirst();

                if (ordersList.empty())
                    return;

                auto* orders = new QList<OrderItem>;

                for (int n = 0; n < ordersList.size(); n++)
                {
                    OrderItem currentOrder;
                    QByteArray currentOrderData = "{" + ordersList.at(n).toLatin1() + "}";

                    currentOrder.oid = getMidData("{", "\":{", &currentOrderData);
                    currentOrder.date = getMidData("timestamp_created\":\"", "\"", &currentOrderData).toUInt();
                    currentOrder.type = getMidData("type\":\"", "\",\"", &currentOrderData) == "sell";
                    currentOrder.status = getMidData("status\":", "}", &currentOrderData).toInt() + 1;
                    currentOrder.amount = getMidData("amount\":", ",\"", &currentOrderData).toDouble();
                    currentOrder.price = getMidData("rate\":", ",\"", &currentOrderData).toDouble();
                    currentOrder.symbol = getMidData("pair\":\"", "\",\"", &currentOrderData).toUpper().replace("_", "/");

                    if (currentOrder.isValid())
                        (*orders) << currentOrder;
                }

                emit orderBookChanged(baseValues.currentPair.symbol, orders);
            }

            break;//orders
        }

    case 305: //order/cancel
        if (success)
        {
            QByteArray oid = getMidData("order_id\":", ",\"", &data);

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

    case 208: ///history
        {
            if (lastHistory != data)
            {
                lastHistory = data;

                if (!success)
                    break;

                QString newLog(getMidData("return\":{", "}}}", &data));
                QStringList dataList = newLog.split("},");

                if (dataList.empty())
                    return;

                newLog.clear();
                qint64 maxId = 0;
                auto* historyItems = new QList<HistoryItem>;

                for (int n = 0; n < dataList.size(); n++)
                {
                    QByteArray curLog("~" + dataList.at(n).toLatin1() + "~");

                    qint64 currentId = getMidData("~\"", "\":{", &curLog).toUInt();

                    if (currentId <= lastHistoryId)
                        break;

                    if (n == 0)
                        maxId = currentId;

                    HistoryItem currentHistoryItem;
                    QByteArray logType = getMidData("type\":\"", "\",\"", &curLog);

                    if (logType == "sell")
                        currentHistoryItem.type = 1;
                    else if (logType == "buy")
                        currentHistoryItem.type = 2;

                    if (currentHistoryItem.type)
                    {
                        if (currentHistoryItem.type == 1 || currentHistoryItem.type == 2)
                            currentHistoryItem.symbol = getMidData("pair\":\"", "\",\"", &curLog).toUpper().replace("_", "/");

                        currentHistoryItem.dateTimeInt = getMidData("timestamp\":\"", "\"", &curLog).toUInt();
                        currentHistoryItem.price = getMidData("rate\":", ",\"", &curLog).toDouble();
                        currentHistoryItem.volume = getMidData("amount\":", ",\"", &curLog).toDouble();

                        if (currentHistoryItem.isValid())
                            (*historyItems) << currentHistoryItem;
                    }
                }

                if (maxId > lastHistoryId)
                    lastHistoryId = maxId;

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

            if (authErrorCount > 2)
            {
                QString authErrorString = getMidData("error\":\"", "\"", &data);

                if (debugLevel)
                    logThread->writeLog("API error: " + authErrorString.toLatin1() + " ReqType: " + QByteArray::number(reqType), 2);

                if (authErrorString == "invalid api key")
                    authErrorString = julyTr("TRUNAUTHORIZED", "Invalid API key.");
                else if (authErrorString.startsWith("invalid nonce parameter"))
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

        if (errorCount < 3)
            return;

        if (debugLevel)
            logThread->writeLog("API error: " + errorString.toLatin1() + " ReqType: " + QByteArray::number(reqType), 2);

        if (errorString.isEmpty())
            return;

        if (errorString == QLatin1String("no orders"))
            return;

        if (reqType < 300)
            emit showErrorMessage("I:>" + errorString);
    }
    else
        errorCount = 0;
}

void Exchange_YObit::depthUpdateOrder(const QString& symbol, double price, double amount, bool isAsk)
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

void Exchange_YObit::depthSubmitOrder(const QString& symbol, QMap<double, double>* currentMap, double priceDouble,
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

bool Exchange_YObit::isReplayPending(int reqType)
{
    if (julyHttp == nullptr)
        return false;

    return julyHttp->isReqTypePending(reqType);
}

void Exchange_YObit::secondSlot()
{
    static int sendCounter = 0;

    switch (sendCounter)
    {
    case 0:
        if (!isReplayPending(103))
            sendToApi(103, "ticker/" + baseValues.currentPair.currRequestPair, false, true);

        break;

    case 1:
        if (!isReplayPending(202))
            sendToApi(202, "", true, true, "method=getInfo&");

        break;

    case 2:
        if (!isReplayPending(109))
            sendToApi(109, "trades/" + baseValues.currentPair.currRequestPair, false, true);

        break;

    case 3:
        if (!tickerOnly && !isReplayPending(204))
            sendToApi(204, "", true, true, "method=ActiveOrders&pair=" + baseValues.currentPair.currRequestPair + "&");

        break;

    case 4:
        if (isDepthEnabled() && (forceDepthLoad || !isReplayPending(111)))
        {
            emit depthRequested();
            sendToApi(111, "depth/" + baseValues.currentPair.currRequestPair + "?limit=" + baseValues.depthCountLimitStr, false,
                      true);
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

void Exchange_YObit::getHistory(bool force)
{
    if (tickerOnly)
        return;

    if (force)
        lastHistory.clear();

    if (!isReplayPending(208))
        sendToApi(208, "", true, true, "method=TradeHistory&pair=" + baseValues.currentPair.currRequestPair + "&");

    if (!isReplayPending(110))
        sendToApi(110, "info", false, true);
}

void Exchange_YObit::buy(const QString& symbol, double apiBtcToBuy, double apiPriceToBuy)
{
    if (tickerOnly)
        return;

    CurrencyPairItem pairItem;
    pairItem = baseValues.currencyPairMap.value(symbol, pairItem);

    if (pairItem.symbol.isEmpty())
        return;

    QByteArray data = "method=Trade&pair=" + pairItem.currRequestPair + "&type=buy&rate=" +
                      JulyMath::byteArrayFromDouble(apiPriceToBuy, pairItem.priceDecimals, 0) + "&amount=" +
                      JulyMath::byteArrayFromDouble(apiBtcToBuy, pairItem.currADecimals, 0) + "&";

    if (debugLevel)
        logThread->writeLog("Buy: " + data, 2);

    sendToApi(306, "", true, true, data);
}

void Exchange_YObit::sell(const QString& symbol, double apiBtcToSell, double apiPriceToSell)
{
    if (tickerOnly)
        return;

    CurrencyPairItem pairItem;
    pairItem = baseValues.currencyPairMap.value(symbol, pairItem);

    if (pairItem.symbol.isEmpty())
        return;

    QByteArray data = "method=Trade&pair=" + pairItem.currRequestPair + "&type=sell&rate=" +
                      JulyMath::byteArrayFromDouble(apiPriceToSell, pairItem.priceDecimals, 0) + "&amount=" +
                      JulyMath::byteArrayFromDouble(apiBtcToSell, pairItem.currADecimals, 0) + "&";

    if (debugLevel)
        logThread->writeLog("Sell: " + data, 2);

    sendToApi(307, "", true, true, data);
}

void Exchange_YObit::cancelOrder(const QString& /*unused*/, const QByteArray& orderData)
{
    if (tickerOnly)
        return;

    QByteArray order = orderData;
    order.prepend("method=CancelOrder&order_id=");
    order.append("&");

    if (debugLevel)
        logThread->writeLog("Cancel order: " + order, 2);

    sendToApi(305, "", true, true, order);
}

void Exchange_YObit::sendToApi(int reqType, const QByteArray& method, bool auth, bool sendNow, QByteArray commands)
{
    if (julyHttp == nullptr)
    {
        if (domain.isEmpty() || port == 0)
        {
            if (useAltDomain)
                julyHttp = new JulyHttp("yobitex.net", "Key: " + getApiKey() + "\r\n", this);
            else
                julyHttp = new JulyHttp("yobit.net", "Key: " + getApiKey() + "\r\n", this);
        }
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
        QByteArray postData = commands + "nonce=" + QByteArray::number(++privateNonce);

        if (sendNow)
            julyHttp->sendData(reqType, m_pairChangeCount, "POST /tapi/" + method, postData,
                               "Sign: " + hmacSha512(getApiSign(), postData).toHex() + "\r\n");
        else
            julyHttp->prepareData(reqType, m_pairChangeCount, "POST /tapi/" + method, postData,
                                  "Sign: " + hmacSha512(getApiSign(), postData).toHex() + "\r\n");
    }
    else
    {
        if (commands.isEmpty())
        {
            if (sendNow)
                julyHttp->sendData(reqType, m_pairChangeCount, "GET /api/3/" + method);
            else
                julyHttp->prepareData(reqType, m_pairChangeCount, "GET /api/3/" + method);
        }
        else
        {
            if (sendNow)
                julyHttp->sendData(reqType, m_pairChangeCount, "POST /api/3/" + method, commands);
            else
                julyHttp->prepareData(reqType, m_pairChangeCount, "POST /api/3/" + method, commands);
        }
    }
}

void Exchange_YObit::sslErrors(const QList<QSslError>& errors)
{
    QStringList errorList;

    for (int n = 0; n < errors.size(); n++)
        errorList << errors.at(n).errorString();

    if (debugLevel)
        logThread->writeLog(errorList.join(" ").toLatin1(), 2);

    emit showErrorMessage("SSL Error: " + errorList.join(" "));
}
