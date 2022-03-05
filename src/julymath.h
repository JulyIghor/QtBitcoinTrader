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

#ifndef JULYMATH_H
#define JULYMATH_H

#include "qmath.h"

namespace JulyMath
{
    inline QByteArray byteArrayFromDouble(double value, int maxDecimals = 8, int minDecimals = 1);
    inline QByteArray byteArrayRoundFromDouble(double value, int maxDecimals = 8);
    inline QString textFromDoubleStr(double value, int maxDecimals = 8, int minDecimals = 1);
    inline QByteArray textFromDouble(double value, int maxDecimals = 8, int minDecimals = 1);

    inline double& cutDoubleDecimals(double& val, int maxDecimals = 8, bool roundUp = false);
    inline double cutDoubleDecimalsCopy(double val, int maxDecimals = 8, bool roundUp = false);

    inline bool validDouble(double value, int decimals = 8);
    inline int decimalsForDouble(double value);
    inline bool compareDoubles(double& valueA, double& valueB, int decimals = 8);

    inline bool compareDoubles(double& valueA, double& valueB, int decimals)
    {
        return qPow(0.1, qMin(8, decimals) + 1) > qAbs(cutDoubleDecimalsCopy(valueA, decimals) - cutDoubleDecimalsCopy(valueB, decimals));
    }

    inline double& cutDoubleDecimals(double& val, int decimals, bool roundUp)
    {
        if (!validDouble(val, decimals))
        {
            val = 0.0;
            return val;
        }

        if (decimals > 8)
            decimals = 8;

        double zeros = qPow(10, decimals);
        double intPart = floor(val);
        val = floor((val - intPart) * zeros + (roundUp ? 0.5 : 0.0)) / zeros + intPart;
        return val;
    }

    inline double cutDoubleDecimalsCopy(double val, int decimals, bool roundUp)
    {
        if (!validDouble(val, decimals))
            return 0.0;

        if (decimals > 8)
            decimals = 8;

        double zeros = qPow(10, decimals);
        double intPart = floor(val);
        intPart = floor((val - intPart) * zeros + (roundUp ? 0.5 : 0.0)) / zeros + intPart;
        return intPart;
    }

    inline int decimalsForDouble(double val)
    {
        if (val > 999999999999999.9)
            return 0;

        if (val > 99999999999999.99)
            return 1;

        if (val > 9999999999999.999)
            return 2;

        if (val > 999999999999.9999)
            return 3;

        if (val > 99999999999.99999)
            return 4;

        if (val > 9999999999.999999)
            return 5;

        if (val > 999999999.9999999)
            return 6;

        if (val > 99999999.99999999)
            return 7;

        if (val > 9999999.999999999)
            return 8;

        if (val > 999999.9999999999)
            return 9;

        if (val > 99999.99999999999)
            return 10;

        if (val > 9999.999999999999)
            return 11;

        if (val > 999.9999999999999)
            return 12;

        if (val > 99.99999999999999)
            return 13;

        if (val > 9.999999999999999)
            return 14;

        if (val > 0.999999999999999)
            return 15;

        return 16;
    }

    inline bool validDouble(double val, int decimals)
    {
        if (val < 0.00000001)
            return false;

        switch (decimals)
        {
        case 0:
            return val <= 9999999999999999.0;

        case 1:
            return val <= 999999999999999.9;

        case 2:
            return val <= 99999999999999.99;

        case 3:
            return val <= 9999999999999.999;

        case 4:
            return val <= 999999999999.9999;

        case 5:
            return val <= 99999999999.99999;

        case 6:
            return val <= 9999999999.999999;

        case 7:
            return val <= 999999999.9999999;

        case 8:
            return val <= 99999999.99999999;

        case 9:
            return val <= 9999999.999999999;

        case 10:
            return val <= 999999.9999999999;

        case 11:
            return val <= 99999.99999999999;

        case 12:
            return val <= 9999.999999999999;

        case 13:
            return val <= 999.9999999999999;

        case 14:
            return val <= 99.99999999999999;

        case 15:
            return val <= 9.999999999999999;
        }

        return true;
    }

