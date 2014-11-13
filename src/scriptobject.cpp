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
    scriptWantsOrderBookData=false;
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
       if(currentCommand.startsWith(QLatin1String("get"))||currentCommand.startsWith(QLatin1String("test"))||commandsList.contains(currentCommand))continue;
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
        indicatorsMap[baseValues.currentPair.symbolSecond()+"_"+scriptName]=spinBox->value();
        addIndicator(spinBox,scriptName);
    }
    indicatorList<<"trader.on(\"AnyValue\").changed";
	indicatorList<<"trader.on(\"Time\").changed";
    indicatorList<<"trader.on(\"LastTrade\").changed";
    indicatorList<<"trader.on(\"MyLastTrade\").changed";
    indicatorList<<"trader.on(\"OpenOrdersCount\").changed";
    indicatorList<<"trader.on(\"OpenAsksCount\").changed";
    indicatorList<<"trader.on(\"OpenBidsCount\").changed";

    functionsList<<"trader.get(\"Time\")";

    functionsList<<"trader.get(\"AsksPrice\",volume)";
    functionsList<<"trader.get(\"AsksVolume\",price)";
    functionsList<<"trader.get(\"BidsPrice\",volume)";
    functionsList<<"trader.get(\"BidsVolume\",price)";

    functionsList<<"trader.get(\"OpenOrdersCount\")";
    functionsList<<"trader.get(\"OpenAsksCount\")";
    functionsList<<"trader.get(\"OpenBidsCount\")";

    indicatorList.removeDuplicates();
    functionsList.removeDuplicates();

    connect(this,SIGNAL(setGroupEnabled(QString, bool)),baseValues.mainWindow_,SLOT(setGroupRunning(QString,bool)));
    connect(this,SIGNAL(startAppSignal(QString,QStringList)),baseValues.mainWindow_,SLOT(startApplication(QString,QStringList)));

    connect(baseValues.mainWindow_,SIGNAL(indicatorEventSignal(QString,QString,double)),this,SLOT(initValueChanged(QString,QString,double)));
    connect(this,SIGNAL(eventSignal(QString,QString,qreal)),baseValues.mainWindow_,SLOT(sendIndicatorEvent(QString,QString,qreal)));

    secondTimer=new QTimer(this);
    secondTimer->setSingleShot(true);
    connect(secondTimer,SIGNAL(timeout()),this,SLOT(secondSlot()));
}

void ScriptObject::sendEvent(QString name, qreal value)
{
    if(testMode)return;
    emit eventSignal(baseValues.currentPair.symbolSecond(),name,value);
}

void ScriptObject::sendEvent(QString symbol, QString name, qreal value)
{
    if(testMode)return;
    emit eventSignal(symbol,name,value);
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
    testResult=val;
}

qreal ScriptObject::getAsksVolByPrice(qreal volume){return getAsksVolByPrice(baseValues.currentPair.symbolSecond(),volume);}
qreal ScriptObject::getAsksPriceByVol(qreal price){return getAsksPriceByVol(baseValues.currentPair.symbolSecond(),price);}
qreal ScriptObject::getBidsVolByPrice(qreal volume){return getBidsVolByPrice(baseValues.currentPair.symbolSecond(),volume);}
qreal ScriptObject::getBidsPriceByVol(qreal price){return getBidsPriceByVol(baseValues.currentPair.symbolSecond(),price);}

qreal ScriptObject::getAsksPriceByVol(QString symbol, qreal price){return orderBookInfo(symbol,price,true,true);}
qreal ScriptObject::getAsksVolByPrice(QString symbol, qreal volume){return orderBookInfo(symbol,volume,true,false);}
qreal ScriptObject::getBidsPriceByVol(QString symbol, qreal price){return orderBookInfo(symbol,price,false,true);}
qreal ScriptObject::getBidsVolByPrice(QString symbol, qreal volume){return orderBookInfo(symbol,volume,false,false);}


qreal ScriptObject::orderBookInfo(QString &symbol,qreal &value, bool isAsk, bool getPrice)
{
    qreal result=0.0;
    if(getPrice)result=mainWindow.getPriceByVolume(symbol,value,isAsk);
    else result=mainWindow.getVolumeByPrice(symbol,value,isAsk);
    if(result<0.0)
    {
        result=-result;
        if(!testMode)
            emit writeLog("Warning! OrderBook info is out of range. OrderBook information is limited to rows count limit.");
    }
    return result;
}

qreal ScriptObject::get(QString indicator)
{
    return get(baseValues.currentPair.symbolSecond(),indicator);
}

