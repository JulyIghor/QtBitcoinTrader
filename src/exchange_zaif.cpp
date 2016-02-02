//  This file is part of Qt Bitcion Trader
//      https://github.com/JulyIGHOR/QtBitcoinTrader
//  Copyright (C) 2013-2015 July IGHOR <julyighor@gmail.com>
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

#include "exchange_zaif.h"
#include <openssl/hmac.h>

Exchange_Zaif::Exchange_Zaif(QByteArray pRestSign, QByteArray pRestKey)
	: Exchange()
{
	checkDuplicatedOID=true;
	accountFee=0.0;
	balanceDisplayAvailableAmount=false;
	minimumRequestIntervalAllowed=600;
	calculatingFeeMode=1;
	isLastTradesTypeSupported=true;
	exchangeSupportsAvailableAmount=true;
	lastBidAskTimestamp=0;
	baseValues.exchangeName="Zaif";
	baseValues.currentPair.name="BTC/JPY";
	baseValues.currentPair.setSymbol("BTCJPY");
	baseValues.currentPair.currRequestPair="btc_jpy";
	baseValues.currentPair.priceDecimals=0;
	baseValues.currentPair.priceMin=qPow(0.1,baseValues.currentPair.priceDecimals);
	baseValues.currentPair.tradeVolumeMin=0.01;
	baseValues.currentPair.tradePriceMin=5;
	depthAsks=0;
	depthBids=0;
	forceDepthLoad=false;
	julyHttp=0;
	tickerOnly=false;
	setApiKeySecret(pRestKey.split(':').last(),pRestSign);
	privateClientId=pRestKey.split(':').first();

	currencyMapFile="Zaif";
	defaultCurrencyParams.currADecimals=8;
	defaultCurrencyParams.currBDecimals=5;
	defaultCurrencyParams.currABalanceDecimals=8;
	defaultCurrencyParams.currBBalanceDecimals=5;
	defaultCurrencyParams.priceDecimals=0;
	defaultCurrencyParams.priceMin=qPow(0.1,baseValues.currentPair.priceDecimals);

	supportsLoginIndicator=false;
    supportsAccountVolume=false;

	moveToThread(this);
	authRequestTime.restart();
    privateNonce=(TimeSync::getTimeT()-1371854884)*10;
}

Exchange_Zaif::~Exchange_Zaif()
{
}

void Exchange_Zaif::filterAvailableUSDAmountValue(double *amount)
{
    double decValue=cutDoubleDecimalsCopy((*amount)*mainWindow.floatFee,baseValues.currentPair.priceDecimals,false);
	decValue+=qPow(0.1,qMax(baseValues.currentPair.priceDecimals,1));
    *amount=cutDoubleDecimalsCopy((*amount)-decValue,baseValues.currentPair.currBDecimals,false);
}

void Exchange_Zaif::clearVariables()
{
	isFirstTicker=true;
	cancelingOrderIDs.clear();
	Exchange::clearVariables();
	secondPart=0;
	apiDownCounter=0;
	lastHistory.clear();
	lastOrders.clear();
	reloadDepth();
	lastInfoReceived=false;
	lastBidAskTimestamp=0;
    lastTradesDate=TimeSync::getTimeT()-600;
	lastTickerDate=0;
	lastFetchTid.clear();
}

void Exchange_Zaif::clearValues()
{
	clearVariables();
	if(julyHttp)julyHttp->clearPendingData();
}

