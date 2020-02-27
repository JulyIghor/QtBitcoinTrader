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

#include "exchange.h"
#include "main.h"
#include "depthitem.h"
#include <QFile>
#include "currencypairitem.h"
#include "iniengine.h"

Exchange::Exchange()
    : QObject()
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
    port   = 0;
    useSsl = true;

    clearVariables();
}

Exchange::~Exchange()
{
    if (debugLevel)
        logThread->writeLogB(baseValues.exchangeName + " API Thread Deleted", 2);
}

bool Exchange::isDepthEnabled()
{
    return depthEnabledFlag || baseValues.scriptsThatUseOrderBookCount;
}

QByteArray Exchange::getMidData(QString a, QString b, QByteArray* data)
{
    QByteArray rez;

    if (b.isEmpty())
        b = "\",";

    int startPos = data->indexOf(a, 0);

    if (startPos > -1)
    {
        int endPos = data->indexOf(b, startPos + a.length());

        if (endPos > -1)
            rez = data->mid(startPos + a.length(), endPos - startPos - a.length());
    }

    return rez;
}

QByteArray Exchange::getMidVal(QString a, QString b, QByteArray* data)
{
    QByteArray rez;

    if (b.isEmpty())
        b = ",";

    int startPos = data->indexOf(a, 0);

    if (startPos > -1)
    {
        startPos += a.length();

        if (startPos < data->size() && data->at(startPos) == '\"')
            ++startPos;

        int endPos = data->indexOf(b, startPos);

        if (endPos > -1)
        {
            if (data->at(endPos - 1) == '\"')
                --endPos;

            rez = data->mid(startPos, endPos - startPos);
        }
    }

    return rez;
}

void Exchange::translateUnicodeStr(QString* str)
{
    const QRegExp rx("(\\\\u[0-9a-fA-F]{4})");
    int pos = 0;

    while ((pos = rx.indexIn(*str, pos)) != -1)
        str->replace(pos++, 6, QChar(rx.cap(1).right(4).toUShort(0, 16)));
}

void Exchange::translateUnicodeOne(QByteArray* str)
{
    if (!str->contains("\\u"))
        return;

    QStringList bytesList = QString(*str).split("\\u");

    if (bytesList.count())
        bytesList.removeFirst();
    else
        return;

    QString strToReturn;

    for (int n = 0; n < bytesList.count(); n++)
        if (bytesList.at(n).length() > 3)
            strToReturn += "\\u" + bytesList.at(n).left(4);

    translateUnicodeStr(&strToReturn);
    *str = strToReturn.toLatin1();
}

void Exchange::run()
{
    if (debugLevel)
        logThread->writeLogB(baseValues.exchangeName + " API Thread Started", 2);

    clearVariables();

    QSettings iniSettings(baseValues.iniFileName, QSettings::IniFormat);
    domain = iniSettings.value("Domain").toString();
    port   = static_cast<quint16>(iniSettings.value("Port", 0).toUInt());
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

void Exchange::dataReceivedAuth(QByteArray, int)
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
}

void Exchange::filterAvailableUSDAmountValue(double*)
{

}

void Exchange::setupApi(QtBitcoinTrader* mainClass, bool tickOnly)//Execute only once
{
    IniEngine::loadExchangeLock(currencyMapFile, defaultCurrencyParams);
    tickerOnly = tickOnly;

    if (!tickerOnly)
    {
        connect(mainClass, SIGNAL(apiBuy(QString, double, double)), this, SLOT(buy(QString, double, double)));
        connect(mainClass, SIGNAL(apiSell(QString, double, double)), this, SLOT(sell(QString, double, double)));
        connect(mainClass, SIGNAL(cancelOrderByOid(QString, QByteArray)), this, SLOT(cancelOrder(QString, QByteArray)));
        connect(mainClass, SIGNAL(getHistory(bool)), this, SLOT(getHistory(bool)));

        connect(this, SIGNAL(orderBookChanged(QString, QList<OrderItem>*)), mainClass, SLOT(orderBookChanged(QString,
                QList<OrderItem>*)));
        connect(this, SIGNAL(historyChanged(QList<HistoryItem>*)), mainClass, SLOT(historyChanged(QList<HistoryItem>*)));
        connect(this, SIGNAL(orderCanceled(QString, QByteArray)), mainClass, SLOT(orderCanceled(QString, QByteArray)));
        connect(this, SIGNAL(ordersIsEmpty()), mainClass, SLOT(ordersIsEmpty()));
    }

    connect(this, SIGNAL(depthRequested()), mainClass, SLOT(depthRequested()));
    connect(this, SIGNAL(depthRequestReceived()), mainClass, SLOT(depthRequestReceived()));
    connect(this, SIGNAL(depthSubmitOrders(QString, QList<DepthItem>*, QList<DepthItem>*)), mainClass,
            SLOT(depthSubmitOrders(QString, QList<DepthItem>*, QList<DepthItem>*)));
    connect(this, SIGNAL(depthFirstOrder(QString, double, double, bool)), mainClass, SLOT(depthFirstOrder(QString, double,
            double, bool)));
    connect(this, SIGNAL(showErrorMessage(QString)), mainClass, SLOT(showErrorMessage(QString)));

    connect(this, SIGNAL(availableAmountChanged(QString, double)), mainClass, SLOT(availableAmountChanged(QString,
            double)));
    connect(mainClass, SIGNAL(clearValues()), this, SLOT(clearValues()));
    connect(mainClass, SIGNAL(reloadDepth()), this, SLOT(reloadDepth()));

    connect(this, SIGNAL(accVolumeChanged(double)), mainClass->ui.accountVolume, SLOT(setValue(double)));
    connect(this, SIGNAL(accFeeChanged(QString, double)), mainClass, SLOT(accFeeChanged(QString, double)));
    connect(this, SIGNAL(accBtcBalanceChanged(QString, double)), mainClass, SLOT(accBtcBalanceChanged(QString, double)));
    connect(this, SIGNAL(accUsdBalanceChanged(QString, double)), mainClass, SLOT(accUsdBalanceChanged(QString, double)));

    connect(this, SIGNAL(loginChanged(QString)), mainClass, SLOT(loginChanged(QString)));

    connect(this, SIGNAL(addLastTrades(QString, QList<TradesItem>*)), mainClass, SLOT(addLastTrades(QString,
            QList<TradesItem>*)));
}

void Exchange::setApiKeySecret(QByteArray key, QByteArray secret)
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

void Exchange::getHistory(bool)
{
}

void Exchange::buy(QString, double, double)
{
}

void Exchange::sell(QString, double, double)
{
}

void Exchange::cancelOrder(QString, QByteArray)
{
}

void Exchange::sslErrors(const QList<QSslError>& errors)
{
    QStringList errorList;

    for (int n = 0; n < errors.count(); n++)
        errorList << errors.at(n).errorString();

    if (debugLevel)
        logThread->writeLog(errorList.join(" ").toLatin1(), 2);

    emit showErrorMessage("SSL Error: " + errorList.join(" "));
}

bool Exchange::checkValue(QByteArray& valueStr, double& lastValue)
{
    if (valueStr.isEmpty())
        return false;

    double value = valueStr.toDouble();

    if (!qFuzzyIsNull(value) && value < 0.0)
        return false;

    if (qFuzzyCompare(value + 1.0, lastValue + 1.0))
        return false;

    lastValue = value;
    return true;
}
