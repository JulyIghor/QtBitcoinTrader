// Copyright (C) 2014 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form http://qtopentrader.com
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#include "addrulewindow.h"
#include "main.h"
#include "julyspinboxfix.h"
#include <QMessageBox>
#include <QTimer>
#include "julyspinboxpicker.h"
#include "exchange.h"
#include <QFileDialog>
#include "thisfeatureunderdevelopment.h"

AddRuleWindow::AddRuleWindow(RuleWidget *parent)
	: QDialog()
{
	wasTrailingVisible=false;
	changingSound=false;
	parentRuleGroup=parent;
	ui.setupUi(this);
	ui.buttonSaveRule->setVisible(false);
	ui.thanValue->setValue(mainWindow.ui.marketLast->value());
	ui.exactPriceValue->setValue(mainWindow.ui.marketLast->value());
	ui.btcValue->setValue(mainWindow.getAvailableBTC());

	setWindowTitle("Qt Bitcoin Trader ["+parentRuleGroup->windowTitle()+"]");
	setWindowFlags(Qt::WindowCloseButtonHint);
	amountChanged();

	mainWindow.fillAllUsdLabels(this,baseValues.currentPair.currBStr);

	mainWindow.fillAllBtcLabels(this,baseValues.currentPair.currAStr);

	new JulySpinBoxFix(ui.thanValue);
	new JulySpinBoxFix(ui.thanValuePercentage);
	new JulySpinBoxFix(ui.btcValue);
	new JulySpinBoxFix(ui.exactPriceValue);

	ui.thanValueBack->layout()->addWidget(new JulySpinBoxPicker(ui.thanValue));
	ui.valuePercentageBackExecPrice->layout()->addWidget(new JulySpinBoxPicker(ui.valueCheckValuePercentage));
	ui.percentageLayout->addWidget(new JulySpinBoxPicker(ui.thanValuePercentage,0,0.01));
	ui.valuePercentageBack100->layout()->addWidget(new JulySpinBoxPicker(ui.valueValuePercentage,0,0.01));
	ui.exactPricePickerLayout->addWidget(new JulySpinBoxPicker(ui.exactPriceValue));
	ui.amountLayoutPicker->addWidget(new JulySpinBoxPicker(ui.btcValue));

	on_exactPrice_toggled(false);

	QString styleGroupBox="QGroupBox {background: "+baseValues.appTheme.white.name()+"; border-radius: 8px; border: 1px solid "+baseValues.appTheme.gray.name()+";QCheckBox {color: "+baseValues.appTheme.black.name()+";}}";
	ui.ifGroupBox->setStyleSheet(styleGroupBox);
	ui.goesGroupBox->setStyleSheet(styleGroupBox);
	ui.reazonGroupBox->setStyleSheet(styleGroupBox);
	ui.atGroupBox->setStyleSheet(styleGroupBox);

	ui.checkMarketHigh->setVisible(currentExchange->exchangeTickerSupportsHiLowPrices);
	ui.checkMarketLow->setVisible(currentExchange->exchangeTickerSupportsHiLowPrices);
	ui.checkMarketHigh_2->setVisible(currentExchange->exchangeTickerSupportsHiLowPrices);
	ui.checkMarketLow_2->setVisible(currentExchange->exchangeTickerSupportsHiLowPrices);

	foreach(QPushButton* pushButtons, findChildren<QPushButton*>())pushButtons->setMinimumWidth(textFontWidth(pushButtons->text())+10);

	foreach(QCheckBox* checkBoxes, findChildren<QCheckBox*>())checkBoxes->setMinimumWidth(textFontWidth(checkBoxes->text())+10);

	foreach(QRadioButton* checkBoxes, ui.ifGroupBox->findChildren<QRadioButton*>())
		connect(checkBoxes,SIGNAL(toggled(bool)),this,SLOT(ifChanged(bool)));

	foreach(QRadioButton* checkBoxes, ui.reazonGroupBox->findChildren<QRadioButton*>())
		connect(checkBoxes,SIGNAL(toggled(bool)),this,SLOT(amountChanged()));

	foreach(QRadioButton* checkBoxes, ui.goesGroupBox->findChildren<QRadioButton*>())
		connect(checkBoxes,SIGNAL(toggled(bool)),this,SLOT(thanTypeChanged()));

	ui.valuePercentageBack->setVisible(false);
	ui.exactPricePanel->setVisible(false);
	ui.priceBtcIcon->setVisible(false);
		ui.pricePercentIcon->setVisible(false);
	languageChanged();
	thanTypeChanged();

	setMaximumSize(width()+400,minimumSizeHint().height()+30);
	resize(width()+100,maximumHeight());
	fixWidth();

	connect(&(baseValues_->julyTranslator_),SIGNAL(languageChanged()),this,SLOT(languageChanged()));
	QTimer::singleShot(100,this,SLOT(checkToEnableButtons()));
}

