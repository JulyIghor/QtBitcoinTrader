//  This file is part of Qt Bitcion Trader
//      https://github.com/JulyIGHOR/QtBitcoinTrader
//  Copyright (C) 2013-2014 July IGHOR <julyighor@gmail.com>
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
#include "exchange.h"
#include "main.h"
#include "time.h"
#include <QMetaMethod>
#include <QDoubleSpinBox>
#include <QScriptEngine>

ScriptObject::ScriptObject(QString _scriptName) :
    QObject()
{
    haveTimer=false;
    secondTimer=0;
    pendingStop=false;
    testResult=0;
    scriptName=_scriptName;
    isRunningFlag=false;
    engine=0;
    testMode=true;

    for(int n=staticMetaObject.methodOffset();n<staticMetaObject.methodCount();n++)
    {
        if(staticMetaObject.method(n).methodType()!=QMetaMethod::Slot||
           staticMetaObject.method(n).access()!=QMetaMethod::Public)continue;
       QString currentCommand;
#if QT_VERSION<0x050000
       currentCommand=QString::fromLocal8Bit(staticMetaObject.method(n).signature());
       currentCommand=currentCommand.split("(").first();
#else
       currentCommand=QString::fromLocal8Bit(staticMetaObject.method(n).name());
#endif
       if(currentCommand.startsWith(QLatin1String("get"))||commandsList.contains(currentCommand))continue;
       if(currentCommand==QLatin1String("groupDone"))continue;

       QList<QByteArray> parameters=staticMetaObject.method(n).parameterNames();
       if(!baseValues.currentExchange_->multiCurrencyTradeSupport&&parameters.contains("symbol"))continue;
       if(commandsList.contains("trader."+currentCommand))continue;
       addCommand(currentCommand,parameters);
    }

    Q_FOREACH(QDoubleSpinBox* spinBox, mainWindow.indicatorsMap.values())
    {
        QString scriptName=spinBox->whatsThis();
        if(scriptName.isEmpty())continue;
        connect(spinBox,SIGNAL(valueChanged(double)),this,SLOT(indicatorValueChanged(double)));
        indicatorsMap[scriptName]=spinBox->value();
        addIndicator(spinBox,scriptName);
    }
    indicatorList<<"trader.on(\"AnyValue\").changed";
    indicatorList<<"trader.on(\"Time\").changed";

    functionsList<<"trader.get(\"Time\")";

    indicatorList.removeDuplicates();
    functionsList.removeDuplicates();

    connect(this,SIGNAL(setGroupEnabled(QString, bool)),baseValues.mainWindow_,SLOT(setGroupRunning(QString,bool)));
    connect(this,SIGNAL(startAppSignal(QString,QStringList)),baseValues.mainWindow_,SLOT(startApplication(QString,QStringList)));

    secondTimer=new QTimer(this);
    secondTimer->setSingleShot(true);
    connect(secondTimer,SIGNAL(timeout()),this,SLOT(secondSlot()));
}

void ScriptObject::test(int val)
{
    testResult=val;
}

qreal ScriptObject::get(QString indicator)
{
    if(indicator==QLatin1String("Time"))return getTimeT();
    if(indicator.length()>8&&indicator.startsWith(QLatin1String("Balance")))
    {
        indicator.remove(0,7);
        if(baseValues.currentPair.currAStr==indicator.toUpper())indicator=QLatin1String("BalanceA");
        else
        if(baseValues.currentPair.currBStr==indicator.toUpper())indicator=QLatin1String("BalanceB");
    }
    return indicatorsMap.value(indicator);
}

qreal ScriptObject::get(QString symbol, QString indicator)
{
    Q_UNUSED(symbol);
    return get(indicator);
}

