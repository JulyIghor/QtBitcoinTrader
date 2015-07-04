//  This file is part of Qt Bitcion Trader
//      https://github.com/JulyIGHOR/QtBitcoinTrader
//  Copyright (C) 2013-2015 July IGHOR <julyighor@gmail.com>
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
#include <QUdpSocket>
#include <QtEndian>
#include <QDateTime>
#include <QSettings>
#include <QStandardPaths>
#include <QApplication>

TimeSync::TimeSync() : QObject()
{
    timeShift=0;

    dateUpdateThread=new QThread;
    connect(dateUpdateThread,SIGNAL(started()),this,SLOT(runThread()));
    connect(this,SIGNAL(finishThread()),dateUpdateThread,SLOT(quit()));//terminate()

    this->moveToThread(dateUpdateThread);
    dateUpdateThread->start();
}

TimeSync::~TimeSync()
{
    emit stopTimer();
    emit deleteTimer();
    emit finishThread();
}

TimeSync* TimeSync::global()
{
    static TimeSync instance;
    return &instance;
}

quint32 TimeSync::getTimeT()
{
    return QDateTime::currentDateTime().addSecs(TimeSync::global()->timeShift.load()).toTime_t();
}

void TimeSync::runThread()
{
    QString appDataDir=QStandardPaths::standardLocations(QStandardPaths::DataLocation).first().replace('\\','/')+"/";
    QSettings mainSettings(appDataDir+"/QtBitcoinTrader.cfg",QSettings::IniFormat);
    if(mainSettings.value("TimeSynchronization",true).toBool()){
        dateUpdateTimer=new QTimer();
        connect(dateUpdateTimer,SIGNAL(timeout()),this,SLOT(getNTPTime()));
        connect(this,SIGNAL(stopTimer()),dateUpdateTimer,SLOT(stop()));
        connect(this,SIGNAL(deleteTimer()),dateUpdateTimer,SLOT(deleteLater()));
        dateUpdateTimer->start(30000);

        getNTPTime();
    }
}

void TimeSync::getNTPTime()
{
    QUdpSocket sock;
    sock.connectToHost("0.pool.ntp.org",123);
    if(!sock.waitForConnected(1000))return;
    QByteArray data(48,char(0)); *(reinterpret_cast<qint32 *>(&data.data()[0]))=4194959577;
    if(sock.write(data)<0||!sock.waitForReadyRead(3000)||sock.bytesAvailable()!=48)return;
    data=sock.readAll();
    quint32 seconds=qToBigEndian(*(reinterpret_cast<quint32 *>(&data.data()[40])));
    quint32 fraction=qToBigEndian(*(reinterpret_cast<quint32 *>(&data.data()[44])));
    timeShift.fetchAndStoreOrdered(QDateTime::fromMSecsSinceEpoch(seconds*1000ll+fraction*1000ll/0x100000000ll-2208988800000ll).toTime_t()-QDateTime::currentDateTime().toTime_t());
}