AddRuleWindow::~AddRuleWindow()
{

}

void AddRuleWindow::on_valueCheckValuePercentageToZero_clicked()
{
	ui.valueCheckValuePercentage->setValue(0.0);
}

void AddRuleWindow::on_exactPrice_toggled(bool on)
{
	ui.exactPricePanel->setVisible(on);
	ui.checkCheckUsePercentageValue->setVisible(!on);
	ui.valuePercentageBackExecPrice->setVisible(ui.checkCheckUsePercentageValue->isChecked()&&!on);
}

void AddRuleWindow::on_checkCheckUsePercentageValue_toggled(bool on)
{
	ui.valuePercentageBackExecPrice->setVisible(on);
}

void AddRuleWindow::on_valueValuePercentage100_clicked()
{
	ui.valueValuePercentage->setValue(100.0);
}
void AddRuleWindow::on_valueValuePercentage2_3_clicked()
{
	ui.valueValuePercentage->setValue(200.0/3.0);
}

void AddRuleWindow::on_valueValuePercentage50_clicked()
{
	ui.valueValuePercentage->setValue(50.0);
}

void AddRuleWindow::on_valueValuePercentage1_3_clicked()
{
	ui.valueValuePercentage->setValue(100.0/3.0);
}

void AddRuleWindow::on_thanValuePercentageToZero_clicked()
{
	ui.thanValuePercentage->setValue(0.0);
}

void AddRuleWindow::thanTypeChanged()
{
	ui.thanValueBack->setVisible(ui.thanExectValueCheckBox->isChecked());
	ui.thanPercentageBack->setVisible(ui.thanPercentageCheckBox->isChecked());
	ui.checkTrailingEnabled->setVisible(ui.checkGoesBelow->isChecked()||ui.checkGoesAbove->isChecked());

	if(ui.checkGoesChanged->isChecked())
	{
		ui.thanValue->setValue(1.0);
		ui.thanValuePercentage->setValue(1.0);
		ui.thanValue->setMinimum(qPow(0.1,ui.thanValue->decimals()));
		ui.thanValuePercentage->setMinimum(qPow(0.1,ui.thanValuePercentage->decimals()));
	}
	else
	{
		ui.thanValue->setMinimum(0.0);
		ui.thanValuePercentage->setMinimum(0.0);
		if(ui.thanExectValueCheckBox->isChecked())
			ui.thanValue->setValue(getSelectedIfValue()*(100.0+ui.thanValuePercentage->value())/100.0);
		else
			ui.thanValuePercentage->setValue(ui.thanValue->value()*100.0/getSelectedIfValue()-100.0);
	}
	checkToEnableButtons();
}

void AddRuleWindow::fixWidth()
{
	if(minimumSizeHint().width()<width())
		setMinimumWidth(minimumSizeHint().width());
}

void AddRuleWindow::on_fillFromBuyPanel_clicked()
{
	ui.checkBuyAmount->setChecked(true);
	ui.exactPrice->setChecked(true);
	ui.exactPriceValue->setValue(mainWindow.ui.buyPricePerCoin->value());
	ui.btcValue->setValue(mainWindow.ui.buyTotalBtc->value());
}

