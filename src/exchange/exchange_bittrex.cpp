//  This file is part of Qt Bitcoin Trader
//      https://github.com/JulyIGHOR/QtBitcoinTrader
//  Copyright (C) 2013-2023 July Ighor <julyighor@gmail.com>
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

#include <QMessageAuthenticationCode>
#include "timesync.h"
#include "exchange_bittrex.h"

Exchange_Bittrex::Exchange_Bittrex(const QByteArray &pRestSign, const QByteArray &pRestKey)
    : Exchange(),
      isFirstAccInfo(true),
      lastTradesTime(0LL),
      lastHistoryTime(0LL),
      privateNonce((TimeSync::getTimeT() - 1371854884LL) * 10LL),
      julyHttp(nullptr),
      depthAsks(nullptr),
      depthBids(nullptr),
      lastDepthAsksMap(),
      lastDepthBidsMap()
{
    clearOpenOrdersOnCurrencyChanged = true;
    clearHistoryOnCurrencyChanged = true;
    calculatingFeeMode = 3;
    baseValues.exchangeName = "Bittrex";
    baseValues.currentPair.name = "LTC/BTC";
    baseValues.currentPair.setSymbol("LTC/BTC");
    baseValues.currentPair.currRequestPair = "BTC-LTC";
    baseValues.currentPair.priceDecimals = 3;
    minimumRequestIntervalAllowed = 500;
    baseValues.currentPair.priceMin = qPow(0.1, baseValues.currentPair.priceDecimals);
    baseValues.currentPair.tradeVolumeMin = 0.01;
    baseValues.currentPair.tradePriceMin = 0.1;
    forceDepthLoad = false;
    tickerOnly = false;
    setApiKeySecret(pRestKey, pRestSign);

    currencyMapFile = "Bittrex";
    defaultCurrencyParams.currADecimals = 8;
    defaultCurrencyParams.currBDecimals = 8;
    defaultCurrencyParams.currABalanceDecimals = 8;
    defaultCurrencyParams.currBBalanceDecimals = 8;
    defaultCurrencyParams.priceDecimals = 3;
    defaultCurrencyParams.priceMin = qPow(0.1, baseValues.currentPair.priceDecimals);

    supportsLoginIndicator = false;
    supportsAccountVolume = false;

    connect(this, &Exchange::threadFinished, this, &Exchange_Bittrex::quitThread, Qt::DirectConnection);
}

Exchange_Bittrex::~Exchange_Bittrex()
{
}

void Exchange_Bittrex::quitThread()
{
    clearValues();

    
        delete depthAsks;

    
        delete depthBids;

    
        delete julyHttp;
}

void Exchange_Bittrex::clearVariables()
{
    isFirstAccInfo = true;
    lastTradesTime = 0LL;
    lastTradesIds.clear();
    lastHistoryTime = 0LL;
    lastHistoryIds.clear();
    Exchange::clearVariables();
    lastHistory.clear();
    lastOrders.clear();
    reloadDepth();
    emit accFeeChanged(baseValues.currentPair.symbol, 0.25);
}

void Exchange_Bittrex::clearValues()
{
    clearVariables();

    if (julyHttp)
        julyHttp->clearPendingData();
}

void Exchange_Bittrex::reloadDepth()
{
    lastDepthBidsMap.clear();
    lastDepthAsksMap.clear();
    lastDepthData.clear();
    Exchange::reloadDepth();
}

