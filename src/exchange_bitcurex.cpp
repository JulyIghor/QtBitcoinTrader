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

#include "exchange_bitcurex.h"
#include <openssl/hmac.h>

Exchange_BitCurex::Exchange_BitCurex(QByteArray pRestSign, QByteArray pRestKey) 
	: Exchange()
{
    calculatingFeeMode=1;
    clearHistoryOnCurrencyChanged=true;
    baseValues.exchangeName="BitCurex";
    baseValues.currentPair.name="BTC/PLN";
    baseValues.currentPair.setSymbol("BTCPLN");
    baseValues.currentPair.currRequestPair="btc_pln";
    baseValues.currentPair.priceDecimals=5;
    minimumRequestIntervalAllowed=500;
	baseValues.currentPair.priceMin=qPow(0.1,baseValues.currentPair.priceDecimals);
	baseValues.currentPair.tradeVolumeMin=0.01;
	baseValues.currentPair.tradePriceMin=0.1;
	depthAsks=0;
	depthBids=0;
	forceDepthLoad=false;
	julyHttp=0;
	isApiDown=false;
	tickerOnly=false;
	setApiKeySecret(pRestKey,pRestSign);

	moveToThread(this);

	currencyMapFile="BitCurex";
	defaultCurrencyParams.currADecimals=8;
	defaultCurrencyParams.currBDecimals=8;
	defaultCurrencyParams.currABalanceDecimals=8;
	defaultCurrencyParams.currBBalanceDecimals=8;
    defaultCurrencyParams.priceDecimals=5;
	defaultCurrencyParams.priceMin=qPow(0.1,baseValues.currentPair.priceDecimals);

	supportsLoginIndicator=false;
    supportsAccountVolume=false;

	authRequestTime.restart();
    privateNonce=(TimeSync::getTimeT()-1371854884)*10;
}

Exchange_BitCurex::~Exchange_BitCurex()
{
}

void Exchange_BitCurex::clearVariables()
{
    isFirstAccInfo=false;
	Exchange::clearVariables();
	lastOpenedOrders=-1;
	apiDownCounter=0;
	lastHistory.clear();
	lastOrders.clear();
	reloadDepth();
    lastFetchTid=0;
    startTradesDate=TimeSync::getTimeT()-600;
    lastTradesDate=0;
    lastHistoryId=0;
}

void Exchange_BitCurex::clearValues()
{
	clearVariables();
	if(julyHttp)julyHttp->clearPendingData();
}

void Exchange_BitCurex::reloadDepth()
{
	lastDepthBidsMap.clear();
	lastDepthAsksMap.clear();
	lastDepthData.clear();
	Exchange::reloadDepth();
}

