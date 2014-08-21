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

#include "julylockfile.h"
#include <QCryptographicHash>
#include <QUdpSocket>
#include <QFile>
#include <QDir>
#include <QTime>

JulyLockFile::JulyLockFile(QString imageName)
{	
    qsrand(QTime::currentTime().msecsTo(QTime(23,59,59,999)));
    lockFile=new QFile;
    lockSocket=new QUdpSocket;
    isLockedFile=false;
    quint16 lockPort=0;

    lockFilePath=QDir().tempPath()+"/Locked_"+QCryptographicHash::hash(imageName.toLatin1(),QCryptographicHash::Md5).toHex()+".lockfile";
    lockFile->setFileName(lockFilePath);
    if(QFile::exists(lockFilePath))
    {
        if(lockFile->open(QIODevice::ReadOnly))
        {
            lockPort=lockFile->readAll().trimmed().toUInt();
            lockFile->close();
        }

        if(lockPort>0)
        {
            isLockedFile=!lockSocket->bind(QHostAddress::LocalHost,lockPort,QUdpSocket::DontShareAddress);
            if(isLockedFile)return;
        }
    }

    if(lockPort==0||lockSocket->state()!=QUdpSocket::BoundState)
    {
        lockPort=1999+qrand()%2000;
        while(!lockSocket->bind(QHostAddress::LocalHost,++lockPort,QUdpSocket::DontShareAddress));
    }

    if(lockFile->open(QIODevice::WriteOnly))
    {
        lockFile->write(QByteArray::number(lockPort));
        lockFile->close();
    }
    else lockSocket->close();
    isLockedFile=lockSocket->state()!=QUdpSocket::BoundState;
}

JulyLockFile::~JulyLockFile()
{
    if(lockFile->isOpen()||lockSocket->state()==QUdpSocket::BoundState)
	{
    lockFile->close();
    lockFile->remove(lockFilePath);
	}
    if(lockFile)delete lockFile;
    if(lockSocket)delete lockSocket;
}

bool JulyLockFile::isLocked()
{
    return isLockedFile;
}
