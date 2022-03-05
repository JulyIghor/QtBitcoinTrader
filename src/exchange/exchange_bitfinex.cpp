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

#include "exchange_bitfinex.h"
#include "iniengine.h"
#include "timesync.h"

Exchange_Bitfinex::Exchange_Bitfinex(const QByteArray& pRestSign, const QByteArray& pRestKey) : Exchange()
{
    orderBookItemIsDedicatedOrder = true;
    clearHistoryOnCurrencyChanged = true;
    isLastTradesTypeSupported = false;
    calculatingFeeMode = 1;
    historyLastTimestamp = "0";
    lastTradesDate = 0;
    tickerLastDate = 0;
    isFirstAccInfo = true;
    lastInfoReceived = false;
    apiDownCounter = 0;
    secondPart = 0;
    baseValues.exchangeName = "Bitfinex";

    setApiKeySecret(pRestKey, pRestSign);

    depthAsks = nullptr;
    depthBids = nullptr;
    forceDepthLoad = false;
    julyHttp = nullptr;
    tickerOnly = false;

    currencyMapFile = "Bitfinex";
    baseValues.currentPair.name = "BTC/USD";
    baseValues.currentPair.setSymbol("BTCUSD");
    baseValues.currentPair.currRequestPair = "btcusd";
    baseValues.currentPair.priceDecimals = 5;
    minimumRequestIntervalAllowed = 500;
    baseValues.currentPair.priceMin = qPow(0.1, baseValues.currentPair.priceDecimals);
    baseValues.currentPair.tradeVolumeMin = 0.01;
    baseValues.currentPair.tradePriceMin = 0.1;
    defaultCurrencyParams.currADecimals = 8;
    defaultCurrencyParams.currBDecimals = 5;
    defaultCurrencyParams.currABalanceDecimals = 8;
    defaultCurrencyParams.currBBalanceDecimals = 5;
    defaultCurrencyParams.priceDecimals = 5;
    defaultCurrencyParams.priceMin = qPow(0.1, baseValues.currentPair.priceDecimals);

    supportsLoginIndicator = false;
    supportsAccountVolume = false;

    privateNonce = (TimeSync::getTimeT() - 1371854884) * 10;

    connect(this, &Exchange::threadFinished, this, &Exchange_Bitfinex::quitThread, Qt::DirectConnection);
}

Exchange_Bitfinex::~Exchange_Bitfinex()
{
}

void Exchange_Bitfinex::quitThread()
{
    clearValues();

    delete depthAsks;

    delete depthBids;

    delete julyHttp;
}

void Exchange_Bitfinex::clearVariables()
{
    historyLastTimestamp = "0";
    isFirstAccInfo = true;
    lastTickerHigh = 0.0;
    lastTickerLow = 0.0;
    lastTickerSell = 0.0;
    lastTickerBuy = 0.0;
    lastTickerVolume = 0.0;
    lastVolume = 0.0;
    secondPart = 0;
    apiDownCounter = 0;
    lastHistory.clear();
    lastOrders.clear();
    reloadDepth();
    lastInfoReceived = false;
    tickerLastDate = 0;
    lastTradesDate = 0;
    lastTradesDateCache = "0";
    lastHistoryId = 0LL;
}

void Exchange_Bitfinex::clearValues()
{
    clearVariables();

    if (julyHttp)
        julyHttp->clearPendingData();
}

