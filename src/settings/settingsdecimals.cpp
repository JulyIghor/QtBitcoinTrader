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

#include "settingsdecimals.h"
#include "main.h"

SettingsDecimals::SettingsDecimals(QWidget* parent) : QWidget(parent)
{
    ui.setupUi(this);
    decimalsSettings = new QSettings(appDataDir + "/QtBitcoinTrader.cfg", QSettings::IniFormat);

    loadDecimals();

    ui.revertChangesButton->setEnabled(false);
    ui.saveButton->setEnabled(false);
}

SettingsDecimals::~SettingsDecimals()
{
    delete decimalsSettings;
}

void SettingsDecimals::loadDecimals()
{
    /*ui.amountMyTransactionsSpinBox->setValue(baseValues.decimalsAmountMyTransactions);
    ui.amountMyTransactionsSpinBox->setMaximum(baseValues.currentPair.currADecimals);
    ui.priceMyTransactionsSpinBox->setValue(baseValues.decimalsPriceMyTransactions);
    ui.priceMyTransactionsSpinBox->setMaximum(8);
    ui.totalMyTransactionsSpinBox->setValue(baseValues.decimalsTotalMyTransactions);
    ui.totalMyTransactionsSpinBox->setMaximum(baseValues.currentPair.currADecimals);

    ui.amountOrderBookSpinBox->setValue(baseValues.decimalsAmountOrderBook);
    ui.amountOrderBookSpinBox->setMaximum(baseValues.currentPair.currADecimals);
    ui.priceOrderBookSpinBox->setValue(baseValues.decimalsPriceOrderBook);
    ui.priceOrderBookSpinBox->setMaximum(baseValues.currentPair.priceDecimals);
    ui.totalOrderBookSpinBox->setValue(baseValues.decimalsTotalOrderBook);
    ui.totalOrderBookSpinBox->setMaximum(baseValues.currentPair.currADecimals);

    ui.amountLastTradesSpinBox->setValue(baseValues.decimalsAmountLastTrades);
    ui.amountLastTradesSpinBox->setMaximum(baseValues.currentPair.currADecimals);
    ui.priceLastTradesSpinBox->setValue(baseValues.decimalsPriceLastTrades);
    ui.priceLastTradesSpinBox->setMaximum(baseValues.currentPair.priceDecimals);
    ui.totalLastTradesSpinBox->setValue(baseValues.decimalsTotalLastTrades);
    ui.totalLastTradesSpinBox->setMaximum(baseValues.currentPair.currADecimals);*/

    ui.amountMyTransactionsSpinBox->setValue(baseValues.decimalsAmountMyTransactions);
    ui.amountMyTransactionsSpinBox->setMaximum(8);
    ui.priceMyTransactionsSpinBox->setValue(baseValues.decimalsPriceMyTransactions);
    ui.priceMyTransactionsSpinBox->setMaximum(8);
    ui.totalMyTransactionsSpinBox->setValue(baseValues.decimalsTotalMyTransactions);
    ui.totalMyTransactionsSpinBox->setMaximum(8);

    ui.amountOrderBookSpinBox->setValue(baseValues.decimalsAmountOrderBook);
    ui.amountOrderBookSpinBox->setMaximum(8);
    ui.priceOrderBookSpinBox->setValue(baseValues.decimalsPriceOrderBook);
    ui.priceOrderBookSpinBox->setMaximum(8);
    ui.totalOrderBookSpinBox->setValue(baseValues.decimalsTotalOrderBook);
    ui.totalOrderBookSpinBox->setMaximum(8);

    ui.amountLastTradesSpinBox->setValue(baseValues.decimalsAmountLastTrades);
    ui.amountLastTradesSpinBox->setMaximum(8);
    ui.priceLastTradesSpinBox->setValue(baseValues.decimalsPriceLastTrades);
    ui.priceLastTradesSpinBox->setMaximum(8);
    ui.totalLastTradesSpinBox->setValue(baseValues.decimalsTotalLastTrades);
    ui.totalLastTradesSpinBox->setMaximum(8);
}

