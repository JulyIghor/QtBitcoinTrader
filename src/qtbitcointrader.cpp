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

#include "aboutdialog.h"
#include "charts/chartsmodel.h"
#include "charts/chartsview.h"
#include "config/config_manager.h"
#include "config/config_manager_dialog.h"
#include "dock/dock_host.h"
#include "exchange/exchange.h"
#include "exchange/exchange_binance.h"
#include "exchange/exchange_bitfinex.h"
#include "exchange/exchange_bitstamp.h"
#include "exchange/exchange_bittrex.h"
#include "exchange/exchange_hitbtc.h"
#include "exchange/exchange_indacoin.h"
#include "exchange/exchange_poloniex.h"
#include "exchange/exchange_yobit.h"
#include "indicatorengine.h"
#include "iniengine.h"
#include "julylightchanges.h"
#include "julymath.h"
#include "julyscrolluponidle.h"
#include "julyspinboxfix.h"
#include "julyspinboxpicker.h"
#include "logobutton.h"
#include "main.h"
#include "menu/currencymenu.h"
#include "menu/networkmenu.h"
#include "news/newsview.h"
#include "orderstablecancelbutton.h"
#include "percentpicker.h"
#include "platform/sound.h"
#include "script/addrulegroup.h"
#include "script/addscriptwindow.h"
#include "script/rulescriptparser.h"
#include "script/scriptwidget.h"
#include "settings/settingsdialog.h"
#include "thisfeatureunderdevelopment.h"
#include "timesync.h"
#include "utils/currencysignloader.h"
#include "utils/traderspinbox.h"
#include "utils/utils.h"
#include <QClipboard>
#include <QDesktopServices>
#include <QDir>
#include <QDockWidget>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QProcess>
#include <QScreen>
#include <QScrollBar>
#include <QSettings>
#include <QStandardPaths>
#include <QSysInfo>
#include <QSystemTrayIcon>
#include <QTableWidget>
#include <QTimeLine>
#include <QUrl>
#include <QtCore/qmath.h>

#ifdef QT_TEXTTOSPEECH_LIB
#include <QTextToSpeech>
#endif

#ifdef Q_OS_WIN
#include <windows.h>
#ifdef SAPI_ENABLED
#include <sapi.h>
#endif
#endif

#include <QStyleFactory>

#ifdef Q_OS_MAC
#include <ApplicationServices/ApplicationServices.h>
#endif

static const int ContentMargin = 4;

QtBitcoinTrader::QtBitcoinTrader() :
    QMainWindow(),
    feeCalculator(nullptr),
    meridianPrice(0.0),
    availableAmount(0.0),
    exchangeId(-1),
    floatFee(0.0),
    floatFeeDec(1.0),
    floatFeeInc(1.0),
    ordersModel(nullptr),
    currencyChangedDate(0),
    iniSettings(new QSettings(baseValues.iniFileName, QSettings::IniFormat, this)),
    isValidSoftLag(false),
    confirmOpenOrder(false),
    lastRuleExecutedTime(QTime(1, 0, 0, 0)),
    secondTimer(new QTimer),

    windowWidget(this),
    lastCopyTable(nullptr),
    swapedDepth(false),
    depthAsksModel(nullptr),
    depthBidsModel(nullptr),
    tradesModel(nullptr),
    historyModel(nullptr),
    isDataPending(false),
    waitingDepthLag(false),
    trayMenu(nullptr),
    trayIcon(nullptr),
    checkForUpdates(true),
    lastLoadedCurrency(-1),
    constructorFinished(false),
    appDir(QApplication::applicationDirPath() + "/"),

    showingMessage(false),
    balanceNotLoaded(true),
    marketPricesNotLoaded(true),
    sellLockBtcToSell(false),
    sellLockPricePerCoin(false),
    sellLockAmountToReceive(false),
    buyLockTotalBtc(false),
    buyLockTotalBtcSelf(false),
    buyLockPricePerCoin(false),
    profitSellThanBuyUnlocked(true),
    profitBuyThanSellUnlocked(true),
    profitBuyThanSellChangedUnlocked(true),
    profitSellThanBuyChangedUnlocked(true),
    debugViewer(nullptr),
    tradesPrecentLast(0.0),
    currentPopupDialogs(0),
    networkMenu(nullptr),
    currencyMenu(nullptr),
    currencySignLoader(new CurrencySignLoader),
    lockedDocks(false),
    actionExit(nullptr),
    actionUpdate(nullptr),
    actionSendBugReport(nullptr),
    actionAbout(nullptr),
    actionAboutQt(nullptr),
    actionLockDocks(nullptr),
    actionConfigManager(nullptr),
    actionSettings(nullptr),
    actionDebug(nullptr),
    actionUninstall(nullptr),
    menuFile(nullptr),
    menuView(nullptr),
    menuConfig(nullptr),
    menuHelp(nullptr),
    configDialog(nullptr),
    dockHost(new DockHost(this)),
    dockLogo(nullptr),
    dockDepth(nullptr),
    buyTotalSpend(new TraderSpinBox(this)),
    buyPricePerCoin(new TraderSpinBox(this)),
    buyTotalBtc(new TraderSpinBox(this)),
    profitLossSpinBox(new TraderSpinBox(this)),
    profitLossSpinBoxPrec(new TraderSpinBox(this)),
    sellTotalBtc(new TraderSpinBox(this)),
    sellPricePerCoin(new TraderSpinBox(this)),
    sellAmountToReceive(new TraderSpinBox(this)),
    sellThanBuySpinBox(new TraderSpinBox(this)),
    sellThanBuySpinBoxPrec(new TraderSpinBox(this))
{
    historyForceUpdate.start();
    speedTestTime.start();
    lastMessageTime.start();
    depthLagTime.restart();
    softLagTime.restart();

    ui.setupUi(this);
    setupWidgets();
    fixAllCurrencyLabels(this);
    setSpinValue(ui.accountFee, 0.0);
    ui.accountLoginLabel->setStyleSheet("background: " + baseValues.appTheme.white.name());
    ui.noOpenedOrdersLabel->setStyleSheet("font-size:27px; border: 1px solid gray; background: " + baseValues.appTheme.white.name() +
                                          "; color: " + baseValues.appTheme.gray.name());
    ui.rulesNoMessage->setStyleSheet("font-size:27px; border: 1px solid gray; background: " + baseValues.appTheme.white.name() +
                                     "; color: " + baseValues.appTheme.gray.name());
    ui.ordersTableFrame->setVisible(false);

    currentlyAddingOrders = false;
    ordersModel = new OrdersModel;
    ordersSortModel = new QSortFilterProxyModel;
    ordersSortModel->setSortRole(Qt::EditRole);
    ordersSortModel->setFilterRole(Qt::WhatsThisRole);
    ordersSortModel->setDynamicSortFilter(true);
    ordersSortModel->setSourceModel(ordersModel);
    ui.ordersTable->setModel(ordersSortModel);
    setColumnResizeMode(ui.ordersTable, 0, QHeaderView::ResizeToContents);
    setColumnResizeMode(ui.ordersTable, 1, QHeaderView::Stretch);
    setColumnResizeMode(ui.ordersTable, 2, QHeaderView::ResizeToContents);
    setColumnResizeMode(ui.ordersTable, 3, QHeaderView::ResizeToContents);
    setColumnResizeMode(ui.ordersTable, 4, QHeaderView::ResizeToContents);
    setColumnResizeMode(ui.ordersTable, 5, QHeaderView::ResizeToContents);
    setColumnResizeMode(ui.ordersTable, 6, QHeaderView::ResizeToContents);
    setColumnResizeMode(ui.ordersTable, 7, QHeaderView::ResizeToContents);
    ui.ordersTable->setItemDelegateForColumn(7, new OrdersTableCancelButton(ui.ordersTable));

    ui.ordersTable->setSortingEnabled(true);
    ui.ordersTable->sortByColumn(0, Qt::AscendingOrder);

    connect(ordersModel, &OrdersModel::ordersIsAvailable, this, &QtBitcoinTrader::ordersIsAvailable);
    connect(ordersModel, &OrdersModel::cancelOrder, this, &QtBitcoinTrader::cancelOrder);
    connect(ordersModel, &OrdersModel::volumeAmountChanged, this, &QtBitcoinTrader::volumeAmountChanged);
    connect(ui.ordersTable->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QtBitcoinTrader::checkValidOrdersButtons);

    tradesModel = new TradesModel;
    ui.tableTrades->setModel(tradesModel);
    setColumnResizeMode(ui.tableTrades, 0, QHeaderView::Stretch);
    setColumnResizeMode(ui.tableTrades, 1, QHeaderView::ResizeToContents);
    setColumnResizeMode(ui.tableTrades, 2, QHeaderView::ResizeToContents);
    setColumnResizeMode(ui.tableTrades, 3, QHeaderView::ResizeToContents);
    setColumnResizeMode(ui.tableTrades, 4, QHeaderView::ResizeToContents);
    setColumnResizeMode(ui.tableTrades, 5, QHeaderView::ResizeToContents);
    setColumnResizeMode(ui.tableTrades, 6, QHeaderView::ResizeToContents);
    setColumnResizeMode(ui.tableTrades, 7, QHeaderView::Stretch);
    connect(tradesModel, &TradesModel::trades10MinVolumeChanged, this, &QtBitcoinTrader::setLastTrades10MinVolume);
    connect(tradesModel, &TradesModel::precentBidsChanged, this, &QtBitcoinTrader::precentBidsChanged);

    historyModel = new HistoryModel;
    ui.tableHistory->setModel(historyModel);
    setColumnResizeMode(ui.tableHistory, 0, QHeaderView::Stretch);
    setColumnResizeMode(ui.tableHistory, 1, QHeaderView::ResizeToContents);
    setColumnResizeMode(ui.tableHistory, 2, QHeaderView::ResizeToContents);
    setColumnResizeMode(ui.tableHistory, 3, QHeaderView::ResizeToContents);
    setColumnResizeMode(ui.tableHistory, 4, QHeaderView::ResizeToContents);
    setColumnResizeMode(ui.tableHistory, 5, QHeaderView::ResizeToContents);
    setColumnResizeMode(ui.tableHistory, 6, QHeaderView::Stretch);
    connect(historyModel, &HistoryModel::accLastSellChanged, this, &QtBitcoinTrader::accLastSellChanged);
    connect(historyModel, &HistoryModel::accLastBuyChanged, this, &QtBitcoinTrader::accLastBuyChanged);

    depthAsksModel = new DepthModel(ui.comboBoxGroupByPrice, true);
    ui.depthAsksTable->setModel(depthAsksModel);
    depthBidsModel = new DepthModel(ui.comboBoxGroupByPrice, false);
    ui.depthBidsTable->setModel(depthBidsModel);

    new JulyScrollUpOnIdle(ui.depthAsksTable->verticalScrollBar());
    new JulyScrollUpOnIdle(ui.depthBidsTable->verticalScrollBar());

    setColumnResizeMode(ui.depthAsksTable, 0, QHeaderView::Stretch);
    setColumnResizeMode(ui.depthAsksTable, 1, QHeaderView::ResizeToContents);
    setColumnResizeMode(ui.depthAsksTable, 2, QHeaderView::ResizeToContents);
    setColumnResizeMode(ui.depthAsksTable, 3, QHeaderView::ResizeToContents);
    setColumnResizeMode(ui.depthAsksTable, 4, QHeaderView::ResizeToContents);
    ui.depthAsksTable->horizontalHeader()->setMinimumSectionSize(0);

    setColumnResizeMode(ui.depthBidsTable, 0, QHeaderView::ResizeToContents);
    setColumnResizeMode(ui.depthBidsTable, 1, QHeaderView::ResizeToContents);
    setColumnResizeMode(ui.depthBidsTable, 2, QHeaderView::ResizeToContents);
    setColumnResizeMode(ui.depthBidsTable, 3, QHeaderView::ResizeToContents);
    setColumnResizeMode(ui.depthBidsTable, 4, QHeaderView::Stretch);
    ui.depthBidsTable->horizontalHeader()->setMinimumSectionSize(0);

    closeToTray = iniSettings->value("UI/CloseToTray", false).toBool();
#ifdef Q_OS_MAC
    closeToTray = false;
#endif

    confirmOpenOrder = iniSettings->value("UI/ConfirmOpenOrder", true).toBool();
    iniSettings->setValue("UI/ConfirmOpenOrder", confirmOpenOrder);

    checkValidOrdersButtons();

    new JulyLightChanges(ui.accountFee);
    new JulyLightChanges(ui.marketVolume);
    new JulyLightChanges(ui.marketBid);
    new JulyLightChanges(ui.marketAsk);
    new JulyLightChanges(ui.marketHigh);
    new JulyLightChanges(ui.marketLow);
    new JulyLightChanges(ui.marketLast);
    new JulyLightChanges(ui.accountBTC);
    new JulyLightChanges(ui.accountUSD);
    new JulyLightChanges(ui.ordersLastSellPrice);
    new JulyLightChanges(ui.ordersLastBuyPrice);
    new JulyLightChanges(ui.tradesVolume5m);

    new JulyLightChanges(ui.ruleTotalToBuyValue);
    new JulyLightChanges(ui.ruleAmountToReceiveValue);
    new JulyLightChanges(ui.ruleTotalToBuyBSValue);
    new JulyLightChanges(ui.ruleAmountToReceiveBSValue);
    new JulyLightChanges(ui.tradesBidsPrecent);

    baseValues.forceDotInSpinBoxes = iniSettings->value("UI/ForceDotInDouble", true).toBool();
    iniSettings->setValue("UI/ForceDotInDouble", baseValues.forceDotInSpinBoxes);

    ui.totalToSpendLayout->addWidget(new JulySpinBoxPicker(buyTotalSpend));
    ui.pricePerCoinLayout->addWidget(new JulySpinBoxPicker(buyPricePerCoin));
    ui.totalBtcToBuyLayout->addWidget(new JulySpinBoxPicker(buyTotalBtc));

    ui.totalToSellLayout->addWidget(new JulySpinBoxPicker(sellTotalBtc));
    ui.pricePerCoinSellLayout->addWidget(new JulySpinBoxPicker(sellPricePerCoin));
    ui.amountToReceiveLayout->addWidget(new JulySpinBoxPicker(sellAmountToReceive));

    ui.gssoProfitLayout->addWidget(new JulySpinBoxPicker(profitLossSpinBox));
    ui.gssoProfitPercLayout->addWidget(new JulySpinBoxPicker(profitLossSpinBoxPrec));

    ui.gssboProfitLayout->addWidget(new JulySpinBoxPicker(sellThanBuySpinBox));
    ui.gsboProfitPercLayout->addWidget(new JulySpinBoxPicker(sellThanBuySpinBoxPrec));

    for (QDoubleSpinBox* spinBox : findChildren<QDoubleSpinBox*>())
    {
        new JulySpinBoxFix(spinBox);

        QString scriptName = spinBox->whatsThis();

        if (scriptName.isEmpty())
            continue;

        indicatorsMap[scriptName] = spinBox;
    }

    double iniFileVersion = iniSettings->value("Profile/Version", 1.0).toDouble();

    if (iniFileVersion < baseValues.appVerReal)
        iniSettings->setValue("Profile/Version", baseValues.appVerReal);

    QSettings settingsMain(appDataDir + "/QtBitcoinTrader.cfg", QSettings::IniFormat);
    checkForUpdates = settingsMain.value("CheckForUpdates", true).toBool();

    int defTextHeight = baseValues.fontMetrics_->boundingRect("0123456789").height();
    defaultHeightForRow = settingsMain.value("RowHeight", defTextHeight * 1.6).toInt();

    if (defaultHeightForRow < defTextHeight)
        defaultHeightForRow = defTextHeight;

    settingsMain.setValue("RowHeight", defaultHeightForRow);

    exchangeId = iniSettings->value("Profile/ExchangeId", 0).toInt();

    if (exchangeId == 11)
    {
        ui.depthComboBoxLimitRows->blockSignals(true);
        ui.depthComboBoxLimitRows->clear();
        ui.depthComboBoxLimitRows->addItems({"1000", "500", "100", "50", "20", "10", "5"});
        ui.depthComboBoxLimitRows->blockSignals(false);
    }

    baseValues.depthCountLimit = iniSettings->value("UI/DepthCountLimit", 100).toInt();

    if (baseValues.depthCountLimit < 0)
        baseValues.depthCountLimit = 100;

    baseValues.depthCountLimitStr = QByteArray::number(baseValues.depthCountLimit);
    int currentDepthComboBoxLimitIndex = 0;

    for (int n = 0; n < ui.depthComboBoxLimitRows->count(); n++)
    {
        int currentValueDouble = ui.depthComboBoxLimitRows->itemText(n).toInt();

        if (currentValueDouble == baseValues.depthCountLimit)
            currentDepthComboBoxLimitIndex = n;

        ui.depthComboBoxLimitRows->setItemData(n, currentValueDouble, Qt::UserRole);
    }

    ui.depthComboBoxLimitRows->setCurrentIndex(currentDepthComboBoxLimitIndex);

    baseValues.apiDownCount = iniSettings->value("Network/ApiDownCounterMax", 5).toInt();

    if (baseValues.apiDownCount < 0)
        baseValues.apiDownCount = 5;

    iniSettings->setValue("Network/ApiDownCounterMax", baseValues.apiDownCount);

    baseValues.httpRetryCount = iniSettings->value("Network/HttpRetryCount", 8).toInt();

    if (baseValues.httpRetryCount < 1 || baseValues.httpRetryCount > 50)
        baseValues.httpRetryCount = 8;

    iniSettings->setValue("Network/HttpRetryCount", baseValues.httpRetryCount);

    baseValues.uiUpdateInterval = iniSettings->value("UI/UiUpdateInterval", 100).toInt();

    if (baseValues.uiUpdateInterval < 1)
        baseValues.uiUpdateInterval = 100;

    baseValues.customUserAgent = iniSettings->value("Network/UserAgent", "").toString();
    baseValues.customCookies = iniSettings->value("Network/Cookies", "").toString();

    baseValues.gzipEnabled = iniSettings->value("Network/GZIPEnabled", true).toBool();
    iniSettings->setValue("Network/GZIPEnabled", baseValues.gzipEnabled);

    feeCalculatorSingleInstance = iniSettings->value("UI/FeeCalcSingleInstance", true).toBool();
    iniSettings->setValue("UI/FeeCalcSingleInstance", feeCalculatorSingleInstance);

    ui.depthAutoResize->setChecked(iniSettings->value("UI/DepthAutoResizeColumns", true).toBool());

    if (!ui.depthAutoResize->isChecked())
    {
        ui.depthAsksTable->setColumnWidth(1, iniSettings->value("UI/DepthColumnAsksSizeWidth", ui.depthAsksTable->columnWidth(1)).toInt());
        ui.depthAsksTable->setColumnWidth(2,
                                          iniSettings->value("UI/DepthColumnAsksVolumeWidth", ui.depthAsksTable->columnWidth(2)).toInt());
        ui.depthAsksTable->setColumnWidth(3, iniSettings->value("UI/DepthColumnAsksPriceWidth", ui.depthAsksTable->columnWidth(3)).toInt());

        ui.depthBidsTable->setColumnWidth(0, iniSettings->value("UI/DepthColumnBidsPriceWidth", ui.depthBidsTable->columnWidth(0)).toInt());
        ui.depthBidsTable->setColumnWidth(1,
                                          iniSettings->value("UI/DepthColumnBidsVolumeWidth", ui.depthBidsTable->columnWidth(1)).toInt());
        ui.depthBidsTable->setColumnWidth(2, iniSettings->value("UI/DepthColumnBidsSizeWidth", ui.depthBidsTable->columnWidth(2)).toInt());
    }

    ui.rulesTabs->setVisible(false);

    iniSettings->setValue("Network/HttpRetryCount", baseValues.httpRetryCount);
    iniSettings->setValue("UI/UiUpdateInterval", baseValues.uiUpdateInterval);
    iniSettings->setValue("UI/DepthAutoResizeColumns", ui.depthAutoResize->isChecked());

    profileName = iniSettings->value("Profile/Name", "Default Profile").toString();
    windowTitleP = profileName + " - " + windowTitle() + " v" + baseValues.appVerStr;

#ifdef QTBUILDTARGETWIN32
    windowTitleP += " (32bit)";
#endif

#ifdef QTBUILDTARGETWIN64
    windowTitleP += " (64bit)";
#endif

    if (debugLevel)
        windowTitleP.append(" [DEBUG MODE]");

    // else if (baseValues.appVerIsBeta)
    //     windowTitleP.append(" [BETA]");

    windowWidget->setWindowTitle(windowTitleP);

    fixTableViews(this);

    int defaultMinWidth = qMax(1024, minimumSizeHint().width());
    int defaultMinHeight = qMax(720, minimumSizeHint().height());

    baseValues.highResolutionDisplay = false;
    int screenCount = QApplication::screens().count();

    QRect currScrRect;

    for (int n = 0; n < screenCount; n++)
    {
        currScrRect = QApplication::screens().at(n)->availableGeometry();

        if (currScrRect.width() > defaultMinWidth && currScrRect.height() > defaultMinHeight)
        {
            baseValues.highResolutionDisplay = true;
            break;
        }
    }

    setSpinValue(ui.accountBTC, 0.0);
    setSpinValue(ui.accountUSD, 0.0);
    setSpinValue(ui.marketBid, 0.0);
    setSpinValue(ui.marketAsk, 0.0);
    setSpinValue(ui.marketHigh, 0.0);
    setSpinValue(ui.marketLow, 0.0);
    setSpinValue(ui.marketLast, 0.0);
    setSpinValue(ui.marketVolume, 0.0);

    if (iniSettings->value("UI/SwapDepth", false).toBool())
        on_swapDepth_clicked();

    ui.depthLag->setValue(0.0);

    copyTableValuesMenu.addAction("Copy selected Rows", this, &QtBitcoinTrader::copySelectedRow);
    copyTableValuesMenu.addSeparator();
    copyTableValuesMenu.addAction("Copy Date", this, &QtBitcoinTrader::copyDate);
    copyTableValuesMenu.addAction("Copy Amount", this, &QtBitcoinTrader::copyAmount);
    copyTableValuesMenu.addAction("Copy Price", this, &QtBitcoinTrader::copyPrice);
    copyTableValuesMenu.addAction("Copy Total", this, &QtBitcoinTrader::copyTotal);
    copyTableValuesMenu.addSeparator();
    copyTableValuesMenu.addAction("Repeat Buy and Sell order", this, &QtBitcoinTrader::repeatBuySellOrder);
    copyTableValuesMenu.addAction("Repeat Buy order", this, &QtBitcoinTrader::repeatBuyOrder);
    copyTableValuesMenu.addAction("Repeat Sell order", this, &QtBitcoinTrader::repeatSellOrder);
    copyTableValuesMenu.addSeparator();
    copyTableValuesMenu.addAction("Cancel Order", this, &QtBitcoinTrader::on_ordersCancelSelected_clicked);
    copyTableValuesMenu.addAction("Cancel All Orders", this, &QtBitcoinTrader::on_ordersCancelAllButton_clicked);

    createActions();
    createMenu();

    networkMenu = new NetworkMenu(ui.networkMenuTool);

    currencyMenu = new CurrencyMenu(ui.currencyMenuTool);
    connect(currencyMenu, &CurrencyMenu::currencyMenuChanged, this, &QtBitcoinTrader::currencyMenuChanged);

    reloadLanguage();

    volumeAmountChanged(0.0, 0.0);

    connect(&julyTranslator, &JulyTranslator::languageChanged, this, &QtBitcoinTrader::languageChanged);

    if (checkForUpdates)
        QProcess::startDetached(QApplication::applicationFilePath(), QStringList("/checkupdate"));

    connect(networkMenu, &NetworkMenu::trafficTotalToZero_clicked, this, &QtBitcoinTrader::trafficTotalToZero_clicked);
    iniSettings->sync();

    chartsView = new ChartsView;
    connect(tradesModel, &TradesModel::addChartsTrades, chartsView->chartsModel.data(), &ChartsModel::addLastTrades);
    connect(this, &QtBitcoinTrader::clearCharts, chartsView->chartsModel.data(), &ChartsModel::clearCharts);
    connect(this, &QtBitcoinTrader::addBound, chartsView->chartsModel.data(), &ChartsModel::addBound);
    ui.chartsLayout->addWidget(chartsView);

    newsView = new NewsView();
    ui.newsLayout->addWidget(newsView);

    // ChatWindow *chatWindow=new ChatWindow();
    // ui.chatLayout->addWidget(chatWindow);

    setContentsMargins(ContentMargin, ContentMargin, ContentMargin, ContentMargin);
    initDocks();
    moveWidgetsToDocks();
    ui.menubar->setFixedHeight(ui.menubar->height() + 2);

    connect(::config, &ConfigManager::onChanged, this, &QtBitcoinTrader::onConfigChanged);
    connect(::config, &ConfigManager::onError, this, &QtBitcoinTrader::onConfigError);
    initConfigMenu();

    if (iniSettings->value("UI/OptimizeInterface", false).toBool())
        recursiveUpdateLayouts(this);

    secondTimer->setSingleShot(true);
    connect(secondTimer.data(), &QTimer::timeout, this, &QtBitcoinTrader::secondSlot);
    secondSlot();
}