void Exchange_Zaif::secondSlot()
{
	static int infoCounter=0;
	switch(infoCounter)
	{
	case 0: if(!tickerOnly&&!isReplayPending(204))sendToApi(204,"active_orders",true,true,"currency_pair=btc_jpy"); break;
	case 1: if(!isReplayPending(202))sendToApi(202,"get_info",true,true); break;
	case 2: if(!isReplayPending(103))sendToApi(103,"ticker",false,true); break;
	case 3: if(!isReplayPending(109))sendToApi(109,"trades",false,true); break;
	case 4: if(lastHistory.isEmpty()&&!isReplayPending(208))sendToApi(208,"trade_history",true,true,"currency_pair=btc_jpy"); break;
    case 5:
        if(lastHistory.isEmpty())getHistory(false);
        break;
	default: break;
	}

	if(isDepthEnabled())
	{
		if(forceDepthLoad||/*infoCounter==5&&*/!isReplayPending(111))
		{
			emit depthRequested();
			sendToApi(111,"depth",false,true);
			forceDepthLoad=false;
		}
	}

	if(++infoCounter>5)
	{
		infoCounter=0;
        quint32 syncNonce=(TimeSync::getTimeT()-1371854884)*10;
		if(privateNonce<syncNonce)privateNonce=syncNonce;
	}

	Exchange::secondSlot();
}

bool Exchange_Zaif::isReplayPending(int reqType)
{
	if(julyHttp==0)return false;
	return julyHttp->isReqTypePending(reqType);
}

void Exchange_Zaif::getHistory(bool force)
{
	if(tickerOnly)return;
	if(force)lastHistory.clear();
	if(!isReplayPending(208))sendToApi(208,"trade_history",true,true,"currency_pair=btc_jpy");
}

void Exchange_Zaif::buy(QString symbol, double apiBtcToBuy, double apiPriceToBuy)
{
	if(tickerOnly)return;

    CurrencyPairItem pairItem;
    pairItem=baseValues.currencyPairMap.value(symbol,pairItem);
    if(pairItem.symbol.isEmpty())return;

    QByteArray params="currency_pair=btc_jpy&action=bid&amount="+byteArrayFromDouble(apiBtcToBuy,pairItem.currADecimals,0)+"&price="+byteArrayFromDouble(apiPriceToBuy,pairItem.priceDecimals,0);
	if(debugLevel)logThread->writeLog("Buy: "+params,2);
	sendToApi(306,"trade",true,true,params);
}

void Exchange_Zaif::sell(QString symbol, double apiBtcToSell, double apiPriceToSell)
{
	if(tickerOnly)return;

    CurrencyPairItem pairItem;
    pairItem=baseValues.currencyPairMap.value(symbol,pairItem);
    if(pairItem.symbol.isEmpty())return;

    QByteArray params="currency_pair=btc_jpy&action=ask&amount="+byteArrayFromDouble(apiBtcToSell,pairItem.currADecimals,0)+"&price="+byteArrayFromDouble(apiPriceToSell,pairItem.priceDecimals,0);
	if(debugLevel)logThread->writeLog("Sell: "+params,2);
	sendToApi(307,"trade",true,true,params);
}

void Exchange_Zaif::cancelOrder(QString, QByteArray order)
{
	if(tickerOnly)return;
	cancelingOrderIDs<<order;
	if(debugLevel)logThread->writeLog("Cancel order: "+order,2);
	sendToApi(305,"cancel_order",true,true,"order_id="+order);
}

void Exchange_Zaif::sendToApi(int reqType, QByteArray method, bool auth, bool sendNow, QByteArray commands)
{
	if(julyHttp==0)
	{ 
		julyHttp=new JulyHttp("api.zaif.jp","",this);
		connect(julyHttp,SIGNAL(anyDataReceived()),baseValues_->mainWindow_,SLOT(anyDataReceived()));
		connect(julyHttp,SIGNAL(setDataPending(bool)),baseValues_->mainWindow_,SLOT(setDataPending(bool)));
		connect(julyHttp,SIGNAL(apiDown(bool)),baseValues_->mainWindow_,SLOT(setApiDown(bool)));
		connect(julyHttp,SIGNAL(errorSignal(QString)),baseValues_->mainWindow_,SLOT(showErrorMessage(QString)));
		connect(julyHttp,SIGNAL(sslErrorSignal(const QList<QSslError> &)),this,SLOT(sslErrors(const QList<QSslError> &)));
		connect(julyHttp,SIGNAL(dataReceived(QByteArray,int)),this,SLOT(dataReceivedAuth(QByteArray,int)));
	}

	if(auth)
	{
        QByteArray postData="method="+method+"&nonce="+QByteArray::number(++privateNonce);
		if(!commands.isEmpty())postData.append("&"+commands);
		if(sendNow)
            julyHttp->sendData(reqType, "POST /tapi", postData, "Key: "+getApiKey()+"\r\n"+"Sign: "+hmacSha512(getApiSign(),postData).toHex()+"\r\n");
		else
            julyHttp->sendData(reqType, "POST /tapi", postData, "Key: "+getApiKey()+"\r\n"+"Sign: "+hmacSha512(getApiSign(),postData).toHex()+"\r\n");

	}
	else
	{
		if(sendNow)
			julyHttp->sendData(reqType, "GET /api/1/"+method+"/btc_jpy");
		else 
			julyHttp->prepareData(reqType, "GET /api/1/"+method+"/btc_jpy");
	}
}