void ScriptObject::timerCreate(int milliseconds, QString &command, bool once)
{
    if(testMode)return;
    if(engine==0||command.isEmpty())return;
    if(milliseconds<0){emit writeLog("Timer have less than 0 interval");return;}
    QTimer *newTimer=new QTimer(this);
    timerMap.insert(newTimer,true);
    newTimer->setSingleShot(once);
    newTimer->setProperty("Command",command);
    newTimer->setProperty("DeleteMe",once);
    connect(newTimer,SIGNAL(timeout()),this,SLOT(timerOut()));
    newTimer->start(milliseconds);
}

void ScriptObject::delay(qreal seconds, QString command)
{
    timerCreate(seconds*1000,command,true);
}

void ScriptObject::timer(qreal seconds, QString command)
{
    timerCreate(seconds*1000,command,false);
}

void ScriptObject::timerOut()
{
    QTimer *senderTimer=qobject_cast<QTimer*>(sender());
    if(senderTimer==0)return;
    QString command=senderTimer->property("Command").toString();
    if(senderTimer->property("DeleteMe").toBool())
    {
        timerMap.remove(senderTimer);
        senderTimer->deleteLater();
    }
    if(command.isEmpty())return;
    if(engine==0)return;
    engine->evaluate(command.replace("\\","\\\\"));
}