void ScriptObject::startApp(QString name){startApp(name,QStringList());}
void ScriptObject::startApp(QString name, QString arg1){startApp(name,QStringList()<<arg1);}
void ScriptObject::startApp(QString name, QString arg1, QString arg2){startApp(name,QStringList()<<arg1<<arg2);}
void ScriptObject::startApp(QString name, QString arg1, QString arg2, QString arg3){startApp(name,QStringList()<<arg1<<arg2<<arg3);}
void ScriptObject::startApp(QString name, QString arg1, QString arg2, QString arg3, QString arg4){startApp(name,QStringList()<<arg1<<arg2<<arg3<<arg4);}
void ScriptObject::startApp(QString name, QStringList list)
{
    emit startAppSignal(name,list);
    if(list.count())name+=" "+list.join(" ");
    writeLog(julyTr("START_APPLICATION","Start application: %1").arg(name));
}

void ScriptObject::beep()
{
    if(testMode)return;
    mainWindow.beep(false);
}

void ScriptObject::playWav(QString fileName)
{
    if(testMode)return;
    if(fileName.isEmpty()){beep();return;}
    mainWindow.playWav(fileName,false);
}

//void ScriptObject::say(QString text){mainWindow.sayText(text);}
//void ScriptObject::say(QVariant text){say(text.toString());}
//void ScriptObject::say(QVariant text1, QVariant text2){say(text1.toString()+" "+text2.toString());}
//void ScriptObject::say(QVariant text1, QVariant text2, QVariant text3){say(text1.toString()+" "+text2.toString()+" "+text3.toString());}
//void ScriptObject::say(QVariant text1, QVariant text2, QVariant text3, QVariant text4){say(text1.toString()+" "+text2.toString()+" "+text3.toString()+" "+text4.toString());}

void ScriptObject::groupStart(QString name)
{
    if(testMode)return;
    if(name.isEmpty()||name==scriptName)setRunning(true);
    else
    emit setGroupEnabled(name, true);
}

void ScriptObject::groupStop()
{
    if(testMode)return;
    groupStop("");
}

void ScriptObject::groupDone()
{
    if(testMode)return;
    pendingStop=true;
    emit setGroupDone(scriptName);
    setRunning(false);
}

void ScriptObject::groupStop(QString name)
{
    if(testMode)return;
    if(name.isEmpty()||name==scriptName)
    {
        pendingStop=true;
        setRunning(false);
    }
    else
    emit setGroupEnabled(name, false);
}

void ScriptObject::addIndicator(QDoubleSpinBox *spinBox, QString value)
{
    if(indicatorList.contains(value))return;
    indicatorList<<"trader.on(\""+value+"\").changed";
    spinBox->setProperty("ScriptName",value);
    spinBoxList<<spinBox;
    indicatorsMap.insert(value,spinBox->value());
    functionsList<<"trader.get(\""+value+"\")";
}

void ScriptObject::addCommand(QString name, QList<QByteArray> parameters)
{
    if(!commandsList.contains(name))
    {
        commandsList<<"trader."+name;
        QString params;
        for(int n=0;n<parameters.count();n++)
        {
            if(parameters.at(n).isEmpty())continue;
            if(params.length())params.append(",");
            params.append(parameters.at(n));
        }
        argumentsList<<params;
    }
}

void ScriptObject::addFunction(QString name)
{
   if(!functionsList.contains(name))functionsList<<name.replace('_','.');
}

void ScriptObject::buy(double amount, double price)
{
    buy("",amount,price);
}

void ScriptObject::sell(double amount, double price)
{
    sell("",amount,price);
}

void ScriptObject::buy(QString symbol, double amount, double price)
{
    symbol=symbol.toUpper();
    if(!testMode)mainWindow.apiBuySend(symbol,amount,price);
    log(symbol+": Buy "+mainWindow.numFromDouble(amount)+" at "+mainWindow.numFromDouble(price));
}

