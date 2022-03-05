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

#include "exchange_bitstamp.h"
#include "iniengine.h"
#include "timesync.h"

Exchange_Bitstamp::Exchange_Bitstamp(const QByteArray& pRestSign, const QByteArray& pRestKey) : Exchange()
{
    clearOpenOrdersOnCurrencyChanged = true;
    checkDuplicatedOID = true;
    accountFee = 0.0;
    minimumRequestIntervalAllowed = 1200;
    calculatingFeeMode = 1;
    isLastTradesTypeSupported = false;
    lastBidAskTimestamp = 0;
    baseValues.exchangeName = "Bitstamp";
    baseValues.currentPair.name = "BTC/USD";
    baseValues.currentPair.setSymbol("BTCUSD");
    baseValues.currentPair.currRequestPair = "BTCUSD";
    baseValues.currentPair.priceDecimals = 2;
    baseValues.currentPair.priceMin = qPow(0.1, baseValues.currentPair.priceDecimals);
    baseValues.currentPair.tradeVolumeMin = 0.01;
    baseValues.currentPair.tradePriceMin = 0.1;
    depthAsks = nullptr;
    depthBids = nullptr;
    forceDepthLoad = false;
    julyHttp = nullptr;
    tickerOnly = false;
    setApiKeySecret(pRestKey.split(':').last(), pRestSign);
    privateClientId = pRestKey.split(':').first();

    currencyMapFile = "Bitstamp";
    defaultCurrencyParams.currADecimals = 8;
    defaultCurrencyParams.currBDecimals = 5;
    defaultCurrencyParams.currABalanceDecimals = 8;
    defaultCurrencyParams.currBBalanceDecimals = 5;
    defaultCurrencyParams.priceDecimals = 2;
    defaultCurrencyParams.priceMin = qPow(0.1, baseValues.currentPair.priceDecimals);

    supportsLoginIndicator = true;
    supportsAccountVolume = false;

    privateNonce = (TimeSync::getTimeT() - 1371854884) * 10;

    connect(this, &Exchange::threadFinished, this, &Exchange_Bitstamp::quitThread, Qt::DirectConnection);
}

Exchange_Bitstamp::~Exchange_Bitstamp()
{
}

void Exchange_Bitstamp::quitThread()
{
    clearValues();

    delete depthAsks;

    delete depthBids;

    delete julyHttp;
}
void Exchange_Bitstamp::filterAvailableUSDAmountValue(double* amount)
{
    double decValue = JulyMath::cutDoubleDecimalsCopy((*amount) * mainWindow.floatFee, baseValues.currentPair.priceDecimals, false);
    decValue += qPow(0.1, qMax(baseValues.currentPair.priceDecimals, 1));
    *amount = JulyMath::cutDoubleDecimalsCopy((*amount) - decValue, baseValues.currentPair.currBDecimals, false);
}

void Exchange_Bitstamp::clearVariables()
{
    cancelingOrderIDs.clear();
    Exchange::clearVariables();
    secondPart = 0;
    apiDownCounter = 0;
    lastHistory.clear();
    lastOrders.clear();
    reloadDepth();
    lastInfoReceived = false;
    lastBidAskTimestamp = 0;
    lastTradesDate = TimeSync::getTimeT() - 600;
    lastTickerDate = 0;
}

void Exchange_Bitstamp::clearValues()
{
    clearVariables();

    if (julyHttp)
        julyHttp->clearPendingData();
}

