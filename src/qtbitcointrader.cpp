// Copyright (C) 2014 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form http://qtopentrader.com
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#include "julyspinboxpicker.h"
#include <QPlastiqueStyle>
#include <QTableWidget>
#include <QTimeLine>
#include <QScrollBar>
#include <QDir>
#include <QMessageBox>
#include <QSettings>
#include "main.h"
#include "julylightchanges.h"
#include "julyspinboxfix.h"
#include "julyscrolluponidle.h"
#include <QFileInfo>
#include <QClipboard>
#include <QProcess>
#include <QFile>
#include <QSysInfo>
#include <QProcess>
#include <QDesktopWidget>
#include <QDesktopServices>
#include <QUrl>
#include "exchange.h"
#include "aboutdialog.h"
#include "audioplayer.h"
#include "exchange_mtgox.h"
#include "exchange_btce.h"
#include "exchange_bitstamp.h"
#include "exchange_btcchina.h"
#include "exchange_bitfinex.h"
#include <QSystemTrayIcon>
#include <QtCore/qmath.h>
#include "debugviewer.h"
#include "addrulegroup.h"
#include "donatepanel.h"
#include "percentpicker.h"
#include <QSound>
#include "thisfeatureunderdevelopment.h"
#include "orderstablecancelbutton.h"
#ifdef Q_OS_WIN
#include "windows.h"
#include <QWindowsXPStyle>
#endif


QtBitcoinTrader::QtBitcoinTrader()
	: QDialog()
{
	windowWidget=this;
	lastRuleExecutedTime=QTime(1,0,0,0);
	feeCalculator=0;
	currencyChangedDate=0;
	meridianPrice=0.0;
	availableAmount=0.0;
	swapedDepth=false;
	waitingDepthLag=false;
	depthLagTime.restart();
	softLagTime.restart();
	isDataPending=false;
	depthAsksLastScrollValue=0;
	depthBidsLastScrollValue=0;
	tradesPrecentLast=0.0;

	trayMenu=0;
	isValidSoftLag=true;

	trayIcon=0;
	isDetachedLog=false;
	isDetachedTrades=false;
	isDetachedRules=false;
	isDetachedDepth=false;
	isDetachedCharts=false;

	lastLoadedCurrency=-1;
	profitSellThanBuyUnlocked=true;
	profitBuyThanSellUnlocked=true;
	profitBuyThanSellChangedUnlocked=true;
	profitSellThanBuyChangedUnlocked=true;

	constructorFinished=false;
	appDir=QApplication::applicationDirPath()+"/";

	showingMessage=false;
	floatFee=0.0;
	floatFeeDec=0.0;
	floatFeeInc=0.0;
	marketPricesNotLoaded=true;
	balanceNotLoaded=true;
	sellLockBtcToSell=false;
	sellLockPricePerCoin=false;
	sellLockAmountToReceive=false;

	buyLockTotalBtcSelf=false;
	buyLockTotalBtc=false;
	buyLockPricePerCoin=false;
	buyLockTotalSpend=false;

	ui.setupUi(this);
	ui.accountFee->setValue(0.0);
	ui.accountLoginLabel->setStyleSheet("background: "+baseValues.appTheme.white.name());
	ui.noOpenedOrdersLabel->setStyleSheet("border: 1px solid gray; background: "+baseValues.appTheme.white.name()+"; color: "+baseValues.appTheme.gray.name());
	ui.rulesNoMessage->setStyleSheet("border: 1px solid gray; background: "+baseValues.appTheme.white.name()+"; color: "+baseValues.appTheme.gray.name());

	iniSettings=new QSettings(baseValues.iniFileName,QSettings::IniFormat,this);

	baseValues.groupPriceValue=iniSettings->value("UI/DepthGroupByPrice",0.0).toDouble();
	if(baseValues.groupPriceValue<0.0)baseValues.groupPriceValue=0.0;
	iniSettings->setValue("UI/DepthGroupByPrice",baseValues.groupPriceValue);

	windowWidget->setAttribute(Qt::WA_QuitOnClose,true);

	windowWidget->setWindowFlags(Qt::Window);

	ui.ordersTableFrame->setVisible(false);

	currentlyAddingOrders=false;
	ordersModel=new OrdersModel;
	ordersSortModel=new QSortFilterProxyModel;
	ordersSortModel->setSortRole(Qt::EditRole);
	ordersSortModel->setFilterRole(Qt::WhatsThisRole);
	ordersSortModel->setDynamicSortFilter(true);
	ordersSortModel->setSourceModel(ordersModel);
	ui.ordersTable->setModel(ordersSortModel);
	ui.ordersTable->horizontalHeader()->setResizeMode(0,QHeaderView::ResizeToContents);
	ui.ordersTable->horizontalHeader()->setResizeMode(1,QHeaderView::Stretch);
	ui.ordersTable->horizontalHeader()->setResizeMode(2,QHeaderView::ResizeToContents);
	ui.ordersTable->horizontalHeader()->setResizeMode(3,QHeaderView::ResizeToContents);
	ui.ordersTable->horizontalHeader()->setResizeMode(4,QHeaderView::ResizeToContents);
	ui.ordersTable->horizontalHeader()->setResizeMode(5,QHeaderView::ResizeToContents);
	ui.ordersTable->horizontalHeader()->setResizeMode(6,QHeaderView::ResizeToContents);
	ui.ordersTable->horizontalHeader()->setResizeMode(7,QHeaderView::ResizeToContents);
	ui.ordersTable->setItemDelegateForColumn(7,new OrdersTableCancelButton(ui.ordersTable));

	ui.ordersTable->setSortingEnabled(true);
	ui.ordersTable->sortByColumn(0,Qt::AscendingOrder);

	connect(ordersModel,SIGNAL(ordersIsAvailable()),this,SLOT(ordersIsAvailable()));
	connect(ordersModel,SIGNAL(cancelOrder(QByteArray)),this,SLOT(cancelOrder(QByteArray)));
	connect(ordersModel,SIGNAL(volumeAmountChanged(double, double)),this,SLOT(volumeAmountChanged(double, double)));
	connect(ui.ordersTable->selectionModel(),SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),this,SLOT(checkValidOrdersButtons()));

	tradesModel=new TradesModel;
	ui.tableTrades->setModel(tradesModel);
	ui.tableTrades->horizontalHeader()->setResizeMode(0,QHeaderView::Stretch);
	ui.tableTrades->horizontalHeader()->setResizeMode(1,QHeaderView::ResizeToContents);
	ui.tableTrades->horizontalHeader()->setResizeMode(2,QHeaderView::ResizeToContents);
	ui.tableTrades->horizontalHeader()->setResizeMode(3,QHeaderView::ResizeToContents);
	ui.tableTrades->horizontalHeader()->setResizeMode(4,QHeaderView::ResizeToContents);
	ui.tableTrades->horizontalHeader()->setResizeMode(5,QHeaderView::ResizeToContents);
	ui.tableTrades->horizontalHeader()->setResizeMode(6,QHeaderView::ResizeToContents);
	ui.tableTrades->horizontalHeader()->setResizeMode(7,QHeaderView::Stretch);
	connect(tradesModel,SIGNAL(trades10MinVolumeChanged(double)),this,SLOT(setLastTrades10MinVolume(double)));
	connect(tradesModel,SIGNAL(precentBidsChanged(double)),this,SLOT(precentBidsChanged(double)));

	historyModel=new HistoryModel;
	ui.tableHistory->setModel(historyModel);
	ui.tableHistory->horizontalHeader()->setResizeMode(0,QHeaderView::Stretch);
	ui.tableHistory->horizontalHeader()->setResizeMode(1,QHeaderView::ResizeToContents);
	ui.tableHistory->horizontalHeader()->setResizeMode(2,QHeaderView::ResizeToContents);
	ui.tableHistory->horizontalHeader()->setResizeMode(3,QHeaderView::ResizeToContents);
	ui.tableHistory->horizontalHeader()->setResizeMode(4,QHeaderView::ResizeToContents);
	ui.tableHistory->horizontalHeader()->setResizeMode(5,QHeaderView::ResizeToContents);
	ui.tableHistory->horizontalHeader()->setResizeMode(6,QHeaderView::Stretch);
	connect(historyModel,SIGNAL(accLastSellChanged(QByteArray,double)),this,SLOT(accLastSellChanged(QByteArray,double)));
	connect(historyModel,SIGNAL(accLastBuyChanged(QByteArray,double)),this,SLOT(accLastBuyChanged(QByteArray,double)));


	depthAsksModel=new DepthModel(true);
	ui.depthAsksTable->setModel(depthAsksModel);
	depthBidsModel=new DepthModel(false);
	ui.depthBidsTable->setModel(depthBidsModel);

	new JulyScrollUpOnIdle(ui.depthAsksTable->verticalScrollBar());
	new JulyScrollUpOnIdle(ui.depthBidsTable->verticalScrollBar());

	ui.depthAsksTable->horizontalHeader()->setResizeMode(0,QHeaderView::Stretch);
	ui.depthAsksTable->horizontalHeader()->setResizeMode(1,QHeaderView::ResizeToContents);
	ui.depthAsksTable->horizontalHeader()->setResizeMode(2,QHeaderView::ResizeToContents);
	ui.depthAsksTable->horizontalHeader()->setResizeMode(3,QHeaderView::ResizeToContents);
	ui.depthAsksTable->horizontalHeader()->setResizeMode(4,QHeaderView::ResizeToContents);
	ui.depthAsksTable->horizontalHeader()->setMinimumSectionSize(0);

	ui.depthBidsTable->horizontalHeader()->setResizeMode(0,QHeaderView::ResizeToContents);
	ui.depthBidsTable->horizontalHeader()->setResizeMode(1,QHeaderView::ResizeToContents);
	ui.depthBidsTable->horizontalHeader()->setResizeMode(2,QHeaderView::ResizeToContents);
	ui.depthBidsTable->horizontalHeader()->setResizeMode(3,QHeaderView::ResizeToContents);
	ui.depthBidsTable->horizontalHeader()->setResizeMode(4,QHeaderView::Stretch);
	ui.depthBidsTable->horizontalHeader()->setMinimumSectionSize(0);

	ui.minimizeOnCloseCheckBox->setChecked(iniSettings->value("UI/CloseToTray",false).toBool());

	ui.confirmOpenOrder->setChecked(iniSettings->value("UI/ConfirmOpenOrder",true).toBool());
	iniSettings->setValue("UI/ConfirmOpenOrder",ui.confirmOpenOrder->isChecked());

	checkValidOrdersButtons();

	new JulyLightChanges(ui.accountFee);
	new JulyLightChanges(ui.marketVolume);
	new JulyLightChanges(ui.marketBid);
	new JulyLightChanges(ui.marketAsk);
	new JulyLightChanges(ui.marketBid);
	new JulyLightChanges(ui.marketHigh);
	new JulyLightChanges(ui.marketLow);
	new JulyLightChanges(ui.accountBTC);
	new JulyLightChanges(ui.accountUSD);
	new JulyLightChanges(ui.marketLast);
	new JulyLightChanges(ui.ordersLastSellPrice);
	new JulyLightChanges(ui.ordersLastBuyPrice);
	new JulyLightChanges(ui.tradesVolume5m);

	new JulyLightChanges(ui.ruleTotalToBuyValue);
	new JulyLightChanges(ui.ruleAmountToReceiveValue);
	new JulyLightChanges(ui.ruleTotalToBuyBSValue);
	new JulyLightChanges(ui.ruleAmountToReceiveBSValue);
	new JulyLightChanges(ui.tradesBidsPrecent);
	
	baseValues.forceDotInSpinBoxes=iniSettings->value("UI/ForceDotInDouble",true).toBool();
	iniSettings->setValue("UI/ForceDotInDouble",baseValues.forceDotInSpinBoxes);

	ui.totalToSpendLayout->addWidget(new JulySpinBoxPicker(ui.buyTotalSpend));
	ui.pricePerCoinLayout->addWidget(new JulySpinBoxPicker(ui.buyPricePerCoin));
	ui.totalBtcToBuyLayout->addWidget(new JulySpinBoxPicker(ui.buyTotalBtc));

	ui.totalToSellLayout->addWidget(new JulySpinBoxPicker(ui.sellTotalBtc));
	ui.pricePerCoinSellLayout->addWidget(new JulySpinBoxPicker(ui.sellPricePerCoin));
	ui.amountToReceiveLayout->addWidget(new JulySpinBoxPicker(ui.sellAmountToReceive));

	ui.gssoProfitLayout->addWidget(new JulySpinBoxPicker(ui.profitLossSpinBox));
	ui.gssoProfitPercLayout->addWidget(new JulySpinBoxPicker(ui.profitLossSpinBoxPrec));

	ui.gssboProfitLayout->addWidget(new JulySpinBoxPicker(ui.sellThanBuySpinBox));
	ui.gsboProfitPercLayout->addWidget(new JulySpinBoxPicker(ui.sellThanBuySpinBoxPrec));

	foreach(QDoubleSpinBox* spinBox, findChildren<QDoubleSpinBox*>())new JulySpinBoxFix(spinBox);

	double iniFileVersion=iniSettings->value("Profile/Version",1.0).toDouble();
	if(iniFileVersion<baseValues.appVerReal)iniSettings->setValue("Profile/Version",baseValues.appVerReal);

	QSettings settingsMain(appDataDir+"/QtBitcoinTrader.cfg",QSettings::IniFormat);
	checkForUpdates=settingsMain.value("CheckForUpdates",true).toBool();

	int defTextHeight=baseValues.fontMetrics_->boundingRect("0123456789").height();
	defaultHeightForRow=settingsMain.value("RowHeight",defTextHeight*1.6).toInt();
	if(defaultHeightForRow<defTextHeight)defaultHeightForRow=defTextHeight;
	settingsMain.setValue("RowHeight",defaultHeightForRow);

	baseValues.depthCountLimit=iniSettings->value("UI/DepthCountLimit",100).toInt();
	if(baseValues.depthCountLimit<0)baseValues.depthCountLimit=100;
	baseValues.depthCountLimitStr=QByteArray::number(baseValues.depthCountLimit);
	int currentDepthComboBoxLimitIndex=0;
	for(int n=0;n<ui.depthComboBoxLimitRows->count();n++)
	{
		int currentValueDouble=ui.depthComboBoxLimitRows->itemText(n).toInt();
		if(currentValueDouble==baseValues.depthCountLimit)currentDepthComboBoxLimitIndex=n;
		ui.depthComboBoxLimitRows->setItemData(n,currentValueDouble,Qt::UserRole);
	}
	ui.depthComboBoxLimitRows->setCurrentIndex(currentDepthComboBoxLimitIndex);

	exchangeId=iniSettings->value("Profile/ExchangeId",0).toInt();

	baseValues.apiDownCount=iniSettings->value("Network/ApiDownCounterMax",5).toInt();
	if(baseValues.apiDownCount<0)baseValues.apiDownCount=5;
	iniSettings->setValue("Network/ApiDownCounterMax",baseValues.apiDownCount);

	baseValues.httpRequestInterval=iniSettings->value("Network/HttpRequestsInterval",500).toInt();
	baseValues.httpRequestTimeout=iniSettings->value("Network/HttpRequestsTimeout",4000).toInt();
	baseValues.httpRetryCount=iniSettings->value("Network/HttpRetryCount",8).toInt();
	if(baseValues.httpRetryCount<1||baseValues.httpRetryCount>50)baseValues.httpRetryCount=8;
	if(iniFileVersion<1.07969)
	{
		baseValues.httpRetryCount=8;
		baseValues.httpRequestTimeout=4000;
		iniSettings->remove("Sounds/MarketHighBeep");
		iniSettings->remove("Sounds/MarketLowBeep");
		iniSettings->remove("Sounds/AccountBTCBeep");
		iniSettings->remove("Sounds/AccountUSDBeep");
	}
	iniSettings->setValue("Network/HttpRetryCount",baseValues.httpRetryCount);

	baseValues.uiUpdateInterval=iniSettings->value("UI/UiUpdateInterval",100).toInt();
	if(baseValues.uiUpdateInterval<1)baseValues.uiUpdateInterval=100;

	baseValues.httpSplitPackets=iniSettings->value("Network/HttpSplitPackets",false).toBool();

	baseValues.customUserAgent=iniSettings->value("Network/UserAgent","").toString();
	baseValues.customCookies=iniSettings->value("Network/Cookies","").toString();

	baseValues.gzipEnabled=iniSettings->value("Network/GZIPEnabled",true).toBool();
	iniSettings->setValue("Network/GZIPEnabled",baseValues.gzipEnabled);

	feeCalculatorSingleInstance=iniSettings->value("UI/FeeCalcSingleInstance",true).toBool();
	iniSettings->setValue("UI/FeeCalcSingleInstance",feeCalculatorSingleInstance);

	ui.depthAutoResize->setChecked(iniSettings->value("UI/DepthAutoResizeColumns",true).toBool());

	if(!ui.depthAutoResize->isChecked())
	{
		ui.depthAsksTable->setColumnWidth(1,iniSettings->value("UI/DepthColumnAsksSizeWidth",ui.depthAsksTable->columnWidth(1)).toInt());
		ui.depthAsksTable->setColumnWidth(2,iniSettings->value("UI/DepthColumnAsksVolumeWidth",ui.depthAsksTable->columnWidth(2)).toInt());
		ui.depthAsksTable->setColumnWidth(3,iniSettings->value("UI/DepthColumnAsksPriceWidth",ui.depthAsksTable->columnWidth(3)).toInt());

		ui.depthBidsTable->setColumnWidth(0,iniSettings->value("UI/DepthColumnBidsPriceWidth",ui.depthBidsTable->columnWidth(0)).toInt());
		ui.depthBidsTable->setColumnWidth(1,iniSettings->value("UI/DepthColumnBidsVolumeWidth",ui.depthBidsTable->columnWidth(1)).toInt());
		ui.depthBidsTable->setColumnWidth(2,iniSettings->value("UI/DepthColumnBidsSizeWidth",ui.depthBidsTable->columnWidth(2)).toInt());
	}

	int currentDepthComboBoxIndex=0;
	for(int n=0;n<ui.comboBoxGroupByPrice->count();n++)
	{
		double currentValueDouble=ui.comboBoxGroupByPrice->itemText(n).toDouble();
		if(currentValueDouble==baseValues.groupPriceValue)currentDepthComboBoxIndex=n;
		ui.comboBoxGroupByPrice->setItemData(n,currentValueDouble,Qt::UserRole);
	}
	ui.comboBoxGroupByPrice->setCurrentIndex(currentDepthComboBoxIndex);

	ui.rulesTabs->setVisible(false);

	if(baseValues.appVerLastReal<1.0763)
	{
		baseValues.httpRequestInterval=500;
		baseValues.httpSplitPackets=false;
		settingsMain.remove("HttpConnectionsCount");
		settingsMain.remove("HttpSwapSocketAfterPacketsCount");
		settingsMain.remove("HttpRequestsInterval");
		settingsMain.remove("HttpRequestsTimeout");
		settingsMain.remove("DepthCountLimit");
		settingsMain.remove("UiUpdateInterval");
		settingsMain.remove("LogEnabled");
		settingsMain.remove("RowHeight");
		settingsMain.remove("ApiDownCount");
		settingsMain.remove("Network/HttpSplitPackets");
		QStringList oldKeys=iniSettings->childKeys();
		for(int n=0;n<oldKeys.count();n++)iniSettings->remove(oldKeys.at(n));

		iniSettings->sync();
	}

	if(baseValues.appVerLastReal<1.07967)
	{
		baseValues.httpRequestTimeout=4000;
	}
	if(baseValues.httpRequestInterval<50)baseValues.httpRequestInterval=500;
	if(baseValues.httpRequestTimeout<100)baseValues.httpRequestTimeout=4000;

	iniSettings->setValue("Network/HttpRequestsInterval",baseValues.httpRequestInterval);
	iniSettings->setValue("Network/HttpRequestsTimeout",baseValues.httpRequestTimeout);
	iniSettings->setValue("Network/HttpSplitPackets",baseValues.httpSplitPackets);
	iniSettings->setValue("Network/HttpRetryCount",baseValues.httpRetryCount);
	iniSettings->setValue("UI/UiUpdateInterval",baseValues.uiUpdateInterval);
	iniSettings->setValue("UI/DepthAutoResizeColumns",ui.depthAutoResize->isChecked());

	iniSettings->setValue("UI/RulesSafeModeInterval",baseValues.rulesSafeModeInterval);
	baseValues.rulesSafeModeInterval=iniSettings->value("UI/RulesSafeModeInterval",baseValues.rulesSafeMode).toInt();

	baseValues.rulesSafeMode=iniSettings->value("UI/RulesSafeMode",baseValues.rulesSafeMode).toBool();
	iniSettings->setValue("UI/RulesSafeMode",baseValues.rulesSafeMode);
	ui.delauBetweenExecutingRules->setChecked(baseValues.rulesSafeMode);


	if(iniFileVersion<1.07968)
	{
		iniSettings->beginGroup("Rules");
		QStringList loadRulesGroups=iniSettings->childKeys();
		iniSettings->endGroup();
		for(int n=0;n<loadRulesGroups.count();n++)
		{
			QString ruleData=iniSettings->value("Rules/"+loadRulesGroups.at(n),"").toString();
			iniSettings->remove("Rules/"+loadRulesGroups.at(n));
			QString rName=QString::number(n+1);
			while(rName.count()<3)rName.prepend("0");
			iniSettings->setValue("Rules/"+rName,ruleData+":"+loadRulesGroups.at(n));
		}
	}

	profileName=iniSettings->value("Profile/Name","Default Profile").toString();
	windowTitleP=profileName+" - "+windowTitle()+" v"+baseValues.appVerStr;
	if(debugLevel)windowTitleP.append(" [DEBUG MODE]");
	else if(baseValues.appVerIsBeta)windowTitleP.append(" [BETA]");

	windowWidget->setWindowTitle(windowTitleP);

	foreach(QTableWidget* tables, findChildren<QTableWidget*>())
	{
		tables->setMinimumWidth(200);
		tables->setMinimumHeight(200);
		tables->verticalHeader()->setDefaultSectionSize(defaultHeightForRow);
	}

	foreach(QTableView* tables, findChildren<QTableView*>())
	{
		QFont tableFont=tables->font();
		tableFont.setFixedPitch(true);
		tables->setFont(tableFont);
		tables->setMinimumWidth(200);
		tables->setMinimumHeight(200);
		tables->verticalHeader()->setDefaultSectionSize(defaultHeightForRow);
	}

	int defaultMinWidth=qMax(1024,minimumSizeHint().width());
	int defaultMinHeight=qMax(720,minimumSizeHint().height());

	baseValues.highResolutionDisplay=false;
	int screenCount=QApplication::desktop()->screenCount();
	
	QRect currScrRect;
	for(int n=0;n<screenCount;n++)
	{
		currScrRect=QApplication::desktop()->availableGeometry(n);
		if(currScrRect.width()>defaultMinWidth&&currScrRect.height()>defaultMinHeight)
		{
			baseValues.highResolutionDisplay=true;
			break;
		}
	}

	if(!baseValues.highResolutionDisplay)
	{
		WindowScrollBars *windScroll=new WindowScrollBars;
		windScroll->setWidgetResizable(true);
		windScroll->resize(qMin(defaultMinWidth,currScrRect.width()-40),qMin(defaultMinHeight,currScrRect.height()-80));
		windScroll->setWidget(this);
		windowWidget=windScroll;
	}

	if(iniSettings->value("UI/SwapDepth",false).toBool())on_swapDepth_clicked();

	ui.depthLag->setValue(0.0);

	ui.tabOrdersLogOnTop->setVisible(false);
	ui.tabRulesOnTop->setVisible(false);
	ui.tabTradesOnTop->setVisible(false);
	ui.tabChartsOnTop->setVisible(false);
	ui.tabDepthOnTop->setVisible(false);

	ui.tabOrdersLog->installEventFilter(this);
	ui.tabRules->installEventFilter(this);
	ui.tabLastTrades->installEventFilter(this);
	ui.tabCharts->installEventFilter(this);
	ui.tabDepth->installEventFilter(this);
	ui.balanceTotalWidget->installEventFilter(this);

	copyTableValuesMenu.addAction("Copy selected Rows",this,SLOT(copySelectedRow()));
	copyTableValuesMenu.addSeparator();
	copyTableValuesMenu.addAction("Copy Date",this,SLOT(copyDate()));
	copyTableValuesMenu.addAction("Copy Amount",this,SLOT(copyAmount()));
	copyTableValuesMenu.addAction("Copy Price",this,SLOT(copyPrice()));
	copyTableValuesMenu.addAction("Copy Total",this,SLOT(copyTotal()));
	copyTableValuesMenu.addSeparator();
	copyTableValuesMenu.addAction("Repeat Buy and Sell order",this,SLOT(repeatBuySellOrder()));
	copyTableValuesMenu.addAction("Repeat Buy order",this,SLOT(repeatBuyOrder()));
	copyTableValuesMenu.addAction("Repeat Sell order",this,SLOT(repeatSellOrder()));
	copyTableValuesMenu.addSeparator();
	copyTableValuesMenu.addAction("Cancel Order",this,SLOT(on_ordersCancelSelected_clicked()));
	copyTableValuesMenu.addAction("Cancel All Orders",this,SLOT(on_ordersCancelAllButton_clicked()));

	on_accountFee_valueChanged(ui.accountFee->value());

	reloadLanguageList();

	connect(&julyTranslator,SIGNAL(languageChanged()),this,SLOT(languageChanged()));

	if(checkForUpdates)QProcess::startDetached(QApplication::applicationFilePath(),QStringList("/checkupdate"));
	if(ui.langComboBox->count()==0)fixAllChildButtonsAndLabels(this);

	iniSettings->sync();

	secondSlot();
}