void ScriptObject::sell(QString symbol, double amount, double price)
{
    symbol=symbol.toUpper();
    if(!testMode)mainWindow.apiSellSend(symbol,amount,price);
    log(symbol+": Sell "+mainWindow.numFromDouble(amount)+" at "+mainWindow.numFromDouble(price));
}

void ScriptObject::cancelOrders()
{
    if(!testMode)mainWindow.cancelPairOrders("");
    log("Cancel all orders");
}

void ScriptObject::cancelOrders(QString symbol)
{
    if(!testMode)mainWindow.cancelPairOrders(symbol);
    if(!symbol.isEmpty())log("Cancel all "+symbol+" orders");
}

void ScriptObject::cancelAsks()
{
    log("Cancel all asks");
    if(!testMode)mainWindow.cancelAskOrders("");
}

void ScriptObject::cancelBids()
{
    log("Cancel all bids");
    if(!testMode)mainWindow.cancelBidOrders("");
}

void ScriptObject::cancelAsks(QString symbol)
{
    log("Cancel all "+symbol+" asks");
    if(!testMode)mainWindow.cancelAskOrders(symbol);
}

void ScriptObject::cancelBids(QString symbol)
{
    log("Cancel all "+symbol+" bids");
    if(!testMode)mainWindow.cancelBidOrders(symbol);
}

void ScriptObject::logClear()
{
    emit logClearSignal();
}

void ScriptObject::log(QVariant arg1, QVariant arg2){log(QVariantList()<<arg1<<arg2);}
void ScriptObject::log(QVariant arg1, QVariant arg2, QVariant arg3){log(QVariantList()<<arg1<<arg2<<arg3);}
void ScriptObject::log(QVariant arg1, QVariant arg2, QVariant arg3, QVariant arg4){log(QVariantList()<<arg1<<arg2<<arg3<<arg4);}
void ScriptObject::log(QVariantList list)
{
    if(testMode)return;
    QString logText;
    for(int n=0;n<list.count();n++)
    {
        if(n!=0)logText.append(QLatin1Char(' '));
        if(list.at(n).type()==QVariant::Double)
            logText.append(mainWindow.numFromDouble(list.at(n).toDouble()));
        else
            logText.append(list.at(n).toString());
    }
    log(logText);
}
void ScriptObject::log(QVariant arg1)
{
    if(testMode)return;
    if(arg1.type()==QVariant::Double)emit writeLog(mainWindow.numFromDouble(arg1.toDouble()));
    else emit writeLog(arg1.toString());
}

void ScriptObject::indicatorValueChanged(double val)
{
    QObject *senderObject=sender();if(senderObject==0)return;
    QString scriptNameInd=senderObject->property("ScriptName").toString();
    indicatorsMap[scriptNameInd]=val;
    if(engine==0||isRunningFlag==false)return;
    if(scriptNameInd==QLatin1String("BalanceA"))scriptNameInd=QLatin1String("Balance")+baseValues.currentPair.currAStr;
    else
    if(scriptNameInd==QLatin1String("BalanceB"))scriptNameInd=QLatin1String("Balance")+baseValues.currentPair.currBStr;
    emit valueChanged(baseValues.currentPair.symbol,scriptNameInd,val);
}

quint32 ScriptObject::getTimeT()
{
    return time(NULL);
}

bool ScriptObject::groupIsRunning(QString name)
{
    if(name.isEmpty()||name==scriptName)return isRunning();
    return mainWindow.getIsGroupRunning(name);
}

void ScriptObject::secondSlot()
{
    emit valueChanged(baseValues.currentPair.symbol,QLatin1String("Time"),getTimeT());
    if(testMode)return;
    secondTimer->start(1000);
}

void ScriptObject::setRunning(bool on)
{
    if(!testMode&&!on){secondTimer->stop();}
    if(on==isRunningFlag)return;
    isRunningFlag=on;
    if(!isRunningFlag)if(engine){engine->deleteLater(); engine=0;}
    if(!testMode)emit runningChanged(isRunningFlag);
}

