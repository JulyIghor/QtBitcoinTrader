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

#include "julylockfile.h"
#include "main.h"
#include "timesync.h"
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QNetworkProxy>
#include <QRandomGenerator>
#include <QTime>
#include <QTimer>
#include <QUdpSocket>

JulyLockFile::JulyLockFile(const QString& imageName, QString tempDir) : QObject()
{
    lockFile = new QFile;
    lockSocket = new QUdpSocket;
    QNetworkProxy proxy;
    proxy.setType(QNetworkProxy::NoProxy);
    lockSocket->setProxy(proxy);
    isLockedFile = false;
    lockPort = 0;

    if (tempDir.isEmpty())
        tempDir = QDir().tempPath();

    lockFilePath = tempDir + "/Locked_" + QCryptographicHash::hash(imageName.toUtf8(), QCryptographicHash::Md5).toHex() + ".lockfile";

    if (QFile::exists(lockFilePath) && QFileInfo(lockFilePath).lastModified().addSecs(240).toSecsSinceEpoch() < TimeSync::getTimeT())
        QFile::remove(lockFilePath);

    lockFile->setFileName(lockFilePath);

    if (QFile::exists(lockFilePath))
    {
        if (lockFile->open(QIODevice::ReadOnly))
        {
            lockPort = lockFile->readAll().trimmed().toUShort();
            lockFile->close();
        }

        if (lockPort > 0)
        {
            isLockedFile = !lockSocket->bind(QHostAddress::LocalHost, lockPort, QUdpSocket::DontShareAddress);

            if (isLockedFile)
                return;
        }
    }

    if (lockPort == 0 || lockSocket->state() != QUdpSocket::BoundState)
    {
        lockPort = QRandomGenerator::global()->bounded(2000, 4000);
        int i = 0;

        while (!lockSocket->bind(QHostAddress::LocalHost, ++lockPort, QUdpSocket::DontShareAddress))
        {
            i++;

            if (i > 100)
            {
                if (lockFile->isOpen())
                {
                    lockFile->close();
                    lockFile->remove(lockFilePath);
                }

                isLockedFile = false;
                return;
            }
        }
    }

    auto* minuteTimer = new QTimer(this);
    connect(minuteTimer, &QTimer::timeout, this, &JulyLockFile::updateLockFile);
    minuteTimer->start(60000);
    updateLockFile();
    isLockedFile = lockSocket->state() != QUdpSocket::BoundState;
}

JulyLockFile::~JulyLockFile()
{
    free();
}

void JulyLockFile::free()
{
    if (lockFile)
    {
        if (lockFile->isOpen() || lockSocket->state() == QUdpSocket::BoundState)
        {
            lockFile->close();
            lockFile->remove(lockFilePath);
        }

        delete lockFile;
        lockFile = nullptr;
    }

    if (lockSocket)
    {
        delete lockSocket;
        lockSocket = nullptr;
    }
}

void JulyLockFile::updateLockFile()
{
    if (lockFile->open(QIODevice::WriteOnly))
    {
        lockFile->write(QByteArray::number(lockPort));
        lockFile->close();
    }
    else
        lockSocket->close();
}

bool JulyLockFile::isLocked() const
{
    return isLockedFile;
}