void Exchange_BitCurex::dataReceivedAuth(QByteArray data, int reqType)
{
    if(debugLevel)logThread->writeLog("RCV: "+data);
    if(data.size()<4)return;
    if(data.at(0)==QLatin1Char('<'))return;

    bool success=true;

	switch(reqType)
	{
    case 103: //ticker
        if(data.startsWith("{\"lowest_tx_price_h\": \""))
        {
            QByteArray tickerHigh=getMidData("\"highest_tx_price_h\": \"","\", \"",&data);
			if(!tickerHigh.isEmpty())
			{
                double newTickerHigh=tickerHigh.toDouble();
                if(newTickerHigh!=lastTickerHigh)
                    IndicatorEngine::setValue(baseValues.exchangeName,baseValues.currentPair.symbol,"High",newTickerHigh);
				lastTickerHigh=newTickerHigh;
			}

            QByteArray tickerLow=getMidData("\"lowest_tx_price_h\": \"","\", \"",&data);
			if(!tickerLow.isEmpty())
			{
                double newTickerLow=tickerLow.toDouble();
                if(newTickerLow!=lastTickerLow)
                    IndicatorEngine::setValue(baseValues.exchangeName,baseValues.currentPair.symbol,"Low",newTickerLow);
				lastTickerLow=newTickerLow;
			}

            QByteArray tickerSell=getMidData("\"best_bid_h\": \"","\", \"",&data);
			if(!tickerSell.isEmpty())
            {
                double newTickerSell=tickerSell.toDouble();
                if(newTickerSell!=lastTickerSell)
                    IndicatorEngine::setValue(baseValues.exchangeName,baseValues.currentPair.symbol,"Sell",newTickerSell);
				lastTickerSell=newTickerSell;
			}

            QByteArray tickerBuy=getMidData("\"best_ask_h\": \"","\", \"",&data);
			if(!tickerBuy.isEmpty())
            {
                double newTickerBuy=tickerBuy.toDouble();
                if(newTickerBuy!=lastTickerBuy)
                    IndicatorEngine::setValue(baseValues.exchangeName,baseValues.currentPair.symbol,"Buy",newTickerBuy);
				lastTickerBuy=newTickerBuy;
			}

            QByteArray tickerVolume=getMidData("\"total_volume_h\": \"","\", \"",&data);
			if(!tickerVolume.isEmpty())
			{
                double newTickerVolume=tickerVolume.toDouble();
                if(newTickerVolume!=lastTickerVolume)
                    IndicatorEngine::setValue(baseValues.exchangeName,baseValues.currentPair.symbol,"Volume",newTickerVolume);
				lastTickerVolume=newTickerVolume;
			}

            QByteArray tickerLast=getMidData("\"last_tx_price_h\": \"","\", \"",&data);
            if(!tickerLast.isEmpty())
            {
                double newTickerLast=tickerLast.toDouble();
                if(newTickerLast!=lastTickerLast)
                    IndicatorEngine::setValue(baseValues.exchangeName,baseValues.currentPair.symbol,"Last",newTickerLast);
                lastTickerLast=newTickerLast;
            }
        }
        else success=false;
		break;//ticker
    case 109: //trades
        if(data.startsWith("{\"status\": \"ok\", \"data\": {\"symbol\": \""+baseValues.currentPair.symbol.toLower().toLatin1()+"\", \"trades\": [{\""))
		{
            QStringList tradeList=QString(data).split("}, {");
			QList<TradesItem> *newTradesItems=new QList<TradesItem>;

            TradesItem newItem;
            for(int n=tradeList.count()-1;n>=0;n--)
			{
				QByteArray tradeData=tradeList.at(n).toLatin1()+"}";

                newItem.date=getMidData("\"ts\": ",", \"",&tradeData).toUInt();
                if(newItem.date<startTradesDate)continue;

                quint32 currentTid=getMidData("\"txid\": ",", \"",&tradeData).toUInt();
                if(lastFetchTid>=currentTid)continue;
                lastFetchTid=currentTid;

                newItem.price=getMidData("\"price\": ",", \"",&tradeData).toDouble();
                newItem.amount=getMidData("\"amount\": ",", \"",&tradeData).toDouble();
                newItem.orderType=getMidData("\"type\": \"","\"}",&tradeData)=="ask"?-1:1;
                newItem.symbol=baseValues.currentPair.symbol;

				if(newItem.isValid())(*newTradesItems)<<newItem;
				else if(debugLevel)logThread->writeLog("Invalid trades fetch data line:"+tradeData,2);
			}

            if(newItem.price>0&&lastTradesDate<newItem.date)
            {
                lastTradesDate=newItem.date;
                IndicatorEngine::setValue(baseValues.exchangeName,baseValues.currentPair.symbol,"Last",newItem.price);
                lastTickerLast=newItem.price;
            }
            if(newTradesItems->count())emit addLastTrades(baseValues.currentPair.symbol,newTradesItems);
			else delete newTradesItems;
        }
        else if(!data.startsWith("{\"status\": \"ok\", \"data\": {\"symbol\": \""))success=false;
		break;//trades
	case 111: //depth
        if(data.startsWith("{\"symbol\": \""+baseValues.currentPair.symbol.toLower().toLatin1()+"\", \"bids\": ["))
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
                    QStringList currentPair=asksList.at(n).split(", ");
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
                    QStringList currentPair=bidsList.at(n).split(", ");
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
        else if(!data.startsWith("{\"symbol\": \""))
        {
            if(debugLevel)logThread->writeLog("Invalid depth data:"+data,2);
            success=false;
        }
		break;
	case 202: //info
        if(data.startsWith("{\"status\": \"ok\", \"data\": {\""))
        {
            QByteArray fundsData=getMidData("data\": {","}",&data);
            QByteArray btcBalance=getMidData(baseValues.currentPair.currAStrLow+"\": \"","\"",&fundsData);
			if(!btcBalance.isEmpty())
			{
                double newBtcBalance=btcBalance.toDouble();
                if(lastBtcBalance!=newBtcBalance)emit accBtcBalanceChanged(baseValues.currentPair.symbol,newBtcBalance);
				lastBtcBalance=newBtcBalance;
			}

            QByteArray usdBalance=getMidData("\""+baseValues.currentPair.currBStrLow+"\": \"","\"",&fundsData);
			if(!usdBalance.isEmpty())
			{
                double newUsdBalance=usdBalance.toDouble();
                if(newUsdBalance!=lastUsdBalance)emit accUsdBalanceChanged(baseValues.currentPair.symbol,newUsdBalance);
				lastUsdBalance=newUsdBalance;
			}

            QByteArray fee=getMidData("\"fee\": ",",",&data);
            if(!fee.isEmpty())
            {
                double newFee=fee.toDouble();
                if(newFee!=lastFee)emit accFeeChanged(baseValues.currentPair.symbol,newFee);
                lastFee=newFee;
            }
        }
        else success=false;
		break;//info
	case 204://orders
        if(data.startsWith("{\"status\": \"ok\", \"data\": [{\""))
        {
            if(lastOrders!=data)
            {
                lastOrders=data;

                QStringList ordersList=QString(data).split("}, {");
                QList<OrderItem> *orders=new QList<OrderItem>;
                for(int n=0;n<ordersList.count();n++)
                {
                    OrderItem currentOrder;
                    QByteArray currentOrderData=ordersList.at(n).toLatin1()+"}";

                    currentOrder.oid=getMidData("id\": \"","\"}",&currentOrderData);
                    QByteArray tempDate=getMidData("issued\": ",", \"",&currentOrderData);
                    tempDate.chop(3);
                    currentOrder.date=tempDate.toUInt();
                    currentOrder.type=getMidData("type\": \"","\", \"",&currentOrderData)=="ask";
                    currentOrder.amount=getMidData("volume\": \"","\", \"",&currentOrderData).toDouble();
                    currentOrder.price=getMidData("limit\": \"","\", \"",&currentOrderData).toDouble();
                    currentOrder.symbol="BTC"+getMidData("currency\": \"","\", \"",&currentOrderData).toUpper();
                    currentOrder.status=1;

                    if(currentOrder.isValid())(*orders)<<currentOrder;
                }
                emit orderBookChanged(baseValues.currentPair.symbol,orders);
            }
        }
        else if(data.startsWith("{\"status\": \"ok\", \"data\": ["))
            emit ordersIsEmpty();
        else
            success=false;
        break;//orders
	case 305: //order/cancel
        if(data.startsWith("{\"status\": \"ok\", \"data\": [{\""))
        {
            QByteArray oid=getMidData("id\": \"","\"",&data);
            if(!oid.isEmpty())emit orderCanceled(baseValues.currentPair.symbol,oid);
		}
        else success=false;
		break;//order/cancel
    case 306: if(debugLevel)logThread->writeLog("Buy OK: "+data,2);break;//order/buy
    case 307: if(debugLevel)logThread->writeLog("Sell OK: "+data,2);break;//order/sell
    case 208: ///history
        if(data.startsWith("{\"status\": \"ok\", \"data\": {\"symbol\": \""+baseValues.currentPair.symbol.toLower().toLatin1()+"\", \"trades\": ["))
        {
            if(lastHistory!=data)
            {
                lastHistory=data;

                QStringList dataList=QString(data).split("}, {");
                QList<HistoryItem> *historyItems=new QList<HistoryItem>;
                quint32 currentId;
                quint32 maxId=0;
                for(int n=dataList.count()-1;n>=0;n--)
                {
                    QByteArray curLog=dataList.at(n).toLatin1()+"}";

                    currentId=getMidData("txid\": ",", \"",&curLog).toUInt();
                    if(currentId<=lastHistoryId)break;
                    if(n==dataList.count()-1)maxId=currentId;

                    HistoryItem currentHistoryItem;
                    QByteArray logType=getMidData("type\": \"","\"}",&curLog);
                    if(logType=="ask")currentHistoryItem.type=2;
                    else if(logType=="bid")currentHistoryItem.type=1;
				
                    if(currentHistoryItem.type)
                    {
                        currentHistoryItem.symbol=baseValues.currentPair.symbol;
                        currentHistoryItem.dateTimeInt=getMidData("ts\": ",", \"",&curLog).toUInt();
                        currentHistoryItem.price=getMidData("price\": ",", \"",&curLog).toDouble();
                        currentHistoryItem.volume=getMidData("amount\": ",", \"",&curLog).toDouble();
                        if(currentHistoryItem.isValid())(*historyItems)<<currentHistoryItem;
                    }
                }
                if(maxId>lastHistoryId)lastHistoryId=maxId;
                emit historyChanged(historyItems);
            }
        }
        else success=false;
		break;//money/wallet/history
	default: break;
	}

    static int authErrorCount=0;
    if(reqType>=200 && reqType<300)
    {
        if(!success)
        {
            authErrorCount++;
            if(authErrorCount>2)
            {
                QString authErrorString=getMidData("data\": \"","\"",&data);
                if(debugLevel)logThread->writeLog("API error: "+authErrorString.toLatin1()+" ReqType: "+QByteArray::number(reqType),2);

                if(authErrorString=="auth_error")authErrorString=julyTr("TRUNAUTHORIZED","Invalid API key.");
                else if(authErrorString=="nonce_error")authErrorString=julyTr("THIS_PROFILE_ALREADY_USED","Invalid nonce parameter.");
                if(!authErrorString.isEmpty())emit showErrorMessage(authErrorString);
            }
        }
        else authErrorCount=0;
    }

	static int errorCount=0;
	if(!success)
	{
        QString errorString;
        errorString=getMidData("{\"status\": \"error\", \"data\": \"","\"",&data);

        errorCount++;
		if(errorCount<3)return;
		if(debugLevel)logThread->writeLog("API error: "+errorString.toLatin1()+" ReqType:"+QByteArray::number(reqType),2);
		if(errorString.isEmpty())return;
		if(errorString==QLatin1String("no orders"))return;
		if(reqType<300)emit showErrorMessage("I:>"+errorString);
	}
	else errorCount=0;
}