QtBitcoinTrader::~QtBitcoinTrader()
{
	if(iniSettings)iniSettings->sync();
	if(trayIcon)trayIcon->hide();
	if(trayMenu)delete trayMenu;
}

void QtBitcoinTrader::setupClass()
{
	switch(exchangeId)
	{
	case 0: currentExchange=new Exchange_MtGox(baseValues.restSign,baseValues.restKey);break;//Mt.Gox
	case 1: currentExchange=new Exchange_BTCe(baseValues.restSign,baseValues.restKey);break;//BTC-E
	case 2: currentExchange=new Exchange_Bitstamp(baseValues.restSign,baseValues.restKey);break;//Bitstamp
	case 3: currentExchange=new Exchange_BTCChina(baseValues.restSign,baseValues.restKey);break;//BTC China
	case 4: currentExchange=new Exchange_Bitfinex(baseValues.restSign,baseValues.restKey);break;//Bitfinex
	default: return;
	}
	currentExchange->setupApi(this,false);
	setApiDown(false);

	if(!currentExchange->exchangeTickerSupportsHiLowPrices)
		for(int n=0;n<ui.highLowLayout->count();n++)
		{
			QWidgetItem *curWid=dynamic_cast<QWidgetItem*>(ui.highLowLayout->itemAt(n));
			if(curWid)curWid->widget()->setVisible(false);
		}

	if(!currentExchange->supportsExchangeFee)
	{
		ui.accountFee->setButtonSymbols(QAbstractSpinBox::UpDownArrows);
		ui.accountFee->setReadOnly(false);
		ui.accountFee->setValue(iniSettings->value("Profile/CustomFee",0.35).toDouble());
		ui.feeSpinboxLayout->addWidget(new JulySpinBoxPicker(ui.accountFee));
	}
	if(!currentExchange->supportsExchangeVolume)
	{
		ui.marketVolumeLabel->setVisible(false);
		ui.btcLabel4->setVisible(false);
		ui.marketVolume->setVisible(false);
	}
	if(currentExchange->clearHistoryOnCurrencyChanged)
	{
		ui.ordersFilterCheckBox->setVisible(false);
		ui.filterOrdersCurrency->setVisible(false);
		ui.centerOrdersTotalSpacer->setVisible(true);
	}
	else
	ui.centerOrdersTotalSpacer->setVisible(false);

	if(!currentExchange->supportsLoginIndicator)
	{
		ui.loginVolumeBack->setVisible(false);
		ui.exchangeLagBack->setVisible(false);
		ui.lagValue->setVisible(false);
		ui.lagMtgoxLabel->setVisible(false);
	}
	else
		if(currentExchange->supportsLoginIndicator&&!currentExchange->supportsAccountVolume)
		{
			ui.labelAccountVolume->setVisible(false);
			ui.btcLabelAccountVolume->setVisible(false);
			ui.accountVolume->setVisible(false);
		}
		if(!currentExchange->supportsExchangeLag)
		{
			ui.exchangeLagBack->setVisible(false);
			ui.lagValue->setVisible(false);
			ui.lagMtgoxLabel->setVisible(false);
		}

		ordersModel->checkDuplicatedOID=currentExchange->checkDuplicatedOID;

		if(iniSettings->value("UI/DetachedLog",isDetachedLog).toBool())detachLog();
		if(iniSettings->value("UI/DetachedRules",isDetachedRules).toBool())detachRules();
		if(iniSettings->value("UI/DetachedTrades",isDetachedRules).toBool())detachTrades();
		if(iniSettings->value("UI/DetachedDepth",isDetachedDepth).toBool())detachDepth();
		if(iniSettings->value("UI/DetachedCharts",isDetachedRules).toBool())detachCharts();

		int savedTab=iniSettings->value("UI/TradesCurrentTab",0).toInt();
		if(savedTab<ui.tabWidget->count())ui.tabWidget->setCurrentIndex(savedTab);

		ui.widgetStaysOnTop->setChecked(iniSettings->value("UI/WindowOnTop",false).toBool());

		loadWindowState(windowWidget,"Window");

		if(baseValues.httpRequestInterval<currentExchange->minimumRequestIntervalAllowed)baseValues.httpRequestInterval=currentExchange->minimumRequestIntervalAllowed;

		iniSettings->sync();

		if(!ui.widgetStaysOnTop->isChecked())
			on_widgetStaysOnTop_toggled(ui.widgetStaysOnTop->isChecked());

		ui.chartsLayout->addWidget(new ThisFeatureUnderDevelopment(this));
		ui.donateGroupBoxBottom->layout()->addWidget(new DonatePanel(this));

		on_currencyComboBox_currentIndexChanged(ui.currencyComboBox->currentIndex());

		iniSettings->beginGroup("Rules");
		QStringList loadRulesGroups=iniSettings->childKeys();
		iniSettings->endGroup();
		for(int n=0;n<loadRulesGroups.count();n++)
		{
			int curGID=loadRulesGroups.at(n).toInt();
			if(baseValues.lastGroupID<curGID)baseValues.lastGroupID=curGID;
			addRuleGroupByID("",0,curGID);
		}

		ui.buyPercentage->setMaximumWidth(ui.buyPercentage->height());
		ui.sellPercentage->setMaximumWidth(ui.sellPercentage->height());

		if(iniSettings->value("UI/NightMode",false).toBool())
			on_buttonNight_clicked();

		languageChanged();
}

void QtBitcoinTrader::on_buyPercentage_clicked()
{
	PercentPicker *percentPicker=new PercentPicker(ui.buyPercentage,ui.buyTotalBtc,getAvailableUSDtoBTC(ui.buyPricePerCoin->value()));
	QPoint execPos=ui.buyAmountPickerBack->mapToGlobal(ui.buyPercentage->geometry().center());
	execPos.setX(execPos.x()-percentPicker->width()/2);
	execPos.setY(execPos.y()-percentPicker->width());
	percentPicker->exec(execPos);
}

void QtBitcoinTrader::on_sellPercentage_clicked()
{
	PercentPicker *percentPicker=new PercentPicker(ui.sellPercentage,ui.sellTotalBtc,getAvailableBTC());
	QPoint execPos=ui.sellAmountPickerBack->mapToGlobal(ui.sellPercentage->geometry().center());
	execPos.setX(execPos.x()-percentPicker->width()/2);
	execPos.setY(execPos.y()-percentPicker->width());
	percentPicker->exec(execPos);
}

void QtBitcoinTrader::ordersFilterChanged()
{
	QString filterSymbol;
	if(ui.ordersFilterCheckBox->isChecked())
		filterSymbol=ui.filterOrdersCurrency->currentText().replace("/","");
	ordersSortModel->setFilterWildcard(filterSymbol);
}

void QtBitcoinTrader::on_delauBetweenExecutingRules_toggled(bool on)
{
	baseValues.rulesSafeMode=on;
	iniSettings->setValue("UI/RulesSafeMode",on);
	iniSettings->sync();
}

void QtBitcoinTrader::tableCopyContextMenuRequested(QPoint point)
{
	QTableView *table=dynamic_cast<QTableView*>(sender());
	if(table==0)return;
	int selectedCount=table->selectionModel()->selectedRows().count();
	if(selectedCount==0)return;

	lastCopyTable=table;
	bool isOpenOrders=table==ui.ordersTable;
	bool isDateAvailable=table!=ui.depthAsksTable&&table!=ui.depthBidsTable;

	copyTableValuesMenu.actions().at(2)->setVisible(isDateAvailable);

	copyTableValuesMenu.actions().at(6)->setEnabled(selectedCount==1);
	copyTableValuesMenu.actions().at(7)->setEnabled(selectedCount==1);
	copyTableValuesMenu.actions().at(8)->setEnabled(selectedCount==1);
	copyTableValuesMenu.actions().at(9)->setEnabled(selectedCount==1);

	copyTableValuesMenu.actions().at(10)->setVisible(isOpenOrders);
	copyTableValuesMenu.actions().at(11)->setVisible(isOpenOrders);
	copyTableValuesMenu.actions().at(12)->setVisible(isOpenOrders);

	copyTableValuesMenu.exec(lastCopyTable->viewport()->mapToGlobal(point));
}

void QtBitcoinTrader::repeatSelectedOrderByType(int type, bool availableOnly)
{
	if(lastCopyTable==0||lastCopyTable->selectionModel()->selectedRows().count()!=1)return;
	int row=lastCopyTable->selectionModel()->selectedRows().first().row();
	if(lastCopyTable==ui.tableTrades)repeatOrderFromTrades(type,row);else
	if(lastCopyTable==ui.tableHistory)repeatOrderFromValues(type,historyModel->getRowPrice(row),historyModel->getRowVolume(row),availableOnly);
	if(lastCopyTable==ui.ordersTable)
	{
		row=ordersSortModel->mapToSource(ordersSortModel->index(row,0)).row();
		repeatOrderFromValues(type,ordersModel->getRowPrice(row),ordersModel->getRowVolume(row)*floatFeeDec,availableOnly);
	}
	if(lastCopyTable==ui.depthAsksTable)depthSelectOrder(swapedDepth?depthBidsModel->index(row, 3):depthAsksModel->index(row, 3),true,type);
	if(lastCopyTable==ui.depthBidsTable)depthSelectOrder(!swapedDepth?depthBidsModel->index(row, 1):depthAsksModel->index(row, 1),false,type);
}

void QtBitcoinTrader::repeatBuySellOrder(){repeatSelectedOrderByType(0);}

void QtBitcoinTrader::repeatBuyOrder(){repeatSelectedOrderByType(1);}

