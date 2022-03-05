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

#include "iniengine.h"
#include "exchange/exchange.h"
#include "julyhttp.h"
#include "julyrsa.h"
#include "main.h"
#include <QDir>
#include <QFileInfo>
#include <QMutex>
#include <QThread>

IniEngine::IniEngine() :
    QObject(),
    iniEngineThread(new QThread),
    julyHttp(nullptr),
    currencyCacheFileName(appDataDir + "/cache/currencies.cache"),
    currencyResourceFileName("://Resources/Currencies.ini"),
    waitForDownload(true)
{
    iniEngineThread->setObjectName("Ini Engine");
    connect(this, &IniEngine::loadExchangeSignal, this, &IniEngine::loadExchange);
    connect(iniEngineThread, &QThread::started, this, &IniEngine::runThread);
    moveToThread(iniEngineThread);
    iniEngineThread->start();
}

IniEngine::~IniEngine()
{
    if (julyHttp)
        julyHttp->deleteLater();

    delete waitTimer;
    delete checkTimer;
    iniEngineThread->deleteLater();
}

void IniEngine::exitFromProgram()
{
    qDebug() << "The program is corrupted. Download from the official site https://centrabit.com.";
    exit(0);
}

void IniEngine::runThread()
{
    QSettings settingsMain(appDataDir + "/QtBitcoinTrader.cfg", QSettings::IniFormat);
    disablePairSynchronization = settingsMain.value("DisablePairSynchronization", false).toBool();

    if (disablePairSynchronization)
    {
        if (JulyRSA::isIniFileSigned(currencyResourceFileName))
            parseCurrency(currencyResourceFileName);
        else
            exitFromProgram();

        return;
    }

    waitTimer = new QTimer;
    connect(waitTimer, &QTimer::timeout, this, &IniEngine::checkWait);
    checkTimer = new QElapsedTimer;

    if (!QFile::exists(appDataDir + "/cache"))
        QDir().mkpath(appDataDir + "/cache");

    QFileInfo currencyFileInfo(currencyCacheFileName);

    if (currencyFileInfo.exists())
    {
        if (JulyRSA::isIniFileSigned(currencyCacheFileName))
        {
            parseCurrency(currencyCacheFileName);

            if (currencyFileInfo.lastModified() > QDateTime::currentDateTime().addDays(-1) &&
                currencyFileInfo.lastModified() < QDateTime::currentDateTime())
                return;
        }
        else
        {
            QFile(currencyCacheFileName).remove();

            if (JulyRSA::isIniFileSigned(currencyResourceFileName))
                parseCurrency(currencyResourceFileName);
            else
                exitFromProgram();
        }
    }
    else
    {
        if (JulyRSA::isIniFileSigned(currencyResourceFileName))
            parseCurrency(currencyResourceFileName);
        else
            exitFromProgram();
    }

    julyHttp = new JulyHttp("centrabit.com", "", this, true, false);
    connect(julyHttp, &JulyHttp::dataReceived, this, &IniEngine::dataReceived);
    julyHttp->secondTimer->stop();
    julyHttp->noReconnect = true;
    julyHttp->ignoreError = true;
    julyHttp->sendData(170, 0, "GET /Downloads/QBT_Resources/Currencies.ini", nullptr, nullptr, 0);
    julyHttp->destroyClass = true;
}

bool IniEngine::existNewFile(const QString& cacheFileName, const QByteArray& data)
{
    QFile cacheFile(cacheFileName);

    if (cacheFile.exists())
    {
        cacheFile.open(QIODevice::ReadWrite);
        QByteArray buffer = cacheFile.readAll();

        if (buffer != data)
        {
            cacheFile.resize(0);
            cacheFile.write(data);
            cacheFile.close();
        }
        else
        {
            qint64 fSize = cacheFile.size();
            cacheFile.resize(fSize + 1);
            cacheFile.resize(fSize);
            cacheFile.close();
            return false;
        }
    }
    else
    {
        cacheFile.open(QIODevice::WriteOnly);
        cacheFile.write(data);
        cacheFile.close();
    }

    return true;
}

void IniEngine::parseCurrency(const QString& currencyFileName)
{
    QSettings settingsCurrencies(currencyFileName, QSettings::IniFormat);
    QStringList currenciesList = settingsCurrencies.childGroups();

    for (int n = 0; n < currenciesList.size(); n++)
    {
        QString symbol = currenciesList.at(n);

        if (symbol.length() < 2 || symbol == "RSA2048Sign")
            continue;

        CurrencyInfo newCurr;
        newCurr.name = QString::fromUtf8(settingsCurrencies.value(symbol + "/Name", "").toByteArray());
        newCurr.sign = QString::fromUtf8(settingsCurrencies.value(symbol + "/Sign", "").toByteArray());
        newCurr.valueSmall = settingsCurrencies.value(symbol + "/ValueSmall", 0.1).toDouble();

        if (!baseValues.supportsUtfUI)
        {
            QString bufferSign = QString::fromUtf8(settingsCurrencies.value(symbol + "/SignNonUTF8", "").toByteArray());

            if (!bufferSign.isEmpty())
                newCurr.sign = bufferSign;
        }

        if (newCurr.isValid())
        {
            currencyMap[symbol] = newCurr;
            currencyMapSign[newCurr.sign] = symbol;
        }
    }
}

