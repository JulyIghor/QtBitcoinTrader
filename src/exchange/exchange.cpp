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

#include "exchange.h"
#include "currencypairitem.h"
#include "depthitem.h"
#include "iniengine.h"
#include "main.h"
#include <QFile>

Exchange::Exchange() : QObject()
{
    multiCurrencyTradeSupport = false;
    exchangeDisplayOnlyCurrentPairOpenOrders = false;
    orderBookItemIsDedicatedOrder = false;
    supportsExchangeFee = true;
    supportsExchangeVolume = true;
    clearOpenOrdersOnCurrencyChanged = false;
    clearHistoryOnCurrencyChanged = false;
    exchangeTickerSupportsHiLowPrices = true;
    depthEnabledFlag = true;
    balanceDisplayAvailableAmount = true;
    minimumRequestIntervalAllowed = 100;
    minimumRequestTimeoutAllowed = 2000;
    decAmountFromOpenOrder = 0.0;
    buySellAmountExcludedFee = false;
    calculatingFeeMode = 0;
    supportsLoginIndicator = true;
    supportsAccountVolume = true;
    exchangeSupportsAvailableAmount = false;
    checkDuplicatedOID = false;
    isLastTradesTypeSupported = true;
    forceDepthLoad = false;
    port = 0;
    useSsl = true;
    m_pairChangeCount = 1;

    clearVariables();
}

Exchange::~Exchange()
{
    if (debugLevel)
        logThread->writeLogB(baseValues.exchangeName + " API Thread Deleted", 2);
}

bool Exchange::isDepthEnabled() const
{
    return depthEnabledFlag || baseValues.scriptsThatUseOrderBookCount;
}

QByteArray Exchange::getMidData(const QString& a, const QString& b, const QByteArray* data)
{
    QByteArray rez;

    int startPos = data->indexOf(a.toLatin1(), 0);

    if (startPos > -1)
    {
        int endPos = data->indexOf(b.isEmpty() ? "\"," : b.toLatin1(), startPos + a.length());

        if (endPos > -1)
            rez = data->mid(startPos + a.length(), endPos - startPos - a.length());
    }

    return rez;
}

QByteArray Exchange::getMidVal(const QString& a, const QString& b, const QByteArray* data)
{
    QByteArray rez;

    int startPos = data->indexOf(a.toLatin1(), 0);

    if (startPos > -1)
    {
        startPos += a.length();

        if (startPos < data->size() && data->at(startPos) == '\"')
            ++startPos;

        int endPos = data->indexOf(b.isEmpty() ? "," : b.toLatin1(), startPos);

        if (endPos > -1)
        {
            if (data->at(endPos - 1) == '\"')
                --endPos;

            rez = data->mid(startPos, endPos - startPos);
        }
    }

    return rez;
}

void Exchange::run()
{
    if (debugLevel)
        logThread->writeLogB(baseValues.exchangeName + " API Thread Started", 2);

    clearVariables();

    QSettings iniSettings(baseValues.iniFileName, QSettings::IniFormat);
    domain = iniSettings.value("Domain").toString();
    port = static_cast<quint16>(iniSettings.value("Port", 0).toUInt());
    useSsl = iniSettings.value("SSL", true).toBool();

    if (domain.startsWith("http://"))
        domain.remove(0, 7);
    else if (domain.startsWith("https://"))
        domain.remove(0, 8);

    secondTimer.reset(new QTimer);
    secondTimer->setSingleShot(true);
    connect(secondTimer.data(), &QTimer::timeout, this, &Exchange::secondSlot);

    connect(QThread::currentThread(), &QThread::finished, this, &Exchange::quitExchange, Qt::DirectConnection);
    secondSlot();
    emit started();
}

void Exchange::quitExchange()
{
    secondTimer.reset();
    emit threadFinished();
}

void Exchange::secondSlot()
{
    if (secondTimer)
        secondTimer->start(baseValues.httpRequestInterval);
}

void Exchange::dataReceivedAuth(const QByteArray& /*unused*/, int /*unused*/, int /*unused*/)
{
}

void Exchange::reloadDepth()
{
    forceDepthLoad = true;
}

void Exchange::clearVariables()
{
    lastTickerLast = 0.0;
    lastTickerHigh = 0.0;
    lastTickerLow = 0.0;
    lastTickerSell = 0.0;
    lastTickerBuy = 0.0;
    lastTickerVolume = 0.0;

    lastBtcBalance = 0.0;
    lastUsdBalance = 0.0;
    lastAvUsdBalance = 0.0;
    lastVolume = 0.0;
    lastFee = -1.0;

    ++m_pairChangeCount;
}