QtBitcoinTrader::~QtBitcoinTrader()
{
    if (currentExchangeThread && currentExchangeThread->isRunning())
    {
        currentExchangeThread->quit();
        currentExchangeThread->wait();
        delete currentExchange;
        currentExchange = nullptr;
        currentExchangeThread.reset();
    }
}

void QtBitcoinTrader::fixTableViews(QWidget* wid)
{
    for (QTableView* tables : wid->findChildren<QTableView*>())
    {
        QFont tableFont = tables->font();
        tableFont.setFixedPitch(true);
        tables->setFont(tableFont);
        tables->setMinimumWidth(200);
        tables->setMinimumHeight(190);
        tables->verticalHeader()->setDefaultSectionSize(defaultHeightForRow);
    }
}

double QtBitcoinTrader::getIndicatorValue(const QString& name)
{
    QDoubleSpinBox* spin = indicatorsMap.value(name, nullptr);

    if (spin == nullptr)
        return 0.0;

    return spin->value();
}

void QtBitcoinTrader::setColumnResizeMode(QTableView* table, int column, QHeaderView::ResizeMode mode)
{
    table->horizontalHeader()->setSectionResizeMode(column, mode);
}

void QtBitcoinTrader::setColumnResizeMode(QTableView* table, QHeaderView::ResizeMode mode)
{
    table->horizontalHeader()->setSectionResizeMode(mode);
}

void QtBitcoinTrader::setupClass()
{
    switch (exchangeId)
    {
    case 0:
        QCoreApplication::quit();
        return; // Secret Excange

    case 2:
        currentExchange = new Exchange_Bitstamp(baseValues.restSign, baseValues.restKey);
        break; // Bitstamp

    case 4:
        currentExchange = new Exchange_Bitfinex(baseValues.restSign, baseValues.restKey);
        break; // Bitfinex

    case 6:
        currentExchange = new Exchange_Indacoin(baseValues.restSign, baseValues.restKey);
        break; // Indacoin

    case 10:
        currentExchange = new Exchange_YObit(baseValues.restSign, baseValues.restKey);
        break; // YObit

    case 11:
        currentExchange = new Exchange_Binance(baseValues.restSign, baseValues.restKey);
        break; // Binance

    case 12:
        currentExchange = new Exchange_Bittrex(baseValues.restSign, baseValues.restKey);
        break; // Bittrex

    case 13:
        currentExchange = new Exchange_HitBTC(baseValues.restSign, baseValues.restKey);
        break; // HitBTC

    case 14:
        currentExchange = new Exchange_Poloniex(baseValues.restSign, baseValues.restKey);
        break; // Poloniex

    default:
        return;
    }

    baseValues.minimumRequestInterval = currentExchange->minimumRequestIntervalAllowed;
    baseValues.httpRequestInterval = iniSettings->value("Network/HttpRequestsInterval", baseValues.minimumRequestInterval).toInt();
    baseValues.minimumRequestTimeout = currentExchange->minimumRequestTimeoutAllowed;
    baseValues.httpRequestTimeout = iniSettings->value("Network/HttpRequestsTimeout", baseValues.minimumRequestTimeout).toInt();

    if (baseValues.httpRequestInterval < 50)
        baseValues.httpRequestInterval = 50;

    if (baseValues.httpRequestTimeout < 100)
        baseValues.httpRequestTimeout = 100;

    iniSettings->setValue("Network/HttpRequestsInterval", baseValues.httpRequestInterval);
    iniSettings->setValue("Network/HttpRequestsTimeout", baseValues.httpRequestTimeout);

    currentExchangeThread.reset(new QThread);
    currentExchangeThread->setObjectName("Exchange Thread");
    currentExchange->moveToThread(currentExchangeThread.data());
    connect(currentExchangeThread.data(), &QThread::started, currentExchange, &Exchange::run);

    baseValues.restSign.clear();

    currentExchange->setupApi(this, false);

    if (currentExchange->domain.isEmpty())
        setCurrencyPairsList();

    setApiDown(false);

    if (!currentExchange->exchangeTickerSupportsHiLowPrices)
        for (int n = 0; n < ui.highLowLayout->count(); n++)
        {
            QWidgetItem* curWid = dynamic_cast<QWidgetItem*>(ui.highLowLayout->itemAt(n));

            if (curWid)
                curWid->widget()->setVisible(false);
        }

    if (!currentExchange->supportsExchangeFee)
    {
        ui.accountFee->setButtonSymbols(QAbstractSpinBox::UpDownArrows);
        ui.accountFee->setReadOnly(false);

        setSpinValue(ui.accountFee, iniSettings->value("Profile/CustomFee", 0.35).toDouble());
        ui.feeSpinboxLayout->addWidget(new JulySpinBoxPicker(ui.accountFee));
    }

    if (!currentExchange->supportsExchangeVolume)
    {
        ui.marketVolumeLabel->setVisible(false);
        ui.btcLabel4->setVisible(false);
        ui.marketVolume->setVisible(false);
    }

    if (currentExchange->clearOpenOrdersOnCurrencyChanged || currentExchange->exchangeDisplayOnlyCurrentPairOpenOrders)
    {
        ui.ordersFilterCheckBox->setVisible(false);

        if (currentExchange->exchangeDisplayOnlyCurrentPairOpenOrders)
            ui.ordersFilterCheckBox->setChecked(true);

        ui.filterOrdersCurrency->setVisible(false);
        ui.centerOrdersTotalSpacer->setVisible(true);
    }
    else
        ui.centerOrdersTotalSpacer->setVisible(false);

    if (!currentExchange->supportsLoginIndicator)
    {
        ui.loginVolumeBack->setVisible(false);
        QSize sz = ui.widgetAccount->maximumSize();
        ui.widgetAccount->setMaximumSize(QSize(200, sz.height()));
    }
    else if (currentExchange->supportsLoginIndicator && !currentExchange->supportsAccountVolume)
    {
        ui.labelAccountVolume->setVisible(false);
        ui.btcLabelAccountVolume->setVisible(false);
        ui.accountVolume->setVisible(false);
    }

    ordersModel->checkDuplicatedOID = currentExchange->checkDuplicatedOID;

    {
        QEventLoop waitStarted;
        connect(currentExchange, &Exchange::started, &waitStarted, &QEventLoop::quit);
        currentExchangeThread->start();
        waitStarted.exec();
    }

    if (!ui.widgetStaysOnTop->isChecked())
        on_widgetStaysOnTop_toggled(ui.widgetStaysOnTop->isChecked());

    ui.widgetStaysOnTop->setChecked(iniSettings->value("UI/WindowOnTop", false).toBool());
    languageChanged();
    reloadScripts();
    IndicatorEngine::global();
    int nextTheme = iniSettings->value("UI/NightMode", 0).toInt();

    if (nextTheme == 1)
        on_buttonNight_clicked();
    else if (nextTheme == 2)
    {
        baseValues.currentTheme = 1;
        on_buttonNight_clicked();
    }
    else
        ui.widgetLogo->setStyleSheet("background:white");

    ::config->load("");

    if (isHidden())
        show();

    ui.buyPercentage->setMaximumWidth(ui.buyPercentage->height());
    ui.sellPercentage->setMaximumWidth(ui.sellPercentage->height());
    fixDepthBidsTable();

    ui.comboBoxGroupByPrice->blockSignals(true);
    ui.comboBoxGroupByPrice->addItem(julyTr("DONT_GROUP", "None"), double(0));
    ui.comboBoxGroupByPrice->blockSignals(false);
    baseValues.groupPriceValue = iniSettings->value("UI/DepthGroupByPrice", 0.0).toDouble();

    if (baseValues.groupPriceValue < 0.0)
        baseValues.groupPriceValue = 0.0;
    else if (baseValues.groupPriceValue > 0.0)
    {
        ui.comboBoxGroupByPrice->addItem(QString::number(baseValues.groupPriceValue), baseValues.groupPriceValue);
        ui.comboBoxGroupByPrice->setCurrentIndex(1);
    }
}

void QtBitcoinTrader::addRuleByHolder(RuleHolder& holder, bool isEnabled, QString titleName)
{
    int findNameTab = -1;

    for (int n = 0; n < ui.rulesTabs->count(); n++)
    {
        QWidget* currentWidget = ui.rulesTabs->widget(n);

        if (!titleName.isEmpty() && currentWidget->windowTitle() == titleName)
            findNameTab = n;

        if (findNameTab > -1)
            break;
    }

    if (findNameTab > -1)
    {
        QWidget* currentWidget = ui.rulesTabs->widget(findNameTab);

        if (currentWidget->property("GroupType").toString() == QLatin1String("Rule"))
            (static_cast<RuleWidget*>(currentWidget))->addRuleByHolder(holder, isEnabled);
        else if (currentWidget->property("GroupType").toString() == QLatin1String("Script"))
            (static_cast<ScriptWidget*>(currentWidget))->replaceScript(RuleScriptParser::holderToScript(holder, false));
    }
    else
    {
        QString nameTemplate(baseValues.scriptFolder + "Script_%1.JLS");
        int ruleN = 1;

        while (QFile::exists(nameTemplate.arg(ruleN)))
            ruleN++;

        ScriptWidget* newRule = new ScriptWidget(titleName, nameTemplate.arg(ruleN));
        findNameTab = ui.rulesTabs->count();
        ui.rulesTabs->insertTab(findNameTab, newRule, newRule->windowTitle());
        newRule->replaceScript(RuleScriptParser::holderToScript(holder, false));
    }

    if (findNameTab > -1 && findNameTab < ui.rulesTabs->count())
        ui.rulesTabs->setCurrentIndex(findNameTab);
}

QStringList QtBitcoinTrader::getRuleGroupsNames()
{
    QStringList rezult;

    for (RuleWidget* ruleWidget : ui.tabRules->findChildren<RuleWidget*>())
        if (ruleWidget)
            rezult << ruleWidget->windowTitle();

    return rezult;
}

QStringList QtBitcoinTrader::getScriptGroupsNames()
{
    QStringList rezult;

    for (ScriptWidget* scriptWidget : ui.tabRules->findChildren<ScriptWidget*>())
        if (scriptWidget)
            rezult << scriptWidget->windowTitle();

    return rezult;
}

bool QtBitcoinTrader::getIsGroupRunning(const QString& name)
{
    for (RuleWidget* ruleWidget : ui.tabRules->findChildren<RuleWidget*>())
        if (ruleWidget && (ruleWidget->windowTitle().compare(name, Qt::CaseInsensitive) == 0))
            return ruleWidget->haveWorkingRules();

    for (ScriptWidget* scriptWidget : ui.tabRules->findChildren<ScriptWidget*>())
        if (scriptWidget && (scriptWidget->windowTitle().compare(name, Qt::CaseInsensitive) == 0))
            return scriptWidget->isRunning();

    return false;
}

void QtBitcoinTrader::setGroupState(const QString& name, bool enabled)
{
    for (RuleWidget* ruleWidget : ui.tabRules->findChildren<RuleWidget*>())
        if (ruleWidget && (ruleWidget->windowTitle().compare(name, Qt::CaseInsensitive) == 0))
        {
            if (enabled)
                ruleWidget->ruleEnableAll();
            else
                ruleWidget->ruleDisableAll();
        }

    for (ScriptWidget* scriptWidget : ui.tabRules->findChildren<ScriptWidget*>())
        if (scriptWidget && (scriptWidget->windowTitle().compare(name, Qt::CaseInsensitive) == 0))
            scriptWidget->setRunning(enabled);
}

void QtBitcoinTrader::clearPendingGroup(const QString& name)
{
    for (int n = pendingGroupStates.size() - 1; n >= 0; n--)
        if (pendingGroupStates.at(n).name == name)
            pendingGroupStates.removeAt(n);
}

void QtBitcoinTrader::setGroupRunning(const QString& name, bool enabled)
{
    if (enabled)
        pendingGroupStates << GroupStateItem(name, enabled);
    else
    {
        clearPendingGroup(name);
        setGroupState(name, enabled);
    }
}

void QtBitcoinTrader::on_buyPercentage_clicked()
{
    PercentPicker* percentPicker = new PercentPicker(buyTotalBtc, getAvailableUSDtoBTC(buyPricePerCoin->value()));
    QPoint execPos = ui.buyAmountPickerBack->mapToGlobal(ui.buyPercentage->geometry().center());
    execPos.setX(execPos.x() - percentPicker->width() / 2);
    execPos.setY(execPos.y() - percentPicker->width());
    percentPicker->exec(execPos);
}

void QtBitcoinTrader::on_sellPercentage_clicked()
{
    PercentPicker* percentPicker = new PercentPicker(sellTotalBtc, getAvailableBTC());
    QPoint execPos = ui.sellAmountPickerBack->mapToGlobal(ui.sellPercentage->geometry().center());
    execPos.setX(execPos.x() - percentPicker->width() / 2);
    execPos.setY(execPos.y() - percentPicker->width());
    percentPicker->exec(execPos);
}

void QtBitcoinTrader::ordersFilterChanged()
{
    QString filterSymbol;
    QString currAStr;
    QString currBStr;
    QPixmap currPixmap;

    if (ui.ordersFilterCheckBox->isChecked())
    {
        filterSymbol = ui.filterOrdersCurrency->currentText();
        int posSplitter = filterSymbol.indexOf('/');

        if (posSplitter == -1)
        {
            currAStr = filterSymbol.left(3);
            currBStr = filterSymbol.right(3);
        }
        else
        {
            currAStr = filterSymbol.left(posSplitter);
            currBStr = filterSymbol.right(filterSymbol.size() - posSplitter - 1);
        }

        if (filterSymbol.size())
            if (baseValues.currentPair.symbol.indexOf("/") == -1)
                filterSymbol.replace("/", "");
    }
    else
    {
        currAStr = baseValues.currentPair.currAStr.toUpper();
        currBStr = baseValues.currentPair.currBStr.toUpper();
    }

    currencySignLoader->getCurrencySign(currAStr, currPixmap);
    ui.currALabel->setPixmap(currPixmap);
    ui.currALabel->setToolTip(currAStr);

    currencySignLoader->getCurrencySign(currBStr, currPixmap);
    ui.currBLabel->setPixmap(currPixmap);
    ui.currBLabel->setToolTip(currBStr);

    ordersSortModel->setFilterWildcard(filterSymbol);
    ordersModel->filterSymbolChanged(filterSymbol);
}

void QtBitcoinTrader::tableCopyContextMenuRequested(QPoint point)
{
    QTableView* table = dynamic_cast<QTableView*>(sender());

    if (table == nullptr)
        return;

    int selectedCount = table->selectionModel()->selectedRows().size();

    if (selectedCount == 0)
        return;

    lastCopyTable = table;
    bool isOpenOrders = table == ui.ordersTable;
    bool isDateAvailable = table != ui.depthAsksTable && table != ui.depthBidsTable;

    copyTableValuesMenu.actions().at(2)->setVisible(isDateAvailable);

    copyTableValuesMenu.actions().at(6)->setEnabled(selectedCount == 1);
    copyTableValuesMenu.actions().at(7)->setEnabled(selectedCount == 1);
    copyTableValuesMenu.actions().at(8)->setEnabled(selectedCount == 1);
    copyTableValuesMenu.actions().at(9)->setEnabled(selectedCount == 1);

    copyTableValuesMenu.actions().at(10)->setVisible(isOpenOrders);
    copyTableValuesMenu.actions().at(11)->setVisible(isOpenOrders);
    copyTableValuesMenu.actions().at(12)->setVisible(isOpenOrders);

    copyTableValuesMenu.exec(lastCopyTable->viewport()->mapToGlobal(point));
}

