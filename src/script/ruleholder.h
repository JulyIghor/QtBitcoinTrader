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

#ifndef RULEHOLDER_H
#define RULEHOLDER_H

#include <QString>

struct RuleHolder
{
    RuleHolder();
    QString description;
    bool isTradingRule() const;
    static bool isValidComparation(const QString& text);
    static bool isValidSymbol(const QString& symbol);
    static bool isValidPlusMinus(const QString& plusMinus);
    static bool isValidCode(const QString& code);
    bool isValid() const;

    bool thanAmountPercentChecked;
    bool thanPricePercentChecked;
    bool variableBPercentChecked;
    int thanAmountFeeIndex;
    int thanPriceFeeIndex;
    int thanTypeIndex;
    int variableBFeeIndex;
    int variableBModeIndex;
    double delayMilliseconds;
    double thanAmount;
    double thanPrice;
    double variableBExact;
    QString comparationText;
    QString thanPricePlusMinusText;
    QString thanPriceTypeCode;
    QString thanText;
    QString tradeSymbolCode;
    QString valueASymbolCode;
    QString valueBSymbolCode;
    QString variableACode;
    QString variableBCode;
    QString variableBplusMinus;
    QString variableBSymbolCode;
    QString sayCode;
};

#endif // RULEHOLDER_H