void QtBitcoinTrader::repeatSellOrder(){repeatSelectedOrderByType(-1);}

void QtBitcoinTrader::copyDate()
{
	if(lastCopyTable==0)return;
	if(lastCopyTable==ui.tableHistory)copyInfoFromTable(ui.tableHistory, historyModel, 1);else
	if(lastCopyTable==ui.tableTrades)copyInfoFromTable(ui.tableTrades, tradesModel, 1);else
	if(lastCopyTable==ui.ordersTable)copyInfoFromTable(ui.ordersTable, ordersSortModel, 1);
}

void QtBitcoinTrader::copyAmount()
{
	if(lastCopyTable==0)return;
	if(lastCopyTable==ui.tableHistory)copyInfoFromTable(ui.tableHistory, historyModel, 2);else
	if(lastCopyTable==ui.tableTrades)copyInfoFromTable(ui.tableTrades, tradesModel, 2);else
		
	if(lastCopyTable==ui.depthAsksTable)copyInfoFromTable(ui.depthAsksTable, swapedDepth?depthBidsModel:depthAsksModel, 3);else
	if(lastCopyTable==ui.depthBidsTable)copyInfoFromTable(ui.depthBidsTable, swapedDepth?depthAsksModel:depthBidsModel, 1);else
	if(lastCopyTable==ui.ordersTable)copyInfoFromTable(ui.ordersTable, ordersSortModel, 4);
}

void QtBitcoinTrader::copyPrice()
{
	if(lastCopyTable==0)return;
	if(lastCopyTable==ui.tableHistory)copyInfoFromTable(ui.tableHistory, historyModel, 4);else
	if(lastCopyTable==ui.tableTrades)copyInfoFromTable(ui.tableTrades, tradesModel, 5);else

	if(lastCopyTable==ui.depthAsksTable)copyInfoFromTable(ui.depthAsksTable, swapedDepth?depthBidsModel:depthAsksModel, 4);else
	if(lastCopyTable==ui.depthBidsTable)copyInfoFromTable(ui.depthBidsTable, swapedDepth?depthAsksModel:depthBidsModel, 0);else
	if(lastCopyTable==ui.ordersTable)copyInfoFromTable(ui.ordersTable, ordersSortModel, 5);
}

void QtBitcoinTrader::copyTotal()
{
	if(lastCopyTable==0)return;
	if(lastCopyTable==ui.tableHistory)copyInfoFromTable(ui.tableHistory, historyModel, 5);else
	if(lastCopyTable==ui.tableTrades)copyInfoFromTable(ui.tableTrades, tradesModel, 6);else

	if(lastCopyTable==ui.depthAsksTable)copyInfoFromTable(ui.depthAsksTable, swapedDepth?depthBidsModel:depthAsksModel, 1);else
	if(lastCopyTable==ui.depthBidsTable)copyInfoFromTable(ui.depthBidsTable, swapedDepth?depthAsksModel:depthBidsModel, 3);else
	if(lastCopyTable==ui.ordersTable)copyInfoFromTable(ui.ordersTable, ordersSortModel, 6);
}

void QtBitcoinTrader::copyInfoFromTable(QTableView *table, QAbstractItemModel *model, int i)
{
	  QModelIndexList selectedRows=table->selectionModel()->selectedRows();
	  if(selectedRows.count()==0)return;
	  QStringList copyData;
	  for(int n=0;n<selectedRows.count();n++)
	  {
		  bool getToolTip=false;
		  if(table==ui.tableHistory&&i==1||
			 table==ui.tableTrades&&i==1)getToolTip=true;

		  copyData<<model->index(selectedRows.at(n).row(), i).data((getToolTip?Qt::ToolTipRole:Qt::DisplayRole)).toString();
	  }
	  QApplication::clipboard()->setText(copyData.join("\n"));
}

void QtBitcoinTrader::copySelectedRow()
{
	QStringList listToCopy;

	if(lastCopyTable==0)return;
	QModelIndexList selectedRows=lastCopyTable->selectionModel()->selectedRows();
	if(selectedRows.count()==0)return;
	for(int n=0;n<selectedRows.count();n++)
	{
		QString currentText=selectedRows.at(n).data(Qt::StatusTipRole).toString();
		if(!currentText.isEmpty())listToCopy<<currentText;
	}

	if(listToCopy.isEmpty())return;
	QApplication::clipboard()->setText(listToCopy.join("\n"));
}

void QtBitcoinTrader::addRuleGroupByID(QString groupName, int copyFrom, int gID)
{
	if(gID==0)gID=++baseValues.lastGroupID;
	RuleWidget *copyFromGroup=0;
	if(copyFrom)
	foreach(RuleWidget* currentGroup, ui.tabRules->findChildren<RuleWidget*>())
		if(currentGroup->getRuleGroupId()==copyFrom)copyFromGroup=currentGroup;
	RuleWidget *newRule=new RuleWidget(gID,groupName,copyFromGroup);
	ui.rulesTabs->insertTab(ui.rulesTabs->count(),newRule,newRule->windowTitle());

	ui.rulesTabs->setVisible(true);
	ui.rulesNoMessage->setVisible(false);
}

void QtBitcoinTrader::on_buttonAddRuleGroup_clicked()
{
	AddRuleGroup ruleGroup;
	if(ui.widgetStaysOnTop->isChecked())ruleGroup.setWindowFlags(Qt::WindowCloseButtonHint|Qt::WindowStaysOnTopHint);

	if(ruleGroup.exec()==QDialog::Rejected||ruleGroup.groupName.isEmpty())return;

	addRuleGroupByID(ruleGroup.groupName,ruleGroup.copyFromExistingGroup);

	ui.rulesTabs->setCurrentIndex(ui.rulesTabs->count()-1);
}

void QtBitcoinTrader::keyPressEvent(QKeyEvent *event)
{
	event->accept();

	if(ui.ordersTable->hasFocus()&&(event->key()==Qt::Key_Delete||event->key()==Qt::Key_Backspace))
	{
		on_ordersCancelSelected_clicked();
		return;
	}

	if(event->modifiers()&Qt::ControlModifier)
	{
		if(event->key()==Qt::Key_B)buyBitcoinsButton();
		else
		if(event->key()==Qt::Key_S)sellBitcoinButton();
		else
		if(event->key()==Qt::Key_H)buttonMinimizeToTray();
		else
		if(event->key()==Qt::Key_N)buttonNewWindow();
		else
		if(event->key()==Qt::Key_T)ui.widgetStaysOnTop->setChecked(!ui.widgetStaysOnTop->isChecked());
		return;
	}
	if(debugLevel==0&&event->key()==Qt::Key_D&&
	  (event->modifiers()&Qt::ControlModifier&&event->modifiers()&Qt::ShiftModifier||
	   event->modifiers()&Qt::ControlModifier&&event->modifiers()&Qt::AltModifier||
	   event->modifiers()&Qt::ShiftModifier&&event->modifiers()&Qt::AltModifier))
	{
		new DebugViewer;
	}
}

void QtBitcoinTrader::precentBidsChanged(double val)
{
	ui.tradesBidsPrecent->setValue(val);

	static bool lastPrecentGrowing=false;
	bool precentGrowing=tradesPrecentLast<val;
	if(lastPrecentGrowing!=precentGrowing)
	{
		lastPrecentGrowing=precentGrowing;
		ui.tradesLabelDirection->setText(precentGrowing?upArrowNoUtfStr:downArrowNoUtfStr);
	}
	tradesPrecentLast=val;
}

void QtBitcoinTrader::on_swapDepth_clicked()
{
	swapedDepth=!swapedDepth;

	if(swapedDepth)
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

	QString tempText=ui.asksLabel->text();
	QString tempId=ui.asksLabel->accessibleName();
	QString tempStyle=ui.asksLabel->styleSheet();

	ui.asksLabel->setText(ui.bidsLabel->text());
	ui.asksLabel->setAccessibleName(ui.bidsLabel->accessibleName());
	ui.asksLabel->setStyleSheet(ui.bidsLabel->styleSheet());

	ui.bidsLabel->setText(tempText);
	ui.bidsLabel->setAccessibleName(tempId);
	ui.bidsLabel->setStyleSheet(tempStyle);

	iniSettings->setValue("UI/SwapDepth",swapedDepth);
	iniSettings->sync();
}

void QtBitcoinTrader::anyDataReceived()
{
	softLagTime.restart();
	setSoftLagValue(0);
}

double QtBitcoinTrader::getFeeForUSDDec(double usd)
{
	double result=getValidDoubleForPercision(usd,baseValues.currentPair.currBDecimals,false);
	double calcFee=getValidDoubleForPercision(result,baseValues.currentPair.priceDecimals,true)*floatFee;
	calcFee=getValidDoubleForPercision(calcFee,baseValues.currentPair.priceDecimals,true);
	result=result-calcFee;
	return result;
}

double QtBitcoinTrader::getValidDoubleForPercision(const double &val, const int &percision, bool roundUp)
{
	int intVal=val;
	int percisionValue=qPow(10,percision);
	int intMultipled=(val-intVal)*percisionValue;
	if(roundUp)intMultipled++;
	return (double)intMultipled/percisionValue+intVal;
}

void QtBitcoinTrader::addPopupDialog(int val)
{
	static int currentPopupDialogs=0;
	currentPopupDialogs+=val;
	ui.aboutTranslationButton->setEnabled(currentPopupDialogs==0);
	ui.buttonNewWindow->setEnabled(currentPopupDialogs==0);
}

void QtBitcoinTrader::buttonMinimizeToTray()
{
	if(trayIcon==0)
	{
		trayIcon=new QSystemTrayIcon(QIcon(":/Resources/QtBitcoinTrader.png"),this);
		trayIcon->setToolTip(windowTitle());
		connect(trayIcon,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),this,SLOT(trayActivated(QSystemTrayIcon::ActivationReason)));
		trayMenu=new QMenu;
		trayIcon->setContextMenu(trayMenu);
		trayMenu->addAction(QIcon(":/Resources/exit.png"),"Exit");
		trayMenu->actions().last()->setWhatsThis("EXIT");
		connect(trayMenu->actions().first(),SIGNAL(triggered(bool)),this,SLOT(exitApp()));
	}
	trayIcon->show();
	trayIcon->showMessage(windowTitleP,windowTitle());
	hide();
	if(ui.tabOrdersLog->parent()==0)ui.tabOrdersLog->hide();
	if(ui.tabLastTrades->parent()==0)ui.tabLastTrades->hide();
	if(ui.tabRules->parent()==0)ui.tabRules->hide();
	if(ui.tabDepth->parent()==0)ui.tabDepth->hide();
	if(ui.tabCharts->parent()==0)ui.tabCharts->hide();
}

void QtBitcoinTrader::trayActivated(QSystemTrayIcon::ActivationReason reazon)
{
	if(trayIcon==0)return;
	if(reazon==QSystemTrayIcon::Context)
	{
		QList<QAction*> tList=trayMenu->actions();
		for(int n=0;n<tList.count();n++)
			tList.at(n)->setText(julyTr(tList.at(n)->whatsThis(),tList.at(n)->text()));
		trayMenu->exec();
		return;
	}
	windowWidget->show();
	if(ui.tabOrdersLog->parent()==0)ui.tabOrdersLog->show();
	if(ui.tabLastTrades->parent()==0)ui.tabLastTrades->show();
	if(ui.tabRules->parent()==0)ui.tabRules->show();
	if(ui.tabDepth->parent()==0)ui.tabDepth->show();
	if(ui.tabCharts->parent()==0)ui.tabCharts->show();
	trayIcon->hide();
	delete trayMenu; trayMenu=0;
	delete trayIcon; trayIcon=0;
}

void QtBitcoinTrader::on_tabOrdersLogOnTop_toggled(bool on)
{
	if(!isDetachedLog)return;
	ui.tabOrdersLog->hide();
	if(on)ui.tabOrdersLog->setWindowFlags(Qt::Window|Qt::WindowStaysOnTopHint);
	else  ui.tabOrdersLog->setWindowFlags(Qt::Window);
	ui.tabOrdersLog->show();
}

void QtBitcoinTrader::on_tabRulesOnTop_toggled(bool on)
{
	if(!isDetachedRules)return;
	ui.tabRules->hide();
	if(on)ui.tabRules->setWindowFlags(Qt::Window|Qt::WindowStaysOnTopHint);
	else  ui.tabRules->setWindowFlags(Qt::Window);
	ui.tabRules->show();
}

void QtBitcoinTrader::on_tabTradesOnTop_toggled(bool on)
{
	if(!isDetachedTrades)return;
	ui.tabLastTrades->hide();
	if(on)ui.tabLastTrades->setWindowFlags(Qt::Window|Qt::WindowStaysOnTopHint);
	else  ui.tabLastTrades->setWindowFlags(Qt::Window);
	ui.tabLastTrades->show();
}

void QtBitcoinTrader::on_tabDepthOnTop_toggled(bool on)
{
	if(!isDetachedDepth)return;
	ui.tabDepth->hide();
	if(on)ui.tabDepth->setWindowFlags(Qt::Window|Qt::WindowStaysOnTopHint);
	else  ui.tabDepth->setWindowFlags(Qt::Window);
	ui.tabDepth->show();
}

void QtBitcoinTrader::on_tabChartsOnTop_toggled(bool on)
{
	if(!isDetachedCharts)return;
	ui.tabCharts->hide();
	if(on)ui.tabCharts->setWindowFlags(Qt::Window|Qt::WindowStaysOnTopHint);
	else  ui.tabCharts->setWindowFlags(Qt::Window);
	ui.tabCharts->show();
}

bool QtBitcoinTrader::isValidGeometry(QRect *geo, int yMargin)
{
	QRect allDesktopsRect=QApplication::desktop()->geometry();
	return geo->width()>100&&geo->height()>100&&allDesktopsRect.contains(QPoint(geo->topLeft().x(),geo->topLeft().y()-yMargin),true)&&allDesktopsRect.contains(geo->bottomRight(),true);
}

void QtBitcoinTrader::checkUpdate()
{
    QProcess::startDetached(QApplication::applicationFilePath(),QStringList("/checkupdatemessage"));
}

void WindowScrollBars::resizeEvent(QResizeEvent *event)
{
	mainWindow.resizeEvent(event);
}

void QtBitcoinTrader::resizeEvent(QResizeEvent *event)
{
	event->accept();
	depthAsksLastScrollValue=-1;
	depthBidsLastScrollValue=-1;
	fixWindowMinimumSize();
}

void QtBitcoinTrader::setLastTrades10MinVolume(double val)
{
	ui.tradesVolume5m->setValue(val);
}

void QtBitcoinTrader::availableAmountChanged(double val)
{
	availableAmount=val;
}

void QtBitcoinTrader::addLastTrades(QList<TradesItem> *newItems)
{
	int newRowsCount=newItems->count();
	tradesModel->addNewTrades(newItems);

	tradesModel->updateTotalBTC();
	if(ui.tradesAutoScrollCheck->isChecked()&&ui.tabLastTrades->isVisible())
	{
		setTradesScrollBarValue(ui.tableTrades->verticalScrollBar()->value()+defaultHeightForRow*newRowsCount);
		tabTradesScrollUp();
	}
}

void QtBitcoinTrader::clearTimeOutedTrades()
{
	if(tradesModel->rowCount()==0)return;
	int lastSliderValue=ui.tableTrades->verticalScrollBar()->value();
	tradesModel->removeDataOlderThen(QDateTime::currentDateTime().addSecs(-600).toTime_t());
	ui.tableTrades->verticalScrollBar()->setValue(qMin(lastSliderValue,ui.tableTrades->verticalScrollBar()->maximum()));
}

void QtBitcoinTrader::depthRequested()
{
	depthLagTime.restart();
	waitingDepthLag=true;
}

void QtBitcoinTrader::depthRequestReceived()
{
	waitingDepthLag=false;
}

void QtBitcoinTrader::secondSlot()
{
	static QTimer secondTimer(this);
	if(secondTimer.interval()==0)
	{
		secondTimer.setSingleShot(true);
		connect(&secondTimer,SIGNAL(timeout()),this,SLOT(secondSlot()));
	}
	static int execCount=0;

	if(execCount==0||execCount==2||execCount==4)
	{
		if(currentExchange)currentExchange->depthEnabled=ui.tabDepth->isVisible()||depthAsksModel->rowCount()==0||depthBidsModel->rowCount()==0;
		clearTimeOutedTrades();
		setSoftLagValue(softLagTime.elapsed());
	}
	else
	if(execCount==1||execCount==3||execCount==5)
	{
		int currentElapsed=depthLagTime.elapsed();
		if(ui.tabDepth->isVisible())ui.depthLag->setValue(currentElapsed/1000.0);
	}

	static QTime historyForceUpdate(0,0,0,0);
	if(historyForceUpdate.elapsed()>=15000)
	{
		historyForceUpdate.restart();
		emit getHistory(true);
	}

	static QTime speedTestTime(0,0,0,0);
	if(speedTestTime.elapsed()>=500)
	{
		speedTestTime.restart();
		checkAllRules();

		static QList<double> halfSecondsList;
		halfSecondsList<<(double)baseValues.trafficSpeed/512.0;
		baseValues.trafficTotal+=baseValues.trafficSpeed;
		updateTrafficTotalValue();
		while(halfSecondsList.count()>10)halfSecondsList.removeFirst();
		static double avSpeed;avSpeed=0.0;
		for(int n=0;n<halfSecondsList.count();n++)avSpeed+=halfSecondsList.at(n);
		if(halfSecondsList.count())
			avSpeed=avSpeed*2.0/halfSecondsList.count();
		ui.trafficSpeed->setValue(avSpeed);
		baseValues.trafficSpeed=0;
	}

	if(++execCount>5)execCount=0;
	secondTimer.start(baseValues.uiUpdateInterval);
}

