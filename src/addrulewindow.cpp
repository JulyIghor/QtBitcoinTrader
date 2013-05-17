//Created by July IGHOR
//Feel free to contact me: julyighor@gmail.com
//Bitcoin Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc

#include "addrulewindow.h"
#include "main.h"
#include "tempwindow.h"
//#include "qtwin.h"
AddRuleWindow::AddRuleWindow(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	ui.thanValue->setValue(mainWindow_->ui.marketLast->value());
	ui.btcValue->setValue(mainWindow_->ui.accountBTC->value());
	resize(minimumSizeHint());
	setMinimumSize(size());
	setMaximumSize(width()+100,height());
	//if(QtWin::isCompositionEnabled())QtWin::extendFrameIntoClientArea(this);
	setWindowFlags(Qt::WindowCloseButtonHint);
	setStyleSheet("QLabel {color: black;} QDoubleSpinBox {background: white;} QTextEdit {background: white;}");
}

AddRuleWindow::~AddRuleWindow()
{

}

void AddRuleWindow::buttonAddRule()
{
	TempWindow(this).exec();
	//accept();
}

QString AddRuleWindow::getIfString()
{
	if(ui.checkMarketBuy->isChecked())return ui.checkMarketBuy->text();
	if(ui.checkMarketSell->isChecked())return ui.checkMarketSell->text();
	if(ui.checkMarketHigh->isChecked())return ui.checkMarketHigh->text();
	if(ui.checkMarketLow->isChecked())return ui.checkMarketLow->text();
	return ui.checkLastPrice->text();
}

QString AddRuleWindow::getGoesString()
{
	if(ui.checkGoesAbove->isChecked())return ui.checkGoesAbove->text();
	return ui.checkGoesBelow->text();
}

QString AddRuleWindow::getPriceString()
{
	return currencySign+" "+ui.thanValue->text();
}

QString AddRuleWindow::getSellBuyString()
{
	if(ui.checkBuyAmount->isChecked())return ui.checkBuyAmount->text();
	if(ui.checkSellAllIn->isChecked())return ui.checkSellAllIn->text();
	if(ui.checkBuyAllIn->isChecked())return ui.checkBuyAllIn->text();
	if(ui.checkSellHalfIn->isChecked())return ui.checkSellHalfIn->text();
	if(ui.checkBuyHalfIn->isChecked())return ui.checkBuyHalfIn->text();
	return ui.checkSellAmount->text();
}

QString AddRuleWindow::getBitcoinsString()
{
	return ui.btcLabel->text()+" "+ui.btcValue->text();
}