void Exchange::filterAvailableUSDAmountValue(double* /*unused*/)
{
}

void Exchange::setupApi(QtBitcoinTrader* mainClass, bool tickOnly) // Execute only once
{
    IniEngine::loadExchangeLock(currencyMapFile, defaultCurrencyParams);
    tickerOnly = tickOnly;

    if (!tickerOnly)
    {
        connect(mainClass, &QtBitcoinTrader::apiBuy, this, &Exchange::buy);
        connect(mainClass, &QtBitcoinTrader::apiSell, this, &Exchange::sell);
        connect(mainClass, &QtBitcoinTrader::cancelOrderByOid, this, &Exchange::cancelOrder);
        connect(mainClass, &QtBitcoinTrader::getHistory, this, &Exchange::getHistory);

        connect(this, &Exchange::orderBookChanged, mainClass, &QtBitcoinTrader::orderBookChanged);
        connect(this, &Exchange::historyChanged, mainClass, &QtBitcoinTrader::historyChanged);
        connect(this, &Exchange::orderCanceled, mainClass, &QtBitcoinTrader::orderCanceled);
        connect(this, &Exchange::ordersIsEmpty, mainClass, &QtBitcoinTrader::ordersIsEmpty);
    }

    connect(this, &Exchange::depthRequested, mainClass, &QtBitcoinTrader::depthRequested);
    connect(this, &Exchange::depthRequestReceived, mainClass, &QtBitcoinTrader::depthRequestReceived);
    connect(this, &Exchange::depthSubmitOrders, mainClass, &QtBitcoinTrader::depthSubmitOrders);
    connect(this, &Exchange::depthFirstOrder, mainClass, &QtBitcoinTrader::depthFirstOrder);
    connect(this, &Exchange::showErrorMessage, mainClass, &QtBitcoinTrader::showErrorMessage);

    connect(this, &Exchange::availableAmountChanged, mainClass, &QtBitcoinTrader::availableAmountChanged);
    connect(mainClass, &QtBitcoinTrader::clearValues, this, &Exchange::clearValues);
    connect(mainClass, &QtBitcoinTrader::reloadDepth, this, &Exchange::reloadDepth);

    connect(this, &Exchange::accVolumeChanged, mainClass->ui.accountVolume, &QDoubleSpinBox::setValue);
    connect(this, &Exchange::accFeeChanged, mainClass, &QtBitcoinTrader::accFeeChanged);
    connect(this, &Exchange::accBtcBalanceChanged, mainClass, &QtBitcoinTrader::accBtcBalanceChanged);
    connect(this, &Exchange::accUsdBalanceChanged, mainClass, &QtBitcoinTrader::accUsdBalanceChanged);

    connect(this, &Exchange::loginChanged, mainClass, &QtBitcoinTrader::loginChanged);
    connect(this, &Exchange::addLastTrades, mainClass, &QtBitcoinTrader::addLastTrades);
}

void Exchange::setApiKeySecret(const QByteArray& key, const QByteArray& secret)
{
    if (!apiKeyChars.isEmpty())
        return;

    privateKey = key;

    for (int n = secret.size() - 1; n >= 0; n--)
        apiSignChars << new char(secret[n]);
}

QByteArray& Exchange::getApiKey()
{
    return privateKey;
}

QByteArray Exchange::getApiSign()
{
    QByteArray result;

    for (int n = apiSignChars.size() - 1; n >= 0; n--)
        result += *(apiSignChars[n]);

    return result;
}

void Exchange::clearValues()
{
}

void Exchange::getHistory(bool /*unused*/)
{
}

void Exchange::buy(const QString& /*unused*/, double /*unused*/, double /*unused*/)
{
}

void Exchange::sell(const QString& /*unused*/, double /*unused*/, double /*unused*/)
{
}

void Exchange::cancelOrder(const QString& /*unused*/, const QByteArray& /*unused*/)
{
}

void Exchange::sslErrors(const QList<QSslError>& errors)
{
    QStringList errorList;

    for (int n = 0; n < errors.size(); n++)
        errorList << errors.at(n).errorString();

    if (debugLevel)
        logThread->writeLog(errorList.join(" ").toLatin1(), 2);

    emit showErrorMessage("SSL Error: " + errorList.join(" "));
}

bool Exchange::checkValue(QByteArray& valueStr, double& lastValue)
{
    double value = valueStr.toDouble();

    if (!qFuzzyIsNull(value) && value < 0.0)
        return false;

    if (qFuzzyCompare(value + 1.0, lastValue + 1.0))
        return false;

    lastValue = value;
    return true;
}