void QtBitcoinTrader::on_trafficTotalToZero_clicked()
{
	baseValues.trafficTotal=0;
	updateTrafficTotalValue();
}

void QtBitcoinTrader::updateTrafficTotalValue()
{
	static int trafficTotalTypeLast=-1;

	if(baseValues.trafficTotal>1073741824)//Gb
		baseValues.trafficTotalType=2;
	else
	if(baseValues.trafficTotal>1048576)//Mb
		baseValues.trafficTotalType=1;
	else
	if(baseValues.trafficTotal>1024)//Kb
		baseValues.trafficTotalType=0;

	if(trafficTotalTypeLast!=baseValues.trafficTotalType)
	{
		trafficTotalTypeLast=baseValues.trafficTotalType;

		switch(trafficTotalTypeLast)
		{
		case 0: ui.trafficTotal->setSuffix(" Kb"); break;
		case 1: ui.trafficTotal->setSuffix(" Mb"); break;
		case 2: ui.trafficTotal->setSuffix(" Gb"); break;
		default: break;
		}
	}
	static int totalValueLast=-1;
	int totalValue=0;
	switch(trafficTotalTypeLast)
	{
	case 0: totalValue=baseValues.trafficTotal/1024; break;
	case 1: totalValue=baseValues.trafficTotal/1048576; break;
	case 2: totalValue=baseValues.trafficTotal/1073741824; break;
	default: break;
	}
	if(totalValueLast!=totalValue)
	{
	totalValueLast=totalValue;
	if(ui.trafficTotal->maximum()<totalValue)ui.trafficTotal->setMaximum(totalValue*10);
	ui.trafficTotal->setValue(totalValue);
	}
}

void QtBitcoinTrader::tabTradesScrollUp()
{
	static QTimeLine timeLine(1,this);
	if(timeLine.duration()==1)
	{
		connect(&timeLine,SIGNAL(frameChanged(int)),this,SLOT(setTradesScrollBarValue(int)));
		timeLine.setDuration(500);
		timeLine.setEasingCurve(QEasingCurve::OutCirc);
		timeLine.setLoopCount(1);
	}
	timeLine.stop();
	int currentScrollPos=ui.tableTrades->verticalScrollBar()->value();
	if(currentScrollPos==0)return;
	timeLine.setFrameRange(currentScrollPos,0);
	timeLine.start();
}

void QtBitcoinTrader::tabTradesIndexChanged(int)
{
	if(ui.tabLastTrades->isVisible()&&ui.tradesAutoScrollCheck->isChecked())tabTradesScrollUp();
}

void QtBitcoinTrader::setTradesScrollBarValue(int val)
{
	if(val>ui.tableTrades->verticalScrollBar()->maximum())tabTradesScrollUp();
	else
	ui.tableTrades->verticalScrollBar()->setValue(val);
	//ui.tableTrades->verticalScrollBar()->setValue(qMin(val,ui.tableTrades->verticalScrollBar()->maximum()));
}

void QtBitcoinTrader::fillAllUsdLabels(QWidget *par, QString curName)
{
	curName=curName.toUpper();
	QPixmap btcPixmap("://Resources/CurrencySign/"+curName+".png");
	foreach(QLabel* labels, par->findChildren<QLabel*>())
		if(labels->accessibleDescription()=="USD_ICON")
		{
			labels->setPixmap(btcPixmap);
			labels->setToolTip(curName);
		}
}
void QtBitcoinTrader::fillAllBtcLabels(QWidget *par, QString curName)
{
	curName=curName.toUpper();
	QPixmap btcPixmap("://Resources/CurrencySign/"+curName+".png");
	foreach(QLabel* labels, par->findChildren<QLabel*>())
	{
		if(labels->accessibleDescription()=="BTC_ICON")
		{
			labels->setPixmap(btcPixmap);
			labels->setToolTip(curName);
		}
		else
		if(labels->accessibleDescription()=="DONATE_BTC_ICON")
		{
			if(labels->toolTip()!="BTC")
			{
			labels->setPixmap(QPixmap("://Resources/CurrencySign/BTC.png"));
			labels->setToolTip("BTC");
			}
		}
	}
}

void QtBitcoinTrader::makeRitchValue(QString *text)
{
	int lastSymbol=text->length()-1;
	if(lastSymbol==-1)return;
	while(lastSymbol>-1&&text->at(lastSymbol)=='0')lastSymbol--;
	if(lastSymbol>-1&&text->at(lastSymbol)=='.')lastSymbol--;
	if(lastSymbol<-1)return;
	QString buff=text->left(lastSymbol+1);
	text->remove(0,lastSymbol+1);
	text->prepend("<font color=gray>");
	text->append("</font>");
	text->prepend(buff);
}

void QtBitcoinTrader::reloadLanguageList(QString preferedLangFile)
{
	if(preferedLangFile.isEmpty())preferedLangFile=julyTranslator.lastFile();
	if(!QFile::exists(preferedLangFile))preferedLangFile.clear();
	constructorFinished=false;
	ui.langComboBox->clear();

	QStringList langList;
	QFile resLanguage(":/Resources/Language/LangList.ini");
	resLanguage.open(QIODevice::ReadOnly);
	QStringList resourceLanguages=QString(resLanguage.readAll().replace("\r","")).split("\n");
	for(int n=0;n<resourceLanguages.count();n++)if(!resourceLanguages.at(n).isEmpty())langList<<":/Resources/Language/"+resourceLanguages.at(n);
	QStringList folderLangList=QDir(appDataDir+"Language","*.lng").entryList();
	folderLangList.sort();
	for(int n=0;n<folderLangList.count();n++)langList<<appDataDir+"Language/"+folderLangList.at(n);
	int selectedLangId=-1;
	if(preferedLangFile.isEmpty()||!preferedLangFile.isEmpty()&&!QFile::exists(preferedLangFile))preferedLangFile=baseValues.defaultLangFile;
	for(int n=0;n<langList.count();n++)
	{
		JulyTranslator translateName;
		translateName.loadFromFile(langList.at(n));
		QString langName=translateName.translateString("LANGUAGE_NAME","");
		if(langName.isEmpty())continue;
		if(preferedLangFile==langList.at(n))selectedLangId=n;
		ui.langComboBox->insertItem(ui.langComboBox->count(),langName,langList.at(n));
	}
	if(selectedLangId>-1)ui.langComboBox->setCurrentIndex(selectedLangId);
	julyTranslator.loadFromFile(preferedLangFile);
	constructorFinished=true;
	languageChanged();
}

void QtBitcoinTrader::languageComboBoxChanged(int val)
{
	if(val<0||!constructorFinished)return;
	QString loadFromFile=ui.langComboBox->itemData(val,Qt::UserRole).toString();
	if(loadFromFile.isEmpty())return;
	julyTranslator.loadFromFile(loadFromFile);
	QSettings settings(appDataDir+"/QtBitcoinTrader.cfg",QSettings::IniFormat);
	settings.setValue("LanguageFile",loadFromFile);
}

void QtBitcoinTrader::fixAllChildButtonsAndLabels(QWidget *par)
{
	foreach(QPushButton* pushButtons, par->findChildren<QPushButton*>())
		if(!pushButtons->text().isEmpty())
			pushButtons->setMinimumWidth(qMin(pushButtons->maximumWidth(),textFontWidth(pushButtons->text())+10));

	foreach(QToolButton* toolButtons, par->findChildren<QToolButton*>())
		if(!toolButtons->text().isEmpty())
			toolButtons->setMinimumWidth(qMin(toolButtons->maximumWidth(),textFontWidth(toolButtons->text())+10));

	foreach(QCheckBox* checkBoxes, par->findChildren<QCheckBox*>())
		checkBoxes->setMinimumWidth(qMin(checkBoxes->maximumWidth(),textFontWidth(checkBoxes->text())+20));

	foreach(QLabel* labels, par->findChildren<QLabel*>())
		if(labels->text().length()&&labels->text().at(0)!='<'&&labels->accessibleDescription()!="IGNORED")
			labels->setMinimumWidth(qMin(labels->maximumWidth(),textFontWidth(labels->text())));

	fixDecimals(this);

	foreach(QGroupBox* groupBox, par->findChildren<QGroupBox*>())
		if(groupBox->maximumWidth()>1000)
		{
			int minWidth=qMax(groupBox->minimumSizeHint().width(),textFontWidth(groupBox->title())+20);
			if(groupBox->accessibleDescription()=="FIXED")groupBox->setFixedWidth(minWidth);
			else groupBox->setMinimumWidth(minWidth);
		}

	QSize minSizeHint=par->minimumSizeHint();
	if(isValidSize(&minSizeHint))
	{
	par->setMinimumSize(par->minimumSizeHint());
	if(par->width()<par->minimumSizeHint().width())par->resize(par->minimumSizeHint().width(),par->height());
	}
}

void QtBitcoinTrader::fixDecimals(QWidget *par)
{
	foreach(QDoubleSpinBox* spinBox, par->findChildren<QDoubleSpinBox*>())
	{
		if(spinBox->accessibleName().startsWith("BTC"))
		{
			if(spinBox->accessibleName().endsWith("BALANCE"))spinBox->setDecimals(baseValues.currentPair.currABalanceDecimals);
			else spinBox->setDecimals(baseValues.currentPair.currADecimals);
			if(spinBox->accessibleDescription()!="CAN_BE_ZERO")
				spinBox->setMinimum(baseValues.currentPair.tradeVolumeMin);
		}
		else
		if(spinBox->accessibleName().startsWith("USD"))
		{
			if(spinBox->accessibleName().endsWith("BALANCE"))spinBox->setDecimals(baseValues.currentPair.currBBalanceDecimals);
			else spinBox->setDecimals(baseValues.currentPair.currBDecimals);
		}
		else
		if(spinBox->accessibleName()=="PRICE")
		{
			spinBox->setDecimals(baseValues.currentPair.priceDecimals);
			if(spinBox->accessibleDescription()!="CAN_BE_ZERO")
				spinBox->setMinimum(baseValues.currentPair.tradePriceMin);
		}
	}
}

void QtBitcoinTrader::loginChanged(QString text)
{
	if(text==" ")text=profileName;
	ui.accountLoginLabel->setText(text);
	ui.accountLoginLabel->setMinimumWidth(textFontWidth(text)+20);
	fixWindowMinimumSize();
}

void QtBitcoinTrader::setCurrencyPairsList(QList<CurrencyPairItem> *currPairs)
{
	currPairsList=*currPairs;
	delete currPairs;

	QString savedCurrency=iniSettings->value("Profile/Currency","BTC/USD").toString();

	ui.currencyComboBox->clear();
	ui.filterOrdersCurrency->clear();
	
	QStringList currencyItems;
	int indexCurrency=-1;
	for(int n=0;n<currPairsList.count();n++)
	{
		if(currPairsList.at(n).name==savedCurrency)indexCurrency=n;
		currencyItems<<currPairsList.at(n).name;
	}
	ui.currencyComboBox->insertItems(0,currencyItems);
	ui.filterOrdersCurrency->insertItems(0,currencyItems);
	ui.filterOrdersCurrency->setCurrentIndex(0);

	if(indexCurrency>-1)ui.currencyComboBox->setCurrentIndex(indexCurrency);
	lastLoadedCurrency=-2;
	on_currencyComboBox_currentIndexChanged(indexCurrency);
}

void QtBitcoinTrader::on_currencyComboBox_currentIndexChanged(int val)
{
	if(!constructorFinished||val<0||currPairsList.count()!=ui.currencyComboBox->count())return;
	
	bool fastChange=ui.currencyComboBox->itemText(val).left(5)==ui.currencyComboBox->itemText(lastLoadedCurrency).left(5);
	if(val!=lastLoadedCurrency)
	{
		bool haveAnyTradeRule=false;

		foreach(RuleWidget* currentGroup, ui.tabRules->findChildren<RuleWidget*>())
			if(currentGroup&&currentGroup->haveAnyTradingRules())
			{
				haveAnyTradeRule=true;
				break;
			}
		if(haveAnyTradeRule)
		{
			ui.currencyComboBox->setCurrentIndex(lastLoadedCurrency);
			QMessageBox::warning(this,windowTitle(),julyTr("CANNOT_CHANGE_CURRENCY","In order to change currency you need to remove trading rules"));
			return;
		}
		lastLoadedCurrency=val;
	}
	else return;
	lastLoadedCurrency=val;

	CurrencyPairItem nextCurrencyPair=currPairsList.at(val);

	bool currencyAChanged=nextCurrencyPair.currAStr!=baseValues.currentPair.currAStr;

	bool currencyBChanged=nextCurrencyPair.currBStr!=baseValues.currentPair.currBStr;

	baseValues.currentPair=nextCurrencyPair;

	if(fastChange)
	{
		ui.accountBTC->setValue(0.0);
		ui.accountUSD->setValue(0.0);
		return;
	}

	fillAllUsdLabels(this,nextCurrencyPair.currBStr);
	fillAllBtcLabels(this,nextCurrencyPair.currAStr);

	if(isDetachedLog)
	{
		fillAllUsdLabels(ui.tabOrdersLog,nextCurrencyPair.currBStr);
		fillAllBtcLabels(ui.tabOrdersLog,nextCurrencyPair.currAStr);
	}


	if(isDetachedRules)
	{
		fillAllUsdLabels(ui.tabRules,nextCurrencyPair.currBStr);
		fillAllBtcLabels(ui.tabRules,nextCurrencyPair.currAStr);
	}

	if(isDetachedTrades)
	{
		fillAllUsdLabels(ui.tabLastTrades,nextCurrencyPair.currBStr);
		fillAllBtcLabels(ui.tabLastTrades,nextCurrencyPair.currAStr);
	}

	if(isDetachedDepth)
	{
		fillAllUsdLabels(ui.tabDepth,nextCurrencyPair.currBStr);
		fillAllBtcLabels(ui.tabDepth,nextCurrencyPair.currAStr);
	}

	if(isDetachedCharts)
	{
		fillAllUsdLabels(ui.tabCharts,nextCurrencyPair.currBStr);
		fillAllBtcLabels(ui.tabCharts,nextCurrencyPair.currAStr);
	}
	
	iniSettings->setValue("Profile/Currency",ui.currencyComboBox->currentText());
	if(currencyAChanged)ui.accountBTC->setValue(0.0);
	if(currencyBChanged)ui.accountUSD->setValue(0.0);
	ui.marketBid->setValue(0.0);
	ui.marketAsk->setValue(0.0);
	ui.marketHigh->setValue(0.0);
	ui.marketLow->setValue(0.0);
	ui.marketLast->setValue(0.0);
	ui.marketVolume->setValue(0.0);
	ui.buyTotalSpend->setValue(0.0);
	ui.sellTotalBtc->setValue(0.0);
	precentBidsChanged(0.0);
	ui.buyPricePerCoin->setValue(100);
	ui.sellPricePerCoin->setValue(200);
	tradesModel->clear();
	ui.tradesVolume5m->setValue(0.0);
	ui.ruleAmountToReceiveValue->setValue(0.0);
	ui.ruleTotalToBuyValue->setValue(0.0);
	ui.ruleAmountToReceiveBSValue->setValue(0.0);
	ui.ruleTotalToBuyBSValue->setValue(0.0);
	tradesPrecentLast=0.0;

	QString buyGroupboxText=julyTr("GROUPBOX_BUY","Buy %1");
	bool buyGroupboxCase=false; if(buyGroupboxText.length()>2)buyGroupboxCase=buyGroupboxText.at(2).isUpper();

	if(buyGroupboxCase)buyGroupboxText=buyGroupboxText.arg(baseValues.currentPair.currAName.toUpper());
	else buyGroupboxText=buyGroupboxText.arg(baseValues.currentPair.currAName);

	ui.buyGroupbox->setTitle(buyGroupboxText);

	QString sellGroupboxText=julyTr("GROUPBOX_SELL","Sell %1");
	bool sellGroupboxCase=true; if(sellGroupboxText.length()>2)sellGroupboxCase=sellGroupboxText.at(2).isUpper();

	if(sellGroupboxCase)sellGroupboxText=sellGroupboxText.arg(baseValues.currentPair.currAName.toUpper());
	else sellGroupboxText=sellGroupboxText.arg(baseValues.currentPair.currAName);

	ui.sellGroupBox->setTitle(sellGroupboxText);

	if(currentExchange->clearHistoryOnCurrencyChanged)
	{
		historyModel->clear();

		ui.ordersLastBuyPrice->setValue(0.0);
		ui.ordersLastSellPrice->setValue(0.0);
	}
	static int firstLoad=0;
	if(firstLoad++>1)
	{
		firstLoad=3;
		emit clearValues();
	}
	clearDepth();

	marketPricesNotLoaded=true;
	balanceNotLoaded=true;
	fixDecimals(this);

	iniSettings->sync();

	depthAsksModel->fixTitleWidths();
	depthBidsModel->fixTitleWidths();

	calcOrdersTotalValues();

	ui.filterOrdersCurrency->setCurrentIndex(val);

	currencyChangedDate=QDateTime::currentDateTime().toTime_t();

	ui.ordersLastBuyPrice->setValue(0.0);
	ui.ordersLastBuyPrice->setValue(0.0);
	emit getHistory(true);
}

void QtBitcoinTrader::clearDepth()
{
	depthAsksModel->clear();
	depthBidsModel->clear();

	emit reloadDepth();
}

void QtBitcoinTrader::volumeAmountChanged(double volumeTotal, double amountTotal)
{
	ui.ordersTotalBTC->setValue(volumeTotal);
	ui.ordersTotalUSD->setValue(amountTotal);
}

