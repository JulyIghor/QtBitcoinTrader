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

#ifndef CHARTSMODEL_H
#define CHARTSMODEL_H

#include <QObject>

struct TradesItem;
class QFontMetrics;

class ChartsModel : public QObject
{
    Q_OBJECT
public:
    int chartsWidth;
    int chartsHeight;
    qint32 widthAmountYAxis;
    qint32 widthPriceYAxis;
    qint16 perfomanceStep;

    QList<qint16> graphTradesX;
    QList<qint16> graphTradesY;
    QList<qint16> graphTradesType;
    QList<qint16> graphBoundsSellX;
    QList<qint16> graphBoundsSellY;
    QList<qint16> graphBoundsBuyX;
    QList<qint16> graphBoundsBuyY;
    QList<qint16> graphAmountX;
    QList<qint16> graphAmountY;

    QList<QString> graphDateText;
    QList<qint16> graphDateTextX;
    QList<QString> graphPriceText;
    QList<qint16> graphPriceTextY;
    QList<QString> graphAmountText;
    QList<qint16> graphAmountTextY;

    ChartsModel();
    ~ChartsModel();

    bool prepareChartsData(int, int);

public slots:
    void addLastTrades(QList<TradesItem>*);
    void addBound(double, bool);
    void clearCharts();

private:
    qint32 intervalDate;
    qint32 intervalCount;
    QScopedPointer<QFontMetrics> fontMetrics;

    QList<quint32> tradesDate;
    QList<double> tradesPrice;
    QList<qint16> tradesType;
    QList<quint32> boundsSellDate;
    QList<double> boundsSellPrice;
    QList<quint32> boundsBuyDate;
    QList<double> boundsBuyPrice;
    QList<quint32> amountDate;
    QList<double> amountPrice;

    quint32 nowTime;
    quint32 graphLastDate;
    quint32 graphFirstDate;
    double graphXScale;
    double amountYScale;
    double priceYScale;
    qint32 iAmountFirst;
    qint32 iTradesFirst;
    qint32 iBoundsSellFirst;
    qint32 iBoundsBuyFirst;
    double amountMax;
    bool priceInit;
    double priceMin;
    double priceMax;
    double priceStepY;

    double stepRound(double);
    double axisRound(double, double);
    void prepareInit();
    void prepareXAxis();
    void prepareAmountYAxis();
    void prepareAmount();
    void preparePriceMinMax();
    void preparePriceYAxis();
    void preparePrice();
    void prepareBound();
};

#endif // CHARTSMODEL_H
