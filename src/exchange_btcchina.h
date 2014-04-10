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

#ifndef EXCHANGE_BTCCHINA_H
#define EXCHANGE_BTCCHINA_H

#include "exchange.h"

class Exchange_BTCChina : public Exchange
{
	Q_OBJECT

public:
	Exchange_BTCChina(QByteArray pRestSign, QByteArray pRestKey);
	~Exchange_BTCChina();

private:
	bool isFirstTicker;
	bool isReplayPending(int);
	bool lastInfoReceived;
	bool tickerOnly;

	int apiDownCounter;
	int secondPart;

	JulyHttp *julyHttpAuth;
	JulyHttp *julyHttpPublic;

	QByteArray historyLastDate;
	QByteArray historyLastID;
	QByteArray lastFetchTid;
	QByteArray historyLastTradesRequest;
	QByteArray getMidData(QString a, QString b,QByteArray *data);
	QByteArray numForBuySellFromDouble(double val, int maxDecimals);

	QList<DepthItem> *depthAsks;
	QList<DepthItem> *depthBids;
	QList<QByteArray> cancelingOrderIDs;

	QMap<double,double> lastDepthAsksMap;
	QMap<double,double> lastDepthBidsMap;

	QString apiLogin;

	QTime authRequestTime;
	QTimer *secondTimer;

	void clearVariables();
	void depthSubmitOrder(QMap<double,double> *,double,double,bool);
	void depthUpdateOrder(double,double,bool);
	void sendToApi(int reqType, QByteArray method, bool auth=false, bool sendNow=true, QByteArray commands=0);
private slots:
	void reloadDepth();
	void sslErrors(const QList<QSslError> &);
	void dataReceivedAuth(QByteArray,int);
	void secondSlot();
public slots:
	void clearValues();
	void getHistory(bool);
	void buy(double, double);
	void sell(double, double);
	void cancelOrder(QByteArray);
};

#endif // EXCHANGE_BTCCHINA_H