qreal ScriptObject::get(QString symbol, QString indicator)
{
    QString indicatorLower=indicator.toLower();
    if(indicatorLower==QLatin1String("time"))return getTimeT();
    if(indicatorLower==QLatin1String("openorderscount"))return getOpenOrdersCount();
    if(indicatorLower==QLatin1String("openaskscount"))return getOpenAsksCount();
    if(indicatorLower==QLatin1String("openbidscount"))return getOpenBidsCount();

    if(indicator.length()>=8&&indicatorLower.startsWith(QLatin1String("balance")))symbol.clear();
    else if(!symbol.isEmpty())symbol.append("_");
    return indicatorsMap.value(symbol+indicator,0.0);
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

void ScriptObject::say(QString text){if(!testMode)mainWindow.sayText(text);}
void ScriptObject::say(double text){say(QVariantList()<<text);}
void ScriptObject::say(int text){say(QVariantList()<<text);}
void ScriptObject::say(QVariant text1){say(QVariantList()<<text1);}
void ScriptObject::say(QVariant text1, QVariant text2){say(QVariantList()<<text1<<text2);}
void ScriptObject::say(QVariant text1, QVariant text2, QVariant text3){say(QVariantList()<<text1<<text2<<text3);}
void ScriptObject::say(QVariant text1, QVariant text2, QVariant text3, QVariant text4){say(QVariantList()<<text1<<text2<<text3<<text4);}
void ScriptObject::say(QVariant text1, QVariant text2, QVariant text3, QVariant text4, QVariant text5){say(QVariantList()<<text1<<text2<<text3<<text4<<text5);}
void ScriptObject::say(QVariant text1, QVariant text2, QVariant text3, QVariant text4, QVariant text5, QVariant text6){say(QVariantList()<<text1<<text2<<text3<<text4<<text5<<text6);}
void ScriptObject::say(QVariantList list)
{
    int detectDoublePoint=0;
    if(detectDoublePoint==0)//If you reading this and know better solution, please tell me
    {
        bool decimalComma=QLocale().decimalPoint()==QChar(',');
        if(!decimalComma)decimalComma=QLocale().country()==QLocale::Ukraine||QLocale().country()==QLocale::RussianFederation;
        if(decimalComma)detectDoublePoint=2;
        else detectDoublePoint=1;
    }

    QString text;
    for(int n=0;n<list.count();n++)
    {
        if(list.at(n).type()==QVariant::Double)
        {
            QString number=textFromDouble(list.at(n).toDouble(),8,0);
            if(detectDoublePoint==2)number.replace(QLatin1Char('.'),QLatin1Char(','));
            text+=number;
        }
        else
        text+=list.at(n).toString();
        if(n<list.count()-1)text+=QLatin1Char(' ');
    }
    say(text);
}

void ScriptObject::groupStart(QString name)
{
    if(testMode)return;
    if(name.isEmpty())name=scriptName;
    log("Start group: \""+name+"\"");
    emit setGroupEnabled(name, true);
}

void ScriptObject::groupStop()
{
    if(testMode)return;
    groupStop(QLatin1String(""));
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
    if(name.isEmpty())name=scriptName;
    log("Stop group: \""+name+"\"");
    if(name==scriptName)
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
    indicatorsMap.insert(baseValues.currentPair.symbolSecond()+"_"+value,spinBox->value());
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
            parameters[n].replace("_","\"");
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
    if(symbol.isEmpty())symbol=baseValues.currentPair.symbolSecond();
    else
    symbol=symbol.toUpper();
    if(!testMode)mainWindow.apiBuySend(symbol,amount,price);
    log(symbol+": Buy "+textFromDouble(amount,8,0)+" at "+textFromDouble(price,8,0));
}

void ScriptObject::sell(QString symbol, double amount, double price)
{
    if(symbol.isEmpty())symbol=baseValues.currentPair.symbolSecond();
    else
    symbol=symbol.toUpper();
    if(!testMode)mainWindow.apiSellSend(symbol,amount,price);
    log(symbol+": Sell "+textFromDouble(amount,8,0)+" at "+textFromDouble(price,8,0));
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
void ScriptObject::log(QVariant arg1, QVariant arg2, QVariant arg3, QVariant arg4, QVariant arg5){log(QVariantList()<<arg1<<arg2<<arg3<<arg4<<arg5);}
void ScriptObject::log(QVariant arg1, QVariant arg2, QVariant arg3, QVariant arg4, QVariant arg5, QVariant arg6){log(QVariantList()<<arg1<<arg2<<arg3<<arg4<<arg5<<arg6);}
void ScriptObject::log(QVariantList list)
{
    if(testMode)return;
    QString logText;
    for(int n=0;n<list.count();n++)
    {
        if(n!=0)logText.append(QLatin1Char(' '));
        if(list.at(n).type()==QVariant::Double)
            logText.append(textFromDouble(list.at(n).toDouble(),8,0));
        else
            logText.append(list.at(n).toString());
    }
    log(logText);
}
void ScriptObject::log(QVariant arg1)
{
    if(testMode)return;
    if(arg1.type()==QVariant::Double)
    {
        QString result;
        bool ok;
        qreal doubleVal=arg1.toDouble(&ok);
        if(ok)result=textFromDouble(doubleVal,8,0);
        else
        {
            quint64 uLongVal=arg1.toULongLong(&ok);
            if(ok)result=QString::number(uLongVal);
            else result=arg1.toString();
        }
        emit writeLog(result);
    }
    else emit writeLog(arg1.toString());
}

void ScriptObject::initValueChangedPrivate(QString &symbol, QString &scriptNameInd, double &val, bool forceEmit)
{
    QString prependName;
    if(scriptNameInd==QLatin1String("BalanceA"))scriptNameInd=QLatin1String("Balance")+baseValues.currentPair.currAStr;
    else
    if(scriptNameInd==QLatin1String("BalanceB"))scriptNameInd=QLatin1String("Balance")+baseValues.currentPair.currBStr;
    else
    if(!symbol.isEmpty())prependName=symbol+"_";

    if(val<0.00000001&&scriptNameInd.endsWith(QLatin1String("price"),Qt::CaseInsensitive))return;

    if(!forceEmit&&!testMode&&indicatorsMap.value(prependName+scriptNameInd,-2508.1987)==val)return;

    indicatorsMap[prependName+scriptNameInd]=val;
    if(engine==0)return;
    emit valueChanged(symbol,scriptNameInd,val);
}

void ScriptObject::initValueChanged(QString symbol, QString scriptNameInd, double val)
{
    initValueChangedPrivate(symbol,scriptNameInd,val,false);
}

void ScriptObject::indicatorValueChanged(double val)
{
    if(!isRunningFlag)return;
    QObject *senderObject=sender();if(senderObject==0)return;
    QString scriptNameInd=senderObject->property("ScriptName").toString();
    if(scriptNameInd.isEmpty())return;
    initValueChanged(baseValues.currentPair.symbolSecond(),scriptNameInd,val);
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
    emit valueChanged(baseValues.currentPair.symbolSecond(),QLatin1String("Time"),getTimeT());
    if(testMode)return;
    secondTimer->start(1000);
}

void ScriptObject::deleteEngine()
{
    if(!engine)return;
    if(!testMode)
        Q_FOREACH(QDoubleSpinBox *spinBox, spinBoxList)
            disconnect(spinBox,SIGNAL(valueChanged(double)),this,SLOT(indicatorValueChanged(double)));

    engine->deleteLater();
    engine=0;
    if(!testMode&&scriptWantsOrderBookData)
    {
        scriptWantsOrderBookData=false;
        baseValues.scriptsThatUseOrderBookCount--;
    }
}

void ScriptObject::setRunning(bool on)
{
    if(!testMode&&!on){secondTimer->stop();}
    if(on==isRunningFlag)return;
    isRunningFlag=on;
    if(!isRunningFlag)
        if(engine)
        {
            qDeleteAll(timerMap.keys());
            timerMap.clear();
            deleteEngine();
        }
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
    if(engine)
    {
        qDeleteAll(timerMap.keys());
        timerMap.clear();
        deleteEngine();
    }
    engine=new QScriptEngine;

    QScriptValue scriptValue=engine->newQObject(this);
    engine->globalObject().setProperty("trader", scriptValue);

    bool anyValue=script.contains("trader.on(\"AnyValue\").changed()",Qt::CaseInsensitive)||script.contains("trader.on('AnyValue').changed()",Qt::CaseInsensitive);
    haveTimer=script.contains("trader.on(\"Time\").changed()",Qt::CaseInsensitive)||script.contains("trader.on('Time').changed()",Qt::CaseInsensitive);

    if(!testMode)
    {
        if(scriptWantsOrderBookData&&baseValues.scriptsThatUseOrderBookCount>0)baseValues.scriptsThatUseOrderBookCount--;
        scriptWantsOrderBookData=script.contains("trader.get(\"Asks",Qt::CaseInsensitive)||script.contains("trader.get('Asks",Qt::CaseInsensitive)||script.contains("trader.get(\"Bids",Qt::CaseInsensitive)||script.contains("trader.get('Bids",Qt::CaseInsensitive);
    }
    else scriptWantsOrderBookData=false;

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
            if(!testMode&&scriptWantsOrderBookData)baseValues.scriptsThatUseOrderBookCount++;
            bool scriptContainsBalance=script.contains("Balance",Qt::CaseInsensitive);
            Q_FOREACH(QDoubleSpinBox *spinBox, spinBoxList)
            {
            QString spinProperty=spinBox->property("ScriptName").toString();
            bool needConnect=anyValue;
            if(!needConnect&&spinProperty.contains("BALANCE",Qt::CaseInsensitive)&&scriptContainsBalance)needConnect=true;
            if(!needConnect)needConnect=script.contains(spinProperty,Qt::CaseInsensitive);
            if(!needConnect)
            {
                if(!testMode)disconnect(spinBox,SIGNAL(valueChanged(double)),this,SLOT(indicatorValueChanged(double)));
                continue;
            }
            double spinValue=spinBox->value();
            QString symbolTemp=baseValues.currentPair.symbolSecond();
            initValueChangedPrivate(symbolTemp,spinProperty,spinValue,testMode);
            if(!testMode)connect(spinBox,SIGNAL(valueChanged(double)),this,SLOT(indicatorValueChanged(double)));
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

    text.replace("\" ,","\",");
    text.replace("' ,","',");
	text.replace("\\","\\\\");

    text.replace("trader.get(\"Balance\",\"","trader.get(\"Balance",Qt::CaseInsensitive);
    text.replace("trader.on(\"Balance\",\"","trader.on(\"Balance",Qt::CaseInsensitive);

    text.replace("trader.get('Balance'',\"","trader.get('Balance",Qt::CaseInsensitive);
    text.replace("trader.on('Balance'',\"","trader.on('Balance",Qt::CaseInsensitive);

    text.replace(").changed()",")",Qt::CaseInsensitive);

    while(replaceString("trader.on('","trader['valueChanged(QString,QString,double)'].connect(function(symbol,name,value){if(name=='",text,true));
    while(replaceString("trader.on(\"","trader['valueChanged(QString,QString,double)'].connect(function(symbol,name,value){if(name==\"",text,true));

    text.replace("trader.get(\"AsksPrice\",","trader.getAsksPriceByVol(",Qt::CaseInsensitive);
    text.replace("trader.get(\"AsksVolume\",","trader.getAsksVolByPrice(",Qt::CaseInsensitive);
    text.replace("trader.get(\"BidsPrice\",","trader.getBidsPriceByVol(",Qt::CaseInsensitive);
    text.replace("trader.get(\"BidsVolume\",","trader.getBidsVolByPrice(",Qt::CaseInsensitive);
    text.replace("trader.get('AsksPrice',","trader.getAsksPriceByVol(",Qt::CaseInsensitive);
    text.replace("trader.get('AsksVolume',","trader.getAsksVolByPrice(",Qt::CaseInsensitive);
    text.replace("trader.get('BidsPrice',","trader.getBidsPriceByVol(",Qt::CaseInsensitive);
    text.replace("trader.get('BidsVolume',","trader.getBidsVolByPrice(",Qt::CaseInsensitive);

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