int QtBitcoinTrader::getOpenOrdersCount(int all) //-1: asks, 0 all, 1: bids
{
    if (all == 0)
        return ordersModel->rowCount();

    if (all == -1)
        return ordersModel->getAsksCount();

    return ordersModel->rowCount() - ordersModel->getAsksCount();
}

void QtBitcoinTrader::repeatSelectedOrderByType(int type, bool availableOnly)
{
    if (lastCopyTable == nullptr || lastCopyTable->selectionModel()->selectedRows().size() != 1)
        return;

    int row = lastCopyTable->selectionModel()->selectedRows().first().row();

    if (lastCopyTable == ui.tableTrades)
        repeatOrderFromTrades(type, row);
    else if (lastCopyTable == ui.tableHistory)
        repeatOrderFromValues(type, historyModel->getRowPrice(row), historyModel->getRowVolume(row), availableOnly);

    if (lastCopyTable == ui.ordersTable)
    {
        row = ordersSortModel->mapToSource(ordersSortModel->index(row, 0)).row();
        repeatOrderFromValues(type, ordersModel->getRowPrice(row), ordersModel->getRowVolume(row) * floatFeeDec, availableOnly);
    }

    if (lastCopyTable == ui.depthAsksTable)
        depthSelectOrder(swapedDepth ? depthBidsModel->index(row, 3) : depthAsksModel->index(row, 3), true, type);

    if (lastCopyTable == ui.depthBidsTable)
        depthSelectOrder(!swapedDepth ? depthBidsModel->index(row, 1) : depthAsksModel->index(row, 1), false, type);
}

void QtBitcoinTrader::repeatBuySellOrder()
{
    repeatSelectedOrderByType(0);
}

void QtBitcoinTrader::repeatBuyOrder()
{
    repeatSelectedOrderByType(1);
}

void QtBitcoinTrader::repeatSellOrder()
{
    repeatSelectedOrderByType(-1);
}

void QtBitcoinTrader::copyDate()
{
    if (lastCopyTable == nullptr)
        return;

    if (lastCopyTable == ui.tableHistory)
        copyInfoFromTable(ui.tableHistory, historyModel, 1);
    else if (lastCopyTable == ui.tableTrades)
        copyInfoFromTable(ui.tableTrades, tradesModel, 1);
    else if (lastCopyTable == ui.ordersTable)
        copyInfoFromTable(ui.ordersTable, ordersSortModel, 1);
}

void QtBitcoinTrader::copyAmount()
{
    if (lastCopyTable == nullptr)
        return;

    if (lastCopyTable == ui.tableHistory)
        copyInfoFromTable(ui.tableHistory, historyModel, 2);
    else if (lastCopyTable == ui.tableTrades)
        copyInfoFromTable(ui.tableTrades, tradesModel, 2);
    else

        if (lastCopyTable == ui.depthAsksTable)
        copyInfoFromTable(ui.depthAsksTable, swapedDepth ? depthBidsModel : depthAsksModel, 3);
    else if (lastCopyTable == ui.depthBidsTable)
        copyInfoFromTable(ui.depthBidsTable, swapedDepth ? depthAsksModel : depthBidsModel, 1);
    else if (lastCopyTable == ui.ordersTable)
        copyInfoFromTable(ui.ordersTable, ordersSortModel, 4);
}

void QtBitcoinTrader::copyPrice()
{
    if (lastCopyTable == nullptr)
        return;

    if (lastCopyTable == ui.tableHistory)
        copyInfoFromTable(ui.tableHistory, historyModel, 4);
    else if (lastCopyTable == ui.tableTrades)
        copyInfoFromTable(ui.tableTrades, tradesModel, 5);
    else

        if (lastCopyTable == ui.depthAsksTable)
        copyInfoFromTable(ui.depthAsksTable, swapedDepth ? depthBidsModel : depthAsksModel, 4);
    else if (lastCopyTable == ui.depthBidsTable)
        copyInfoFromTable(ui.depthBidsTable, swapedDepth ? depthAsksModel : depthBidsModel, 0);
    else if (lastCopyTable == ui.ordersTable)
        copyInfoFromTable(ui.ordersTable, ordersSortModel, 5);
}

void QtBitcoinTrader::copyTotal()
{
    if (lastCopyTable == nullptr)
        return;

    if (lastCopyTable == ui.tableHistory)
        copyInfoFromTable(ui.tableHistory, historyModel, 5);
    else if (lastCopyTable == ui.tableTrades)
        copyInfoFromTable(ui.tableTrades, tradesModel, 6);
    else

        if (lastCopyTable == ui.depthAsksTable)
        copyInfoFromTable(ui.depthAsksTable, swapedDepth ? depthBidsModel : depthAsksModel, 1);
    else if (lastCopyTable == ui.depthBidsTable)
        copyInfoFromTable(ui.depthBidsTable, swapedDepth ? depthAsksModel : depthBidsModel, 3);
    else if (lastCopyTable == ui.ordersTable)
        copyInfoFromTable(ui.ordersTable, ordersSortModel, 6);
}

void QtBitcoinTrader::copyInfoFromTable(QTableView* table, QAbstractItemModel* model, int i)
{
    QModelIndexList selectedRows = table->selectionModel()->selectedRows();

    if (selectedRows.size() == 0)
        return;

    QStringList copyData;

    for (int n = 0; n < selectedRows.size(); n++)
    {
        bool getToolTip = false;

        if ((table == ui.tableHistory && i == 1) || (table == ui.tableTrades && i == 1))
            getToolTip = true;

        copyData << model->index(selectedRows.at(n).row(), i).data((getToolTip ? Qt::ToolTipRole : Qt::DisplayRole)).toString();
    }

    QApplication::clipboard()->setText(copyData.join("\n"));
}

void QtBitcoinTrader::copySelectedRow()
{
    QStringList listToCopy;

    if (lastCopyTable == nullptr)
        return;

    QModelIndexList selectedRows = lastCopyTable->selectionModel()->selectedRows();

    if (selectedRows.size() == 0)
        return;

    for (int n = 0; n < selectedRows.size(); n++)
    {
        QString currentText = selectedRows.at(n).data(Qt::StatusTipRole).toString();

        if (!currentText.isEmpty())
            listToCopy << currentText;
    }

    if (listToCopy.isEmpty())
        return;

    QApplication::clipboard()->setText(listToCopy.join("\n"));
}

void QtBitcoinTrader::on_buttonAddRuleGroup_clicked()
{
    AddRuleGroup ruleGroup;

    if (ui.widgetStaysOnTop->isChecked())
        ruleGroup.setWindowFlags(Qt::WindowCloseButtonHint | Qt::WindowStaysOnTopHint);

    if (ruleGroup.exec() == QDialog::Rejected || ruleGroup.fileName.isEmpty() || !QFile::exists(ruleGroup.fileName))
    {
        return;
    }

    RuleWidget* newRule = new RuleWidget(ruleGroup.fileName);
    ui.rulesTabs->insertTab(ui.rulesTabs->count(), newRule, newRule->windowTitle());

    QStringList rulesListLoad = ruleGroup.groupsList;

    if (rulesListLoad.size())
    {
        ui.rulesTabs->setVisible(true);
        ui.rulesNoMessage->setVisible(false);
        return;
    }

    ui.rulesTabs->setCurrentIndex(ui.rulesTabs->count() - 1);

    ui.rulesTabs->setVisible(true);
    ui.rulesNoMessage->setVisible(false);
}

void QtBitcoinTrader::on_buttonAddScript_clicked()
{
    AddScriptWindow scriptWindow;

    if (ui.widgetStaysOnTop->isChecked())
        scriptWindow.setWindowFlags(Qt::WindowCloseButtonHint | Qt::WindowStaysOnTopHint);

    if (scriptWindow.exec() == QDialog::Rejected || scriptWindow.scriptName.isEmpty())
        return;

    ScriptWidget* newScript = new ScriptWidget(scriptWindow.scriptName, "", scriptWindow.copyFromExistingScript);
    ui.rulesTabs->insertTab(ui.rulesTabs->count(), newScript, newScript->windowTitle());

    ui.rulesTabs->setCurrentIndex(ui.rulesTabs->count() - 1);

    ui.rulesTabs->setVisible(true);
    ui.rulesNoMessage->setVisible(false);
}

void QtBitcoinTrader::reloadScripts()
{
    for (const QString& currentFilePath : QDir(baseValues.scriptFolder)
                                              .entryList(QStringList() << "*.JLS"
                                                                       << "*.JLR"))
    {
        QString currentFile = baseValues.scriptFolder + currentFilePath;

        QString suffix = QFileInfo(currentFile).suffix().toUpper();

        if (suffix == QLatin1String("JLS"))
        {
            QSettings loadScript(currentFile, QSettings::IniFormat);
            QString currentName = loadScript.value("JLScript/Name").toString();

            if (currentName != "")
            {
                ScriptWidget* scriptWidget = new ScriptWidget(currentName, currentFile);
                ui.rulesTabs->insertTab(ui.rulesTabs->count(), scriptWidget, scriptWidget->windowTitle());
            }
            else
                QFile::remove(currentFile);
        }
        else if (suffix == QLatin1String("JLR"))
        {
            QSettings loadScript(currentFile, QSettings::IniFormat);
            QString currentName = loadScript.value("JLRuleGroup/Name", "").toString();

            if (currentName != "")
            {
                RuleWidget* newRule = new RuleWidget(currentFile);
                ui.rulesTabs->insertTab(ui.rulesTabs->count(), newRule, newRule->windowTitle());
            }
            else
                QFile::remove(currentFile);
        }
    }

    ui.rulesTabs->setVisible(ui.rulesTabs->count());
    ui.rulesNoMessage->setVisible(ui.rulesTabs->count() == 0);
}

void QtBitcoinTrader::keyPressEvent(QKeyEvent* event)
{
    event->accept();

    if (ui.ordersTable->hasFocus() && (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace))
    {
        on_ordersCancelSelected_clicked();
        return;
    }

    if (event->modifiers() & Qt::ControlModifier)
    {
        if (event->key() == Qt::Key_B)
            buyBitcoinsButton();
        else if (event->key() == Qt::Key_S)
            sellBitcoinButton();
        else
#ifndef Q_OS_MAC
            if (event->key() == Qt::Key_H)
            buttonMinimizeToTray();
        else
#endif
            if (event->key() == Qt::Key_N)
            buttonNewWindow();
        else if (event->key() == Qt::Key_T)
            ui.widgetStaysOnTop->setChecked(!ui.widgetStaysOnTop->isChecked());

        return;
    }

    int modifiersPressed = 0;

    if (event->modifiers() & Qt::ControlModifier)
        modifiersPressed++;

    if (event->modifiers() & Qt::ShiftModifier)
        modifiersPressed++;

    if (event->modifiers() & Qt::AltModifier)
        modifiersPressed++;

    if (event->modifiers() & Qt::MetaModifier)
        modifiersPressed++;

    if (event->key() == Qt::Key_T && modifiersPressed > 1)
    {
        (new TranslationAbout(windowWidget))->showWindow();
        return;
    }

    if (event->key() == Qt::Key_D && modifiersPressed > 1)
    {
        onActionDebug();
        return;
    }
}

void QtBitcoinTrader::precentBidsChanged(double val)
{
    ui.tradesBidsPrecent->setValue(val);

    static bool lastPrecentGrowing = false;
    bool precentGrowing = tradesPrecentLast < val;

    if (lastPrecentGrowing != precentGrowing)
    {
        lastPrecentGrowing = precentGrowing;
        ui.tradesLabelDirection->setText(precentGrowing ? upArrowNoUtfStr : downArrowNoUtfStr);
    }

    tradesPrecentLast = val;
}

void QtBitcoinTrader::on_swapDepth_clicked()
{
    swapedDepth = !swapedDepth;

    if (swapedDepth)
    {
        depthBidsModel->setAsk(true);
        ui.depthAsksTable->setModel(depthBidsModel);
        depthAsksModel->setAsk(false);
        ui.depthBidsTable->setModel(depthAsksModel);
    }
    else
    {
        depthAsksModel->setAsk(true);
        ui.depthAsksTable->setModel(depthAsksModel);
        depthBidsModel->setAsk(false);
        ui.depthBidsTable->setModel(depthBidsModel);
    }

    QString tempText = ui.asksLabel->text();
    QString tempId = ui.asksLabel->accessibleName();
    QString tempStyle = ui.asksLabel->styleSheet();

    ui.asksLabel->setText(ui.bidsLabel->text());
    ui.asksLabel->setAccessibleName(ui.bidsLabel->accessibleName());
    ui.asksLabel->setStyleSheet(ui.bidsLabel->styleSheet());

    ui.bidsLabel->setText(tempText);
    ui.bidsLabel->setAccessibleName(tempId);
    ui.bidsLabel->setStyleSheet(tempStyle);

    iniSettings->setValue("UI/SwapDepth", swapedDepth);
    iniSettings->sync();
}

void QtBitcoinTrader::anyDataReceived()
{
    softLagTime.restart();
    setSoftLagValue(0);
}

double QtBitcoinTrader::getFeeForUSDDec(double usd)
{
    double result = JulyMath::cutDoubleDecimalsCopy(usd, baseValues.currentPair.currBDecimals, false);
    double calcFee = JulyMath::cutDoubleDecimalsCopy(result, baseValues.currentPair.priceDecimals, true) * floatFee;
    calcFee = JulyMath::cutDoubleDecimalsCopy(calcFee, baseValues.currentPair.priceDecimals, true);
    result = result - calcFee;
    return result;
}

void QtBitcoinTrader::addPopupDialog(int val)
{
    currentPopupDialogs += val;
    ui.buttonNewWindow->setEnabled(currentPopupDialogs == 0);
}

void QtBitcoinTrader::buttonMinimizeToTray()
{
    if (trayIcon == nullptr)
    {
        trayIcon = new QSystemTrayIcon(QIcon(":/Resources/QtBitcoinTrader.png"), this);
        trayIcon->setToolTip(windowTitle());
        connect(trayIcon, &QSystemTrayIcon::activated, this, &QtBitcoinTrader::trayActivated);
        trayMenu = new QMenu(this);
        trayIcon->setContextMenu(trayMenu);
        trayMenu->addAction(QIcon(":/Resources/exit.png"), "Exit");
        trayMenu->actions().last()->setWhatsThis("EXIT");
        connect(trayMenu->actions().first(), &QAction::triggered, this, &QtBitcoinTrader::exitApp);
    }

    trayIcon->show();
    trayIcon->showMessage(windowTitleP, windowTitle());
    windowWidget->hide();
    dockHost->setFloatingVisible(false);
}

void QtBitcoinTrader::trayActivated(QSystemTrayIcon::ActivationReason reazon)
{
    if (trayIcon == nullptr)
        return;

    if (reazon == QSystemTrayIcon::Context)
    {
        QList<QAction*> tList = trayMenu->actions();

        for (int n = 0; n < tList.size(); n++)
            tList.at(n)->setText(julyTr(tList.at(n)->whatsThis(), tList.at(n)->text()));

        trayMenu->exec();
        return;
    }

    windowWidget->show();
    dockHost->setFloatingVisible(true);

    trayIcon->hide();
    delete trayMenu;
    trayMenu = nullptr;
    delete trayIcon;
    trayIcon = nullptr;
}

void QtBitcoinTrader::checkUpdate()
{
    QProcess::startDetached(QApplication::applicationFilePath(), QStringList("/checkupdatemessage"));
}

void QtBitcoinTrader::startApplication(const QString& name, QStringList params)
{
#ifdef Q_OS_MAC

    if (QFileInfo(name).fileName().contains(QLatin1Char('.')))
        params.prepend(name);

    QProcess::startDetached(QLatin1String("open"), params);
#else
    QProcess::startDetached(name, params);
#endif
}

void QtBitcoinTrader::sayText(const QString& text)
{
#ifdef QT_TEXTTOSPEECH_LIB
    static QTextToSpeech ttsEngine;

    if (ttsEngine.availableEngines().isEmpty())
#endif
#ifdef Q_OS_WIN
        startApplication(QLatin1String("say.exe"), QStringList() << text);
#else
    startApplication(QLatin1String("say"), QStringList() << text);
#endif
#ifdef QT_TEXTTOSPEECH_LIB
    else
        ttsEngine.say(text);
#endif
}

void QtBitcoinTrader::setLastTrades10MinVolume(double val)
{
    ui.tradesVolume5m->setValue(val);
}

void QtBitcoinTrader::availableAmountChanged(const QString& symbol, double val)
{
    if (baseValues.currentPair.symbolSecond() == symbol)
        availableAmount = val;
}

void QtBitcoinTrader::addLastTrades(const QString& symbol, QList<TradesItem>* newItems)
{
    if (secondTimer.isNull())
    {
        delete newItems;
        return;
    }

    if (baseValues.currentPair.symbol != symbol)
    {
        delete newItems;
        return;
    }

    int newRowsCount = newItems->size();
    tradesModel->addNewTrades(newItems);

    tradesModel->updateTotalBTC();

    if (ui.tradesAutoScrollCheck->isChecked() && ui.tabLastTrades->isVisible())
    {
        setTradesScrollBarValue(ui.tableTrades->verticalScrollBar()->value() + defaultHeightForRow * newRowsCount);
        tabTradesScrollUp();
    }

    ui.tableTrades->resizeColumnToContents(1);
    ui.tableTrades->resizeColumnToContents(2);
    ui.tableTrades->resizeColumnToContents(3);
    ui.tableTrades->resizeColumnToContents(4);
    ui.tableTrades->resizeColumnToContents(5);
    ui.tableTrades->resizeColumnToContents(6);
}

void QtBitcoinTrader::clearTimeOutedTrades()
{
    if (secondTimer.isNull())
        return;

    if (tradesModel->rowCount() == 0)
        return;

    int lastSliderValue = ui.tableTrades->verticalScrollBar()->value();
    tradesModel->removeDataOlderThen(TimeSync::getTimeT() - 600);
    ui.tableTrades->verticalScrollBar()->setValue(qMin(lastSliderValue, ui.tableTrades->verticalScrollBar()->maximum()));
}

void QtBitcoinTrader::depthRequested()
{
    depthLagTime.restart();
    waitingDepthLag = true;
}

void QtBitcoinTrader::depthRequestReceived()
{
    waitingDepthLag = false;
}

void QtBitcoinTrader::secondSlot()
{
    while (pendingGroupStates.size() && pendingGroupStates.first().elapsed.elapsed() >= 100)
    {
        setGroupState(pendingGroupStates.first().name, pendingGroupStates.first().enabled);
        pendingGroupStates.removeFirst();
    }

    static int execCount = 0;

    if (execCount == 0 || execCount == 2 || execCount == 4)
    {
        clearTimeOutedTrades();
        setSoftLagValue(softLagTime.elapsed());
    }
    else if (execCount == 1 || execCount == 3 || execCount == 5)
    {
        int currentElapsed = depthLagTime.elapsed();

        if (ui.tabDepth->isVisible())
            ui.depthLag->setValue(currentElapsed / 1000.0);
    }

    if (historyForceUpdate.elapsed() >= 15000)
    {
        historyForceUpdate.restart();
        emit getHistory(true);
    }

    if (speedTestTime.elapsed() >= 500)
    {
        speedTestTime.restart();

        static QList<double> halfSecondsList;
        halfSecondsList << static_cast<double>(baseValues.trafficSpeed / 512.0);
        baseValues.trafficTotal += baseValues.trafficSpeed;
        updateTrafficTotalValue();

        while (halfSecondsList.size() > 10)
            halfSecondsList.removeFirst();

        static double avSpeed;
        avSpeed = 0.0;

        for (int n = 0; n < halfSecondsList.size(); n++)
            avSpeed += halfSecondsList.at(n);

        if (halfSecondsList.size())
            avSpeed = avSpeed * 2.0 / halfSecondsList.size();

        ui.trafficSpeed->setValue(avSpeed);
        baseValues.trafficSpeed = 0;
    }

    if (++execCount > 5)
        execCount = 0;

    secondTimer->start(baseValues.uiUpdateInterval);
}

void QtBitcoinTrader::depthVisibilityChanged(bool visible)
{
    if (currentExchange)
        currentExchange->depthEnabledFlag = visible || depthAsksModel->rowCount() == 0 || depthBidsModel->rowCount() == 0;
}

void QtBitcoinTrader::trafficTotalToZero_clicked()
{
    baseValues.trafficTotal = 0;
    updateTrafficTotalValue();
}