    inline QByteArray byteArrayFromDouble(double val, int maxDecimals, int minDecimals)
    {
        /*maxDecimals=qMin(maxDecimals,decimalsForDouble(val));
        if(minDecimals>maxDecimals)minDecimals=maxDecimals;
        if(!validDouble(val,maxDecimals))return QByteArray("0");
        if(maxDecimals>8)maxDecimals=8;
        QByteArray numberText=QByteArray::number(cutDoubleDecimalsCopy(val,maxDecimals,true),'f',8);
        int dotPos=numberText.size()-9;
        numberText.resize(maxDecimals+dotPos+1);
        int curPos=numberText.size()-1;
        while(curPos>0&&numberText.at(curPos)=='0'&&(dotPos+minDecimals<curPos))curPos--;
        if(curPos==dotPos)curPos--;
        numberText.resize(curPos+1);
        if(numberText.size()-dotPos-1==maxDecimals)
        {
            QByteArray numberTextLess=QByteArray::number(cutDoubleDecimalsCopy(val,maxDecimals),'f',8);
            int dotPosLess=numberTextLess.size()-9;
            numberTextLess.resize(maxDecimals+dotPosLess+1);
            int curPosLess=numberTextLess.size()-1;
            while(curPosLess>0&&numberTextLess.at(curPosLess)=='0'&&(dotPosLess+minDecimals<curPosLess))curPosLess--;
            if(curPosLess==dotPosLess)curPosLess--;
            numberTextLess.resize(curPosLess+1);
            if(numberText.size()>numberTextLess.size())
                return numberTextLess;
        }
        return numberText;*/

        if (maxDecimals > 8)
            maxDecimals = 8;

        if (maxDecimals < 0)
            maxDecimals = 0;

        if (minDecimals > maxDecimals)
            minDecimals = maxDecimals;

        if (minDecimals < 0)
            minDecimals = 0;

        int floorLength = QByteArray::number(floor(val), 'f', 0).length();
        int decimals = 16 - floorLength;

        if (val < 0)
            decimals++;

        if (decimals < minDecimals)
            decimals = minDecimals;

        QByteArray numberText = QByteArray::number(val, 'f', decimals);

        int resultLength = floorLength + maxDecimals + 1;

        if (numberText.length() > resultLength)
            numberText.chop(numberText.length() - resultLength);

        int minLength = floorLength;

        if (minDecimals > 0)
            minLength += minDecimals + 1;

        while (numberText[numberText.length() - 1] == '0' && numberText.length() > minLength)
            numberText.chop(1);

        if (numberText[numberText.length() - 1] == '.')
            numberText.chop(1);

        return numberText;
    }

    inline QByteArray byteArrayRoundFromDouble(double val, int maxDecimals)
    {
        if (maxDecimals > 8)
            maxDecimals = 8;

        QByteArray numberText = QByteArray::number(val, 'f', maxDecimals);

        int numberTextLength = numberText.length();

        if (val < 0)
            numberTextLength--;

        if (numberTextLength > 16)
        {
            int deltaLength = numberTextLength - 16;
            maxDecimals -= deltaLength;

            if (maxDecimals < 0)
                maxDecimals = 0;

            numberText = QByteArray::number(val, 'f', maxDecimals);
        }

        return numberText;
    }

    inline QString textFromDoubleStr(double val, int maxDecimals, int minDecimals)
    {
        if (maxDecimals > 8)
            maxDecimals = 8;

        if (maxDecimals < 0)
            maxDecimals = 0;

        if (minDecimals > maxDecimals)
            minDecimals = maxDecimals;

        if (minDecimals < 0)
            minDecimals = 0;

        int floorLength = QString::number(floor(val), 'f', 0).length();
        int decimals = 16 - floorLength;

        if (val < 0)
            decimals++;

        if (decimals < minDecimals)
            decimals = minDecimals;

        QString numberText = QString::number(val, 'f', decimals);

        int resultLength = floorLength + maxDecimals + 1;

        if (numberText.length() > resultLength)
            numberText.chop(numberText.length() - resultLength);

        int minLength = floorLength;

        if (minDecimals > 0)
            minLength += minDecimals + 1;

        while (numberText[numberText.length() - 1] == '0' && numberText.length() > minLength)
            numberText.chop(1);

        if (numberText[numberText.length() - 1] == '.')
            numberText.chop(1);

        return numberText;
    }

    inline QByteArray textFromDouble(double val, int maxDecimals, int minDecimals)
    {
        if (maxDecimals > 8)
            maxDecimals = 8;

        if (maxDecimals < 0)
            maxDecimals = 0;

        if (minDecimals > maxDecimals)
            minDecimals = maxDecimals;

        if (minDecimals < 0)
            minDecimals = 0;

        int floorLength = QByteArray::number(floor(val), 'f', 0).length();
        int decimals = 16 - floorLength;

        if (val < 0)
            decimals++;

        if (decimals < minDecimals)
            decimals = minDecimals;

        QByteArray numberText = QByteArray::number(val, 'f', decimals);

        int resultLength = floorLength + maxDecimals + 1;

        if (numberText.length() > resultLength)
            numberText.chop(numberText.length() - resultLength);

        int minLength = floorLength;

        if (minDecimals > 0)
            minLength += minDecimals + 1;

        while (numberText[numberText.length() - 1] == '0' && numberText.length() > minLength)
            numberText.chop(1);

        if (numberText[numberText.length() - 1] == '.')
            numberText.chop(1);

        return numberText;
    }
} // namespace JulyMath

#endif // JULYMATH_H
