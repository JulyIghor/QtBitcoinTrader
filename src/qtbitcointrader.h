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
#include "depthmodel.h"
#include <QHttp>
#include <QCloseEvent>
#include <QSortFilterProxyModel>
#include "ruleholder.h"
#include <QSystemTrayIcon>
#include <QSettings>
#include <QMenu>
#include <QTime>
#include "tradesmodel.h"
#include "ordersmodel.h"
#include "orderitem.h"
#include "historymodel.h"
#include <QKeyEvent>
#include "currencypairitem.h"
#include "rulewidget.h"
#include "feecalculator.h"

class Exchange;

class QtBitcoinTrader : public QDialog
{
	Q_OBJECT

public:
	bool feeCalculatorSingleInstance;
	FeeCalculator *feeCalculator;

	double meridianPrice;
	double availableAmount;
	int exchangeId;
	double getAvailableBTC();
	double getAvailableUSD();
	double getAvailableUSDtoBTC(double price);

	double getFeeForUSDDec(double usd);
	double getValidDoubleForPercision(const double &val, const int &percision, bool roundUp=true);

	double floatFee;
	double floatFeeDec;
	double floatFeeInc;

	QString numFromDouble(const double &value, int maxDecimals=10);

	void addPopupDialog(int);

	void setupClass();
	bool isValidSize(QSize *sizeV){if(sizeV->width()<3||sizeV->width()>2000||sizeV->height()<3||sizeV->height()>2000)return false; return true;}
	void reloadLanguageList(QString preferedLangFile="");
	void fixAllChildButtonsAndLabels(QWidget *par);
	void fixDecimals(QWidget *par);
	void fillAllBtcLabels(QWidget *par, QString curName);
	void fillAllUsdLabels(QWidget *par, QString curName);

	void checkAndExecuteRule(int ruleType, double price);

	Ui::QtBitcoinTraderClass ui;
	
	QByteArray getMidData(QString a, QString b,QByteArray *data);
	QtBitcoinTrader();
	~QtBitcoinTrader();

	OrdersModel *ordersModel;

	quint32 currencyChangedDate;

	QSettings *iniSettings;
	bool isValidSoftLag;
	void beep();

	void apiSellSend(double btc, double price);
	void apiBuySend(double btc, double price);

	QTime lastRuleExecutedTime;

private:	
	QMenu copyTableValuesMenu;
	QTableView *lastCopyTable;

	void copyInfoFromTable(QTableView *table, QAbstractItemModel *model, int i);

	void addRuleGroupByName(QString groupName, QString copyFrom="");
	void keyPressEvent(QKeyEvent *event);
	bool swapedDepth;
	DepthModel *depthAsksModel;
	DepthModel *depthBidsModel;
	TradesModel *tradesModel;
	HistoryModel *historyModel;
	QSortFilterProxyModel *ordersSortModel;
	void clearDepth();
	void calcOrdersTotalValues();
	void ruleTotalToBuyValueChanged();
	void ruleAmountToReceiveValueChanged();
	void ruleTotalToBuyBSValueChanged();
	void ruleAmountToReceiveBSValueChanged();
	bool isDataPending;
	QTime softLagTime;
	QTime depthLagTime;
	bool waitingDepthLag;
	int depthAsksLastScrollValue;
	int depthBidsLastScrollValue;

	QMenu *trayMenu;
	QRect currentDesktopRect;
	void saveDetachedWindowsSettings(bool force=false);
	QString windowTitleP;
	QSystemTrayIcon *trayIcon;
	QString profileName;
	void resizeEvent(QResizeEvent *);
	void makeRitchValue(QString *text);
	bool checkForUpdates;

	void addRuleByHolderToTable(RuleHolder);
	int lastLoadedCurrency;
	void checkAllRules();

	double lastMarketLowPrice;
	double lastMarketHighPrice;

	bool constructorFinished;
	void closeEvent(QCloseEvent *event);
	void reject(){};
	QString clearData(QString data);

	QString appDir;
	bool showingMessage;

	bool balanceNotLoaded;
	bool marketPricesNotLoaded;
	void checkValidSellButtons();
	void checkValidBuyButtons();

	bool sellLockBtcToSell;
	bool sellLockPricePerCoin;
	bool sellLockAmountToReceive;

