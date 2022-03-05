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

#include "charts/chartsmodel.h"
#include "charts/chartsview.h"
#include "julymath.h"
#include "main.h"
#include "timesync.h"
#include "tradesitem.h"

ChartsModel::ChartsModel() :
    QObject(), perfomanceStep(1), intervalDate(60), intervalCount(10), fontMetrics(new QFontMetrics(QApplication::font()))
{
}

ChartsModel::~ChartsModel()
{
}

void ChartsModel::addLastTrades(QList<TradesItem>* newItems)
{
    for (qint32 n = 0; n < newItems->size(); ++n)
    {
        if (!tradesDate.empty() && tradesDate.last() > newItems->at(n).date)
            continue;

        tradesDate.append(newItems->at(n).date);
        tradesPrice.append(newItems->at(n).price);
        tradesType.append(newItems->at(n).orderType);

        if (!amountDate.empty() && newItems->at(n).date < (amountDate.last() + intervalDate))
            amountPrice.last() += newItems->at(n).amount;
        else
        {
            amountPrice.append(newItems->at(n).amount);
            amountDate.append(int(newItems->at(n).date / intervalDate) * intervalDate);
        }
    }

    delete newItems;

    baseValues_->mainWindow_->chartsView->comeNewData();
}

void ChartsModel::addBound(double price, bool type)
{
    if (type)
    {
        boundsSellDate.append(TimeSync::getTimeT());
        boundsSellPrice.append(price);
    }
    else
    {
        boundsBuyDate.append(TimeSync::getTimeT());
        boundsBuyPrice.append(price);
    }
}

void ChartsModel::clearCharts()
{
    amountDate.clear();
    amountPrice.clear();
    tradesDate.clear();
    tradesPrice.clear();
    tradesType.clear();
    boundsSellDate.clear();
    boundsSellPrice.clear();
    boundsBuyDate.clear();
    boundsBuyPrice.clear();
}

double ChartsModel::stepRound(double step)
{
    qint16 signCount = 0;

    while (step > 100)
    {
        step /= 10;
        ++signCount;
    }

    while (step < 10)
    {
        step *= 10;
        --signCount;
    }

    if (step > 50)
        step = 50;
    else
    {
        if (step > 25)
            step = 25;
        else
        {
            if (step > 20)
                step = 20;
            else
                step = 10;
        }
    }

    for (qint16 i = 0; i < signCount; ++i)
        step *= 10;

    for (qint16 i = 0; i < -signCount; ++i)
        step /= 10;

    return step;
}

double ChartsModel::axisRound(double old_x, double old_step)
{
    double x = old_x;
    double step = old_step;
    int signCount = 0;

    while (step > 1)
    {
        x /= 10;
        step /= 10;
        ++signCount;
    }

    while (step < 0.1)
    {
        x *= 10;
        step *= 10;
        --signCount;
    }

    x = qRound64(x - 0.5);

    for (int i = 0; i < signCount; ++i)
        x *= 10;

    for (int i = 0; i < -signCount; ++i)
        x /= 10;

    while (x + old_step < old_x)
        x += old_step;

    return x;
}

void ChartsModel::prepareInit()
{
    graphDateText.clear();
    graphDateTextX.clear();
    graphAmountText.clear();
    graphAmountTextY.clear();
    graphPriceText.clear();
    graphPriceTextY.clear();

    graphAmountX.clear();
    graphAmountY.clear();
    graphTradesX.clear();
    graphTradesY.clear();
    graphTradesType.clear();
    graphBoundsSellX.clear();
    graphBoundsSellY.clear();
    graphBoundsBuyX.clear();
    graphBoundsBuyY.clear();

    nowTime = TimeSync::getTimeT();
    graphLastDate = int(nowTime / intervalDate) * intervalDate + intervalDate;
    graphFirstDate = graphLastDate - intervalDate * 10;
}

void ChartsModel::prepareXAxis()
{
    graphXScale = double(chartsWidth) / double(graphLastDate - graphFirstDate);

    for (quint32 i = graphFirstDate; i <= graphLastDate; i += intervalDate)
    {
        graphDateTextX.append(qRound64(graphXScale * (i - graphFirstDate)));
        graphDateText.append(QDateTime::fromSecsSinceEpoch(i).toString("h:mm"));
    }
}

