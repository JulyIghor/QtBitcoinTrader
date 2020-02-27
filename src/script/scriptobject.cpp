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

#include "scriptobject.h"
#include <QVariant>
#include <QVariantList>
#include <QStringList>
#include <QTime>
#include "exchange/exchange.h"
#include "main.h"
#include "timesync.h"
#include "time.h"
#include <QMetaMethod>
#include <QDoubleSpinBox>
#include <QScriptEngine>
#include <QScriptValueIterator>

ScriptObject::ScriptObject(const QString& _scriptName) :
    QObject()
{
    scriptWantsOrderBookData = false;
    haveTimer = false;
    secondTimer = nullptr;
    pendingStop = false;
    testResult = 0;
    scriptName = _scriptName;
    isRunningFlag = false;
    engine = nullptr;
    testMode = true;

    for (int n = staticMetaObject.methodOffset(); n < staticMetaObject.methodCount(); n++)
    {
        if (staticMetaObject.method(n).methodType() != QMetaMethod::Slot ||
            staticMetaObject.method(n).access() != QMetaMethod::Public)
            continue;

        QString currentCommand;
#if QT_VERSION<0x050000
        currentCommand = QString::fromLocal8Bit(staticMetaObject.method(n).signature());
        currentCommand = currentCommand.split("(").first();
#else
        currentCommand = QString::fromLocal8Bit(staticMetaObject.method(n).name());
#endif

        if (currentCommand.startsWith(QLatin1String("get")) || currentCommand.startsWith(QLatin1String("test")) ||
            commandsList.contains(currentCommand))
            continue;

        if (currentCommand == QLatin1String("groupDone"))
            continue;

        QList<QByteArray> parameters = staticMetaObject.method(n).parameterNames();

        if (!baseValues.currentExchange_->multiCurrencyTradeSupport && parameters.contains("symbol"))
            continue;

        if (commandsList.contains("trader." + currentCommand))
            continue;

        addCommand(currentCommand, parameters);
    }

    Q_FOREACH (QDoubleSpinBox* spinBox, mainWindow.indicatorsMap.values())
    {
        QString scriptName = spinBox->whatsThis();

        if (scriptName.isEmpty())
            continue;

        indicatorsMap[baseValues.currentPair.symbolSecond() + "_" + scriptName] = spinBox->value();
        addIndicator(spinBox, scriptName);
    }

    indicatorList << "trader.on(\"AnyValue\").changed";
    indicatorList << "trader.on(\"Time\").changed";
    indicatorList << "trader.on(\"LastTrade\").changed";
    indicatorList << "trader.on(\"MyLastTrade\").changed";
    indicatorList << "trader.on(\"OpenOrdersCount\").changed";
    indicatorList << "trader.on(\"OpenAsksCount\").changed";
    indicatorList << "trader.on(\"OpenBidsCount\").changed";

    functionsList << "trader.get(\"Time\")";

    functionsList << "trader.get(\"AsksPrice\",volume)";
    functionsList << "trader.get(\"AsksVolume\",price)";
    functionsList << "trader.get(\"BidsPrice\",volume)";
    functionsList << "trader.get(\"BidsVolume\",price)";

    functionsList << "trader.get(\"OpenOrdersCount\")";
    functionsList << "trader.get(\"OpenAsksCount\")";
    functionsList << "trader.get(\"OpenBidsCount\")";

    indicatorList.removeDuplicates();
    functionsList.removeDuplicates();

    connect(this, SIGNAL(setGroupEnabled(QString, bool)), baseValues.mainWindow_, SLOT(setGroupRunning(QString, bool)));
    connect(this, SIGNAL(startAppSignal(QString, QStringList)), baseValues.mainWindow_, SLOT(startApplication(QString,
            QStringList)));

    connect(baseValues.mainWindow_, SIGNAL(indicatorEventSignal(QString, QString, double)), this,
            SLOT(initValueChanged(QString, QString, double)));
    connect(this, SIGNAL(eventSignal(QString, QString, double)), baseValues.mainWindow_, SLOT(sendIndicatorEvent(QString,
            QString, double)));

    secondTimer = new QTimer(this);
    secondTimer->setSingleShot(true);
    connect(secondTimer, SIGNAL(timeout()), this, SLOT(secondSlot()));

    connect(this, SIGNAL(performFileWrite(QString, QByteArray)), &performThread, SLOT(performFileWrite(QString,
            QByteArray)));
    connect(this, SIGNAL(performFileAppend(QString, QByteArray)), &performThread, SLOT(performFileAppend(QString,
            QByteArray)));
    connect(this, SIGNAL(performFileReadLine(QString, qint64, quint32)), &performThread, SLOT(performFileReadLine(QString,
            qint64, quint32)));
    connect(this, SIGNAL(performFileReadLineSimple(QString, quint32)), &performThread,
            SLOT(performFileReadLineSimple(QString, quint32)));
    connect(this, SIGNAL(performFileRead(QString, qint64, quint32)), &performThread, SLOT(performFileRead(QString, qint64,
            quint32)));
    connect(this, SIGNAL(performFileReadAll(QString, quint32)), &performThread, SLOT(performFileReadAll(QString, quint32)));
    connect(&performThread, SIGNAL(fileReadResult(QByteArray, quint32)), this, SLOT(fileReadResult(QByteArray, quint32)));
    fileOperationNumber = 0;
    fileOpenCount = 0;
}

