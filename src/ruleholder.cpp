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

#include "ruleholder.h"
#include "main.h"

RuleHolder::RuleHolder()
{
    thanAmountPercentChecked=false;
    thanPricePercentChecked=false;
    variableBPercentChecked=false;
    thanAmountFeeIndex=-1;
    thanPriceFeeIndex=-1;
    thanTypeIndex=-1;
    variableBFeeIndex=-1;
    variableBModeIndex=-1;
    thanAmount=0.0;
    thanPrice=0.0;
    variableBExact=0.0;
}

bool RuleHolder::isValidComparation(QString &text)
{
    return text==QLatin1String("=")||text==QLatin1String("<")||text==QLatin1String(">")||text==QLatin1String("<=")||text==QLatin1String(">=");
}

bool RuleHolder::isValidSymbol(QString &symbol)
{
    CurrencyPairItem pairItem;
    pairItem=baseValues.currencyPairMap.value(symbol,pairItem);
    if(pairItem.symbol.isEmpty())return false;
    return true;
}

bool RuleHolder::isValidPlusMinus(QString &plusMinus)
{
    return plusMinus==QLatin1String("+")||plusMinus==QLatin1String("-");
}

bool RuleHolder::isValidCode(QString &code)
{
    return code==QLatin1String("EXACT")||code==QLatin1String("IMMEDIATELY")||mainWindow.indicatorsMap.value(code,0)!=0;
}

bool RuleHolder::isValid()
{
    return thanAmountFeeIndex>-1&&thanPriceFeeIndex>-1&&thanPriceFeeIndex>-1&&thanTypeIndex>-1&&
            (thanTypeIndex>3||(thanAmount>0.0&&thanPrice>0.0))&&
            variableBExact!=0&&
            isValidComparation(comparationText)&&
            isValidPlusMinus(thanPricePlusMinusText)&&
            isValidCode(thanPriceTypeCode)&&
            isValidSymbol(tradeSymbolCode)&&
            isValidSymbol(valueASymbolCode)&&
            isValidSymbol(valueBSymbolCode)&&
            isValidCode(variableACode)&&
            isValidCode(variableBCode)&&
            isValidPlusMinus(variableBplusMinus)&&
            isValidSymbol(variableBSymbolCode);
}
