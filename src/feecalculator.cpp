//Created by July IGHOR
//http://trader.uax.co
//Bitcoin Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc

#include "feecalculator.h"
#include "main.h"

#ifdef Q_OS_WIN
#include "qtwin.h"
#endif

FeeCalculator::FeeCalculator()
	: QDialog()
{
	buyPaidLocked=false;
	buyBtcLocked=false;
	buyBtcReceivedLocked=false;
	ui.setupUi(this);
	setWindowFlags(Qt::WindowCloseButtonHint);
	setFixedSize(minimumSizeHint());
	setStyleSheet("QLabel {color: black;} QDoubleSpinBox {background: white;} QTextEdit {background: white;}");
	setWindowIcon(QIcon(":/Resources/QtBitcoinTrader.png"));
#ifdef Q_OS_WIN
	if(QtWin::isCompositionEnabled())QtWin::extendFrameIntoClientArea(this);
#endif
	ui.buyPrice->setValue(mainWindow_->ui.marketBuy->value());
	double btcVal=mainWindow_->ui.accountUSD->value()/ui.buyPrice->value();
	if(btcVal<0.01)btcVal=1.0;
	ui.buyTotalBtc->setValue(btcVal);

	ui.feeValue->setValue(mainWindow_->ui.accountFee->value());
	ui.sellPrice->setValue((ui.buyPrice->value()+ui.buyPrice->value()*ui.feeValue->value()/100)*(1+ui.feeValue->value()/100)+0.01);
	
	QPixmap btcPixmap("://Resources/BTC.png");
	ui.btcLabel1->setPixmap(btcPixmap);
	ui.btcLabel2->setPixmap(btcPixmap);
	ui.btcLabel3->setPixmap(btcPixmap);
	ui.btcLabel4->setPixmap(btcPixmap);

	QPixmap curPix(":/Resources/"+currencyStr+".png");
	ui.usdLabel1->setPixmap(curPix);
	ui.usdLabel2->setPixmap(curPix);
	ui.usdLabel3->setPixmap(curPix);
	ui.usdLabel4->setPixmap(curPix);
	ui.usdLabel5->setPixmap(curPix);
	ui.usdLabel6->setPixmap(curPix);
	ui.usdLabel7->setPixmap(curPix);
}

FeeCalculator::~FeeCalculator()
{

}

void FeeCalculator::profitLossChanged(double val)
{
	if(val<0)
		ui.profitLoss->setStyleSheet("QLabel {background: #ffaaaa;}");
	else
		ui.profitLoss->setStyleSheet("");
}

void FeeCalculator::buyBtcChanged(double val)
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