void AddRuleWindow::on_fillFromSellPanel_clicked()
{
	ui.checkSellAmount->setChecked(true);
	ui.exactPrice->setChecked(true);
	ui.exactPriceValue->setValue(mainWindow.ui.sellPricePerCoin->value());
	ui.btcValue->setValue(mainWindow.ui.sellTotalBtc->value());
}

void AddRuleWindow::checkToEnableButtons()
{
	fixWidth();
	//int currentThanType=thanType();
	int currentRuleIfType=getRuleIfType();

	bool leftCorrect=ui.checkImmediatelyExecution->isChecked();
		 leftCorrect=leftCorrect||ui.checkGoesChanged->isChecked();
		 leftCorrect=leftCorrect||currentRuleIfType>7;
		 leftCorrect=leftCorrect||currentRuleIfType<8&&(ui.thanExectValueCheckBox->isChecked()&&ui.thanValue->value()>=baseValues.currentPair.tradePriceMin||ui.thanExectValueCheckBox->isChecked());
		 leftCorrect=leftCorrect||ui.thanPercentageCheckBox->isChecked();

	bool midCorrect=!ui.midWidget->isEnabled()||!ui.btcValue->isVisible()||ui.btcValue->isVisible()&&ui.btcValue->value()>=baseValues.currentPair.tradeVolumeMin;
	bool rightCorrect=!ui.sellBack->isEnabled()||!ui.exactPriceValue->isVisible()||ui.exactPriceValue->isVisible()&&ui.exactPriceValue->value()>baseValues.currentPair.tradePriceMin;
	
	bool canBeEnabled=leftCorrect&&midCorrect&&rightCorrect;
	ui.buttonAddRule->setEnabled(canBeEnabled);
	ui.buttonSaveRule->setEnabled(canBeEnabled);
}

void AddRuleWindow::languageChanged()
{
	julyTranslator.translateUi(this);

	QString curA(baseValues.currentPair.currAStr);
	QString curB(baseValues.currentPair.currBStr);

	ui.checkBtcBalance->setText(julyTr("IF_BALANCE","%1 Balance").arg(curA));
	ui.checkUsdBalance->setText(julyTr("IF_BALANCE","%1 Balance").arg(curB));

	ui.checkSellAllIn->setText(julyTr("SELL_ALL_MY","Sell All my %1").arg(curA));
	ui.checkSellHalfIn->setText(julyTr("SELL_HALF_MY","Sell Half my %1").arg(curA));

	ui.checkBuyAllIn->setText(julyTr("SPEND_ALL_MY","Spend All my %1").arg(curB));
	ui.checkBuyHalfIn->setText(julyTr("SPEND_HALF_MY","Spend Half my %1").arg(curB));

	mainWindow.fixAllChildButtonsAndLabels(this);
}

void AddRuleWindow::buttonAddRule()
{
	wasTrailingVisible=ui.checkTrailingEnabled->isVisible();
	if(!ui.ruleIsEnabled->isChecked()||parentRuleGroup->ui.ruleSequencialMode->isChecked()&&parentRuleGroup->rulesModel->rowCount()>0||checkIsValidRule())accept();
	else QMessageBox::warning(this,windowTitle(),julyTr("INVALID_RULE_CHECK","This rule will be executed instantly.<br>This means that you make a mistake.<br>Please check values you entered."));
}