ScriptObject::~ScriptObject()
{

}

void ScriptObject::sendEvent(const QString& name, double value)
{
    if (testMode)
        return;

    emit eventSignal(baseValues.currentPair.symbolSecond(), name, value);
}

void ScriptObject::sendEvent(const QString& symbol, const QString& name, double value)
{
    if (testMode)
        return;

    emit eventSignal(symbol, name, value);
}

int ScriptObject::getOpenAsksCount()
{
    return mainWindow.getOpenOrdersCount(-1);
}

int ScriptObject::getOpenBidsCount()
{
    return mainWindow.getOpenOrdersCount(1);
}

int ScriptObject::getOpenOrdersCount()
{
    return mainWindow.getOpenOrdersCount(0);
}

void ScriptObject::test(int val)
{
    testResult = val;
}

double ScriptObject::getAsksVolByPrice(double volume)
{
    return getAsksVolByPrice(baseValues.currentPair.symbolSecond(), volume);
}
double ScriptObject::getAsksPriceByVol(double price)
{
    return getAsksPriceByVol(baseValues.currentPair.symbolSecond(), price);
}
double ScriptObject::getBidsVolByPrice(double volume)
{
    return getBidsVolByPrice(baseValues.currentPair.symbolSecond(), volume);
}
double ScriptObject::getBidsPriceByVol(double price)
{
    return getBidsPriceByVol(baseValues.currentPair.symbolSecond(), price);
}

double ScriptObject::getAsksPriceByVol(const QString& symbol, double price)
{
    return orderBookInfo(symbol, price, true, true);
}
double ScriptObject::getAsksVolByPrice(const QString& symbol, double volume)
{
    return orderBookInfo(symbol, volume, true, false);
}
double ScriptObject::getBidsPriceByVol(const QString& symbol, double price)
{
    return orderBookInfo(symbol, price, false, true);
}
double ScriptObject::getBidsVolByPrice(const QString& symbol, double volume)
{
    return orderBookInfo(symbol, volume, false, false);
}

double ScriptObject::orderBookInfo(const QString& symbol, double& value, bool isAsk, bool getPrice)
{
    double result = 0.0;

    if (getPrice)
        result = mainWindow.getPriceByVolume(symbol, value, isAsk);
    else
        result = mainWindow.getVolumeByPrice(symbol, value, isAsk);

    if (result < 0.0)
    {
        result = -result;

        if (!testMode)
            emit writeLog("Warning! OrderBook info is out of range. OrderBook information is limited to rows count limit.");
    }

    return result;
}

double ScriptObject::get(const QString& indicator)
{
    return get(baseValues.currentPair.symbolSecond(), indicator);
}

void ScriptObject::timerCreate(int milliseconds, const QString& command, bool once)
{
    if (testMode)
        return;

    if (engine == nullptr || command.isEmpty())
        return;

    if (milliseconds < 0)
    {
        emit writeLog("Timer have less than 0 interval");
        return;
    }

    QTimer* newTimer = new QTimer(this);
    timerMap.insert(newTimer, true);
    newTimer->setSingleShot(once);
    newTimer->setProperty("Command", command);
    newTimer->setProperty("DeleteMe", once);
    connect(newTimer, SIGNAL(timeout()), this, SLOT(timerOut()));
    newTimer->start(milliseconds);
}

