// Copyright (C) 2013 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form https://sourceforge.net/projects/bitcointrader/
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#ifndef QTBITCOINTRADER_H
#define QTBITCOINTRADER_H

#include <QtGui/QDialog>
#include "ui_qtbitcointrader.h"
#include <QHttp>
#include <QCloseEvent>
#include "ruleholder.h"
#include <QSystemTrayIcon>
#include <QSettings>
#include <QMenu>
#include <QTime>

class QtBitcoinTrader : public QDialog
{
	Q_OBJECT

public:
	double ruleTotalToBuyValue;
	double ruleAmountToReceiveValue;
	double ruleTotalToBuyBSValue;
	double ruleAmountToReceiveBSValue;
	QString numFromDouble(const double &value);

	QString upArrow;
	QString downArrow;

	void addPopupDialog(int);

	void loadUiSettings();
	bool isValidSize(QSize *sizeV){if(sizeV->width()<3||sizeV->width()>2000||sizeV->height()<3||sizeV->height()>2000)return false; return true;}
	void reloadLanguageList(QString preferedLangFile="");
	void fixAllChildButtonsAndLabels(QWidget *par);
	void fixDecimals(QWidget *par);
	void fillAllBtcLabels(QWidget *par, QString curName);
	void fillAllUsdLabels(QWidget *par, QString curName);

	void checkAndExecuteRule(QList<RuleHolder> *ruleHolder, double price);

	Ui::QtBitcoinTraderClass ui;
	
	QByteArray getMidData(QString a, QString b,QByteArray *data);
	QtBitcoinTrader();
	~QtBitcoinTrader();

private:
	void calcOrdersTotalValues();
	void ruleTotalToBuyValueChanged();
	void ruleAmountToReceiveValueChanged();
	void ruleTotalToBuyBSValueChanged();
	void ruleAmountToReceiveBSValueChanged();
	bool isDataPending;
	int defaultSectionSize;
	QTime softLagTime;
	QTime depthLagTime;
	QMap<double,double> depthAsksMap;
	QMap<double,double> depthBidsMap;
	int depthAsksLastScrollValue;
	int depthBidsLastScrollValue;

	int depthCurrentAsksSyncIndex;
	int depthCurrentBidsSyncIndex;
	double depthAsksIncVolume;
	double depthBidsIncVolume;

	QMenu *trayMenu;
	QSettings *iniSettings;
	QRect currentDesktopRect;
	bool isValidSoftLag;
	void saveDetachedWindowsSettings(bool force=false);
	QString windowTitleP;
	QSystemTrayIcon *trayIcon;
	int exchangeId;
	QString profileName;
	void resizeEvent(QResizeEvent *);
	void makeRitchValue(QString *text);
	bool forcedReloadOrders;
	bool checkForUpdates;
	QList<RuleHolder> rulesLastPrice;
	QList<RuleHolder> rulesMarketBuyPrice;
	QList<RuleHolder> rulesMarketSellPrice;
	QList<RuleHolder> rulesMarketHighPrice;
	QList<RuleHolder> rulesMarketLowPrice;
	QList<RuleHolder> rulesOrdersLastBuyPrice;
	QList<RuleHolder> rulesOrdersLastSellPrice;
	QList<RuleHolder> rulesBtcBalance;
	QList<RuleHolder> rulesUsdBalance;
	QList<RuleHolder> rulesTotalToBuy;
	QList<RuleHolder> rulesAmountToReceive;
	QList<RuleHolder> rulesTotalToBuyBS;
	QList<RuleHolder> rulesAmountToReceiveBS;

	void addRuleByHolderToTable(RuleHolder);
	int lastLoadedCurrency;
	void postWorkAtTableItem(QTableWidgetItem *, int align=0);
	void checkAllRules();

	void removeRuleByGuid(uint guid);
	bool removeRuleByGuidInRuleHolderList(uint guid, QList<RuleHolder> *ruleHolderList);
	RuleHolder getRuleHolderByGuid(uint guid);
	bool fillHolderByFindedGuid(QList<RuleHolder>*holdersList,RuleHolder *holder, uint guid);
	void addRuleByHolderInToTable(RuleHolder holder, int preferedRow=-1);

	double lastMarketLowPrice;
	double lastMarketHighPrice;

	bool constructorFinished;
	void closeEvent(QCloseEvent *event);
	void reject(){};
	QString clearData(QString data);

	QString appDir;
	bool showingMessage;
	void beep();

	void setRuleStateBuGuid(quint64 guid, int state);
	void setRulesTableRowState(int row, int state);
	void setOrdersTableRowState(int row, int state);
	void setOrdersTableRowStateByText(int row, QString text);

	double floatFee;
	double floatFeeDec;
	double floatFeeInc;

	bool balanceNotLoaded;
	bool marketPricesNotLoaded;
	void checkValidSellButtons();
	void checkValidBuyButtons();
	bool sellLockBtcToSell;
	bool sellLockPricePerCoin;
	bool sellLockAmountToReceive;