void QtBitcoinTrader::updateTrafficTotalValue()
{
    static int trafficTotalTypeLast = -1;

    if (baseValues.trafficTotal > 1073741824) // Gb
        baseValues.trafficTotalType = 2;
    else if (baseValues.trafficTotal > 1048576) // Mb
        baseValues.trafficTotalType = 1;
    else if (baseValues.trafficTotal > 1024) // Kb
        baseValues.trafficTotalType = 0;

    if (trafficTotalTypeLast != baseValues.trafficTotalType)
    {
        trafficTotalTypeLast = baseValues.trafficTotalType;

        switch (trafficTotalTypeLast)
        {
        case 0:
            networkMenu->setSuffix(" Kb");
            break;

        case 1:
            networkMenu->setSuffix(" Mb");
            break;

        case 2:
            networkMenu->setSuffix(" Gb");
            break;

        default:
            break;
        }
    }

    static int totalValueLast = -1;
    int totalValue = 0;

    switch (trafficTotalTypeLast)
    {
    case 0:
        totalValue = static_cast<int>(baseValues.trafficTotal / 1024);
        break;

    case 1:
        totalValue = static_cast<int>(baseValues.trafficTotal / 1048576);
        break;

    case 2:
        totalValue = static_cast<int>(baseValues.trafficTotal / 1073741824);
        break;

    default:
        break;
    }

    if (totalValueLast != totalValue)
    {
        totalValueLast = totalValue;

        if (networkMenu->getNetworkTotalMaximum() < totalValue)
            networkMenu->setNetworkTotalMaximum(totalValue * 10);

        networkMenu->setNetworkTotal(totalValue);
    }
}

void QtBitcoinTrader::tabTradesScrollUp()
{
    if (secondTimer.isNull())
        return;

    static QTimeLine timeLine(1, this);

    if (timeLine.duration() == 1)
    {
        connect(&timeLine, &QTimeLine::frameChanged, this, &QtBitcoinTrader::setTradesScrollBarValue);
        timeLine.setDuration(500);
        timeLine.setEasingCurve(QEasingCurve::OutCirc);
        timeLine.setLoopCount(1);
    }

    timeLine.stop();
    int currentScrollPos = ui.tableTrades->verticalScrollBar()->value();

    if (currentScrollPos == 0)
        return;

    timeLine.setFrameRange(currentScrollPos, 0);
    timeLine.start();
}

void QtBitcoinTrader::tabTradesIndexChanged(int)
{
    if (ui.tabLastTrades->isVisible() && ui.tradesAutoScrollCheck->isChecked())
        tabTradesScrollUp();
}

void QtBitcoinTrader::setTradesScrollBarValue(int val)
{
    if (secondTimer.isNull())
        return;

    if (val > ui.tableTrades->verticalScrollBar()->maximum())
        tabTradesScrollUp();
    else
        ui.tableTrades->verticalScrollBar()->setValue(val);

    // ui.tableTrades->verticalScrollBar()->setValue(qMin(val,ui.tableTrades->verticalScrollBar()->maximum()));
}

void QtBitcoinTrader::fixAllCurrencyLabels(QWidget* par)
{
    for (QLabel* label : par->findChildren<QLabel*>())
        if (label->accessibleDescription() == "USD_ICON" || label->accessibleDescription() == "BTC_ICON")
        {
            label->setMaximumSize(20, 20);
            label->setScaledContents(true);
        }
}

void QtBitcoinTrader::fillAllUsdLabels(QWidget* par, QString curName)
{
    curName = curName.toUpper();
    QPixmap btcPixmap;
    currencySignLoader->getCurrencySign(curName, btcPixmap);

    for (QLabel* labels : par->findChildren<QLabel*>())
        if (labels->accessibleDescription() == "USD_ICON")
        {
            labels->setPixmap(btcPixmap);
            labels->setToolTip(curName);
        }
}
void QtBitcoinTrader::fillAllBtcLabels(QWidget* par, QString curName)
{
    curName = curName.toUpper();
    QPixmap btcPixmap;
    currencySignLoader->getCurrencySign(curName, btcPixmap);

    for (QLabel* labels : par->findChildren<QLabel*>())
    {
        if (labels->accessibleDescription() == "BTC_ICON")
        {
            labels->setPixmap(btcPixmap);
            labels->setToolTip(curName);
        }
        else if (labels->accessibleDescription() == "DONATE_BTC_ICON")
        {
            if (labels->toolTip() != "BTC")
            {
                QPixmap btcPixmapBTC;
                currencySignLoader->getCurrencySign("BTC", btcPixmapBTC);
                labels->setPixmap(btcPixmapBTC);
                labels->setToolTip("BTC");
            }
        }
    }
}

void QtBitcoinTrader::makeRitchValue(QString* text)
{
    int lastSymbol = text->length() - 1;

    if (lastSymbol == -1)
        return;

    while (lastSymbol > -1 && text->at(lastSymbol) == '0')
        lastSymbol--;

    if (lastSymbol > -1 && text->at(lastSymbol) == '.')
        lastSymbol--;

    if (lastSymbol < -1)
        return;

    QString buff = text->left(lastSymbol + 1);
    text->remove(0, lastSymbol + 1);
    text->prepend("<font color=gray>");
    text->append("</font>");
    text->prepend(buff);
}

void QtBitcoinTrader::reloadLanguage(const QString& preferedLangFile)
{
    constructorFinished = false;

    QString preferedLangFilePath = (preferedLangFile.isEmpty() || !QFile::exists(preferedLangFile)) ? julyTranslator.lastFile() :
                                                                                                      preferedLangFile;

    julyTranslator.loadFromFile(preferedLangFilePath);
    constructorFinished = true;

    languageChanged();
}

void QtBitcoinTrader::fixAllChildButtonsAndLabels(QWidget* par)
{
    for (QPushButton* pushButtons : par->findChildren<QPushButton*>())
        if (!pushButtons->text().isEmpty())
            pushButtons->setMinimumWidth(qMin(pushButtons->maximumWidth(), textFontWidth(pushButtons->text()) + 10));

    for (QToolButton* toolButtons : par->findChildren<QToolButton*>())
        if (!toolButtons->text().isEmpty())
            toolButtons->setMinimumWidth(toolButtons->minimumSizeHint().width());

    for (QCheckBox* checkBoxes : par->findChildren<QCheckBox*>())
        checkBoxes->setMinimumWidth(qMin(checkBoxes->maximumWidth(), textFontWidth(checkBoxes->text()) + 20));

    for (QLabel* labels : par->findChildren<QLabel*>())
        if (labels->text().length() && labels->text().at(0) != '<' && labels->accessibleDescription() != "IGNORED")
            labels->setMinimumWidth(qMin(labels->maximumWidth(), textFontWidth(labels->text())));

    fixDecimals(this);

    for (QWidget* widget : par->findChildren<QWidget*>())
    {
        if (widget->accessibleName() == "LOGOBUTTON")
        {
            QLayout* layout = widget->layout();

            if (layout == nullptr)
            {
                layout = new QGridLayout();
                layout->setContentsMargins(0, 0, 0, 0);
                layout->setSpacing(0);
                widget->setLayout(layout);
                LogoButton* logoButton = new LogoButton(false);
                connect(this, &QtBitcoinTrader::themeChanged, logoButton, &LogoButton::themeChanged);
                layout->addWidget(logoButton);
            }
        }
    }

    if (par == this)
        return;

    QSize minSizeHint = par->minimumSizeHint();

    if (isValidSize(&minSizeHint))
    {
        par->setMinimumSize(par->minimumSizeHint());

        if (par->width() < par->minimumSizeHint().width())
            par->resize(par->minimumSizeHint().width(), par->height());
    }
}

void QtBitcoinTrader::fixDecimals(QWidget* par)
{
    for (QDoubleSpinBox* spinBox : par->findChildren<QDoubleSpinBox*>())
    {
        if (!spinBox->whatsThis().isEmpty())
            continue;

        if (spinBox->accessibleName().startsWith("BTC"))
        {
            if (spinBox->accessibleName().endsWith("BALANCE"))
                spinBox->setDecimals(baseValues.currentPair.currABalanceDecimals);
            else
                spinBox->setDecimals(baseValues.currentPair.currADecimals);

            if (spinBox->accessibleDescription() != "CAN_BE_ZERO")
                spinBox->setMinimum(baseValues.currentPair.tradeVolumeMin);
        }
        else if (spinBox->accessibleName().startsWith("USD"))
        {
            if (spinBox->accessibleName().endsWith("BALANCE"))
                spinBox->setDecimals(baseValues.currentPair.currBBalanceDecimals);
            else
                spinBox->setDecimals(baseValues.currentPair.currBDecimals);
        }
        else if (spinBox->accessibleName() == "PRICE")
        {
            spinBox->setDecimals(baseValues.currentPair.priceDecimals);

            if (spinBox->accessibleDescription() != "CAN_BE_ZERO")
                spinBox->setMinimum(baseValues.currentPair.tradePriceMin);
        }
    }
}

void QtBitcoinTrader::loginChanged(const QString& text)
{
    const QString& profileNameText = text.compare(QLatin1String(" ")) ? text : profileName;
    ui.accountLoginLabel->setText(profileNameText);
    ui.accountLoginLabel->setMinimumWidth(textFontWidth(profileNameText) + 20);
}

void QtBitcoinTrader::setCurrencyPairsList()
{
    baseValues.currencyPairMap.clear();
    QString savedCurrency = iniSettings->value("Profile/Currency", "BTC/USD").toString();
    int indexCurrency = 0;
    QStringList currencyItems;
    QStringList filterItems;

    for (int n = 0; n < IniEngine::getPairsCount(); n++)
    {
        CurrencyPairItem pairItem = IniEngine::getPairs()->at(n);

        if (pairItem.currRequestSecond.isEmpty())
            baseValues.currencyPairMap[pairItem.symbol] = pairItem;
        else
        {
            baseValues.currencyPairMap[pairItem.symbol + pairItem.currRequestSecond.toUpper()] = pairItem;

            if (pairItem.currRequestSecond == "exchange")
            {
                baseValues.currencyPairMap[pairItem.symbol] = pairItem;
                filterItems << pairItem.currAStr + "/" + pairItem.currBStr;
            }
        }

        if (pairItem.name == savedCurrency)
            indexCurrency = n;

        currencyItems << pairItem.name;
    }

    currencyMenu->setPairs(currencyItems);
    currencyMenu->setCurrentIndex(indexCurrency);

    ui.filterOrdersCurrency->clear();
    ui.filterOrdersCurrency->insertItems(0, filterItems.isEmpty() ? currencyItems : filterItems);
    ui.filterOrdersCurrency->setCurrentIndex(0);
}

void QtBitcoinTrader::currencyMenuChanged(int val)
{
    if (!constructorFinished || val < 0 || IniEngine::getPairsCount() != currencyMenu->count())
        return;

    /*bool fastChange = ui.currencyComboBox->itemText(val).left(5) ==
                      ui.currencyComboBox->itemText(lastLoadedCurrency).left(5);
    if (val == lastLoadedCurrency)
        return;
    lastLoadedCurrency = val;*/

    CurrencyPairItem nextCurrencyPair = IniEngine::getPairs()->at(val);

    bool currencyAChanged = nextCurrencyPair.currAStr != baseValues.currentPair.currAStr;
    bool currencyBChanged = nextCurrencyPair.currBStr != baseValues.currentPair.currBStr;

    /*if (fastChange)
    {
        baseValues.currentPair = nextCurrencyPair;

        setSpinValue(ui.accountBTC, 0.0);
        setSpinValue(ui.accountUSD, 0.0);

        for (RuleWidget* currentGroup: ui.tabRules->findChildren<RuleWidget*>())
            currentGroup->currencyChanged();

        for (ScriptWidget* currentGroup: ui.tabRules->findChildren<ScriptWidget*>())
            currentGroup->currencyChanged();

        return;
    }*/

    fillAllUsdLabels(this, nextCurrencyPair.currBStr);
    fillAllBtcLabels(this, nextCurrencyPair.currAStr);

    // TODO: ?? fillAll Usd/Btc for float

    iniSettings->setValue("Profile/Currency", ui.currencyMenuTool->text());

    if (currencyAChanged)
        setSpinValue(ui.accountBTC, 0.0);

    if (currencyBChanged)
        setSpinValue(ui.accountUSD, 0.0);

    buyTotalSpend->setValue(0.0);
    sellTotalBtc->setValue(0.0);
    ui.tradesVolume5m->setValue(0.0);
    setSpinValue(ui.ruleAmountToReceiveValue, 0.0);
    setSpinValue(ui.ruleTotalToBuyValue, 0.0);
    setSpinValue(ui.ruleAmountToReceiveBSValue, 0.0);
    setSpinValue(ui.ruleTotalToBuyBSValue, 0.0);

    precentBidsChanged(0.0);
    tradesModel->clear();
    tradesPrecentLast = 0.0;

    QString buyGroupboxText = julyTr("GROUPBOX_BUY", "Buy %1");
    bool buyGroupboxCase = false;

    if (buyGroupboxText.length() > 2)
        buyGroupboxCase = buyGroupboxText.at(2).isUpper();

    if (buyGroupboxCase)
        buyGroupboxText = buyGroupboxText.arg(nextCurrencyPair.currAName.toUpper());
    else
        buyGroupboxText = buyGroupboxText.arg(nextCurrencyPair.currAName);

    ui.widgetBuy->parentWidget()->setWindowTitle(buyGroupboxText);

    QString sellGroupboxText = julyTr("GROUPBOX_SELL", "Sell %1");
    bool sellGroupboxCase = true;

    if (sellGroupboxText.length() > 2)
        sellGroupboxCase = sellGroupboxText.at(2).isUpper();

    if (sellGroupboxCase)
        sellGroupboxText = sellGroupboxText.arg(nextCurrencyPair.currAName.toUpper());
    else
        sellGroupboxText = sellGroupboxText.arg(nextCurrencyPair.currAName);

    ui.widgetSell->parentWidget()->setWindowTitle(sellGroupboxText);

    static int firstLoad = 0;

    if (++firstLoad > 1)
    {
        firstLoad = 3;
        emit clearValues();
    }

    if (ui.comboBoxGroupByPrice->count() > 1)
    {
        ui.comboBoxGroupByPrice->blockSignals(true);
        ui.comboBoxGroupByPrice->clear();
        ui.comboBoxGroupByPrice->blockSignals(false);
        ui.comboBoxGroupByPrice->addItem(julyTr("DONT_GROUP", "None"), double(0));
    }

    marketPricesNotLoaded = true;
    balanceNotLoaded = true;
    fixDecimals(this);

    iniSettings->sync();

    baseValues.currentPair = nextCurrencyPair;
    depthAsksModel->fixTitleWidths();
    depthBidsModel->fixTitleWidths();

    setSpinValue(ui.ordersLastBuyPrice, 0.0);
    setSpinValue(ui.ordersLastSellPrice, 0.0);

    if (currentExchange->clearHistoryOnCurrencyChanged)
        historyModel->clear();
    else
        historyModel->loadLastPrice();

    calcOrdersTotalValues();

    ui.filterOrdersCurrency->setCurrentIndex(val);

    currencyChangedDate = TimeSync::getTimeT();

    fixDecimals(this);

    emit getHistory(true);
    emit clearCharts();
    chartsView->clearCharts();

    setSpinValue(ui.marketHigh, IndicatorEngine::getValue(baseValues.exchangeName + '_' + baseValues.currentPair.symbol + "_High"));
    setSpinValue(ui.marketLow, IndicatorEngine::getValue(baseValues.exchangeName + '_' + baseValues.currentPair.symbol + "_Low"));
    setSpinValue(ui.marketLast, IndicatorEngine::getValue(baseValues.exchangeName + '_' + baseValues.currentPair.symbol + "_Last"));
    setSpinValue(ui.marketVolume, IndicatorEngine::getValue(baseValues.exchangeName + '_' + baseValues.currentPair.symbol + "_Volume"));
    setSpinValue(ui.marketAsk, IndicatorEngine::getValue(baseValues.exchangeName + '_' + baseValues.currentPair.symbol + "_Buy"));
    setSpinValue(ui.marketBid, IndicatorEngine::getValue(baseValues.exchangeName + '_' + baseValues.currentPair.symbol + "_Sell"));

    if (qFuzzyIsNull(ui.marketAsk->value()))
        buyPricePerCoin->setValue(100.0);
    else
        buyPricePerCoin->setValue(ui.marketAsk->value());

    if (qFuzzyIsNull(ui.marketBid->value()))
        sellPricePerCoin->setValue(200.0);
    else
        sellPricePerCoin->setValue(ui.marketBid->value());
}

void QtBitcoinTrader::clearDepth()
{
    depthAsksModel->clear();
    depthBidsModel->clear();
    emit reloadDepth();
}

void QtBitcoinTrader::volumeAmountChanged(double volumeTotal, double amountTotal)
{
    setSpinValue(ui.ordersTotalBTC, volumeTotal);
    setSpinValue(ui.ordersTotalUSD, amountTotal);
}

void QtBitcoinTrader::calcOrdersTotalValues()
{
    checkValidBuyButtons();
    checkValidSellButtons();
}

void QtBitcoinTrader::profitSellThanBuyCalc()
{
    if (!profitSellThanBuyUnlocked)
        return;

    profitSellThanBuyUnlocked = false;
    double calcValue = 0.0;

    if (sellTotalBtc->value() != 0.0 && ui.buyTotalBtcResult->value() != 0.0)
        calcValue = ui.buyTotalBtcResult->value() - sellTotalBtc->value();

    sellThanBuySpinBox->setValue(calcValue);
    profitSellThanBuyUnlocked = true;
}

void QtBitcoinTrader::profitBuyThanSellCalc()
{
    if (!profitBuyThanSellUnlocked)
        return;

    profitBuyThanSellUnlocked = false;
    double calcValue = 0.0;

    if (buyTotalSpend->value() != 0.0 && sellAmountToReceive->value() != 0.0)
        calcValue = sellAmountToReceive->value() - buyTotalSpend->value();

    profitLossSpinBox->setValue(calcValue);
    profitBuyThanSellUnlocked = true;
}

void QtBitcoinTrader::profitLossSpinBoxPrec_valueChanged(double val)
{
    if (profitBuyThanSellChangedUnlocked)
    {
        profitBuyThanSellChangedUnlocked = false;
        profitLossSpinBox->setValue(val == 0.0 ? 0.0 : buyTotalSpend->value() * val / 100.0);
        profitBuyThanSellChangedUnlocked = true;
    }
}

void QtBitcoinTrader::profitLossSpinBox_valueChanged(double val)
{
    QString styleChanged;

    if (val < -0.009)
        styleChanged = "QDoubleSpinBox {background: " + baseValues.appTheme.lightRed.name() + ";}";
    else if (val > 0.009)
        styleChanged = "QDoubleSpinBox {background: " + baseValues.appTheme.lightGreen.name() + ";}";

    if (profitBuyThanSellChangedUnlocked)
    {
        profitBuyThanSellChangedUnlocked = false;
        profitLossSpinBoxPrec->setValue(buyTotalSpend->value() == 0.0 ? 0.0 : val * 100.0 / buyTotalSpend->value());
        profitBuyThanSellChangedUnlocked = true;
    }

    profitLossSpinBox->setStyleSheet(styleChanged);
    profitLossSpinBoxPrec->setStyleSheet(styleChanged);

    ui.buttonBuyThenSellApply->setEnabled(true);
}

void QtBitcoinTrader::sellThanBuySpinBoxPrec_valueChanged(double val)
{
    if (profitBuyThanSellChangedUnlocked)
    {
        profitBuyThanSellChangedUnlocked = false;
        sellThanBuySpinBox->setValue(val == 0.0 ? 0.0 : sellTotalBtc->value() * val / 100.0);
        profitBuyThanSellChangedUnlocked = true;
    }
}

void QtBitcoinTrader::sellThanBuySpinBox_valueChanged(double val)
{
    QString styleChanged;

    if (val < -0.009)
        styleChanged = "QDoubleSpinBox {background: " + baseValues.appTheme.lightRed.name() + ";}";
    else if (val > 0.009)
        styleChanged = "QDoubleSpinBox {background: " + baseValues.appTheme.lightGreen.name() + ";}";

    if (profitBuyThanSellChangedUnlocked)
    {
        profitBuyThanSellChangedUnlocked = false;
        sellThanBuySpinBoxPrec->setValue(sellTotalBtc->value() == 0.0 ? 0.0 : val * 100.0 / sellTotalBtc->value());
        profitBuyThanSellChangedUnlocked = true;
    }

    sellThanBuySpinBox->setStyleSheet(styleChanged);
    sellThanBuySpinBoxPrec->setStyleSheet(styleChanged);

    ui.buttonSellThenBuyApply->setEnabled(true);
}

