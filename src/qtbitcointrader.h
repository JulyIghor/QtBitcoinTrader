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

#ifndef QTBITCOINTRADER_H
#define QTBITCOINTRADER_H

#include <QMainWindow>

#include "currencypairitem.h"
#include "debugviewer.h"
#include "depthmodel.h"
#include "feecalculator.h"
#include "historymodel.h"
#include "orderitem.h"
#include "ordersmodel.h"
#include "percentpicker.h"
#include "script/rulewidget.h"
#include "script/scriptwidget.h"
#include "tradesmodel.h"
#include "ui_qtbitcointrader.h"
#include <QCloseEvent>
#include <QElapsedTimer>
#include <QKeyEvent>
#include <QMenu>
#include <QScrollArea>
#include <QSettings>
#include <QSortFilterProxyModel>
#include <QSystemTrayIcon>
#include <QTime>
#include <time.h>

class Exchange;
class QDockWidget;
class ConfigManager;
class ConfigManagerDialog;
class DockHost;
class NetworkMenu;
class CurrencyMenu;
class CurrencySignLoader;
class ChartsView;
class NewsView;
class TraderSpinBox;

struct GroupStateItem
{
    GroupStateItem(const QString& newName, bool newEnabled) : name(newName), enabled(newEnabled)
    {
        elapsed.restart();
    }
    QString name;
    bool enabled;
    QElapsedTimer elapsed;
};

class QtBitcoinTrader : public QMainWindow
{
    Q_OBJECT

public:
    Ui::QtBitcoinTraderClass ui;

    void addRuleByHolder(RuleHolder& holder, bool isEnabled, QString titleName);

    QStringList getRuleGroupsNames();
    QStringList getScriptGroupsNames();
    int getOpenOrdersCount(int all = 0);
    void fixTableViews(QWidget* wid);
    double getIndicatorValue(const QString&);
    QMap<QString, QDoubleSpinBox*> indicatorsMap;

    bool feeCalculatorSingleInstance;
    FeeCalculator* feeCalculator;

    double meridianPrice;
    double availableAmount;
    int exchangeId;
    double getAvailableBTC();
    double getAvailableUSD();
    double getAvailableUSDtoBTC(double price);

    double getFeeForUSDDec(double usd);

    double floatFee;
    double floatFeeDec;
    double floatFeeInc;

    void addPopupDialog(int);

    void setupClass();
    bool isValidSize(QSize* sizeV)
    {
        if (sizeV->width() < 3 || sizeV->width() > 2000 || sizeV->height() < 3 || sizeV->height() > 2000)
            return false;

        return true;
    }
    void reloadLanguage(const QString& preferedLangFile = "");
    void fixAllChildButtonsAndLabels(QWidget* par);
    void fixDecimals(QWidget* par);
    void fixAllCurrencyLabels(QWidget* par);
    void fillAllBtcLabels(QWidget* par, QString curName);
    void fillAllUsdLabels(QWidget* par, QString curName);

    QByteArray getMidData(const QString& a, QString b, QByteArray* data);
    QtBitcoinTrader();
    ~QtBitcoinTrader();

    OrdersModel* ordersModel;

    qint64 currencyChangedDate;

    QSettings* iniSettings;
    bool isValidSoftLag;
    void beep(bool noBlink = false);
    void play(const QString&, bool noBlink = false);
    void blinkWindow();

    bool confirmOpenOrder;
    void apiSellSend(const QString& symbol, double btc, double price);
    void apiBuySend(const QString& symbol, double btc, double price);

    QTime lastRuleExecutedTime;

    bool confirmExitApp();
    bool hasWorkingRules();
    bool executeConfirmExitDialog();

    QSortFilterProxyModel* ordersSortModel;
    bool currentlyAddingOrders;
    void keyPressEvent(QKeyEvent* event);
    void closeEvent(QCloseEvent* event);
    void changeEvent(QEvent* event);

    bool isDetachedLog;
    bool isDetachedTrades;
    bool isDetachedRules;
    bool isDetachedDepth;
    bool isDetachedCharts;

    void setColumnResizeMode(QTableView*, int, QHeaderView::ResizeMode);
    void setColumnResizeMode(QTableView*, QHeaderView::ResizeMode);

    void clearPendingGroup(const QString&);

    double getVolumeByPrice(const QString& symbol, double price, bool isAsk);
    double getPriceByVolume(const QString& symbol, double size, bool isAsk);

    void fixWidthComboBoxGroupByPrice();

    bool closeToTray;

    ChartsView* chartsView = nullptr;
    NewsView* newsView = nullptr;

    QScopedPointer<QTimer> secondTimer;

private:
    QList<GroupStateItem> pendingGroupStates;

    void setSpinValue(QDoubleSpinBox* spin, double val);
    void setSpinValueP(QDoubleSpinBox* spin, double& val);
    QWidget* windowWidget;
    QMenu copyTableValuesMenu;
    QTableView* lastCopyTable;