void Exchange_Zaif::depthUpdateOrder(QString symbol, double price, double amount, bool isAsk)
{
    if(symbol!=baseValues.currentPair.symbol)return;

	if(isAsk)
	{
		if(depthAsks==0)return;
		DepthItem newItem;
		newItem.price=price;
		newItem.volume=amount;
		if(newItem.isValid())
			(*depthAsks)<<newItem;
	}
	else
	{
		if(depthBids==0)return;
		DepthItem newItem;
		newItem.price=price;
		newItem.volume=amount;
		if(newItem.isValid())
			(*depthBids)<<newItem;
	}
}

void Exchange_Zaif::depthSubmitOrder(QString symbol, QMap<double,double> *currentMap ,double priceDouble, double amount, bool isAsk)
{
    if(symbol!=baseValues.currentPair.symbol)return;

	if(priceDouble==0.0||amount==0.0)return;

	if(isAsk)
	{
		(*currentMap)[priceDouble]=amount;
		if(lastDepthAsksMap.value(priceDouble,0.0)!=amount)
            depthUpdateOrder(symbol,priceDouble,amount,true);
	}
	else
	{
		(*currentMap)[priceDouble]=amount;
		if(lastDepthBidsMap.value(priceDouble,0.0)!=amount)
            depthUpdateOrder(symbol,priceDouble,amount,false);
	}
}

void Exchange_Zaif::reloadDepth()
{
	lastDepthBidsMap.clear();
	lastDepthAsksMap.clear();
	lastDepthData.clear();
	Exchange::reloadDepth();
}