double AddRuleWindow::getSelectedIfValue()
{
	if(ui.checkLastPrice->isChecked())return mainWindow.ui.marketLast->value();
	if(ui.checkMarketBuy->isChecked())return mainWindow.ui.marketBid->value();
	if(ui.checkMarketSell->isChecked())return mainWindow.ui.marketAsk->value();
	if(ui.checkMarketHigh->isChecked())return mainWindow.ui.marketHigh->value();
	if(ui.checkMarketLow->isChecked())return mainWindow.ui.marketLow->value();
	if(ui.checkOrdersLastBuyPrice->isChecked())return mainWindow.ui.ordersLastBuyPrice->value();
	if(ui.checkOrdersLastSellPrice->isChecked())return mainWindow.ui.ordersLastSellPrice->value();
	if(ui.checkBtcBalance->isChecked())return mainWindow.getAvailableBTC();
	if(ui.checkUsdBalance->isChecked())return mainWindow.getAvailableUSD();
	if(ui.checkTotalToBuy->isChecked())return mainWindow.ui.ruleTotalToBuyValue->value();
	if(ui.checkAmountToReceive->isChecked())return mainWindow.ui.ruleAmountToReceiveValue->value();
	if(ui.checkTotalToBuyBS->isChecked())return mainWindow.ui.ruleTotalToBuyBSValue->value();
	if(ui.checkAmountToReceiveBS->isChecked())return mainWindow.ui.ruleAmountToReceiveBSValue->value();
	if(ui.check10MinVolume->isChecked())return mainWindow.ui.tradesVolume5m->value();
	if(ui.check10MinBuySellVolume->isChecked())return mainWindow.ui.tradesBidsPrecent->value();
	return 0.0;
}

bool AddRuleWindow::checkIsValidRule()
{
	checkToEnableButtons();
	if(getRuleHolder().isAchieved(getSelectedIfValue()))return false;
	return true;
}

void AddRuleWindow::amountChanged()
{
	if(ui.checkSendEmail->isChecked()||ui.checkExecuteCommand->isChecked())
	{
		ui.checkPlaySound->setChecked(true);
		ThisFeatureUnderDevelopment featureNotAvailable;
		featureNotAvailable.setWindowFlags(windowFlags());
		 featureNotAvailable.setWindowFlags(Qt::WindowCloseButtonHint);
		julyTranslator.translateUi(&featureNotAvailable);
		featureNotAvailable.setFixedSize(380,featureNotAvailable.minimumSizeHint().height());
		featureNotAvailable.exec();
		return;
	}

	bool btcVisible=ui.checkSellAmount->isChecked()||ui.checkBuyAmount->isChecked();
	ui.amountBack->setVisible(btcVisible);
	ui.labelSellAll->setVisible(ui.checkSellAllIn->isChecked());
	ui.labelSellHalf->setVisible(ui.checkSellHalfIn->isChecked());
	ui.labelSpendAll->setVisible(ui.checkBuyAllIn->isChecked());
	ui.labelSpendHalf->setVisible(ui.checkBuyHalfIn->isChecked());
	ui.labelCancelAllOrders->setVisible(ui.checkCancelAllOrders->isChecked());
	ui.labelEnableAllRules->setVisible(ui.checkEnableAllRules->isChecked());
	ui.labelDisableAllRules->setVisible(ui.checkDisableAllRules->isChecked());
	ui.labelEnableAllRulesForSpecGroup->setVisible(ui.checkEnableAllRulesForSpecGroup->isChecked());
	ui.labelDisableAllRulesForSpecGroup->setVisible(ui.checkDisableAllRulesForSpecGroup->isChecked());
	ui.labelPlaySound->setVisible(ui.checkPlaySound->isChecked());
	ui.labelPlaySoundWav->setVisible(ui.checkPlaySoundWav->isChecked());

	bool lastObjectVisible=ui.checkSellAllIn->isChecked()||ui.checkBuyAllIn->isChecked();
	ui.checkUsePercentageValue->setVisible(lastObjectVisible);
	ui.valuePercentageBack->setVisible(ui.checkUsePercentageValue->isChecked()&&lastObjectVisible);

	ui.enableAllRulesInGroupIdValueBack->setVisible(ui.checkEnableAllRulesForSpecGroup->isChecked());
	ui.disableAllRulesInGroupIdValueBack->setVisible(ui.checkDisableAllRulesForSpecGroup->isChecked());

	lastObjectVisible=ui.checkSellAmount->isChecked()||ui.checkBuyAmount->isChecked()||ui.checkSellAllIn->isChecked()||ui.checkSellHalfIn->isChecked()||ui.checkBuyAllIn->isChecked()||ui.checkBuyHalfIn->isChecked();
	ui.sellBack->setEnabled(lastObjectVisible);

	checkToEnableButtons();

	if(changingSound==false&&ui.checkPlaySoundWav->isChecked()&&(sound.isEmpty()||sound=="Beep"))
	{
		static QString lastOpenFile=mainWindow.iniSettings->value("UI/LastOpenWavFile","").toString();
		sound=QFileDialog::getOpenFileName(this, julyTr("OPEN_SOUND_FILE","Open WAV file"), lastOpenFile, "Sound File (*.wav)");
		if(sound.length()<4||!QFile::exists(sound))
			ui.checkPlaySound->setChecked(true);
		else
		{
			lastOpenFile=sound;
			ui.checkPlaySoundWav->setToolTip(sound);
			mainWindow.iniSettings->setValue("UI/LastOpenWavFile",lastOpenFile);
			mainWindow.iniSettings->sync();
		}
	}
	else
	if(!ui.checkPlaySoundWav->isChecked())sound.clear();

	ui.playSoundBeep->setVisible(ui.checkPlaySound->isChecked());
	ui.playSound->setVisible(ui.checkPlaySoundWav->isChecked());
}

