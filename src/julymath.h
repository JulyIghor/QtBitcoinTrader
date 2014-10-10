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

#ifndef JULYMATH_H
#define JULYMATH_H

#include <QtMath>

static QByteArray byteArrayFromDouble(const qreal &value, int maxDecimals=8, int minDecimals=1);
static QString textFromDouble(const qreal &value, int maxDecimals=8, int minDecimals=1);

static qreal &cutDoubleDecimals(qreal &val, int maxDecimals=8, bool roundUp=false);
static qreal cutDoubleDecimalsCopy(const qreal &val, int maxDecimals=8, bool roundUp=false);

static bool validDouble(const qreal &value);
static bool compareDoubles(qreal &valueA, qreal &valueB, int decimals=8);

static bool compareDoubles(qreal &valueA, qreal &valueB, int decimals)
{
    return qPow(0.1,qMin(8,decimals)+1)>qAbs(cutDoubleDecimalsCopy(valueA,decimals)-cutDoubleDecimalsCopy(valueB,decimals));
}

static qreal &cutDoubleDecimals(qreal &val, int decimals, bool roundUp)
{
    if(!validDouble(val)){val=0.0;return val;}
    if(decimals>8)decimals=8;
    qreal zeros=qPow(10,decimals);
    qreal intPart=floor(val);
    val=floor((val-intPart)*zeros+(roundUp?0.5:0.0))/zeros+intPart;
    return val;
}

static qreal cutDoubleDecimalsCopy(const qreal &val, int decimals, bool roundUp)
{
    if(!validDouble(val))return 0.0;
    if(decimals>8)decimals=8;
    qreal zeros=qPow(10,decimals);
    qreal intPart=floor(val);
    intPart=floor((val-intPart)*zeros+(roundUp?0.5:0.0))/zeros+intPart;
    return intPart;
}

static bool validDouble(const qreal &val)
{
    return !(val<0.00000001||val>99999999.99999999);
}

static QByteArray byteArrayFromDouble(const qreal &val, int maxDecimals, int minDecimals)
{
    if(!validDouble(val))return QByteArray("0");
    if(maxDecimals>8)maxDecimals=8;
    QByteArray numberText=QByteArray::number(cutDoubleDecimalsCopy(val,maxDecimals),'f',8);
    int dotPos=numberText.size()-9;
    numberText.resize(maxDecimals+dotPos+1);
    int curPos=numberText.size()-1;
    while(curPos>0&&numberText.at(curPos)=='0'&&(dotPos+minDecimals<curPos))curPos--;
    if(curPos==dotPos)curPos--;
    numberText.resize(curPos+1);
    if(numberText.size()-dotPos-1==maxDecimals)
    {
        QByteArray numberTextLess=QByteArray::number(cutDoubleDecimalsCopy(val,maxDecimals,true),'f',8);
        int dotPosLess=numberTextLess.size()-9;
        numberTextLess.resize(maxDecimals+dotPosLess+1);
        int curPosLess=numberTextLess.size()-1;
        while(curPosLess>0&&numberTextLess.at(curPosLess)=='0'&&(dotPosLess+minDecimals<curPosLess))curPosLess--;
        if(curPosLess==dotPosLess)curPosLess--;
        numberTextLess.resize(curPosLess+1);
        if(numberText.size()>numberTextLess.size())
            return numberTextLess;
    }
    return numberText;
}

static QString textFromDouble(const qreal &val, int maxDecimals, int minDecimals)
{
    if(!validDouble(val))return QLatin1String("0");
    if(maxDecimals>8)maxDecimals=8;
    QString numberText=QString::number(cutDoubleDecimalsCopy(val,maxDecimals),'f',8);
    int dotPos=numberText.size()-9;
    numberText.resize(maxDecimals+dotPos+1);
    int curPos=numberText.size()-1;
    while(curPos>0&&numberText.at(curPos)=='0'&&(dotPos+minDecimals<curPos))curPos--;
    if(curPos==dotPos)curPos--;
    numberText.resize(curPos+1);

    if(numberText.size()-dotPos-1==maxDecimals)
    {
        QString numberTextLess=QString::number(cutDoubleDecimalsCopy(val,maxDecimals,true),'f',8);
        int dotPosLess=numberTextLess.size()-9;
        numberTextLess.resize(maxDecimals+dotPosLess+1);
        int curPosLess=numberTextLess.size()-1;
        while(curPosLess>0&&numberTextLess.at(curPosLess)=='0'&&(dotPosLess+minDecimals<curPosLess))curPosLess--;
        if(curPosLess==dotPosLess)curPosLess--;
        numberTextLess.resize(curPosLess+1);
        if(numberText.size()>numberTextLess.size())
            return numberTextLess;
    }
    return numberText;
}

#endif // JULYMATH_H