void QtBitcoinTrader::calcOrdersTotalValues()
{
	checkValidBuyButtons();
	checkValidSellButtons();
}

void QtBitcoinTrader::firstTicker()
{
	on_sellPriceAsMarketBid_clicked();
	on_buyPriceAsMarketAsk_clicked();
}

void QtBitcoinTrader::profitSellThanBuyCalc()
{
	if(!profitSellThanBuyUnlocked)return;
	profitSellThanBuyUnlocked=false;
	double calcValue=0.0;
	if(ui.sellTotalBtc->value()!=0.0&&ui.buyTotalBtcResult->value()!=0.0)
		calcValue=ui.buyTotalBtcResult->value()-ui.sellTotalBtc->value();
	ui.sellThanBuySpinBox->setValue(calcValue);
	profitSellThanBuyUnlocked=true;
}

void QtBitcoinTrader::profitBuyThanSellCalc()
{
	if(!profitBuyThanSellUnlocked)return;
	profitBuyThanSellUnlocked=false;
	double calcValue=0.0;
	if(ui.buyTotalSpend->value()!=0.0&&ui.sellAmountToReceive->value()!=0.0)
		calcValue=ui.sellAmountToReceive->value()-ui.buyTotalSpend->value();
	ui.profitLossSpinBox->setValue(calcValue);
	profitBuyThanSellUnlocked=true;
}

void QtBitcoinTrader::on_profitLossSpinBoxPrec_valueChanged(double val)
{
	if(profitBuyThanSellChangedUnlocked)
	{
		profitBuyThanSellChangedUnlocked=false;
		ui.profitLossSpinBox->setValue(val==0.0?0.0:ui.buyTotalSpend->value()*val/100.0);
		profitBuyThanSellChangedUnlocked=true;
	}
}

void QtBitcoinTrader::on_profitLossSpinBox_valueChanged(double val)
{
	QString styleChanged;
	if(val<-0.009)styleChanged="QDoubleSpinBox {background: "+baseValues.appTheme.lightRed.name()+";}";
	else
	if(val>0.009)styleChanged="QDoubleSpinBox {background: "+baseValues.appTheme.lightGreen.name()+";}";

	if(profitBuyThanSellChangedUnlocked)
	{
		profitBuyThanSellChangedUnlocked=false;
		ui.profitLossSpinBoxPrec->setValue(ui.buyTotalSpend->value()==0.0?0.0:val*100.0/ui.buyTotalSpend->value());
		profitBuyThanSellChangedUnlocked=true;
	}

	ui.profitLossSpinBox->setStyleSheet(styleChanged);
	ui.profitLossSpinBoxPrec->setStyleSheet(styleChanged);

	ui.buttonBuyThenSellApply->setEnabled(true);
}

void QtBitcoinTrader::on_sellThanBuySpinBoxPrec_valueChanged(double val)
{
	if(profitBuyThanSellChangedUnlocked)
	{
		profitBuyThanSellChangedUnlocked=false;
		ui.sellThanBuySpinBox->setValue(val==0.0?0.0:ui.sellTotalBtc->value()*val/100.0);
		profitBuyThanSellChangedUnlocked=true;
	}
}

void QtBitcoinTrader::on_sellThanBuySpinBox_valueChanged(double val)
{
	QString styleChanged;
	if(val<-0.009)styleChanged="QDoubleSpinBox {background: "+baseValues.appTheme.lightRed.name()+";}";
	else
	if(val>0.009)styleChanged="QDoubleSpinBox {background: "+baseValues.appTheme.lightGreen.name()+";}";

	if(profitBuyThanSellChangedUnlocked)
	{
		profitBuyThanSellChangedUnlocked=false;
		ui.sellThanBuySpinBoxPrec->setValue(ui.sellTotalBtc->value()==0.0?0.0:val*100.0/ui.sellTotalBtc->value());
		profitBuyThanSellChangedUnlocked=true;
	}

	ui.sellThanBuySpinBox->setStyleSheet(styleChanged);
	ui.sellThanBuySpinBoxPrec->setStyleSheet(styleChanged);

	ui.buttonSellThenBuyApply->setEnabled(true);
}

void QtBitcoinTrader::on_zeroSellThanBuyProfit_clicked()
{
	ui.sellThanBuySpinBox->setValue(0.0);
}

void QtBitcoinTrader::on_zeroBuyThanSellProfit_clicked()
{
	ui.profitLossSpinBox->setValue(0.0);
}

void QtBitcoinTrader::profitSellThanBuy()
{
	profitSellThanBuyUnlocked=false;
	ui.buyTotalSpend->setValue(ui.sellAmountToReceive->value());
	ui.buyPricePerCoin->setValue(ui.buyTotalSpend->value()/((ui.sellTotalBtc->value()+ui.sellThanBuySpinBox->value())/floatFeeDec)-baseValues.currentPair.priceMin);
	profitSellThanBuyUnlocked=true;
	profitBuyThanSellCalc();
	profitSellThanBuyCalc();
	ui.buttonSellThenBuyApply->setEnabled(false);
}

void QtBitcoinTrader::profitBuyThanSell()
{
	profitBuyThanSellUnlocked=false;
	ui.sellTotalBtc->setValue(ui.buyTotalBtcResult->value());
	ui.sellPricePerCoin->setValue((ui.buyTotalSpend->value()+ui.profitLossSpinBox->value())/(ui.sellTotalBtc->value()*floatFeeDec)+baseValues.currentPair.priceMin);
	profitBuyThanSellUnlocked=true;
	profitBuyThanSellCalc();
	profitSellThanBuyCalc();
	ui.buttonBuyThenSellApply->setEnabled(false);
}

void QtBitcoinTrader::setApiDown(bool on)
{
	if(currentExchange->supportsExchangeLag)
	{
		ui.lagValue->setVisible(!on);
		ui.lagMtgoxLabel->setVisible(!on);
		ui.apiDownLabel->setVisible(on);
	}
	else
	ui.exchangeLagBack->setVisible(on);
}

QString QtBitcoinTrader::clearData(QString data)
{
	while(data.count()&&(data.at(0)=='{'||data.at(0)=='['||data.at(0)=='\"'))data.remove(0,1);
	while(data.count()&&(data.at(data.length()-1)=='}'||data.at(data.length()-1)==']'||data.at(data.length()-1)=='\"'))data.remove(data.length()-1,1);
	return data;
}

void QtBitcoinTrader::on_accountFee_valueChanged(double val)
{
	floatFee=val/100.0;
	floatFeeDec=1.0-floatFee;       
	floatFeeInc=1.0+floatFee;
	if(currentExchange&&!currentExchange->supportsExchangeFee)
		iniSettings->setValue("Profile/CustomFee",ui.accountFee->value());
}

QByteArray QtBitcoinTrader::getMidData(QString a, QString b,QByteArray *data)
{
	QByteArray rez;
	if(b.isEmpty())b="\",";
	int startPos=data->indexOf(a,0);
	if(startPos>-1)
	{
	int endPos=data->indexOf(b,startPos+a.length());
	if(endPos>-1)rez=data->mid(startPos+a.length(),endPos-startPos-a.length());
	}
	return rez;
}

void QtBitcoinTrader::fixWindowMinimumSize()
{
	static QTime lastFixedTime;
	if(lastFixedTime.elapsed()<500)return;

	ui.depthAsksTable->setMinimumWidth(ui.depthAsksTable->columnWidth(1)+ui.depthAsksTable->columnWidth(2)+ui.depthAsksTable->columnWidth(3)+20);
	ui.depthAsksTable->horizontalScrollBar()->setValue(ui.depthAsksTable->horizontalScrollBar()->maximum());
	ui.depthBidsTable->setMinimumWidth(ui.depthBidsTable->columnWidth(0)+ui.depthBidsTable->columnWidth(1)+ui.depthBidsTable->columnWidth(2)+20);
	ui.depthBidsTable->horizontalScrollBar()->setValue(0);

	ui.marketGroupBox->setMinimumWidth(ui.marketGroupBox->minimumSizeHint().width());
	ui.buyThenSellGroupBox->setMinimumWidth(ui.buyThenSellGroupBox->minimumSizeHint().width());
	ui.sellThenBuyGroupBox->setMinimumWidth(ui.sellThenBuyGroupBox->minimumSizeHint().width());
	ui.groupBoxAccount->setMinimumWidth(ui.groupBoxAccount->minimumSizeHint().width());
	QSize minSizeHint=minimumSizeHint();
	if(isValidSize(&minSizeHint))setMinimumSize(minSizeHint);
	lastFixedTime.restart();
}

void QtBitcoinTrader::on_lagValue_valueChanged(double val)
{
	if(val>=1.0)
		ui.lagValue->setStyleSheet("QDoubleSpinBox {background: "+baseValues.appTheme.lightRed.name()+";}");
	else
		ui.lagValue->setStyleSheet("");
}

void QtBitcoinTrader::updateLogTable()
{
	emit getHistory(false);
}

void QtBitcoinTrader::balanceChanged(double)
{
	checkValidSellButtons();
	checkValidBuyButtons();
	emit getHistory(true);
}

void QtBitcoinTrader::ordersIsAvailable()
{
	if(ui.ordersTableFrame->isVisible())return;

	ui.noOpenedOrdersLabel->setVisible(false);
	ui.ordersTableFrame->setVisible(true);
}

void QtBitcoinTrader::ordersIsEmpty()
{
	if(ordersModel->rowCount())
	{
		if(debugLevel)logThread->writeLog("Order table cleared");
		ordersModel->clear();
		ui.ordersTotalBTC->setValue(0.0);
		ui.ordersTotalUSD->setValue(0.0);
		ui.ordersTableFrame->setVisible(false);
		ui.noOpenedOrdersLabel->setVisible(true);
	}
	calcOrdersTotalValues();
}

void QtBitcoinTrader::orderCanceled(QByteArray oid)
{
	if(debugLevel)logThread->writeLog("Removed order: "+oid);
	ordersModel->setOrderCanceled(oid);
}

QString QtBitcoinTrader::numFromDouble(const double &val, int maxDecimals)
{
	QString numberText=QString::number(val,'f',maxDecimals);
	int curPos=numberText.size()-1;
	while(curPos>0&&numberText.at(curPos)=='0')numberText.remove(curPos--,1);
	if(numberText.size()&&numberText.at(numberText.size()-1)=='.')numberText.append(QLatin1String("0"));
	if(curPos==-1)numberText.append(QLatin1String(".0"));
	return numberText;
}

void QtBitcoinTrader::ordersChanged(QList<OrderItem> *orders)
{
	currentlyAddingOrders=true;
	ordersModel->ordersChanged(orders);

	calcOrdersTotalValues();
	checkValidOrdersButtons();

	depthAsksModel->reloadVisibleItems();
	depthBidsModel->reloadVisibleItems();
	currentlyAddingOrders=false;
}

void QtBitcoinTrader::showErrorMessage(QString message)
{
	static QTime lastMessageTime;
	if(!showingMessage&&lastMessageTime.elapsed()>10000)
	{
		if(!showingMessage)
		{
			showingMessage=true;
			if(debugLevel)logThread->writeLog(baseValues.exchangeName.toAscii()+" Error: "+message.toAscii(),2);
			lastMessageTime.restart();
			if(message.startsWith("I:>"))
			{
				message.remove(0,3);
				identificationRequired(message);
			}
			else
			QMessageBox::warning(windowWidget,julyTr("AUTH_ERROR","%1 Error").arg(baseValues.exchangeName),message);
			showingMessage=false;
		}
	}
}

void QtBitcoinTrader::identificationRequired(QString message)
{
	if(!message.isEmpty())message.prepend("<br><br>");
		message.prepend(julyTr("TRUNAUTHORIZED","Identification required to access private API.<br>Please enter valid API key and Secret."));

	QMessageBox::warning(windowWidget,julyTr("AUTH_ERROR","%1 Error").arg(baseValues.exchangeName),message);
}

void QtBitcoinTrader::historyChanged(QList<HistoryItem>* historyItems)
{
	historyModel->historyChanged(historyItems);
	if(debugLevel)logThread->writeLog("Log Table Updated");
}

void QtBitcoinTrader::accLastSellChanged(QByteArray priceCurrency, double val)
{
	ui.ordersLastSellPrice->setValue(val);
	if(ui.usdLabelLastSell->toolTip()!=priceCurrency)
	{
		ui.usdLabelLastSell->setPixmap(QPixmap(":/Resources/CurrencySign/"+priceCurrency.toUpper()+".png"));
		ui.usdLabelLastSell->setToolTip(priceCurrency);
	}
}

void QtBitcoinTrader::accLastBuyChanged(QByteArray priceCurrency, double val)
{
	ui.ordersLastBuyPrice->setValue(val);
	if(ui.usdLabelLastBuy->toolTip()!=priceCurrency)
	{
		ui.usdLabelLastBuy->setPixmap(QPixmap(":/Resources/CurrencySign/"+priceCurrency.toUpper()+".png"));
		ui.usdLabelLastBuy->setToolTip(priceCurrency);
	}
}

void QtBitcoinTrader::translateUnicodeStr(QString *str)
{
	const QRegExp rx("(\\\\u[0-9a-fA-F]{4})");
	int pos=0;
	while((pos=rx.indexIn(*str,pos))!=-1)str->replace(pos++, 6, QChar(rx.cap(1).right(4).toUShort(0, 16)));
}

void QtBitcoinTrader::cancelOrder(QByteArray oid)
{
	emit cancelOrderByOid(oid);
}

void QtBitcoinTrader::on_ordersCancelBidsButton_clicked()
{
	QByteArray cancelSymbol;
	if(ui.ordersFilterCheckBox->isChecked())cancelSymbol=currPairsList.at(ui.filterOrdersCurrency->currentIndex()).currSymbol;
	ordersModel->ordersCancelBids(cancelSymbol);
}

void QtBitcoinTrader::on_ordersCancelAsksButton_clicked()
{
	QByteArray cancelSymbol;
	if(ui.ordersFilterCheckBox->isChecked())cancelSymbol=currPairsList.at(ui.filterOrdersCurrency->currentIndex()).currSymbol;
	ordersModel->ordersCancelAsks(cancelSymbol);
}

void QtBitcoinTrader::on_ordersCancelAllButton_clicked()
{
	QByteArray cancelSymbol;
	if(ui.ordersFilterCheckBox->isChecked())cancelSymbol=currPairsList.at(ui.filterOrdersCurrency->currentIndex()).currSymbol;
	ordersModel->ordersCancelAll(cancelSymbol);
}

void QtBitcoinTrader::cancelAllCurrentPairOrders()
{
	ordersModel->ordersCancelAll(baseValues.currentPair.currSymbol);
}

void QtBitcoinTrader::on_ordersCancelSelected_clicked()
{
	QModelIndexList selectedRows=ui.ordersTable->selectionModel()->selectedRows();
	if(selectedRows.count()==0)return;
	for(int n=0;n<selectedRows.count();n++)
	{
	QByteArray oid=selectedRows.at(n).data(Qt::UserRole).toByteArray();
	if(!oid.isEmpty())cancelOrder(oid);
	}
}

void QtBitcoinTrader::cancelOrderByXButton()
{
	QWidget* buttonCancel=dynamic_cast<QWidget*>(sender());
	if(!buttonCancel)return;
	for(int n=0;n<ordersModel->rowCount();n++)
	{
		QWidget* currentButtonCancel=ui.ordersTable->indexWidget(ordersSortModel->index(n,7));
		if(!currentButtonCancel)continue;
		if(buttonCancel!=currentButtonCancel)continue;
		cancelOrder(ordersSortModel->index(n,0).data(Qt::UserRole).toByteArray());
		break;
	}
}

void QtBitcoinTrader::on_buttonNight_clicked()
{
	baseValues.nightMode=!baseValues.nightMode;

	if(baseValues.nightMode)
	{
		baseValues.appTheme=baseValues.appThemeDark;
		qApp->setStyle(new QPlastiqueStyle);
	}
	else
	{
		baseValues.appTheme=baseValues.appThemeLight;
		qApp->setStyle(baseValues.osStyle);
	}

	qApp->setPalette(baseValues.appTheme.palette);
	qApp->setStyleSheet(baseValues.appTheme.styleSheet);

	ui.accountLoginLabel->setStyleSheet("color: "+baseValues.appTheme.black.name()+"; background: "+baseValues.appTheme.white.name());
	ui.noOpenedOrdersLabel->setStyleSheet("border: 1px solid gray; background: "+baseValues.appTheme.white.name()+"; color: "+baseValues.appTheme.gray.name());
	ui.rulesNoMessage->setStyleSheet("border: 1px solid gray; background: "+baseValues.appTheme.white.name()+"; color: "+baseValues.appTheme.gray.name());

	ui.buyBitcoinsButton->setStyleSheet("QPushButton {color: "+baseValues.appTheme.blue.name()+"} QPushButton::disabled {color: "+baseValues.appTheme.gray.name()+"}");
	ui.sellBitcoinsButton->setStyleSheet("QPushButton {color: "+baseValues.appTheme.red.name()+"} QPushButton::disabled {color: "+baseValues.appTheme.gray.name()+"}");

	on_profitLossSpinBox_valueChanged(ui.profitLossSpinBox->value());
	on_sellThanBuySpinBox_valueChanged(ui.sellThanBuySpinBox->value());
	on_buyTotalSpend_valueChanged(ui.buyTotalSpend->value());

	foreach(RuleWidget* currentGroup, ui.tabRules->findChildren<RuleWidget*>())currentGroup->updateStyleSheets();

	if(!baseValues.nightMode)
		ui.buttonNight->setIcon(QIcon("://Resources/Night.png"));
	else
		ui.buttonNight->setIcon(QIcon("://Resources/Day.png"));

	if(swapedDepth)
	{
		ui.asksLabel->setStyleSheet("color: "+baseValues.appTheme.blue.name());
		ui.bidsLabel->setStyleSheet("color: "+baseValues.appTheme.red.name());
	}
	else
	{
		ui.asksLabel->setStyleSheet("color: "+baseValues.appTheme.red.name());
		ui.bidsLabel->setStyleSheet("color: "+baseValues.appTheme.blue.name());
	}

	iniSettings->setValue("UI/NightMode",baseValues.nightMode);

	emit themeChanged();
}