void ChartsModel::prepareAmountYAxis()
{
    iAmountFirst = std::lower_bound(amountDate.begin(), amountDate.end(), graphFirstDate) - amountDate.begin();
    widthAmountYAxis = 5;

    if (iAmountFirst < amountDate.size())
    {
        amountMax = amountPrice.at(std::max_element(amountPrice.begin() + iAmountFirst, amountPrice.end()) - amountPrice.begin());
        amountYScale = double(chartsHeight) * 0.9 / amountMax;
        double amountStepY = stepRound(amountMax / 5);

        for (double amountY = 0; amountY < amountMax; amountY += amountStepY)
        {
            graphAmountText.append(baseValues.currentPair.currASign + QLatin1String(" ") + JulyMath::textFromDoubleStr(amountY, 8, 0));
            graphAmountTextY.append(qRound64(amountYScale * amountY));
            widthAmountYAxis = qMax(fontMetrics->horizontalAdvance(graphAmountText.last()), widthAmountYAxis);
        }
    }

    widthAmountYAxis += 14;
}

void ChartsModel::prepareAmount()
{
    if (iAmountFirst < amountDate.size())
    {
        for (qint32 i = iAmountFirst; i < amountPrice.size(); ++i)
        {
            graphAmountX.append(qRound64(graphXScale * (amountDate.at(i) - graphFirstDate + intervalDate / 2)));
            graphAmountY.append(qRound64(amountYScale * amountPrice.at(i)));
        }
    }
}

void ChartsModel::preparePriceMinMax()
{
    priceInit = false;
    priceMin = -1;
    priceMax = -1;
    iTradesFirst = std::lower_bound(tradesDate.begin(), tradesDate.end(), graphFirstDate) - tradesDate.begin();
    iBoundsSellFirst = std::lower_bound(boundsSellDate.begin(), boundsSellDate.end(), graphFirstDate) - boundsSellDate.begin();
    iBoundsBuyFirst = std::lower_bound(boundsBuyDate.begin(), boundsBuyDate.end(), graphFirstDate) - boundsBuyDate.begin();

    double temp;
    QList<double> min;
    QList<double> max;

    if (iTradesFirst < tradesDate.size())
    {
        temp = tradesPrice.at(std::min_element(tradesPrice.begin() + iTradesFirst, tradesPrice.end()) - tradesPrice.begin());

        if (temp > 0)
            min.push_back(temp);

        temp = tradesPrice.at(std::max_element(tradesPrice.begin() + iTradesFirst, tradesPrice.end()) - tradesPrice.begin());

        if (temp > 0)
            max.push_back(temp);
    }

    if (iBoundsSellFirst < boundsSellDate.size())
    {
        int tempIBoundsSellFirst = iBoundsSellFirst;

        if (tempIBoundsSellFirst > 0)
            --tempIBoundsSellFirst;

        temp = boundsSellPrice.at(std::min_element(boundsSellPrice.begin() + tempIBoundsSellFirst, boundsSellPrice.end()) -
                                  boundsSellPrice.begin());

        if (temp > 0)
            min.push_back(temp);

        temp = boundsSellPrice.at(std::max_element(boundsSellPrice.begin() + tempIBoundsSellFirst, boundsSellPrice.end()) -
                                  boundsSellPrice.begin());

        if (temp > 0)
            max.push_back(temp);
    }

    if (iBoundsBuyFirst < boundsBuyDate.size())
    {
        int tempIBoundsBuyFirst = iBoundsBuyFirst;

        if (tempIBoundsBuyFirst > 0)
            --tempIBoundsBuyFirst;

        temp = boundsBuyPrice.at(std::min_element(boundsBuyPrice.begin() + tempIBoundsBuyFirst, boundsBuyPrice.end()) -
                                 boundsBuyPrice.begin());

        if (temp > 0)
        {
            if (min.size() < 2)
                min.push_back(temp);
            else
                min.last() = qMin(min.last(), temp);
        }

        temp = boundsBuyPrice.at(std::max_element(boundsBuyPrice.begin() + iBoundsBuyFirst, boundsBuyPrice.end()) - boundsBuyPrice.begin());

        if (temp > 0)
        {
            if (max.size() < 2)
                max.push_back(temp);
            else
                max.last() = qMax(max.last(), temp);
        }
    }

    if (min.empty() || max.empty())
        return;

    priceMin = qMin(min.first(), min.last());
    priceMax = qMax(max.first(), max.last());

    if (priceMax != priceMin)
    {
        priceInit = true;

        if ((priceMax - priceMin) * 100 < priceMax)
        {
            priceMax += (priceMax - priceMin) * 0.2;
            priceMin -= (priceMax - priceMin) * 0.1;
        }

        double deltaPrice = (priceMax - priceMin) * 0.1;
        priceMin -= deltaPrice;

        if (priceMin < 0)
            priceMin = 0;

        priceMax += deltaPrice;

        priceStepY = stepRound((priceMax - priceMin) / 5);
        priceMin = axisRound(priceMin, priceStepY);
    }
}