void ScriptObject::delay(double seconds, const QString& command)
{
    timerCreate(seconds * 1000, command, true);
}

void ScriptObject::timer(double seconds, const QString& command)
{
    timerCreate(seconds * 1000, command, false);
}

void ScriptObject::timerOut()
{
    QTimer* senderTimer = qobject_cast<QTimer*>(sender());

    if (senderTimer == nullptr)
        return;

    QString command = senderTimer->property("Command").toString();

    if (senderTimer->property("DeleteMe").toBool())
    {
        timerMap.remove(senderTimer);
        senderTimer->deleteLater();
    }

    if (command.isEmpty())
        return;

    if (engine == nullptr)
        return;

    engine->evaluate(command.replace("\\", "\\\\"));
}

double ScriptObject::get(const QString& symbol, const QString& indicator)
{
    QString indicatorLower = indicator.toLower();

    if (indicatorLower == QLatin1String("time"))
        return getTimeT();

    if (indicatorLower == QLatin1String("openorderscount"))
        return getOpenOrdersCount();

    if (indicatorLower == QLatin1String("openaskscount"))
        return getOpenAsksCount();

    if (indicatorLower == QLatin1String("openbidscount"))
        return getOpenBidsCount();

    QString symbolCopy = symbol;

    if (indicator.length() >= 8 && indicatorLower.startsWith(QLatin1String("balance")))
        symbolCopy.clear();
    else if (!symbolCopy.isEmpty())
        symbolCopy.append("_");

    return indicatorsMap.value(symbolCopy + indicator, 0.0);
}

void ScriptObject::startApp(const QString& name)
{
    startApp(name, QStringList());
}
void ScriptObject::startApp(const QString& name, const QString& arg1)
{
    startApp(name, QStringList() << arg1);
}
void ScriptObject::startApp(const QString& name, const QString& arg1, const QString& arg2)
{
    startApp(name, QStringList() << arg1 << arg2);
}
void ScriptObject::startApp(const QString& name, const QString& arg1, const QString& arg2, const QString& arg3)
{
    startApp(name, QStringList() << arg1 << arg2 << arg3);
}
void ScriptObject::startApp(const QString& name, const QString& arg1, const QString& arg2, const QString& arg3, const QString& arg4)
{
    startApp(name, QStringList() << arg1 << arg2 << arg3 << arg4);
}
void ScriptObject::startApp(const QString& name, const QStringList& list)
{
    if (testMode)
        return;

    emit startAppSignal(name, list);

    writeLog(julyTr("START_APPLICATION", "Start application: %1").arg(list.count() ? name + " " + list.join(" ") : name));
}

void ScriptObject::beep()
{
    if (testMode)
        return;

    mainWindow.beep(false);
}

void ScriptObject::playWav(const QString& fileName)
{
    if (testMode)
        return;

    if (fileName.isEmpty())
    {
        beep();
        return;
    }

    mainWindow.playWav(fileName, false);
}

void ScriptObject::say(const QString& text)
{
    if (!testMode)
        mainWindow.sayText(text);
}
void ScriptObject::say(double text)
{
    say(QVariantList() << text);
}
void ScriptObject::say(int text)
{
    say(QVariantList() << text);
}
void ScriptObject::say(const QVariant& text1)
{
    say(QVariantList() << text1);
}
void ScriptObject::say(const QVariant& text1, const QVariant& text2)
{
    say(QVariantList() << text1 << text2);
}
void ScriptObject::say(const QVariant& text1, const QVariant& text2, const QVariant& text3)
{
    say(QVariantList() << text1 << text2 << text3);
}
void ScriptObject::say(const QVariant& text1, const QVariant& text2, const QVariant& text3, const QVariant& text4)
{
    say(QVariantList() << text1 << text2 << text3 << text4);
}
void ScriptObject::say(const QVariant& text1, const QVariant& text2, const QVariant& text3, const QVariant& text4, const QVariant& text5)
{
    say(QVariantList() << text1 << text2 << text3 << text4 << text5);
}
void ScriptObject::say(const QVariant& text1, const QVariant& text2, const QVariant& text3, const QVariant& text4, const QVariant& text5,
                       const QVariant& text6)
{
    say(QVariantList() << text1 << text2 << text3 << text4 << text5 << text6);
}
void ScriptObject::say(const QVariantList& list)
{
    int detectDoublePoint = 0;

    bool decimalComma = QLocale().decimalPoint() == QChar(',');

    if (!decimalComma)
        decimalComma = QLocale().country() == QLocale::Ukraine || QLocale().country() == QLocale::RussianFederation;

    if (decimalComma)
        detectDoublePoint = 2;
    else
        detectDoublePoint = 1;

    QString text;

    for (int n = 0; n < list.count(); n++)
    {
        if (list.at(n).type() == QVariant::Double)
        {
            QString number = JulyMath::textFromDouble(list.at(n).toDouble(), 8, 0);

            if (detectDoublePoint == 2)
                number.replace(QLatin1Char('.'), QLatin1Char(','));

            text += number;
        }
        else
            text += list.at(n).toString();

        if (n < list.count() - 1)
            text += QLatin1Char(' ');
    }

    say(text);
}