void Exchange_Bitstamp::secondSlot()
{
    static int sendCounter = 0;

    switch (sendCounter)
    {
    case 0:
        if (!isReplayPending(103))
            sendToApi(103, "v2/ticker/" + baseValues.currentPair.currRequestPair.toLower() + "/", false, true);

        break;

    case 1:
        if (!isReplayPending(202))
            sendToApi(202, "v2/balance/", true, true);

        break;

    case 2:
        if (!isReplayPending(109))
            sendToApi(109, "v2/transactions/" + baseValues.currentPair.currRequestPair.toLower() + "/", false, true);

        break;

    case 3:
        if (!tickerOnly && !isReplayPending(204))
            sendToApi(204, "v2/open_orders/" + baseValues.currentPair.currRequestPair.toLower() + "/", true, true);

        break;

    case 4:
        if (isDepthEnabled() && (forceDepthLoad || !isReplayPending(111)))
        {
            emit depthRequested();
            sendToApi(111, "v2/order_book/" + baseValues.currentPair.currRequestPair.toLower() + "/", false, true);
            forceDepthLoad = false;
        }

        break;

    case 5:
        if (lastHistory.isEmpty() && !isReplayPending(208))
            sendToApi(208, "v2/user_transactions/", true, true);

        break;

    default:
        break;
    }

    if (sendCounter++ >= 5)
        sendCounter = 0;

    Exchange::secondSlot();
}

bool Exchange_Bitstamp::isReplayPending(int reqType)
{
    if (julyHttp == nullptr)
        return false;

    return julyHttp->isReqTypePending(reqType);
}

void Exchange_Bitstamp::getHistory(bool force)
{
    if (tickerOnly)
        return;

    if (force)
        lastHistory.clear();

    if (!isReplayPending(208))
        sendToApi(208, "v2/user_transactions/", true, true);
}

void Exchange_Bitstamp::buy(const QString& symbol, double apiBtcToBuy, double apiPriceToBuy)
{
    if (tickerOnly)
        return;

    CurrencyPairItem pairItem;
    pairItem = baseValues.currencyPairMap.value(symbol, pairItem);

    if (pairItem.symbol.isEmpty())
        return;

    QByteArray params = "amount=" + JulyMath::byteArrayFromDouble(apiBtcToBuy, pairItem.currADecimals, 0) +
                        "&price=" + JulyMath::byteArrayFromDouble(apiPriceToBuy, pairItem.priceDecimals, 0);

    if (debugLevel)
        logThread->writeLog("Buy: " + params, 2);

    sendToApi(306, "v2/buy/" + baseValues.currentPair.currRequestPair.toLower() + "/", true, true, params);
}

void Exchange_Bitstamp::sell(const QString& symbol, double apiBtcToSell, double apiPriceToSell)
{
    if (tickerOnly)
        return;

    CurrencyPairItem pairItem;
    pairItem = baseValues.currencyPairMap.value(symbol, pairItem);

    if (pairItem.symbol.isEmpty())
        return;

    QByteArray params = "amount=" + JulyMath::byteArrayFromDouble(apiBtcToSell, pairItem.currADecimals, 0) +
                        "&price=" + JulyMath::byteArrayFromDouble(apiPriceToSell, pairItem.priceDecimals, 0);

    if (debugLevel)
        logThread->writeLog("Sell: " + params, 2);

    sendToApi(307, "v2/sell/" + baseValues.currentPair.currRequestPair.toLower() + "/", true, true, params);
}

void Exchange_Bitstamp::cancelOrder(const QString& /*unused*/, const QByteArray& order)
{
    if (tickerOnly)
        return;

    cancelingOrderIDs << order;

    if (debugLevel)
        logThread->writeLog("Cancel order: " + order, 2);

    sendToApi(305, "v2/cancel_order/", true, true, "id=" + order);
}

