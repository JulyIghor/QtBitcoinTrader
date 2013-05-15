//Created by July IGHOR
//Feel free to contact me: julyighor@gmail.com
//Bitcoin Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc

#ifndef GSGTRADER_H
#define GSGTRADER_H

#include <QtGui/QDialog>
#include "ui_gsgtrader.h"
#include <QHttp>
#include <QSslSocket>
#include "socketthread.h"
#include <QHttp>
#include <QCloseEvent>

class BitcoinTrader : public QDialog
{
	Q_OBJECT

public:
	//QHash<doulbe,QByteArray> ruleLastPrice;
	//QHash<doulbe,QByteArray> ruleMarketBuyPrice;
	//QHash<doulbe,QByteArray> ruleMarketSellPrice;
	//QHash<doulbe,QByteArray> ruleMarketHighPrice;
	//QHash<doulbe,QByteArray> ruleMarketLowPrice;

	Ui::gsgTraderClass ui;
	bool confirmBuySell;
	
	QByteArray getMidData(QString a, QString b,QByteArray *data);
	QTimer *secondTimer;
	QTimer *updateCheckTimer;
	SocketThread *socketThreadAuth;
	BitcoinTrader();
	~BitcoinTrader();

private:
	bool apiDownState;
	void setApiDown(bool);
	void closeEvent(QCloseEvent *event);
	void reject(){};
	QString clearData(QString data);
	QHttp *httpUpdate;

	QString appDir;
	bool authErrorOnce;
	bool showingMessage;
	bool ordersLogLoaded;
	void beep();

	QString btcChar;
	void cancelOrderByOid(QByteArray);
	bool lastLagState;
	void setRowState(int row, int state);
	void setRowStateByText(int row, QByteArray text);

	double floatFee;
	double floatFeeDec;
	double floatFeeInc;

	bool firstPriceLoad;
	void checkValidSellButtons();
	void checkValidBuyButtons();
	bool sellLockBtcToSell;
	bool sellLockPricePerCoin;
	bool sellLockBtcPerOrder;
	bool sellLockAmountToReceive;

	bool buyLockTotalBtc;
	bool buyLockPricePerCoin;
	bool buyLockBtcPerOrder;
	bool buyLockTotalSpend;

	QMap<QByteArray,QString> oidMap;
	void insertIntoTable(QByteArray,QString);
	QDateTime lastUpdate;
	QTime updateLogTime;
public slots:
	void apiDownSlot();
	void setSslEnabled(bool);
	void calcButtonClicked();
	void checkUpdate();
	void httpUpdateDone(bool);

	//void messageReceived(const QString&);

	void saveSoundToggles();
	//void trafficChanged(quint64);
	void ruleAddButton();
	void ruleRemove();
	void ruleRemoveAll();

	void copyDonateButton();

	void accountUSDChanged(double);
	void accountBTCChanged(double);
	void marketLowChanged(double);
	void marketHighChanged(double);

	void balanceChanged(double);
	void updateLogTable();
	void ordersSelectionChanged();
	void mtgoxLagChanged(double);
	void ordersCancelSelected();
	void secondSlot();
	void dataReceivedAuth(QByteArray);
	void ordersCancelAll();
	void accountFeeChanged(double);

	void buyOrdersCountToDefault();
	void buyBtcToBuyChanged(double);
	void buyPricePerCoinChanged(double);
	void buyOrdersCountChanged(int);
	void buyBtcPerOrderChanged(double);
	void buyBtcToBuyAllIn();
	void buyBtcToBuyHalfIn();
	void buyPricePerCoinAsMarketPrice();
	void buyBitcoinsButton();
	void buyTotalToSpendInUsdChanged(double);

	void sellOrdersCountToDefault();
	void sellOrdersCountChanged(int);
	void sellBtcPerOrderChanged(double);
	void sellBitcoinButton();
	void sellAmountToReceiveChanged(double);
	void sellPricePerCoinInUsdChanged(double);
	void sellPricePerCoinAsMarketPrice();
	void sellTotalBtcToSellAllIn();
	void sellTotalBtcToSellHalfIn();
	void sellTotalBtcToSellChanged(double);
	void lastSoftLagChanged(double);

	void sellBuyAsMarketPrice();
	void sellBuyApply();
	void sellBuyButtonSellBuy();
	void sellBuyMidPriceChanged(double);

	void sellBuyDelta01();
	void sellBuyDelta02();
	void sellBuyDelta05();
};

#endif // GSGTRADER_H