void IniEngine::dataReceived(const QByteArray& data, int reqType, int /*unused*/)
{
    if (!waitForDownload)
        return;

    switch (reqType)
    {
    case 170:
        {
            if (!existNewFile(currencyCacheFileName, data))
                break;

            if (!JulyRSA::isIniFileSigned(currencyCacheFileName))
            {
                QFile(currencyCacheFileName).remove();
                break;
            }

            parseCurrency(currencyCacheFileName);
        }
        break;

    case 171:
        {
            existNewFile(exchangeCacheFileName, data);
            parseExchangeCheck();
        }
        break;
    }
}

void IniEngine::loadExchangeLock(const QString& exchangeIniFileName, CurrencyPairItem& tempDefaultExchangeParams)
{
    IniEngine* instance = IniEngine::global();
    instance->defaultExchangeParams = tempDefaultExchangeParams;
    emit instance->loadExchangeSignal(exchangeIniFileName);

    while (instance->waitForDownload)
    {
        QThread::msleep(100);
    }
}

void IniEngine::loadExchange(const QString& exchangeIniFileName)
{
    exchangeCacheFileName = appDataDir + "/cache/" + exchangeIniFileName + ".cache";
    exchangeResourceFileName = ":/Resources/Exchanges/" + exchangeIniFileName + ".ini";

    if (disablePairSynchronization)
    {
        if (JulyRSA::isIniFileSigned(exchangeResourceFileName))
            parseExchange(exchangeResourceFileName);
        else
            exitFromProgram();

        return;
    }

    QFileInfo exchangeFileInfo(exchangeCacheFileName);

    if (exchangeFileInfo.exists())
    {
        if (JulyRSA::isIniFileSigned(exchangeCacheFileName))
        {
            if (exchangeFileInfo.lastModified() > QDateTime::currentDateTime().addDays(-1) &&
                exchangeFileInfo.lastModified() < QDateTime::currentDateTime())
            {
                parseExchange(exchangeCacheFileName);
                return;
            }
        }
        else
            QFile(exchangeCacheFileName).remove();
    }

    if (julyHttp == nullptr)
    {
        julyHttp = new JulyHttp("centrabit.com", "", this, true, false);
        connect(julyHttp, &JulyHttp::dataReceived, this, &IniEngine::dataReceived);
        julyHttp->secondTimer->stop();
        julyHttp->noReconnect = true;
        julyHttp->ignoreError = true;
    }

    checkTimer->start();
    julyHttp->destroyClass = false;
    julyHttp->sendData(171, 0, "GET /Downloads/QBT_Resources/Exchanges/" + exchangeIniFileName.toLatin1() + ".ini", nullptr, nullptr, 0);
    julyHttp->destroyClass = true;

    if (waitForDownload)
        waitTimer->start(100);
}

void IniEngine::checkWait()
{
    if (checkTimer->elapsed() > 5000)
    {
        waitTimer->stop();
        parseExchangeCheck();
    }
}

void IniEngine::parseExchangeCheck()
{
    if (JulyRSA::isIniFileSigned(exchangeCacheFileName))
        parseExchange(exchangeCacheFileName);
    else
    {
        QFile(currencyCacheFileName).remove();

        if (JulyRSA::isIniFileSigned(exchangeResourceFileName))
            parseExchange(exchangeResourceFileName);
        else
            exitFromProgram();
    }
}

const QString& IniEngine::checkSymbol(const QString& symbol)
{
    if (symbol.contains("/"))
        return symbol;

    auto iPairs = IniEngine::global()->exchangeAltPairs.find(symbol.toUpper());

    if (iPairs == IniEngine::global()->exchangeAltPairs.end())
        return symbol;

    return iPairs.value();
}