void Exchange_Bitstamp::sendToApi(int reqType, const QByteArray& method, bool auth, bool sendNow, QByteArray commands)
{
    if (julyHttp == nullptr)
    {
        if (domain.isEmpty() || port == 0)
            julyHttp = new JulyHttp("www.bitstamp.net", "", this);
        else
        {
            julyHttp = new JulyHttp(domain, "", this, useSsl);
            julyHttp->setPortForced(port);
        }

        connect(julyHttp, &JulyHttp::anyDataReceived, baseValues_->mainWindow_, &QtBitcoinTrader::anyDataReceived);
        connect(julyHttp, &JulyHttp::setDataPending, baseValues_->mainWindow_, &QtBitcoinTrader::setDataPending);
        connect(julyHttp, &JulyHttp::apiDown, baseValues_->mainWindow_, &QtBitcoinTrader::setApiDown);
        connect(julyHttp, &JulyHttp::errorSignal, baseValues_->mainWindow_, &QtBitcoinTrader::showErrorMessage);
        connect(julyHttp, &JulyHttp::sslErrorSignal, this, &Exchange_Bitstamp::sslErrors);
        connect(julyHttp, &JulyHttp::dataReceived, this, &Exchange_Bitstamp::dataReceivedAuth);
    }

    if (auth)
    {
        QByteArray postData = QByteArray::number(++privateNonce);
        postData = "key=" + getApiKey() +
                   "&signature=" + hmacSha256(getApiSign(), QByteArray(postData + privateClientId + getApiKey())).toHex().toUpper() +
                   "&nonce=" + postData;

        if (!commands.isEmpty())
            postData.append("&" + commands);

        if (sendNow)
            julyHttp->sendData(reqType, m_pairChangeCount, "POST /api/" + method, postData);
        else
            julyHttp->prepareData(reqType, m_pairChangeCount, "POST /api/" + method, postData);
    }
    else
    {
        if (sendNow)
            julyHttp->sendData(reqType, m_pairChangeCount, "GET /api/" + method);
        else
            julyHttp->prepareData(reqType, m_pairChangeCount, "GET /api/" + method);
    }
}

void Exchange_Bitstamp::depthUpdateOrder(const QString& symbol, double price, double amount, bool isAsk)
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