void Exchange_BitCurex::depthUpdateOrder(QString symbol, double price, double amount, bool isAsk)
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

void Exchange_BitCurex::depthSubmitOrder(QString symbol, QMap<double,double> *currentMap ,double priceDouble, double amount, bool isAsk)
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

bool Exchange_BitCurex::isReplayPending(int reqType)
{
	if(julyHttp==0)return false;
	return julyHttp->isReqTypePending(reqType);
}

void Exchange_BitCurex::secondSlot()
{
    static int sendCounter=0;
    switch(sendCounter)
    {
    case 0:
        if(!isReplayPending(103))sendToApi(103,baseValues.currentPair.currBStrLow.toLatin1()+"/ticker",false);
        break;
    case 1:
        if(!isReplayPending(202))sendToApi(202,"balance/",true,"nonce="+QByteArray::number(++privateNonce));
        break;
    case 2:
        if(!isReplayPending(109))sendToApi(109,baseValues.currentPair.currBStrLow.toLatin1()+'/'+QByteArray::number(lastTradesDate)+"/trades",false);
        break;
    case 3:
        if(!tickerOnly&&!isReplayPending(204))sendToApi(204,"offers/",true,"nonce="+QByteArray::number(++privateNonce));
        break;
    case 4:
        if(isDepthEnabled()&&(forceDepthLoad||!isReplayPending(111)))
        {
            emit depthRequested();
            sendToApi(111,baseValues.currentPair.currBStrLow.toLatin1()+"/"+baseValues.depthCountLimitStr+"/orderbook",false);
            forceDepthLoad=false;
        }
        break;
    case 5:
        if(lastHistory.isEmpty())getHistory(false);
        break;
    default: break;
    }
    if(sendCounter++>=5)sendCounter=0;

	Exchange::secondSlot();
}