void QtBitcoinTrader::on_calcButton_clicked()
{
	if(feeCalculatorSingleInstance&&feeCalculator)feeCalculator->activateWindow();
	else feeCalculator=new FeeCalculator;
}

void QtBitcoinTrader::checkValidSellButtons()
{
	ui.sellThenBuyGroupBox->setEnabled(ui.sellTotalBtc->value()>=baseValues.currentPair.tradeVolumeMin);
	ui.sellBitcoinsButton->setEnabled(ui.sellThenBuyGroupBox->isEnabled()&&/*ui.sellTotalBtc->value()<=getAvailableBTC()&&*/ui.sellTotalBtc->value()>0.0);

	profitBuyThanSellCalc();
	profitSellThanBuyCalc();
}

void QtBitcoinTrader::on_sellPricePerCoinAsMarketLastPrice_clicked()
{
	ui.sellPricePerCoin->setValue(ui.marketLast->value());
}

void QtBitcoinTrader::on_sellTotalBtcAllIn_clicked()
{
	ui.sellTotalBtc->setValue(getAvailableBTC());
}

void QtBitcoinTrader::on_sellTotalBtcHalfIn_clicked()
{
	ui.sellTotalBtc->setValue(getAvailableBTC()/2.0);
}

void QtBitcoinTrader::setDataPending(bool on)
{
	isDataPending=on;
}

void QtBitcoinTrader::setSoftLagValue(int mseconds)
{
	if(!isDataPending&&mseconds<baseValues.httpRequestTimeout)mseconds=0;

	static int lastSoftLag=-1;
	if(lastSoftLag==mseconds)return;

	ui.lastUpdate->setValue(mseconds/1000.0);

	static bool lastSoftLagValid=true;
	isValidSoftLag=mseconds<=baseValues.httpRequestTimeout+baseValues.httpRequestInterval+200;

	if(isValidSoftLag!=lastSoftLagValid)
	{
		lastSoftLagValid=isValidSoftLag;
		if(!isValidSoftLag)ui.lastUpdate->setStyleSheet("QDoubleSpinBox {background: "+baseValues.appTheme.lightRed.name()+";}");
		else ui.lastUpdate->setStyleSheet("");
		checkValidSellButtons();
		checkValidBuyButtons();
		ui.ordersControls->setEnabled(isValidSoftLag);
		ui.buyButtonBack->setEnabled(isValidSoftLag);
		ui.sellButtonBack->setEnabled(isValidSoftLag);
		QString toolTip;
		if(!isValidSoftLag)toolTip=julyTr("TOOLTIP_API_LAG_TO_HIGH","API Lag is to High");

		ui.ordersControls->setToolTip(toolTip);
		ui.buyButtonBack->setToolTip(toolTip);
		ui.sellButtonBack->setToolTip(toolTip);
	}
}

void QtBitcoinTrader::disableGroupId(int id)
{
	foreach(RuleWidget* currentGroup, ui.tabRules->findChildren<RuleWidget*>())
		if(currentGroup&&currentGroup->getRuleGroupId()==id)
		{
			currentGroup->ruleDisableAll();
			return;
		}
}

void QtBitcoinTrader::enableGroupId(int id)
{
	foreach(RuleWidget* currentGroup, ui.tabRules->findChildren<RuleWidget*>())
		if(currentGroup&&currentGroup->getRuleGroupId()==id)
		{
			currentGroup->ruleEnableAll();
			return;
		}
}

void QtBitcoinTrader::on_tradesVolume5m_valueChanged(double val)
{
	checkAndExecuteRule(14,val);
}

void QtBitcoinTrader::on_tradesBidsPrecent_valueChanged(double val)
{
	checkAndExecuteRule(15,val);
}

void QtBitcoinTrader::checkAllRules()
{
	if(!isValidSoftLag)return;

	checkAndExecuteRule(9,ui.accountUSD->value());
	checkAndExecuteRule(8,ui.accountBTC->value());
	checkAndExecuteRule(10,ui.ruleTotalToBuyValue->value());
	checkAndExecuteRule(12,ui.ruleTotalToBuyBSValue->value());
	checkAndExecuteRule(13,ui.ruleAmountToReceiveBSValue->value());
	checkAndExecuteRule(14,ui.tradesVolume5m->value());
	checkAndExecuteRule(15,ui.tradesBidsPrecent->value());
	checkAndExecuteRule(16,0.0);

	on_marketBid_valueChanged(ui.marketBid->value());
	on_marketAsk_valueChanged(ui.marketAsk->value());
	on_marketLow_valueChanged(ui.marketLow->value());
	on_marketHigh_valueChanged(ui.marketHigh->value());
	on_marketLast_valueChanged(ui.marketLast->value());
	on_ordersLastBuyPrice_valueChanged(ui.ordersLastBuyPrice->value());
	on_ordersLastSellPrice_valueChanged(ui.ordersLastSellPrice->value());
}

void QtBitcoinTrader::on_sellTotalBtc_valueChanged(double val)
{
	if(val==0.0)ui.sellTotalBtc->setStyleSheet("QDoubleSpinBox {background: "+baseValues.appTheme.lightRed.name()+";}");
	else ui.sellTotalBtc->setStyleSheet("");

	profitSellThanBuyCalc();
	if(sellLockBtcToSell)return;
	sellLockBtcToSell=true;

	sellLockAmountToReceive=true;
	ui.sellAmountToReceive->setValue(ui.sellPricePerCoin->value()*val*floatFeeDec);

	sellLockAmountToReceive=false;

	checkValidSellButtons();
	sellLockBtcToSell=false;
}

void QtBitcoinTrader::on_sellPricePerCoin_valueChanged(double)
{
	profitSellThanBuyCalc();
	if(!sellLockPricePerCoin)
	{
	sellLockPricePerCoin=true;
	on_sellTotalBtc_valueChanged(ui.sellTotalBtc->value());
	sellLockPricePerCoin=false;
	}
	ui.sellNextMaxBuyPrice->setValue(ui.sellPricePerCoin->value()*floatFeeDec*floatFeeDec-baseValues.currentPair.priceMin);
	ui.sellNextMaxBuyStep->setValue(ui.sellPricePerCoin->value()-ui.sellNextMaxBuyPrice->value());
	checkValidSellButtons();
	ui.buttonSellThenBuyApply->setEnabled(true);
	profitBuyThanSellCalc();
	profitSellThanBuyCalc();
}

void QtBitcoinTrader::on_sellAmountToReceive_valueChanged(double val)
{
	profitBuyThanSellCalc();
	profitSellThanBuyCalc();
	if(sellLockAmountToReceive)return;
	sellLockAmountToReceive=true;

	sellLockBtcToSell=true;
	sellLockPricePerCoin=true;

	ui.sellTotalBtc->setValue(val/ui.sellPricePerCoin->value()/floatFeeDec);

	sellLockPricePerCoin=false;
	sellLockBtcToSell=false;

	sellLockAmountToReceive=false;
	checkValidSellButtons();
}

void QtBitcoinTrader::sellBitcoinButton()
{
	checkValidSellButtons();
	if(ui.sellBitcoinsButton->isEnabled()==false)return;
	double sellTotalBtc=ui.sellTotalBtc->value();
	double sellPricePerCoin=ui.sellPricePerCoin->value();
	if(ui.confirmOpenOrder->isChecked())
	{
	QMessageBox msgBox(windowWidget);
	msgBox.setIcon(QMessageBox::Question);
	msgBox.setWindowTitle(julyTr("MESSAGE_CONFIRM_SELL_TRANSACTION","Please confirm transaction"));
	msgBox.setText(julyTr("MESSAGE_CONFIRM_SELL_TRANSACTION_TEXT","Are you sure to sell %1 at %2 ?<br><br>Note: If total orders amount of your Bitcoins exceeds your balance, %3 will remove this order immediately.").arg(baseValues.currentPair.currASign+" "+numFromDouble(sellTotalBtc,baseValues.currentPair.currADecimals)).arg(baseValues.currentPair.currBSign+" "+numFromDouble(sellPricePerCoin,baseValues.currentPair.currBDecimals)).arg(baseValues.exchangeName));
	msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	msgBox.setDefaultButton(QMessageBox::Yes);
	msgBox.setButtonText(QMessageBox::Yes,julyTr("YES","Yes"));
	msgBox.setButtonText(QMessageBox::No,julyTr("NO","No"));
	if(msgBox.exec()!=QMessageBox::Yes)return;
	}

	apiSellSend(sellTotalBtc,sellPricePerCoin);
}

void QtBitcoinTrader::on_buyTotalSpend_valueChanged(double val)
{
	if(val==0.0)ui.buyTotalSpend->setStyleSheet("QDoubleSpinBox {background: "+baseValues.appTheme.lightRed.name()+";}");
	else ui.buyTotalSpend->setStyleSheet("");

	buyLockTotalBtc=true;
	if(!buyLockTotalBtcSelf)ui.buyTotalBtc->setValue(val/ui.buyPricePerCoin->value());
	buyLockTotalBtc=false;

	double valueForResult=getFeeForUSDDec(val)/ui.buyPricePerCoin->value();
	valueForResult=getValidDoubleForPercision(valueForResult,baseValues.currentPair.currADecimals,false);
	ui.buyTotalBtcResult->setValue(valueForResult);

	if(buyLockTotalSpend)return;
	buyLockTotalSpend=true;


	buyLockTotalSpend=false;
	checkValidBuyButtons();
}


void QtBitcoinTrader::on_buyTotalBtc_valueChanged(double)
{
	if(buyLockTotalBtc)
	{
		checkValidBuyButtons();
		return;
	}
	buyLockTotalBtc=true;
	buyLockTotalBtcSelf=true;

	buyLockTotalSpend=true;

	ui.buyTotalSpend->setValue(ui.buyTotalBtc->value()*ui.buyPricePerCoin->value());
	buyLockTotalSpend=false;
	buyLockTotalBtcSelf=false;
	buyLockTotalBtc=false;
	checkValidBuyButtons();
}

void QtBitcoinTrader::on_buyPricePerCoin_valueChanged(double)
{
	if(!buyLockPricePerCoin)
	{
	buyLockPricePerCoin=true;
	on_buyTotalSpend_valueChanged(ui.buyTotalSpend->value());
	buyLockPricePerCoin=false;
	}
	ui.buyNextInSellPrice->setValue(ui.buyPricePerCoin->value()*floatFeeInc*floatFeeInc+baseValues.currentPair.priceMin);
	ui.buyNextMinBuyStep->setValue(ui.buyNextInSellPrice->value()-ui.buyPricePerCoin->value());
	checkValidBuyButtons();
	ui.buttonBuyThenSellApply->setEnabled(true);
}

void QtBitcoinTrader::checkValidBuyButtons()
{
	ui.buyThenSellGroupBox->setEnabled(ui.buyTotalBtc->value()>=baseValues.currentPair.tradeVolumeMin);
	ui.buyBitcoinsButton->setEnabled(ui.buyThenSellGroupBox->isEnabled()&&/*ui.buyTotalSpend->value()<=getAvailableUSD()&&*/ui.buyTotalSpend->value()>0.0);
	profitBuyThanSellCalc();
	profitSellThanBuyCalc();
}

void QtBitcoinTrader::checkValidOrdersButtons()
{
	ui.ordersCancelAllButton->setEnabled(ordersModel->rowCount());
	ui.ordersCancelSelected->setEnabled(ui.ordersTable->selectionModel()->selectedRows().count());
}

void QtBitcoinTrader::on_buyTotalBtcAllIn_clicked()
{
	ui.buyTotalBtc->setValue(getAvailableUSDtoBTC(ui.buyPricePerCoin->value()));
}

void QtBitcoinTrader::on_buyTotalBtcHalfIn_clicked()
{
	ui.buyTotalBtc->setValue(getAvailableUSDtoBTC(ui.buyPricePerCoin->value())/2.0);
}

void QtBitcoinTrader::on_sellPriceAsMarketBid_clicked()
{
	ui.sellPricePerCoin->setValue(ui.marketBid->value());
}

void QtBitcoinTrader::on_buyPriceAsMarketBid_clicked()
{
	ui.buyPricePerCoin->setValue(ui.marketBid->value());
}

void QtBitcoinTrader::on_sellPriceAsMarketAsk_clicked()
{
	ui.sellPricePerCoin->setValue(ui.marketAsk->value());
}

void QtBitcoinTrader::on_buyPriceAsMarketAsk_clicked()
{
	ui.buyPricePerCoin->setValue(ui.marketAsk->value());
}

void QtBitcoinTrader::on_buyPriceAsMarketLastPrice_clicked()
{
	ui.buyPricePerCoin->setValue(ui.marketLast->value());
}

bool QtBitcoinTrader::windowCloseRequested()
{
	if(ui.minimizeOnCloseCheckBox->isChecked())
	{
		buttonMinimizeToTray();
		return true;
	}
	bool haveWorkingRules=false;

	foreach(RuleWidget* currentGroup, ui.tabRules->findChildren<RuleWidget*>())
		if(currentGroup)
		{
			if(!haveWorkingRules&&currentGroup->haveWorkingRules())haveWorkingRules=true;
			currentGroup->saveRulesData();
		}

		if(haveWorkingRules)
		{
			QMessageBox msgBox(windowWidget);
			msgBox.setIcon(QMessageBox::Question);
			msgBox.setWindowTitle("Qt Bitcoin Trader");
			msgBox.setText(julyTr("CONFIRM_EXIT","Are you sure to close Application?<br>Active rules works only while application is running."));
			msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
			msgBox.setDefaultButton(QMessageBox::Yes);
			msgBox.setButtonText(QMessageBox::Yes,julyTr("YES","Yes"));
			msgBox.setButtonText(QMessageBox::No,julyTr("NO","No"));
			if(msgBox.exec()!=QMessageBox::Yes)return true;
		}

		exitApp();
		return false;
}

void WindowScrollBars::closeEvent(QCloseEvent *event)
{
	mainWindow.closeEvent(event);
}

void WindowScrollBars::keyPressEvent(QKeyEvent *event)
{
	mainWindow.keyPressEvent(event);
}

void QtBitcoinTrader::closeEvent(QCloseEvent *event)
{
	if(windowCloseRequested())event->ignore();
	else event->accept();
}

void QtBitcoinTrader::saveWindowState(QWidget *par, QString name)
{
	bool windowMaximized=par->windowState()==Qt::WindowMaximized;
	iniSettings->setValue("UI/"+name+"Maximized",windowMaximized);
	if(windowMaximized)iniSettings->setValue("UI/"+name+"Geometry",rectInRect(par->geometry(),par->minimumSizeHint()));
	else	   iniSettings->setValue("UI/"+name+"Geometry",QRect(par->x(),par->y(),par->width(),par->height()));
}

void QtBitcoinTrader::loadWindowState(QWidget *par, QString name)
{
	QRect savedGeometry=iniSettings->value("UI/"+name+"Geometry",par->geometry()).toRect();
	if(isValidGeometry(&savedGeometry,0))
	{
		par->resize(savedGeometry.size());
		par->move(savedGeometry.topLeft());
	}
	if(iniSettings->value("UI/"+name+"Maximized",false).toBool())par->setWindowState(Qt::WindowMaximized);
}

void QtBitcoinTrader::saveDetachedWindowsSettings(bool force)
{
	if(!force)
	{
	iniSettings->setValue("UI/DetachedLog",isDetachedLog);
	iniSettings->setValue("UI/DetachedRules",isDetachedRules);
	iniSettings->setValue("UI/DetachedTrades",isDetachedTrades);
	iniSettings->setValue("UI/DetachedDepth",isDetachedDepth);
	iniSettings->setValue("UI/DetachedCharts",isDetachedCharts);
	}
	if(isDetachedLog)saveWindowState(ui.tabOrdersLog,"DetachedLog");
	if(isDetachedRules)saveWindowState(ui.tabRules,"DetachedRules");
	if(isDetachedTrades)saveWindowState(ui.tabLastTrades,"DetachedTrades");
	if(isDetachedDepth)saveWindowState(ui.tabDepth,"DetachedDepth");
	if(isDetachedCharts)saveWindowState(ui.tabCharts,"DetachedCharts");

	iniSettings->setValue("UI/TabLogOrdersOnTop",ui.tabOrdersLogOnTop->isChecked());
	iniSettings->setValue("UI/TabRulesOnTop",ui.tabRulesOnTop->isChecked());
	iniSettings->setValue("UI/TabTradesOnTop",ui.tabTradesOnTop->isChecked());
	iniSettings->setValue("UI/TabDepthOnTop",ui.tabDepthOnTop->isChecked());
	iniSettings->setValue("UI/TabChartsOnTop",ui.tabChartsOnTop->isChecked());

	iniSettings->sync();
}

QRect QtBitcoinTrader::rectInRect(QRect aRect, QSize bSize)
{
	return QRect(aRect.x()+(aRect.width()-bSize.width())/2.0,aRect.y()+(aRect.height()-bSize.height())/2.0,bSize.width(),qMax(bSize.height(),300));
}