void Exchange_Zaif::dataReceivedAuth(QByteArray data, int reqType)
{
	if(debugLevel)logThread->writeLog("RCV: "+data);
    if(data.size()&&data.at(0)==QLatin1Char('<'))return;

    bool success=((!data.startsWith("{\"success\": 0, \"error\"")&&(data.startsWith("{")))||data.startsWith("["))||data=="true"||data=="false";
    QString errorString="";

    switch(reqType)
	{
	case 103: //ticker
        if(data.startsWith("{\"last\":"))
        {
            QByteArray tickerHigh=getMidData("\"high\":",",",&data);
			if(!tickerHigh.isEmpty())
			{
                double newTickerHigh=tickerHigh.toDouble();
                if(newTickerHigh!=lastTickerHigh)
                    IndicatorEngine::setValue(baseValues.exchangeName,baseValues.currentPair.symbol,"High",newTickerHigh);
				lastTickerHigh=newTickerHigh;
			}

			QByteArray tickerLow=getMidData("\"low\":",",",&data);
			if(!tickerLow.isEmpty())
			{
                double newTickerLow=tickerLow.toDouble();
                if(newTickerLow!=lastTickerLow)
                    IndicatorEngine::setValue(baseValues.exchangeName,baseValues.currentPair.symbol,"Low",newTickerLow);
				lastTickerLow=newTickerLow;
			}

            QByteArray tickerSell=getMidData("\"bid\":",",",&data);
			if(!tickerSell.isEmpty())
			{
                double newTickerSell=tickerSell.toDouble();
                if(newTickerSell!=lastTickerSell)
                    IndicatorEngine::setValue(baseValues.exchangeName,baseValues.currentPair.symbol,"Sell",newTickerSell);
				lastTickerSell=newTickerSell;
			}

            QByteArray tickerBuy=getMidData("\"ask\":","}",&data);
			if(!tickerBuy.isEmpty())
			{
                double newTickerBuy=tickerBuy.toDouble();
                if(newTickerBuy!=lastTickerBuy)
                    IndicatorEngine::setValue(baseValues.exchangeName,baseValues.currentPair.symbol,"Buy",newTickerBuy);
				lastTickerBuy=newTickerBuy;
			}

            QByteArray tickerVolume=getMidData("\"volume\":",",",&data);
			if(!tickerVolume.isEmpty())
			{
                double newTickerVolume=tickerVolume.toDouble();
                if(newTickerVolume!=lastTickerVolume)
                    IndicatorEngine::setValue(baseValues.exchangeName,baseValues.currentPair.symbol,"Volume",newTickerVolume);
				lastTickerVolume=newTickerVolume;
			}

            QByteArray tickerLast=getMidData("\"last\":",",",&data);
            if(!tickerLast.isEmpty())
            {
                double newTickerLast=tickerLast.toDouble();
                if(newTickerLast!=lastTickerLast)
                    IndicatorEngine::setValue(baseValues.exchangeName,baseValues.currentPair.symbol,"Last",newTickerLast);
                lastTickerLast=newTickerLast;
            }

			if(isFirstTicker)
			{
				emit firstTicker();
				isFirstTicker=false;
			}
		}
        else {
            success=false;
            errorString+="Invalid ticker data. ";
        }
		break;//ticker
	case 109: //trades
		if(success&&data.size()>32)
		{
			if(data.startsWith("[{\"date\":"))
			{
				QStringList tradeList=QString(data).split("}, {");
				QList<TradesItem> *newTradesItems=new QList<TradesItem>;

				for(int n=tradeList.count()-1;n>=0;n--)
				{
					QByteArray tradeData=tradeList.at(n).toLatin1();
					TradesItem newItem;
					QByteArray nextFetchTid=getMidData("\"tid\": ",",",&tradeData);
//					if(nextFetchTid<=lastFetchTid)continue;
					newItem.date=getMidData("\"date\": ",",",&tradeData).toUInt();
					if(newItem.date<=lastTradesDate)continue;
					newItem.amount=getMidData("\"amount\": ",",",&tradeData).toDouble();
		            newItem.symbol=baseValues.currentPair.symbol;
					newItem.orderType=getMidData("\"trade_type\": \"","\"",&tradeData)=="bid"?1:-1;
					QByteArray priceBytes=getMidData("\"price\": ",",",&tradeData);
					newItem.price=priceBytes.toDouble();
					if(n==0&&newItem.price>0.0)
					{
						lastTradesDate=newItem.date;
						if(lastTickerDate<newItem.date)
						{
						lastTickerDate=newItem.date;
                        IndicatorEngine::setValue(baseValues.exchangeName,baseValues.currentPair.symbol,"Last",newItem.price);
						}
					}
					newItem.symbol=baseValues.currentPair.symbol;
					//newItem.type=0;
                    if(n==tradeList.count()-1&&!nextFetchTid.isEmpty())lastFetchTid=nextFetchTid;

					if(newItem.isValid())(*newTradesItems)<<newItem;
					else if(debugLevel)logThread->writeLog("Invalid trades fetch data line:"+tradeData,2);
				}
				if(newTradesItems->count())emit addLastTrades(baseValues.currentPair.symbol,newTradesItems);
				else delete newTradesItems;
			}
			else if(debugLevel)logThread->writeLog("Invalid trades fetch data:"+data,2);
		}
		break;
	case 111: //depth
        if(data.startsWith("{\"asks\": [["))
		{
			emit depthRequestReceived();

			if(lastDepthData!=data)
			{
				lastDepthData=data;
				depthAsks=new QList<DepthItem>;
				depthBids=new QList<DepthItem>;

                QMap<double,double> currentAsksMap;
				QStringList asksList=QString(getMidData("asks\": [[","]]",&data)).split("], [");
                double groupedPrice=0.0;
                double groupedVolume=0.0;
				int rowCounter=0;

				for(int n=0;n<asksList.count();n++)
				{
					if(baseValues.depthCountLimit&&rowCounter>=baseValues.depthCountLimit)break;
					QStringList currentPair=asksList.at(n).split(",");
					if(currentPair.count()!=2)continue;
                    double priceDouble=currentPair.first().toDouble();
                    double amount=currentPair.last().toDouble();

					if(baseValues.groupPriceValue>0.0)
					{
						if(n==0)
						{
                            emit depthFirstOrder(baseValues.currentPair.symbol,priceDouble,amount,true);
							groupedPrice=baseValues.groupPriceValue*(int)(priceDouble/baseValues.groupPriceValue);
							groupedVolume=amount;
						}
						else
						{
							bool matchCurrentGroup=priceDouble<groupedPrice+baseValues.groupPriceValue;
							if(matchCurrentGroup)groupedVolume+=amount;
							if(!matchCurrentGroup||n==asksList.count()-1)
							{
                                depthSubmitOrder(baseValues.currentPair.symbol,
                                                 &currentAsksMap,groupedPrice+baseValues.groupPriceValue,groupedVolume,true);
								rowCounter++;
								groupedVolume=amount;
								groupedPrice+=baseValues.groupPriceValue;
							}
						}
					}
					else
					{
                        depthSubmitOrder(baseValues.currentPair.symbol,
                                         &currentAsksMap,priceDouble,amount,true);
						rowCounter++;
					}
				}
                QList<double> currentAsksList=lastDepthAsksMap.keys();
				for(int n=0;n<currentAsksList.count();n++)
                    if(currentAsksMap.value(currentAsksList.at(n),0)==0)depthUpdateOrder(baseValues.currentPair.symbol,
                                                                                         currentAsksList.at(n),0.0,true);
				lastDepthAsksMap=currentAsksMap;

                QMap<double,double> currentBidsMap;
				QStringList bidsList=QString(getMidData("bids\": [[","]]",&data)).split("], [");
				groupedPrice=0.0;
				groupedVolume=0.0;
				rowCounter=0;

				for(int n=0;n<bidsList.count();n++)
				{
					if(baseValues.depthCountLimit&&rowCounter>=baseValues.depthCountLimit)break;
					QStringList currentPair=bidsList.at(n).split(",");
					if(currentPair.count()!=2)continue;
                    double priceDouble=currentPair.first().toDouble();
                    double amount=currentPair.last().toDouble();
					if(baseValues.groupPriceValue>0.0)
					{
						if(n==0)
						{
                            emit depthFirstOrder(baseValues.currentPair.symbol,priceDouble,amount,false);
							groupedPrice=baseValues.groupPriceValue*(int)(priceDouble/baseValues.groupPriceValue);
							groupedVolume=amount;
						}
						else
						{
							bool matchCurrentGroup=priceDouble>groupedPrice-baseValues.groupPriceValue;
							if(matchCurrentGroup)groupedVolume+=amount;
							if(!matchCurrentGroup||n==asksList.count()-1)
							{
                                depthSubmitOrder(baseValues.currentPair.symbol,
                                                 &currentBidsMap,groupedPrice-baseValues.groupPriceValue,groupedVolume,false);
								rowCounter++;
								groupedVolume=amount;
								groupedPrice-=baseValues.groupPriceValue;
							}
						}
					}
					else
					{
                        depthSubmitOrder(baseValues.currentPair.symbol,
                                         &currentBidsMap,priceDouble,amount,false);
						rowCounter++;
					}
				}
                QList<double> currentBidsList=lastDepthBidsMap.keys();
				for(int n=0;n<currentBidsList.count();n++)
                    if(currentBidsMap.value(currentBidsList.at(n),0)==0)depthUpdateOrder(baseValues.currentPair.symbol,
                                                                                         currentBidsList.at(n),0.0,false);
				lastDepthBidsMap=currentBidsMap;

                emit depthSubmitOrders(baseValues.currentPair.symbol,depthAsks, depthBids);
				depthAsks=0;
				depthBids=0;
			}
		}
        else {
            success=false;
            errorString+="Invalid depth data. ";
            if(debugLevel)logThread->writeLog("Invalid depth data:"+data,2);
        }
		break;
	case 202: //get_info
        if(data.startsWith("{\"success\": 1, \"return\": {\"funds\":"))
        {
            QByteArray fundsData=getMidData("funds\": {","}",&data)+",";
            QByteArray btcBalance=getMidData("\""+baseValues.currentPair.currAStr.toLower()+"\":",",",&fundsData);
			if(!btcBalance.isEmpty())
            {
                double newBtcBalance=btcBalance.toDouble();
                if(lastBtcBalance!=newBtcBalance)emit accBtcBalanceChanged(baseValues.currentPair.symbol,newBtcBalance);
				lastBtcBalance=newBtcBalance;
			}

            QByteArray usdBalance=getMidData("\""+baseValues.currentPair.currBStr.toLower()+"\":",",",&fundsData);
			if(!usdBalance.isEmpty())
			{
                double newUsdBalance=usdBalance.toDouble();
                if(newUsdBalance!=lastUsdBalance)emit accUsdBalanceChanged(baseValues.currentPair.symbol,newUsdBalance);
				lastUsdBalance=newUsdBalance;
			}
		}
        else {
            success=false;
            errorString+="Invalid info data. ";
        }
		break;//get_info
	case 204://open_orders
        if(data.startsWith("{\"success\": 1, \"return\": {"))
		{
//          if(lastOrders!=data){
            lastOrders=data;
            if(data.startsWith("{\"success\": 1 ,\"return\": {}"))
            {
				emit ordersIsEmpty();
				break;
			}

            QByteArray dataBuy=getMidData("\"return\": {","}}}",&data);
            QStringList ordersList=QString(dataBuy).split("}, {");
            if(ordersList.count()==0)return;
            QList<OrderItem> *orders=new QList<OrderItem>;

			for(int n=0;n<ordersList.count();n++)
			{
				OrderItem currentOrder;
                QByteArray currentOrderData=ordersList.at(n).toLatin1()+"}";

                currentOrder.oid=getMidData("\"","\": {",&currentOrderData);
                currentOrder.date=getMidData("\"timestamp\": \"","\"",&currentOrderData).toUInt();
                currentOrder.type=getMidData("\"action\":\"","\"",&currentOrderData)=="bid";
                currentOrder.amount=getMidData("\"amount\":",",",&currentOrderData).toDouble();
                currentOrder.price=getMidData("\"price\":",",",&currentOrderData).toDouble();
//                currentOrder.symbol=getMidData("\"currency_pair\": \"","\"",&currentOrderData);
                currentOrder.symbol=baseValues.currentPair.symbol;
				currentOrder.status=1;
				if(currentOrder.isValid())(*orders)<<currentOrder;
			}
            emit orderBookChanged(baseValues.currentPair.symbol,orders);
//          }
		}
        else {
            success=false;
            errorString+="Invalid orders data. ";
        }
		break;//active_orders
	case 305: //cancel_order
		{
			if(!success)break;
			if(!cancelingOrderIDs.isEmpty())
            {
                if(data.startsWith("{\"success\": 1"))emit orderCanceled(baseValues.currentPair.symbol,cancelingOrderIDs.first());
                if(debugLevel)logThread->writeLog("Order canceled:"+cancelingOrderIDs.first(),2);
                cancelingOrderIDs.removeFirst();
            }
		}
		break;//cancel_order
	case 306: //order/buy
		if(!success||!debugLevel)break;
			  if(data.startsWith("{\"success\": 1"))logThread->writeLog("Buy OK: "+data);
			  else logThread->writeLog("Invalid Order Buy Data:"+data);
		break;//order/buy
	case 307: //order/sell
		if(!success||!debugLevel)break;
			  if(data.startsWith("{\"success\": 1"))logThread->writeLog("Sell OK: "+data);
			  else logThread->writeLog("Invalid Order Sell Data:"+data);
		break;//order/sell
	case 208: //trade_history
		if(!success)break;
        if(data.startsWith("{\"success\": 1, \"return\":"))
        {
            QByteArray historyData=getMidData("\"return\": {","}}}",&data)+"^";
            if(lastHistory!=historyData)
            {
                lastHistory=historyData;
                if(historyData=="^")break;
                QString newLog(historyData);
                QStringList dataList=newLog.split("}, ");
                if(dataList.count()==0)return;

//                quint32 currentId;
//                quint32 maxId=0;
                QList<HistoryItem> *historyItems=new QList<HistoryItem>;
                for(int n=0;n<dataList.count();n++)
                {
                    QByteArray curLog(dataList.at(n).toLatin1());

//                    currentId=getMidData("\"","\": {",&curLog).toUInt();
//                    if(currentId<=lastHistoryId)break;
//                    if(n==0)maxId=currentId;

                    HistoryItem currentHistoryItem;
                    QByteArray logType=getMidData("your_action\": \"","\"",&curLog);
                    if(logType=="ask")currentHistoryItem.type=1;else if(logType=="bid")currentHistoryItem.type=2;
                    if(currentHistoryItem.type)
                    {
                        currentHistoryItem.symbol=getMidData("currency_pair\": \"","\"",&curLog);
                        currentHistoryItem.symbol=baseValues.currentPair.symbol;
                        currentHistoryItem.dateTimeInt=getMidData("timestamp\": \"","\",",&curLog).toUInt();
                        currentHistoryItem.price=getMidData("price\":",",",&curLog).toDouble();
                        currentHistoryItem.volume=getMidData("amount\":",",",&curLog).toDouble();

                        if(currentHistoryItem.isValid())(*historyItems)<<currentHistoryItem;
                    }
                }
//                if(maxId>lastHistoryId)lastHistoryId=maxId;
                emit historyChanged(historyItems);
            }
        }
        else {
            success=false;
            errorString+="Invalid history data. ";
        }
		break;//trade_history
	default: break;
	}

	static int errorCount=0;
	if(!success&&reqType!=305)
	{
		errorCount++;
		QString errorString;
		bool invalidMessage=!data.startsWith("{");
		if(!invalidMessage)
		{
			errorString=getMidData("[\"","\"]",&data);
			if(errorString.isEmpty())
			{
				QByteArray nErrorString=getMidData("{\"error\":","}",&data);
				errorString=getMidData("\"","\"",&nErrorString);
			}
		}
		else errorString=data;
		if(debugLevel)logThread->writeLog("API Error: "+errorString.toLatin1()+" ReqType:"+QByteArray::number(reqType),2);
		if(errorCount<3&&reqType<300&&errorString!="Invalid username and/or password")return;
		if(errorString.isEmpty())return;
		errorString.append("<br>"+QString::number(reqType));
		if(invalidMessage||reqType<300)emit showErrorMessage("I:>"+errorString);
	}
	else errorCount=0;
}

void Exchange_Zaif::sslErrors(const QList<QSslError> &errors)
{
	QStringList errorList;
	for(int n=0;n<errors.count();n++)errorList<<errors.at(n).errorString();
	if(debugLevel)logThread->writeLog(errorList.join(" ").toLatin1(),2);
	emit showErrorMessage("SSL Error: "+errorList.join(" "));
}
