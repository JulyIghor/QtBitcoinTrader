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

#include "tradesitem.h"
#include "julymath.h"
#include "main.h"

TradesItem::TradesItem()
{
    backGray = false;
    displayFullDate = false;
    date = 0;
    amount = 0.0;
    price = 0.0;
    total = 0.0;
    orderType = 0;
    direction = 0;
}

void TradesItem::cacheStrings()
{
    QDateTime itemDate = QDateTime::fromSecsSinceEpoch(date);

    if (baseValues_->use24HourTimeFormat)
    {
        timeStr = itemDate.toString(baseValues.timeFormat);
        dateStr = itemDate.toString(baseValues.dateTimeFormat);
    }
    else
    {
        QString mmssTemp = itemDate.toString("mm:ss");
        QString hTemp = itemDate.toString("H");
        qint16 hTempInt = hTemp.toShort();

        if (hTempInt <= 12)
            timeStr = hTemp + ':' + mmssTemp + " am";
        else
            timeStr = QString::number(hTempInt - 12) + ':' + mmssTemp + " pm";

        dateStr = itemDate.toString("dd.MM.yyyy") + ' ' + timeStr;
    }

    if (price > 0.0)
        priceStr = JulyMath::textFromDouble(price, baseValues.decimalsPriceLastTrades);

    if (amount > 0.0)
        amountStr = JulyMath::textFromDouble(amount, baseValues.decimalsAmountLastTrades);

    if (amount > 0.0 && price > 0.0)
        totalStr = JulyMath::textFromDouble(price * amount, qMin(baseValues.currentPair.currBDecimals, baseValues.decimalsTotalLastTrades));
}

bool TradesItem::isValid()
{
    bool valid = date > 0 && price > 0.0 && amount > 0.0;

    if (valid)
        cacheStrings();

    return valid;
}