	bool buyLockTotalBtc;
	bool buyLockPricePerCoin;
	bool buyLockTotalSpend;

	QMap<QByteArray,QString> oidMap;
	void insertIntoOrdersTable(QByteArray,QString);
	bool profitSellThanBuyUnlocked;
	bool profitBuyThanSellUnlocked;
	bool profitBuyThanSellChangedUnlocked;
	bool profitSellThanBuyChangedUnlocked;

	void translateUnicodeStr(QString *str);
	void cacheFirstRowGuid();
	uint firstRowGuid;

	bool eventFilter(QObject *obj, QEvent *event);

	void checkIsTabWidgetVisible();

	void clearTimeOutedTrades();
	bool isValidGeometry(QRect *geo, int yMargin=20);
	QRect rectInRect(QRect aRect, QSize bSize);
	void saveWindowState(QWidget *, QString name);
	void loadWindowState(QWidget *, QString name);
	bool isDetachedLog;
	bool isDetachedTrades;
	bool isDetachedRules;
	bool isDetachedDepth;
	bool isDetachedCharts;
public slots:
	void setDataPending(bool);
	void anyDataReceived();
	void depthUpdateOrder(double,double,bool);
	void showErrorMessage(QString);
	void exitApp();
	void setWindowStaysOnTop(bool);
	void setSoftLagValue(int);
	void trayActivated(QSystemTrayIcon::ActivationReason);
	void buttonMinimizeToTray();
	void tabLogOrdersOnTop(bool);
	void tabRulesOnTop(bool);
	void tabTradesOnTop(bool);
	void tabChartsOnTop(bool);
	void tabDepthOnTop(bool);

	void secondSlot();
	void setTradesScrollBarValue(int);
	void tabTradesIndexChanged(int);
	void tabTradesScrollUp();
	void addLastTrade(double, qint64, double, QByteArray, bool);

	void detachLog();
	void detachTrades();
	void detachRules();
	void detachCharts();
	void detachDepth();

	void attachLog();
	void attachTrades();
	void attachRules();
	void attachCharts();
	void attachDepth();

	void loginChanged(QString);

	void ordersChanged(QString);

	void setApiDown(bool);

	void identificationRequired(QString);

	void updateLogTable();
	void ordersLogChanged(QString);

	void accLastSellChanged(QByteArray,double);
	void accLastBuyChanged(QByteArray,double);

	void orderCanceled(QByteArray);
	void ordersIsEmpty();
	void firstTicker();
	void firstAccInfo();

	void fixWindowMinimumSize();
	void ruleUp();
	void ruleDown();

	void languageComboBoxChanged(int);

	void languageChanged();
	void zeroSellThanBuyProfit();
	void zeroBuyThanSellProfit();
	void profitSellThanBuy();
	void profitSellThanBuyChanged(double);
	void profitSellThanBuyPrecChanged(double);
	void profitSellThanBuyCalc();
	void profitBuyThanSellCalc();
	void profitBuyThanSell();
	void profitBuyThanSellChanged(double);
	void profitBuyThanSellPrecChanged(double);

	void buttonNewWindow();

	void checkValidRulesButtons();
	void aboutTranslationButton();

	void currencyChanged(int);

	void calcButtonClicked();
	void checkUpdate();

	void saveSoundToggles();
	void ruleAddButton();
	void ruleEditButton();
	void ruleRemove();
	void ruleRemoveAll();

	void copyDonateButton();

	void accountUSDChanged(double);
	void accountBTCChanged(double);
	void marketBuyChanged(double);
	void marketSellChanged(double);
	void marketLowChanged(double);
	void marketHighChanged(double);
	void marketLastChanged(double);
	void ordersLastBuyPriceChanged(double);
	void ordersLastSellPriceChanged(double);

	void balanceChanged(double);
	void ordersSelectionChanged();
	void mtgoxLagChanged(double);
	void ordersCancelSelected();

	void ordersCancelAll();
	void accountFeeChanged(double);

	void buyBtcToBuyChanged(double);
	void buyPricePerCoinChanged(double);
	void buyBtcToBuyAllIn();
	void buyBtcToBuyHalfIn();
	void buyPricePerCoinAsMarketPrice();
	void buyBitcoinsButton();
	void buyTotalToSpendInUsdChanged(double);

	void sellBitcoinButton();
	void sellAmountToReceiveChanged(double);
	void sellPricePerCoinInUsdChanged(double);
	void sellPricePerCoinAsMarketPrice();
	void sellTotalBtcToSellAllIn();
	void sellTotalBtcToSellHalfIn();
	void sellTotalBtcToSellChanged(double);
signals:
	void reloadOrders();
	void cancelOrderByOid(QByteArray);
	void apiSell(double btc, double price);
	void apiBuy(double btc, double price);
	void getHistory(bool);
	void quit();
	void clearValues();
};

#endif // QTBITCOINTRADER_H