bool ScriptObject::stopScript()
{
    if(!isRunningFlag)return false;
    setRunning(false);
    return true;
}

bool ScriptObject::executeScript(QString script, bool _testMode)
{
    setRunning(false);
    if(script.isEmpty()){emit errorHappend(-1, "Script is empty");return false;}

    testMode=_testMode;
    if(engine){engine->deleteLater();}
    engine=new QScriptEngine;

    QScriptValue scriptValue=engine->newQObject(this);
    engine->globalObject().setProperty("trader", scriptValue);

    bool anyValue=script.contains("trader.on(\"AnyValue\").changed()",Qt::CaseInsensitive);
    haveTimer=script.contains("trader.on(\"Time\").changed()",Qt::CaseInsensitive);
    script=sourceToScript(script);
    pendingStop=false;
       QScriptValue handler=engine->evaluate(script);

       if(haveTimer)secondSlot();

       if(pendingStop)
       {
           setRunning(false);
           return true;
       }

       if(!testMode)setRunning(true);

        if(engine->hasUncaughtException())
        {
            int lineNumber=engine->uncaughtExceptionLineNumber();
            QString errorText=engine->uncaughtException().toString()+" (Line:"+QString::number(lineNumber)+")";

            emit errorHappend(lineNumber, errorText);

            if(!testMode)setRunning(false);
            return false;
        }
        else
        {
            if(testMode)
            Q_FOREACH(QDoubleSpinBox *spin, spinBoxList)
            {
            if(!anyValue&&!script.contains("name==\""+spin->property("ScriptName").toString()+"\"",Qt::CaseInsensitive))continue;
            emit valueChanged(baseValues.currentPair.symbol,spin->property("ScriptName").toString(),spin->value());
            }
        }
   if(testMode)
   {
       setRunning(false);
   }
   return true;
}

QString ScriptObject::sourceToScript(QString text)
{
    if(text.isEmpty())return text;

    text.replace("trader.get(\"Balance\",\"","trader.get(\"Balance",Qt::CaseInsensitive);
    text.replace("trader.on(\"Balance\",\"","trader.on(\"Balance",Qt::CaseInsensitive);
    text.replace(").changed()",")",Qt::CaseInsensitive);
   // while(replaceString("trader.on(","console['valueChanged(QString,QString,double)'].connect(function(symbol,name,value",text,false));

   // Q_FOREACH(QString name,indicatorList)
     //   text.replace("on."+name+"Changed()","on.valueChanged('"+name+"')",Qt::CaseInsensitive);

    while(replaceString("trader.on('","trader['valueChanged(QString,QString,double)'].connect(function(symbol,name,value){if(name=='",text,true));
    while(replaceString("trader.on(\"","trader['valueChanged(QString,QString,double)'].connect(function(symbol,name,value){if(name==\"",text,true));

    return text;
}

bool ScriptObject::replaceString(QString what, QString to, QString &text, bool skipFirstLeft)
{
    int indexOf_what=text.indexOf(what,0,Qt::CaseInsensitive);
    if(indexOf_what==-1)return false;

    int leftPos=0;
    leftPos=text.indexOf("{",indexOf_what);
    if(leftPos==-1)return false;
    leftPos++;
    if(leftPos>=text.length())return false;

    int rightPos=-1;

    int counter=0;
    int curPos=leftPos;

    do
    {
        if(text.at(curPos)==QLatin1Char('{'))counter++;
        else
        if(text.at(curPos)==QLatin1Char('}'))
        {
            if(counter==0)
            {
                rightPos=curPos;
                break;
            }
            counter--;
        }
        curPos++;
    }
    while(curPos<text.length());

    if(rightPos==-1)return false;

    if(skipFirstLeft)text.insert(rightPos+1,"})");
    else text.insert(rightPos+1,")");
    text.remove(indexOf_what,what.length());
    text.insert(indexOf_what,to);
    return true;
}