void AddRuleWindow::fillByRuleHolder(RuleHolder *holder)
{
	ui.addNewRuleGroupbox->setTitle(julyTr("EDIT_RULE","Edit rule"));

	ui.buttonAddRule->setVisible(false);
	ui.buttonSaveRule->setVisible(true);

	switch(holder->getRulePriceType())
	{
	case 1: ui.checkLastPrice->setChecked(true);break;
	case 2: ui.checkMarketBuy->setChecked(true);break;
	case 3: ui.checkMarketSell->setChecked(true);break;
	case 4: ui.checkMarketHigh->setChecked(true);break;
	case 5: ui.checkMarketLow->setChecked(true);break;
	case 6: ui.checkOrdersLastBuyPrice->setChecked(true);break;
	case 7: ui.checkOrdersLastSellPrice->setChecked(true);break;
	case 8: ui.checkBtcBalance->setChecked(true);break;
	case 9: ui.checkUsdBalance->setChecked(true);break;
	case 10: ui.checkTotalToBuy->setChecked(true);break;
	case 11: ui.checkAmountToReceive->setChecked(true);break;
	case 12: ui.checkTotalToBuyBS->setChecked(true);break;
	case 13: ui.checkAmountToReceiveBS->setChecked(true);break;
	case 14: ui.check10MinVolume->setChecked(true);break;
	case 15: ui.check10MinBuySellVolume->setChecked(true);break;
	case 16: ui.checkImmediatelyExecution->setChecked(true);break;
	default: break;
	}

	switch(holder->getRuleMoreLessEqual())
	{
	case -2: ui.checkGoesChanged->setChecked(true); break;
	case -1: ui.checkGoesBelow->setChecked(true); break;
	case 0: ui.checkEqual->setChecked(true); break;
	case 1: ui.checkGoesAbove->setChecked(true); break;
	default: break;
	}

	double ruleCheckPercentage=holder->getRuleCheckPricePercentage();
	if(ruleCheckPercentage==0.0)
		ui.thanValue->setValue(holder->getRuleCheckPrice());
	else
	{
		ui.thanPercentageCheckBox->setChecked(true);
		ui.thanValuePercentage->setValue(ruleCheckPercentage*100.0);
	}

	double fillRuleBtc=holder->getRuleBtc();
	if(fillRuleBtc>-1.0)
	{
		ui.btcValue->setValue(fillRuleBtc);
		if(holder->isBuying())ui.checkBuyAmount->setChecked(true);
		else ui.checkSellAmount->setChecked(true);
	}
	else
	{
		if(fillRuleBtc==-1.0)ui.checkSellAllIn->setChecked(true);
		if(fillRuleBtc==-2.0)ui.checkSellHalfIn->setChecked(true);
		if(fillRuleBtc==-3.0)ui.checkBuyAllIn->setChecked(true);
		if(fillRuleBtc==-4.0)ui.checkBuyHalfIn->setChecked(true);
		if(fillRuleBtc==-5.0)ui.checkCancelAllOrders->setChecked(true);
		if(fillRuleBtc==-6.0)ui.checkEnableAllRules->setChecked(true);
		if(fillRuleBtc==-7.0)ui.checkDisableAllRules->setChecked(true);
		if(fillRuleBtc==-8.0)
		{
			ui.checkEnableAllRulesForSpecGroup->setChecked(true);
			ui.enableAllRulesInGroupIdValue->setValue(holder->getRuleGroupId());
		}
		if(fillRuleBtc==-9.0)
		{
			ui.checkDisableAllRulesForSpecGroup->setChecked(true);
			ui.disableAllRulesInGroupIdValue->setValue(holder->getRuleGroupId());
		}
		if(fillRuleBtc==-10.0)ui.checkPlaySound->setChecked(true);
		if(fillRuleBtc==-11.0)
		{
			changingSound=true;
			ui.checkPlaySoundWav->setChecked(true);
			sound=holder->ruleWavFile;
			ui.checkPlaySoundWav->setToolTip(sound);
			changingSound=false;
		}
	}

	ui.valueValuePercentage->setValue(holder->getRuleAmountPercentage()*100.0);
	ui.checkUsePercentageValue->setChecked(ui.valueValuePercentage->value()!=100.0);

	double fillRulePrice=holder->getRuleExecutePrice();
	if(fillRulePrice>-1.0)
	{
		ui.exactPrice->setChecked(true);
		ui.exactPriceValue->setValue(fillRulePrice);
	}
	else
	{
	if(fillRulePrice==-1.0)ui.checkLastPrice_2->setChecked(true);
	if(fillRulePrice==-2.0)ui.checkMarketBuy_2->setChecked(true);
	if(fillRulePrice==-3.0)ui.checkMarketSell_2->setChecked(true);
	if(fillRulePrice==-4.0)ui.checkMarketHigh_2->setChecked(true);
	if(fillRulePrice==-5.0)ui.checkMarketLow_2->setChecked(true);
	if(fillRulePrice==-6.0)ui.checkOrdersLastBuyPrice_2->setChecked(true);
	if(fillRulePrice==-7.0)ui.checkOrdersLastSellPrice_2->setChecked(true);
	}

	qDebug()<<holder->isTrailingEnabled();
	ui.checkTrailingEnabled->setChecked(holder->isTrailingEnabled());

	ui.ruleIsEnabled->setChecked(holder->getRuleState()==1);

	double ruleExecutePricePercentage=holder->getRuleExecutePricePercentage()*100.0;
	if(fillRulePrice<=-1.0&&ruleExecutePricePercentage>0.0)
	{
		ui.checkCheckUsePercentageValue->setChecked(true);
		ui.valueCheckValuePercentage->setValue(ruleExecutePricePercentage);
	}
	checkToEnableButtons();
}


