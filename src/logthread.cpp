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

#include "logthread.h"
#include "main.h"
#include "timesync.h"
#include <QApplication>
#include <QFile>
#include <QThread>

LogThread::LogThread(bool wrf) : QObject()
{
    writeFile = wrf;

    m_thread.reset(new QThread);
    m_thread->setObjectName("Log Thread");

    connect(m_thread.data(), &QThread::started, this, &LogThread::run);

    moveToThread(m_thread.data());
    m_thread->start();
}

LogThread::~LogThread()
{
    if (!m_thread.isNull() && m_thread->isRunning())
    {
        m_thread->quit();
        m_thread->wait();
    }
}

void LogThread::run() const
{
    connect(this, &LogThread::writeLogSignal, this, &LogThread::writeLogSlot, Qt::QueuedConnection);
}

void LogThread::writeLog(const QByteArray& data, int dbLvl)
{
    if (debugLevel == 0)
        return;

    if (debugLevel == 2 && dbLvl != 2)
        return; // 0: Disabled; 1: Debug; 2: Log

    emit writeLogSignal(data, dbLvl);
}

void LogThread::writeLogSlot(QByteArray data, int dbLvl)
{
    data = "------------------\r\n" + QDateTime::fromSecsSinceEpoch(TimeSync::getTimeT()).toString("yyyy-MM-dd HH:mm:ss LVL:").toLatin1() +
           QByteArray::number(dbLvl) + "\r\n" + data + "\r\n------------------\r\n\r\n";

    if (writeFile)
    {
        QFile logFile(baseValues.logFileName);

        if (logFile.open(QIODevice::Append))
        {
            logFile.write(data);
            logFile.close();
        }
    }
    else
        emit sendLogSignal(data);
}