    void copyInfoFromTable(QTableView* table, QAbstractItemModel* model, int i);

    bool swapedDepth;
    DepthModel* depthAsksModel;
    DepthModel* depthBidsModel;
    TradesModel* tradesModel;
    HistoryModel* historyModel;
    void fixDepthBidsTable();
    void clearDepth();
    void calcOrdersTotalValues();
    void ruleTotalToBuyValueChanged();
    void ruleAmountToReceiveValueChanged();
    void ruleTotalToBuyBSValueChanged();
    void ruleAmountToReceiveBSValueChanged();
    bool isDataPending;
    QElapsedTimer softLagTime;
    QElapsedTimer depthLagTime;
    bool waitingDepthLag;

    QMenu* trayMenu;
    QString windowTitleP;
    QSystemTrayIcon* trayIcon;
    QString profileName;
    void makeRitchValue(QString* text);
    bool checkForUpdates;

    int lastLoadedCurrency;

    bool constructorFinished;
    void reject()
    {
    }
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

    bool profitSellThanBuyUnlocked;
    bool profitBuyThanSellUnlocked;
    bool profitBuyThanSellChangedUnlocked;
    bool profitSellThanBuyChangedUnlocked;

    DebugViewer* debugViewer;

    void checkIsTabWidgetVisible();

    void clearTimeOutedTrades();
    void depthSelectOrder(QModelIndex, bool isSel, int type = 0);
    double tradesPrecentLast;

    void repeatOrderFromTrades(int type, int row);
    void repeatOrderFromValues(int type, double price, double amount, bool availableOnly = true);
    void repeatSelectedOrderByType(int type, bool availableOnly = true);

    void updateTrafficTotalValue();
    void setCurrencyPairsList();

    qint16 currentPopupDialogs;
    NetworkMenu* networkMenu;
    CurrencyMenu* currencyMenu;
    QScopedPointer<CurrencySignLoader> currencySignLoader;

    QElapsedTimer historyForceUpdate;
    QElapsedTimer speedTestTime;
    QElapsedTimer lastMessageTime;

public slots:
    void sendIndicatorEvent(const QString& symbol, const QString& name, double value);

    void setRuleTabRunning(const QString&, bool);
    void startApplication(const QString&, QStringList);
    void setGroupRunning(const QString& name, bool enabled);
    void setGroupState(const QString& name, bool enabled);
    bool getIsGroupRunning(const QString& name);

    void reloadScripts();
    void on_buyPercentage_clicked();
    void on_sellPercentage_clicked();
    void on_buyPriceAsMarketBid_clicked();
    void on_sellPriceAsMarketAsk_clicked();
    void trafficTotalToZero_clicked();
    void on_buttonNight_clicked();
    void ordersFilterChanged();
    void cancelOrderByXButton();
    void cancelPairOrders(const QString&);
    void cancelAskOrders(const QString&);
    void cancelBidOrders(const QString&);

    void repeatBuySellOrder();
    void repeatBuyOrder();
    void repeatSellOrder();
    void copySelectedRow();
    void copyDate();
    void copyAmount();
    void copyPrice();
    void copyTotal();

    void tableCopyContextMenuRequested(QPoint);

    void on_rulesTabs_tabCloseRequested(int);
    void on_buttonAddRuleGroup_clicked();

    void availableAmountChanged(const QString&, double);
    void precentBidsChanged(double);
    void depthRequested();
    void depthRequestReceived();
    void on_swapDepth_clicked();
    void checkValidOrdersButtons();
    void cancelOrder(const QString&, const QByteArray&);
    void volumeAmountChanged(double, double);
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
    void depthFirstOrder(const QString&, double, double, bool);
    void depthSubmitOrders(const QString&, QList<DepthItem>*, QList<DepthItem>*);
    void showErrorMessage(const QString&);
    void saveAppState();
    void on_widgetStaysOnTop_toggled(bool);
    void setSoftLagValue(int);
    void trayActivated(QSystemTrayIcon::ActivationReason);
    void buttonMinimizeToTray();

    void secondSlot();
    void setTradesScrollBarValue(int);
    void tabTradesIndexChanged(int);
    void tabTradesScrollUp();
    void addLastTrades(const QString& symbol, QList<TradesItem>* newItems);

    void sayText(const QString&);

    void loginChanged(const QString&);

    void orderBookChanged(const QString&, QList<OrderItem>* orders);

    void setApiDown(bool);

    void identificationRequired(QString);

    void updateLogTable();
    void historyChanged(QList<HistoryItem>*);

    void accLastSellChanged(const QString&, double);
    void accLastBuyChanged(const QString&, double);

    void orderCanceled(const QString&, QByteArray);
    void ordersIsAvailable();
    void ordersIsEmpty();

    void languageChanged();
    void on_zeroSellThanBuyProfit_clicked();
    void on_zeroBuyThanSellProfit_clicked();
    void profitSellThanBuy();
    void profitSellThanBuyCalc();
    void profitBuyThanSellCalc();
    void profitBuyThanSell();

