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

#ifndef SCRIPTOBJECT_H
#define SCRIPTOBJECT_H

#include <QObject>

#ifdef QT_SCRIPT_LIB
#include <QScriptEngine>
#include <QScriptValue>
#include <QScriptValueIterator>

typedef QScriptEngine JSEngine;
typedef QScriptValue JSValue;
typedef QScriptValueIterator JSValueIterator;
#else
#include <QJSEngine>
#include <QJSValue>
#include <QJSValueIterator>

typedef QJSEngine JSEngine;
typedef QJSValue JSValue;
typedef QJSValueIterator JSValueIterator;
#endif

class QDoubleSpinBox;

#include "scriptobjectthread.h"
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QVariant>
#include <memory>

class ScriptObject : public QObject
{
    Q_OBJECT
public:
    int testResult = 00;
    QString scriptName;
    bool isRunning()
    {
        return isRunningFlag;
    }
    bool stopScript();
    bool executeScript(const QString&, bool);
    explicit ScriptObject(const QString& scriptName);
    ~ScriptObject();
    QStringList indicatorList;
    QStringList functionsList;
    QStringList argumentsList;
    QStringList commandsList;

private:
    void initValueChangedPrivate(const QString& symbol, QString& scriptNameInd, double val, bool forceEmit = false);
    void deleteEngine();
    bool scriptWantsOrderBookData = false;
    void timerCreate(int milliseconds, const QString& command, bool once);
    QMap<QTimer*, bool> timerMap;
    double orderBookInfo(const QString& symbol, double& value, bool isAsk, bool getPrice);
    bool haveTimer = false;
    QTimer* secondTimer = nullptr;
    bool pendingStop = false;
    void setRunning(bool);
    bool isRunningFlag = false;
    bool replaceString(const QString& what, const QString& to, QString& text, bool skipFirstLeft) const;
    QString sourceToScript(const QString&) const;
    std::unique_ptr<JSEngine> engine;
    QStringList functionNames;
    QList<QDoubleSpinBox*> spinBoxList;
    void addIndicator(QDoubleSpinBox* spinbox, QString value);
    void addCommand(const QString&, QList<QByteArray>);
    void addFunction(const QString& name);
    bool testMode = true;
    QMap<QString, double> indicatorsMap;
    ScriptObjectThread performThread;
    QHash<quint32, QString> arrayFileReadResult;
    quint32 fileOperationNumber;
    qint32 fileOpenCount;
    void log(const QVariantList&);
    void say(const QVariantList&);
public slots:
    void sendEvent(const QString& symbol, const QString& name, double value);
    void sendEvent(const QString& name, double value);
    void timer(double seconds, const QString& _command_);
    void delay(double seconds, const QString& _command_);
    void log(const QVariant&);
    void log(const QVariant&, const QVariant&);
    void log(const QVariant&, const QVariant&, const QVariant&);
    void log(const QVariant&, const QVariant&, const QVariant&, const QVariant&);
    void log(const QVariant&, const QVariant&, const QVariant&, const QVariant&, const QVariant&);
    void log(const QVariant&, const QVariant&, const QVariant&, const QVariant&, const QVariant&, const QVariant&);
    void logClear();
    void test(int);

    void beep() const;
    void playWav(const QString& _filePath_);
    void play(const QString& _filePath_);

    void say(const QString& _text_);
    void say(int);
    void say(double);
    void say(const QVariant&);
    void say(const QVariant&, const QVariant&);
    void say(const QVariant&, const QVariant&, const QVariant&);
    void say(const QVariant&, const QVariant&, const QVariant&, const QVariant&);
    void say(const QVariant&, const QVariant&, const QVariant&, const QVariant&, const QVariant&);
    void say(const QVariant&, const QVariant&, const QVariant&, const QVariant&, const QVariant&, const QVariant&);

    void groupDone();
    void groupStart(const QString& _name_);
    void groupStop(const QString& _name_);
    void groupStop();
    bool groupIsRunning(const QString& _name_);

    void startApp(const QString& _filePath_);
    void startApp(const QString&, const QStringList&);
    void startApp(const QString&, const QString&);
    void startApp(const QString&, const QString&, const QString&);
    void startApp(const QString&, const QString&, const QString&, const QString&);
    void startApp(const QString&, const QString&, const QString&, const QString&, const QString&);

    void sell(double amount, double price);
    void sell(const QString& symbol, double amount, double price);
    void buy(double amount, double price);
    void buy(const QString& symbol, double amount, double price);
    void cancelOrders(const QString& symbolR);
    void cancelOrders();
    void cancelAsks(const QString& symbolR);
    void cancelBids(const QString& symbolR);
    void cancelAsks();
    void cancelBids();

    int getOpenAsksCount();
    int getOpenBidsCount();
    int getOpenOrdersCount();

    double getAsksVolByPrice(double price);
    double getAsksPriceByVol(double volume);
    double getAsksVolByPrice(const QString& symbol, double price);
    double getAsksPriceByVol(const QString& symbol, double volume);

    double getBidsVolByPrice(double price);
    double getBidsPriceByVol(double volume);
    double getBidsVolByPrice(const QString& symbol, double price);
    double getBidsPriceByVol(const QString& symbol, double volume);

    quint32 getTimeT();
    double get(const QString& indicator);
    double get(const QString& symbol, const QString& indicator);

    void fileWrite(const QVariant& path, const QVariant& data);
    void fileAppend(const QVariant& path, const QVariant& data);
    QVariant fileReadLine(const QVariant& path, qint64 seek = -1);
    QVariant fileReadLineSimple(const QVariant& path);
    QVariant fileRead(const QVariant& path, qint64 size);
    QVariant fileReadAll(const QVariant& path);
    QVariant dateTimeString();
private slots:
    void initValueChanged(const QString& symbol, QString name, double val);
    void timerOut();
    void secondSlot();
    void indicatorValueChanged(double);
    void fileReadResult(const QByteArray& data, quint32);
signals:
    void eventSignal(const QString& symbol, const QString& name, double value);
    void startAppSignal(QString, QStringList);
    void setGroupEnabled(const QString& name, bool enabled);
    void setGroupDone(QString);
    void runningChanged(bool);
    void errorHappend(int, QString);
    void logClearSignal();
    void writeLog(QString);
    void valueChanged(QString, QString, double);

    void performFileWrite(QString, QByteArray);
    void performFileAppend(QString, QByteArray);
    void performFileReadLine(QString, qint64, quint32);
    void performFileReadLineSimple(QString, quint32);
    void performFileRead(QString, qint64, quint32);
    void performFileReadAll(QString, quint32);
    void fileReadExitLoop();
};

#endif // SCRIPTOBJECT_H
