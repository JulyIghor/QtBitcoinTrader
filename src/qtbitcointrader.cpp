// Copyright (C) 2013 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form https://sourceforge.net/projects/bitcointrader/
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#include <QTableWidget>
#include <QTimeLine>
#include <QScrollBar>
#include <QDir>
#include <QMessageBox>
#include <QSettings>
#include "main.h"
#include "julylightchanges.h"
#include "julyspinboxfix.h"
#include "addrulewindow.h"
#include <QFileInfo>
#include <QClipboard>
#include <QProcess>
#include "feecalculator.h"
#include <QFile>
#include <QSysInfo>
#include <QProcess>
#include <QDesktopWidget>
#include <QDesktopServices>
#include <QUrl>
#include "aboutdialog.h"
#include "audioplayer.h"
#include "exchange_mtgox.h"
#include "exchange_btce.h"
#include "exchange_bitstamp.h"
#include "exchange_btcchina.h"
#include <QSystemTrayIcon>
#include <QtCore/qmath.h>
#include "debugviewer.h"
#ifdef Q_OS_WIN
#include "windows.h"
#endif

QtBitcoinTrader::QtBitcoinTrader()
	: QDialog()
{
	availableAmount=0.0;
	exchangeSupportsAvailableAmount=false;
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

	if(supportsUtfUI)
	{
		upArrow=QByteArray::fromBase64("4oaR");
		downArrow=QByteArray::fromBase64("4oaT");
	}
	else
	{
		upArrow=">";
		downArrow="<";
	}

	trayIcon=0;
	minTradePrice=0.01;
	btcDecimals=8;
	usdDecimals=5;
	priceDecimals=5;
	minTradeVolume=0.01;
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

	buyLockTotalBtc=false;
	buyLockPricePerCoin=false;
	buyLockTotalSpend=false;

	lastMarketLowPrice=0.0;
	lastMarketHighPrice=0.0;

	ui.setupUi(this);

	iniSettings=new QSettings(iniFileName,QSettings::IniFormat,this);

	groupPriceValue=iniSettings->value("UI/DepthGroupByPrice",0.0).toDouble();
	if(groupPriceValue<0.0)groupPriceValue=0.0;
	iniSettings->setValue("UI/DepthGroupByPrice",groupPriceValue);

	setAttribute(Qt::WA_QuitOnClose,true);

	setWindowFlags(Qt::Window);

	ui.ordersTableFrame->setVisible(false);

	ordersModel=new OrdersModel;
	ordersSortModel=new QSortFilterProxyModel(this);
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

	ui.ordersTable->setSortingEnabled(true);
	ui.ordersTable->sortByColumn(0,Qt::AscendingOrder);

	connect(ordersModel,SIGNAL(ordersIsAvailable()),this,SLOT(ordersIsAvailable()));
	connect(ordersModel,SIGNAL(cancelOrder(QByteArray)),this,SLOT(cancelOrder(QByteArray)));
	connect(ordersModel,SIGNAL(volumeAmountChanged(double, double)),this,SLOT(volumeAmountChanged(double, double)));
	connect(ui.ordersTable->selectionModel(),SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),this,SLOT(checkValidOrdersButtons()));

	ui.rulesNoMessage->setVisible(true);
	ui.rulesTable->setVisible(false);

	rulesModel=new RulesModel;
	ui.rulesTable->setModel(rulesModel);
	ui.rulesTable->horizontalHeader()->setResizeMode(0,QHeaderView::ResizeToContents);
	ui.rulesTable->horizontalHeader()->setResizeMode(1,QHeaderView::Stretch);
	ui.rulesTable->horizontalHeader()->setResizeMode(2,QHeaderView::ResizeToContents);
	ui.rulesTable->horizontalHeader()->setResizeMode(3,QHeaderView::ResizeToContents);
	ui.rulesTable->horizontalHeader()->setResizeMode(4,QHeaderView::ResizeToContents);
	connect(ui.rulesTable->selectionModel(),SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),this,SLOT(checkValidRulesButtons()));
	ui.rulesTable->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui.rulesTable, SIGNAL(customContextMenuRequested(const QPoint&)), SLOT(rulesMenuRequested(const QPoint&)));


	tradesModel=new TradesModel;
	ui.tableTrades->setModel(tradesModel);
	ui.tableTrades->horizontalHeader()->setResizeMode(0,QHeaderView::ResizeToContents);
	ui.tableTrades->horizontalHeader()->setResizeMode(1,QHeaderView::Stretch);
	ui.tableTrades->horizontalHeader()->setResizeMode(2,QHeaderView::ResizeToContents);
	ui.tableTrades->horizontalHeader()->setResizeMode(3,QHeaderView::ResizeToContents);
	ui.tableTrades->horizontalHeader()->setResizeMode(4,QHeaderView::Stretch);
	connect(tradesModel,SIGNAL(trades10MinVolumeChanged(double)),this,SLOT(setLastTrades10MinVolume(double)));
	connect(tradesModel,SIGNAL(precentBidsChanged(double)),this,SLOT(precentBidsChanged(double)));

	historyModel=new HistoryModel;
	ui.tableHistory->setModel(historyModel);
	ui.tableHistory->horizontalHeader()->setResizeMode(0,QHeaderView::ResizeToContents);
	ui.tableHistory->horizontalHeader()->setResizeMode(1,QHeaderView::Stretch);
	ui.tableHistory->horizontalHeader()->setResizeMode(2,QHeaderView::ResizeToContents);
	ui.tableHistory->horizontalHeader()->setResizeMode(3,QHeaderView::Stretch);
	connect(historyModel,SIGNAL(accLastSellChanged(QByteArray,double)),this,SLOT(accLastSellChanged(QByteArray,double)));
	connect(historyModel,SIGNAL(accLastBuyChanged(QByteArray,double)),this,SLOT(accLastBuyChanged(QByteArray,double)));


	depthAsksModel=new DepthModel(true);
	ui.depthAsksTable->setModel(depthAsksModel);
	depthBidsModel=new DepthModel(false);
	ui.depthBidsTable->setModel(depthBidsModel);

	ui.depthAsksTable->horizontalHeader()->setResizeMode(0,QHeaderView::Stretch);
	ui.depthAsksTable->horizontalHeader()->setResizeMode(1,QHeaderView::ResizeToContents);
	ui.depthAsksTable->horizontalHeader()->setResizeMode(2,QHeaderView::ResizeToContents);
	ui.depthAsksTable->horizontalHeader()->setResizeMode(3,QHeaderView::ResizeToContents);
	ui.depthAsksTable->horizontalHeader()->setMinimumSectionSize(0);

	ui.depthBidsTable->horizontalHeader()->setResizeMode(0,QHeaderView::ResizeToContents);
	ui.depthBidsTable->horizontalHeader()->setResizeMode(1,QHeaderView::ResizeToContents);
	ui.depthBidsTable->horizontalHeader()->setResizeMode(2,QHeaderView::ResizeToContents);
	ui.depthBidsTable->horizontalHeader()->setResizeMode(3,QHeaderView::Stretch);
	ui.depthBidsTable->horizontalHeader()->setMinimumSectionSize(0);

	ui.accountBTCBeep->setChecked(iniSettings->value("Sounds/AccountBTCBeep",false).toBool());
	ui.accountUSDBeep->setChecked(iniSettings->value("Sounds/AccountUSDBeep",false).toBool());
	ui.marketHighBeep->setChecked(iniSettings->value("Sounds/MarketHighBeep",false).toBool());
	ui.marketLowBeep->setChecked(iniSettings->value("Sounds/MarketLowBeep",false).toBool());
	ui.ruleBeep->setChecked(iniSettings->value("Sounds/RuleExecutedBeep",false).toBool());

	ui.minimizeOnCloseCheckBox->setChecked(iniSettings->value("UI/CloseToTray",false).toBool());

	checkValidOrdersButtons();

	new JulyLightChanges(ui.marketVolume);
	new JulyLightChanges(ui.marketBuy);
	new JulyLightChanges(ui.marketSell);
	new JulyLightChanges(ui.marketBuy);
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

	foreach(QDoubleSpinBox* spinBox, findChildren<QDoubleSpinBox*>())new JulySpinBoxFix(spinBox);

	QSettings settingsMain(appDataDir+"/QtBitcoinTrader.cfg",QSettings::IniFormat);
	checkForUpdates=settingsMain.value("CheckForUpdates",true).toBool();

	int defTextHeight=fontMetrics_->boundingRect("0123456789").height();
	defaultSectionSize=settingsMain.value("RowHeight",defTextHeight*1.6).toInt();
	if(defaultSectionSize<defTextHeight)defaultSectionSize=defTextHeight;
	settingsMain.setValue("RowHeight",defaultSectionSize);

	depthCountLimit=iniSettings->value("UI/DepthCountLimit",100).toInt();
	if(depthCountLimit<0)depthCountLimit=100;
	depthCountLimitStr=QByteArray::number(depthCountLimit);
	int currentDepthComboBoxLimitIndex=0;
	for(int n=0;n<ui.depthComboBoxLimitRows->count();n++)
	{
		int currentValueDouble=ui.depthComboBoxLimitRows->itemText(n).toInt();
		if(currentValueDouble==depthCountLimit)currentDepthComboBoxLimitIndex=n;
		ui.depthComboBoxLimitRows->setItemData(n,currentValueDouble,Qt::UserRole);
	}
	ui.depthComboBoxLimitRows->setCurrentIndex(currentDepthComboBoxLimitIndex);

	exchangeId=iniSettings->value("Profile/ExchangeId",0).toInt();

	apiDownCount=iniSettings->value("Network/ApiDownCounterMax",5).toInt();
	if(apiDownCount<0)apiDownCount=5;
	iniSettings->setValue("Network/ApiDownCounterMax",apiDownCount);

	httpRequestInterval=iniSettings->value("Network/HttpRequestsInterval",500).toInt();
	httpRequestTimeout=iniSettings->value("Network/HttpRequestsTimeout",3000).toInt();
	httpRetryCount=iniSettings->value("Network/HttpRetryCount",5).toInt();
	if(httpRetryCount<1||httpRetryCount>50)httpRetryCount=5;

	uiUpdateInterval=iniSettings->value("UI/UiUpdateInterval",100).toInt();
	if(uiUpdateInterval<1)uiUpdateInterval=100;

	httpSplitPackets=iniSettings->value("Network/HttpSplitPackets",false).toBool();

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
		if(currentValueDouble==groupPriceValue)currentDepthComboBoxIndex=n;
		ui.comboBoxGroupByPrice->setItemData(n,currentValueDouble,Qt::UserRole);
	}
	ui.comboBoxGroupByPrice->setCurrentIndex(currentDepthComboBoxIndex);


	if(appVerLastReal<1.0763)
	{
		httpRequestInterval=500;
		httpRequestTimeout=3000;
		httpSplitPackets=false;
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
	if(httpRequestInterval<50)httpRequestInterval=500;
	if(httpRequestTimeout<100)httpRequestTimeout=3000;

	if(exchangeId==2)//Bitstamp exception
	{
		if(httpRequestInterval<1200)httpRequestInterval=1200;
	}

	iniSettings->setValue("Network/HttpRequestsInterval",httpRequestInterval);
	iniSettings->setValue("Network/HttpRequestsTimeout",httpRequestTimeout);
	iniSettings->setValue("Network/HttpSplitPackets",httpSplitPackets);
	iniSettings->setValue("Network/HttpRetryCount",httpRetryCount);
	iniSettings->setValue("UI/UiUpdateInterval",uiUpdateInterval);
	iniSettings->setValue("UI/DepthAutoResizeColumns",ui.depthAutoResize->isChecked());

	profileName=iniSettings->value("Profile/Name","Default Profile").toString();
	windowTitleP=profileName+" - "+windowTitle()+" v"+appVerStr;
	if(debugLevel)windowTitleP.append(" [DEBUG MODE]");
	else if(appVerIsBeta)windowTitleP.append(" [BETA]");

	setWindowTitle(windowTitleP);

	foreach(QTableWidget* tables, findChildren<QTableWidget*>())
	{
		tables->setMinimumWidth(200);
		tables->setMinimumHeight(200);
		tables->verticalHeader()->setDefaultSectionSize(defaultSectionSize);
	}
	foreach(QTableView* tables, findChildren<QTableView*>())
	{
		QFont tableFont=tables->font();
		tableFont.setFixedPitch(true);
		tables->setFont(tableFont);
		tables->setMinimumWidth(200);
		tables->setMinimumHeight(200);
		tables->verticalHeader()->setDefaultSectionSize(defaultSectionSize);
	}

	highResolutionDisplay=false;
	int screenCount=QApplication::desktop()->screenCount();
	QPoint cursorPos=QCursor::pos();
	currentDesktopRect=QRect(0,0,1024,720);
	for(int n=0;n<screenCount;n++)
	{
		QRect currScrRect=QApplication::desktop()->screenGeometry(n);
		if(currScrRect.contains(cursorPos))
			currentDesktopRect=QApplication::desktop()->availableGeometry(n);
		if(currentDesktopRect.width()>1024&&currentDesktopRect.height()>768)highResolutionDisplay=true;
	}

	rulesEnableDisableMenu=new QMenu;
	rulesEnableDisableMenu->addAction("Enable Selected");
	connect(rulesEnableDisableMenu->actions().last(),SIGNAL(triggered(bool)),this,SLOT(ruleEnableSelected()));
	rulesEnableDisableMenu->addAction("Disable Selected");
	connect(rulesEnableDisableMenu->actions().last(),SIGNAL(triggered(bool)),this,SLOT(ruleDisableSelected()));
	rulesEnableDisableMenu->addSeparator();
	rulesEnableDisableMenu->addAction("Enable All");
	connect(rulesEnableDisableMenu->actions().last(),SIGNAL(triggered(bool)),this,SLOT(ruleEnableAll()));
	rulesEnableDisableMenu->addAction("Disable All");
	connect(rulesEnableDisableMenu->actions().last(),SIGNAL(triggered(bool)),this,SLOT(ruleDisableAll()));
	ui.ruleEnableDisable->setMenu(rulesEnableDisableMenu);
	connect(rulesEnableDisableMenu,SIGNAL(aboutToShow()),this,SLOT(ruleDisableEnableMenuFix()));

	rulesModel->restoreRulesFromString(iniSettings->value("Rules/Data","").toString());

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
	
	setApiDown(false);

	accountFeeChanged(ui.accountFee->value());

	reloadLanguageList();

	checkValidRulesButtons();
	connect(julyTranslator,SIGNAL(languageChanged()),this,SLOT(languageChanged()));

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

void QtBitcoinTrader::keyPressEvent(QKeyEvent *event)
{
	event->accept();
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
		ui.tradesLabelDirection->setText(precentGrowing?upArrow:downArrow);
	}
	tradesPrecentLast=val;
}