void QtBitcoinTrader::buyBitcoinsButton()
{
	checkValidBuyButtons();
	if(ui.buyBitcoinsButton->isEnabled()==false)return;

	double btcToBuy=0.0;
	double priceToBuy=ui.buyPricePerCoin->value();
	if(currentExchange->buySellAmountExcludedFee)btcToBuy=ui.buyTotalBtcResult->value();
	else btcToBuy=ui.buyTotalBtc->value();

	//double amountWithoutFee=getAvailableUSD()/priceToBuy;
	//amountWithoutFee=getValidDoubleForPercision(amountWithoutFee,baseValues.currentPair.currADecimals,false);

	if(ui.confirmOpenOrder->isChecked())
	{
	QMessageBox msgBox(windowWidget);
	msgBox.setIcon(QMessageBox::Question);
	msgBox.setWindowTitle(julyTr("MESSAGE_CONFIRM_BUY_TRANSACTION","Please confirm new order"));
	msgBox.setText(julyTr("MESSAGE_CONFIRM_BUY_TRANSACTION_TEXT","Are you sure to buy %1 at %2 ?<br><br>Note: If total orders amount of your funds exceeds your balance, %3 will remove this order immediately.").arg(baseValues.currentPair.currASign+" "+numFromDouble(ui.buyTotalBtc->value(),baseValues.currentPair.currADecimals)).arg(baseValues.currentPair.currBSign+" "+numFromDouble(ui.buyPricePerCoin->value(),baseValues.currentPair.currBDecimals)).arg(baseValues.exchangeName));
	msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	msgBox.setDefaultButton(QMessageBox::Yes);
	msgBox.setButtonText(QMessageBox::Yes,julyTr("YES","Yes"));
	msgBox.setButtonText(QMessageBox::No,julyTr("NO","No"));
	if(msgBox.exec()!=QMessageBox::Yes)return;
	}
	
	apiBuySend(btcToBuy,priceToBuy);
}

void QtBitcoinTrader::playWav(QString wav, bool noBlink)
{
	QSound::play(wav);
	if(!noBlink)blinkWindow();
}

void QtBitcoinTrader::beep(bool noBlink)
{
#ifdef USE_QTMULTIMEDIA
	static AudioPlayer *player=0;
	if(player==0)player=new AudioPlayer(this);
	if(player->invalidDevice)
	{
		QApplication::beep();
		delete player;
		player=0;
	}
	else player->beep();
#endif
	if(!noBlink)blinkWindow();
}

void QtBitcoinTrader::blinkWindow()
{
#ifdef  Q_OS_WIN
	if(!isActiveWindow())
	{
		FLASHWINFO flashInfo;
		flashInfo.cbSize=sizeof(FLASHWINFO);
		flashInfo.hwnd=windowWidget->winId();
		flashInfo.dwFlags=FLASHW_ALL;
		flashInfo.uCount=20;
		flashInfo.dwTimeout=400;
		::FlashWindowEx(&flashInfo);
	}
#endif//ToDo: make this feature works on Mac OS X
}

void QtBitcoinTrader::ruleTotalToBuyValueChanged()
{
	if(ui.marketLast->value()==0.0)return;
	double newValue=ui.accountUSD->value()/ui.marketLast->value()*floatFeeDec;
	if(newValue!=ui.ruleTotalToBuyValue->value())
	{
		ui.ruleTotalToBuyValue->setValue(newValue);
		checkAndExecuteRule(10,ui.ruleTotalToBuyValue->value());
	}
}

void QtBitcoinTrader::ruleAmountToReceiveValueChanged()
{
	if(ui.marketLast->value()==0.0)return;
	double newValue=ui.accountBTC->value()*ui.marketLast->value()*floatFeeDec;
	if(newValue!=ui.ruleAmountToReceiveValue->value())
	{
		ui.ruleAmountToReceiveValue->setValue(newValue);
		checkAndExecuteRule(11,ui.ruleAmountToReceiveValue->value());
	}
}

void QtBitcoinTrader::ruleTotalToBuyBSValueChanged()
{
	if(ui.marketBid->value()==0.0)return;
	double newValue=ui.accountUSD->value()/ui.marketBid->value()*floatFeeDec;
	if(newValue!=ui.ruleTotalToBuyValue->value())
	{
		ui.ruleTotalToBuyBSValue->setValue(newValue);
		checkAndExecuteRule(12,ui.ruleTotalToBuyBSValue->value());
	}
}

void QtBitcoinTrader::ruleAmountToReceiveBSValueChanged()
{
	if(ui.marketAsk->value()==0.0)return;
	double newValue=ui.accountBTC->value()*ui.marketAsk->value()*floatFeeDec;
	if(newValue!=ui.ruleAmountToReceiveBSValue->value())
	{
		ui.ruleAmountToReceiveBSValue->setValue(newValue);
		checkAndExecuteRule(13,ui.ruleAmountToReceiveBSValue->value());
	}
}

void QtBitcoinTrader::on_accountUSD_valueChanged(double val)
{
	ruleTotalToBuyValueChanged();
	ruleTotalToBuyBSValueChanged();
	checkAndExecuteRule(9,val);
}

void QtBitcoinTrader::on_accountBTC_valueChanged(double val)
{
	ruleAmountToReceiveValueChanged();
	ruleAmountToReceiveBSValueChanged();
	checkAndExecuteRule(8,val);
}

void QtBitcoinTrader::on_marketLow_valueChanged(double val)
{
	checkAndExecuteRule(5,val);
}

void QtBitcoinTrader::on_marketHigh_valueChanged(double val)
{
	checkAndExecuteRule(4,val);
}

void QtBitcoinTrader::on_marketBid_valueChanged(double val)
{
	checkAndExecuteRule(2,val);
	ruleTotalToBuyBSValueChanged();
	meridianPrice=(val+ui.marketAsk->value())/2;
}

void QtBitcoinTrader::on_marketAsk_valueChanged(double val)
{
	checkAndExecuteRule(3,val);
	ruleAmountToReceiveBSValueChanged();
	meridianPrice=(val+ui.marketBid->value())/2;
}

void QtBitcoinTrader::on_marketLast_valueChanged(double val)
{
	ruleTotalToBuyValueChanged();
	ruleAmountToReceiveValueChanged();
	checkAndExecuteRule(1,val);
	if(val>0.0)
	{
		static double lastValue=val;
		static int priceDirection=0;
		int lastPriceDirection=priceDirection;
		if(lastValue<val)priceDirection=1;else
		if(lastValue>val)priceDirection=-1;else
		priceDirection=lastPriceDirection;
		lastValue=val;

		static QString directionChar("-");
		switch(priceDirection)
		{
		case -1: directionChar=downArrowNoUtfStr; break;
		case 1: directionChar=upArrowNoUtfStr; break;
		default: break;
		}
		static QString titleText;
		titleText=baseValues.currentPair.currBSign+" "+numFromDouble(val)+" "+directionChar+" "+windowTitleP;
		if(windowWidget->isVisible())windowWidget->setWindowTitle(titleText);
		if(trayIcon&&trayIcon->isVisible())trayIcon->setToolTip(titleText);
	}
}

void QtBitcoinTrader::on_ordersLastBuyPrice_valueChanged(double val){checkAndExecuteRule(6,val);}
void QtBitcoinTrader::on_ordersLastSellPrice_valueChanged(double val){checkAndExecuteRule(7,val);}

void QtBitcoinTrader::checkAndExecuteRule(int ruleType, double price)
{
	foreach(RuleWidget* currentGroup, ui.tabRules->findChildren<RuleWidget*>())
		if(currentGroup)currentGroup->checkAndExecuteRule(ruleType,price);
}

void QtBitcoinTrader::checkIsTabWidgetVisible()
{
	bool isTabWidgetVisible=ui.tabWidget->count();
	static bool lastTabWidgetVisible=isTabWidgetVisible;
	if(isTabWidgetVisible!=lastTabWidgetVisible)
	{
	ui.tabWidget->setVisible(isTabWidgetVisible);
	ui.centerLayout->setHorizontalSpacing(isTabWidgetVisible?6:0);
	ui.groupOrders->setMaximumWidth(isTabWidgetVisible?600:16777215);
	lastTabWidgetVisible=isTabWidgetVisible;
	}
	fixWindowMinimumSize();
}

void QtBitcoinTrader::detachLog()
{
	ui.tabOrdersLog->setParent(0);
	ui.tabOrdersLog->move(mapToGlobal(ui.tabWidget->geometry().topLeft()));
	ui.detachOrdersLog->setVisible(false);
	ui.tabOrdersLogOnTop->setVisible(true);
	loadWindowState(ui.tabOrdersLog,"DetachedLog");
	ui.tabOrdersLogOnTop->setChecked(iniSettings->value("UI/TabLogOrdersOnTop",false).toBool());
	isDetachedLog=true;
	on_tabOrdersLogOnTop_toggled(ui.tabOrdersLogOnTop->isChecked());
	checkIsTabWidgetVisible();
}

void QtBitcoinTrader::detachRules()
{
	ui.tabRules->setParent(0);
	ui.tabRules->move(mapToGlobal(ui.tabWidget->geometry().topLeft()));
	ui.detachRules->setVisible(false);
	ui.tabRulesOnTop->setVisible(true);
	loadWindowState(ui.tabRules,"DetachedRules");
	ui.tabRulesOnTop->setChecked(iniSettings->value("UI/TabRulesOnTop",false).toBool());
	isDetachedRules=true;
	on_tabRulesOnTop_toggled(ui.tabRulesOnTop->isChecked());
	checkIsTabWidgetVisible();
}

void QtBitcoinTrader::detachTrades()
{
	ui.tabLastTrades->setParent(0);
	ui.tabLastTrades->move(mapToGlobal(ui.tabWidget->geometry().topLeft()));
	ui.detachTrades->setVisible(false);
	ui.tabTradesOnTop->setVisible(true);
	loadWindowState(ui.tabLastTrades,"DetachedTrades");
	ui.tabTradesOnTop->setChecked(iniSettings->value("UI/TabTradesOnTop",false).toBool());
	isDetachedTrades=true;
	on_tabTradesOnTop_toggled(ui.tabTradesOnTop->isChecked());
	checkIsTabWidgetVisible();
}

void QtBitcoinTrader::detachDepth()
{
	ui.tabDepth->setParent(0);
	ui.tabDepth->move(mapToGlobal(ui.tabWidget->geometry().topLeft()));
	ui.detachDepth->setVisible(false);
	ui.tabDepthOnTop->setVisible(true);
	loadWindowState(ui.tabDepth,"DetachedDepth");
	ui.tabDepthOnTop->setChecked(iniSettings->value("UI/TabDepthOnTop",false).toBool());
	isDetachedDepth=true;
	on_tabDepthOnTop_toggled(ui.tabDepthOnTop->isChecked());
	checkIsTabWidgetVisible();
	depthAsksLastScrollValue=-1;
	depthBidsLastScrollValue=-1;
}

void QtBitcoinTrader::detachCharts()
{
	ui.tabCharts->setParent(0);
	ui.tabCharts->move(mapToGlobal(ui.tabWidget->geometry().topLeft()));
	ui.detachCharts->setVisible(false);
	ui.tabChartsOnTop->setVisible(true);
	loadWindowState(ui.tabCharts,"DetachedCharts");
	ui.tabChartsOnTop->setChecked(iniSettings->value("UI/TabChartsOnTop",false).toBool());
	isDetachedCharts=true;
	on_tabChartsOnTop_toggled(ui.tabChartsOnTop->isChecked());
	checkIsTabWidgetVisible();
}

void QtBitcoinTrader::attachLog()
{
	saveDetachedWindowsSettings(true);
	ui.tabOrdersLogOnTop->setVisible(false);
	ui.detachOrdersLog->setVisible(true);
	ui.tabWidget->insertTab(0,ui.tabOrdersLog,ui.tabOrdersLog->accessibleName());
	checkIsTabWidgetVisible();
	ui.tabWidget->setCurrentWidget(ui.tabOrdersLog);
	isDetachedLog=false;
}

void QtBitcoinTrader::attachRules()
{
	saveDetachedWindowsSettings(true);
	ui.tabRulesOnTop->setVisible(false);
	ui.detachRules->setVisible(true);
	ui.tabWidget->insertTab(isDetachedLog?0:1,ui.tabRules,ui.tabRules->accessibleName());
	checkIsTabWidgetVisible();
	ui.tabWidget->setCurrentWidget(ui.tabRules);
	isDetachedRules=false;
}

void QtBitcoinTrader::attachDepth()
{
	saveDetachedWindowsSettings(true);
	ui.tabDepthOnTop->setVisible(false);
	ui.detachDepth->setVisible(true);

	int newTabPos=2;
	if(isDetachedLog&&isDetachedRules)newTabPos=0;
	else if(isDetachedLog||isDetachedRules)newTabPos=1;

	ui.tabWidget->insertTab(newTabPos,ui.tabDepth,ui.tabDepth->accessibleName());
	checkIsTabWidgetVisible();
	ui.tabWidget->setCurrentWidget(ui.tabDepth);
	isDetachedDepth=false;
	depthAsksLastScrollValue=-1;
	depthBidsLastScrollValue=-1;
}

void QtBitcoinTrader::attachTrades()
{
	saveDetachedWindowsSettings(true);
	ui.tabTradesOnTop->setVisible(false);
	ui.detachTrades->setVisible(true);

	int newTabPos=3;
	if(isDetachedLog&&isDetachedRules&&isDetachedDepth)newTabPos=0;
	else if(isDetachedLog&&isDetachedRules||isDetachedRules&&isDetachedDepth||isDetachedLog&&isDetachedDepth)newTabPos=1;
	else if(isDetachedLog||isDetachedRules||isDetachedDepth)newTabPos=2;

	ui.tabWidget->insertTab(newTabPos,ui.tabLastTrades,ui.tabLastTrades->accessibleName());
	checkIsTabWidgetVisible();
	ui.tabWidget->setCurrentWidget(ui.tabLastTrades);
	isDetachedTrades=false;
}

void QtBitcoinTrader::attachCharts()
{
	saveDetachedWindowsSettings(true);
	ui.tabChartsOnTop->setVisible(false);
	ui.detachCharts->setVisible(true);
	int newTabPos=4;
	if(isDetachedLog&&isDetachedRules&&isDetachedTrades&&isDetachedDepth)newTabPos=0;
	else if(isDetachedLog&&isDetachedRules&&isDetachedDepth||
			isDetachedLog&&isDetachedRules&&isDetachedTrades||
			isDetachedLog&&isDetachedDepth&&isDetachedTrades||
			isDetachedRules&&isDetachedDepth&&isDetachedTrades)newTabPos=1;
	else if(isDetachedLog&&isDetachedRules||
		isDetachedLog&&isDetachedDepth||
		isDetachedLog&&isDetachedTrades||
		isDetachedRules&&isDetachedDepth||
		isDetachedRules&&isDetachedTrades||
		isDetachedDepth&&isDetachedTrades)newTabPos=2;
	else if(isDetachedLog||isDetachedRules||isDetachedDepth||isDetachedTrades)newTabPos=3;
	ui.tabWidget->insertTab(newTabPos,ui.tabCharts,ui.tabCharts->accessibleName());
	checkIsTabWidgetVisible();
	ui.tabWidget->setCurrentWidget(ui.tabCharts);
	isDetachedCharts=false;
}

void QtBitcoinTrader::historyDoubleClicked(QModelIndex index)
{
	repeatOrderFromValues(0,historyModel->getRowPrice(index.row()),historyModel->getRowVolume(index.row()),false);
}

void QtBitcoinTrader::repeatOrderFromValues(int type,double itemPrice, double itemVolume, bool availableOnly)
{
	if(itemPrice==0.0)return;
	if(QApplication::keyboardModifiers()!=Qt::NoModifier)availableOnly=!availableOnly;
	if(type==1||type==0)
	{
		ui.buyPricePerCoin->setValue(itemPrice);
		if(availableOnly)itemVolume=qMin(itemVolume,getAvailableUSD()/itemPrice);
		ui.buyTotalBtc->setValue(itemVolume);
	}
	if(type==-1||type==0)
	{
		ui.sellPricePerCoin->setValue(itemPrice);
		if(availableOnly)itemVolume=qMin(getAvailableBTC(),itemVolume);
		ui.sellTotalBtc->setValue(itemVolume);
	}
}

void QtBitcoinTrader::repeatOrderFromTrades(int type, int row)
{
	repeatOrderFromValues(type,tradesModel->getRowPrice(row),tradesModel->getRowVolume(row));
}

void QtBitcoinTrader::tradesDoubleClicked(QModelIndex index)
{
	repeatOrderFromTrades(0,index.row());
}

void QtBitcoinTrader::depthSelectOrder(QModelIndex index, bool isSell, int type)
{
	double itemPrice=0.0;
	double itemVolume=0.0;
	int row=index.row();
	int col=index.column();

	if(swapedDepth)isSell=!isSell;

	if(isSell)
	{
		if(!swapedDepth)col=depthAsksModel->columnCount()-col-1;
		if(row<0||depthAsksModel->rowCount()<=row)return;
		itemPrice=depthAsksModel->rowPrice(row);
		if(col==0||col==1)itemVolume=depthAsksModel->rowVolume(row);
		else itemVolume=depthAsksModel->rowSize(row);
	}
	else
	{
		if(swapedDepth)col=depthBidsModel->columnCount()-col-1;
		if(row<0||depthBidsModel->rowCount()<=row)return;
		itemPrice=depthBidsModel->rowPrice(row);
		if(col==0||col==1)itemVolume=depthBidsModel->rowVolume(row);
		else itemVolume=depthBidsModel->rowSize(row);
	}

	repeatOrderFromValues(type,itemPrice,itemVolume*floatFeeDec);
}