void AddRuleWindow::on_playSoundBeep_clicked()
{
	mainWindow.beep(true);
}

void AddRuleWindow::on_playSound_clicked()
{
	mainWindow.playWav(sound,true);
}

int AddRuleWindow::getRuleIfType()
{
	if(ui.checkLastPrice->isChecked())return 1;
	if(ui.checkMarketBuy->isChecked())return 2;
	if(ui.checkMarketSell->isChecked())return 3;
	if(ui.checkMarketHigh->isChecked())return 4;
	if(ui.checkMarketLow->isChecked())return 5;
	if(ui.checkOrdersLastBuyPrice->isChecked())return 6;
	if(ui.checkOrdersLastSellPrice->isChecked())return 7;
	if(ui.checkBtcBalance->isChecked())return 8;
	if(ui.checkUsdBalance->isChecked())return 9;
	if(ui.checkTotalToBuy->isChecked())return 10;
	if(ui.checkAmountToReceive->isChecked())return 11;
	if(ui.checkTotalToBuyBS->isChecked())return 12;
	if(ui.checkAmountToReceiveBS->isChecked())return 13;
	if(ui.check10MinVolume->isChecked())return 14;
	if(ui.check10MinBuySellVolume->isChecked())return 15;
	if(ui.checkImmediatelyExecution->isChecked())return 16;
	return 1;
}