void QtBitcoinTrader::on_zeroSellThanBuyProfit_clicked()
{
    sellThanBuySpinBox->setValue(0.0);
}

void QtBitcoinTrader::on_zeroBuyThanSellProfit_clicked()
{
    profitLossSpinBox->setValue(0.0);
}

void QtBitcoinTrader::profitSellThanBuy()
{
    profitSellThanBuyUnlocked = false;
    buyTotalSpend->setValue(sellAmountToReceive->value());
    buyPricePerCoin->setValue(buyTotalSpend->value() / ((sellTotalBtc->value() + sellThanBuySpinBox->value()) / floatFeeDec) -
                              baseValues.currentPair.priceMin);
    profitSellThanBuyUnlocked = true;
    profitBuyThanSellCalc();
    profitSellThanBuyCalc();
    ui.buttonSellThenBuyApply->setEnabled(false);
}

void QtBitcoinTrader::profitBuyThanSell()
{
    profitBuyThanSellUnlocked = false;
    sellTotalBtc->setValue(ui.buyTotalBtcResult->value());
    sellPricePerCoin->setValue((buyTotalSpend->value() + profitLossSpinBox->value()) / (sellTotalBtc->value() * floatFeeDec) +
                               baseValues.currentPair.priceMin);
    profitBuyThanSellUnlocked = true;
    profitBuyThanSellCalc();
    profitSellThanBuyCalc();
    ui.buttonBuyThenSellApply->setEnabled(false);
}

void QtBitcoinTrader::setApiDown(bool)
{
    // ui.exchangeLagBack->setVisible(on);
}

QString QtBitcoinTrader::clearData(QString data)
{
    while (data.size() && (data.at(0) == '{' || data.at(0) == '[' || data.at(0) == '\"'))
        data.remove(0, 1);

    while (data.size() && (data.at(data.length() - 1) == '}' || data.at(data.length() - 1) == ']' || data.at(data.length() - 1) == '\"'))
        data.remove(data.length() - 1, 1);

    return data;
}

void QtBitcoinTrader::on_accountFee_valueChanged(double val)
{
    floatFee = val / 100.0;
    floatFeeDec = 1.0 - floatFee;
    floatFeeInc = 1.0 + floatFee;

    if (currentExchange && !currentExchange->supportsExchangeFee)
        iniSettings->setValue("Profile/CustomFee", ui.accountFee->value());

    bool notZeroFee = floatFee > 0.0;
    ui.calcButton->setVisible(notZeroFee);
    ui.label_6->setVisible(notZeroFee);
    ui.label_10->setVisible(notZeroFee);
    ui.label_28->setVisible(notZeroFee);
    ui.label_29->setVisible(notZeroFee);
    ui.buyNextInSellPrice->setVisible(notZeroFee);
    ui.buyNextMinBuyStep->setVisible(notZeroFee);
    ui.sellNextMaxBuyPrice->setVisible(notZeroFee);
    ui.sellNextMaxBuyStep->setVisible(notZeroFee);
    ui.usdLabel9->setVisible(notZeroFee);
    ui.usdLabel10->setVisible(notZeroFee);
    ui.usdLabel13->setVisible(notZeroFee);
    ui.usdLabel14->setVisible(notZeroFee);
    ui.buyTotalBtcResult->setVisible(notZeroFee);
    ui.btcLabel6->setVisible(notZeroFee);
    ui.label_62->setVisible(notZeroFee);
}

QByteArray QtBitcoinTrader::getMidData(const QString& a, QString b, QByteArray* data)
{
    QByteArray rez;

    if (b.isEmpty())
        b = "\",";

    int startPos = data->indexOf(a.toLatin1(), 0);

    if (startPos > -1)
    {
        int endPos = data->indexOf(b.toLatin1(), startPos + a.length());

        if (endPos > -1)
            rez = data->mid(startPos + a.length(), endPos - startPos - a.length());
    }

    return rez;
}

static void adjustDockMinSize(QWidget* widget)
{
    QDockWidget* dock = static_cast<QDockWidget*>(widget->parentWidget());
    int minHint = widget->minimumSizeHint().width();
    int textWidth = textFontWidth(dock->windowTitle());
    static const int TitleExtra = 50; // left padding + undock/close buttons
    int minWidth = qMax(minHint, textWidth + TitleExtra);
    widget->setMinimumWidth(minWidth);
    widget->setMaximumWidth(minWidth + 100);
}

void QtBitcoinTrader::updateLogTable()
{
    emit getHistory(false);
}

void QtBitcoinTrader::balanceChanged(double)
{
    calcOrdersTotalValues();
    emit getHistory(true);
}

void QtBitcoinTrader::ordersIsAvailable()
{
    if (ui.ordersTableFrame->isVisible())
        return;

    ui.noOpenedOrdersLabel->setVisible(false);
    ui.ordersTableFrame->setVisible(true);
}

void QtBitcoinTrader::ordersIsEmpty()
{
    if (ordersModel->rowCount())
    {
        if (debugLevel)
            logThread->writeLog("Order table cleared");

        ordersModel->clear();
        setSpinValue(ui.ordersTotalBTC, 0.0);
        setSpinValue(ui.ordersTotalUSD, 0.0);
        ui.ordersTableFrame->setVisible(false);
        ui.noOpenedOrdersLabel->setVisible(true);
    }

    // calcOrdersTotalValues();
}

void QtBitcoinTrader::orderCanceled(const QString& symbol, QByteArray oid)
{
    if (debugLevel)
        logThread->writeLog("Removed order: " + oid + " " + symbol.toLatin1());

    ordersModel->setOrderCanceled(oid);
}

void QtBitcoinTrader::orderBookChanged(const QString& symbol, QList<OrderItem>* orders)
{
    if (symbol != baseValues.currentPair.symbol)
    {
        delete orders;
        return;
    }

    currentlyAddingOrders = true;
    ordersModel->orderBookChanged(orders);

    calcOrdersTotalValues();
    checkValidOrdersButtons();

    depthAsksModel->reloadVisibleItems();
    depthBidsModel->reloadVisibleItems();
    currentlyAddingOrders = false;

    ui.tableHistory->resizeColumnToContents(0);
    ui.tableHistory->resizeColumnToContents(2);
    ui.tableHistory->resizeColumnToContents(3);
    ui.tableHistory->resizeColumnToContents(4);
    ui.tableHistory->resizeColumnToContents(5);
    ui.tableHistory->resizeColumnToContents(6);
}

void QtBitcoinTrader::showErrorMessage(const QString& message)
{
    if (!showingMessage && lastMessageTime.elapsed() > 10000)
    {
        showingMessage = true;

        if (debugLevel)
            logThread->writeLog(baseValues.exchangeName.toLatin1() + " Error: " + message.toUtf8(), 2);

        lastMessageTime.restart();

        if (message.startsWith("I:>"))
            identificationRequired(message.right(message.size() - 3));
        else
            QMessageBox::warning(windowWidget, julyTr("AUTH_ERROR", "%1 Error").arg(baseValues.exchangeName), message);

        showingMessage = false;
    }
}

void QtBitcoinTrader::identificationRequired(QString message)
{
    if (!message.isEmpty())
        message.prepend("<br><br>");

    message.prepend(julyTr("TRUNAUTHORIZED", "Identification required to access private API.<br>Please enter valid API key and Secret."));

    QMessageBox::warning(windowWidget, julyTr("AUTH_ERROR", "%1 Error").arg(baseValues.exchangeName), message);
}

void QtBitcoinTrader::historyChanged(QList<HistoryItem>* historyItems)
{
    historyModel->historyChanged(historyItems);
    ui.tableHistory->resizeColumnToContents(1);
    ui.tableHistory->resizeColumnToContents(2);
    ui.tableHistory->resizeColumnToContents(3);
    ui.tableHistory->resizeColumnToContents(4);
    ui.tableHistory->resizeColumnToContents(5);

    if (debugLevel)
        logThread->writeLog("Log Table Updated");
}

void QtBitcoinTrader::accLastSellChanged(const QString& priceCurrency, double val)
{
    setSpinValue(ui.ordersLastSellPrice, val);

    if (ui.usdLabelLastSell->toolTip() != priceCurrency)
    {
        QPixmap btcPixmap;
        currencySignLoader->getCurrencySign(priceCurrency.toUpper(), btcPixmap);
        ui.usdLabelLastSell->setPixmap(btcPixmap);
        ui.usdLabelLastSell->setToolTip(priceCurrency);
    }
}

void QtBitcoinTrader::accLastBuyChanged(const QString& priceCurrency, double val)
{
    setSpinValue(ui.ordersLastBuyPrice, val);

    if (ui.usdLabelLastBuy->toolTip() != priceCurrency)
    {
        QPixmap btcPixmap;
        currencySignLoader->getCurrencySign(priceCurrency.toUpper(), btcPixmap);
        ui.usdLabelLastBuy->setPixmap(btcPixmap);
        ui.usdLabelLastBuy->setToolTip(priceCurrency);
    }
}

void QtBitcoinTrader::cancelOrder(const QString& symbol, const QByteArray& oid)
{
    emit cancelOrderByOid(symbol, oid);
}

void QtBitcoinTrader::on_ordersCancelBidsButton_clicked()
{
    QString cancelSymbol;

    if (ui.ordersFilterCheckBox->isChecked())
        cancelSymbol = IniEngine::getPairSymbol(ui.filterOrdersCurrency->currentIndex());

    ordersModel->ordersCancelBids(cancelSymbol);
}

void QtBitcoinTrader::on_ordersCancelAsksButton_clicked()
{
    QString cancelSymbol;

    if (ui.ordersFilterCheckBox->isChecked())
        cancelSymbol = IniEngine::getPairSymbol(ui.filterOrdersCurrency->currentIndex());

    ordersModel->ordersCancelAsks(cancelSymbol);
}

void QtBitcoinTrader::on_ordersCancelAllButton_clicked()
{
    QString cancelSymbol;

    if (ui.ordersFilterCheckBox->isChecked())
        cancelSymbol = IniEngine::getPairSymbol(ui.filterOrdersCurrency->currentIndex());

    ordersModel->ordersCancelAll(cancelSymbol);
}

void QtBitcoinTrader::cancelPairOrders(const QString& symbol)
{
    ordersModel->ordersCancelAll(symbol);
}

void QtBitcoinTrader::cancelAskOrders(const QString& symbol)
{
    ordersModel->ordersCancelAsks(symbol);
}

void QtBitcoinTrader::cancelBidOrders(const QString& symbol)
{
    ordersModel->ordersCancelBids(symbol);
}

void QtBitcoinTrader::cancelAllCurrentPairOrders()
{
    cancelPairOrders(baseValues.currentPair.symbol);
}

void QtBitcoinTrader::on_ordersCancelSelected_clicked()
{
    QModelIndexList selectedRows = ui.ordersTable->selectionModel()->selectedRows();

    if (selectedRows.size() == 0)
        return;

    for (int n = 0; n < selectedRows.size(); n++)
    {
        QByteArray oid = selectedRows.at(n).data(Qt::UserRole).toByteArray();

        if (!oid.isEmpty())
            cancelOrder(selectedRows.at(n).data(Qt::AccessibleTextRole).toString(), oid);
    }
}

void QtBitcoinTrader::cancelOrderByXButton()
{
    QPushButton* buttonCancel = dynamic_cast<QPushButton*>(sender());

    if (!buttonCancel)
        return;

    QByteArray order = buttonCancel->property("OrderId").toByteArray();

    if (!order.isEmpty())
        cancelOrder(buttonCancel->property("Symbol").toString(), order);
}

void QtBitcoinTrader::on_buttonNight_clicked()
{
    baseValues.currentTheme++;

    if (baseValues.currentTheme > 2)
        baseValues.currentTheme = 0;

    if (baseValues.currentTheme == 1)
    {
        baseValues.appTheme = baseValues.appThemeDark;
#if QT_VERSION < 0x050000
        qApp->setStyle(new QPlastiqueStyle);
#endif
    }
    else if (baseValues.currentTheme == 2)
    {
        baseValues.appTheme = baseValues.appThemeGray;
        qApp->setStyle(baseValues.osStyle);
    }
    else if (baseValues.currentTheme == 0)
    {
        baseValues.appTheme = baseValues.appThemeLight;
        qApp->setStyle(baseValues.osStyle);
    }

    qApp->setPalette(baseValues.appTheme.palette);
    qApp->setStyleSheet(baseValues.appTheme.styleSheet);

    ui.accountLoginLabel->setStyleSheet("color: " + baseValues.appTheme.black.name() + "; background: " + baseValues.appTheme.white.name());
    ui.noOpenedOrdersLabel->setStyleSheet("font-size:27px; border: 1px solid gray; background: " + baseValues.appTheme.white.name() +
                                          "; color: " + baseValues.appTheme.gray.name());
    ui.rulesNoMessage->setStyleSheet("font-size:27px; border: 1px solid gray; background: " + baseValues.appTheme.white.name() +
                                     "; color: " + baseValues.appTheme.gray.name());

    ui.buyBitcoinsButton->setStyleSheet("QPushButton {font-size:21px; color: " + baseValues.appTheme.blue.name() +
                                        "} QPushButton::disabled {color: " + baseValues.appTheme.gray.name() + "}");
    ui.sellBitcoinsButton->setStyleSheet("QPushButton {font-size:21px; color: " + baseValues.appTheme.red.name() +
                                         "} QPushButton::disabled {color: " + baseValues.appTheme.gray.name() + "}");

    profitLossSpinBox_valueChanged(profitLossSpinBox->value());
    sellThanBuySpinBox_valueChanged(sellThanBuySpinBox->value());
    buyTotalSpend_valueChanged(buyTotalSpend->value());
    sellTotalBtc_valueChanged(sellTotalBtc->value());

    for (RuleWidget* currentGroup : ui.tabRules->findChildren<RuleWidget*>())
        currentGroup->updateStyleSheets();

    if (baseValues.currentTheme == 2)
        ui.buttonNight->setIcon(QIcon("://Resources/Day.png"));
    else if (baseValues.currentTheme == 0)
        ui.buttonNight->setIcon(QIcon("://Resources/Night.png"));
    else
        ui.buttonNight->setIcon(QIcon("://Resources/Gray.png"));

    if (swapedDepth)
    {
        ui.asksLabel->setStyleSheet("color: " + baseValues.appTheme.blue.name());
        ui.bidsLabel->setStyleSheet("color: " + baseValues.appTheme.red.name());
    }
    else
    {
        ui.asksLabel->setStyleSheet("color: " + baseValues.appTheme.red.name());
        ui.bidsLabel->setStyleSheet("color: " + baseValues.appTheme.blue.name());
    }

    iniSettings->setValue("UI/NightMode", baseValues.currentTheme);

    emit themeChanged();

    chartsView->setStyleSheet("background: " + baseValues.appTheme.white.name());
    chartsView->refreshCharts();

    if (baseValues.currentTheme == 1)
        ui.widgetLogo->setStyleSheet("background:black");
    else
        ui.widgetLogo->setStyleSheet("background:white");
}

void QtBitcoinTrader::on_calcButton_clicked()
{
    if (feeCalculatorSingleInstance && feeCalculator)
        feeCalculator->activateWindow();
    else
        feeCalculator = new FeeCalculator;
}

void QtBitcoinTrader::checkValidSellButtons()
{
    ui.widgetSellThenBuy->setEnabled(sellTotalBtc->value() >= baseValues.currentPair.tradeVolumeMin &&
                                     sellAmountToReceive->value() >= baseValues.currentPair.tradeTotalMin);
    ui.sellBitcoinsButton->setEnabled(ui.widgetSellThenBuy->isEnabled() &&
                                      /*ui.sellTotalBtc->value()<=getAvailableBTC()&&*/ sellTotalBtc->value() > 0.0);
}

void QtBitcoinTrader::on_sellPricePerCoinAsMarketLastPrice_clicked()
{
    sellPricePerCoin->setValue(ui.marketLast->value());
}

void QtBitcoinTrader::on_sellTotalBtcAllIn_clicked()
{
    sellTotalBtc->setValue(getAvailableBTC());
}

void QtBitcoinTrader::on_sellTotalBtcHalfIn_clicked()
{
    sellTotalBtc->setValue(getAvailableBTC() / 2.0);
}

void QtBitcoinTrader::setDataPending(bool on)
{
    isDataPending = on;
}

void QtBitcoinTrader::setSoftLagValue(int mseconds)
{
    if (secondTimer.isNull())
        return;

    if (!isDataPending && mseconds < baseValues.httpRequestTimeout)
        mseconds = 0;

    static int lastSoftLag = -1;

    if (lastSoftLag == mseconds)
        return;

    ui.lastUpdate->setValue(mseconds / 1000.0);

    static bool lastSoftLagValid = true;
    isValidSoftLag = mseconds <= baseValues.httpRequestTimeout + baseValues.httpRequestInterval + 200;

    if (isValidSoftLag != lastSoftLagValid)
    {
        lastSoftLagValid = isValidSoftLag;

        if (!isValidSoftLag)
            ui.lastUpdate->setStyleSheet("QDoubleSpinBox {background: " + baseValues.appTheme.lightRed.name() + ";}");
        else
            ui.lastUpdate->setStyleSheet("");

        calcOrdersTotalValues();
        // ui.ordersControls->setEnabled(isValidSoftLag);
        // ui.buyButtonBack->setEnabled(isValidSoftLag);
        // ui.sellButtonBack->setEnabled(isValidSoftLag);
        QString toolTip;

        if (!isValidSoftLag)
            toolTip = julyTr("TOOLTIP_API_LAG_TO_HIGH", "API Lag is to High");

        ui.ordersControls->setToolTip(toolTip);
        ui.buyButtonBack->setToolTip(toolTip);
        ui.sellButtonBack->setToolTip(toolTip);
    }
}

void QtBitcoinTrader::sellTotalBtc_valueChanged(double val)
{
    if (val == 0.0)
        sellTotalBtc->setStyleSheet("QDoubleSpinBox {background: " + baseValues.appTheme.lightRed.name() + ";}");
    else
        sellTotalBtc->setStyleSheet("");

    profitBuyThanSellCalc();
    profitSellThanBuyCalc();

    if (sellLockBtcToSell)
        return;

    sellLockBtcToSell = true;

    sellLockAmountToReceive = true;
    sellAmountToReceive->setValue(sellPricePerCoin->value() * val * floatFeeDec);

    sellLockAmountToReceive = false;

    checkValidSellButtons();
    sellLockBtcToSell = false;
}

void QtBitcoinTrader::sellPricePerCoin_valueChanged(double)
{
    if (!sellLockPricePerCoin)
    {
        sellLockPricePerCoin = true;
        sellTotalBtc_valueChanged(sellTotalBtc->value());
        sellLockPricePerCoin = false;
    }

    ui.sellNextMaxBuyPrice->setValue(sellPricePerCoin->value() * floatFeeDec * floatFeeDec - baseValues.currentPair.priceMin);
    ui.sellNextMaxBuyStep->setValue(sellPricePerCoin->value() - ui.sellNextMaxBuyPrice->value());
    checkValidSellButtons();
    ui.buttonSellThenBuyApply->setEnabled(true);
    profitBuyThanSellCalc();
    profitSellThanBuyCalc();
}

void QtBitcoinTrader::sellAmountToReceive_valueChanged(double val)
{
    profitBuyThanSellCalc();
    profitSellThanBuyCalc();

    if (sellLockAmountToReceive)
        return;

    sellLockAmountToReceive = true;

    sellLockBtcToSell = true;
    sellLockPricePerCoin = true;

    sellTotalBtc->setValue(val / sellPricePerCoin->value() / floatFeeDec);

    sellLockPricePerCoin = false;
    sellLockBtcToSell = false;

    sellLockAmountToReceive = false;
    checkValidSellButtons();
}

