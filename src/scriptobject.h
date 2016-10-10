//  This file is part of Qt Bitcion Trader
//      https://github.com/JulyIGHOR/QtBitcoinTrader
//  Copyright (C) 2013-2016 July IGHOR <julyighor@gmail.com>
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
class QScriptContext;
class QScriptEngine;
class QScriptValue;
class QDoubleSpinBox;
#include <QVariant>
#include <QString>
class QScriptEngine;
#include <QStringList>
#include <QTimer>

class ScriptObject : public QObject
{
    Q_OBJECT
public:
    int testResult;
    QString scriptName;
    bool isRunning() {return isRunningFlag;}
    bool stopScript();
    bool executeScript(QString, bool);
    explicit ScriptObject(QString scriptName);
    QStringList indicatorList;
    QStringList functionsList;
    QStringList argumentsList;
    QStringList commandsList;
private:
    void initValueChangedPrivate(QString &symbol, QString &scriptNameInd, double &val, bool forceEmit=false);
	void deleteEngine();
    bool scriptWantsOrderBookData;
    void timerCreate(int milliseconds, QString &command, bool once);
    QMap<QTimer *,bool> timerMap;
    double orderBookInfo(QString &symbol,double &value, bool isAsk, bool getPrice);
    bool haveTimer;
    QTimer *secondTimer;
    bool pendingStop;
    void setRunning(bool);
    bool isRunningFlag;
    bool replaceString(QString what, QString to, QString &text, bool skipFirstLeft);
    QString sourceToScript(QString);
    QScriptEngine *engine;
    QStringList functionNames;
    QList<QDoubleSpinBox*> spinBoxList;
    void addIndicator(QDoubleSpinBox *spinbox, QString value);
    void addCommand(QString, QList<QByteArray>);
    void addFunction(QString name);
    bool testMode;
    QMap<QString,double> indicatorsMap;
public slots:
    void sendEvent(QString symbol, QString name, double value);
    void sendEvent(QString name, double value);
    void timer(double seconds, QString _command_);
    void delay(double seconds, QString _command_);
    void log(QVariant);
    void log(QVariant,QVariant);
    void log(QVariant,QVariant,QVariant);
    void log(QVariant,QVariant,QVariant,QVariant);
    void log(QVariant,QVariant,QVariant,QVariant,QVariant);
    void log(QVariant,QVariant,QVariant,QVariant,QVariant,QVariant);
    void log(QVariantList);
    void logClear();
    void test(int);

    void beep();
    void playWav(QString _filePath_);

    void say(QString _text_);
    void say(int);
    void say(double);
    void say(QVariant);
    void say(QVariantList);
    void say(QVariant,QVariant);
    void say(QVariant,QVariant,QVariant);
    void say(QVariant,QVariant,QVariant,QVariant);
    void say(QVariant,QVariant,QVariant,QVariant,QVariant);
    void say(QVariant,QVariant,QVariant,QVariant,QVariant,QVariant);

    void groupDone();
    void groupStart(QString _name_);
    void groupStop(QString _name_);
    void groupStop();
    bool groupIsRunning(QString _name_);

    void startApp(QString _filePath_);
    void startApp(QString,QStringList);
    void startApp(QString,QString);
    void startApp(QString,QString,QString);
    void startApp(QString,QString,QString,QString);
    void startApp(QString,QString,QString,QString,QString);

    void sell(double amount, double price);
    void sell(QString symbol, double amount, double price);
    void buy(double amount, double price);
    void buy(QString symbol, double amount, double price);
    void cancelOrders(QString symbol);
    void cancelOrders();
    void cancelAsks(QString symbol);
    void cancelBids(QString symbol);
    void cancelAsks();
    void cancelBids();

    int getOpenAsksCount();
    int getOpenBidsCount();
    int getOpenOrdersCount();

    double getAsksVolByPrice(double price);
    double getAsksPriceByVol(double volume);
    double getAsksVolByPrice(QString symbol, double price);
    double getAsksPriceByVol(QString symbol, double volume);

    double getBidsVolByPrice(double price);
    double getBidsPriceByVol(double volume);
    double getBidsVolByPrice(QString symbol, double price);
    double getBidsPriceByVol(QString symbol, double volume);

    quint32 getTimeT();
    double get(QString indicator);
    double get(QString symbol, QString indicator);
private slots:
    void initValueChanged(QString symbol, QString name, double val);
    void timerOut();
    void secondSlot();
    void indicatorValueChanged(double);
signals:
    void eventSignal(QString symbol, QString name, double value);
    void startAppSignal(QString,QStringList);
    void setGroupEnabled(QString name, bool enabled);
    void setGroupDone(QString);
    void runningChanged(bool);
    void errorHappend(int, QString);
    void logClearSignal();
    void writeLog(QString);
    void valueChanged(QString,QString,double);
};

#endif // SCRIPTOBJECT_H
