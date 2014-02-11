// Copyright (C) 2014 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form http://qtopentrader.com
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#include "feecalculator.h"
#include "main.h"
#include "julyspinboxfix.h"
#include "julyspinboxpicker.h"

FeeCalculator::FeeCalculator()
	: QDialog()
{
	buyPaidLocked=false;
	buyBtcLocked=true;
	buyBtcReceivedLocked=false;
	ui.setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose,true);
	setWindowFlags(Qt::WindowCloseButtonHint);
	foreach(QDoubleSpinBox* spinBox, findChildren<QDoubleSpinBox*>())new JulySpinBoxFix(spinBox);

	mainWindow.fillAllBtcLabels(this,baseValues.currentPair.currAStr);
	mainWindow.fillAllUsdLabels(this,baseValues.currentPair.currBStr);
	mainWindow.fixDecimals(this);

	ui.feeValue->setValue(mainWindow.ui.accountFee->value());

	ui.buyPrice->setValue(mainWindow.ui.marketBid->value());
	double btcVal=mainWindow.getAvailableUSD()/ui.buyPrice->value();
	if(btcVal<baseValues.currentPair.tradeVolumeMin)btcVal=baseValues.currentPair.tradeVolumeMin;
	ui.buyTotalBtc->setValue(btcVal);

	ui.buyBtcLayout->addWidget(new JulySpinBoxPicker(ui.buyTotalBtc));
	ui.buyPriceLayout->addWidget(new JulySpinBoxPicker(ui.buyPrice));
	ui.sellPriceLayout->addWidget(new JulySpinBoxPicker(ui.sellPrice));
	ui.feeLayout->addWidget(new JulySpinBoxPicker(ui.feeValue));
	ui.totalPaidLayout->addWidget(new JulySpinBoxPicker(ui.totalPaid));
	ui.receivedLayout->addWidget(new JulySpinBoxPicker(ui.btcReceived));

	buyBtcLocked=false;
	buyBtcChanged(ui.buyTotalBtc->value());
	setZeroProfitPrice();
	buyBtcChanged(ui.buyTotalBtc->value());//I'll remove this soon
	setZeroProfitPrice();//and this too

	ui.singleInstance->setChecked(mainWindow.feeCalculatorSingleInstance);

	julyTranslator.translateUi(this);

	languageChanged();
	connect(&julyTranslator,SIGNAL(languageChanged()),this,SLOT(languageChanged()));

	ui.groupBox->setStyleSheet("QGroupBox {background: rgba(255,255,255,60); border: 1px solid "+baseValues.appTheme.gray.name()+";border-radius: 3px;margin-top: 7px;}");

	if(mainWindow.ui.widgetStaysOnTop->isChecked())ui.widgetStaysOnTop->setChecked(true);
	else setStaysOnTop(false);
}

FeeCalculator::~FeeCalculator()
{
	if(mainWindow.feeCalculatorSingleInstance)mainWindow.feeCalculator=0;
}

void FeeCalculator::on_singleInstance_toggled(bool on)
{
	mainWindow.feeCalculatorSingleInstance=on;
}

void FeeCalculator::setStaysOnTop(bool on)
{
	hide();
	if(on)setWindowFlags(Qt::WindowCloseButtonHint|Qt::WindowStaysOnTopHint);
	else  setWindowFlags(Qt::WindowCloseButtonHint);
	show();
}

void FeeCalculator::languageChanged()
{
	julyTranslator.translateUi(this);
	setWindowTitle(julyTr("FEE_CALCULATOR_TITLE","Calculator"));

	mainWindow.fixAllChildButtonsAndLabels(this);
	QSize minSizeHint=minimumSizeHint();
	if(mainWindow.isValidSize(&minSizeHint))setMaximumSize(minimumSizeHint().width()+200,minimumSizeHint().height());
}

void FeeCalculator::setZeroProfitPrice()
{
	ui.sellPrice->setValue(ui.buyPrice->value()*mainWindow.floatFeeInc*mainWindow.floatFeeInc);
}

void FeeCalculator::profitLossChanged(double val)
{
	if(val<0)
		ui.profitLoss->setStyleSheet("QDoubleSpinBox {background: "+baseValues.appTheme.lightRed.name()+";}");
	else
		ui.profitLoss->setStyleSheet("QDoubleSpinBox {background: "+baseValues.appTheme.lightGreen.name()+";}");
}

void FeeCalculator::buyBtcChanged(double)
{
	ui.buyFee->setValue(ui.buyTotalBtc->value()*ui.feeValue->value()/100);

	if(buyBtcLocked)return;
	buyBtcLocked=true;

	buyPaidLocked=true;
	ui.totalPaid->setValue(ui.buyTotalBtc->value()*ui.buyPrice->value());
	buyPaidLocked=false;

	buyBtcReceivedLocked=true;
	ui.btcReceived->setValue(ui.buyTotalBtc->value()-ui.buyFee->value());
	buyBtcReceivedLocked=false;

	buyBtcLocked=false;
}

void FeeCalculator::buyPriceChanged(double)
{
	buyBtcChanged(ui.buyTotalBtc->value());
}

void FeeCalculator::buyTotalPaidChanged(double)
{
	ui.buyFee->setValue(ui.buyTotalBtc->value()*ui.feeValue->value()/100);
	ui.profitLoss->setValue(ui.sellFiatReceived->value()-ui.totalPaid->value());
	if(buyPaidLocked)return;
	buyPaidLocked=true;

	buyBtcLocked=true;
	ui.buyTotalBtc->setValue(ui.totalPaid->value()/ui.buyPrice->value());
	buyBtcLocked=false;

	buyBtcReceivedLocked=true;
	ui.btcReceived->setValue(ui.buyTotalBtc->value()-ui.buyFee->value());
	buyBtcReceivedLocked=false;

	buyPaidLocked=false;
}

void FeeCalculator::buyBtcReceivedChanged(double val)
{
	ui.sellAmount->setValue(val*ui.sellPrice->value());
	ui.sellFiatReceived->setValue(ui.sellAmount->value()-ui.sellFee->value());
	ui.sellBtc->setValue(val);

	if(buyBtcReceivedLocked)return;
	buyBtcReceivedLocked=true;
	
	buyBtcLocked=true;
	ui.buyTotalBtc->setValue(ui.btcReceived->value()+ui.feeValue->value()*ui.btcReceived->value()/100);
	buyBtcLocked=false;

	buyPaidLocked=true;
	ui.totalPaid->setValue(ui.buyTotalBtc->value()*ui.buyPrice->value());
	buyPaidLocked=false;

	buyBtcReceivedLocked=false;
}

void FeeCalculator::sellPriceChanged(double)
{
	buyBtcReceivedChanged(ui.sellBtc->value());
}

void FeeCalculator::sellAmountChanged(double)
{
	ui.sellFee->setValue(ui.sellAmount->value()*ui.feeValue->value()/100);
}

void FeeCalculator::sellFiatReceived(double)
{
	ui.profitLoss->setValue(ui.sellFiatReceived->value()-ui.totalPaid->value());
}

void FeeCalculator::feeChanged(double)
{
	buyBtcChanged(ui.buyTotalBtc->value());
}