	bool buyLockTotalBtc;
	bool buyLockTotalBtcSelf;
	bool buyLockPricePerCoin;
	bool buyLockTotalSpend;

	bool profitSellThanBuyUnlocked;
	bool profitBuyThanSellUnlocked;
	bool profitBuyThanSellChangedUnlocked;
	bool profitSellThanBuyChangedUnlocked;

	void translateUnicodeStr(QString *str);

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
	void depthSelectOrder(QModelIndex, bool isSel, int type=0);
	double tradesPrecentLast;
	QList<CurrencyPairItem> currPairsList;

	void repeatOrderFromTrades(int type,int row);
	void repeatOrderFromValues(int type,double price, double amount);
	void repeatOrderByType(int type);

public slots:
	void on_delauBetweenExecutingRules_toggled(bool);

	void repeatBuySellOrder();
	void repeatBuyOrder();
	void repeatSellOrder();
	void copySelectedRow();
	void copyDate();
	void copyAmount();
	void copyPrice();
	void copyTotal();

	void tableCopyContextMenuRequested(QPoint);

	void rulesCountChanged();

	void on_rulesTabs_tabCloseRequested(int);
	void on_buttonAddRuleGroup_clicked();
	void setCurrencyPairsList(QList<CurrencyPairItem> *currPairs);

	void availableAmountChanged(double);
	void precentBidsChanged(double);
	void depthRequested();
	void depthRequestReceived();
	void on_swapDepth_clicked();
	void checkValidOrdersButtons();
	void cancelOrder(QByteArray);
	void volumeAmountChanged(double,double);
	void setLastTrades10MinVolume(double);
	void on_depthAutoResize_toggled(bool);
	void on_depthComboBoxLimitRows_currentIndexChanged(int);
	void on_comboBoxGroupByPrice_currentIndexChanged(int);
	void depthSelectSellOrder(QModelIndex);
	void depthSelectBuyOrder(QModelIndex);
	void historyDoubleClicked(QModelIndex);
	void tradesDoubleClicked(QModelIndex);
	void setDataPending(bool);
	void anyDataReceived();
	void depthFirstOrder(double,double,bool);
	void depthSubmitOrders(QList<DepthItem> *, QList<DepthItem> *);
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
	void addLastTrades(QList<TradesItem> *newItems);

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

	void ordersChanged(QList<OrderItem> *orders);

	void setApiDown(bool);

	void identificationRequired(QString);

	void updateLogTable();
	void historyChanged(QList<HistoryItem>*);

	void accLastSellChanged(QByteArray,double);
	void accLastBuyChanged(QByteArray,double);

	void orderCanceled(QByteArray);
	void ordersIsAvailable();
	void ordersIsEmpty();
	void firstTicker();

	void fixWindowMinimumSize();

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

	void aboutTranslationButton();

	void currencyChanged(int);

	void calcButtonClicked();
	void checkUpdate();

	void saveSoundToggles();
	
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
	void mtgoxLagChanged(double);
	void ordersCancelSelected();

	void ordersCancelAll();
	void accountFeeChanged(double);

	void buyBtcToBuyChanged(double);
	void buyPricePerCoinChanged(double);
	void buyBtcToBuyAllIn();
	void buyBtcToBuyHalfIn();
	void on_buyPriceAsMarketPrice_clicked();
	void on_buyPriceAsMarketLastPrice_clicked();
	void buyBitcoinsButton();
	void buyTotalToSpendInUsdChanged(double);

	void sellBitcoinButton();
	void sellAmountToReceiveChanged(double);
	void sellPricePerCoinInUsdChanged(double);
	void on_sellPricePerCoinAsMarketPrice_clicked();
	void on_sellPricePerCoinAsMarketLastPrice_clicked();
	void sellTotalBtcToSellAllIn();
	void sellTotalBtcToSellHalfIn();
	void sellTotalBtcToSellChanged(double);
signals:
	void reloadDepth();
	void cancelOrderByOid(QByteArray);
	void apiSell(double btc, double price);
	void apiBuy(double btc, double price);
	void getHistory(bool);
	void quit();
	void clearValues();
};

#endif // QTBITCOINTRADER_H