void Exchange_Bittrex::dataReceivedAuth(const QByteArray& data, int reqType, int pairChangeCount)
{
    if (pairChangeCount != m_pairChangeCount)
        return;

    if (debugLevel)
        logThread->writeLog("RCV: " + data);

    if (!data.size() || (data.at(0) != QLatin1Char('{') && data.at(0) != QLatin1Char('[')))
        return;

    bool fail = data.startsWith("{\"code\":\"");
    QString errorString;

    if (fail)
    {
        errorString = getMidData("\"detail\":\"", "\"", &data);

        if (errorString.isEmpty())
            errorString = getMidData("\"code\":\"", "\"", &data);

        if (debugLevel)
            logThread->writeLog("Invalid data:" + data, 2);
    }
    else switch (reqType)
    {
    case 103: //ticker
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

            double tickerVolume = getMidData("\"volume\":\"", "\"", &data).toDouble();

            if (tickerVolume > 0.0 && !qFuzzyCompare(tickerVolume, lastTickerVolume))
            {
                IndicatorEngine::setValue(baseValues.exchangeName, baseValues.currentPair.symbol, "Volume", tickerVolume);
                lastTickerVolume = tickerVolume;
            }
        }
        break;//ticker

    case 104: //ticker
        {
            double tickerSell = getMidData("\"bidRate\":\"", "\"", &data).toDouble();

            if (tickerSell > 0.0 && !qFuzzyCompare(tickerSell, lastTickerSell))
            {
                IndicatorEngine::setValue(baseValues.exchangeName, baseValues.currentPair.symbol, "Sell", tickerSell);
                lastTickerSell = tickerSell;
            }

            double tickerBuy = getMidData("\"askRate\":\"", "\"", &data).toDouble();

            if (tickerBuy > 0.0 && !qFuzzyCompare(tickerBuy, lastTickerBuy))
            {
                IndicatorEngine::setValue(baseValues.exchangeName, baseValues.currentPair.symbol, "Buy", tickerBuy);
                lastTickerBuy = tickerBuy;
            }

            double tickerLastDouble = getMidData("\"lastTradeRate\":\"", "\"", &data).toDouble();

            if (tickerLastDouble > 0.0 && !qFuzzyCompare(tickerLastDouble, lastTickerLast))
            {
                IndicatorEngine::setValue(baseValues.exchangeName, baseValues.currentPair.symbol, "Last", tickerLastDouble);
                lastTickerLast = tickerLastDouble;
            }
        }
        break;//ticker

    case 109: //trades
        {
            qint64 time10Min = TimeSync::getMSecs() - 600000;
            QStringList tradeList = QString(data).split("},{");
            auto* newTradesItems = new QList<TradesItem>;

            for (int n = tradeList.size() - 1; n >= 0; --n)
            {
                QByteArray tradeData = tradeList.at(n).toLatin1();

                QDateTime date = QDateTime::fromString(getMidData("\"executedAt\":\"", "\"", &tradeData), Qt::ISODate);
                date.setTimeSpec(Qt::UTC);
                qint64 dateInt = date.toMSecsSinceEpoch();

                if (dateInt < lastTradesTime || dateInt < time10Min)
                    continue;

                QByteArray id = getMidData("\"id\":\"", "\"", &tradeData);

                if (dateInt > lastTradesTime)
                    lastTradesIds.clear();
                else if (lastTradesIds.contains(id))
                    continue;

                lastTradesTime = dateInt;
                lastTradesIds[id] = true;

                TradesItem newItem;
                newItem.date = dateInt / 1000;
                newItem.amount = getMidData("\"quantity\":\"", "\"", &tradeData).toDouble();
                newItem.price  = getMidData("\"rate\":\"",     "\"", &tradeData).toDouble();
                newItem.symbol = baseValues.currentPair.symbol;
                newItem.orderType = getMidData("\"takerSide\":\"", "\"", &tradeData) == "BUY" ? 1 : -1;

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
                QStringList asksList = QString(getMidData("\"ask\":[{\"quantity\":\"", "\"}]}", &data)).split("\"},{\"quantity\":\"");
                double groupedPrice = 0.0;
                double groupedVolume = 0.0;
                int rowCounter = 0;

                for (int n = 0; n < asksList.size(); n++)
                {
                    if (baseValues.depthCountLimit && rowCounter >= baseValues.depthCountLimit)
                        break;

                    QStringList currentPair = asksList.at(n).split("\",\"rate\":\"");

                    if (currentPair.size() != 2)
                        continue;

                    double priceDouble = currentPair.last().toDouble();
                    double amount      = currentPair.first().toDouble();

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
                QStringList bidsList = QString(getMidData("\"bid\":[{\"quantity\":\"", "\"}],", &data)).split("\"},{\"quantity\":\"");
                groupedPrice = 0.0;
                groupedVolume = 0.0;
                rowCounter = 0;

                for (int n = 0; n < bidsList.size(); n++)
                {
                    if (baseValues.depthCountLimit && rowCounter >= baseValues.depthCountLimit)
                        break;

                    QStringList currentPair = bidsList.at(n).split("\",\"rate\":\"");

                    if (currentPair.size() != 2)
                        continue;

                    double priceDouble = currentPair.last().toDouble();
                    double amount      = currentPair.first().toDouble();

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
            QByteArray btcBalance = getMidData("\"currencySymbol\":\"" + baseValues.currentPair.currAStr + "\"", "}", &data);
            QByteArray usdBalance = getMidData("\"currencySymbol\":\"" + baseValues.currentPair.currBStr + "\"", "}", &data);
            btcBalance = getMidData("\"available\":\"", "\"", &btcBalance);
            usdBalance = getMidData("\"available\":\"", "\"", &usdBalance);

            if (checkValue(btcBalance, lastBtcBalance))
                emit accBtcBalanceChanged(baseValues.currentPair.symbol, lastBtcBalance);

            if (checkValue(usdBalance, lastUsdBalance))
                emit accUsdBalanceChanged(baseValues.currentPair.symbol, lastUsdBalance);
        }
        break;//info

    case 204://orders
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

                    //0=Canceled, 1=Open, 2=Pending, 3=Post-Pending
                    if (getMidData("\"status\":\"", "\"", &currentOrderData) == "CLOSED")
                        currentOrder.status = 0;
                    else
                        currentOrder.status = 1;

                    QDateTime date = QDateTime::fromString(getMidData("\"createdAt\":\"", "\"", &currentOrderData), Qt::ISODate);
                    date.setTimeSpec(Qt::UTC);
                    currentOrder.date   = date.toSecsSinceEpoch();
                    currentOrder.oid    = getMidData("\"id\":\"",           "\"", &currentOrderData);
                    currentOrder.type   = getMidData("\"direction\":\"",    "\"", &currentOrderData) == "SELL";
                    currentOrder.amount = getMidData("\"quantity\":\"",     "\"", &currentOrderData).toDouble();
                    currentOrder.price  = getMidData("\"limit\":\"",        "\"", &currentOrderData).toDouble();
                    QList<QByteArray> p = getMidData("\"marketSymbol\":\"", "\"", &currentOrderData).split('-');
                    currentOrder.symbol = p.first() + '/' + p.last();

                    if (currentOrder.isValid())
                        (*orders) << currentOrder;
                }

                if (!orders->empty())
                    emit orderBookChanged(baseValues.currentPair.symbol, orders);
                else
                    delete orders;
            }

            break;//orders
        }

    case 305: //order/cancel
    {
        QByteArray id      = getMidData("\"id\":\"",           "\"", &data);
        QByteArray request = getMidData("\"marketSymbol\":\"", "\"", &data);

        if (request == baseValues.currentPair.currRequestPair)
            emit orderCanceled(request, id);

        break;//order/cancel
    }
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
                auto* historyItems = new QList<HistoryItem>;

                for (int n = historyList.size() - 1; n >= 0; --n)
                {
                    QByteArray logData(historyList.at(n).toLatin1());

                    QDateTime date = QDateTime::fromString(getMidData("\"closedAt\":\"", "\"", &logData), Qt::ISODate);
                    date.setTimeSpec(Qt::UTC);
                    qint64 dateInt = date.toMSecsSinceEpoch();

                    if (dateInt < lastHistoryTime)
                        continue;

                    QByteArray id = getMidData("\"id\":\"", "\"", &logData);

                    if (dateInt > lastHistoryTime)
                        lastHistoryIds.clear();
                    else if (lastHistoryIds.contains(id))
                        continue;

                    lastHistoryTime = dateInt;
                    lastHistoryIds[id] = true;

                    HistoryItem currentHistoryItem;

                    if (getMidData("\"direction\":\"", "\"", &logData) == "SELL")
                        currentHistoryItem.type = 1;
                    else
                        currentHistoryItem.type = 2;

                    QList<QByteArray> pair         = getMidData("\"marketSymbol\":\"", "\"", &logData).split('-');
                    currentHistoryItem.symbol      = pair.first() + '/' + pair.last();
                    currentHistoryItem.price       = getMidData("\"limit\":\"",    "\"", &logData).toDouble();
                    currentHistoryItem.volume      = getMidData("\"quantity\":\"", "\"", &logData).toDouble();
                    currentHistoryItem.dateTimeInt = dateInt / 1000;

                    if (currentHistoryItem.isValid())
                        historyItems->prepend(currentHistoryItem);
                }

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

        if (fail)
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

        if (fail)
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

void Exchange_Bittrex::depthUpdateOrder(const QString& symbol, double price, double amount, bool isAsk)
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

void Exchange_Bittrex::depthSubmitOrder(const QString& symbol, QMap<double, double>* currentMap, double priceDouble,
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

bool Exchange_Bittrex::isReplayPending(int reqType)
{
    if (julyHttp == nullptr)
        return false;

    return julyHttp->isReqTypePending(reqType);
}

void Exchange_Bittrex::secondSlot()
{
    static int sendCounter = 0;

    switch (sendCounter)
    {
    case 0:
            if (!isReplayPending(103))
                sendToApi(103, "markets/" + baseValues.currentPair.currRequestPair + "/summary");
            break;

    case 1:
            if (!isReplayPending(104))
                sendToApi(104, "markets/" + baseValues.currentPair.currRequestPair + "/ticker");
            break;

        case 2:
            if (!isReplayPending(202))
                sendToApi(202, "balances", true);

            break;

        case 3:
            if (!isReplayPending(109))
                sendToApi(109, "markets/" + baseValues.currentPair.currRequestPair + "/trades");

            break;

        case 4:
            if (!tickerOnly && !isReplayPending(204))
                sendToApi(204, "orders/open", true);
            //https://bittrex.github.io/guides/v3/upgrade

            break;

        case 5:
            if (isDepthEnabled() && (forceDepthLoad || !isReplayPending(111)))
            {
                emit depthRequested();
                sendToApi(111, "markets/" + baseValues.currentPair.currRequestPair + "/orderbook?depth=500");
                forceDepthLoad = false;
            }

            break;

        case 6:
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

void Exchange_Bittrex::getHistory(bool force)
{
    if (tickerOnly)
        return;

    if (force)
        lastHistory.clear();

    if (!isReplayPending(208))
        sendToApi(208, "orders/closed?marketSymbol=" + baseValues.currentPair.currRequestPair, true);
}

void Exchange_Bittrex::buy(const QString& symbol, double apiBtcToBuy, double apiPriceToBuy)
{
    if (tickerOnly)
        return;

    CurrencyPairItem pairItem;
    pairItem = baseValues.currencyPairMap.value(symbol, pairItem);

    if (pairItem.symbol.isEmpty())
        return;

    QByteArray data = "{\"marketSymbol\":\"" + pairItem.currRequestPair +
            "\",\"direction\":\"BUY\",\"type\":\"LIMIT\"," +
            "\"quantity\":\"" + JulyMath::byteArrayFromDouble(apiBtcToBuy,   pairItem.currADecimals, 0) +
            "\",\"limit\":\"" + JulyMath::byteArrayFromDouble(apiPriceToBuy, pairItem.priceDecimals, 0) +
            "\",\"timeInForce\":\"GOOD_TIL_CANCELLED\"}";

    if (debugLevel)
        logThread->writeLog("Buy: " + data, 2);

    sendToApi(306, "orders", true, "POST", data);
}

void Exchange_Bittrex::sell(const QString& symbol, double apiBtcToSell, double apiPriceToSell)
{
    if (tickerOnly)
        return;

    CurrencyPairItem pairItem;
    pairItem = baseValues.currencyPairMap.value(symbol, pairItem);

    if (pairItem.symbol.isEmpty())
        return;

    QByteArray data = "{\"marketSymbol\":\"" + pairItem.currRequestPair +
            "\",\"direction\":\"SELL\",\"type\":\"LIMIT\"," +
            "\"quantity\":\"" + JulyMath::byteArrayFromDouble(apiBtcToSell,   pairItem.currADecimals, 0) +
            "\",\"limit\":\"" + JulyMath::byteArrayFromDouble(apiPriceToSell, pairItem.priceDecimals, 0) +
            "\",\"timeInForce\":\"GOOD_TIL_CANCELLED\"}";

    if (debugLevel)
        logThread->writeLog("Sell: " + data, 2);

    sendToApi(307, "orders", true, "POST", data);
}

void Exchange_Bittrex::cancelOrder(const QString& /*unused*/, const QByteArray& order)
{
    if (tickerOnly)
        return;

    if (debugLevel)
        logThread->writeLog("Cancel order: " + order, 2);

    sendToApi(305, "orders/" + order, true, "DELETE");
}

void Exchange_Bittrex::sendToApi(int reqType, const QByteArray& method, bool auth,
                                 const QByteArray& type, const QByteArray& postData)
{
    if (julyHttp == nullptr)
    {
        if (domain.isEmpty() || port == 0)
            julyHttp = new JulyHttp("api.bittrex.com", "", this, true, true, "application/json");
        else
        {
            julyHttp = new JulyHttp(domain, "", this, useSsl, true, "application/json");
            julyHttp->setPortForced(port);
        }

        baseHeader = julyHttp->getHeader();

        connect(julyHttp, &JulyHttp::anyDataReceived, baseValues_->mainWindow_, &QtBitcoinTrader::anyDataReceived);
        connect(julyHttp, &JulyHttp::apiDown, baseValues_->mainWindow_, &QtBitcoinTrader::setApiDown);
        connect(julyHttp, &JulyHttp::setDataPending, baseValues_->mainWindow_, &QtBitcoinTrader::setDataPending);
        connect(julyHttp, &JulyHttp::errorSignal, baseValues_->mainWindow_, &QtBitcoinTrader::showErrorMessage);
        connect(julyHttp, &JulyHttp::sslErrorSignal, this, &Exchange::sslErrors);
        connect(julyHttp, &JulyHttp::dataReceived, this, &Exchange::dataReceivedAuth);
    }

    if (auth)
    {
        QByteArray timestamp = QByteArray::number(TimeSync::getMSecs());
        QByteArray contentHash = QCryptographicHash::hash(postData, QCryptographicHash::Sha512).toHex();//from requestBody
        QByteArray signBody = timestamp + "https://api.bittrex.com/v3/" + method + type + contentHash;
        QByteArray sign = QMessageAuthenticationCode::hash(signBody, getApiSign(), QCryptographicHash::Sha512).toHex();

        julyHttp->setHeader(baseHeader +
                            "Api-Key: " + getApiKey() + "\r\n" +
                            "Api-Timestamp: " + timestamp + "\r\n" +
                            "Api-Content-Hash: " + contentHash + "\r\n" +
                            "Api-Signature: " + sign + "\r\n");

        julyHttp->sendData(reqType, m_pairChangeCount, type + " /v3/" + method, postData);
    }
    else
    {
        julyHttp->setHeader(baseHeader);
        julyHttp->sendData(reqType, m_pairChangeCount, "GET /v3/" + method);
    }
}

void Exchange_Bittrex::sslErrors(const QList<QSslError>& errors)
{
    QStringList errorList;

    for (int n = 0; n < errors.size(); n++)
        errorList << errors.at(n).errorString();

    if (debugLevel)
        logThread->writeLog(errorList.join(" ").toLatin1(), 2);

    emit showErrorMessage("SSL Error: " + errorList.join(" "));
}