void ScriptObject::groupStart(QString name)
{
    if (testMode)
        return;

    if (name.isEmpty())
        name = scriptName;

    log("Start group: \"" + name + "\"");
    emit setGroupEnabled(name, true);
}

void ScriptObject::groupStop()
{
    if (testMode)
        return;

    groupStop(QLatin1String(""));
}

void ScriptObject::groupDone()
{
    if (testMode)
        return;

    pendingStop = true;
    emit setGroupDone(scriptName);
    setRunning(false);
}

void ScriptObject::groupStop(QString name)
{
    if (testMode)
        return;

    if (name.isEmpty())
        name = scriptName;

    log("Stop group: \"" + name + "\"");

    if (name == scriptName)
    {
        pendingStop = true;
        setRunning(false);
    }
    else
        emit setGroupEnabled(name, false);
}

void ScriptObject::addIndicator(QDoubleSpinBox* spinBox, QString value)
{
    if (indicatorList.contains(value))
        return;

    indicatorList << "trader.on(\"" + value + "\").changed";
    spinBox->setProperty("ScriptName", value);
    spinBoxList << spinBox;
    indicatorsMap.insert(baseValues.currentPair.symbolSecond() + "_" + value, spinBox->value());
    functionsList << "trader.get(\"" + value + "\")";
}

void ScriptObject::addCommand(const QString& name, QList<QByteArray> parameters)
{
    if (!commandsList.contains(name))
    {
        commandsList << "trader." + name;
        QString params;

        for (int n = 0; n < parameters.count(); n++)
        {
            if (parameters.at(n).isEmpty())
                continue;

            parameters[n].replace("_", "\"");

            if (params.length())
                params.append(",");

            params.append(parameters.at(n));
        }

        argumentsList << params;
    }
}

void ScriptObject::addFunction(const QString& name)
{
    if (!functionsList.contains(name))
        functionsList << QString(name).replace('_', '.');
}

void ScriptObject::buy(double amount, double price)
{
    buy("", amount, price);
}

void ScriptObject::sell(double amount, double price)
{
    sell("", amount, price);
}

void ScriptObject::buy(const QString& symbolR, double amount, double price)
{
    QString symbol = symbolR;

    if (symbol.isEmpty())
        symbol = baseValues.currentPair.symbolSecond();
    else
        symbol = symbol.toUpper();

    if (!testMode)
        mainWindow.apiBuySend(symbol, amount, price);

    log(symbol + ": Buy " + JulyMath::textFromDouble(amount, 8, 0) + " at " + JulyMath::textFromDouble(price, 8, 0));
}

void ScriptObject::sell(const QString& symbolR, double amount, double price)
{
    QString symbol = symbolR;

    if (symbol.isEmpty())
        symbol = baseValues.currentPair.symbolSecond();
    else
        symbol = symbol.toUpper();

    if (!testMode)
        mainWindow.apiSellSend(symbol, amount, price);

    log(symbol + ": Sell " + JulyMath::textFromDouble(amount, 8, 0) + " at " + JulyMath::textFromDouble(price, 8, 0));
}

void ScriptObject::cancelOrders()
{
    if (!testMode)
        mainWindow.cancelPairOrders("");

    log("Cancel all orders");
}

void ScriptObject::cancelOrders(const QString& symbol)
{
    if (!testMode)
        mainWindow.cancelPairOrders(symbol);

    if (!symbol.isEmpty())
        log("Cancel all " + symbol + " orders");
}