RuleHolder AddRuleWindow::getRuleHolder()
{
	double btcValue=ui.btcValue->value();
	bool isBuying=ui.checkBuyAmount->isChecked();
	if(ui.checkSellAllIn->isChecked())btcValue=-1.0;
	if(ui.checkSellHalfIn->isChecked())btcValue=-2.0;
	if(ui.checkBuyAllIn->isChecked()){isBuying=true;btcValue=-3.0;}
	if(ui.checkBuyHalfIn->isChecked()){isBuying=true;btcValue=-4.0;}
	if(ui.checkCancelAllOrders->isChecked())btcValue=-5.0;
	if(ui.checkEnableAllRules->isChecked())btcValue=-6.0;
	if(ui.checkDisableAllRules->isChecked())btcValue=-7.0;

	if(ui.checkEnableAllRulesForSpecGroup->isChecked())btcValue=-8.0;
	if(ui.checkDisableAllRulesForSpecGroup->isChecked())btcValue=-9.0;
	if(ui.checkPlaySound->isChecked())btcValue=-10.0;
	if(ui.checkPlaySoundWav->isChecked())btcValue=-11.0;

	double ruleExecutePrice=ui.exactPriceValue->value();

	if(ui.checkLastPrice_2->isChecked())ruleExecutePrice=-1.0;
	if(ui.checkMarketBuy_2->isChecked())ruleExecutePrice=-2.0;
	if(ui.checkMarketSell_2->isChecked())ruleExecutePrice=-3.0;
	if(ui.checkMarketHigh_2->isChecked())ruleExecutePrice=-4.0;
	if(ui.checkMarketLow_2->isChecked())ruleExecutePrice=-5.0;
	if(ui.checkOrdersLastBuyPrice_2->isChecked())ruleExecutePrice=-6.0;
	if(ui.checkOrdersLastSellPrice_2->isChecked())ruleExecutePrice=-7.0;

	int moreLessEqual=0;
	if(ui.checkGoesAbove->isChecked())moreLessEqual=1;
	if(ui.checkGoesBelow->isChecked())moreLessEqual=-1;
	if(ui.checkGoesChanged->isChecked())moreLessEqual=-2;

	int gID=0;
	if(ui.checkEnableAllRulesForSpecGroup->isChecked())
		gID=ui.enableAllRulesInGroupIdValue->value();
	else
	if(ui.checkDisableAllRulesForSpecGroup->isChecked())
		gID=ui.disableAllRulesInGroupIdValue->value();

	double rulePricePercentage=0.0;
	if(ui.thanPercentageCheckBox->isChecked())
		rulePricePercentage=ui.thanValuePercentage->value();

	double execPricePercentage=0.0;
	if(!ui.exactPrice->isChecked()&&ui.checkCheckUsePercentageValue->isChecked())execPricePercentage=ui.valueCheckValuePercentage->value();

	double amountPercentage=100.0;
	if(ui.checkSellAllIn->isChecked()||ui.checkBuyAllIn->isChecked())amountPercentage=ui.valueValuePercentage->value();

	if(ui.checkPlaySound->isChecked())sound="Beep";

	return RuleHolder(moreLessEqual, ui.thanValue->value(), btcValue, isBuying, ruleExecutePrice, getRuleIfType(), rulePricePercentage, execPricePercentage, amountPercentage, gID, sound, wasTrailingVisible&&ui.checkTrailingEnabled->isChecked(), ui.ruleIsEnabled->isChecked());
}

