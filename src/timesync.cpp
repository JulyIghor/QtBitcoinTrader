//  This file is part of Qt Bitcoin Trader
//      https://github.com/JulyIGHOR/QtBitcoinTrader
//  Copyright (C) 2013-2018 July IGHOR <julyighor@gmail.com>
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

#include <QThread>
#include <QElapsedTimer>
#include <QUdpSocket>
#include <QtEndian>
#include <QStandardPaths>
#include <QMessageBox>
#include "main.h"
#include "timesync.h"

TimeSync::TimeSync()
    : QObject(),
      dateUpdateThread(new QThread),
      started(0),
      startTime(QDateTime::currentDateTime().toTime_t()),
      timeShift(0),
      getNTPTimeRetryCount(0)
{
    connect(dateUpdateThread.data(), &QThread::started, this, &TimeSync::runThread);
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

TimeSync* TimeSync::global()
{
    static TimeSync instance;

    static QAtomicInt created = 0;

    if (created)
        return &instance;

    static QMutex mut;
    QMutexLocker lock(&mut);

    while (instance.started == 0)
        QThread::msleep(100);

    created = 1;

    return &instance;
}

qint64 TimeSync::getTimeT()
{
    TimeSync* timeSync = TimeSync::global();

    if (timeSync->additionalTimer == nullptr)
        return QDateTime::currentDateTime().toTime_t();

    qint64 additionalBuffer = 0;
    timeSync->mutex.lock();
    additionalBuffer = timeSync->additionalTimer->elapsed();
    timeSync->mutex.unlock();

    return timeSync->startTime + timeSync->timeShift + qint64(additionalBuffer / 1000);
}

void TimeSync::syncNow()
{
    QSettings mainSettings(appDataDir + "/QtBitcoinTrader.cfg", QSettings::IniFormat);

    if (mainSettings.value("TimeSynchronization", true).toBool())
    {
        emit TimeSync::global()->startSync();
    }
}

void TimeSync::runThread()
{
    connect(QThread::currentThread(), &QThread::finished, this, &TimeSync::quitThread, Qt::DirectConnection);
    startTime = QDateTime::currentDateTime().toTime_t();

    additionalTimer.reset(new QElapsedTimer);
    additionalTimer->start();

    started = 1;
}

void TimeSync::quitThread()
{
    additionalTimer.reset();
}
void TimeSync::getNTPTime()
{
    QUdpSocket sock;
    sock.connectToHost("0.pool.ntp.org", 123);

    if (!sock.waitForConnected(1000))
        return;

    QByteArray data(48, char(0));
    *(reinterpret_cast<qint32*>(&data.data()[0])) = -100007719;

    if (sock.write(data) < 0 || !sock.waitForReadyRead(3000) || sock.bytesAvailable() != 48)
        return;

    data = sock.readAll();
    qint64 seconds  = qToBigEndian(*(reinterpret_cast<quint32*>(&data.data()[40])));
    qint64 fraction = qToBigEndian(*(reinterpret_cast<quint32*>(&data.data()[44])));
    qint64 newTime  = QDateTime::fromMSecsSinceEpoch(seconds * 1000ll + fraction * 1000ll / 0x100000000ll -
                      2208988800000ll).toTime_t();

    if (newTime < 1451606400 || newTime > 4000000000)
    {
        QThread::msleep(500);
        emit startSync();
        return;
    }

    qint64 tempTimeShift = newTime - QDateTime::currentDateTime().toTime_t();

    if (timeShift != 0)
        tempTimeShift = (timeShift + tempTimeShift) / 2;

    if (tempTimeShift > 3600 || tempTimeShift < -3600)
    {
        static bool showMessage = true;

        if (showMessage)
            emit warningMessage(julyTr("TIME_SYNC_ERROR",
                                       "Your clock is not set. Please close the Qt Bitcoin Trader and set the clock. Changing time at Qt Bitcoin Trader enabled can cause errors and damage the keys."));

        showMessage = false;
    }
    else
        timeShift = tempTimeShift;

    getNTPTimeRetryCount++;

    if (getNTPTimeRetryCount < 3)
        emit startSync();
}