void ScriptObject::cancelAsks()
{
    log("Cancel all asks");

    if (!testMode)
        mainWindow.cancelAskOrders("");
}

void ScriptObject::cancelBids()
{
    log("Cancel all bids");

    if (!testMode)
        mainWindow.cancelBidOrders("");
}

void ScriptObject::cancelAsks(const QString& symbol)
{
    log("Cancel all " + symbol + " asks");

    if (!testMode)
        mainWindow.cancelAskOrders(symbol);
}

void ScriptObject::cancelBids(const QString& symbol)
{
    log("Cancel all " + symbol + " bids");

    if (!testMode)
        mainWindow.cancelBidOrders(symbol);
}

void ScriptObject::logClear()
{
    emit logClearSignal();
}

void ScriptObject::log(const QVariant& arg1, const QVariant& arg2)
{
    log(QVariantList() << arg1 << arg2);
}
void ScriptObject::log(const QVariant& arg1, const QVariant& arg2, const QVariant& arg3)
{
    log(QVariantList() << arg1 << arg2 << arg3);
}
void ScriptObject::log(const QVariant& arg1, const QVariant& arg2, const QVariant& arg3, const QVariant& arg4)
{
    log(QVariantList() << arg1 << arg2 << arg3 << arg4);
}
void ScriptObject::log(const QVariant& arg1, const QVariant& arg2, const QVariant& arg3, const QVariant& arg4, const QVariant& arg5)
{
    log(QVariantList() << arg1 << arg2 << arg3 << arg4 << arg5);
}
void ScriptObject::log(const QVariant& arg1, const QVariant& arg2, const QVariant& arg3, const QVariant& arg4, const QVariant& arg5,
                       const QVariant& arg6)
{
    log(QVariantList() << arg1 << arg2 << arg3 << arg4 << arg5 << arg6);
}
void ScriptObject::log(const QVariantList& list)
{
    if (testMode)
        return;

    QString logText;

    for (int n = 0; n < list.count(); n++)
    {
        if (n != 0)
            logText.append(QLatin1Char(' '));

        if (list.at(n).type() == QVariant::Double)
            logText.append(JulyMath::textFromDouble(list.at(n).toDouble(), 8, 0));
        else
            logText.append(list.at(n).toString());
    }

    log(logText);
}
void ScriptObject::log(const QVariant& arg1)
{
    if (testMode)
        return;

    if (arg1.type() == QVariant::Double)
    {
        QString result;
        bool ok;
        double doubleVal = arg1.toDouble(&ok);

        if (ok)
            result = JulyMath::textFromDouble(doubleVal, 8, 0);
        else
        {
            quint64 uLongVal = arg1.toULongLong(&ok);

            if (ok)
                result = QString::number(uLongVal);
            else
                result = arg1.toString();
        }

        emit writeLog(result);
    }
    else
        emit writeLog(arg1.toString());
}

void ScriptObject::initValueChangedPrivate(const QString& symbol, QString& scriptNameInd, double& val, bool forceEmit)
{
    QString prependName;

    if (scriptNameInd == QLatin1String("BalanceA"))
        scriptNameInd = QLatin1String("Balance") + baseValues.currentPair.currAStr;
    else if (scriptNameInd == QLatin1String("BalanceB"))
        scriptNameInd = QLatin1String("Balance") + baseValues.currentPair.currBStr;
    else if (!symbol.isEmpty())
        prependName = symbol + "_";

    if (val < 0.00000001 && scriptNameInd.endsWith(QLatin1String("price"), Qt::CaseInsensitive))
        return;

    if (!forceEmit && !testMode && qFuzzyCompare(indicatorsMap.value(prependName + scriptNameInd, -2508.1987) + 1.0, val + 1.0))
        return;

    indicatorsMap[prependName + scriptNameInd] = val;

    if (engine == nullptr)
        return;

    if (val < 0.00000001 && scriptNameInd.contains(QLatin1String("price"), Qt::CaseInsensitive))
        return;

    emit valueChanged(symbol, scriptNameInd, val);
}

void ScriptObject::initValueChanged(const QString& symbol, QString scriptNameInd, double val)
{
    initValueChangedPrivate(symbol, scriptNameInd, val, false);
}