void Exchange_BitCurex::getHistory(bool force)
{
	if(tickerOnly)return;
	if(force)lastHistory.clear();
    if(!isReplayPending(208))sendToApi(208,"trades/"+baseValues.currentPair.currBStrLow.toLatin1()+"/"+QByteArray::number(lastHistoryId)+"/",true,"market="+baseValues.currentPair.currBStrLow.toLatin1()+"&nonce="+QByteArray::number(++privateNonce)+"&txid="+QByteArray::number(lastHistoryId));
}

void Exchange_BitCurex::buy(QString symbol, double apiBtcToBuy, double apiPriceToBuy)
{
	if(tickerOnly)return;

    CurrencyPairItem pairItem;
    pairItem=baseValues.currencyPairMap.value(symbol,pairItem);
    if(pairItem.symbol.isEmpty())return;

    QByteArray hashData="limit="     +byteArrayFromDouble(apiPriceToBuy,pairItem.priceDecimals,0)+"&market="      +baseValues.currentPair.currBStrLow.toLatin1()+"&nonce="      +QByteArray::number(++privateNonce)+"&offer_type=bid&volume="            +byteArrayFromDouble(apiBtcToBuy,pairItem.currADecimals,0);
    QByteArray postData="{\"limit\":"+byteArrayFromDouble(apiPriceToBuy,pairItem.priceDecimals,0)+",\"market\":\""+baseValues.currentPair.currBStrLow.toLatin1()+"\",\"nonce\":"+QByteArray::number(privateNonce)  +",\"offer_type\":\"bid\",\"volume\":"+byteArrayFromDouble(apiBtcToBuy,pairItem.currADecimals,0)+"}";

    if(debugLevel)logThread->writeLog("Buy: "+hashData,2);
    sendToApi(306,"offer/"+baseValues.currentPair.currBStrLow.toLatin1()+'/',true,hashData,postData);
}

