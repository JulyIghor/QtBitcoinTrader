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

#include "timesync.h"
#include "main.h"
#include <QElapsedTimer>
#include <QMessageBox>
#include <QStandardPaths>
#include <QThread>
#include <QTimeZone>
#include <QUdpSocket>
#include <QtEndian>

const qint64 c_deltaTime = 3600000;
const int c_maxErrorCount = 20;

TimeSync::TimeSync() : QObject(), dateUpdateThread(new QThread), started(false), timeShift(0LL)
{
    dateUpdateThread->setObjectName("Time Sync");
    connect(dateUpdateThread.data(), &QThread::started, this, &TimeSync::runThread, Qt::DirectConnection);
    connect(this, &TimeSync::startSync, this, &TimeSync::getNTPTime, Qt::QueuedConnection);
    moveToThread(dateUpdateThread.data());
    dateUpdateThread->start();
}

TimeSync::~TimeSync()
{
    if (dateUpdateThread && dateUpdateThread->isRunning())
    {
        dateUpdateThread->quit();
        dateUpdateThread->wait();
    }
}

void TimeSync::runThread()
{
    started = true;
}

void TimeSync::syncNow()
{
    QSettings mainSettings(appDataDir + "/QtBitcoinTrader.cfg", QSettings::IniFormat);

    if (mainSettings.value("TimeSynchronization", true).toBool())
    {
        emit TimeSync::global()->startSync();
    }
}

TimeSync* TimeSync::global()
{
    static TimeSync instance;
    return &instance;
}

qint64 TimeSync::getMSecs()
{
    return QDateTime::currentDateTime().toMSecsSinceEpoch() + TimeSync::global()->timeShift;
}

qint64 TimeSync::getTimeT()
{
    return QDateTime::currentDateTimeUtc().toSecsSinceEpoch() + (TimeSync::global()->timeShift / 1000);
}

void TimeSync::getNTPTime()
{
    QUdpSocket sock;
    static int errorCount = 0;

    switch (errorCount / 4)
    {
    case 0:
        sock.connectToHost("0.pool.ntp.org", 123);
        break;

    case 1:
        sock.connectToHost("1.pool.ntp.org", 123);
        break;

    case 2:
        sock.connectToHost("2.pool.ntp.org", 123);
        break;

    default:
        sock.connectToHost("3.pool.ntp.org", 123);
        break;
    }

    if (!sock.waitForConnected(1000))
    {
        if (errorCount > c_maxErrorCount)
            return;

        ++errorCount;
        QThread::msleep(500);
        emit startSync();
        return;
    }

    QByteArray data(48, char(0));
    *(reinterpret_cast<qint32*>(&data.data()[0])) = -100007719;

    if (sock.write(data) < 0 || !sock.waitForReadyRead(3000) || sock.bytesAvailable() != 48)
    {
        if (errorCount > c_maxErrorCount)
            return;

        ++errorCount;
        QThread::msleep(500);
        emit startSync();
        return;
    }

    data = sock.readAll();
    qint64 seconds = qToBigEndian(*(reinterpret_cast<quint32*>(&data.data()[40])));
    qint64 fraction = qToBigEndian(*(reinterpret_cast<quint32*>(&data.data()[44])));

    if (seconds < 1 || fraction > 4290672329)
    {
        if (errorCount > c_maxErrorCount)
            return;

        ++errorCount;
        QThread::msleep(500);
        emit startSync();
        return;
    }

    qint64 time = seconds * 1000LL + fraction * 1000LL / 0x100000000LL - 2208988800000LL;

    if (time < 1547337932000 || time > 9000000000000)
    {
        if (errorCount > c_maxErrorCount)
            return;

        ++errorCount;
        QThread::msleep(500);
        emit startSync();
        return;
    }

    qint64 tempTimeShift = time - QDateTime::currentDateTime().toMSecsSinceEpoch();
    timeShift.store(timeShift == 0 ? tempTimeShift : (timeShift + tempTimeShift) / 2);

    if (timeShift > c_deltaTime || timeShift < -c_deltaTime)
    {
        static bool showMessage = true;

        if (showMessage)
        {
            QDateTime local = QDateTime::currentDateTime();
            QDateTime server = QDateTime::fromMSecsSinceEpoch(getMSecs());

            emit warningMessage(julyTr("TIME_SYNC_ERROR",
                                       "Your clock is not set. Please close the Qt Bitcoin Trader and set the clock. "
                                       "Changing time at Qt Bitcoin Trader enabled can cause errors and damage the keys.") +
                                "<br><br>" + julyTr("LOCAL_TIME", "Local time") + ": " + local.toString(baseValues.dateTimeFormat) + " " +
                                local.timeZone().displayName(local, QTimeZone::OffsetName) + "<br>" + julyTr("SERVER_TIME", "Server time") +
                                ": " + server.toString(baseValues.dateTimeFormat) + " " +
                                server.timeZone().displayName(server, QTimeZone::OffsetName));
            showMessage = false;
        }
    }

    static int retryCount = 0;
    ++retryCount;

    if (retryCount < 3)
        emit startSync();
}