void QtBitcoinTrader::saveRulesData()
{
	iniSettings->setValue("Rules/Data",rulesModel->saveRulesToString());
	iniSettings->sync();
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

	QString tempText=ui.sellOrdersLabel->text();
	QString tempId=ui.sellOrdersLabel->accessibleName();
	QString tempStyle=ui.sellOrdersLabel->styleSheet();

	ui.sellOrdersLabel->setText(ui.buyOrdersLabel->text());
	ui.sellOrdersLabel->setAccessibleName(ui.buyOrdersLabel->accessibleName());
	ui.sellOrdersLabel->setStyleSheet(ui.buyOrdersLabel->styleSheet());

	ui.buyOrdersLabel->setText(tempText);
	ui.buyOrdersLabel->setAccessibleName(tempId);
	ui.buyOrdersLabel->setStyleSheet(tempStyle);

	iniSettings->setValue("UI/SwapDepth",swapedDepth);
	iniSettings->sync();
}

void QtBitcoinTrader::rulesMenuRequested(const QPoint& point)
{
	rulesEnableDisableMenu->exec(ui.rulesTable->viewport()->mapToGlobal(point));
}

void QtBitcoinTrader::ruleDisableEnableMenuFix()
{
	bool haveRules=rulesModel->rowCount();
	QModelIndexList selectedRows=ui.rulesTable->selectionModel()->selectedRows();

	int selectedRulesCount=selectedRows.count();
	bool ifSelectedOneRuleIsItEnabled=selectedRulesCount==1;
	if(ifSelectedOneRuleIsItEnabled)
	{
		ifSelectedOneRuleIsItEnabled=rulesModel->getRuleHolderByRow(selectedRows.first().row())->getRuleState()==1;

		rulesEnableDisableMenu->actions().at(0)->setEnabled(!ifSelectedOneRuleIsItEnabled);
		rulesEnableDisableMenu->actions().at(1)->setEnabled(ifSelectedOneRuleIsItEnabled);
	}
	else
	{
		rulesEnableDisableMenu->actions().at(0)->setEnabled(selectedRulesCount>0);
		rulesEnableDisableMenu->actions().at(1)->setEnabled(selectedRulesCount>0);
	}
	rulesEnableDisableMenu->actions().at(2)->setEnabled(haveRules);
	rulesEnableDisableMenu->actions().at(3)->setEnabled(haveRules);
}

void QtBitcoinTrader::anyDataReceived()
{
	softLagTime.restart();
	setSoftLagValue(0);
}

double QtBitcoinTrader::getFeeForUSDDec(double usd)
{
	double result=getValidDoubleForPercision(usd,usdDecimals,false);
	double calcFee=getValidDoubleForPercision(result,priceDecimals,true)*floatFee;
	calcFee=getValidDoubleForPercision(calcFee,priceDecimals,true);
	result=result-calcFee;
	if(result==usd)result-=qPow(0.1,usdDecimals);
	return result;
}

double QtBitcoinTrader::getFeeForUSDInc(double usd)
{
	double result=getValidDoubleForPercision(usd,usdDecimals,false);
	result+=getValidDoubleForPercision(result*floatFee,usdDecimals,true);
	if(usd==result)result+=qPow(0.1,usdDecimals);
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
	show();
	if(ui.tabOrdersLog->parent()==0)ui.tabOrdersLog->show();
	if(ui.tabLastTrades->parent()==0)ui.tabLastTrades->show();
	if(ui.tabRules->parent()==0)ui.tabRules->show();
	if(ui.tabDepth->parent()==0)ui.tabDepth->show();
	if(ui.tabCharts->parent()==0)ui.tabCharts->show();
	trayIcon->hide();
	delete trayMenu; trayMenu=0;
	delete trayIcon; trayIcon=0;
}

void QtBitcoinTrader::tabLogOrdersOnTop(bool on)
{
	if(!isDetachedLog)return;
	ui.tabOrdersLog->hide();
	if(on)ui.tabOrdersLog->setWindowFlags(Qt::Window|Qt::WindowStaysOnTopHint);
	else  ui.tabOrdersLog->setWindowFlags(Qt::Window);
	ui.tabOrdersLog->show();
}

void QtBitcoinTrader::tabRulesOnTop(bool on)
{
	if(!isDetachedRules)return;
	ui.tabRules->hide();
	if(on)ui.tabRules->setWindowFlags(Qt::Window|Qt::WindowStaysOnTopHint);
	else  ui.tabRules->setWindowFlags(Qt::Window);
	ui.tabRules->show();
}

void QtBitcoinTrader::tabTradesOnTop(bool on)
{
	if(!isDetachedTrades)return;
	ui.tabLastTrades->hide();
	if(on)ui.tabLastTrades->setWindowFlags(Qt::Window|Qt::WindowStaysOnTopHint);
	else  ui.tabLastTrades->setWindowFlags(Qt::Window);
	ui.tabLastTrades->show();
}