int AddRuleWindow::thanType()
{
	if(ui.check10MinVolume->isChecked()||ui.checkBtcBalance->isChecked()||ui.checkTotalToBuy->isChecked()||ui.checkTotalToBuyBS->isChecked())return 1;//BTC Volume
	if(ui.checkUsdBalance->isChecked()||ui.checkAmountToReceive->isChecked()||ui.checkAmountToReceiveBS->isChecked())return 2;//USD Volume
	if(ui.check10MinBuySellVolume->isChecked())return 3;//Percentage
	return 0; //Price
}

void AddRuleWindow::ifChanged(bool on)
{
	if(!on)return;

	bool equalEnabled=!ui.checkTotalToBuy->isChecked()&&!ui.checkTotalToBuyBS->isChecked()&&!ui.checkAmountToReceive->isChecked()&&!ui.checkAmountToReceiveBS->isChecked()&&!ui.check10MinVolume->isChecked()&&!ui.check10MinBuySellVolume->isChecked();
	if(!equalEnabled&&ui.checkEqual->isChecked())ui.checkGoesAbove->setChecked(true);
	ui.checkEqual->setEnabled(equalEnabled);
	switch(thanType())
	{
	case 1:
		{
		ui.thanValue->setDecimals(baseValues.currentPair.currADecimals);
		if(ui.checkBtcBalance->isChecked())ui.thanValue->setValue(mainWindow.getAvailableBTC());
		if(ui.checkTotalToBuy->isChecked())ui.thanValue->setValue(mainWindow.ui.ruleTotalToBuyValue->value());
		if(ui.checkTotalToBuyBS->isChecked())ui.thanValue->setValue(mainWindow.ui.ruleTotalToBuyBSValue->value());

		if(ui.check10MinVolume->isChecked())ui.thanValue->setValue(mainWindow.ui.tradesVolume5m->value());
		if(ui.check10MinBuySellVolume->isChecked())ui.thanValue->setValue(mainWindow.ui.tradesBidsPrecent->value());

		ui.priceBtcIcon->setVisible(true);
		ui.priceUsdIcon->setVisible(false);
		ui.pricePercentIcon->setVisible(false);
		ui.thanValue->setMaximum(999999999.0);
		}
		break;
		case 2:
		{
		ui.thanValue->setDecimals(baseValues.currentPair.currBDecimals);
		if(ui.checkUsdBalance->isChecked())ui.thanValue->setValue(mainWindow.getAvailableUSD());
		if(ui.checkAmountToReceive->isChecked())ui.thanValue->setValue(mainWindow.ui.ruleAmountToReceiveValue->value());
		if(ui.checkAmountToReceiveBS->isChecked())ui.thanValue->setValue(mainWindow.ui.ruleAmountToReceiveBSValue->value());

		if(ui.check10MinVolume->isChecked())ui.thanValue->setValue(mainWindow.ui.tradesVolume5m->value());
		if(ui.check10MinBuySellVolume->isChecked())ui.thanValue->setValue(mainWindow.ui.tradesBidsPrecent->value());

		ui.priceBtcIcon->setVisible(false);
		ui.priceUsdIcon->setVisible(true);
		ui.pricePercentIcon->setVisible(false);
		ui.thanValue->setMaximum(999999999.0);
		}
		break;
		case 3:
		{
			ui.priceBtcIcon->setVisible(false);
			ui.priceUsdIcon->setVisible(false);
			ui.pricePercentIcon->setVisible(true);
			ui.thanValue->setMaximum(100.0);
		}
		break;
		default:
		{
			ui.thanValue->setDecimals(baseValues.currentPair.priceDecimals);
			ui.thanValue->setValue(mainWindow.ui.marketLast->value());

			ui.priceBtcIcon->setVisible(false);
			ui.priceUsdIcon->setVisible(true);
			ui.pricePercentIcon->setVisible(false);
			ui.thanValue->setMaximum(999999999.0);
		}
		break;
	}
	setMinimumSize(minimumSizeHint());
	checkToEnableButtons();
	thanTypeChanged();
}