void Exchange_Bitfinex::secondSlot()
{
    static int sendCounter = 0;

    switch (sendCounter)
    {
    case 0:
        if (!isReplayPending(103))
            sendToApi(103, "pubticker/" + baseValues.currentPair.currRequestPair, false, true);

        break;

    case 1:
        if (!isReplayPending(202))
            sendToApi(202, "balances", true, true);

        break;

    case 2:
        if (!isReplayPending(109))
            sendToApi(109,
                      "trades/" + baseValues.currentPair.currRequestPair + "?timestamp=" + lastTradesDateCache +
                          "&limit_trades=200" /*astTradesDateCache*/,
                      false,
                      true);

        break;

    case 3:
        if (!tickerOnly && !isReplayPending(204))
            sendToApi(204, "orders", true, true);

        break;

    case 4:
        if (isDepthEnabled() && (forceDepthLoad || !isReplayPending(111)))
        {
            emit depthRequested();
            sendToApi(111,
                      "book/" + baseValues.currentPair.currRequestPair + "?limit_bids=" + baseValues.depthCountLimitStr +
                          "&limit_asks=" + baseValues.depthCountLimitStr,
                      false,
                      true);
            forceDepthLoad = false;
        }

        break;

    case 5:
        if (lastHistory.isEmpty())
        {
            if (!isReplayPending(208))
                sendToApi(208,
                          "mytrades",
                          true,
                          true,
                          ", \"symbol\": \"" + baseValues.currentPair.currRequestPair + "\", \"timestamp\": " + historyLastTimestamp +
                              ", \"limit_trades\": 200");

            if (!isReplayPending(209))
                sendToApi(209, "account_infos", true, true);
        }

        break;

    default:
        break;
    }

    if (sendCounter++ >= 5)
        sendCounter = 0;

    Exchange::secondSlot();
}

bool Exchange_Bitfinex::isReplayPending(int reqType)
{
    if (julyHttp == nullptr)
        return false;

    return julyHttp->isReqTypePending(reqType);
}

void Exchange_Bitfinex::getHistory(bool force)
{
    if (tickerOnly)
        return;

    if (force)
        lastHistory.clear();

    if (!isReplayPending(208))
        sendToApi(208,
                  "mytrades",
                  true,
                  true,
                  ", \"symbol\": \"" + baseValues.currentPair.currRequestPair + "\", \"timestamp\": " + historyLastTimestamp +
                      ", \"limit_trades\": 100");

    if (!isReplayPending(209))
        sendToApi(209, "account_infos", true, true);
}

void Exchange_Bitfinex::buy(const QString& symbol, double apiBtcToBuy, double apiPriceToBuy)
{
    if (tickerOnly)
        return;

    CurrencyPairItem pairItem;
    pairItem = baseValues.currencyPairMap.value(symbol.toUpper(), pairItem);

    if (pairItem.symbol.isEmpty())
        return;

    QByteArray orderType = "limit";

    if (pairItem.currRequestSecond == "exchange")
        orderType.prepend("exchange ");

    QByteArray params = ", \"symbol\": \"" + pairItem.currRequestPair + "\", \"amount\": \"";
    params += JulyMath::textFromDouble(apiBtcToBuy, pairItem.currADecimals);
    params += "\", \"price\": \"";
    params += JulyMath::textFromDouble(apiPriceToBuy, pairItem.priceDecimals);
    params += "\", \"exchange\": \"all\", \"side\": \"buy\", \"type\": \"" + orderType + "\"";

    if (debugLevel)
        logThread->writeLog("Buy: " + params, 1);

    sendToApi(306, "order/new", true, true, params);
}

void Exchange_Bitfinex::sell(const QString& symbol, double apiBtcToSell, double apiPriceToSell)
{
    if (tickerOnly)
        return;

    CurrencyPairItem pairItem;
    pairItem = baseValues.currencyPairMap.value(symbol.toUpper(), pairItem);

    if (pairItem.symbol.isEmpty())
        return;

    QByteArray orderType = "limit";

    if (pairItem.currRequestSecond == "exchange")
        orderType.prepend("exchange ");

    QByteArray params = ", \"symbol\": \"" + pairItem.currRequestPair + "\", \"amount\": \"";
    params += JulyMath::textFromDouble(apiBtcToSell, pairItem.currADecimals);
    params += "\", \"price\": \"";
    params += JulyMath::textFromDouble(apiPriceToSell, pairItem.priceDecimals);
    params += "\", \"exchange\": \"all\", \"side\": \"sell\", \"type\": \"" + orderType + "\"";

    if (debugLevel)
        logThread->writeLog("Sell: " + params, 1);

    sendToApi(307, "order/new", true, true, params);
}

void Exchange_Bitfinex::cancelOrder(const QString& /*unused*/, const QByteArray& order)
{
    if (tickerOnly)
        return;

    if (debugLevel)
        logThread->writeLog("Cancel order: " + order, 1);

    sendToApi(305, "order/cancel", true, true, ", \"order_id\": " + order);
}

