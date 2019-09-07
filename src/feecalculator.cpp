//  This file is part of Qt Bitcoin Trader
//      https://github.com/JulyIGHOR/QtBitcoinTrader
//  Copyright (C) 2013-2019 July Ighor <julyighor@gmail.com>
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

#include "feecalculator.h"
#include "main.h"
#include "julyspinboxfix.h"
#include "julyspinboxpicker.h"
#include "indicatorengine.h"

FeeCalculator::FeeCalculator()
    : QDialog(),
      locked(true)
{
    ui.setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose, true);
    setWindowFlags(Qt::WindowCloseButtonHint);

    Q_FOREACH (QDoubleSpinBox* spinBox, findChildren<QDoubleSpinBox*>())
        new JulySpinBoxFix(spinBox);

    mainWindow.fixAllCurrencyLabels(this);
    mainWindow.fillAllBtcLabels(this, baseValues.currentPair.currAStr);
    mainWindow.fillAllUsdLabels(this, baseValues.currentPair.currBStr);
    mainWindow.fixDecimals(this);

    ui.feeValue->setValue(mainWindow.ui.accountFee->value());
    ui.feeValue->setDecimals(baseValues.feeDecimals);
    fee = 1 - (ui.feeValue->value() / 100);

    buyPrice = IndicatorEngine::getValue(baseValues.exchangeName + '_' + baseValues.currentPair.symbol + "_Sell");
    ui.buyPrice->setValue(buyPrice);
    double btcVal = mainWindow.getAvailableUSD() / buyPrice;

    if (btcVal < baseValues.currentPair.tradeVolumeMin)
        btcVal = baseValues.currentPair.tradeVolumeMin;

    buy = btcVal;
    ui.buyTotalBtc->setValue(buy);

    ui.buyBtcLayout->addWidget(new JulySpinBoxPicker(ui.buyTotalBtc));
    ui.buyPriceLayout->addWidget(new JulySpinBoxPicker(ui.buyPrice));
    ui.sellPriceLayout->addWidget(new JulySpinBoxPicker(ui.sellPrice));
    ui.feeLayout->addWidget(new JulySpinBoxPicker(ui.feeValue));
    ui.totalPaidLayout->addWidget(new JulySpinBoxPicker(ui.totalPaid));
    ui.receivedLayout->addWidget(new JulySpinBoxPicker(ui.btcReceived));

    setZeroProfitPrice();
    locked = false;
    buyCalc();

    ui.singleInstance->setChecked(mainWindow.feeCalculatorSingleInstance);

    julyTranslator.translateUi(this);

    languageChanged();
    connect(&julyTranslator, SIGNAL(languageChanged()), this, SLOT(languageChanged()));

    ui.groupBox->setStyleSheet("QGroupBox {background: rgba(255,255,255,60); border: 1px solid " +
                               baseValues.appTheme.gray.name() + ";border-radius: 3px;margin-top: 7px;}");

    if (mainWindow.ui.widgetStaysOnTop->isChecked())
        ui.widgetStaysOnTop->setChecked(true);
    else
        setStaysOnTop(false);
}

FeeCalculator::~FeeCalculator()
{
    mainWindow.feeCalculator = nullptr;
}

void FeeCalculator::on_singleInstance_toggled(bool on)
{
    mainWindow.feeCalculatorSingleInstance = on;
}

void FeeCalculator::setStaysOnTop(bool on)
{
    hide();

    if (on)
        setWindowFlags(Qt::WindowCloseButtonHint | Qt::WindowStaysOnTopHint);
    else
        setWindowFlags(Qt::WindowCloseButtonHint);

    show();
}

void FeeCalculator::languageChanged()
{
    julyTranslator.translateUi(this);
    setWindowTitle(julyTr("FEE_CALCULATOR_TITLE", "Calculator"));

    mainWindow.fixAllChildButtonsAndLabels(this);
    QSize minSizeHint = minimumSizeHint();

    if (mainWindow.isValidSize(&minSizeHint))
        setMaximumSize(minimumSizeHint().width() + 200, minimumSizeHint().height());
}

void FeeCalculator::setZeroProfitPrice()
{
    sellPrice = buyPrice / fee / fee;
    locked = true;
    ui.sellPrice->setValue(sellPrice);
    locked = false;
    sellCalc();
}

void FeeCalculator::profitLossChanged(double val)
{
    if (val < 0)
        ui.profitLoss->setStyleSheet("QDoubleSpinBox {background: " + baseValues.appTheme.lightRed.name() + ";}");
    else
        ui.profitLoss->setStyleSheet("QDoubleSpinBox {background: " + baseValues.appTheme.lightGreen.name() + ";}");
}

void FeeCalculator::buyCalc()
{
    buySum = buy * buyPrice;
    buyRez = buy * fee;
    buyFee = buy - buyRez;

    locked = true;
    ui.totalPaid->setValue(buySum);
    ui.buyFee->setValue(buyFee);
    ui.btcReceived->setValue(buyRez);
    locked = false;

    sellCalc();
}

void FeeCalculator::buyBtcChanged(double val)
{
    if (locked)
        return;

    buy = val;
    buyCalc();
}

void FeeCalculator::buyPriceChanged(double val)
{
    if (locked)
        return;

    buyPrice = val;
    buySum = buy * buyPrice;

    locked = true;
    ui.totalPaid->setValue(buySum);
    locked = false;
}

void FeeCalculator::buyTotalPaidChanged(double val)
{
    if (locked)
        return;

    buySum = val;
    buy = buySum / buyPrice;
    buyRez = buy * fee;
    buyFee = buy - buyRez;

    locked = true;
    ui.buyTotalBtc->setValue(buy);
    ui.buyFee->setValue(buyFee);
    ui.btcReceived->setValue(buyRez);
    locked = false;

    sellCalc();
}

void FeeCalculator::buyBtcReceivedChanged(double val)
{
    if (locked)
        return;

    buyRez = val;
    buy = buyRez / fee;
    buySum = buy * buyPrice;
    buyFee = buy - buyRez;

    locked = true;
    ui.buyTotalBtc->setValue(buy);
    ui.totalPaid->setValue(buySum);
    ui.buyFee->setValue(buyFee);
    locked = false;

    sellCalc();
}

void FeeCalculator::sellCalc()
{
    sell = buyRez;
    sellSum = sell * sellPrice;
    sellRez = sellSum * fee;
    sellFee = sellSum - sellRez;

    ui.sellBtc->setValue(sell);
    ui.sellAmount->setValue(sellSum);
    ui.sellFee->setValue(sellFee);
    ui.sellFee->setValue(sellFee);
    ui.sellFiatReceived->setValue(sellRez);
    ui.profitLoss->setValue(sellRez - buySum);
}

void FeeCalculator::sellPriceChanged(double val)
{
    if (locked)
        return;

    sellPrice = val;
    sellCalc();
}

void FeeCalculator::feeChanged(double val)
{
    fee = 1 - (val / 100);
    buyCalc();
}
