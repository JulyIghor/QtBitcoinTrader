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

#ifndef INIENGINE_H
#define INIENGINE_H

#include <QObject>
#include <QMap>
#include <atomic>
#include "currencyinfo.h"
#include "currencypairitem.h"

class QThread;
class JulyHttp;
class QTimer;
class QElapsedTimer;

class IniEngine : public QObject
{
    Q_OBJECT

public:
    IniEngine();
    ~IniEngine();
    static IniEngine* global();
    static CurrencyInfo getCurrencyInfo(QString);
    static void loadExchangeLock(QString, CurrencyPairItem&);
    static QList<CurrencyPairItem>* getPairs();
    static QString getPairName(int);
    static QString getPairRequest(int);
    static QString getPairSymbol(int);
    static QString getPairSymbolSecond(int);
    static int getPairsCount();
    void loadPairs(QStringList* pairsList);

signals:
    void loadExchangeSignal(QString);

private slots:
    void runThread();
    void dataReceived(QByteArray, int);
    void loadExchange(QString);
    void checkWait();

private:
    QThread* iniEngineThread;
    bool disablePairSynchronization;
    JulyHttp* julyHttp;
    QMap<QString, QString> currencyMapSign;
    QMap<QString, CurrencyInfo> currencyMap;
    QString currencyCacheFileName;
    QString currencyResourceFileName;
    QString exchangeCacheFileName;
    QString exchangeResourceFileName;
    CurrencyPairItem defaultExchangeParams;
    QList<CurrencyPairItem> exchangePairs;
    std::atomic<bool> waitForDownload;
    QTimer* waitTimer;
    QElapsedTimer* checkTimer;
    bool existNewFile(QString, QByteArray&);
    void parseCurrency(QString);
    void parseExchange(QString);
    void parseExchangeCheck();
    [[noreturn]] void exitFromProgram();
};

#endif // INIENGINE_H
