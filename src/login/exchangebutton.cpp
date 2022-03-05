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

#include "exchangebutton.h"

ExchangeButton::ExchangeButton(const QString& logo,
                               const QString& currencies,
                               const QString& url,
                               qint32 num,
                               FeaturedExchangesDialog* toParrentForm) :
    QWidget()
{
    parrentForm = toParrentForm;
    ui.setupUi(this);
    exchangeNum = num;

    setLogo(logo);
    setCurrencies(currencies);
    setURL(url);
    setFixedSize(208, 124);
}

ExchangeButton::~ExchangeButton()
{
}

void ExchangeButton::mouseReleaseEvent(QMouseEvent* event)
{
    event->accept();

    if (event->button() == Qt::LeftButton)
    {
        foreach (ExchangeButton* exchangeButton, this->parent()->findChildren<ExchangeButton*>())
        {
            exchangeButton->ui.widgetButton->setStyleSheet("#widgetButton{}");
        }

        ui.widgetButton->setStyleSheet(
            "#widgetButton{background:rgba(0,0,0,7%);border-left:1px solid #333;border-top:1px solid #333;border-right:1px solid #fff;border-bottom:1px solid #fff}");
        parrentForm->selectExchange(exchangeNum);
    }
}

void ExchangeButton::mouseDoubleClickEvent(QMouseEvent* event)
{
    event->ignore();
    QCoreApplication::processEvents();
    parrentForm->selectExchange(exchangeNum);
    parrentForm->on_okButton_clicked();
}

void ExchangeButton::setLogo(const QString& logo)
{
    ui.imageLabel->setPixmap(QPixmap(":/Resources/Exchanges/Logos/" + logo));
    ui.imageLabel->setFixedSize(180, 50);
}

void ExchangeButton::setCurrencies(const QString& currencies)
{
    ui.currenciesLabel->setToolTip(currencies);
    ui.currenciesLabel->setText(currencies.length() > 28 ? currencies.left(28) + " ..." : currencies);
    ui.currenciesLabel->setWordWrap(true);
    ui.currenciesLabel->setFixedSize(180, 14);
}

void ExchangeButton::setURL(const QString& url)
{
    ui.urlLabel->setText("<a href=\"" + url + "\" title=\"" + url + "\">" + url + "</a>");
    ui.urlLabel->setToolTip(url);
    ui.urlLabel->setOpenExternalLinks(true);
    ui.urlLabel->setFixedSize(ui.urlLabel->minimumSizeHint());
}