void Exchange_Bitfinex::sendToApi(int reqType, const QByteArray& method, bool auth, bool sendNow, QByteArray commands)
{
    if (julyHttp == nullptr)
    {
        if (domain.isEmpty() || port == 0)
            julyHttp = new JulyHttp("api.bitfinex.com", "X-BFX-APIKEY: " + getApiKey() + "\r\n", this);
        else
        {
            julyHttp = new JulyHttp(domain, "X-BFX-APIKEY: " + getApiKey() + "\r\n", this, useSsl);
            julyHttp->setPortForced(port);
        }

        connect(julyHttp, &JulyHttp::anyDataReceived, baseValues_->mainWindow_, &QtBitcoinTrader::anyDataReceived);
        connect(julyHttp, &JulyHttp::setDataPending, baseValues_->mainWindow_, &QtBitcoinTrader::setDataPending);
        connect(julyHttp, &JulyHttp::apiDown, baseValues_->mainWindow_, &QtBitcoinTrader::setApiDown);
        connect(julyHttp, &JulyHttp::errorSignal, baseValues_->mainWindow_, &QtBitcoinTrader::showErrorMessage);
        connect(julyHttp, &JulyHttp::sslErrorSignal, this, &Exchange_Bitfinex::sslErrors);
        connect(julyHttp, &JulyHttp::dataReceived, this, &Exchange_Bitfinex::dataReceivedAuth);
    }

    if (auth)
    {
        QByteArray postData = "{\"request\": \"/v1/" + method + "\", \"nonce\": \"" + QByteArray::number(++privateNonce) + "\"";
        postData.append(commands);
        postData.append("}");
        QByteArray payload = postData.toBase64();
        QByteArray forHash = hmacSha384(getApiSign(), payload).toHex();

        if (sendNow)
            julyHttp->sendData(reqType,
                               m_pairChangeCount,
                               "POST /v1/" + method,
                               postData,
                               "X-BFX-PAYLOAD: " + payload + "\r\nX-BFX-SIGNATURE: " + forHash + "\r\n");
        else
            julyHttp->prepareData(reqType,
                                  m_pairChangeCount,
                                  "POST /v1/" + method,
                                  postData,
                                  "X-BFX-PAYLOAD: " + payload + "\r\nX-BFX-SIGNATURE: " + forHash + "\r\n");
    }
    else
    {
        if (sendNow)
            julyHttp->sendData(reqType, m_pairChangeCount, "GET /v1/" + method);
        else
            julyHttp->prepareData(reqType, m_pairChangeCount, "GET /v1/" + method);
    }
}

void Exchange_Bitfinex::depthUpdateOrder(const QString& symbol, double price, double amount, bool isAsk)
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