void ScriptObject::indicatorValueChanged(double val)
{
    if (!isRunningFlag)
        return;

    QObject* senderObject = sender();

    if (senderObject == nullptr)
        return;

    QString scriptNameInd = senderObject->property("ScriptName").toString();

    if (scriptNameInd.isEmpty())
        return;

    initValueChanged(baseValues.currentPair.symbolSecond(), scriptNameInd, val);
}

quint32 ScriptObject::getTimeT()
{
    return TimeSync::getTimeT();
}

bool ScriptObject::groupIsRunning(const QString& name)
{
    if (name.isEmpty() || name == scriptName)
        return isRunning();

    return mainWindow.getIsGroupRunning(name);
}

void ScriptObject::secondSlot()
{
    emit valueChanged(baseValues.currentPair.symbolSecond(), QLatin1String("Time"), getTimeT());

    if (testMode)
        return;

    secondTimer->start(1000);
}

void ScriptObject::deleteEngine()
{
    qDeleteAll(timerMap.keys());
    timerMap.clear();

    if (!engine)
        return;

    if (!testMode)
        Q_FOREACH (QDoubleSpinBox* spinBox, spinBoxList)
            disconnect(spinBox, SIGNAL(valueChanged(double)), this, SLOT(indicatorValueChanged(double)));

    if (testMode)
    {
        QScriptValueIterator it(engine->globalObject());

        while (it.hasNext())
        {
            it.next();
            it.setValue(0);
        }
    }

    engine->deleteLater();
    engine = nullptr;

    if (!testMode && scriptWantsOrderBookData)
    {
        scriptWantsOrderBookData = false;
        baseValues.scriptsThatUseOrderBookCount--;
    }
}

void ScriptObject::setRunning(bool on)
{
    if (!testMode && !on)
    {
        secondTimer->stop();
    }

    if (on == isRunningFlag)
        return;

    isRunningFlag = on;

    if (!isRunningFlag)
        if (engine)
        {
            deleteEngine();
        }

    if (!testMode)
        emit runningChanged(isRunningFlag);
}

bool ScriptObject::stopScript()
{
    if (!isRunningFlag)
        return false;

    setRunning(false);
    return true;
}

bool ScriptObject::executeScript(QString script, bool _testMode)
{
    setRunning(false);

    if (script.isEmpty())
    {
        emit errorHappend(-1, "Script is empty");
        return false;
    }

    if (engine)
    {
        deleteEngine();
    }

    testMode = _testMode;
    engine = new QScriptEngine;

    QScriptValue scriptValue = engine->newQObject(this);
    engine->globalObject().setProperty("trader", scriptValue);

    bool anyValue = script.contains("trader.on(\"AnyValue\").changed()", Qt::CaseInsensitive) ||
                    script.contains("trader.on('AnyValue').changed()", Qt::CaseInsensitive);
    haveTimer = script.contains("trader.on(\"Time\").changed()", Qt::CaseInsensitive) ||
                script.contains("trader.on('Time').changed()", Qt::CaseInsensitive);

    if (!testMode)
    {
        if (scriptWantsOrderBookData && baseValues.scriptsThatUseOrderBookCount > 0)
            baseValues.scriptsThatUseOrderBookCount--;

        scriptWantsOrderBookData = script.contains("trader.get(\"Asks", Qt::CaseInsensitive) ||
                                   script.contains("trader.get('Asks", Qt::CaseInsensitive) || script.contains("trader.get(\"Bids", Qt::CaseInsensitive) ||
                                   script.contains("trader.get('Bids", Qt::CaseInsensitive);
    }
    else
        scriptWantsOrderBookData = false;

    script = sourceToScript(script);
    pendingStop = false;
    QScriptValue handler = engine->evaluate(script);

    if (haveTimer)
        secondSlot();

    if (!testMode)
        setRunning(true);

    if (pendingStop)
    {
        setRunning(false);
        return true;
    }

    if (engine->hasUncaughtException())
    {
        int lineNumber = engine->uncaughtExceptionLineNumber();
        QString errorText = engine->uncaughtException().toString() + " (Line:" + QString::number(lineNumber) + ")";

        emit errorHappend(lineNumber, errorText);

        if (!testMode)
            setRunning(false);

        return false;
    }
    else
    {
        if (!testMode && scriptWantsOrderBookData)
            baseValues.scriptsThatUseOrderBookCount++;

        bool scriptContainsBalance = script.contains("Balance", Qt::CaseInsensitive);

        Q_FOREACH (QDoubleSpinBox* spinBox, spinBoxList)
        {
            QString spinProperty = spinBox->property("ScriptName").toString();
            bool needConnect = anyValue;

            if (!needConnect && spinProperty.contains("BALANCE", Qt::CaseInsensitive) && scriptContainsBalance)
                needConnect = true;

            if (!needConnect)
                needConnect = script.contains(spinProperty, Qt::CaseInsensitive);

            if (!needConnect)
            {
                if (!testMode)
                    disconnect(spinBox, SIGNAL(valueChanged(double)), this, SLOT(indicatorValueChanged(double)));

                continue;
            }

            double spinValue = spinBox->value();
            QString symbolTemp = baseValues.currentPair.symbolSecond();
            initValueChangedPrivate(symbolTemp, spinProperty, spinValue, testMode);

            if (!testMode)
                connect(spinBox, SIGNAL(valueChanged(double)), this, SLOT(indicatorValueChanged(double)));
        }
    }

    if (testMode)
    {
        setRunning(false);
    }

    return true;
}