void SettingsDecimals::saveDecimals()
{
    decimalsSettings->beginGroup("Decimals");
    decimalsSettings->setValue("AmountMyTransactions", ui.amountMyTransactionsSpinBox->value());
    decimalsSettings->setValue("PriceMyTransactions", ui.priceMyTransactionsSpinBox->value());
    decimalsSettings->setValue("TotalMyTransactions", ui.totalMyTransactionsSpinBox->value());
    decimalsSettings->setValue("AmountOrderBook", ui.amountOrderBookSpinBox->value());
    decimalsSettings->setValue("PriceOrderBook", ui.priceOrderBookSpinBox->value());
    decimalsSettings->setValue("TotalOrderBook", ui.totalOrderBookSpinBox->value());
    decimalsSettings->setValue("AmountLastTrades", ui.amountLastTradesSpinBox->value());
    decimalsSettings->setValue("PriceLastTrades", ui.priceLastTradesSpinBox->value());
    decimalsSettings->setValue("TotalLastTrades", ui.totalLastTradesSpinBox->value());
    decimalsSettings->endGroup();
}

void SettingsDecimals::activateDecimals()
{
    baseValues.decimalsAmountMyTransactions = ui.amountMyTransactionsSpinBox->value();
    baseValues.decimalsPriceMyTransactions = ui.priceMyTransactionsSpinBox->value();
    baseValues.decimalsTotalMyTransactions = ui.totalMyTransactionsSpinBox->value();
    baseValues.decimalsAmountOrderBook = ui.amountOrderBookSpinBox->value();
    baseValues.decimalsPriceOrderBook = ui.priceOrderBookSpinBox->value();
    baseValues.decimalsTotalOrderBook = ui.totalOrderBookSpinBox->value();
    baseValues.decimalsAmountLastTrades = ui.amountLastTradesSpinBox->value();
    baseValues.decimalsPriceLastTrades = ui.priceLastTradesSpinBox->value();
    baseValues.decimalsTotalLastTrades = ui.totalLastTradesSpinBox->value();
}

void SettingsDecimals::on_saveButton_clicked()
{
    saveDecimals();
    activateDecimals();

    ui.revertChangesButton->setEnabled(false);
    ui.saveButton->setEnabled(false);
}

void SettingsDecimals::on_revertChangesButton_clicked()
{
    loadDecimals();

    ui.revertChangesButton->setEnabled(false);
    ui.saveButton->setEnabled(false);
}

void SettingsDecimals::on_restoreDefaultsButton_clicked()
{
    /*ui.amountMyTransactionsSpinBox->setValue(baseValues.currentPair.currADecimals);
    ui.priceMyTransactionsSpinBox->setValue(8);
    ui.totalMyTransactionsSpinBox->setValue(baseValues.currentPair.currADecimals);
    ui.amountOrderBookSpinBox->setValue(baseValues.currentPair.currADecimals);
    ui.priceOrderBookSpinBox->setValue(baseValues.currentPair.priceDecimals);
    ui.totalOrderBookSpinBox->setValue(baseValues.currentPair.currADecimals);
    ui.amountLastTradesSpinBox->setValue(baseValues.currentPair.currADecimals);
    ui.priceLastTradesSpinBox->setValue(baseValues.currentPair.priceDecimals);
    ui.totalLastTradesSpinBox->setValue(baseValues.currentPair.currADecimals);*/
    ui.amountMyTransactionsSpinBox->setValue(8);
    ui.priceMyTransactionsSpinBox->setValue(8);
    ui.totalMyTransactionsSpinBox->setValue(8);
    ui.amountOrderBookSpinBox->setValue(8);
    ui.priceOrderBookSpinBox->setValue(8);
    ui.totalOrderBookSpinBox->setValue(8);
    ui.amountLastTradesSpinBox->setValue(8);
    ui.priceLastTradesSpinBox->setValue(8);
    ui.totalLastTradesSpinBox->setValue(8);

    ui.restoreDefaultsButton->setEnabled(false);
}

void SettingsDecimals::anyValueChanged()
{
    if (!ui.revertChangesButton->isEnabled())
        ui.revertChangesButton->setEnabled(true);

    if (!ui.restoreDefaultsButton->isEnabled())
        ui.restoreDefaultsButton->setEnabled(true);

    if (!ui.saveButton->isEnabled())
        ui.saveButton->setEnabled(true);
}
