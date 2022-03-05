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

#include "feecalculator.h"
#include "indicatorengine.h"
#include "julyspinboxfix.h"
#include "julyspinboxpicker.h"
#include "main.h"
#include "utils/traderspinbox.h"

FeeCalculator::FeeCalculator() :
    QDialog(),
    buyTotalBtcBox(new TraderSpinBox(this)),
    buyPriceBox(new TraderSpinBox(this)),
    totalPaidBox(new TraderSpinBox(this)),
    btcReceivedBox(new TraderSpinBox(this)),
    sellPriceBox(new TraderSpinBox(this)),
    feeValueBox(new TraderSpinBox(this)),
    locked(true)
{
    ui.setupUi(this);
    setupWidgets();
    setAttribute(Qt::WA_DeleteOnClose, true);
    setWindowFlags(Qt::WindowCloseButtonHint);

    for (QDoubleSpinBox* spinBox : findChildren<QDoubleSpinBox*>())
        new JulySpinBoxFix(spinBox);

    mainWindow.fixAllCurrencyLabels(this);
    mainWindow.fillAllBtcLabels(this, baseValues.currentPair.currAStr);
    mainWindow.fillAllUsdLabels(this, baseValues.currentPair.currBStr);
    mainWindow.fixDecimals(this);

    feeValueBox->setValue(mainWindow.ui.accountFee->value());
    feeValueBox->setDecimals(baseValues.feeDecimals);
    fee = 1 - (feeValueBox->value() / 100);

    buyPrice = IndicatorEngine::getValue(baseValues.exchangeName + '_' + baseValues.currentPair.symbol + "_Sell");
    buyPriceBox->setValue(buyPrice);
    double btcVal = mainWindow.getAvailableUSD() / buyPrice;

    if (btcVal < baseValues.currentPair.tradeVolumeMin)
        btcVal = baseValues.currentPair.tradeVolumeMin;

    buy = btcVal;
    buyTotalBtcBox->setValue(buy);

    ui.buyBtcLayout->addWidget(new JulySpinBoxPicker(buyTotalBtcBox));
    ui.buyPriceLayout->addWidget(new JulySpinBoxPicker(buyPriceBox));
    ui.sellPriceLayout->addWidget(new JulySpinBoxPicker(sellPriceBox));
    ui.feeLayout->addWidget(new JulySpinBoxPicker(feeValueBox));
    ui.totalPaidLayout->addWidget(new JulySpinBoxPicker(totalPaidBox));
    ui.receivedLayout->addWidget(new JulySpinBoxPicker(btcReceivedBox));

    setZeroProfitPrice();
    locked = false;
    buyCalc();

    ui.singleInstance->setChecked(mainWindow.feeCalculatorSingleInstance);

    julyTranslator.translateUi(this);

    languageChanged();
    connect(&julyTranslator, &JulyTranslator::languageChanged, this, &FeeCalculator::languageChanged);

    ui.groupBox->setStyleSheet("QGroupBox {background: rgba(255,255,255,60); border: 1px solid " + baseValues.appTheme.gray.name() +
                               ";border-radius: 3px;margin-top: 7px;}");

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
    sellPriceBox->setValue(sellPrice);
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
    totalPaidBox->setValue(buySum);
    ui.buyFee->setValue(buyFee);
    btcReceivedBox->setValue(buyRez);
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
    totalPaidBox->setValue(buySum);
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
    buyTotalBtcBox->setValue(buy);
    ui.buyFee->setValue(buyFee);
    btcReceivedBox->setValue(buyRez);
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
    buyTotalBtcBox->setValue(buy);
    totalPaidBox->setValue(buySum);
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

void FeeCalculator::setupWidgets()
{
    buyTotalBtcBox->setAccessibleName("BTC");
    buyTotalBtcBox->setDecimals(8);
    buyTotalBtcBox->setMaximum(999999999.0);
    ui.buyBtcLayout->addWidget(buyTotalBtcBox);

    buyPriceBox->setAccessibleName("PRICE");
    buyPriceBox->setDecimals(5);
    buyPriceBox->setMinimum(0.01);
    buyPriceBox->setMaximum(999999999.0);
    ui.buyPriceLayout->addWidget(buyPriceBox);

    totalPaidBox->setAccessibleName("USD");
    totalPaidBox->setDecimals(5);
    totalPaidBox->setMaximum(999999999.0);
    ui.totalPaidLayout->addWidget(totalPaidBox);

    btcReceivedBox->setAccessibleName("BTC");
    btcReceivedBox->setDecimals(8);
    btcReceivedBox->setMaximum(999999999.0);
    ui.receivedLayout->addWidget(btcReceivedBox);

    sellPriceBox->setAccessibleName("PRICE");
    sellPriceBox->setDecimals(5);
    sellPriceBox->setMinimum(0.00001);
    sellPriceBox->setMaximum(999999999.0);
    ui.sellPriceLayout->addWidget(sellPriceBox);

    feeValueBox->setMinimumWidth(0);
    feeValueBox->setAlignment(Qt::AlignCenter | Qt::AlignHCenter);
    feeValueBox->setSuffix(" %");
    feeValueBox->setDecimals(2);
    feeValueBox->setMaximum(10.0);
    feeValueBox->setValue(0.6);
    ui.feeLayout->addWidget(feeValueBox);

    connect(buyTotalBtcBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &FeeCalculator::buyBtcChanged);
    connect(buyPriceBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &FeeCalculator::buyPriceChanged);
    connect(totalPaidBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &FeeCalculator::buyTotalPaidChanged);
    connect(btcReceivedBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &FeeCalculator::buyBtcReceivedChanged);
    connect(sellPriceBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &FeeCalculator::sellPriceChanged);
    connect(feeValueBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &FeeCalculator::feeChanged);
}