void QtBitcoinTrader::tabDepthOnTop(bool on)
{
	if(!isDetachedDepth)return;
	ui.tabDepth->hide();
	if(on)ui.tabDepth->setWindowFlags(Qt::Window|Qt::WindowStaysOnTopHint);
	else  ui.tabDepth->setWindowFlags(Qt::Window);
	ui.tabDepth->show();
}

void QtBitcoinTrader::tabChartsOnTop(bool on)
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

void QtBitcoinTrader::loadUiSettings()
{
	QString savedCurrency=iniSettings->value("Profile/Currency","BTC/USD").toString();
	int indexCurrency=-1;
	switch(exchangeId)
	{
	case 0:
		{
			btcDecimals=8;
			usdDecimals=5;
			btcBalanceDecimals=8;
			usdBalanceDecimals=5;
			priceDecimals=5;
			QFile curMap(":/Resources/CurrenciesMtGox.map");
			curMap.open(QIODevice::ReadOnly);
			QStringList curencyList=QString(curMap.readAll().replace("\r","")).split("\n");
			curMap.close();
			for(int n=0;n<curencyList.count();n++)
			{
				QStringList curDataList=curencyList.at(n).split("=");
				if(curDataList.count()!=5)continue;
				QString curName=curDataList.first();
				curDataList.removeFirst();
				if(curName==savedCurrency)indexCurrency=ui.currencyComboBox->count();
				ui.currencyComboBox->insertItem(ui.currencyComboBox->count(),curName,curDataList);
			}

			exchangeName="Mt.Gox"; (new Exchange_MtGox(restSign,restKey))->setupApi(this,false);
		}break;
	case 1:
		{//BTC-E
			ui.accountFee->setValue(0.2);
			btcDecimals=8;
			usdDecimals=8;
			btcBalanceDecimals=8;
			usdBalanceDecimals=8;
			priceDecimals=3;
			exchangeName="BTC-E";
			ui.loginVolumeBack->setVisible(false);
			ui.exchangeLagBack->setVisible(false);
			ui.lagValue->setVisible(false);
			ui.lagMtgoxLabel->setVisible(false);

			QFile curMap(":/Resources/CurrenciesBTCe.map");
			curMap.open(QIODevice::ReadOnly);
			QStringList curencyList=QString(curMap.readAll().replace("\r","")).split("\n");
			curMap.close();
			for(int n=0;n<curencyList.count();n++)
			{
				QStringList curDataList=curencyList.at(n).split("=");
				if(curDataList.count()!=5)continue;
				QString curName=curDataList.first();
				curDataList.removeFirst();
				if(curName==savedCurrency)indexCurrency=ui.currencyComboBox->count();
				ui.currencyComboBox->insertItem(ui.currencyComboBox->count(),curName,curDataList);
			}

			(new Exchange_BTCe(restSign,restKey))->setupApi(this,false);
		}break;
	case 2:
		{//Bitstamp
			exchangeSupportsAvailableAmount=true;
			ui.tableTrades->horizontalHeader()->hideSection(2);
			ui.tradesBidsPrecent->setVisible(false);
			ui.tradesLabelDirection->setVisible(false);

			btcDecimals=8;
			usdDecimals=5;
			btcBalanceDecimals=8;
			usdBalanceDecimals=5;
			priceDecimals=2;
			QFile curMap(":/Resources/CurrenciesBitstamp.map");
			curMap.open(QIODevice::ReadOnly);
			QStringList curencyList=QString(curMap.readAll().replace("\r","")).split("\n");
			curMap.close();
			for(int n=0;n<curencyList.count();n++)
			{
				QStringList curDataList=curencyList.at(n).split("=");
				if(curDataList.count()!=5)continue;
				QString curName=curDataList.first();
				curDataList.removeFirst();
				if(curName==savedCurrency)indexCurrency=ui.currencyComboBox->count();
				ui.currencyComboBox->insertItem(ui.currencyComboBox->count(),curName,curDataList);
			}
			ordersModel->checkDuplicatedOID=true;
			exchangeName="Bitstamp"; (new Exchange_Bitstamp(restSign,restKey))->setupApi(this,false);
			ui.loginVolumeBack->setVisible(false);
			ui.exchangeLagBack->setVisible(false);
			ui.lagValue->setVisible(false);
			ui.lagMtgoxLabel->setVisible(false);
		}
		break;
	case 3:
		{//BTC China
			ui.tableTrades->horizontalHeader()->hideSection(2);
			ui.tradesBidsPrecent->setVisible(false);
			ui.tradesLabelDirection->setVisible(false);
			btcDecimals=3;
			usdDecimals=2;
			btcBalanceDecimals=8;
			usdBalanceDecimals=5;
			priceDecimals=5;
			QFile curMap(":/Resources/CurrenciesBTCChina.map");
			curMap.open(QIODevice::ReadOnly);
			QStringList curencyList=QString(curMap.readAll().replace("\r","")).split("\n");
			curMap.close();
			for(int n=0;n<curencyList.count();n++)
			{
				QStringList curDataList=curencyList.at(n).split("=");
				if(curDataList.count()!=5)continue;
				QString curName=curDataList.first();
				curDataList.removeFirst();
				if(curName==savedCurrency)indexCurrency=ui.currencyComboBox->count();
				ui.currencyComboBox->insertItem(ui.currencyComboBox->count(),curName,curDataList);
			}
			btcBalanceDecimals=8;
			usdBalanceDecimals=5;
			exchangeName="BTC China"; (new Exchange_BTCChina(restSign,restKey))->setupApi(this,false);
			//ui.loginVolumeBack->setVisible(false);
			ui.labelAccountVolume->setVisible(false);
			ui.btcLabel2->setVisible(false);
			ui.accountVolume->setVisible(false);

			ui.exchangeLagBack->setVisible(false);
			ui.lagValue->setVisible(false);
			ui.lagMtgoxLabel->setVisible(false);
		}break;
	default:
		ui.loginVolumeBack->setVisible(false);
		ui.lagValue->setVisible(false);
		ui.lagMtgoxLabel->setVisible(false);
		ui.apiDownLabel->setVisible(true);
	break;
	}
	priceMinimumValue=qPow(0.1,priceDecimals);
	if(indexCurrency>-1)ui.currencyComboBox->setCurrentIndex(indexCurrency);
	currencyChanged(ui.currencyComboBox->currentIndex());

	if(iniSettings->value("UI/DetachedLog",isDetachedLog).toBool())detachLog();

	if(iniSettings->value("UI/DetachedRules",isDetachedRules).toBool())detachRules();
	
	if(iniSettings->value("UI/DetachedTrades",isDetachedRules).toBool())detachTrades();

	if(iniSettings->value("UI/DetachedDepth",isDetachedDepth).toBool())detachDepth();

	if(iniSettings->value("UI/DetachedCharts",isDetachedRules).toBool())detachCharts();
	
	int savedTab=iniSettings->value("UI/TradesCurrentTab",0).toInt();
	if(savedTab<ui.tabWidget->count())ui.tabWidget->setCurrentIndex(savedTab);

	ui.widgetStaysOnTop->setChecked(iniSettings->value("UI/WindowOnTop",false).toBool());

	loadWindowState(this,"Window");
	iniSettings->sync();
	
	if(!ui.widgetStaysOnTop->isChecked())
		setWindowStaysOnTop(ui.widgetStaysOnTop->isChecked());

	languageChanged();
}