void Exchange_Bitfinex::depthSubmitOrder(const QString& symbol,
                                         QMap<double, double>* currentMap,
                                         double priceDouble,
                                         double amount,
                                         bool isAsk)
{
    if (symbol != baseValues.currentPair.symbol)
        return;

    if (priceDouble == 0.0 || amount == 0.0)
        return;

    if (orderBookItemIsDedicatedOrder)
        amount += currentMap->value(priceDouble, 0.0);

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

void Exchange_Bitfinex::reloadDepth()
{
    lastDepthBidsMap.clear();
    lastDepthAsksMap.clear();
    lastDepthData.clear();
    Exchange::reloadDepth();
}

void Exchange_Bitfinex::dataReceivedAuth(const QByteArray& data, int reqType, int pairChangeCount)
{
    if (pairChangeCount != m_pairChangeCount)
        return;

    if (debugLevel)
        logThread->writeLog("RCV: " + data);

    if (data.size() && data.at(0) == QLatin1Char('<'))
        return;

    bool success = (data.startsWith("{") || data.startsWith("[")) && !data.startsWith("{\"message\"");

    switch (reqType)
    {
    case 103: // ticker
        if (!success)
            break;

        if (data.startsWith("{\"mid\":"))
        {
            QByteArray tickerSell = getMidData("bid\":\"", "\"", &data);

            if (!tickerSell.isEmpty())
            {
                double newTickerSell = tickerSell.toDouble();

                if (newTickerSell != lastTickerSell)
                    IndicatorEngine::setValue(baseValues.exchangeName, baseValues.currentPair.symbol, "Sell", newTickerSell);

                lastTickerSell = newTickerSell;
            }

            QByteArray tickerBuy = getMidData("ask\":\"", "\"", &data);

            if (!tickerBuy.isEmpty())
            {
                double newTickerBuy = tickerBuy.toDouble();

                if (newTickerBuy != lastTickerBuy)
                    IndicatorEngine::setValue(baseValues.exchangeName, baseValues.currentPair.symbol, "Buy", newTickerBuy);

                lastTickerBuy = newTickerBuy;
            }

            qint64 tickerNow = getMidData("timestamp\":\"", ".", &data).toUInt();

            if (tickerLastDate < tickerNow)
            {
                QByteArray tickerLast = getMidData("last_price\":\"", "\"", &data);
                double newTickerLast = tickerLast.toDouble();

                if (newTickerLast > 0.0)
                {
                    IndicatorEngine::setValue(baseValues.exchangeName, baseValues.currentPair.symbol, "Last", newTickerLast);
                    tickerLastDate = tickerNow;
                }
            }

            QByteArray tickerHigh = getMidData("high\":\"", "\"", &data);

            if (!tickerHigh.isEmpty())
            {
                double newTickerHigh = tickerHigh.toDouble();

                if (newTickerHigh != lastTickerHigh)
                    IndicatorEngine::setValue(baseValues.exchangeName, baseValues.currentPair.symbol, "High", newTickerHigh);

                lastTickerHigh = newTickerHigh;
            }

            QByteArray tickerLow = getMidData("\"low\":\"", "\"", &data);

            if (!tickerLow.isEmpty())
            {
                double newTickerLow = tickerLow.toDouble();

                if (newTickerLow != lastTickerLow)
                    IndicatorEngine::setValue(baseValues.exchangeName, baseValues.currentPair.symbol, "Low", newTickerLow);

                lastTickerLow = newTickerLow;
            }

            QByteArray tickerVolume = getMidData("\"volume\":\"", "\"", &data);

            if (!tickerVolume.isEmpty())
            {
                double newTickerVolume = tickerVolume.toDouble();

                if (newTickerVolume != lastTickerVolume)
                    IndicatorEngine::setValue(baseValues.exchangeName, baseValues.currentPair.symbol, "Volume", newTickerVolume);

                lastTickerVolume = newTickerVolume;
            }
        }
        else if (debugLevel)
            logThread->writeLog("Invalid ticker fast data:" + data, 2);

        break; // ticker

    case 109: // money/trades/fetch
        if (success && data.size() > 32)
        {
            QStringList tradeList = QString(data).split("},{");
            auto* newTradesItems = new QList<TradesItem>;

            for (int n = tradeList.size() - 1; n >= 0; n--)
            {
                QByteArray tradeData = tradeList.at(n).toLatin1();
                qint64 currentTradeDate = getMidData("timestamp\":", ",", &tradeData).toLongLong();

                if (lastTradesDate >= currentTradeDate || currentTradeDate == 0)
                    continue;

                TradesItem newItem;
                newItem.amount = getMidData("\"amount\":\"", "\",", &tradeData).toDouble();
                newItem.price = getMidData("\"price\":\"", "\",", &tradeData).toDouble();
                newItem.orderType = getMidData("\"type\":\"", "\"", &tradeData) == "sell" ? 1 : -1;

                newItem.symbol = baseValues.currentPair.symbol;
                newItem.date = currentTradeDate;

                if (newItem.isValid())
                    (*newTradesItems) << newItem;
                else if (debugLevel)
                    logThread->writeLog("Invalid trades fetch data line:" + tradeData, 2);

                if (n == 0)
                {
                    IndicatorEngine::setValue(baseValues.exchangeName, baseValues.currentPair.symbol, "Last", newItem.price);
                    tickerLastDate = currentTradeDate;
                    lastTradesDate = currentTradeDate;
                    lastTradesDateCache = QByteArray::number(tickerLastDate + 1);
                }
            }

            if (!newTradesItems->empty())
                emit addLastTrades(baseValues.currentPair.symbol, newTradesItems);
            else
                delete newTradesItems;
        }

        break;

    case 111: // depth
        if (data.startsWith("{\"bids\""))
        {
            emit depthRequestReceived();

            if (lastDepthData != data)
            {
                lastDepthData = data;

                depthAsks = new QList<DepthItem>;
                depthBids = new QList<DepthItem>;

                QMap<double, double> currentAsksMap;

                QStringList asksList = QString(getMidData("asks\":[{", "}]", &data)).split("},{");
                double groupedPrice = 0.0;
                double groupedVolume = 0.0;
                int rowCounter = 0;

                for (int n = 0; n < asksList.size(); n++)
                {
                    if (baseValues.depthCountLimit && rowCounter >= baseValues.depthCountLimit)
                        break;

                    QByteArray currentRow = asksList.at(n).toLatin1();
                    double priceDouble = getMidData("price\":\"", "\"", &currentRow).toDouble();
                    double amount = getMidData("amount\":\"", "\"", &currentRow).toDouble();

                    if (n == 0)
                        IndicatorEngine::setValue(baseValues.exchangeName, baseValues.currentPair.symbol, "Buy", priceDouble);

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
                    if (currentAsksMap.value(currentAsksList.at(n), 0) == 0)
                        depthUpdateOrder(baseValues.currentPair.symbol, currentAsksList.at(n), 0.0, true); // Remove price

                lastDepthAsksMap = currentAsksMap;

                QMap<double, double> currentBidsMap;
                QStringList bidsList = QString(getMidData("bids\":[{", "}]", &data)).split("},{");
                groupedPrice = 0.0;
                groupedVolume = 0.0;
                rowCounter = 0;

                for (int n = 0; n < bidsList.size(); n++)
                {
                    if (baseValues.depthCountLimit && rowCounter >= baseValues.depthCountLimit)
                        break;

                    QByteArray currentRow = bidsList.at(n).toLatin1();
                    double priceDouble = getMidData("price\":\"", "\"", &currentRow).toDouble();
                    double amount = getMidData("amount\":\"", "\"", &currentRow).toDouble();

                    if (n == 0)
                        IndicatorEngine::setValue(baseValues.exchangeName, baseValues.currentPair.symbol, "Sell", priceDouble);

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
                            bool matchCurrentGroup = priceDouble > groupedPrice + baseValues.groupPriceValue;

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
                    if (currentBidsMap.value(currentBidsList.at(n), 0) == 0)
                        depthUpdateOrder(baseValues.currentPair.symbol, currentBidsList.at(n), 0.0, false); // Remove price

                lastDepthBidsMap = currentBidsMap;

                for (int n = depthAsks->size() - 1; n > 0; n--)
                    if (depthAsks->at(n).price == depthAsks->at(n - 1).price)
                        depthAsks->removeAt(--n);

                for (int n = depthBids->size() - 1; n > 0; n--)
                    if (depthBids->at(n).price == depthBids->at(n - 1).price)
                        depthBids->removeAt(--n);

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

            if (data.startsWith("[{\"type\""))
            {
                lastInfoReceived = true;

                if (debugLevel)
                    logThread->writeLog("Info: " + data);

                QByteArray btcBalance;
                QByteArray usdBalance;

                QStringList balances = QString(data).split("},{");

                for (int n = 0; n < balances.size(); n++)
                {
                    QByteArray currentBalance = balances.at(n).toLatin1();
                    QByteArray balanceType = getMidData("type\":\"", "\"", &currentBalance);

                    if (balanceType != baseValues.currentPair.currRequestSecond)
                        continue;

                    QByteArray balanceCurrency = getMidData("currency\":\"", "\"", &currentBalance);

                    if (btcBalance.isEmpty() && balanceCurrency == baseValues.currentPair.currAStrLow)
                        btcBalance = getMidData("available\":\"", "\"", &currentBalance);

                    if (usdBalance.isEmpty() && balanceCurrency == baseValues.currentPair.currBStrLow)
                    {
                        usdBalance = getMidData("available\":\"", "\"", &currentBalance);
                    }
                }

                if (checkValue(btcBalance, lastBtcBalance))
                    emit accBtcBalanceChanged(baseValues.currentPair.symbolSecond(), lastBtcBalance);

                if (checkValue(usdBalance, lastUsdBalance))
                    emit accUsdBalanceChanged(baseValues.currentPair.symbolSecond(), lastUsdBalance);
            }
            else if (debugLevel)
                logThread->writeLog("Invalid Info data:" + data, 2);
        }
        break; // info

    case 204: // orders
        if (!success)
            break;

        if (data.size() <= 30)
        {
            lastOrders.clear();
            emit ordersIsEmpty();
            break;
        }

        if (lastOrders != data)
        {
            lastOrders = data;
            QStringList ordersList = QString(data).split("},{");
            auto* orders = new QList<OrderItem>;
            QByteArray filterType = "limit";

            if (baseValues.currentPair.currRequestSecond == "exchange")
                filterType.prepend("exchange ");

            for (int n = 0; n < ordersList.size(); n++)
            {
                if (!ordersList.at(n).contains("\"type\":\"" + filterType + "\""))
                    continue;

                QByteArray currentOrderData = ordersList.at(n).toLatin1();
                OrderItem currentOrder;
                currentOrder.oid = getMidData("\"id\":", ",", &currentOrderData);
                currentOrder.date = getMidData("timestamp\":\"", "\"", &currentOrderData).split('.').first().toUInt();
                currentOrder.type = getMidData("side\":\"", "\"", &currentOrderData).toLower() == "sell";

                bool isCanceled = getMidData("is_cancelled\":", ",", &currentOrderData) == "true";

                // 0=Canceled, 1=Open, 2=Pending, 3=Post-Pending
                if (isCanceled)
                    currentOrder.status = 0;
                else
                    currentOrder.status = 1;

                currentOrder.amount = getMidData("remaining_amount\":\"", "\"", &currentOrderData).toDouble();
                currentOrder.price = getMidData("price\":\"", "\"", &currentOrderData).toDouble();
                currentOrder.symbol = IniEngine::getSymbolByRequest(getMidData("symbol\":\"", "\"", &currentOrderData));

                if (currentOrder.isValid())
                    (*orders) << currentOrder;
            }

            emit orderBookChanged(baseValues.currentPair.symbol, orders);

            lastInfoReceived = false;
        }

        break; // orders

    // case 210: //positions
    //   {
    //       data="[{\"id\":72119,\"symbol\":\"btcusd\",\"status\":\"ACTIVE\",\"base\":\"804.7899\",\"amount\":\"0.001\",\"timestamp\":\"1389624548.0\",\"swap\":\"0.0\",\"pl\":\"-0.0055969\"},{\"id\":72120,\"symbol\":\"ltcbtc\",\"status\":\"ACTIVE\",\"base\":\"0.02924999\",\"amount\":\"0.001\",\"timestamp\":\"1389624559.0\",\"swap\":\"0.0\",\"pl\":\"-0.00000067280018\"},{\"id\":72122,\"symbol\":\"ltcusd\",\"status\":\"ACTIVE\",\"base\":\"23.23\",\"amount\":\"0.001\",\"timestamp\":\"1389624576.0\",\"swap\":\"0.0\",\"pl\":\"-0.00016465\"}]";

    //  }//positions
    case 305: // order/cancel
        {
            if (!success)
                break;

            QByteArray oid = getMidData("\"id\":", ",", &data);

            if (!oid.isEmpty())
                emit orderCanceled(baseValues.currentPair.symbol, oid);
            else if (debugLevel)
                logThread->writeLog("Invalid Order/Cancel data:" + data, 2);
        }
        break; // order/cancel

    case 306: // order/buy
        if (!success || !debugLevel)
            break;

        if (data.startsWith("{\"id\""))
            logThread->writeLog("Buy OK: " + data);
        else
            logThread->writeLog("Invalid Order Buy Data:" + data);

        break; // order/buy

    case 307: // order/sell
        if (!success || !debugLevel)
            break;

        if (data.startsWith("{\"id\""))
            logThread->writeLog("Sell OK: " + data);
        else
            logThread->writeLog("Invalid Order Sell Data:" + data);

        break; // order/sell

    case 208: // money/wallet/history
        if (!success)
            break;

        if (data.startsWith("["))
        {
            if (lastHistory != data)
            {
                lastHistory = data;

                auto* historyItems = new QList<HistoryItem>;
                bool firstTimestampReceived = false;
                QStringList dataList = QString(data).split("},{");
                qint64 maxId = 0LL;

                for (int n = 0; n < dataList.size(); n++)
                {
                    QByteArray curLog(dataList.at(n).toLatin1() + "}");
                    qint64 currentId = getMidData("tid\":", ",", &curLog).toLongLong();

                    if (currentId <= lastHistoryId)
                        break;

                    if (currentId > maxId)
                        maxId = currentId;

                    QByteArray currentTimeStamp = getMidData("\"timestamp\":\"", "\"", &curLog).split('.').first();

                    if (!firstTimestampReceived && !currentTimeStamp.isEmpty())
                    {
                        historyLastTimestamp = currentTimeStamp;
                        firstTimestampReceived = true;
                    }

                    HistoryItem currentHistoryItem;
                    QByteArray logType = getMidData("\"type\":\"", "\"", &curLog);

                    if (logType == "Sell")
                        currentHistoryItem.type = 1;
                    else if (logType == "Buy")
                        currentHistoryItem.type = 2;
                    else if (logType == "fee")
                        currentHistoryItem.type = 3;
                    else if (logType == "deposit")
                        currentHistoryItem.type = 4;
                    else if (logType == "withdraw")
                        currentHistoryItem.type = 5;

                    if (currentHistoryItem.type)
                    {
                        currentHistoryItem.price = getMidData("\"price\":\"", "\"", &curLog).toDouble();
                        currentHistoryItem.volume = getMidData("\"amount\":\"", "\"", &curLog).toDouble();
                        currentHistoryItem.dateTimeInt = currentTimeStamp.toLongLong();
                        currentHistoryItem.symbol = baseValues.currentPair.symbol;
                        currentHistoryItem.currRequestSecond = baseValues.currentPair.currRequestSecond;

                        if (currentHistoryItem.isValid())
                        {
                            currentHistoryItem.description += getMidData("exchange\":\"", "\"", &curLog);
                            (*historyItems) << currentHistoryItem;
                        }
                    }
                }

                if (maxId > lastHistoryId)
                    lastHistoryId = maxId;

                emit historyChanged(historyItems);
            }
        }
        else if (debugLevel)
            logThread->writeLog("Invalid History data:" + data.left(200), 2);

        break; // money/wallet/history

    case 209: // fee
        if (!success)
            break;

        if (data.startsWith("[{\""))
        {
            bool feeInit = false;
            double newFee(0.0);

            QStringList feeList = QString(getMidData("[{\"pairs\":\"", "}]}]", &data)).split("},{\"pairs\":\"");

            for (int n = 0; n < feeList.size(); n++)
            {
                if (!feeList.at(n).startsWith(baseValues.currentPair.currAStr))
                    continue;

                QByteArray currentFeeData = feeList.at(n).toLatin1();
                newFee = getMidData("taker_fees\":\"", "\"", &currentFeeData).toDouble();

                feeInit = true;
                break;
            }

            if (!feeInit)
            {
                newFee = getMidData("taker_fees\":\"", "\"", &data).toDouble();
            }

            if (!qFuzzyCompare(newFee + 1.0, lastFee + 1.0))
                emit accFeeChanged(baseValues.currentPair.symbol, newFee);

            lastFee = newFee;
        }

        break; // fee

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
                QString authErrorString = getMidData("message\":\"", "\"", &data);

                if (debugLevel)
                    logThread->writeLog("API error: " + authErrorString.toLatin1() + " ReqType: " + QByteArray::number(reqType), 2);

                if (authErrorString == "Could not find a key matching the given X-BFX-APIKEY.")
                    authErrorString = julyTr("TRUNAUTHORIZED", "Invalid API key.");
                else if (authErrorString == "Nonce is too small.")
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

        QString errorString = getMidData("\"message\":\"", "\"", &data);

        if (errorString.isEmpty())
            errorString = data;

        if (debugLevel)
            logThread->writeLog(errorString.toLatin1(), 2);

        if (errorString.isEmpty())
            return;

        errorString.append("<br>" + QString::number(reqType));

        if (errorString.contains("X-BFX-SIGNATURE") || errorString.contains("X-BFX-APIKEY"))
            errorString.prepend("I:>");

        if (reqType < 300)
            emit showErrorMessage(errorString);
    }
    else
        errorCount = 0;
}