void QtBitcoinTrader::sellBitcoinButton()
{
    checkValidSellButtons();

    if (ui.sellBitcoinsButton->isEnabled() == false)
        return;

    double sellTotalBtcV = sellTotalBtc->value();
    double sellPricePerCoinV = sellPricePerCoin->value();

    if (confirmOpenOrder)
    {
        QMessageBox msgBox(windowWidget);
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setWindowTitle(julyTr("MESSAGE_CONFIRM_SELL_TRANSACTION", "Please confirm transaction"));
        msgBox.setText(
            julyTr(
                "MESSAGE_CONFIRM_SELL_TRANSACTION_TEXT",
                "Are you sure to sell %1 at %2 ?<br><br>Note: If total orders amount of your Bitcoins exceeds your balance, %3 will remove this order immediately.")
                .arg(baseValues.currentPair.currASign + " " + JulyMath::textFromDouble(sellTotalBtcV, baseValues.currentPair.currADecimals))
                .arg(baseValues.currentPair.currBSign + " " +
                     JulyMath::textFromDouble(sellPricePerCoinV, baseValues.currentPair.priceDecimals))
                .arg(baseValues.exchangeName));

        auto buttonYes = msgBox.addButton(julyTr("YES", "Yes"), QMessageBox::YesRole);
        msgBox.addButton(julyTr("NO", "No"), QMessageBox::NoRole);
        msgBox.exec();
        if (msgBox.clickedButton() != buttonYes)
            return;
    }

    apiSellSend(baseValues.currentPair.symbolSecond(), sellTotalBtcV, sellPricePerCoinV);
}

void QtBitcoinTrader::buyTotalSpend_valueChanged(double val)
{
    if (val == 0.0)
        buyTotalSpend->setStyleSheet("QDoubleSpinBox {background: " + baseValues.appTheme.lightRed.name() + ";}");
    else
        buyTotalSpend->setStyleSheet("");

    profitBuyThanSellCalc();
    profitSellThanBuyCalc();

    buyLockTotalBtc = true;

    if (!buyLockTotalBtcSelf)
        buyTotalBtc->setValue(val / buyPricePerCoin->value());

    buyLockTotalBtc = false;

    double valueForResult = val / buyPricePerCoin->value();
    valueForResult *= floatFeeDec;
    valueForResult = JulyMath::cutDoubleDecimalsCopy(valueForResult, baseValues.currentPair.currADecimals, false);
    setSpinValue(ui.buyTotalBtcResult, valueForResult);

    checkValidBuyButtons();
}

void QtBitcoinTrader::sendIndicatorEvent(const QString& symbol, const QString& name, double val)
{
    emit indicatorEventSignal(symbol, name, val);
}

void QtBitcoinTrader::buyTotalBtc_valueChanged(double)
{
    if (buyLockTotalBtc)
    {
        checkValidBuyButtons();
        return;
    }

    buyLockTotalBtc = true;
    buyLockTotalBtcSelf = true;

    buyTotalSpend->setValue(buyTotalBtc->value() * buyPricePerCoin->value());
    buyLockTotalBtcSelf = false;
    buyLockTotalBtc = false;
    checkValidBuyButtons();
}

void QtBitcoinTrader::buyPricePerCoin_valueChanged(double)
{
    if (!buyLockPricePerCoin)
    {
        buyLockPricePerCoin = true;
        buyTotalSpend_valueChanged(buyTotalSpend->value());
        buyLockPricePerCoin = false;
    }

    ui.buyNextInSellPrice->setValue(buyPricePerCoin->value() * floatFeeInc * floatFeeInc + baseValues.currentPair.priceMin);
    ui.buyNextMinBuyStep->setValue(ui.buyNextInSellPrice->value() - buyPricePerCoin->value());
    checkValidBuyButtons();
    ui.buttonBuyThenSellApply->setEnabled(true);
    profitBuyThanSellCalc();
    profitSellThanBuyCalc();
}

void QtBitcoinTrader::checkValidBuyButtons()
{
    ui.widgetBuyThenSell->setEnabled(buyTotalBtc->value() >= baseValues.currentPair.tradeVolumeMin &&
                                     buyTotalSpend->value() >= baseValues.currentPair.tradeTotalMin);
    ui.buyBitcoinsButton->setEnabled(ui.widgetBuyThenSell->isEnabled() &&
                                     /*ui.buyTotalSpend->value()<=getAvailableUSD()&&*/ buyTotalSpend->value() > 0.0);
}

void QtBitcoinTrader::checkValidOrdersButtons()
{
    ui.ordersCancelAllButton->setEnabled(ordersModel->rowCount());
    ui.ordersCancelSelected->setEnabled(ui.ordersTable->selectionModel()->selectedRows().size());
}

void QtBitcoinTrader::on_buyTotalBtcAllIn_clicked()
{
    buyTotalBtc->setValue(getAvailableUSDtoBTC(buyPricePerCoin->value()));
}

void QtBitcoinTrader::on_buyTotalBtcHalfIn_clicked()
{
    buyTotalBtc->setValue(getAvailableUSDtoBTC(buyPricePerCoin->value()) / 2.0);
}

void QtBitcoinTrader::on_sellPriceAsMarketBid_clicked()
{
    sellPricePerCoin->setValue(ui.marketBid->value());
}

void QtBitcoinTrader::on_buyPriceAsMarketBid_clicked()
{
    buyPricePerCoin->setValue(ui.marketBid->value());
}

void QtBitcoinTrader::on_sellPriceAsMarketAsk_clicked()
{
    sellPricePerCoin->setValue(ui.marketAsk->value());
}

void QtBitcoinTrader::on_buyPriceAsMarketAsk_clicked()
{
    buyPricePerCoin->setValue(ui.marketAsk->value());
}

void QtBitcoinTrader::on_buyPriceAsMarketLastPrice_clicked()
{
    buyPricePerCoin->setValue(ui.marketLast->value());
}

bool QtBitcoinTrader::hasWorkingRules()
{
    for (RuleWidget* group : ui.tabRules->findChildren<RuleWidget*>())
    {
        if (group)
        {
            if (group->haveWorkingRules())
                return true;
        }
    }

    for (ScriptWidget* group : ui.tabRules->findChildren<ScriptWidget*>())
    {
        if (group)
        {
            if (group->isRunning())
                return true;
        }
    }

    return false;
}

void QtBitcoinTrader::buyBitcoinsButton()
{
    checkValidBuyButtons();

    if (ui.buyBitcoinsButton->isEnabled() == false)
        return;

    double btcToBuy = 0.0;
    double priceToBuy = buyPricePerCoin->value();

    if (currentExchange->buySellAmountExcludedFee && floatFee != 0.0)
        btcToBuy = ui.buyTotalBtcResult->value();
    else
        btcToBuy = buyTotalBtc->value();

    // double amountWithoutFee=getAvailableUSD()/priceToBuy;
    // amountWithoutFee=JulyMath::cutDoubleDecimalsCopy(amountWithoutFee,baseValues.currentPair.currADecimals,false);

    if (confirmOpenOrder)
    {
        QMessageBox msgBox(windowWidget);
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setWindowTitle(julyTr("MESSAGE_CONFIRM_BUY_TRANSACTION", "Please confirm new order"));
        msgBox.setText(
            julyTr(
                "MESSAGE_CONFIRM_BUY_TRANSACTION_TEXT",
                "Are you sure to buy %1 at %2 ?<br><br>Note: If total orders amount of your funds exceeds your balance, %3 will remove this order immediately.")
                .arg(baseValues.currentPair.currASign + " " + JulyMath::textFromDouble(btcToBuy, baseValues.currentPair.currADecimals))
                .arg(baseValues.currentPair.currBSign + " " + JulyMath::textFromDouble(priceToBuy, baseValues.currentPair.priceDecimals))
                .arg(baseValues.exchangeName));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);
        auto buttonYes = msgBox.addButton(julyTr("YES", "Yes"), QMessageBox::YesRole);
        msgBox.addButton(julyTr("NO", "No"), QMessageBox::NoRole);
        msgBox.exec();
        if (msgBox.clickedButton() != buttonYes)
            return;
    }

    apiBuySend(baseValues.currentPair.symbolSecond(), btcToBuy, priceToBuy);
}

void QtBitcoinTrader::play(const QString& wav, bool noBlink)
{
    Platform::playSound(wav);

    if (!noBlink)
        blinkWindow();
}

void QtBitcoinTrader::beep(bool noBlink)
{
    QString fileName = appDataDir + "/Sound/Beep.wav";

    if (!QFile::exists(fileName))
    {
        QDir().mkpath(QFileInfo(fileName).dir().path());
        QFile::copy(":/Resources/Sound/800.wav", fileName);
    }

    if (!QFile::exists(fileName))
        QApplication::beep();
    else
        play(fileName, noBlink);

    if (!noBlink)
        blinkWindow();
}

void QtBitcoinTrader::blinkWindow()
{
#ifdef Q_OS_WIN

    if (!isActiveWindow())
    {
        FLASHWINFO flashInfo;
        flashInfo.cbSize = sizeof(FLASHWINFO);
        flashInfo.hwnd = reinterpret_cast<HWND>(windowWidget->winId());
        flashInfo.dwFlags = FLASHW_ALL;
        flashInfo.uCount = 10;
        flashInfo.dwTimeout = 400;
        ::FlashWindowEx(&flashInfo);
    }

#endif // ToDo: make this feature works on Mac OS X
}

void QtBitcoinTrader::ruleTotalToBuyValueChanged()
{
    if (ui.marketLast->value() == 0.0)
        return;

    double newValue = ui.accountUSD->value() / ui.marketLast->value() * floatFeeDec;

    if (!qFuzzyCompare(newValue, ui.ruleTotalToBuyValue->value()))
    {
        setSpinValueP(ui.ruleTotalToBuyValue, newValue);
    }
}

void QtBitcoinTrader::ruleAmountToReceiveValueChanged()
{
    if (ui.marketLast->value() == 0.0)
        return;

    double newValue = ui.accountBTC->value() * ui.marketLast->value() * floatFeeDec;

    if (!qFuzzyCompare(newValue, ui.ruleAmountToReceiveValue->value()))
    {
        setSpinValueP(ui.ruleAmountToReceiveValue, newValue);
    }
}

void QtBitcoinTrader::ruleTotalToBuyBSValueChanged()
{
    if (ui.marketBid->value() == 0.0)
        return;

    double newValue = ui.accountUSD->value() / ui.marketBid->value() * floatFeeDec;

    if (!qFuzzyCompare(newValue, ui.ruleTotalToBuyValue->value()))
    {
        setSpinValueP(ui.ruleTotalToBuyBSValue, newValue);
    }
}

void QtBitcoinTrader::ruleAmountToReceiveBSValueChanged()
{
    if (ui.marketAsk->value() == 0.0)
        return;

    double newValue = ui.accountBTC->value() * ui.marketAsk->value() * floatFeeDec;

    if (!qFuzzyCompare(newValue, ui.ruleAmountToReceiveBSValue->value()))
    {
        setSpinValueP(ui.ruleAmountToReceiveBSValue, newValue);
    }
}

void QtBitcoinTrader::on_accountUSD_valueChanged(double)
{
    ruleTotalToBuyValueChanged();
    ruleTotalToBuyBSValueChanged();
}

void QtBitcoinTrader::on_accountBTC_valueChanged(double)
{
    ruleAmountToReceiveValueChanged();
    ruleAmountToReceiveBSValueChanged();
}

void QtBitcoinTrader::on_marketBid_valueChanged(double val)
{
    ruleTotalToBuyBSValueChanged();
    meridianPrice = (val + ui.marketAsk->value()) / 2;

    if (val > 0.000000001)
    {
        emit addBound(val, false);

        double ask = ui.marketAsk->value();

        if (ask > 0.000000001)
        {
            val = (val + ask) / 2;

            static double lastValue = val;
            static int priceDirection = 0;
            int lastPriceDirection = priceDirection;

            if (lastValue < val)
                priceDirection = 1;
            else if (lastValue > val)
                priceDirection = -1;
            else
                priceDirection = lastPriceDirection;

            lastValue = val;

            static QString directionChar("-");

            switch (priceDirection)
            {
            case -1:
                directionChar = downArrowNoUtfStr;
                break;

            case 1:
                directionChar = upArrowNoUtfStr;
                break;

            default:
                break;
            }

            static QString titleText;
            titleText = baseValues.currentPair.currBSign + " " + JulyMath::textFromDouble(val) + " " + directionChar + " " + windowTitleP;

            if (windowWidget->isVisible())
                windowWidget->setWindowTitle(titleText);

            if (trayIcon && trayIcon->isVisible())
                trayIcon->setToolTip(titleText);
        }
    }
}

void QtBitcoinTrader::on_marketAsk_valueChanged(double val)
{
    ruleAmountToReceiveBSValueChanged();
    meridianPrice = (val + ui.marketBid->value()) / 2;

    if (val > 0.000000001)
    {
        emit addBound(val, true);

        double bid = ui.marketBid->value();

        if (bid > 0.000000001)
        {
            val = (val + bid) / 2;

            static double lastValue = val;
            static int priceDirection = 0;
            int lastPriceDirection = priceDirection;

            if (lastValue < val)
                priceDirection = 1;
            else if (lastValue > val)
                priceDirection = -1;
            else
                priceDirection = lastPriceDirection;

            lastValue = val;

            static QString directionChar("-");

            switch (priceDirection)
            {
            case -1:
                directionChar = downArrowNoUtfStr;
                break;

            case 1:
                directionChar = upArrowNoUtfStr;
                break;

            default:
                break;
            }

            static QString titleText;
            titleText = baseValues.currentPair.currBSign + " " + JulyMath::textFromDouble(val) + " " + directionChar + " " + windowTitleP;

            if (windowWidget->isVisible())
                windowWidget->setWindowTitle(titleText);

            if (trayIcon && trayIcon->isVisible())
                trayIcon->setToolTip(titleText);
        }
    }
}

void QtBitcoinTrader::on_marketLast_valueChanged(double)
{
    ruleTotalToBuyValueChanged();
    ruleAmountToReceiveValueChanged();
}

void QtBitcoinTrader::historyDoubleClicked(QModelIndex index)
{
    repeatOrderFromValues(0, historyModel->getRowPrice(index.row()), historyModel->getRowVolume(index.row()), false);
}

void QtBitcoinTrader::repeatOrderFromValues(int type, double itemPrice, double itemVolume, bool availableOnly)
{
    if (itemPrice == 0.0)
        return;

    if (QApplication::keyboardModifiers() != Qt::NoModifier)
        availableOnly = !availableOnly;

    if (type == 1 || type == 0)
    {
        buyPricePerCoin->setValue(itemPrice);

        if (availableOnly)
            itemVolume = qMin(itemVolume, getAvailableUSD() / itemPrice);

        buyTotalBtc->setValue(itemVolume);
    }

    if (type == -1 || type == 0)
    {
        sellPricePerCoin->setValue(itemPrice);

        if (availableOnly)
            itemVolume = qMin(getAvailableBTC(), itemVolume);

        sellTotalBtc->setValue(itemVolume);
    }
}

void QtBitcoinTrader::repeatOrderFromTrades(int type, int row)
{
    repeatOrderFromValues(type, tradesModel->getRowPrice(row), tradesModel->getRowVolume(row));
}

void QtBitcoinTrader::tradesDoubleClicked(QModelIndex index)
{
    repeatOrderFromTrades(0, index.row());
}

void QtBitcoinTrader::depthSelectOrder(QModelIndex index, bool isSell, int type)
{
    double itemPrice = 0.0;
    double itemVolume = 0.0;
    int row = index.row();
    int col = index.column();

    if (swapedDepth)
        isSell = !isSell;

    if (isSell)
    {
        if (!swapedDepth)
            col = depthAsksModel->columnCount() - col - 1;

        if (row < 0 || depthAsksModel->rowCount() <= row)
            return;

        itemPrice = depthAsksModel->rowPrice(row);

        if (col == 0 || col == 1)
            itemVolume = depthAsksModel->rowVolume(row);
        else
            itemVolume = depthAsksModel->rowSize(row);
    }
    else
    {
        if (swapedDepth)
            col = depthBidsModel->columnCount() - col - 1;

        if (row < 0 || depthBidsModel->rowCount() <= row)
            return;

        itemPrice = depthBidsModel->rowPrice(row);

        if (col == 0 || col == 1)
            itemVolume = depthBidsModel->rowVolume(row);
        else
            itemVolume = depthBidsModel->rowSize(row);
    }

    repeatOrderFromValues(type, itemPrice, itemVolume * floatFeeDec);
}

void QtBitcoinTrader::depthSelectSellOrder(QModelIndex index)
{
    depthSelectOrder(index, true);
}

void QtBitcoinTrader::depthSelectBuyOrder(QModelIndex index)
{
    depthSelectOrder(index, false);
}

void QtBitcoinTrader::translateTab(QWidget* tab)
{
    QDockWidget* dock = static_cast<QDockWidget*>(tab->parentWidget());

    if (dock)
    {
        QString key = tab->accessibleName();
        QString defaultValue = dock->windowTitle();
        QString s = julyTr(key, defaultValue);

        if (dock->isFloating())
        {
            s += " [" + profileName + "]";
        }

        tab->parentWidget()->setWindowTitle(s);

        if (dock->isFloating())
        {
            julyTranslator.translateUi(tab);
            fixAllChildButtonsAndLabels(tab);
        }
    }
}

void QtBitcoinTrader::lockLogo(bool lock)
{
    if (lock)
    {
        dockLogo->setFeatures(QDockWidget::NoDockWidgetFeatures);
    }
    else
    {
        dockLogo->setFeatures(QDockWidget::DockWidgetMovable);
    }

    dockLogo->setAllowedAreas(Qt::AllDockWidgetAreas);
}

void QtBitcoinTrader::initConfigMenu()
{
    menuConfig->clear();
    menuConfig->addAction(actionConfigManager);
    menuConfig->addSeparator();

    for (const QString& name : ::config->getConfigNames())
    {
        QAction* action = menuConfig->addAction(name);
        connect(action, &QAction::triggered, this, &QtBitcoinTrader::onMenuConfigTriggered);
    }
}