void QtBitcoinTrader::checkUpdate()
{
    QProcess::startDetached(QApplication::applicationFilePath(),QStringList("/checkupdatemessage"));
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

void QtBitcoinTrader::addLastTrade(double volumeDouble, qint64 dateT, double priceDouble, QByteArray symbol, bool isAsk)
{
	if(dateT<1000)return;

	tradesModel->addNewTrade(dateT,volumeDouble,priceDouble,symbol,isAsk);
	tradesModel->updateTotalBTC();
	if(ui.tradesAutoScrollCheck->isChecked()&&ui.tabLastTrades->isVisible())
	{
		setTradesScrollBarValue(ui.tableTrades->verticalScrollBar()->value()+ui.tableTrades->rowHeight(0));
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
		clearTimeOutedTrades();
		setSoftLagValue(softLagTime.elapsed());
		if(isValidSoftLag)checkAllRules();
	}
	else
	if(execCount==1||execCount==3||execCount==5)
	{
		int currentElapsed=depthLagTime.elapsed();
		depthRefreshBlocked=currentElapsed<=400;
		if(ui.tabDepth->isVisible())ui.depthLag->setValue(currentElapsed/1000.0);
	}

	if(++execCount>5)execCount=0;
	secondTimer.start(uiUpdateInterval);
}

void QtBitcoinTrader::tabTradesScrollUp()
{
	static QTimeLine timeLine(1,this);
	if(timeLine.duration()==1)
	{
		connect(&timeLine,SIGNAL(frameChanged(int)),this,SLOT(setTradesScrollBarValue(int)));
		timeLine.setDuration(600);
		timeLine.setEasingCurve(QEasingCurve::OutQuad);
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
	if(preferedLangFile.isEmpty())preferedLangFile=julyTranslator->lastFile();
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
	if(preferedLangFile.isEmpty()||!preferedLangFile.isEmpty()&&!QFile::exists(preferedLangFile))preferedLangFile=defaultLangFile;
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
	julyTranslator->loadFromFile(preferedLangFile);
	constructorFinished=true;
	languageChanged();
}

void QtBitcoinTrader::languageComboBoxChanged(int val)
{
	if(val<0||!constructorFinished)return;
	QString loadFromFile=ui.langComboBox->itemData(val,Qt::UserRole).toString();
	if(loadFromFile.isEmpty())return;
	julyTranslator->loadFromFile(loadFromFile);
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
		if(labels->text().length()&&labels->text().at(0)!='<')
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
			if(spinBox->accessibleName().endsWith("BALANCE"))spinBox->setDecimals(btcBalanceDecimals);
			else spinBox->setDecimals(btcDecimals);
			if(spinBox->accessibleDescription()!="CAN_BE_ZERO")
				spinBox->setMinimum(minTradeVolume);
		}
		else
			if(spinBox->accessibleName().startsWith("USD"))
			{
				if(spinBox->accessibleName().endsWith("BALANCE"))spinBox->setDecimals(usdBalanceDecimals);
				else spinBox->setDecimals(usdDecimals);
			}
			else
				if(spinBox->accessibleName()=="PRICE")
				{
					spinBox->setDecimals(priceDecimals);
					if(spinBox->accessibleDescription()!="CAN_BE_ZERO")
						spinBox->setMinimum(minTradePrice);
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

void QtBitcoinTrader::currencyChanged(int val)
{
	if(!constructorFinished||val<0)return;
	QStringList curDataList=ui.currencyComboBox->itemData(val,Qt::UserRole).toStringList();
	if(curDataList.count()!=4)return;
	if(val!=lastLoadedCurrency)lastLoadedCurrency=val;else return;

	QStringList curPair=ui.currencyComboBox->currentText().split("/");

	bool currencyAChanged=curPair.first()!=currencyAStr;
	currencyAStr=curPair.first().toAscii();
	currencyAStrLow=currencyAStr.toLower();
	currencyASign=currencySignMap->value(currencyAStr,"$");

	bool currencyBChanged=curPair.last()!=currencyBStr;
	currencyBStr=curPair.last().toAscii();
	currencyBStrLow=currencyBStr.toLower();
	currencyBSign=currencySignMap->value(currencyBStr,"$");

	currencySymbol=currencyAStr+currencyBStr;

	fillAllUsdLabels(this,currencyBStr);
	fillAllBtcLabels(this,currencyAStr);

	currencyRequestPair=curDataList.first().toAscii();
	priceDecimals=curDataList.at(1).toInt();

	priceMinimumValue=qPow(0.1,priceDecimals);
	minTradeVolume=curDataList.at(2).toDouble();
	minTradePrice=curDataList.at(3).toDouble();

	iniSettings->setValue("Profile/Currency",ui.currencyComboBox->currentText());
	if(currencyAChanged)ui.accountBTC->setValue(0.0);
	if(currencyBChanged)ui.accountUSD->setValue(0.0);
	ui.marketBuy->setValue(0.0);
	ui.marketSell->setValue(0.0);
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

	QString curCurrencyName=currencyNamesMap->value(currencyAStr,"BITCOINS");

	QString buyGroupboxText=julyTr("GROUPBOX_BUY","Buy %1");
	bool buyGroupboxCase=false; if(buyGroupboxText.length()>2)buyGroupboxCase=buyGroupboxText.at(2).isUpper();

	if(buyGroupboxCase)buyGroupboxText=buyGroupboxText.arg(curCurrencyName.toUpper());
	else buyGroupboxText=buyGroupboxText.arg(curCurrencyName);

	ui.buyGroupbox->setTitle(buyGroupboxText);

	QString sellGroupboxText=julyTr("GROUPBOX_SELL","Sell %1");
	bool sellGroupboxCase=true; if(sellGroupboxText.length()>2)sellGroupboxCase=sellGroupboxText.at(2).isUpper();

	if(sellGroupboxCase)sellGroupboxText=sellGroupboxText.arg(curCurrencyName.toUpper());
	else sellGroupboxText=sellGroupboxText.arg(curCurrencyName);

	ui.sellGroupBox->setTitle(sellGroupboxText);


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
	on_sellPricePerCoinAsMarketPrice_clicked();
	on_buyPriceAsMarketPrice_clicked();
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

void QtBitcoinTrader::profitBuyThanSellPrecChanged(double val)
{
	if(profitBuyThanSellChangedUnlocked)
	{
		profitBuyThanSellChangedUnlocked=false;
		ui.profitLossSpinBox->setValue(val==0.0?0.0:ui.buyTotalSpend->value()*val/100.0);
		profitBuyThanSellChangedUnlocked=true;
	}
}

void QtBitcoinTrader::profitBuyThanSellChanged(double val)
{
	QString styleChanged;
	if(val<-0.009)styleChanged="QDoubleSpinBox {background: #ffaaaa;}";
	else
	if(val>0.009)styleChanged="QDoubleSpinBox {background: #aaffaa;}";

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

void QtBitcoinTrader::profitSellThanBuyPrecChanged(double val)
{
	if(profitBuyThanSellChangedUnlocked)
	{
		profitBuyThanSellChangedUnlocked=false;
		ui.sellThanBuySpinBox->setValue(val==0.0?0.0:ui.sellTotalBtc->value()*val/100.0);
		profitBuyThanSellChangedUnlocked=true;
	}
}

void QtBitcoinTrader::profitSellThanBuyChanged(double val)
{
	QString styleChanged;
	if(val<-0.009)styleChanged="QDoubleSpinBox {background: #ffaaaa;}";
	else
	if(val>0.009)styleChanged="QDoubleSpinBox {background: #aaffaa;}";

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

void QtBitcoinTrader::zeroSellThanBuyProfit()
{
	ui.sellThanBuySpinBox->setValue(0.0);
}

void QtBitcoinTrader::zeroBuyThanSellProfit()
{
	ui.profitLossSpinBox->setValue(0.0);
}

void QtBitcoinTrader::profitSellThanBuy()
{
	profitSellThanBuyUnlocked=false;
	ui.buyTotalSpend->setValue(ui.sellAmountToReceive->value());
	ui.buyPricePerCoin->setValue(ui.buyTotalSpend->value()/((ui.sellTotalBtc->value()+ui.sellThanBuySpinBox->value())/floatFeeDec)-priceMinimumValue);
	profitSellThanBuyUnlocked=true;
	profitBuyThanSellCalc();
	profitSellThanBuyCalc();
	ui.buttonSellThenBuyApply->setEnabled(false);
}

void QtBitcoinTrader::profitBuyThanSell()
{
	profitBuyThanSellUnlocked=false;
	ui.sellTotalBtc->setValue(ui.buyTotalBtcResult->value());
	ui.sellPricePerCoin->setValue((ui.buyTotalSpend->value()+ui.profitLossSpinBox->value())/(ui.sellTotalBtc->value()*floatFeeDec)+priceMinimumValue);
	profitBuyThanSellUnlocked=true;
	profitBuyThanSellCalc();
	profitSellThanBuyCalc();
	ui.buttonBuyThenSellApply->setEnabled(false);
}

void QtBitcoinTrader::setApiDown(bool on)
{
	switch(exchangeId)
	{
	case 0: 
		ui.lagValue->setVisible(!on);
		ui.lagMtgoxLabel->setVisible(!on);
		ui.apiDownLabel->setVisible(on);
		break;
	case 1:
		ui.exchangeLagBack->setVisible(on);
		break;
	case 2: 
		ui.lagValue->setVisible(!on);
		ui.lagMtgoxLabel->setVisible(!on);
		ui.apiDownLabel->setVisible(on);
		break;
	default: break;
	}
}

QString QtBitcoinTrader::clearData(QString data)
{
	while(data.count()&&(data.at(0)=='{'||data.at(0)=='['||data.at(0)=='\"'))data.remove(0,1);
	while(data.count()&&(data.at(data.length()-1)=='}'||data.at(data.length()-1)==']'||data.at(data.length()-1)=='\"'))data.remove(data.length()-1,1);
	return data;
}

void QtBitcoinTrader::saveSoundToggles()
{
	if(!constructorFinished)return;
	iniSettings->setValue("Sounds/AccountBTCBeep",ui.accountBTCBeep->isChecked());
	iniSettings->setValue("Sounds/AccountUSDBeep",ui.accountUSDBeep->isChecked());
	iniSettings->setValue("Sounds/MarketHighBeep",ui.marketHighBeep->isChecked());
	iniSettings->setValue("Sounds/MarketLowBeep",ui.marketLowBeep->isChecked());
	iniSettings->setValue("Sounds/RuleExecutedBeep",ui.ruleBeep->isChecked());
	iniSettings->sync();
}

void QtBitcoinTrader::accountFeeChanged(double val)
{
	floatFee=val/100;
	floatFeeDec=1.0-floatFee;                 
	floatFeeInc=1.0+floatFee;
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

void QtBitcoinTrader::mtgoxLagChanged(double val)
{
	if(val>=1.0)
		ui.lagValue->setStyleSheet("QDoubleSpinBox {background: #ffaaaa;}");
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
	emit getHistory(false);
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
	ui.ordersTable->setSortingEnabled(false);

	ordersSortModel->setSourceModel(0);
	ordersModel->ordersChanged(orders);
	ordersSortModel->setSourceModel(ordersModel);

	ui.ordersTable->horizontalHeader()->setResizeMode(0,QHeaderView::ResizeToContents);
	ui.ordersTable->horizontalHeader()->setResizeMode(1,QHeaderView::Stretch);
	ui.ordersTable->horizontalHeader()->setResizeMode(2,QHeaderView::ResizeToContents);
	ui.ordersTable->horizontalHeader()->setResizeMode(3,QHeaderView::ResizeToContents);
	ui.ordersTable->horizontalHeader()->setResizeMode(4,QHeaderView::ResizeToContents);
	ui.ordersTable->horizontalHeader()->setResizeMode(5,QHeaderView::ResizeToContents);
	ui.ordersTable->horizontalHeader()->setResizeMode(6,QHeaderView::ResizeToContents);

	ui.ordersTable->setSortingEnabled(true);
	
	calcOrdersTotalValues();
	checkValidOrdersButtons();
}

void QtBitcoinTrader::showErrorMessage(QString message)
{
	static QTime lastMessageTime;
	if(!showingMessage&&lastMessageTime.elapsed()>10000)
	{
		if(!showingMessage)
		{
			showingMessage=true;
			if(debugLevel)logThread->writeLog(exchangeName.toAscii()+" Error: "+message.toAscii());
			lastMessageTime.restart();
			if(message.startsWith("I:>"))
			{
				message.remove(0,3);
				identificationRequired(message);
			}
			else
			QMessageBox::warning(this,julyTr("AUTH_ERROR","%1 Error").arg(exchangeName),message);
			showingMessage=false;
		}
	}
}

void QtBitcoinTrader::identificationRequired(QString message)
{
	if(!message.isEmpty())message.prepend("<br><br>");
		message.prepend(julyTr("TRUNAUTHORIZED","Identification required to access private API.<br>Please enter valid API key and Secret."));

	QMessageBox::warning(this,julyTr("AUTH_ERROR","%1 Error").arg(exchangeName),message);
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

void QtBitcoinTrader::ordersCancelAll()
{
	ordersModel->ordersCancelAll();
}

void QtBitcoinTrader::ordersCancelSelected()
{
	QModelIndexList selectedRows=ui.ordersTable->selectionModel()->selectedRows();
	if(selectedRows.count()==0)return;
	for(int n=0;n<selectedRows.count();n++)
	{
	QByteArray oid=selectedRows.at(n).data(Qt::UserRole).toByteArray();
	if(!oid.isEmpty())cancelOrder(oid);
	}
}

void QtBitcoinTrader::calcButtonClicked()
{
	new FeeCalculator;
}

void QtBitcoinTrader::checkValidSellButtons()
{
	ui.sellThenBuyGroupBox->setEnabled(ui.sellTotalBtc->value()>=minTradeVolume);
	ui.sellBitcoinsButton->setEnabled(ui.sellThenBuyGroupBox->isEnabled()&&/*ui.sellTotalBtc->value()<=getAvailableBTC()&&*/ui.sellTotalBtc->value()>0.0);
}

void QtBitcoinTrader::on_sellPricePerCoinAsMarketPrice_clicked()
{
	ui.sellPricePerCoin->setValue(ui.marketSell->value());
}

void QtBitcoinTrader::on_sellPricePerCoinAsMarketLastPrice_clicked()
{
	ui.sellPricePerCoin->setValue(ui.marketLast->value());
}

void QtBitcoinTrader::sellTotalBtcToSellAllIn()
{
	ui.sellTotalBtc->setValue(getAvailableBTC());
}

void QtBitcoinTrader::sellTotalBtcToSellHalfIn()
{
	ui.sellTotalBtc->setValue(getAvailableBTC()/2.0);
}

void QtBitcoinTrader::setDataPending(bool on)
{
	isDataPending=on;
}

void QtBitcoinTrader::setSoftLagValue(int mseconds)
{
	if(!isDataPending&&mseconds<httpRequestTimeout)mseconds=0;

	static int lastSoftLag=-1;
	if(lastSoftLag==mseconds)return;

	ui.lastUpdate->setValue(mseconds/1000.0);

	static bool lastSoftLagValid=true;
	isValidSoftLag=mseconds<=httpRequestTimeout+httpRequestInterval+200;

	if(isValidSoftLag!=lastSoftLagValid)
	{
		lastSoftLagValid=isValidSoftLag;
		if(!isValidSoftLag)ui.lastUpdate->setStyleSheet("QDoubleSpinBox {background: #ffaaaa;}");
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

void QtBitcoinTrader::checkAllRules()
{
	marketBuyChanged(ui.marketBuy->value());
	marketSellChanged(ui.marketSell->value());
	marketLowChanged(ui.marketLow->value());
	marketHighChanged(ui.marketHigh->value());
	marketLastChanged(ui.marketLast->value());
	ordersLastBuyPriceChanged(ui.ordersLastBuyPrice->value());
	ordersLastSellPriceChanged(ui.ordersLastSellPrice->value());
}

void QtBitcoinTrader::sellTotalBtcToSellChanged(double val)
{
	if(val==0.0)ui.sellTotalBtc->setStyleSheet("QDoubleSpinBox {background: #ffaaaa;}");
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

void QtBitcoinTrader::sellPricePerCoinInUsdChanged(double)
{
	profitSellThanBuyCalc();
	if(!sellLockPricePerCoin)
	{
	sellLockPricePerCoin=true;
	sellTotalBtcToSellChanged(ui.sellTotalBtc->value());
	sellLockPricePerCoin=false;
	}
	ui.sellNextMaxBuyPrice->setValue(ui.sellPricePerCoin->value()*floatFeeDec*floatFeeDec-priceMinimumValue);
	ui.sellNextMaxBuyStep->setValue(ui.sellPricePerCoin->value()-ui.sellNextMaxBuyPrice->value());
	checkValidSellButtons();
}

void QtBitcoinTrader::sellAmountToReceiveChanged(double val)
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

	QMessageBox msgBox(this);
	msgBox.setIcon(QMessageBox::Question);
	msgBox.setWindowTitle(julyTr("MESSAGE_CONFIRM_SELL_TRANSACTION","Please confirm transaction"));
	msgBox.setText(julyTr("MESSAGE_CONFIRM_SELL_TRANSACTION_TEXT","Are you sure to sell %1 at %2 ?<br><br>Note: If total orders amount of your Bitcoins exceeds your balance, %3 will remove this order immediately.").arg(currencyASign+" "+numFromDouble(ui.sellTotalBtc->value(),btcDecimals)).arg(currencyBSign+" "+numFromDouble(ui.sellPricePerCoin->value(),usdDecimals)).arg(exchangeName));
	msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	msgBox.setDefaultButton(QMessageBox::Yes);
	msgBox.setButtonText(QMessageBox::Yes,julyTr("YES","Yes"));
	msgBox.setButtonText(QMessageBox::No,julyTr("NO","No"));
	if(msgBox.exec()!=QMessageBox::Yes)return;

	emit apiSell(ui.sellTotalBtc->value(),ui.sellPricePerCoin->value());
}

void QtBitcoinTrader::buyTotalToSpendInUsdChanged(double val)
{
	if(val==0.0)ui.buyTotalSpend->setStyleSheet("QDoubleSpinBox {background: #ffaaaa;}");
	else ui.buyTotalSpend->setStyleSheet("");

	profitBuyThanSellCalc();
	profitSellThanBuyCalc();

	buyLockTotalBtc=true;
	ui.buyTotalBtc->setValue(val/ui.buyPricePerCoin->value());
	buyLockTotalBtc=false;

	double valueForResult=getFeeForUSDDec(val)/ui.buyPricePerCoin->value();
	valueForResult=getValidDoubleForPercision(valueForResult,btcDecimals,false);
	if(valueForResult==ui.buyTotalBtc->value())valueForResult-=qPow(0.1,btcDecimals);
	ui.buyTotalBtcResult->setValue(valueForResult);

	if(buyLockTotalSpend)return;
	buyLockTotalSpend=true;


	buyLockTotalSpend=false;
	checkValidBuyButtons();
}


void QtBitcoinTrader::buyBtcToBuyChanged(double)
{
	if(buyLockTotalBtc)
	{
		profitSellThanBuyCalc();
		profitBuyThanSellCalc();
		return;
	}
	buyLockTotalBtc=true;

	buyLockTotalSpend=true;
	ui.buyTotalSpend->setValue(ui.buyTotalBtc->value()*ui.buyPricePerCoin->value());
	buyLockTotalSpend=false;

	buyLockTotalBtc=false;
	profitSellThanBuyCalc();
	profitBuyThanSellCalc();
	checkValidBuyButtons();
}

void QtBitcoinTrader::buyPricePerCoinChanged(double)
{
	if(!buyLockPricePerCoin)
	{
	buyLockPricePerCoin=true;
	buyTotalToSpendInUsdChanged(ui.buyTotalSpend->value());
	buyLockPricePerCoin=false;
	}
	ui.buyNextInSellPrice->setValue(ui.buyPricePerCoin->value()*floatFeeInc*floatFeeInc+priceMinimumValue);
	ui.buyNextMinBuyStep->setValue(ui.buyNextInSellPrice->value()-ui.buyPricePerCoin->value());
	checkValidBuyButtons();
}

void QtBitcoinTrader::checkValidBuyButtons()
{
	ui.buyThenSellGroupBox->setEnabled(ui.buyTotalBtc->value()>=minTradeVolume);
	ui.buyBitcoinsButton->setEnabled(ui.buyThenSellGroupBox->isEnabled()&&/*ui.buyTotalSpend->value()<=getAvailableUSD()&&*/ui.buyTotalSpend->value()>0.0);
}

void QtBitcoinTrader::checkValidOrdersButtons()
{
	ui.ordersCancelAllButton->setEnabled(ordersModel->rowCount());
	ui.ordersCancelSelected->setEnabled(ui.ordersTable->selectionModel()->selectedRows().count());
}

void QtBitcoinTrader::checkValidRulesButtons()
{
	int selectedCount=ui.rulesTable->selectionModel()->selectedRows().count();
	ui.ruleEditButton->setEnabled(selectedCount==1);
	ui.ruleRemove->setEnabled(selectedCount);
	rulesEnableDisableMenu->actions().at(0)->setEnabled(selectedCount);
	rulesEnableDisableMenu->actions().at(1)->setEnabled(selectedCount);
	ui.ruleEnableDisable->setEnabled(rulesModel->rowCount());
	ui.ruleRemoveAll->setEnabled(rulesModel->rowCount());
	ui.currencyComboBox->setEnabled(rulesModel->rowCount()==0);
	ui.ruleConcurrentMode->setEnabled(rulesModel->rowCount()==0||rulesModel->allDisabled);
	ui.ruleSequencialMode->setEnabled(rulesModel->rowCount()==0||rulesModel->allDisabled);

	ui.rulesNoMessage->setVisible(rulesModel->rowCount()==0);
	ui.rulesTable->setVisible(rulesModel->rowCount());

	ui.ruleUp->setEnabled(ui.ruleEditButton->isEnabled()&&rulesModel->rowCount()>1);
	ui.ruleDown->setEnabled(ui.ruleEditButton->isEnabled()&&rulesModel->rowCount()>1);
}

void QtBitcoinTrader::ruleUp()
{
	QModelIndexList selectedRows=ui.rulesTable->selectionModel()->selectedRows();
	if(selectedRows.count()==0)return;
	int curRow=selectedRows.first().row();
	if(curRow<1)return;
	rulesModel->moveRowUp(curRow);
}

void QtBitcoinTrader::ruleDown()
{
	QModelIndexList selectedRows=ui.rulesTable->selectionModel()->selectedRows();
	if(selectedRows.count()==0)return;
	int curRow=selectedRows.first().row();
	if(curRow>=rulesModel->rowCount()-1)return;

	rulesModel->moveRowDown(curRow);
}

void QtBitcoinTrader::buyBtcToBuyAllIn()
{
	ui.buyTotalSpend->setValue(getAvailableUSD());
}

void QtBitcoinTrader::buyBtcToBuyHalfIn()
{
	ui.buyTotalSpend->setValue(getAvailableUSD()/2.0);
}

void QtBitcoinTrader::on_buyPriceAsMarketPrice_clicked()
{
	ui.buyPricePerCoin->setValue(ui.marketBuy->value());
}

void QtBitcoinTrader::on_buyPriceAsMarketLastPrice_clicked()
{
	ui.buyPricePerCoin->setValue(ui.marketLast->value());
}

void QtBitcoinTrader::closeEvent(QCloseEvent *event)
{
	if(ui.minimizeOnCloseCheckBox->isChecked())
	{
		event->ignore();
		buttonMinimizeToTray();
		return;
	}
	if(rulesModel->haveWorkingRule())
	{
	QMessageBox msgBox(this);
	msgBox.setIcon(QMessageBox::Question);
	msgBox.setWindowTitle("Qt Bitcoin Trader");
	msgBox.setText(julyTr("CONFIRM_EXIT","Are you sure to close Application?<br>Active rules works only while application is running."));
	msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	msgBox.setDefaultButton(QMessageBox::Yes);
	msgBox.setButtonText(QMessageBox::Yes,julyTr("YES","Yes"));
	msgBox.setButtonText(QMessageBox::No,julyTr("NO","No"));
	if(msgBox.exec()!=QMessageBox::Yes){event->ignore();return;}
	}

	exitApp();
	event->accept();
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

	QMessageBox msgBox(this);
	msgBox.setIcon(QMessageBox::Question);
	msgBox.setWindowTitle(julyTr("MESSAGE_CONFIRM_BUY_TRANSACTION","Please confirm new order"));
	msgBox.setText(julyTr("MESSAGE_CONFIRM_BUY_TRANSACTION_TEXT","Are you sure to buy %1 at %2 ?<br><br>Note: If total orders amount of your funds exceeds your balance, %3 will remove this order immediately.").arg(currencyASign+" "+numFromDouble(ui.buyTotalBtc->value(),btcDecimals)).arg(currencyBSign+" "+numFromDouble(ui.buyPricePerCoin->value(),usdDecimals)).arg(exchangeName));
	msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	msgBox.setDefaultButton(QMessageBox::Yes);
	msgBox.setButtonText(QMessageBox::Yes,julyTr("YES","Yes"));
	msgBox.setButtonText(QMessageBox::No,julyTr("NO","No"));
	if(msgBox.exec()!=QMessageBox::Yes)return;
	
	double btcToBuy=0.0;
	double priceToBuy=ui.buyPricePerCoin->value();
	if(exchangeId==2||exchangeId==3)//Bitstamp exception
		 btcToBuy=ui.buyTotalBtcResult->value();
	else btcToBuy=ui.buyTotalBtc->value();

	double amountWithoutFee=getAvailableUSD()/priceToBuy;
	amountWithoutFee=getValidDoubleForPercision(amountWithoutFee,btcDecimals,false);
	if(amountWithoutFee==btcToBuy)btcToBuy-=qPow(0.1,btcDecimals);

	emit apiBuy(btcToBuy,priceToBuy);
}

void QtBitcoinTrader::copyDonateButton()
{
	QApplication::clipboard()->setText(ui.bitcoinAddress->text());
	QDesktopServices::openUrl(QUrl("bitcoin:"+ui.bitcoinAddress->text()));
	QMessageBox::information(this,"Qt Bitcoin Trader",julyTr("COPY_DONATE_MESSAGE","Bitcoin address copied to clipboard.<br>Thank you for support!"));
}

void QtBitcoinTrader::ruleEditButton()
{
	QModelIndexList selectedRows=ui.rulesTable->selectionModel()->selectedRows();
	if(selectedRows.count()==0)return;
	int curRow=selectedRows.first().row();
	if(curRow<0)return;

	AddRuleWindow addRule;
	addRule.setWindowFlags(windowFlags());
	RuleHolder *curHolder=rulesModel->getRuleHolderByRow(curRow);
	if(curHolder==0||curHolder->invalidHolder)return;
	addRule.fillByRuleHolder(curHolder);
	if(addRule.exec()!=QDialog::Accepted)return;
	RuleHolder updatedRule=addRule.getRuleHolder();
	rulesModel->updateHolderByRow(curRow,&updatedRule);
	saveRulesData();
}

void QtBitcoinTrader::ruleAddButton()
{
	AddRuleWindow addRule;
	addRule.setWindowFlags(windowFlags());
	if(addRule.exec()!=QDialog::Accepted)return;
	RuleHolder *newHolder=new RuleHolder(addRule.getRuleHolder());
	newHolder->setRuleState(1);
	rulesModel->addRule(newHolder);

	ui.rulesNoMessage->setVisible(false);
	ui.rulesTable->setVisible(true);
	checkValidRulesButtons();
	checkAllRules();
	saveRulesData();
}

void QtBitcoinTrader::ruleRemove()
{
	QMessageBox msgBox(this);
	msgBox.setIcon(QMessageBox::Question);
	msgBox.setWindowTitle("Qt Bitcoin Trader");
	msgBox.setText(julyTr("RULE_CONFIRM_REMOVE","Are you sure to remove this rule?"));
	msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	msgBox.setDefaultButton(QMessageBox::Yes);
	msgBox.setButtonText(QMessageBox::Yes,julyTr("YES","Yes"));
	msgBox.setButtonText(QMessageBox::No,julyTr("NO","No"));
	if(msgBox.exec()!=QMessageBox::Yes)return;

	QModelIndexList selectedRows=ui.rulesTable->selectionModel()->selectedRows();
	if(selectedRows.count()==0)return;
	int curRow=selectedRows.first().row();
	rulesModel->removeRuleByRow(curRow);
	checkValidRulesButtons();
	saveRulesData();
}

void QtBitcoinTrader::ruleRemoveAll()
{
	QMessageBox msgBox(this);
	msgBox.setIcon(QMessageBox::Question);
	msgBox.setWindowTitle("Qt Bitcoin Trader");
	msgBox.setText(julyTr("RULE_CONFIRM_REMOVE_ALL","Are you sure to remove all rules?"));
	msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	msgBox.setDefaultButton(QMessageBox::Yes);
	msgBox.setButtonText(QMessageBox::Yes,julyTr("YES","Yes"));
	msgBox.setButtonText(QMessageBox::No,julyTr("NO","No"));
	if(msgBox.exec()!=QMessageBox::Yes)return;

	rulesModel->clear();
	checkValidRulesButtons();
	saveRulesData();
}

void QtBitcoinTrader::beep()
{
#ifdef USE_QTMULTIMEDIA
	static AudioPlayer *player=0;
	if(player==0)player=new AudioPlayer(this);
	if(player->invalidDevice)
	{
		delete player;
		player=0;
	}
	else player->beep();
#endif

#ifdef  Q_OS_WIN
	if(!isActiveWindow())
	{
	FLASHWINFO flashInfo;
	flashInfo.cbSize=sizeof(FLASHWINFO);
	flashInfo.hwnd=this->winId();
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
	if(ui.marketBuy->value()==0.0)return;
	double newValue=ui.accountUSD->value()/ui.marketBuy->value()*floatFeeDec;
	if(newValue!=ui.ruleTotalToBuyValue->value())
	{
		ui.ruleTotalToBuyBSValue->setValue(newValue);
		checkAndExecuteRule(12,ui.ruleTotalToBuyBSValue->value());
	}
}

void QtBitcoinTrader::ruleAmountToReceiveBSValueChanged()
{
	if(ui.marketSell->value()==0.0)return;
	double newValue=ui.accountBTC->value()*ui.marketSell->value()*floatFeeDec;
	if(newValue!=ui.ruleAmountToReceiveBSValue->value())
	{
		ui.ruleAmountToReceiveBSValue->setValue(newValue);
		checkAndExecuteRule(13,ui.ruleAmountToReceiveBSValue->value());
	}
}

void QtBitcoinTrader::accountUSDChanged(double val)
{
	ruleTotalToBuyValueChanged();
	ruleTotalToBuyBSValueChanged();
	checkAndExecuteRule(9,val);
	if(ui.accountUSDBeep->isChecked())beep();
}

void QtBitcoinTrader::accountBTCChanged(double val)
{
	ruleAmountToReceiveValueChanged();
	ruleAmountToReceiveBSValueChanged();
	checkAndExecuteRule(8,val);
	if(ui.accountBTCBeep->isChecked())beep();
}

void QtBitcoinTrader::marketLowChanged(double val)
{
	checkAndExecuteRule(5,val);
	if(ui.marketLowBeep->isChecked()&&lastMarketLowPrice!=val)
	{
		lastMarketLowPrice=val;
		beep();
	}
}

void QtBitcoinTrader::marketHighChanged(double val)
{
	checkAndExecuteRule(4,val);
	if(ui.marketHighBeep->isChecked()&&lastMarketHighPrice!=val)
	{
		lastMarketHighPrice=val;
		beep();
	}
}

void QtBitcoinTrader::marketBuyChanged(double val)
{
	checkAndExecuteRule(2,val);
	ruleTotalToBuyBSValueChanged();
}

void QtBitcoinTrader::marketSellChanged(double val)
{
	checkAndExecuteRule(3,val);
	ruleAmountToReceiveBSValueChanged();
}

void QtBitcoinTrader::marketLastChanged(double val)
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
		case -1: directionChar=downArrow; break;
		case 1: directionChar=upArrow; break;
		default: break;
		}
		static QString titleText;
		titleText=currencyBSign+" "+numFromDouble(val)+" "+directionChar+" "+windowTitleP;
		if(isVisible())setWindowTitle(titleText);
		if(trayIcon&&trayIcon->isVisible())trayIcon->setToolTip(titleText);
	}
}
void QtBitcoinTrader::ordersLastBuyPriceChanged(double val)	{checkAndExecuteRule(6,val);}
void QtBitcoinTrader::ordersLastSellPriceChanged(double val){checkAndExecuteRule(7,val);}

void QtBitcoinTrader::on_ruleConcurrentMode_toggled(bool on)
{
	rulesModel->isConcurrentMode=on;
}

void QtBitcoinTrader::checkAndExecuteRule(int ruleType, double price)
{
	QList<RuleHolder *> achievedHolderList=rulesModel->getAchievedRules(ruleType,price);
	for(int n=0;n<achievedHolderList.count();n++)
	{
		if(!isValidSoftLag){achievedHolderList.at(n)->startWaitingLowLag();continue;}

		double ruleBtc=achievedHolderList.at(n)->getRuleBtc();
		bool isBuying=achievedHolderList.at(n)->isBuying();
		double priceToExec=achievedHolderList.at(n)->getRulePrice();

		if(ruleBtc<0)
		{
			double avBTC=getAvailableBTC();
			double avUSD=getAvailableUSD();
			if(ruleBtc==-1.0)ruleBtc=avBTC;else//"Sell All my BTC"
			if(ruleBtc==-2.0)ruleBtc=avBTC/2.0;else//"Sell Half my BTC"
			if(ruleBtc==-3.0)//"Spend All my Funds"
			{
				ruleBtc=getFeeForUSDDec(avUSD)/priceToExec;
				ruleBtc=getValidDoubleForPercision(ruleBtc,btcDecimals,false);

				double amountWithoutFee=avUSD/priceToExec;
				amountWithoutFee=getValidDoubleForPercision(amountWithoutFee,btcDecimals,false);
				if(amountWithoutFee==ruleBtc)ruleBtc-=qPow(0.1,btcDecimals);
			}
			else
			if(ruleBtc==-4.0)ruleBtc=getFeeForUSDDec(avUSD)/priceToExec/2.0;else//"Spend Half my Funds"
			if(ruleBtc==-5.0)//"Cancel All Orders"
			{
				if(ui.ruleConcurrentMode->isChecked())
				{
					ordersCancelAll();if(ui.ruleBeep->isChecked())beep();continue;
				}
				else
				{
					if(ordersModel->rowCount()==0)
					{
						if(ui.ruleBeep->isChecked())beep();
						rulesModel->setRuleStateByHolder(achievedHolderList.at(n),2);
						return;
					}
					else
					{
						static QTime ordersCancelTime(1,0,0,0);
						if(ordersCancelTime.elapsed()>5000)ordersCancelAll();
						ordersCancelTime.restart();
						continue;
					}
				}
			}
		}

		if(priceToExec<0)
		{
			if(priceToExec==-1.0)priceToExec=ui.marketLast->value();
			if(priceToExec==-2.0)priceToExec=ui.marketBuy->value();
			if(priceToExec==-3.0)priceToExec=ui.marketSell->value();
			if(priceToExec==-4.0)priceToExec=ui.marketHigh->value();
			if(priceToExec==-5.0)priceToExec=ui.marketLow->value();
			if(priceToExec==-6.0)priceToExec=ui.ordersLastBuyPrice->value();
			if(priceToExec==-7.0)priceToExec=ui.ordersLastSellPrice->value();
		}			

		if(ruleBtc>=minTradeVolume)
		{
		if(isBuying)emit apiBuy(ruleBtc,priceToExec);
		else emit apiSell(ruleBtc,priceToExec);
		}
		rulesModel->setRuleStateByHolder(achievedHolderList.at(n),2);
		if(ui.ruleBeep->isChecked())beep();
	}
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
	tabLogOrdersOnTop(ui.tabOrdersLogOnTop->isChecked());
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
	tabRulesOnTop(ui.tabRulesOnTop->isChecked());
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
	tabTradesOnTop(ui.tabTradesOnTop->isChecked());
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
	tabDepthOnTop(ui.tabDepthOnTop->isChecked());
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
	tabChartsOnTop(ui.tabChartsOnTop->isChecked());
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
	double itemPrice=historyModel->getRowPrice(index.row());
	double itemVolume=historyModel->getRowVolume(index.row());
	if(itemPrice==0.0)return;
	int rowType=historyModel->getRowType(index.row());

	if(rowType==1)
	{
		ui.sellPricePerCoin->setValue(itemPrice);
		ui.sellTotalBtc->setValue(itemVolume);
	}
	if(rowType==2)
	{
		ui.buyPricePerCoin->setValue(itemPrice);
		double avUSD=getAvailableUSD();
		double totalBtcValue=avUSD/itemPrice;
		if(totalBtcValue>itemVolume)
		{
			totalBtcValue=itemVolume;
			ui.buyTotalBtc->setValue(getFeeForUSDDec(itemVolume*itemPrice)/itemPrice);
		}
		else ui.buyTotalBtc->setValue(getFeeForUSDDec(avUSD)/itemPrice);

		ui.buyTotalBtc->setValue(totalBtcValue);
	}
}

void QtBitcoinTrader::tradesDoubleClicked(QModelIndex index)
{
	double itemPrice=tradesModel->getRowPrice(index.row());
	double itemVolume=tradesModel->getRowVolume(index.row());
	if(itemPrice==0.0)return;
	if(!tradesModel->getRowType(index.row()))
	{
		ui.buyPricePerCoin->setValue(itemPrice);
		double avUSD=getAvailableUSD();
		double totalBtcValue=avUSD/itemPrice;
		if(totalBtcValue>itemVolume)
		{
			totalBtcValue=itemVolume;
			ui.buyTotalBtc->setValue(getFeeForUSDDec(itemVolume*itemPrice)/itemPrice);
		}
		else ui.buyTotalBtc->setValue(getFeeForUSDDec(avUSD)/itemPrice);

		ui.buyTotalBtc->setValue(totalBtcValue);
	}
	else
	{
		ui.sellPricePerCoin->setValue(itemPrice);
		ui.sellTotalBtc->setValue(qMin(getAvailableBTC(),itemVolume));
	}
}

void QtBitcoinTrader::depthSelectOrder(QModelIndex index, bool isSell)
{
	if(swapedDepth)isSell=!isSell;
	if(isSell)
	{
		int row=index.row();
		if(row<0||depthAsksModel->rowCount()<=row)return;
		double itemPrice=depthAsksModel->rowPrice(row);
		ui.buyPricePerCoin->setValue(itemPrice);
		double itemVolume=0.0;
		if(index.column()==1)itemVolume=depthAsksModel->rowSize(row);
		else itemVolume=depthAsksModel->rowVolume(row);


		double avUSD=getAvailableUSD();
		double totalBtcValue=avUSD/itemPrice;
		if(totalBtcValue>itemVolume)
		{
			totalBtcValue=itemVolume;
			ui.buyTotalBtc->setValue(getFeeForUSDDec(itemVolume*itemPrice)/itemPrice);
		}
		else ui.buyTotalBtc->setValue(getFeeForUSDDec(avUSD)/itemPrice);

		ui.buyTotalBtc->setValue(totalBtcValue);
	}
	else
	{
		int row=index.row();
		if(row<0||depthBidsModel->rowCount()<=row)return;
		ui.sellPricePerCoin->setValue(depthBidsModel->rowPrice(row));
		double itemVolume=0.0;
		if(index.column()==2)itemVolume=depthBidsModel->rowSize(row);
		else itemVolume=depthBidsModel->rowVolume(row);
		ui.sellTotalBtc->setValue(qMin(getAvailableBTC(),itemVolume));
	}
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
	(new TranslationAbout(this))->show();
}

void QtBitcoinTrader::languageChanged()
{
	if(!constructorFinished)return;
	julyTranslator->translateUi(this);
	localDateTimeFormat=julyTr("DATETIME_FORMAT",localDateTimeFormat);
	localTimeFormat=julyTr("TIME_FORMAT",localTimeFormat);
	QStringList ordersLabels;
	ordersLabels<<julyTr("ORDERS_COUNTER","#")<<julyTr("ORDERS_DATE","Date")<<julyTr("ORDERS_TYPE","Type")<<julyTr("ORDERS_STATUS","Status")<<julyTr("ORDERS_AMOUNT","Amount")<<julyTr("ORDERS_PRICE","Price")<<julyTr("ORDERS_TOTAL","Total");
	ordersModel->setHorizontalHeaderLabels(ordersLabels);
	QStringList rulesLabels;
	rulesLabels<<julyTr("RULES_T_STATE","State")<<julyTr("RULES_T_DESCR","Description")<<julyTr("RULES_T_ACTION","Action")<<julyTr("ORDERS_AMOUNT","Amount")<<julyTr("RULES_T_PRICE","Price");
	rulesModel->setHorizontalHeaderLabels(rulesLabels);

	QStringList tradesLabels;
	tradesLabels<<julyTr("ORDERS_DATE","Date")<<julyTr("ORDERS_AMOUNT","Amount")<<julyTr("ORDERS_TYPE","Type")<<julyTr("ORDERS_PRICE","Price");
	historyModel->setHorizontalHeaderLabels(tradesLabels);
	tradesLabels.insert(3,"-");
	tradesModel->setHorizontalHeaderLabels(tradesLabels);

	QStringList depthHeaderLabels;depthHeaderLabels<<julyTr("ORDERS_PRICE","Price")<<julyTr("ORDERS_AMOUNT","Amount")<<julyTr("ORDERS_TOTAL","Total")<<"";
	depthBidsModel->setHorizontalHeaderLabels(depthHeaderLabels);
	depthAsksModel->setHorizontalHeaderLabels(depthHeaderLabels);

	ui.tabOrdersLog->setAccessibleName(julyTr("TAB_ORDERS_LOG","Orders Log"));
	ui.tabOrdersLog->setWindowTitle(ui.tabOrdersLog->accessibleName()+" ["+profileName+"]");
	if(isDetachedLog){julyTranslator->translateUi(ui.tabOrdersLog);fixAllChildButtonsAndLabels(ui.tabOrdersLog);}

	ui.tabRules->setAccessibleName(julyTr("TAB_RULES_FOR_ORDERS","Rules for creating Orders"));
	ui.tabRules->setWindowTitle(ui.tabRules->accessibleName()+" ["+profileName+"]");
	if(isDetachedRules){julyTranslator->translateUi(ui.tabRules);fixAllChildButtonsAndLabels(ui.tabRules);}

	ui.tabLastTrades->setAccessibleName(julyTr("TAB_LAST_TRADES","Last Trades"));
	ui.tabLastTrades->setWindowTitle(ui.tabLastTrades->accessibleName()+" ["+profileName+"]");
	if(isDetachedTrades){julyTranslator->translateUi(ui.tabLastTrades);fixAllChildButtonsAndLabels(ui.tabLastTrades);}

	ui.tabDepth->setAccessibleName(julyTr("TAB_DEPTH","Depth"));
	ui.tabDepth->setWindowTitle(ui.tabDepth->accessibleName()+" ["+profileName+"]");
	if(isDetachedDepth){julyTranslator->translateUi(ui.tabDepth);fixAllChildButtonsAndLabels(ui.tabDepth);}

	ui.tabCharts->setAccessibleName(julyTr("TAB_CHARTS","Charts"));
	ui.tabCharts->setWindowTitle(ui.tabCharts->accessibleName()+" ["+profileName+"]");
	if(isDetachedCharts){julyTranslator->translateUi(ui.tabCharts);fixAllChildButtonsAndLabels(ui.tabCharts);}

	ui.groupBoxAccount->setTitle(julyTr("ACCOUNT_GROUPBOX","%1 Account").arg(exchangeName));

	for(int n=0;n<ui.tabWidget->count();n++)
		ui.tabWidget->setTabText(n,ui.tabWidget->widget(n)->accessibleName());

	QString curCurrencyName=currencyNamesMap->value(currencyAStr,"BITCOINS");
	ui.buyGroupbox->setTitle(julyTr("GROUPBOX_BUY","Buy %1").arg(curCurrencyName));
	ui.sellGroupBox->setTitle(julyTr("GROUPBOX_SELL","Sell %1").arg(curCurrencyName));

	foreach(QToolButton* toolButton, findChildren<QToolButton*>())
		if(toolButton->accessibleDescription()=="TOGGLE_SOUND")
			toolButton->setToolTip(julyTr("TOGGLE_SOUND","Toggle sound notification on value change"));

	ui.comboBoxGroupByPrice->setItemText(0,julyTr("DONT_GROUP","None"));
	ui.comboBoxGroupByPrice->setMinimumWidth(qMax(textFontWidth(ui.comboBoxGroupByPrice->itemText(0))+(int)(ui.comboBoxGroupByPrice->height()*1.1),textFontWidth("50.000")));

	rulesEnableDisableMenu->actions().at(0)->setText(julyTr("RULE_ENABLE","Enable Selected"));
	rulesEnableDisableMenu->actions().at(1)->setText(julyTr("RULE_DISABLE","Disable Selected"));
	rulesEnableDisableMenu->actions().at(3)->setText(julyTr("RULE_ENABLE_ALL","Enable All"));
	rulesEnableDisableMenu->actions().at(4)->setText(julyTr("RULE_DISABLE_ALL","Disable All"));

	fixAllChildButtonsAndLabels(this);
	//emit clearValues();
}

void QtBitcoinTrader::buttonNewWindow()
{
	QProcess::startDetached(QApplication::applicationFilePath(),QStringList());
}

bool QtBitcoinTrader::eventFilter(QObject *obj, QEvent *event)
{
	if(obj!=this)
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

void QtBitcoinTrader::setWindowStaysOnTop(bool on)
{
	hide();
	if(on)setWindowFlags(Qt::Window|Qt::WindowStaysOnTopHint);
	else  setWindowFlags(Qt::Window);

	show();
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

	depthAsksModel->depthUpdateOrders(asks);
	depthBidsModel->depthUpdateOrders(bids);
}

void QtBitcoinTrader::exitApp()
{
	if(trayIcon)trayIcon->hide();
	saveRulesData();

	saveDetachedWindowsSettings();
	iniSettings->setValue("UI/TradesCurrentTab",ui.tabWidget->currentIndex());
	iniSettings->setValue("UI/CloseToTray",ui.minimizeOnCloseCheckBox->isChecked());

	iniSettings->setValue("UI/WindowOnTop",ui.widgetStaysOnTop->isChecked());

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

	saveWindowState(this,"Window");
	iniSettings->sync();

	emit quit();
}

void QtBitcoinTrader::on_depthComboBoxLimitRows_currentIndexChanged(int val)
{
	depthCountLimit=ui.depthComboBoxLimitRows->itemData(val,Qt::UserRole).toInt();
	depthCountLimitStr=QByteArray::number(depthCountLimit);
	iniSettings->setValue("UI/DepthCountLimit",depthCountLimit);
	iniSettings->sync();
	clearDepth();
}

void QtBitcoinTrader::on_comboBoxGroupByPrice_currentIndexChanged(int val)
{
	groupPriceValue=ui.comboBoxGroupByPrice->itemData(val,Qt::UserRole).toDouble();
	iniSettings->setValue("UI/DepthGroupByPrice",groupPriceValue);
	iniSettings->sync();
	clearDepth();
}

void QtBitcoinTrader::on_depthAutoResize_toggled(bool on)
{
	if(on)
	{
	ui.depthAsksTable->horizontalHeader()->showSection(0);
	ui.depthBidsTable->horizontalHeader()->showSection(3);

	ui.depthAsksTable->horizontalHeader()->setResizeMode(0,QHeaderView::Stretch);
	ui.depthAsksTable->horizontalHeader()->setResizeMode(1,QHeaderView::ResizeToContents);
	ui.depthAsksTable->horizontalHeader()->setResizeMode(2,QHeaderView::ResizeToContents);
	ui.depthAsksTable->horizontalHeader()->setResizeMode(3,QHeaderView::ResizeToContents);

	ui.depthBidsTable->horizontalHeader()->setResizeMode(0,QHeaderView::ResizeToContents);
	ui.depthBidsTable->horizontalHeader()->setResizeMode(1,QHeaderView::ResizeToContents);
	ui.depthBidsTable->horizontalHeader()->setResizeMode(2,QHeaderView::ResizeToContents);
	ui.depthBidsTable->horizontalHeader()->setResizeMode(3,QHeaderView::Stretch);
	}
	else
	{
	ui.depthAsksTable->horizontalHeader()->setResizeMode(QHeaderView::Interactive);
	ui.depthBidsTable->horizontalHeader()->setResizeMode(QHeaderView::Interactive);
	ui.depthAsksTable->horizontalHeader()->hideSection(0);
	ui.depthBidsTable->horizontalHeader()->hideSection(3);
	}
}

void QtBitcoinTrader::ruleEnableSelected()
{
	QModelIndexList selectedRows=ui.rulesTable->selectionModel()->selectedRows();
	if(selectedRows.count()==0)return;
	int curRow=selectedRows.first().row();
	rulesModel->setRuleStateByRow(curRow,1);//Enable
	checkValidRulesButtons();
}

void QtBitcoinTrader::ruleDisableSelected()
{
	QModelIndexList selectedRows=ui.rulesTable->selectionModel()->selectedRows();
	if(selectedRows.count()==0)return;
	int curRow=selectedRows.first().row();
	rulesModel->setRuleStateByRow(curRow,0);//Disable
	checkValidRulesButtons();
}

void QtBitcoinTrader::ruleEnableAll()
{
	rulesModel->enableAll();
	checkValidRulesButtons();
}

void QtBitcoinTrader::ruleDisableAll()
{
	rulesModel->disableAll();
	checkValidRulesButtons();
}

double QtBitcoinTrader::getAvailableBTC()
{
	if(exchangeId==2||exchangeId==3)//Bitstamp and BTCChina exception
	{
		return ui.accountBTC->value()-ui.ordersTotalBTC->value();
	}
	return ui.accountBTC->value();
}

double QtBitcoinTrader::getAvailableUSD()
{
	double amountToReturn=0.0;
	if(exchangeId==2||exchangeId==3)//Bitstamp and BTCChina exception
		amountToReturn=ui.accountUSD->value()-ui.ordersTotalUSD->value();
	else amountToReturn=ui.accountUSD->value();
	amountToReturn=getValidDoubleForPercision(amountToReturn,usdDecimals,false);

	if(exchangeSupportsAvailableAmount)amountToReturn=qMin(availableAmount,amountToReturn);
	return amountToReturn;
}