void QtBitcoinTrader::depthSelectSellOrder(QModelIndex index)
{
	depthSelectOrder(index,true);
}

void QtBitcoinTrader::depthSelectBuyOrder(QModelIndex index)
{
	depthSelectOrder(index,false);
}

void QtBitcoinTrader::aboutTranslationButton()
{
	(new TranslationAbout(windowWidget))->showWindow();
}

void QtBitcoinTrader::languageChanged()
{
	if(!constructorFinished)return;
	julyTranslator.translateUi(this);
	baseValues.dateTimeFormat=julyTr("DATETIME_FORMAT",baseValues.dateTimeFormat);
	baseValues.timeFormat=julyTr("TIME_FORMAT",baseValues.timeFormat);
	QStringList ordersLabels;
	ordersLabels<<julyTr("ORDERS_COUNTER","#")<<julyTr("ORDERS_DATE","Date")<<julyTr("ORDERS_TYPE","Type")<<julyTr("ORDERS_STATUS","Status")<<julyTr("ORDERS_AMOUNT","Amount")<<julyTr("ORDERS_PRICE","Price")<<julyTr("ORDERS_TOTAL","Total");
	ordersModel->setHorizontalHeaderLabels(ordersLabels);

	QStringList tradesLabels;
	tradesLabels<<""<<julyTr("ORDERS_DATE","Date")<<julyTr("ORDERS_AMOUNT","Amount")<<julyTr("ORDERS_TYPE","Type")<<julyTr("ORDERS_PRICE","Price")<<julyTr("ORDERS_TOTAL","Total")<<"";
	historyModel->setHorizontalHeaderLabels(tradesLabels);
	tradesLabels.insert(4,upArrowStr+downArrowStr);
	tradesModel->setHorizontalHeaderLabels(tradesLabels);

	QStringList depthHeaderLabels;depthHeaderLabels<<julyTr("ORDERS_PRICE","Price")<<julyTr("ORDERS_AMOUNT","Amount")<<upArrowStr+downArrowStr<<julyTr("ORDERS_TOTAL","Total")<<"";
	depthBidsModel->setHorizontalHeaderLabels(depthHeaderLabels);
	depthAsksModel->setHorizontalHeaderLabels(depthHeaderLabels);

	ui.tabOrdersLog->setAccessibleName(julyTr("TAB_ORDERS_LOG","Orders Log"));
	ui.tabOrdersLog->setWindowTitle(ui.tabOrdersLog->accessibleName()+" ["+profileName+"]");
	if(isDetachedLog){julyTranslator.translateUi(ui.tabOrdersLog);fixAllChildButtonsAndLabels(ui.tabOrdersLog);}

	ui.tabRules->setAccessibleName(julyTr("TAB_RULES_FOR_ORDERS","Rules for creating Orders"));
	ui.tabRules->setWindowTitle(ui.tabRules->accessibleName()+" ["+profileName+"]");
	if(isDetachedRules){julyTranslator.translateUi(ui.tabRules);fixAllChildButtonsAndLabels(ui.tabRules);}

	ui.tabLastTrades->setAccessibleName(julyTr("TAB_LAST_TRADES","Last Trades"));
	ui.tabLastTrades->setWindowTitle(ui.tabLastTrades->accessibleName()+" ["+profileName+"]");
	if(isDetachedTrades){julyTranslator.translateUi(ui.tabLastTrades);fixAllChildButtonsAndLabels(ui.tabLastTrades);}

	ui.tabDepth->setAccessibleName(julyTr("TAB_DEPTH","Depth"));
	ui.tabDepth->setWindowTitle(ui.tabDepth->accessibleName()+" ["+profileName+"]");
	if(isDetachedDepth){julyTranslator.translateUi(ui.tabDepth);fixAllChildButtonsAndLabels(ui.tabDepth);}

	ui.tabCharts->setAccessibleName(julyTr("TAB_CHARTS","Charts"));
	ui.tabCharts->setWindowTitle(ui.tabCharts->accessibleName()+" ["+profileName+"]");
	if(isDetachedCharts){julyTranslator.translateUi(ui.tabCharts);fixAllChildButtonsAndLabels(ui.tabCharts);}

	ui.groupBoxAccount->setTitle(julyTr("ACCOUNT_GROUPBOX","%1 Account").arg(baseValues.exchangeName));

	for(int n=0;n<ui.tabWidget->count();n++)
		ui.tabWidget->setTabText(n,ui.tabWidget->widget(n)->accessibleName());

	QString curCurrencyName=baseValues.currencyMap.value(baseValues.currentPair.currAStr,CurencyInfo("BITCOINS")).name;
	ui.buyGroupbox->setTitle(julyTr("GROUPBOX_BUY","Buy %1").arg(curCurrencyName));
	ui.sellGroupBox->setTitle(julyTr("GROUPBOX_SELL","Sell %1").arg(curCurrencyName));

	foreach(QToolButton* toolButton, findChildren<QToolButton*>())
		if(toolButton->accessibleDescription()=="TOGGLE_SOUND")
			toolButton->setToolTip(julyTr("TOGGLE_SOUND","Toggle sound notification on value change"));

	ui.comboBoxGroupByPrice->setItemText(0,julyTr("DONT_GROUP","None"));
	ui.comboBoxGroupByPrice->setMinimumWidth(qMax(textFontWidth(ui.comboBoxGroupByPrice->itemText(0))+(int)(ui.comboBoxGroupByPrice->height()*1.1),textFontWidth("50.000")));

	copyTableValuesMenu.actions().at(0)->setText(julyTr("COPY_ROW","Copy selected Rows"));

	copyTableValuesMenu.actions().at(2)->setText(julyTr("COPY_DATE","Copy Date"));
	copyTableValuesMenu.actions().at(3)->setText(julyTr("COPY_AMOUNT","Copy Amount"));
	copyTableValuesMenu.actions().at(4)->setText(julyTr("COPY_PRICE","Copy Price"));
	copyTableValuesMenu.actions().at(5)->setText(julyTr("COPY_TOTAL","Copy Total"));

	copyTableValuesMenu.actions().at(7)->setText(julyTr("REPEAT_BUYSELL_ORDER","Repeat Buy and Sell order"));
	copyTableValuesMenu.actions().at(8)->setText(julyTr("REPEAT_BUY_ORDER","Repeat Buy order"));
	copyTableValuesMenu.actions().at(9)->setText(julyTr("REPEAT_SELL_ORDER","Repeat Sell order"));

	copyTableValuesMenu.actions().at(11)->setText(julyTr("CANCEL_ORDER","Cancel selected Orders"));
	copyTableValuesMenu.actions().at(12)->setText(julyTranslator.translateCheckBox("TR00075","Cancel All Orders"));

	QString staysOnTopText=julyTranslator.translateButton("STAYS_ON_TOP","Stay on top");
	ui.widgetStaysOnTop->setToolTip(staysOnTopText);
	ui.tabRulesOnTop->setToolTip(staysOnTopText);
	ui.tabDepthOnTop->setToolTip(staysOnTopText);
	ui.tabTradesOnTop->setToolTip(staysOnTopText);
	ui.tabChartsOnTop->setToolTip(staysOnTopText);

	foreach(RuleWidget* currentGroup, ui.tabRules->findChildren<RuleWidget*>())
		if(currentGroup)currentGroup->languageChanged();

	fixAllChildButtonsAndLabels(this);
	//emit clearValues();
}

void QtBitcoinTrader::buttonNewWindow()
{
	QProcess::startDetached(QApplication::applicationFilePath(),QStringList());
}

void QtBitcoinTrader::on_rulesTabs_tabCloseRequested(int tab)
{
	RuleWidget *currentGroup=dynamic_cast<RuleWidget*>(ui.rulesTabs->widget(tab));
	if(currentGroup==0)return;
	if(currentGroup->haveAnyRules())
	{
		QMessageBox msgBox(windowWidget);
		msgBox.setIcon(QMessageBox::Question);
		msgBox.setWindowTitle(julyTr("RULES_CONFIRM_DELETION","Please confirm rules group deletion"));
		msgBox.setText(julyTr("RULES_ARE_YOU_SURE_TO_DELETE_GROUP","Are you sure to delete rules group %1?").arg(currentGroup->windowTitle()));
		msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
		msgBox.setDefaultButton(QMessageBox::Yes);
		msgBox.setButtonText(QMessageBox::Yes,julyTr("YES","Yes"));
		msgBox.setButtonText(QMessageBox::No,julyTr("NO","No"));
		if(msgBox.exec()!=QMessageBox::Yes)return;
	}
	currentGroup->removeGroup();
	if(baseValues.lastGroupID==currentGroup->getRuleGroupId())baseValues.lastGroupID--;
	delete ui.rulesTabs->widget(tab);
	
	ui.rulesTabs->setVisible(ui.rulesTabs->count());
	ui.rulesNoMessage->setVisible(!ui.rulesTabs->isVisible());
}

bool QtBitcoinTrader::eventFilter(QObject *obj, QEvent *event)
{
	if(obj!=this&&obj!=windowWidget)
	{
		if(event->type()==QEvent::Close)
		{
			if(obj==ui.tabOrdersLog)QTimer::singleShot(50,this,SLOT(attachLog()));
			else
			if(obj==ui.tabRules)QTimer::singleShot(50,this,SLOT(attachRules()));
			else
			if(obj==ui.tabLastTrades)QTimer::singleShot(50,this,SLOT(attachTrades()));
			else
			if(obj==ui.tabDepth)QTimer::singleShot(50,this,SLOT(attachDepth()));
			else
			if(obj==ui.tabCharts)QTimer::singleShot(50,this,SLOT(attachCharts()));
		}
		else
		if(event->type()==QEvent::Resize)
		{
			if(obj==ui.balanceTotalWidget)
			{
				int sizeParent=ui.balanceTotalWidget->width();
				int sizeA=ui.totalAtBuySellGroupBox->minimumWidth();
				int sizeB=ui.totalAtLastGroupBox->minimumWidth();

				ui.totalAtBuySellGroupBox->setVisible(sizeParent>=sizeA);
				ui.totalAtLastGroupBox->setVisible(sizeParent>=sizeA+sizeB+ui.balanceTotalWidget->layout()->spacing());
			}
		}
	}
	return QObject::eventFilter(obj, event);
}

void QtBitcoinTrader::on_widgetStaysOnTop_toggled(bool on)
{
	windowWidget->hide();
	if(on)windowWidget->setWindowFlags(Qt::Window|Qt::WindowStaysOnTopHint);
	else  windowWidget->setWindowFlags(Qt::Window);

	windowWidget->show();
}

void QtBitcoinTrader::depthFirstOrder(double price, double volume, bool isAsk)
{
	waitingDepthLag=false;

	if(price==0.0||ui.comboBoxGroupByPrice->currentIndex()==0)return;

	if(isAsk)
		depthAsksModel->depthFirstOrder(price,volume);
	else
		depthBidsModel->depthFirstOrder(price,volume);
}

void QtBitcoinTrader::depthSubmitOrders(QList<DepthItem> *asks, QList<DepthItem> *bids)
{
	waitingDepthLag=false;
	int currentAsksScroll=ui.depthAsksTable->verticalScrollBar()->value();
	int currentBidsScroll=ui.depthBidsTable->verticalScrollBar()->value();
	depthAsksModel->depthUpdateOrders(asks);
	depthBidsModel->depthUpdateOrders(bids);
	ui.depthAsksTable->verticalScrollBar()->setValue(qMin(currentAsksScroll,ui.depthAsksTable->verticalScrollBar()->maximum()));
	ui.depthBidsTable->verticalScrollBar()->setValue(qMin(currentBidsScroll,ui.depthBidsTable->verticalScrollBar()->maximum()));
}

void QtBitcoinTrader::exitApp()
{
	if(trayIcon)trayIcon->hide();
	//saveRulesData();////

	saveDetachedWindowsSettings();

	iniSettings->setValue("UI/ConfirmOpenOrder",ui.confirmOpenOrder->isChecked());

	iniSettings->setValue("UI/TradesCurrentTab",ui.tabWidget->currentIndex());
	iniSettings->setValue("UI/CloseToTray",ui.minimizeOnCloseCheckBox->isChecked());

	iniSettings->setValue("UI/WindowOnTop",ui.widgetStaysOnTop->isChecked());
	
	iniSettings->setValue("UI/RulesSafeMode",baseValues.rulesSafeMode);
	iniSettings->setValue("UI/RulesSafeModeInterval",baseValues.rulesSafeModeInterval);

	iniSettings->setValue("UI/DepthAutoResizeColumns",ui.depthAutoResize->isChecked());
	if(!ui.depthAutoResize->isChecked())
	{
		iniSettings->setValue("UI/DepthColumnAsksSizeWidth",ui.depthAsksTable->columnWidth(1));
		iniSettings->setValue("UI/DepthColumnAsksVolumeWidth",ui.depthAsksTable->columnWidth(2));
		iniSettings->setValue("UI/DepthColumnAsksPriceWidth",ui.depthAsksTable->columnWidth(3));

		iniSettings->setValue("UI/DepthColumnBidsPriceWidth",ui.depthBidsTable->columnWidth(0));
		iniSettings->setValue("UI/DepthColumnBidsVolumeWidth",ui.depthBidsTable->columnWidth(1));
		iniSettings->setValue("UI/DepthColumnBidsSizeWidth",ui.depthBidsTable->columnWidth(2));
	}
	iniSettings->setValue("UI/FeeCalcSingleInstance",feeCalculatorSingleInstance);

	saveWindowState(windowWidget,"Window");
	iniSettings->sync();

	emit quit();
}

void QtBitcoinTrader::on_depthComboBoxLimitRows_currentIndexChanged(int val)
{
	baseValues.depthCountLimit=ui.depthComboBoxLimitRows->itemData(val,Qt::UserRole).toInt();
	baseValues.depthCountLimitStr=QByteArray::number(baseValues.depthCountLimit);
	iniSettings->setValue("UI/DepthCountLimit",baseValues.depthCountLimit);
	iniSettings->sync();
	clearDepth();
}

void QtBitcoinTrader::on_comboBoxGroupByPrice_currentIndexChanged(int val)
{
	baseValues.groupPriceValue=ui.comboBoxGroupByPrice->itemData(val,Qt::UserRole).toDouble();
	iniSettings->setValue("UI/DepthGroupByPrice",baseValues.groupPriceValue);
	iniSettings->sync();
	clearDepth();
}

void QtBitcoinTrader::on_depthAutoResize_toggled(bool on)
{
	if(on)
	{
	ui.depthAsksTable->horizontalHeader()->showSection(0);
	ui.depthBidsTable->horizontalHeader()->showSection(4);

	ui.depthAsksTable->horizontalHeader()->setResizeMode(0,QHeaderView::Stretch);
	ui.depthAsksTable->horizontalHeader()->setResizeMode(1,QHeaderView::ResizeToContents);
	ui.depthAsksTable->horizontalHeader()->setResizeMode(2,QHeaderView::ResizeToContents);
	ui.depthAsksTable->horizontalHeader()->setResizeMode(3,QHeaderView::ResizeToContents);
	ui.depthAsksTable->horizontalHeader()->setResizeMode(4,QHeaderView::ResizeToContents);

	ui.depthBidsTable->horizontalHeader()->setResizeMode(0,QHeaderView::ResizeToContents);
	ui.depthBidsTable->horizontalHeader()->setResizeMode(1,QHeaderView::ResizeToContents);
	ui.depthBidsTable->horizontalHeader()->setResizeMode(2,QHeaderView::ResizeToContents);
	ui.depthBidsTable->horizontalHeader()->setResizeMode(3,QHeaderView::ResizeToContents);
	ui.depthBidsTable->horizontalHeader()->setResizeMode(4,QHeaderView::Stretch);
	}
	else
	{
	ui.depthAsksTable->horizontalHeader()->setResizeMode(QHeaderView::Interactive);
	ui.depthBidsTable->horizontalHeader()->setResizeMode(QHeaderView::Interactive);
	ui.depthAsksTable->horizontalHeader()->hideSection(0);
	ui.depthBidsTable->horizontalHeader()->hideSection(4);
	}
}

double QtBitcoinTrader::getAvailableBTC()
{
	if(currentExchange->balanceDisplayAvailableAmount)return ui.accountBTC->value();
	return ui.accountBTC->value()-ui.ordersTotalBTC->value();	
}

double QtBitcoinTrader::getAvailableUSD()
{
	double amountToReturn=0.0;
	if(currentExchange->balanceDisplayAvailableAmount)amountToReturn=ui.accountUSD->value();
	else amountToReturn=ui.accountUSD->value()-ui.ordersTotalUSD->value();
	
	amountToReturn=getValidDoubleForPercision(amountToReturn,baseValues.currentPair.currBDecimals,false);

	if(currentExchange->exchangeSupportsAvailableAmount)amountToReturn=qMin(availableAmount,amountToReturn);
	
	currentExchange->filterAvailableUSDAmountValue(&amountToReturn);

	return amountToReturn;
}

double QtBitcoinTrader::getAvailableUSDtoBTC(double priceToBuy)
{
	double decValue=0.0;
	if(currentExchange->calculatingFeeMode==1)decValue=qPow(0.1,qMax(baseValues.currentPair.currADecimals,1));else
	if(currentExchange->calculatingFeeMode==2)decValue=2.0*qPow(0.1,qMax(baseValues.currentPair.currADecimals,1));
	return getValidDoubleForPercision(getAvailableUSD()/priceToBuy-decValue,baseValues.currentPair.currADecimals,false);
}

void QtBitcoinTrader::apiSellSend(double btc, double price)
{
	emit apiSell(btc,price);
}

void QtBitcoinTrader::apiBuySend(double btc, double price)
{
	emit apiBuy(btc,price);
}