void QtBitcoinTrader::languageChanged()
{
    if (!constructorFinished)
        return;

    julyTranslator.translateUi(this);
    julyTranslator.translateUi(networkMenu);
    baseValues.dateTimeFormat = julyTr("DATETIME_FORMAT", baseValues.dateTimeFormat);
    baseValues.timeFormat = julyTr("TIME_FORMAT", baseValues.timeFormat);
    QStringList ordersLabels;
    ordersLabels << julyTr("ORDERS_COUNTER", "#") << julyTr("ORDERS_DATE", "Date") << julyTr("ORDERS_TYPE", "Type")
                 << julyTr("ORDERS_STATUS", "Status") << julyTr("ORDERS_AMOUNT", "Amount") << julyTr("ORDERS_PRICE", "Price")
                 << julyTr("ORDERS_TOTAL", "Total");
    ordersModel->setHorizontalHeaderLabels(ordersLabels);

    QStringList tradesLabels;
    tradesLabels << "" << julyTr("ORDERS_DATE", "Date") << julyTr("ORDERS_AMOUNT", "Amount") << julyTr("ORDERS_TYPE", "Type")
                 << julyTr("ORDERS_PRICE", "Price") << julyTr("ORDERS_TOTAL", "Total") << "";
    historyModel->setHorizontalHeaderLabels(tradesLabels);
    tradesLabels.insert(4, upArrowStr + downArrowStr);
    tradesModel->setHorizontalHeaderLabels(tradesLabels);

    QStringList depthHeaderLabels;
    depthHeaderLabels << julyTr("ORDERS_PRICE", "Price") << julyTr("ORDERS_AMOUNT", "Amount") << upArrowStr + downArrowStr
                      << julyTr("ORDERS_TOTAL", "Total") << "";
    depthBidsModel->setHorizontalHeaderLabels(depthHeaderLabels);
    depthAsksModel->setHorizontalHeaderLabels(depthHeaderLabels);

    translateTab(ui.tabOrdersLog);
    translateTab(ui.tabRules);
    translateTab(ui.tabLastTrades);
    translateTab(ui.tabDepth);
    translateTab(ui.tabCharts);
    translateTab(ui.tabNews);
    // translateTab(ui.tabChat);

    ui.widgetAccount->parentWidget()->setWindowTitle(julyTr("ACCOUNT_GROUPBOX", "%1 Account").arg(baseValues.exchangeName));

    QString curCurrencyName = IniEngine::getCurrencyInfo(baseValues.currentPair.currAStr).name;
    ui.widgetBuy->parentWidget()->setWindowTitle(julyTr("GROUPBOX_BUY", "Buy %1").arg(curCurrencyName));
    ui.widgetSell->parentWidget()->setWindowTitle(julyTr("GROUPBOX_SELL", "Sell %1").arg(curCurrencyName));

    for (QToolButton* toolButton : findChildren<QToolButton*>())
        if (toolButton->accessibleDescription() == "TOGGLE_SOUND")
            toolButton->setToolTip(julyTr("TOGGLE_SOUND", "Toggle sound notification on value change"));

    if (ui.comboBoxGroupByPrice->count())
    {
        ui.comboBoxGroupByPrice->setItemText(0, julyTr("DONT_GROUP", "None"));
        fixWidthComboBoxGroupByPrice();
    }

    copyTableValuesMenu.actions().at(0)->setText(julyTr("COPY_ROW", "Copy selected Rows"));

    copyTableValuesMenu.actions().at(2)->setText(julyTr("COPY_DATE", "Copy Date"));
    copyTableValuesMenu.actions().at(3)->setText(julyTr("COPY_AMOUNT", "Copy Amount"));
    copyTableValuesMenu.actions().at(4)->setText(julyTr("COPY_PRICE", "Copy Price"));
    copyTableValuesMenu.actions().at(5)->setText(julyTr("COPY_TOTAL", "Copy Total"));

    copyTableValuesMenu.actions().at(7)->setText(julyTr("REPEAT_BUYSELL_ORDER", "Repeat Buy and Sell order"));
    copyTableValuesMenu.actions().at(8)->setText(julyTr("REPEAT_BUY_ORDER", "Repeat Buy order"));
    copyTableValuesMenu.actions().at(9)->setText(julyTr("REPEAT_SELL_ORDER", "Repeat Sell order"));

    copyTableValuesMenu.actions().at(11)->setText(julyTr("CANCEL_ORDER", "Cancel selected Orders"));
    copyTableValuesMenu.actions().at(12)->setText(julyTranslator.translateCheckBox("TR00075", "Cancel All Orders"));

    ui.tradesBidsPrecent->setToolTip(julyTr("10_MIN_BIDS_VOLUME", "(10 min Bids Volume)/(10 min Asks Volume)*100"));

    for (RuleWidget* currentGroup : ui.tabRules->findChildren<RuleWidget*>())
        if (currentGroup)
            currentGroup->languageChanged();

    for (ScriptWidget* currentGroup : ui.tabRules->findChildren<ScriptWidget*>())
        if (currentGroup)
            currentGroup->languageChanged();

    actionLockDocks->setText(julyTr("LOCK_DOCKS", "&Lock Docks"));
    actionExit->setText(julyTr("EXIT", "E&xit"));
    actionUpdate->setText(julyTr("UPDATE", "Check for &updates"));
    actionSendBugReport->setText(julyTr("SEND_BUG_REPORT", "&Send bug report"));
    actionAbout->setText(julyTr("ABOUT", "&About Qt Bitcoin Trader"));
    actionAboutQt->setText(julyTr("ABOUT_QT", "About &Qt"));
#ifndef Q_OS_MAC

    if (!baseValues_->portableMode && actionUninstall)
        actionUninstall->setText(julyTr("UNINSTALL", "&Uninstall"));

#endif
    actionConfigManager->setText(julyTr("CONFIG_MANAGER", "&Save..."));
    actionSettings->setText(julyTr("CONFIG_SETTINGS", "Se&ttings"));
    actionDebug->setText(julyTr("CONFIG_DEBUG", "&Debug"));
    menuFile->setTitle("&QtBitcoinTrader");
    menuView->setTitle(julyTr("MENU_VIEW", "&View"));
    menuConfig->setTitle(julyTr("MENU_CONFIG", "&Interface"));
    menuHelp->setTitle(julyTr("MENU_HELP", "&Help"));

    if (configDialog)
        configDialog->setWindowTitle(julyTr("CONFIG_MANAGER_TITLE", "Config Manager"));

    ::config->translateDefaultNames();

    if (configDialog)
        ::config->onChanged();

    adjustDockMinSize(ui.widgetTotalAtLast);
    adjustDockMinSize(ui.widgetTotalAtBuySell);

    fixAllChildButtonsAndLabels(this);
}

void QtBitcoinTrader::buttonNewWindow()
{
    QProcess::startDetached(QApplication::applicationFilePath(), QStringList());
}

void QtBitcoinTrader::on_rulesTabs_tabCloseRequested(int tab)
{
    ScriptWidget* currentScript = dynamic_cast<ScriptWidget*>(ui.rulesTabs->widget(tab));
    RuleWidget* currentGroup = dynamic_cast<RuleWidget*>(ui.rulesTabs->widget(tab));

    if (currentGroup == nullptr && currentScript == nullptr)
        return;

    if (currentScript || currentGroup->haveAnyRules())
    {
        QMessageBox msgBox(windowWidget);
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setWindowTitle(julyTr("RULES_CONFIRM_DELETION", "Please confirm rules group deletion"));
        msgBox.setText(julyTr("RULES_ARE_YOU_SURE_TO_DELETE_GROUP", "Are you sure to delete rules group %1?")
                           .arg(ui.rulesTabs->widget(tab)->windowTitle()));
        auto buttonYes = msgBox.addButton(julyTr("YES", "Yes"), QMessageBox::YesRole);
        msgBox.addButton(julyTr("NO", "No"), QMessageBox::NoRole);
        msgBox.exec();
        if (msgBox.clickedButton() != buttonYes)
            return;
    }

    // bool removed = false;
    // Q_UNUSED(removed);

    if (currentGroup)
    {
        // removed = currentGroup->removeGroup();
        currentGroup->removeGroup();
    }
    else if (currentScript)
    {
        // removed = currentScript->removeGroup();
        currentScript->removeGroup();
    }

    delete ui.rulesTabs->widget(tab);

    ui.rulesTabs->setVisible(ui.rulesTabs->count());
    ui.rulesNoMessage->setVisible(!ui.rulesTabs->isVisible());
}

void QtBitcoinTrader::on_widgetStaysOnTop_toggled(bool on)
{
    bool visible = isVisible();

    if (on)
        windowWidget->setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint);
    else
        windowWidget->setWindowFlags(Qt::Window);

    dockHost->setStaysOnTop(on);

    if (visible)
        windowWidget->show();
}

void QtBitcoinTrader::depthFirstOrder(const QString& symbol, double price, double volume, bool isAsk)
{
    if (symbol != baseValues.currentPair.symbol)
        return;

    waitingDepthLag = false;

    if (price == 0.0 || ui.comboBoxGroupByPrice->currentIndex() == 0)
        return;

    if (isAsk)
        depthAsksModel->depthFirstOrder(price, volume);
    else
        depthBidsModel->depthFirstOrder(price, volume);
}

void QtBitcoinTrader::fixDepthBidsTable()
{
    ui.depthAsksTable->resizeColumnToContents(1);
    ui.depthAsksTable->resizeColumnToContents(3);
    ui.depthAsksTable->resizeColumnToContents(4);

    ui.depthBidsTable->resizeColumnToContents(0);
    ui.depthBidsTable->resizeColumnToContents(1);
    ui.depthBidsTable->resizeColumnToContents(3);

    int asksWidth = ui.depthAsksTable->columnWidth(1) + ui.depthAsksTable->columnWidth(2) + ui.depthAsksTable->columnWidth(3) +
                    ui.depthAsksTable->columnWidth(4) + 16;
    int bidsWidth = ui.depthBidsTable->columnWidth(0) + ui.depthBidsTable->columnWidth(1) + ui.depthBidsTable->columnWidth(2) +
                    ui.depthBidsTable->columnWidth(3) + 16;

    ui.depthAsksTable->setMinimumWidth(asksWidth);
    ui.depthAsksTable->horizontalScrollBar()->setValue(ui.depthAsksTable->horizontalScrollBar()->maximum());
    ui.depthBidsTable->setMinimumWidth(bidsWidth);
    ui.depthBidsTable->horizontalScrollBar()->setValue(0);

    ui.tabDepth->setMinimumWidth(qMax(ui.gridLayout_31->minimumSize().width(), asksWidth + bidsWidth + 24));
}

void QtBitcoinTrader::depthSubmitOrders(const QString& symbol, QList<DepthItem>* asks, QList<DepthItem>* bids)
{
    if (symbol != baseValues.currentPair.symbol)
        return;

    waitingDepthLag = false;
    int currentAsksScroll = ui.depthAsksTable->verticalScrollBar()->value();
    int currentBidsScroll = ui.depthBidsTable->verticalScrollBar()->value();
    depthAsksModel->depthUpdateOrders(asks);
    depthBidsModel->depthUpdateOrders(bids);
    depthVisibilityChanged(dockDepth->isVisible());
    ui.depthAsksTable->verticalScrollBar()->setValue(qMin(currentAsksScroll, ui.depthAsksTable->verticalScrollBar()->maximum()));
    ui.depthBidsTable->verticalScrollBar()->setValue(qMin(currentBidsScroll, ui.depthBidsTable->verticalScrollBar()->maximum()));

    fixDepthBidsTable();
}

void QtBitcoinTrader::saveAppState()
{
    for (RuleWidget* currentGroup : ui.tabRules->findChildren<RuleWidget*>())
        if (currentGroup)
            currentGroup->saveRulesData();

    for (ScriptWidget* currentGroup : ui.tabRules->findChildren<ScriptWidget*>())
        if (currentGroup)
            currentGroup->saveScriptToFile();

    if (trayIcon)
        trayIcon->hide();

    // saveRulesData();////

    iniSettings->setValue("UI/ConfirmOpenOrder", confirmOpenOrder);

    iniSettings->setValue("UI/CloseToTray", closeToTray);

    iniSettings->setValue("UI/WindowOnTop", ui.widgetStaysOnTop->isChecked());

    iniSettings->setValue("UI/DepthAutoResizeColumns", ui.depthAutoResize->isChecked());

    if (!ui.depthAutoResize->isChecked())
    {
        iniSettings->setValue("UI/DepthColumnAsksSizeWidth", ui.depthAsksTable->columnWidth(1));
        iniSettings->setValue("UI/DepthColumnAsksVolumeWidth", ui.depthAsksTable->columnWidth(2));
        iniSettings->setValue("UI/DepthColumnAsksPriceWidth", ui.depthAsksTable->columnWidth(3));

        iniSettings->setValue("UI/DepthColumnBidsPriceWidth", ui.depthBidsTable->columnWidth(0));
        iniSettings->setValue("UI/DepthColumnBidsVolumeWidth", ui.depthBidsTable->columnWidth(1));
        iniSettings->setValue("UI/DepthColumnBidsSizeWidth", ui.depthBidsTable->columnWidth(2));
    }

    iniSettings->setValue("UI/FeeCalcSingleInstance", feeCalculatorSingleInstance);

    iniSettings->setValue("UI/LockedDocks", lockedDocks);

    iniSettings->sync();
}

void QtBitcoinTrader::on_depthComboBoxLimitRows_currentIndexChanged(int val)
{
    baseValues.depthCountLimit = ui.depthComboBoxLimitRows->itemData(val, Qt::UserRole).toInt();
    baseValues.depthCountLimitStr = QByteArray::number(baseValues.depthCountLimit);
    iniSettings->setValue("UI/DepthCountLimit", baseValues.depthCountLimit);
    iniSettings->sync();
    clearDepth();
}

void QtBitcoinTrader::on_comboBoxGroupByPrice_currentIndexChanged(int val)
{
    baseValues.groupPriceValue = ui.comboBoxGroupByPrice->itemData(val, Qt::UserRole).toDouble();
    iniSettings->setValue("UI/DepthGroupByPrice", baseValues.groupPriceValue);
    iniSettings->sync();
    clearDepth();
}

void QtBitcoinTrader::fixWidthComboBoxGroupByPrice()
{
    if (ui.comboBoxGroupByPrice->count() == 0)
        return;

    int width = textFontWidth(ui.comboBoxGroupByPrice->itemText(0));

    if (ui.comboBoxGroupByPrice->count() > 1)
        width = qMax(textFontWidth(ui.comboBoxGroupByPrice->itemText(ui.comboBoxGroupByPrice->count() - 1)), width);

    if (ui.comboBoxGroupByPrice->count() > 2)
        width = qMax(textFontWidth(ui.comboBoxGroupByPrice->itemText(1)), width);

    int bonus = static_cast<int>(ui.comboBoxGroupByPrice->height() * 1.1);
    ui.comboBoxGroupByPrice->setMinimumWidth(width + bonus);
}

void QtBitcoinTrader::on_depthAutoResize_toggled(bool on)
{
    if (on)
    {
        ui.depthAsksTable->horizontalHeader()->showSection(0);
        ui.depthBidsTable->horizontalHeader()->showSection(4);

        setColumnResizeMode(ui.depthAsksTable, 0, QHeaderView::Stretch);
        setColumnResizeMode(ui.depthAsksTable, 1, QHeaderView::ResizeToContents);
        setColumnResizeMode(ui.depthAsksTable, 2, QHeaderView::ResizeToContents);
        setColumnResizeMode(ui.depthAsksTable, 3, QHeaderView::ResizeToContents);
        setColumnResizeMode(ui.depthAsksTable, 4, QHeaderView::ResizeToContents);

        setColumnResizeMode(ui.depthBidsTable, 0, QHeaderView::ResizeToContents);
        setColumnResizeMode(ui.depthBidsTable, 1, QHeaderView::ResizeToContents);
        setColumnResizeMode(ui.depthBidsTable, 2, QHeaderView::ResizeToContents);
        setColumnResizeMode(ui.depthBidsTable, 3, QHeaderView::ResizeToContents);
        setColumnResizeMode(ui.depthBidsTable, 4, QHeaderView::Stretch);

        QCoreApplication::processEvents();
        ui.depthAsksTable->horizontalScrollBar()->setValue(ui.depthAsksTable->horizontalScrollBar()->maximum());
        ui.depthBidsTable->horizontalScrollBar()->setValue(0);
    }
    else
    {
        setColumnResizeMode(ui.depthAsksTable, QHeaderView::Interactive);
        setColumnResizeMode(ui.depthBidsTable, QHeaderView::Interactive);
        ui.depthAsksTable->horizontalHeader()->hideSection(0);
        ui.depthBidsTable->horizontalHeader()->hideSection(4);
    }
}

double QtBitcoinTrader::getAvailableBTC()
{
    if (currentExchange->balanceDisplayAvailableAmount)
        return JulyMath::cutDoubleDecimalsCopy(ui.accountBTC->value(), baseValues.currentPair.currADecimals, false);

    return JulyMath::cutDoubleDecimalsCopy(
        ui.accountBTC->value() - ui.ordersTotalBTC->value(), baseValues.currentPair.currADecimals, false);
}

double QtBitcoinTrader::getAvailableUSD()
{
    if (floatFee == 0.0)
        return ui.accountUSD->value();

    double amountToReturn = 0.0;

    if (currentExchange->balanceDisplayAvailableAmount)
        amountToReturn = ui.accountUSD->value();
    else
        amountToReturn = ui.accountUSD->value() - ui.ordersTotalUSD->value();

    amountToReturn = JulyMath::cutDoubleDecimalsCopy(amountToReturn, baseValues.currentPair.currBDecimals, false);

    if (currentExchange->exchangeSupportsAvailableAmount)
        amountToReturn = qMin(availableAmount, amountToReturn);

    currentExchange->filterAvailableUSDAmountValue(&amountToReturn);

    return amountToReturn;
}

double QtBitcoinTrader::getAvailableUSDtoBTC(double priceToBuy)
{
    double avUSD = getAvailableUSD();
    double decValue = 0.0L;

    if (floatFee > 0.0)
    {
        switch (currentExchange->calculatingFeeMode)
        {
        case 1:
            decValue = qPow(0.1, qMax(baseValues.currentPair.currADecimals, 1));
            break;

        case 2:
            decValue = 2.0 * qPow(0.1, qMax(baseValues.currentPair.currADecimals, 1));
            break;

        case 3:
            {
                double zeros = qPow(10, baseValues.currentPair.currBDecimals);
                avUSD = ceil(avUSD / (floatFee + 1.0) * zeros) / zeros;
                break;
            }

        default:
            break;
        }
    }

    return JulyMath::cutDoubleDecimalsCopy(avUSD / priceToBuy - decValue, baseValues.currentPair.currADecimals, false);
}

void QtBitcoinTrader::apiSellSend(const QString& symbol, double btc, double price)
{
    if (baseValues.currentPair.symbolSecond().startsWith(symbol, Qt::CaseInsensitive))
    {
        emit apiSell(symbol, btc, price);
    }
}

void QtBitcoinTrader::apiBuySend(const QString& symbol, double btc, double price)
{
    if (baseValues.currentPair.symbolSecond().startsWith(symbol, Qt::CaseInsensitive))
    {
        emit apiBuy(symbol, btc, price);
    }
}

void QtBitcoinTrader::accFeeChanged(const QString& symbol, double val)
{
    if (baseValues.currentPair.symbolSecond().startsWith(symbol, Qt::CaseInsensitive))
        setSpinValue(ui.accountFee, val);
}

void QtBitcoinTrader::accBtcBalanceChanged(const QString& symbol, double val)
{
    if (baseValues.currentPair.symbolSecond().startsWith(symbol, Qt::CaseInsensitive))
        setSpinValue(ui.accountBTC, val);
}

void QtBitcoinTrader::accUsdBalanceChanged(const QString& symbol, double val)
{
    if (baseValues.currentPair.symbolSecond().startsWith(symbol, Qt::CaseInsensitive))
        setSpinValue(ui.accountUSD, val);
}

void QtBitcoinTrader::indicatorHighChanged(const QString& symbol, double val)
{
    if (baseValues.currentPair.symbolSecond().startsWith(symbol, Qt::CaseInsensitive))
        setSpinValue(ui.marketHigh, val);
}

void QtBitcoinTrader::indicatorLowChanged(const QString& symbol, double val)
{
    if (baseValues.currentPair.symbolSecond().startsWith(symbol, Qt::CaseInsensitive))
        setSpinValue(ui.marketLow, val);
}

void QtBitcoinTrader::indicatorBuyChanged(const QString& symbol, double val)
{
    if (secondTimer.isNull())
        return;

    if (baseValues.currentPair.symbolSecond().startsWith(symbol, Qt::CaseInsensitive))
    {
        if (val == 0.0)
            val = ui.marketLast->value();

        if (val == 0.0)
            val = ui.marketBid->value();

        if (ui.marketAsk->value() == 0.0 && val > 0.0)
            buyPricePerCoin->setValue(val);

        setSpinValue(ui.marketAsk, val);
    }
}

void QtBitcoinTrader::indicatorLastChanged(const QString& symbol, double val)
{
    if (baseValues.currentPair.symbolSecond().startsWith(symbol, Qt::CaseInsensitive))
        setSpinValue(ui.marketLast, val);
}

void QtBitcoinTrader::indicatorSellChanged(const QString& symbol, double val)
{
    if (secondTimer.isNull())
        return;

    if (baseValues.currentPair.symbolSecond().startsWith(symbol, Qt::CaseInsensitive))
    {
        if (val == 0.0)
            val = ui.marketLast->value();

        if (val == 0.0)
            val = ui.marketAsk->value();

        if (ui.marketBid->value() == 0.0 && val > 0.0)
            sellPricePerCoin->setValue(val);

        setSpinValue(ui.marketBid, val);
    }
}

void QtBitcoinTrader::indicatorVolumeChanged(const QString& symbol, double val)
{
    if (baseValues.currentPair.symbolSecond().startsWith(symbol, Qt::CaseInsensitive))
        setSpinValue(ui.marketVolume, val);
}

void QtBitcoinTrader::setRuleTabRunning(const QString& name, bool on)
{
    if (secondTimer.isNull())
        return;

    for (int n = 0; n < ui.rulesTabs->count(); n++)
        if (ui.rulesTabs->tabText(n) == name)
        {
            static QIcon playIcon(":/Resources/Play.png");
            ui.rulesTabs->setTabIcon(n, on ? playIcon : QIcon());
        }
}