    void buttonNewWindow();

    void currencyMenuChanged(int);

    void on_calcButton_clicked();
    void checkUpdate();

    void accFeeChanged(const QString&, double);
    void accBtcBalanceChanged(const QString&, double);
    void accUsdBalanceChanged(const QString&, double);

    void indicatorHighChanged(const QString&, double);
    void indicatorLowChanged(const QString&, double);
    void indicatorSellChanged(const QString&, double);
    void indicatorLastChanged(const QString&, double);
    void indicatorBuyChanged(const QString&, double);
    void indicatorVolumeChanged(const QString&, double);

    void on_accountUSD_valueChanged(double);
    void on_accountBTC_valueChanged(double);
    void on_marketBid_valueChanged(double);
    void on_marketAsk_valueChanged(double);
    void on_marketLast_valueChanged(double);

    void balanceChanged(double);

    void on_ordersCancelBidsButton_clicked();
    void on_ordersCancelAsksButton_clicked();
    void on_ordersCancelSelected_clicked();
    void on_ordersCancelAllButton_clicked();
    void cancelAllCurrentPairOrders();
    void on_accountFee_valueChanged(double);

    void on_buyTotalBtcAllIn_clicked();
    void on_buyTotalBtcHalfIn_clicked();
    void on_buyPriceAsMarketAsk_clicked();
    void on_buyPriceAsMarketLastPrice_clicked();
    void buyBitcoinsButton();

    void sellBitcoinButton();
    void on_sellPriceAsMarketBid_clicked();
    void on_sellPricePerCoinAsMarketLastPrice_clicked();
    void on_sellTotalBtcAllIn_clicked();
    void on_sellTotalBtcHalfIn_clicked();
signals:
    void indicatorEventSignal(const QString& symbol, const QString& name, double value);
    void themeChanged();
    void reloadDepth();
    void cancelOrderByOid(const QString&, const QByteArray&);
    void apiSell(const QString& symbol, double btc, double price);
    void apiBuy(const QString& symbol, double btc, double price);
    void getHistory(bool);
    void clearValues();
    void clearCharts();
    void addBound(double, bool);
private slots:
    void uninstall();
    void on_buttonAddScript_clicked();
    void on_helpButton_clicked();
    void depthVisibilityChanged(bool);

private:
    void initDocks();
    void createActions();
    void createMenu();
    QDockWidget* createDock(QWidget* widget, const QString& title);
    void moveWidgetsToDocks();
    void translateTab(QWidget* tab);
    void lockLogo(bool lock);
    void initConfigMenu();
    void setupWidgets();

    QScopedPointer<QThread> currentExchangeThread;

private slots:
    void onActionSendBugReport();
    void onActionAbout();
    void onActionAboutQt();
    void onActionLockDocks(bool checked);
    void onActionConfigManager();
    void onActionSettings();
    void onActionDebug();
    void onMenuConfigTriggered();
    void onConfigChanged();
    void onConfigError(const QString& error);
    void exitApp();

private:
    bool lockedDocks;
    QAction* actionExit;
    QAction* actionUpdate;
    QAction* actionSendBugReport;
    QAction* actionAbout;
    QAction* actionAboutQt;
    QAction* actionLockDocks;
    QAction* actionConfigManager;
    QAction* actionSettings;
    QAction* actionDebug;
    QAction* actionUninstall;
    QMenu* menuFile;
    QMenu* menuView;
    QMenu* menuConfig;
    QMenu* menuHelp;
    ConfigManagerDialog* configDialog;
    DockHost* dockHost;
    QDockWidget* dockLogo;
    QDockWidget* dockDepth;

public:
    TraderSpinBox* buyTotalSpend;
    TraderSpinBox* buyPricePerCoin;
    TraderSpinBox* buyTotalBtc;
    TraderSpinBox* profitLossSpinBox;
    TraderSpinBox* profitLossSpinBoxPrec;
    TraderSpinBox* sellTotalBtc;
    TraderSpinBox* sellPricePerCoin;
    TraderSpinBox* sellAmountToReceive;
    TraderSpinBox* sellThanBuySpinBox;
    TraderSpinBox* sellThanBuySpinBoxPrec;

private slots:
    void buyTotalSpend_valueChanged(double);
    void buyPricePerCoin_valueChanged(double);
    void buyTotalBtc_valueChanged(double);
    void profitLossSpinBox_valueChanged(double);
    void profitLossSpinBoxPrec_valueChanged(double);
    void sellTotalBtc_valueChanged(double);
    void sellPricePerCoin_valueChanged(double);
    void sellAmountToReceive_valueChanged(double);
    void sellThanBuySpinBox_valueChanged(double);
    void sellThanBuySpinBoxPrec_valueChanged(double);
};

#endif // QTBITCOINTRADER_H