void ChartsModel::preparePriceYAxis()
{
    widthPriceYAxis = 5;

    if (priceInit)
    {
        priceYScale = double(chartsHeight) / (priceMax - priceMin);

        for (double priceY = priceMin; priceY < priceMax; priceY += priceStepY)
        {
            graphPriceText.append(baseValues.currentPair.currBSign + " " + JulyMath::textFromDouble(priceY, 8, 0));
            graphPriceTextY.append(qRound64(priceYScale * (priceY - priceMin)));
            widthPriceYAxis = qMax(fontMetrics->horizontalAdvance(graphPriceText.last()), widthPriceYAxis);
        }
    }

    widthPriceYAxis += 14;
}

void ChartsModel::preparePrice()
{
    if (!priceInit)
        return;

    for (qint32 i = iTradesFirst; i < tradesDate.size(); ++i)
    {
        qint16 x = qRound64(graphXScale * (tradesDate.at(i) - graphFirstDate));
        qint16 y = qRound64(priceYScale * (tradesPrice.at(i) - priceMin));

        if (!graphTradesX.empty())
        {
            if (abs(x - graphTradesX.last()) <= perfomanceStep && abs(y - graphTradesY.last()) <= perfomanceStep)
                continue;
        }

        graphTradesX.append(x);
        graphTradesY.append(y);
        graphTradesType.append(tradesType.at(i));
    }
}

void ChartsModel::prepareBound()
{
    if (priceInit)
    {
        qint16 graphBoundX;

        if (iBoundsSellFirst > 0 && iBoundsSellFirst <= boundsSellPrice.size())
        {
            graphBoundsSellX.append(1);
            graphBoundsSellY.append(qRound64(priceYScale * (boundsSellPrice[iBoundsSellFirst - 1] - priceMin)));
        }

        if (iBoundsBuyFirst > 0 && iBoundsBuyFirst <= boundsBuyPrice.size())
        {
            graphBoundsBuyX.append(1);
            graphBoundsBuyY.append(qRound64(priceYScale * (boundsBuyPrice[iBoundsBuyFirst - 1] - priceMin)));
        }

        for (qint32 i = iBoundsSellFirst; i < boundsSellPrice.size(); ++i)
        {
            graphBoundX = qRound64(graphXScale * (boundsSellDate[i] - graphFirstDate));

            if (!graphBoundsSellY.empty())
            {
                graphBoundsSellX.append(graphBoundX);
                graphBoundsSellY.append(graphBoundsSellY.last());
            }

            graphBoundsSellX.append(graphBoundX);
            graphBoundsSellY.append(qRound64(priceYScale * (boundsSellPrice[i] - priceMin)));
        }

        for (qint32 i = iBoundsBuyFirst; i < boundsBuyPrice.size(); ++i)
        {
            graphBoundX = qRound64(graphXScale * (boundsBuyDate[i] - graphFirstDate));

            if (!graphBoundsBuyY.empty())
            {
                graphBoundsBuyX.append(graphBoundX);
                graphBoundsBuyY.append(graphBoundsBuyY.last());
            }

            graphBoundsBuyX.append(graphBoundX);
            graphBoundsBuyY.append(qRound64(priceYScale * (boundsBuyPrice[i] - priceMin)));
        }

        if (!graphBoundsSellX.empty())
        {
            graphBoundsSellX.append(qRound64(graphXScale * (nowTime - graphFirstDate)));
            graphBoundsSellY.append(graphBoundsSellY.last());
        }

        if (!graphBoundsBuyX.empty())
        {
            graphBoundsBuyX.append(qRound64(graphXScale * (nowTime - graphFirstDate)));
            graphBoundsBuyY.append(graphBoundsBuyY.last());
        }
    }
}

bool ChartsModel::prepareChartsData(int parentWidth, int parentHeight)
{
    if (boundsSellDate.empty())
        return false;

    chartsHeight = parentHeight - 5;

    prepareInit();
    prepareAmountYAxis();
    preparePriceMinMax();
    preparePriceYAxis();

    chartsWidth = parentWidth - widthAmountYAxis - widthPriceYAxis - 1;
    prepareXAxis();
    prepareAmount();
    preparePrice();
    prepareBound();

    return true;
}
