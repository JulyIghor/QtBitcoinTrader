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

#include "indicatorengine.h"
#include "main.h"
#include <QThread>

IndicatorEngine::IndicatorEngine() : QObject()
{
    connect(this, &IndicatorEngine::indicatorChanged, this, &IndicatorEngine::setValueSlot, Qt::DirectConnection);

    connect(this, &IndicatorEngine::indicatorHighChanged, baseValues.mainWindow_, &QtBitcoinTrader::indicatorHighChanged);
    connect(this, &IndicatorEngine::indicatorLowChanged, baseValues.mainWindow_, &QtBitcoinTrader::indicatorLowChanged);
    connect(this, &IndicatorEngine::indicatorSellChanged, baseValues.mainWindow_, &QtBitcoinTrader::indicatorSellChanged);
    connect(this, &IndicatorEngine::indicatorBuyChanged, baseValues.mainWindow_, &QtBitcoinTrader::indicatorBuyChanged);
    connect(this, &IndicatorEngine::indicatorLastChanged, baseValues.mainWindow_, &QtBitcoinTrader::indicatorLastChanged);
    connect(this, &IndicatorEngine::indicatorVolumeChanged, baseValues.mainWindow_, &QtBitcoinTrader::indicatorVolumeChanged);

    m_thread.reset(new QThread);
    m_thread->setObjectName("Indicator Engine");

    moveToThread(m_thread.data());
    m_thread->start();
}

IndicatorEngine::~IndicatorEngine()
{
    if (m_thread && m_thread->isRunning())
    {
        m_thread->quit();
        m_thread->wait();
    }
}

//---------------------------------------- Static ----------------------------------------
IndicatorEngine* IndicatorEngine::global()
{
    static IndicatorEngine instance;
    return &instance;
}

void IndicatorEngine::setValue(const QString& exchange, const QString& symbol, const QString& indicator, double value)
{
    emit IndicatorEngine::global()->indicatorChanged(exchange, symbol, indicator, value);
}

double IndicatorEngine::getValue(const QString& index)
{
    IndicatorEngine::global()->locker.lock();
    double value = IndicatorEngine::global()->indicators[index.toLatin1()];
    IndicatorEngine::global()->locker.unlock();

    return value;
}

//---------------------------------------- Private ----------------------------------------
void IndicatorEngine::setValueSlot(const QString& exchange, const QString& symbol, const QString& name, double value)
{
    if (exchange == baseValues.exchangeName && symbol == baseValues.currentPair.symbol)
    {
        QByteArray index = (exchange + '_' + symbol + '_' + name).toLatin1();

        if (indicators.contains(index))
        {
            if (qFuzzyCompare(indicators[index], value))
                return;

            locker.lock();
            indicators[index] = value;
            locker.unlock();
        }
        else
        {
            locker.lock();
            indicators.insert(index, value);
            locker.unlock();
        }

        if (name == "High")
            emit indicatorHighChanged(symbol, value);
        else if (name == "Low")
            emit indicatorLowChanged(symbol, value);
        else if (name == "Sell")
            emit indicatorSellChanged(symbol, value);
        else if (name == "Buy")
            emit indicatorBuyChanged(symbol, value);
        else if (name == "Last")
            emit indicatorLastChanged(symbol, value);
        else if (name == "Volume")
            emit indicatorVolumeChanged(symbol, value);
    }
}
