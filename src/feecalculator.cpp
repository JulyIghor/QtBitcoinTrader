// Copyright (C) 2013 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form https://sourceforge.net/projects/bitcointrader/
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#include "feecalculator.h"
#include "main.h"
#include "julyspinboxfix.h"

#ifdef Q_OS_WIN
#include "qtwin.h"
#endif

FeeCalculator::FeeCalculator()
	: QDialog()
{
	buyPaidLocked=false;
	buyBtcLocked=true;
	buyBtcReceivedLocked=false;
	ui.setupUi(this);
	setWindowFlags(Qt::WindowCloseButtonHint);
	foreach(QDoubleSpinBox* spinBox, findChildren<QDoubleSpinBox*>())new JulySpinBoxFix(spinBox);
#ifdef Q_OS_WIN
	if(QtWin::isCompositionEnabled())QtWin::extendFrameIntoClientArea(this);
#endif
	ui.feeValue->setValue(mainWindow.ui.accountFee->value());

	ui.buyPrice->setValue(mainWindow.ui.marketBuy->value());
	double btcVal=mainWindow.ui.accountUSD->value()/ui.buyPrice->value();
	if(btcVal<0.01)btcVal=1.0;
	ui.buyTotalBtc->setValue(btcVal);

	buyBtcLocked=false;
	buyBtcChanged(ui.buyTotalBtc->value());
	setZeroProfitPrice();
	buyBtcChanged(ui.buyTotalBtc->value());//I'll remove this soon
	setZeroProfitPrice();//and this too

	mainWindow.fillAllBtcLabels(this,currencyAStr);
	mainWindow.fillAllUsdLabels(this,currencyBStr);

	julyTranslator->translateUi(this);

	languageChanged();
	connect(julyTranslator,SIGNAL(languageChanged()),this,SLOT(languageChanged()));

	if(mainWindow.ui.widgetStaysOnTop->isChecked())ui.widgetStaysOnTop->setChecked(true);
	else setStaysOnTop(false);
}

FeeCalculator::~FeeCalculator()
{

}

void FeeCalculator::setStaysOnTop(bool on)
{
	hide();
	if(on)setWindowFlags(Qt::WindowCloseButtonHint|Qt::WindowStaysOnTopHint);
	else  setWindowFlags(Qt::WindowCloseButtonHint);
	show();
#ifdef Q_OS_WIN
	if(QtWin::isCompositionEnabled())QtWin::extendFrameIntoClientArea(this);
#endif
}

void FeeCalculator::languageChanged()
{
	julyTranslator->translateUi(this);
	setWindowTitle(julyTr("FEE_CALCULATOR_TITLE","Calculator"));

	mainWindow.fixAllChildButtonsAndLabels(this);
	QSize minSizeHint=minimumSizeHint();
	if(mainWindow.isValidSize(&minSizeHint))setMaximumSize(minimumSizeHint().width()+200,minimumSizeHint().height());
}

void FeeCalculator::setZeroProfitPrice()
{
	ui.sellPrice->setValue(ui.buyPrice->value()*(1+ui.feeValue->value()/100)*(1+ui.feeValue->value()/100)+0.01);
}

void FeeCalculator::profitLossChanged(double val)
{
	if(val<0)
		ui.profitLoss->setStyleSheet("QDoubleSpinBox {background: #ffaaaa;}");
	else
		ui.profitLoss->setStyleSheet("QDoubleSpinBox {background: #aaffaa;}");
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

