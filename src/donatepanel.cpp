//  This file is part of Qt Bitcion Trader
//      https://github.com/JulyIGHOR/QtBitcoinTrader
//  Copyright (C) 2013-2014 July IGHOR <julyighor@gmail.com>
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