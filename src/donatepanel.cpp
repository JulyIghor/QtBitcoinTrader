// Copyright (C) 2014 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form http://qtopentrader.com
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#include "donatepanel.h"
#include "donatewebmoney.h"
#include "donatebtc.h"
#include "main.h"

DonatePanel::DonatePanel(QWidget *parent)
	: QWidget(parent)
{
	parentWindow=parent;
	ui.setupUi(this);
	julyTranslator.translateUi(this);
	setFixedHeight(minimumSizeHint().height());
	setFixedWidth(width());
}

DonatePanel::~DonatePanel()
{

}

QPoint DonatePanel::getFixedPoint(QWidget *buttonWidget, QWidget *menuWidget)
{
	if(parentWindow==0)return QCursor::pos();
	QPoint pointToShow=mapToGlobal(buttonWidget->geometry().bottomLeft());
	if(pointToShow.x()<parentWindow->geometry().x())pointToShow.setX(parentWindow->geometry().x());
	if(pointToShow.y()<parentWindow->geometry().y())pointToShow.setY(parentWindow->geometry().y());
	if(pointToShow.x()>parentWindow->geometry().right()-menuWidget->width())pointToShow.setX(parentWindow->geometry().right()-menuWidget->width());
	if(pointToShow.y()>parentWindow->geometry().bottom()-menuWidget->height())pointToShow.setY(parentWindow->geometry().bottom()-menuWidget->height());
	return pointToShow;
}

void DonatePanel::on_buttonBitcoin_clicked()
{
	DonateBTC *menu=new DonateBTC(ui.buttonBitcoin,true);
	menu->exec(getFixedPoint(ui.buttonBitcoin,menu));
}

void DonatePanel::on_buttonLitecoin_clicked()
{
	DonateBTC *menu=new DonateBTC(ui.buttonLitecoin,false);
	menu->exec(getFixedPoint(ui.buttonLitecoin,menu));
}

void DonatePanel::on_buttonWebMoney_clicked()
{
	DonateWebMoney *menu=new DonateWebMoney(ui.buttonWebMoney);
	menu->exec(getFixedPoint(ui.buttonWebMoney,menu));
}