void IniEngine::parseExchange(const QString& exchangeFileName)
{
    QSettings settingsParams(exchangeFileName, QSettings::IniFormat);
    QStringList symbolList = settingsParams.childGroups();
    exchangePairs.clear();
    exchangeAltPairs.clear();
    symbolByRequest.clear();

    for (int n = 0; n < symbolList.size(); ++n)
    {
        CurrencyPairItem currentPair = defaultExchangeParams;
        currentPair.name = settingsParams.value(symbolList.at(n) + "/Symbol", "").toByteArray();

        if (currentPair.name.length() < 5)
            continue;

        int twoDotsIndex = currentPair.name.indexOf(':');

        if (twoDotsIndex > -1)
            currentPair.name.replace(twoDotsIndex, 1, '/');

        currentPair.setSymbol(currentPair.name.toLatin1());

        if (currentPair.name.size() == 6 && currentPair.name.indexOf('/') == -1)
            currentPair.name.insert(3, "/");

        currentPair.currRequestSecond = settingsParams.value(symbolList.at(n) + "/RequestSecond", "").toByteArray();

        if (!currentPair.currRequestSecond.isEmpty())
            currentPair.name.append(" [" + currentPair.currRequestSecond + "]");

        currentPair.currRequestPair = settingsParams.value(symbolList.at(n) + "/Request", "").toByteArray();

        if (currentPair.currRequestPair.isEmpty())
            continue;

        currentPair.priceDecimals = settingsParams.value(symbolList.at(n) + "/PriceDecimals", "").toInt();
        currentPair.priceMin = settingsParams.value(symbolList.at(n) + "/PriceMin", "").toDouble();
        currentPair.tradeVolumeMin = settingsParams.value(symbolList.at(n) + "/TradeVolumeMin", "").toDouble();
        currentPair.tradeTotalMin = settingsParams.value(symbolList.at(n) + "/TradeTotalMin", "").toDouble();
        currentPair.tradePriceMin = settingsParams.value(symbolList.at(n) + "/TradePriceMin", "").toDouble();
        currentPair.currADecimals = settingsParams.value(symbolList.at(n) + "/ItemDecimals").toInt();
        currentPair.currBDecimals = settingsParams.value(symbolList.at(n) + "/ValueDecimals").toInt();
        exchangePairs.append(currentPair);
        exchangeAltPairs[currentPair.currAStr.toUpper() + currentPair.currBStr.toUpper()] = currentPair.name;
        symbolByRequest[currentPair.currRequestPair] = currentPair.symbol;
    }

    if (julyHttp)
    {
        waitTimer->stop();
        julyHttp->deleteLater();
        julyHttp = nullptr;
    }

    waitForDownload = false;
}

// void IniEngine::loadPairs(QStringList* pairsList)
//{
//     exchangePairs.clear();

//    for (int n = 0; n < pairsList->size(); ++n)
//    {
//        QByteArray pairData = pairsList->at(n).toUtf8();
//        CurrencyPairItem currentPair = defaultExchangeParams;
//        currentPair.name = Exchange::getMidData("Symbol\":\"", "\"", &pairData);

//        if (currentPair.name.length() < 5)
//            continue;

//        int twoDotsIndex = currentPair.name.indexOf(':');

//        if (twoDotsIndex > -1)
//            currentPair.name.replace(twoDotsIndex, 1, '/');

//        currentPair.setSymbol(currentPair.name.toLatin1());

//        if (currentPair.name.size() == 6 && currentPair.name.indexOf('/') == -1)
//            currentPair.name.insert(3, "/");

//        currentPair.currRequestSecond = Exchange::getMidData("RequestSecond\":\"", "\"", &pairData);
//        currentPair.currRequestPair   = Exchange::getMidData("Request\":\"", "\"", &pairData);

//        if (currentPair.currRequestPair.isEmpty())
//            continue;

//        currentPair.priceMin       = Exchange::getMidData("PriceMin\":\"", "\"", &pairData).toDouble();
//        currentPair.tradeVolumeMin = Exchange::getMidData("TradeVolumeMin\":\"", "\"", &pairData).toDouble();
//        currentPair.tradePriceMin  = Exchange::getMidData("TradePriceMin\":\"", "\"", &pairData).toDouble();
//        currentPair.tradeTotalMin  = Exchange::getMidData("TradeTotalMin\":\"", "\"", &pairData).toDouble();
//        currentPair.currADecimals  = Exchange::getMidData("ItemDecimals\":\"", "\"", &pairData).toInt();
//        currentPair.priceDecimals  = Exchange::getMidData("PriceDecimals\":\"", "\"", &pairData).toInt();
//        currentPair.currBDecimals  = Exchange::getMidData("ValueDecimals\":\"", "\"", &pairData).toInt();
//        exchangePairs.append(currentPair);
//    }

//    delete pairsList;
//}

IniEngine* IniEngine::global()
{
    static IniEngine instance;
    return &instance;
}

CurrencyInfo IniEngine::getCurrencyInfo(const QString& symbol)
{
    return IniEngine::global()->currencyMap.value(symbol, CurrencyInfo("$"));
}

QList<CurrencyPairItem>* IniEngine::getPairs()
{
    return &IniEngine::global()->exchangePairs;
}

QString IniEngine::getPairName(int index)
{
    if (index >= 0 && index < IniEngine::global()->exchangePairs.size())
        return IniEngine::global()->exchangePairs.at(index).name;
    return "";
}

QString IniEngine::getPairRequest(int index)
{
    if (index >= 0 && index < IniEngine::global()->exchangePairs.size())
        return IniEngine::global()->exchangePairs.at(index).currRequestPair;
    return "";
}

QString IniEngine::getPairSymbol(int index)
{
    if (index >= 0 && index < IniEngine::global()->exchangePairs.size())
        return IniEngine::global()->exchangePairs.at(index).symbol;
    return "";
}

QString IniEngine::getPairSymbolSecond(int index)
{
    if (index >= 0 && index < IniEngine::global()->exchangePairs.size())
        return IniEngine::global()->exchangePairs.at(index).symbolSecond();
    return "";
}

int IniEngine::getPairsCount()
{
    return IniEngine::global()->exchangePairs.size();
}

QString IniEngine::getSymbolByRequest(const QString& request)
{
    return IniEngine::global()->symbolByRequest.value(request);
}
