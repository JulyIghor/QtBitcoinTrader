//  This file is part of Qt Bitcoin Trader
//      https://github.com/JulyIGHOR/QtBitcoinTrader
//  Copyright (C) 2013-2023 July Ighor <julyighor@gmail.com>
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

#include "orderitem.h"
#include "iniengine.h"
#include "julymath.h"
#include "main.h"

bool OrderItem::isValid()
{
    bool isVal = date > 0 && price > 0.0 && symbol.size() >= 5;

    if (isVal)
    {
        QDateTime itemDate = QDateTime::fromSecsSinceEpoch(date);

        if (baseValues_->use24HourTimeFormat)
        {
            dateStr = itemDate.toString(baseValues.dateTimeFormat);
        }
        else
        {
            QString mmssTemp = itemDate.toString("mm:ss");
            QString hTemp = itemDate.toString("H");
            qint16 hTempInt = hTemp.toShort();
            QString timeStr;

            if (hTempInt <= 12)
                timeStr = hTemp + ':' + mmssTemp + " am";
            else
                timeStr = QString::number(hTempInt - 12) + ':' + mmssTemp + " pm";

            dateStr = itemDate.toString("dd.MM.yyyy") + ' ' + timeStr;
        }

        QString currAStr, currBStr;
        int posSplitter = symbol.indexOf('/');

        if (posSplitter == -1)
        {
            currAStr = symbol.left(3);
            currBStr = symbol.right(3);
        }
        else
        {
            currAStr = symbol.left(posSplitter);
            currBStr = symbol.right(symbol.size() - posSplitter - 1);
        }

        QString priceSign = IniEngine::getCurrencyInfo(currBStr).sign;
        amountStr = IniEngine::getCurrencyInfo(currAStr).sign + JulyMath::textFromDouble(amount);
        priceStr = priceSign + JulyMath::textFromDouble(price);
        total = price * amount;
        totalStr = priceSign + JulyMath::textFromDouble(total, baseValues.currentPair.currBDecimals);
    }

    return isVal;
}