QString ScriptObject::sourceToScript(QString text)
{
    if (text.isEmpty())
        return text;

    text.replace("\" ,", "\",");
    text.replace("' ,", "',");
    text.replace("\\", "\\\\");
    text.replace(", \"", ",\"");
    text.replace(", '", ",'");

    text.replace("trader.get(\"Balance\",", "trader.get(\"Balance\"+", Qt::CaseInsensitive);
    text.replace("trader.on(\"Balance\",", "trader.on(\"Balance\"+", Qt::CaseInsensitive);

    text.replace("trader.get('Balance'',", "trader.get('Balance'+", Qt::CaseInsensitive);
    text.replace("trader.on('Balance'',", "trader.on('Balance'+", Qt::CaseInsensitive);

    text.replace(").changed()", ")", Qt::CaseInsensitive);

    while (replaceString("trader.on('",
                         "trader['valueChanged(QString,QString,double)'].connect(function(symbol,name,value){if(name=='", text, true));

    while (replaceString("trader.on(\"",
                         "trader['valueChanged(QString,QString,double)'].connect(function(symbol,name,value){if(name==\"", text, true));

    text.replace("trader.get(\"AsksPrice\",", "trader.getAsksPriceByVol(", Qt::CaseInsensitive);
    text.replace("trader.get(\"AsksVolume\",", "trader.getAsksVolByPrice(", Qt::CaseInsensitive);
    text.replace("trader.get(\"BidsPrice\",", "trader.getBidsPriceByVol(", Qt::CaseInsensitive);
    text.replace("trader.get(\"BidsVolume\",", "trader.getBidsVolByPrice(", Qt::CaseInsensitive);
    text.replace("trader.get('AsksPrice',", "trader.getAsksPriceByVol(", Qt::CaseInsensitive);
    text.replace("trader.get('AsksVolume',", "trader.getAsksVolByPrice(", Qt::CaseInsensitive);
    text.replace("trader.get('BidsPrice',", "trader.getBidsPriceByVol(", Qt::CaseInsensitive);
    text.replace("trader.get('BidsVolume',", "trader.getBidsVolByPrice(", Qt::CaseInsensitive);

    return text;
}

bool ScriptObject::replaceString(const QString& what, const QString& to, QString& text, bool skipFirstLeft)
{
    int indexOf_what = text.indexOf(what, 0, Qt::CaseInsensitive);

    if (indexOf_what == -1)
        return false;

    int leftPos = 0;
    leftPos = text.indexOf("{", indexOf_what);

    if (leftPos == -1)
        return false;

    leftPos++;

    if (leftPos >= text.length())
        return false;

    int rightPos = -1;

    int counter = 0;
    int curPos = leftPos;

    do
    {
        if (text.at(curPos) == QLatin1Char('{'))
            counter++;
        else if (text.at(curPos) == QLatin1Char('}'))
        {
            if (counter == 0)
            {
                rightPos = curPos;
                break;
            }

            counter--;
        }

        curPos++;
    }
    while (curPos < text.length());

    if (rightPos == -1)
        return false;

    if (skipFirstLeft)
        text.insert(rightPos + 1, "})");
    else
        text.insert(rightPos + 1, ")");

    text.remove(indexOf_what, what.length());
    text.insert(indexOf_what, to);
    return true;
}