void Exchange_Bitstamp::depthSubmitOrder(const QString& symbol,
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

void Exchange_Bitstamp::reloadDepth()
{
    lastDepthBidsMap.clear();
    lastDepthAsksMap.clear();
    lastDepthData.clear();
    Exchange::reloadDepth();
}

void Exchange_Bitstamp::dataReceivedAuth(const QByteArray& data, int reqType, int pairChangeCount)
{
    if (pairChangeCount != m_pairChangeCount)
        return;

    if (debugLevel)
        logThread->writeLog("RCV: " + data);

    if (data.size() && data.at(0) == QLatin1Char('<'))
        return;

    bool success = ((!data.startsWith("{\"error\"") && (data.startsWith("{"))) || data.startsWith("[")) || data == "true" ||
                   data == "false";

    switch (reqType)
    {
    case 103: // ticker
        if (!success)
            break;

        if (data.startsWith("{\"high\":"))
        {
            double tickerHigh = getMidData("\"high\": \"", "\"", &data).toDouble();

            if (tickerHigh > 0.0 && !qFuzzyCompare(tickerHigh, lastTickerHigh))
            {
                IndicatorEngine::setValue(baseValues.exchangeName, baseValues.currentPair.symbol, "High", tickerHigh);
                lastTickerHigh = tickerHigh;
            }

            double tickerLow = getMidData("\"low\": \"", "\"", &data).toDouble();

            if (tickerLow > 0.0 && !qFuzzyCompare(tickerLow, lastTickerLow))
            {
                IndicatorEngine::setValue(baseValues.exchangeName, baseValues.currentPair.symbol, "Low", tickerLow);
                lastTickerLow = tickerLow;
            }

            double tickerVolume = getMidData("\"volume\": \"", "\"", &data).toDouble();

            if (tickerVolume > 0.0 && !qFuzzyCompare(tickerVolume, lastTickerVolume))
            {
                IndicatorEngine::setValue(baseValues.exchangeName, baseValues.currentPair.symbol, "Volume", tickerVolume);
                lastTickerVolume = tickerVolume;
            }

            qint64 tickerTimestamp = getMidData("\"timestamp\": \"", "\"", &data).toUInt();

            if (tickerTimestamp > lastBidAskTimestamp)
            {
                double tickerSell = getMidData("\"bid\": \"", "\"", &data).toDouble();

                if (tickerSell > 0.0 && !qFuzzyCompare(tickerSell, lastTickerSell))
                {
                    IndicatorEngine::setValue(baseValues.exchangeName, baseValues.currentPair.symbol, "Sell", tickerSell);
                    lastTickerSell = tickerSell;
                }

                double tickerBuy = getMidData("\"ask\": \"", "\"", &data).toDouble();

                if (tickerBuy > 0.0 && !qFuzzyCompare(tickerBuy, lastTickerBuy))
                {
                    IndicatorEngine::setValue(baseValues.exchangeName, baseValues.currentPair.symbol, "Buy", tickerBuy);
                    lastTickerBuy = tickerBuy;
                }

                lastBidAskTimestamp = tickerTimestamp;
            }

            if (tickerTimestamp > lastTickerDate)
            {
                double tickerLast = getMidData("\"last\": \"", "\"", &data).toDouble();

                if (tickerLast > 0.0 && !qFuzzyCompare(tickerLast, lastTickerLast))
                {
                    IndicatorEngine::setValue(baseValues.exchangeName, baseValues.currentPair.symbol, "Last", tickerLast);
                    lastTickerLast = tickerLast;
                }

                lastTickerDate = tickerTimestamp;
            }
        }
        else if (debugLevel)
            logThread->writeLog("Invalid ticker data:" + data, 2);

        break; // ticker

    case 109: // api/transactions
        if (success && data.size() > 32)
        {
            if (data.startsWith("[{\"date\":"))
            {
                QStringList tradeList = QString(data).split("}, {");
                auto* newTradesItems = new QList<TradesItem>;

                for (int n = tradeList.size() - 1; n >= 0; n--)
                {
                    QByteArray tradeData = tradeList.at(n).toLatin1();
                    TradesItem newItem;
                    newItem.date = getMidData("\"date\": \"", "\"", &tradeData).toLongLong();

                    if (newItem.date <= lastTradesDate)
                        continue;

                    newItem.amount = getMidData("\"amount\": \"", "\"", &tradeData).toDouble();
                    newItem.price = getMidData("\"price\": \"", "\"", &tradeData).toDouble();
                    newItem.orderType = getMidData("\"type\": \"", "\"", &tradeData).toInt() == 1 ? 1 : -1;

                    if (n == 0 && newItem.price > 0.0)
                    {
                        lastTradesDate = newItem.date;

                        if (lastTickerDate < newItem.date)
                        {
                            lastTickerDate = newItem.date;
                            IndicatorEngine::setValue(baseValues.exchangeName, baseValues.currentPair.symbol, "Last", newItem.price);
                        }
                    }

                    newItem.symbol = baseValues.currentPair.symbol;

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
            else if (debugLevel)
                logThread->writeLog("Invalid trades fetch data:" + data, 2);
        }

        break;

    case 111: // api/order_book
        if (data.startsWith("{\"timestamp\":"))
        {
            emit depthRequestReceived();

            if (lastDepthData != data)
            {
                lastDepthData = data;
                depthAsks = new QList<DepthItem>;
                depthBids = new QList<DepthItem>;

                qint64 tickerTimestamp = getMidData("\"timestamp\": \"", "\"", &data).toUInt();
                QMap<double, double> currentAsksMap;
                QStringList asksList = QString(getMidData("\"asks\": [[", "]]", &data)).split("], [");
                double groupedPrice = 0.0;
                double groupedVolume = 0.0;
                int rowCounter = 0;
                bool updateTicker = tickerTimestamp > lastBidAskTimestamp;

                if (updateTicker)
                    lastBidAskTimestamp = tickerTimestamp;

                for (int n = 0; n < asksList.size(); n++)
                {
                    if (baseValues.depthCountLimit && rowCounter >= baseValues.depthCountLimit)
                        break;

                    QByteArray currentRow = asksList.at(n).toLatin1();
                    double priceDouble = getMidData("\"", "\"", &currentRow).toDouble();
                    double amount = getMidData(", \"", "\"", &currentRow).toDouble();

                    if (n == 0 && updateTicker)
                        IndicatorEngine::setValue(baseValues.exchangeName, baseValues.currentPair.symbol, "Buy", priceDouble);

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
                        depthUpdateOrder(baseValues.currentPair.symbol, currentAsksList.at(n), 0.0, true); // Remove price

                lastDepthAsksMap = currentAsksMap;

                QMap<double, double> currentBidsMap;
                QStringList bidsList = QString(getMidData("\"bids\": [[", "]]", &data)).split("], [");
                groupedPrice = 0.0;
                groupedVolume = 0.0;
                rowCounter = 0;

                for (int n = 0; n < bidsList.size(); n++)
                {
                    if (baseValues.depthCountLimit && rowCounter >= baseValues.depthCountLimit)
                        break;

                    QByteArray currentRow = bidsList.at(n).toLatin1();
                    double priceDouble = getMidData("\"", "\"", &currentRow).toDouble();
                    double amount = getMidData(", \"", "\"", &currentRow).toDouble();

                    if (n == 0 && updateTicker)
                        IndicatorEngine::setValue(baseValues.exchangeName, baseValues.currentPair.symbol, "Sell", priceDouble);

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

                            if (!matchCurrentGroup || n == bidsList.size() - 1)
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
                        depthUpdateOrder(baseValues.currentPair.symbol, currentBidsList.at(n), 0.0, false); // Remove price

                lastDepthBidsMap = currentBidsMap;

                emit depthSubmitOrders(baseValues.currentPair.symbol, depthAsks, depthBids);
                depthAsks = nullptr;
                depthBids = nullptr;
            }
        }
        else if (debugLevel)
            logThread->writeLog("Invalid depth data:" + data, 2);

        break;

    case 202: // balance
        {
            if (!success)
                break;

            if (data.startsWith("{\""))
            {
                lastInfoReceived = true;
                QByteArray accFee = getMidData(baseValues.currentPair.currRequestPair.toLower() + "_fee\": \"", "\"", &data);
                QByteArray btcBalance = getMidData("\"" + baseValues.currentPair.currAStrLow + "_available\": \"", "\"", &data);
                QByteArray usdBalance = getMidData("\"" + baseValues.currentPair.currBStrLow + "_available\": \"", "\"", &data);

                if (checkValue(accFee, accountFee))
                    emit accFeeChanged(baseValues.currentPair.symbol, accountFee);

                if (checkValue(btcBalance, lastBtcBalance))
                    emit accBtcBalanceChanged(baseValues.currentPair.symbol, lastBtcBalance);

                if (checkValue(usdBalance, lastUsdBalance))
                    emit accUsdBalanceChanged(baseValues.currentPair.symbol, lastUsdBalance);

                static bool balanceSent = false;

                if (!balanceSent)
                {
                    balanceSent = true;
                    emit loginChanged(privateClientId);
                }
            }
            else if (debugLevel)
                logThread->writeLog("Invalid Info data:" + data, 2);
        }
        break; // balance

    case 204: // open_orders
        if (!success)
            break;

        if (data == "[]")
        {
            lastOrders.clear();
            emit ordersIsEmpty();
            break;
        }

        if (data.startsWith("[") && data.endsWith("]"))
        {
            if (lastOrders != data)
            {
                lastOrders = data;

                QStringList ordersList = QString(data.size() > 3 ? data.mid(2, data.size() - 4) : data).split("}, {");
                auto* orders = new QList<OrderItem>;

                for (int n = 0; n < ordersList.size(); n++)
                {
                    OrderItem currentOrder;
                    QByteArray currentOrderData = ordersList.at(n).toLatin1();
                    currentOrder.oid = getMidData("\"id\": \"", "\",", &currentOrderData);

                    QByteArray dateTimeData = getMidData("\"datetime\": \"", "\"", &currentOrderData);
                    QDateTime orderDateTime = QDateTime::fromString(dateTimeData, "yyyy-MM-dd HH:mm:ss");
                    orderDateTime.setTimeSpec(Qt::UTC);
                    currentOrder.date = orderDateTime.toSecsSinceEpoch();
                    currentOrder.type = getMidData("\"type\": \"", "\",", &currentOrderData) == "1";
                    currentOrder.status = 1;
                    currentOrder.amount = getMidData("\"amount\": \"", "\"", &currentOrderData).toDouble();
                    currentOrder.price = getMidData("\"price\": \"", "\"", &currentOrderData).toDouble();
                    currentOrder.symbol = baseValues.currentPair.symbol;

                    if (currentOrder.isValid())
                        (*orders) << currentOrder;
                }

                emit orderBookChanged(baseValues.currentPair.symbol, orders);
                lastInfoReceived = false;
            }
        }
        else if (debugLevel)
            logThread->writeLog("Invalid Orders data:" + data, 2);

        break; // open_orders

    case 305: // cancel_order
        {
            if (!success)
                break;

            if (data.contains("\"id\""))
            {
                QByteArray id = getMidData("\"id\": ", ",", &data);

                if (id.size())
                    emit orderCanceled(baseValues.currentPair.symbol, id);

                if (debugLevel)
                    logThread->writeLog("Order canceled:" + id, 2);
            }

            if (!cancelingOrderIDs.isEmpty())
            {
                if (data == "true")
                    emit orderCanceled(baseValues.currentPair.symbol, cancelingOrderIDs.first());

                if (debugLevel)
                    logThread->writeLog("Order canceled:" + cancelingOrderIDs.first(), 2);

                cancelingOrderIDs.removeFirst();
            }
        }
        break; // cancel_order

    case 306: // order/buy
        if (!success || !debugLevel)
            break;

        if (data.startsWith("{\"result\":\"success\",\"data\":\""))
            logThread->writeLog("Buy OK: " + data);
        else
            logThread->writeLog("Invalid Order Buy Data:" + data);

        break; // order/buy

    case 307: // order/sell
        if (!success || !debugLevel)
            break;

        if (data.startsWith("{\"result\":\"success\",\"data\":\""))
            logThread->writeLog("Sell OK: " + data);
        else
            logThread->writeLog("Invalid Order Sell Data:" + data);

        break; // order/sell

    case 208: // user_transactions
        if (!success)
            break;

        if (data.startsWith("["))
        {
            if (lastHistory != data)
            {
                lastHistory = data;

                if (data == "[]")
                    break;

                auto* historyItems = new QList<HistoryItem>;
                QString newLog(data);
                QStringList dataList = newLog.split("}, {");
                newLog.clear();

                for (int n = 0; n < dataList.size(); n++)
                {
                    HistoryItem currentHistoryItem;
                    QByteArray curLog(dataList.at(n).toLatin1());
                    QString firstCurrency = "";

                    QDateTime orderDateTime =
                        QDateTime::fromString(getMidData("\"datetime\": \"", "\"", &curLog).left(19), "yyyy-MM-dd HH:mm:ss");
                    orderDateTime.setTimeSpec(Qt::UTC);
                    currentHistoryItem.dateTimeInt = orderDateTime.toSecsSinceEpoch();

                    int logTypeInt = getMidData("\"type\": \"", "\"", &curLog).toInt();
                    QString bufferCurrency;
                    QStringList bufferCurrencies;
                    QList<CurrencyPairItem>* pairList = IniEngine::getPairs();

                    if (logTypeInt == 0 || logTypeInt == 1)
                    {
                        for (int m = 0; m < pairList->size(); ++m)
                        {
                            bufferCurrency = pairList->at(m).currAStrLow;

                            if (!bufferCurrencies.contains(bufferCurrency))
                            {
                                bufferCurrencies.append(bufferCurrency);
                                QByteArray volStr = getMidData("\"" + bufferCurrency + "\": \"", "\"", &curLog);

                                if (volStr.startsWith("-"))
                                    volStr.remove(0, 1);

                                double bufferVolume = volStr.toDouble();

                                if (bufferVolume > 0.0)
                                {
                                    bufferCurrency = bufferCurrency.toUpper();
                                    currentHistoryItem.volume = bufferVolume;
                                    currentHistoryItem.symbol = bufferCurrency + bufferCurrency;
                                    break;
                                }
                            }

                            bufferCurrency = pairList->at(m).currBStrLow;

                            if (!bufferCurrencies.contains(bufferCurrency))
                            {
                                bufferCurrencies.append(bufferCurrency);
                                QByteArray volStr = getMidData("\"" + bufferCurrency + "\": \"", "\"", &curLog);

                                if (volStr.startsWith("-"))
                                    volStr.remove(0, 1);

                                double bufferVolume = volStr.toDouble();

                                if (bufferVolume > 0.0)
                                {
                                    bufferCurrency = bufferCurrency.toUpper();
                                    currentHistoryItem.volume = bufferVolume;
                                    currentHistoryItem.symbol = bufferCurrency + bufferCurrency;
                                    break;
                                }
                            }
                        }

                        if (logTypeInt == 0)
                            currentHistoryItem.type = 4; // Deposit
                        else if (logTypeInt == 1)
                            currentHistoryItem.type = 5; // Withdrawal
                    }
                    else if (logTypeInt == 2) // Market Trade
                    {
                        for (int m = 0; m < pairList->size(); ++m)
                        {
                            QString request = pairList->at(m).currAStrLow + "_" + pairList->at(m).currBStrLow;

                            if (request.size() < 5)
                                continue;

                            if (curLog.indexOf(request.toLatin1()) != -1)
                            {
                                currentHistoryItem.price = getMidData(request + "\": ", ",", &curLog).toDouble();
                                currentHistoryItem.symbol = pairList->at(m).symbol;
                                firstCurrency = request.left(request.indexOf('_'));
                                break;
                            }
                        }

                        if (firstCurrency.isEmpty())
                            continue;

                        QByteArray btcAmount = getMidData("\"" + firstCurrency + "\": \"", "\"", &curLog);
                        bool negativeAmount = btcAmount.startsWith("-");

                        if (negativeAmount)
                            btcAmount.remove(0, 1);

                        currentHistoryItem.volume = btcAmount.toDouble();

                        if (negativeAmount)
                            currentHistoryItem.type = 1; // Sell
                        else
                            currentHistoryItem.type = 2; // Buy
                    }
                    else
                        continue;

                    if (currentHistoryItem.isValid())
                        (*historyItems) << currentHistoryItem;
                }

                emit historyChanged(historyItems);
            }
        }
        else if (debugLevel)
            logThread->writeLog("Invalid History data:" + data.left(200), 2);

        break; // user_transactions

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
                QString authErrorString = getMidData("error\": \"", "\"", &data);

                if (debugLevel)
                    logThread->writeLog("API error: " + authErrorString.toLatin1() + " ReqType: " + QByteArray::number(reqType), 2);

                if (authErrorString == "API key not found")
                    authErrorString = julyTr("TRUNAUTHORIZED", "Invalid API key.");
                else if (authErrorString == "Invalid nonce")
                    authErrorString = julyTr("THIS_PROFILE_ALREADY_USED", "Invalid nonce parameter.");

                if (!authErrorString.isEmpty())
                    emit showErrorMessage(authErrorString);
            }
        }
        else
            authErrorCount = 0;
    }

    static int errorCount = 0;

    if (!success && reqType != 305)
    {
        errorCount++;
        QString errorString;
        bool invalidMessage = !data.startsWith("{");

        if (!invalidMessage)
        {
            errorString = getMidData("[\"", "\"]", &data);

            if (errorString.isEmpty())
            {
                QByteArray nErrorString = getMidData("{\"error\":", "}", &data);
                errorString = getMidData("\"", "\"", &nErrorString);
            }
        }
        else
            errorString = data;

        if (debugLevel)
            logThread->writeLog("API Error: " + errorString.toLatin1() + " ReqType:" + QByteArray::number(reqType), 2);

        if (errorCount < 3 && reqType < 300 && errorString != "Invalid username and/or password")
            return;

        if (errorString.isEmpty())
            return;

        errorString.append("<br>" + QString::number(reqType));

        if (invalidMessage || reqType < 300)
            emit showErrorMessage("I:>" + errorString);
    }
    else
        errorCount = 0;
}

void Exchange_Bitstamp::sslErrors(const QList<QSslError>& errors)
{
    QStringList errorList;

    for (int n = 0; n < errors.size(); n++)
        errorList << errors.at(n).errorString();

    if (debugLevel)
        logThread->writeLog(errorList.join(" ").toLatin1(), 2);

    emit showErrorMessage("SSL Error: " + errorList.join(" "));
}
