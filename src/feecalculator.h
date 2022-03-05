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

#ifndef FEECALCULATOR_H
#define FEECALCULATOR_H

#include "ui_feecalculator.h"
#include <QDialog>

class TraderSpinBox;

class FeeCalculator : public QDialog
{
    Q_OBJECT

public:
    FeeCalculator();
    ~FeeCalculator();

public slots:
    void languageChanged();

private slots:
    void on_singleInstance_toggled(bool);
    void setStaysOnTop(bool);
    void setZeroProfitPrice();
    void profitLossChanged(double);
    void buyBtcChanged(double);
    void buyPriceChanged(double);
    void buyTotalPaidChanged(double);
    void buyBtcReceivedChanged(double);
    void sellPriceChanged(double);
    void feeChanged(double);

private:
    void setupWidgets();

private:
    TraderSpinBox* buyTotalBtcBox;
    TraderSpinBox* buyPriceBox;
    TraderSpinBox* totalPaidBox;
    TraderSpinBox* btcReceivedBox;
    TraderSpinBox* sellPriceBox;
    TraderSpinBox* feeValueBox;

    double buy;
    double buyPrice;
    double buySum;
    double buyFee;
    double buyRez;
    double sell;
    double sellPrice;
    double sellSum;
    double sellFee;
    double sellRez;
    double fee;
    bool locked;
    Ui::FeeCalculator ui;

    void buyCalc();
    void sellCalc();
};

#endif // FEECALCULATOR_H