void ScriptObject::fileWrite(const QVariant& path, const QVariant& data)
{
    if (testMode)
        return;

    QString result;

    if (data.type() == QVariant::Double)
    {
        bool ok;
        double doubleVal = data.toDouble(&ok);

        if (ok)
            result = JulyMath::textFromDouble(doubleVal, 8, 0);
        else
        {
            quint64 uLongVal = data.toULongLong(&ok);

            if (ok)
                result = QString::number(uLongVal);
            else
                result = data.toString();
        }
    }
    else
        result = data.toString();

    emit performFileWrite(path.toString(), result.toLatin1());
}

void ScriptObject::fileAppend(const QVariant& path, const QVariant& data)
{
    if (testMode)
        return;

    QString result;

    if (data.type() == QVariant::Double)
    {
        bool ok;
        double doubleVal = data.toDouble(&ok);

        if (ok)
            result = JulyMath::textFromDouble(doubleVal, 8, 0);
        else
        {
            quint64 uLongVal = data.toULongLong(&ok);

            if (ok)
                result = QString::number(uLongVal);
            else
                result = data.toString();
        }
    }
    else
        result = data.toString();

    emit performFileAppend(path.toString(), result.toLatin1());
}

QVariant ScriptObject::fileReadLine(const QVariant& path, qint64 seek)
{
    if (testMode)
        return QLatin1String("");

    fileOpenCount++;
    quint32 tempFileOperationNumber = fileOperationNumber++;
    emit performFileReadLine(path.toString(), seek, tempFileOperationNumber);

    QEventLoop waitLoop;
    connect(this, &ScriptObject::fileReadExitLoop, &waitLoop, &QEventLoop::quit);
    QTimer::singleShot(5000, &waitLoop, &QEventLoop::quit);
    waitLoop.exec();

    return arrayFileReadResult.take(tempFileOperationNumber);
}

QVariant ScriptObject::fileReadLineSimple(const QVariant& path)
{
    if (testMode)
        return "";

    fileOpenCount++;
    quint32 tempFileOperationNumber = fileOperationNumber++;
    emit performFileReadLineSimple(path.toString(), tempFileOperationNumber);

    QEventLoop waitLoop;
    connect(this, &ScriptObject::fileReadExitLoop, &waitLoop, &QEventLoop::quit);
    QTimer::singleShot(5000, &waitLoop, &QEventLoop::quit);
    waitLoop.exec();

    return arrayFileReadResult.take(tempFileOperationNumber);
}

QVariant ScriptObject::fileRead(const QVariant& path, qint64 size)
{
    if (testMode)
        return "";

    fileOpenCount++;
    quint32 tempFileOperationNumber = fileOperationNumber++;
    emit performFileRead(path.toString(), size, tempFileOperationNumber);

    QEventLoop waitLoop;
    connect(this, &ScriptObject::fileReadExitLoop, &waitLoop, &QEventLoop::quit);
    QTimer::singleShot(5000, &waitLoop, &QEventLoop::quit);
    waitLoop.exec();

    return arrayFileReadResult.take(tempFileOperationNumber);
}

QVariant ScriptObject::fileReadAll(const QVariant& path)
{
    if (testMode)
        return "";

    fileOpenCount++;
    quint32 tempFileOperationNumber = fileOperationNumber++;
    emit performFileReadAll(path.toString(), tempFileOperationNumber);

    QEventLoop waitLoop;
    connect(this, &ScriptObject::fileReadExitLoop, &waitLoop, &QEventLoop::quit);
    QTimer::singleShot(5000, &waitLoop, &QEventLoop::quit);
    waitLoop.exec();

    return arrayFileReadResult.take(tempFileOperationNumber);
}

QVariant ScriptObject::dateTimeString()
{
    return QDateTime::fromTime_t(TimeSync::getTimeT()).toString(baseValues.dateTimeFormat);
}

void ScriptObject::fileReadResult(const QByteArray& data, quint32 tempFileOperationNumber)
{
    fileOpenCount--;
    arrayFileReadResult.insert(tempFileOperationNumber, data);

    if (fileOpenCount < 1)
        emit fileReadExitLoop();
}