void QtBitcoinTrader::setSpinValueP(QDoubleSpinBox* spin, double& val)
{
    if (spin == nullptr || secondTimer.isNull())
        return;

    spin->blockSignals(true);
    int needChangeDecimals = 0;

    if (val < 0.00000001)
        spin->setDecimals(1);
    else
    {
        QByteArray valueStr = JulyMath::byteArrayFromDouble(val);
        int dotPos = valueStr.indexOf('.');

        if (dotPos == -1)
            spin->setDecimals(1);
        else
        {
            int newDecimals = valueStr.size() - dotPos - 1;

            if (spin->decimals() > newDecimals)
                needChangeDecimals = newDecimals;
            else
                spin->setDecimals(newDecimals);
        }
    }

    spin->blockSignals(false);

    spin->setMaximum(val);
    spin->setValue(val);

    if (needChangeDecimals)
    {
        spin->blockSignals(true);
        spin->setDecimals(needChangeDecimals);
        spin->blockSignals(false);
    }
}

void QtBitcoinTrader::setSpinValue(QDoubleSpinBox* spin, double val)
{
    setSpinValueP(spin, val);
}

double QtBitcoinTrader::getVolumeByPrice(const QString& symbol, double price, bool isAsk)
{
    if (!baseValues.currentPair.symbolSecond().startsWith(symbol, Qt::CaseInsensitive))
        return 0.0;

    return (isAsk ? depthAsksModel : depthBidsModel)->getVolumeByPrice(price, isAsk);
}

double QtBitcoinTrader::getPriceByVolume(const QString& symbol, double size, bool isAsk)
{
    if (!baseValues.currentPair.symbolSecond().startsWith(symbol, Qt::CaseInsensitive))
        return 0.0;

    return (isAsk ? depthAsksModel : depthBidsModel)->getPriceByVolume(size);
}

void QtBitcoinTrader::on_helpButton_clicked()
{
    QString helpType = "JLRule";

    if (ui.rulesTabs->count())
    {
        if (qobject_cast<ScriptWidget*>(ui.rulesTabs->currentWidget()))
            helpType = "JLScript";
    }

    QDesktopServices::openUrl(QUrl("https://qbtapi.centrabit.com/?Object=Help&Method=" + helpType + "&Locale=" + QLocale().name()));
}

void QtBitcoinTrader::initDocks()
{
    setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::North);
    setDockOptions(QMainWindow::AnimatedDocks | QMainWindow::AllowNestedDocks | QMainWindow::AllowTabbedDocks);
}

void QtBitcoinTrader::createActions()
{
    actionLockDocks = new QAction("&Lock Docks", this);
    actionLockDocks->setCheckable(true);
    connect(actionLockDocks, &QAction::triggered, this, &QtBitcoinTrader::onActionLockDocks);

    actionExit = new QAction("E&xit", this);
    actionExit->setShortcut(QKeySequence::Quit);
    connect(actionExit, &QAction::triggered, this, &QtBitcoinTrader::exitApp);

    actionUpdate = new QAction("Check for &updates...", this);
    connect(actionUpdate, &QAction::triggered, this, &QtBitcoinTrader::checkUpdate);

    actionSendBugReport = new QAction("&Send bug report", this);
    connect(actionSendBugReport, &QAction::triggered, this, &QtBitcoinTrader::onActionSendBugReport);

    actionAbout = new QAction("&About", this);
    connect(actionAbout, &QAction::triggered, this, &QtBitcoinTrader::onActionAbout);

    actionAboutQt = new QAction("About &Qt", this);
    connect(actionAboutQt, &QAction::triggered, this, &QtBitcoinTrader::onActionAboutQt);

    actionConfigManager = new QAction("&Save...", this);
    connect(actionConfigManager, &QAction::triggered, this, &QtBitcoinTrader::onActionConfigManager);

    actionSettings = new QAction("Se&ttings", this);
    connect(actionSettings, &QAction::triggered, this, &QtBitcoinTrader::onActionSettings);

    actionDebug = new QAction("&Debug", this);
    connect(actionDebug, &QAction::triggered, this, &QtBitcoinTrader::onActionDebug);

#ifndef Q_OS_MAC

    if (!baseValues_->portableMode)
    {
        actionUninstall = new QAction("&Uninstall", this);
        connect(actionUninstall, &QAction::triggered, this, &QtBitcoinTrader::uninstall);
    }

#endif
}

void QtBitcoinTrader::createMenu()
{
    menuFile = menuBar()->addMenu("&QtBitcoinTrader");
    menuFile->addSeparator();

    menuFile->addSeparator();
    menuFile->addAction(actionSettings);
    menuFile->addAction(actionDebug);
    menuFile->addSeparator();
    menuFile->addAction(actionExit);
#ifdef Q_OS_MAC
    actionSettings->setMenuRole(QAction::ApplicationSpecificRole);
    actionDebug->setMenuRole(QAction::ApplicationSpecificRole);
#endif
    actionExit->setMenuRole(QAction::QuitRole);

    menuView = menuBar()->addMenu("&View");
    menuView->addAction(actionLockDocks);
    menuView->addSeparator();

    menuConfig = menuBar()->addMenu("&Config");

    menuHelp = menuBar()->addMenu("&Help");
    menuHelp->addAction(actionUpdate);
    menuHelp->addAction(actionSendBugReport);
    menuHelp->addSeparator();
    menuHelp->addAction(actionAbout);
    menuHelp->addAction(actionAboutQt);

#ifndef Q_OS_MAC

    if (actionUninstall && !baseValues_->portableMode)
    {
        menuHelp->addSeparator();
        menuHelp->addAction(actionUninstall);
    }

#endif

    ui.menubar->setStyleSheet("font-size:12px");
}

void QtBitcoinTrader::uninstall()
{
    QMessageBox msgBox(windowWidget);
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setWindowTitle("Qt Bitcoin Trader");
    msgBox.setText(julyTr("CONFIRM_UNINSTALL", "Are you sure to uninstall Application?<br>All configs, scripts, rules will be deleted"));

    auto buttonYes = msgBox.addButton(julyTr("YES", "Yes"), QMessageBox::YesRole);
    msgBox.addButton(julyTr("NO", "No"), QMessageBox::NoRole);
    msgBox.exec();
    if (msgBox.clickedButton() != buttonYes)
        return;

    QString tmpDir = QStandardPaths::standardLocations(QStandardPaths::TempLocation).first();

    QString fileName = QFileInfo(QCoreApplication::applicationFilePath()).fileName();
    QFile::Permissions selfPerms = QFile(QCoreApplication::applicationFilePath()).permissions();

    if (QFile::exists(tmpDir + "/TMP_" + fileName))
    {
        QFile::remove(tmpDir + "/TMP_" + fileName);

        if (QFile::exists(tmpDir + "/TMP_" + fileName))
            QFile::rename(tmpDir + "/TMP_" + fileName,
                          tmpDir + "/TMP_" + QString::number(QDateTime::currentSecsSinceEpoch()) + "_" + fileName);
    }

    QFile::copy(QCoreApplication::applicationFilePath(), tmpDir + "/TMP_" + fileName);
    QFile(tmpDir + "/TMP_" + fileName).setPermissions(selfPerms);
    QProcess proc;

    if (!proc.startDetached(tmpDir + "/TMP_" + fileName, QStringList() << "/uninstall"))
    {
        QMessageBox::warning(this,
                             "Qt Bitcoin Trader",
                             julyTr("UNINSTALL_ERROR", "Can't uninstall app, you can delete files manually") + "\n" + appDataDir + "\n" +
                                 tmpDir + "/TMP_" + fileName + "\n" + proc.errorString());
        QFile::remove(tmpDir + "/TMP_" + fileName);
        return;
    }

    QCoreApplication::quit();
}

QDockWidget* QtBitcoinTrader::createDock(QWidget* widget, const QString& title)
{
    widget->setProperty("IsDockable", true);
    QDockWidget* dock = dockHost->createDock(this, widget, title);

    if (widget != ui.widgetLogo)
    {
        menuView->addAction(dock->toggleViewAction());
    }

    return dock;
}

void QtBitcoinTrader::moveWidgetsToDocks()
{
    // Top
    Qt::DockWidgetArea area = Qt::TopDockWidgetArea;
    addDockWidget(area, createDock(ui.widgetAccount, "Exchange Account"));
    addDockWidget(area, createDock(ui.widgetBalance, "Balance"));
    addDockWidget(area, createDock(ui.widgetTotalAtLast, "Total at Last Price"));
    addDockWidget(area, createDock(ui.widgetTotalAtBuySell, "Total at Buy/Sell Price"));

    QWidget* titleNULL = new QWidget();
    QDockWidget* dockNULL = new QDockWidget();
    dockNULL->setObjectName("dockNULL");
    dockNULL->setTitleBarWidget(titleNULL);
    dockNULL->setMinimumSize(0, 1);
    addDockWidget(area, dockNULL);

    addDockWidget(area, createDock(ui.widgetMarket, "Market"));
    addDockWidget(area, createDock(ui.widgetNetwork, "Network"));

    // Bottom
    QDockWidget* dockBuy = createDock(ui.widgetBuy, "Buy Bitcoin");
    QDockWidget* dockBuySell = createDock(ui.widgetBuyThenSell, "Generate subsequent sell order");
    QDockWidget* dockSell = createDock(ui.widgetSell, "Sell Bitcoin");
    QDockWidget* dockSellBuy = createDock(ui.widgetSellThenBuy, "Generate subsequent buy order");
    QDockWidget* dockGeneral = createDock(ui.widgetSellBuy, "General");
    dockLogo = createDock(ui.widgetLogo, "Qt Trader Exchange"); // Powered By
    dockLogo->setMinimumSize(170, 70);
    ui.widgetBuyThenSell->setFixedHeight(ui.widgetBuyThenSell->minimumSizeHint().height());
    ui.widgetSellThenBuy->setFixedHeight(ui.widgetSellThenBuy->minimumSizeHint().height());

    QWidget* titleBottomNULL = new QWidget();
    QDockWidget* dockHSpacer = new QDockWidget();
    dockHSpacer->setObjectName("dockHSpacer");
    dockHSpacer->setMinimumSize(0, 1);
    dockHSpacer->setTitleBarWidget(titleBottomNULL);

    addDockWidget(Qt::BottomDockWidgetArea, dockBuy);
    splitDockWidget(dockBuy, dockSell, Qt::Horizontal);
    splitDockWidget(dockSell, dockHSpacer, Qt::Horizontal);
    splitDockWidget(dockHSpacer, dockGeneral, Qt::Horizontal);
    splitDockWidget(dockBuy, dockBuySell, Qt::Vertical);
    splitDockWidget(dockSell, dockSellBuy, Qt::Vertical);
    splitDockWidget(dockGeneral, dockLogo, Qt::Vertical);

    // Left
    QDockWidget* dockGroupOrders = createDock(ui.groupOrders, "Your Open Orders");
    addDockWidget(Qt::LeftDockWidgetArea, dockGroupOrders);

    // lock Logo
    lockLogo(false);

    // tabs
    QDockWidget* dockOrdersLog = createDock(ui.tabOrdersLog, "Orders Log");
    QDockWidget* dockRules = createDock(ui.tabRules, "Rules");
    dockDepth = createDock(ui.tabDepth, "Order Book");
    QDockWidget* dockLastTrades = createDock(ui.tabLastTrades, "Trades");
    QDockWidget* dockCharts = createDock(ui.tabCharts, "Charts");
    QDockWidget* dockNews = createDock(ui.tabNews, "News");
    // QDockWidget* dockChat = createDock(ui.tabChat, "Chat");
    splitDockWidget(dockGroupOrders, dockOrdersLog, Qt::Horizontal);
    tabifyDockWidget(dockOrdersLog, dockRules);
    tabifyDockWidget(dockOrdersLog, dockDepth);
    tabifyDockWidget(dockOrdersLog, dockLastTrades);
    tabifyDockWidget(dockOrdersLog, dockCharts);
    tabifyDockWidget(dockOrdersLog, dockNews);
    // tabifyDockWidget(dockOrdersLog, dockChat);
    delete ui.tabWidget;

    // Central
    QDockWidget* centralDockNULL = new QDockWidget(this);
    centralDockNULL->setFixedWidth(0);
    setCentralWidget(centralDockNULL);
    // centralWidget()->deleteLater();

    connect(dockDepth, &QDockWidget::visibilityChanged, this, &QtBitcoinTrader::depthVisibilityChanged);
    connect(dockCharts, &QDockWidget::visibilityChanged, chartsView, &ChartsView::visibilityChanged);
    connect(dockNews, &QDockWidget::visibilityChanged, newsView, &NewsView::visibilityChanged);

    lockedDocks = iniSettings->value("UI/LockedDocks", false).toBool();

    if (lockedDocks)
        actionLockDocks->setChecked(true);

    onActionLockDocks(lockedDocks);
}

void QtBitcoinTrader::onActionAbout()
{
    (new TranslationAbout(windowWidget))->showWindow();
}

void QtBitcoinTrader::onActionSendBugReport()
{
    QDesktopServices::openUrl(QUrl("https://github.com/JulyIGHOR/QtBitcoinTrader/issues"));
}

void QtBitcoinTrader::onActionAboutQt()
{
    QApplication::aboutQt();
}

void QtBitcoinTrader::onActionLockDocks(bool checked)
{
    lockedDocks = checked;
    dockHost->lockDocks(checked);
    lockLogo(checked);
}

void QtBitcoinTrader::onActionConfigManager()
{
    if (!configDialog)
    {
        configDialog = new ConfigManagerDialog(this);
    }

    julyTranslator.translateUi(configDialog);
    configDialog->setWindowTitle(julyTr("CONFIG_MANAGER_TITLE", "Config Manager"));
    configDialog->setWindowFlags(configDialog->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    configDialog->show();
}

void QtBitcoinTrader::onActionSettings()
{
    SettingsDialog settingsDialog;

    if (currentPopupDialogs != 0)
        settingsDialog.disableTranslateButton();

    settingsDialog.exec();
}

void QtBitcoinTrader::onActionDebug()
{
    if (debugLevel == 0)
        debugViewer = new DebugViewer;
    else
    {
        if (debugViewer == nullptr)
            debugViewer = new DebugViewer;

        debugViewer->setWindowState(Qt::WindowActive);
        debugViewer->activateWindow();
    }
}

void QtBitcoinTrader::onMenuConfigTriggered()
{
    QAction* action = static_cast<QAction*>(sender());
    QString name = action->text();

    ::config->load(name);
}

void QtBitcoinTrader::onConfigChanged()
{
    initConfigMenu();
}

void QtBitcoinTrader::onConfigError(const QString&)
{
}

void QtBitcoinTrader::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::WindowStateChange)
    {
        QWindowStateChangeEvent* stateChangeEvent = static_cast<QWindowStateChangeEvent*>(event);

        if (stateChangeEvent)
        {
            if (stateChangeEvent->oldState() == Qt::WindowMaximized && windowState() == Qt::WindowNoState)
            {
                adjustWidgetGeometry(this);
            }
        }
    }
}

bool QtBitcoinTrader::executeConfirmExitDialog()
{
    QMessageBox msgBox(windowWidget);
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setWindowTitle("Qt Bitcoin Trader");
    msgBox.setText(julyTr("CONFIRM_EXIT", "Are you sure to close Application?<br>Active rules works only while application is running."));

    auto buttonYes = msgBox.addButton(julyTr("YES", "Yes"), QMessageBox::YesRole);
    msgBox.addButton(julyTr("NO", "No"), QMessageBox::NoRole);
    msgBox.exec();
    if (msgBox.clickedButton() != buttonYes)
        return false;
    return true;
}

void QtBitcoinTrader::closeEvent(QCloseEvent* event)
{
    event->ignore();

    if (closeToTray)
    {
        buttonMinimizeToTray();
    }
    else if (confirmExitApp())
    {
        exitApp();
    }
}

bool QtBitcoinTrader::confirmExitApp()
{
    if (hasWorkingRules())
    {
        return executeConfirmExitDialog();
    }
    else
    {
        return true;
    }
}

void QtBitcoinTrader::exitApp()
{
    secondTimer.reset();

    saveAppState();
    ::config->save("", false);
    dockHost->hideFloatingWindow();
    hide();

    if (configDialog)
        configDialog->deleteLater();

    QCoreApplication::quit();
}

void QtBitcoinTrader::setupWidgets()
{
    buyTotalSpend->setAccessibleName("USD");
    buyTotalSpend->setAccessibleDescription("CAN_BE_ZERO");
    buyTotalSpend->setDecimals(5);
    buyTotalSpend->setMaximum(999999999.0);
    ui.totalToSpendLayout->addWidget(buyTotalSpend);

    buyPricePerCoin->setAccessibleName("PRICE");
    buyPricePerCoin->setDecimals(5);
    buyPricePerCoin->setMinimum(0.01);
    buyPricePerCoin->setMaximum(999999999.0);
    buyPricePerCoin->setValue(100.0);
    ui.pricePerCoinLayout->addWidget(buyPricePerCoin);

    buyTotalBtc->setAccessibleName("BTC");
    buyTotalBtc->setAccessibleDescription("CAN_BE_ZERO");
    buyTotalBtc->setDecimals(8);
    buyTotalBtc->setMaximum(999999999.0);
    ui.totalBtcToBuyLayout->addWidget(buyTotalBtc);

    profitLossSpinBox->setAccessibleName("USD");
    profitLossSpinBox->setAccessibleDescription("CAN_BE_ZERO");
    profitLossSpinBox->setDecimals(5);
    profitLossSpinBox->setMinimum(-999999999.0);
    profitLossSpinBox->setMaximum(999999999.0);
    ui.profitLossSpinBoxLayout->addWidget(profitLossSpinBox);

    profitLossSpinBoxPrec->setDecimals(3);
    profitLossSpinBoxPrec->setMinimum(-1000.0);
    profitLossSpinBoxPrec->setMaximum(1000.0);
    ui.profitLossSpinBoxPrecLayout->addWidget(profitLossSpinBoxPrec);

    sellTotalBtc->setAccessibleName("BTC");
    sellTotalBtc->setAccessibleDescription("CAN_BE_ZERO");
    sellTotalBtc->setDecimals(8);
    sellTotalBtc->setMaximum(999999999.0);
    ui.totalToSellLayout->addWidget(sellTotalBtc);

    sellPricePerCoin->setAccessibleName("PRICE");
    sellPricePerCoin->setDecimals(5);
    sellPricePerCoin->setMinimum(0.01);
    sellPricePerCoin->setMaximum(999999999.0);
    sellPricePerCoin->setValue(200.0);
    ui.pricePerCoinSellLayout->addWidget(sellPricePerCoin);

    sellAmountToReceive->setAccessibleName("USD");
    sellAmountToReceive->setAccessibleDescription("CAN_BE_ZERO");
    sellAmountToReceive->setDecimals(5);
    sellAmountToReceive->setMaximum(999999999.0);
    ui.amountToReceiveLayout->addWidget(sellAmountToReceive);

    sellThanBuySpinBox->setAccessibleName("BTC");
    sellThanBuySpinBox->setAccessibleDescription("CAN_BE_ZERO");
    sellThanBuySpinBox->setDecimals(8);
    sellThanBuySpinBox->setMinimum(-999999999.0);
    sellThanBuySpinBox->setMaximum(999999999.0);
    ui.sellThanBuySpinBoxLayout->addWidget(sellThanBuySpinBox);

    sellThanBuySpinBoxPrec->setDecimals(3);
    sellThanBuySpinBoxPrec->setMinimum(-9999.0);
    sellThanBuySpinBoxPrec->setMaximum(9999.0);
    ui.sellThanBuySpinBoxPrecLayout->addWidget(sellThanBuySpinBoxPrec);

    connect(buyTotalSpend, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &QtBitcoinTrader::buyTotalSpend_valueChanged);
    connect(buyPricePerCoin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &QtBitcoinTrader::buyPricePerCoin_valueChanged);
    connect(buyTotalBtc, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &QtBitcoinTrader::buyTotalBtc_valueChanged);
    connect(
        profitLossSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &QtBitcoinTrader::profitLossSpinBox_valueChanged);
    connect(profitLossSpinBoxPrec,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this,
            &QtBitcoinTrader::profitLossSpinBoxPrec_valueChanged);
    connect(sellTotalBtc, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &QtBitcoinTrader::sellTotalBtc_valueChanged);
    connect(sellPricePerCoin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &QtBitcoinTrader::sellPricePerCoin_valueChanged);
    connect(sellAmountToReceive,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this,
            &QtBitcoinTrader::sellAmountToReceive_valueChanged);
    connect(
        sellThanBuySpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &QtBitcoinTrader::sellThanBuySpinBox_valueChanged);
    connect(sellThanBuySpinBoxPrec,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this,
            &QtBitcoinTrader::sellThanBuySpinBoxPrec_valueChanged);
}