void Exchange_BitCurex::sell(QString symbol, double apiBtcToSell, double apiPriceToSell)
{
	if(tickerOnly)return;

    CurrencyPairItem pairItem;
    pairItem=baseValues.currencyPairMap.value(symbol,pairItem);
    if(pairItem.symbol.isEmpty())return;

    QByteArray hashData="limit="     +byteArrayFromDouble(apiPriceToSell,pairItem.priceDecimals,0)+"&market="      +baseValues.currentPair.currBStrLow.toLatin1()+"&nonce="      +QByteArray::number(++privateNonce)+"&offer_type=ask&volume="            +byteArrayFromDouble(apiBtcToSell,pairItem.currADecimals,0);
    QByteArray postData="{\"limit\":"+byteArrayFromDouble(apiPriceToSell,pairItem.priceDecimals,0)+",\"market\":\""+baseValues.currentPair.currBStrLow.toLatin1()+"\",\"nonce\":"+QByteArray::number(privateNonce)  +",\"offer_type\":\"ask\",\"volume\":"+byteArrayFromDouble(apiBtcToSell,pairItem.currADecimals,0)+"}";

    if(debugLevel)logThread->writeLog("Sell: "+hashData,2);
    sendToApi(307,"offer/"+baseValues.currentPair.currBStrLow.toLatin1()+'/',true,hashData,postData);
}

void Exchange_BitCurex::cancelOrder(QString,QByteArray order)
{
	if(tickerOnly)return;

    QByteArray hashData="nonce="+QByteArray::number(++privateNonce)+"&offer_id="+order;
    order.prepend("{\"offer_id\":\"");
    order.append("\",\"nonce\":"+QByteArray::number(privateNonce)+"}");
	if(debugLevel)logThread->writeLog("Cancel order: "+order,2);
    sendToApi(305,"offer/del/",true,hashData,order);
}

void Exchange_BitCurex::sendToApi(int reqType, QByteArray method, bool auth, QByteArray hashData, QByteArray postData)
{
	if(julyHttp==0)
	{ 
        julyHttp=new JulyHttp("api.bitcurex.com","Key: "+getApiKey()+"\r\n",this,true);
		connect(julyHttp,SIGNAL(anyDataReceived()),baseValues_->mainWindow_,SLOT(anyDataReceived()));
		connect(julyHttp,SIGNAL(apiDown(bool)),baseValues_->mainWindow_,SLOT(setApiDown(bool)));
		connect(julyHttp,SIGNAL(setDataPending(bool)),baseValues_->mainWindow_,SLOT(setDataPending(bool)));
		connect(julyHttp,SIGNAL(errorSignal(QString)),baseValues_->mainWindow_,SLOT(showErrorMessage(QString)));
		connect(julyHttp,SIGNAL(sslErrorSignal(const QList<QSslError> &)),this,SLOT(sslErrors(const QList<QSslError> &)));
		connect(julyHttp,SIGNAL(dataReceived(QByteArray,int)),this,SLOT(dataReceivedAuth(QByteArray,int)));
	}

    if(auth)
    {
        if(postData.isEmpty())
        {
            QByteArray checkData=getApiKey()+'/'+hmacSha512(getApiSign(),hashData).toHex()+"/?nonce="+QByteArray::number(privateNonce);
            julyHttp->sendData(reqType, "GET /v2/"+method+checkData);
        }
        else
        {
            if(reqType==305)
                julyHttp->sendData(reqType, "DELETE /v2/"+method+getApiKey()+'/'+hmacSha512(getApiSign(),hashData).toHex(), postData);
            else
                julyHttp->sendData(reqType, "POST /v2/"+method+getApiKey()+'/'+hmacSha512(getApiSign(),hashData).toHex(), postData);
        }
	}
	else
    {
        julyHttp->sendData(reqType, "GET /v2/"+method);
	}
}

void Exchange_BitCurex::sslErrors(const QList<QSslError> &errors)
{
	QStringList errorList;
	for(int n=0;n<errors.count();n++)errorList<<errors.at(n).errorString();
	if(debugLevel)logThread->writeLog(errorList.join(" ").toLatin1(),2);
	emit showErrorMessage("SSL Error: "+errorList.join(" "));
}
