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

#include "scriptobjectthread.h"
#include "main.h"
#include "timesync.h"
#include <QFile>
#include <QThread>

ScriptObjectThread::ScriptObjectThread() : QObject()
{
    m_thread.reset(new QThread);
    m_thread->setObjectName("Script Engine");
    moveToThread(m_thread.data());
    m_thread->start();
}

ScriptObjectThread::~ScriptObjectThread()
{
    if (m_thread && m_thread->isRunning())
    {
        m_thread->quit();
        m_thread->wait();
    }
}

void ScriptObjectThread::performFileWrite(const QString& path, const QByteArray& data)
{
    QFile dataFile(path);

    if (!dataFile.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    dataFile.write(data + "\n");
    dataFile.close();
}

void ScriptObjectThread::performFileAppend(const QString& path, const QByteArray& data)
{
    QFile dataFile(path);

    if (!dataFile.open(QIODevice::Append | QIODevice::Text))
        return;

    dataFile.write(data + "\n");
    dataFile.close();
}

void ScriptObjectThread::performFileReadLine(const QString& path, qint64 seek, quint32 fileOperationNumber)
{
    if (seek == -1 && positions.contains(path))
    {
        if (positions[path] == -1)
        {
            emit fileReadResult("", fileOperationNumber);
            return;
        }

        seek = positions[path];
    }

    QFile dataFile(path);

    if (!dataFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        emit fileReadResult("", fileOperationNumber);
        return;
    }

    if (seek > 0 && seek < dataFile.size())
        dataFile.seek(seek);

    QByteArray data = dataFile.readLine();

    if (data.at(data.length() - 1) == '\n')
        data.chop(1);

    emit fileReadResult(data, fileOperationNumber);

    if (dataFile.atEnd())
        positions[path] = -1;
    else
        positions[path] = dataFile.pos();

    dataFile.close();
}

void ScriptObjectThread::performFileReadLineSimple(const QString& path, quint32 fileOperationNumber)
{
    qint64 seek = 0;

    if (positions.contains(path))
    {
        if (positions[path] == -1)
        {
            emit fileReadResult("", fileOperationNumber);
            return;
        }

        seek = positions[path];
    }

    QFile dataFile(path);

    if (!dataFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        emit fileReadResult("", fileOperationNumber);
        return;
    }

    if (seek > 0 && seek < dataFile.size())
        dataFile.seek(seek);

    QByteArray data = dataFile.readLine();

    if (data.at(data.length() - 1) == '\n')
        data.chop(1);

    emit fileReadResult(data, fileOperationNumber);

    if (dataFile.atEnd())
        positions[path] = -1;
    else
        positions[path] = dataFile.pos();

    dataFile.close();
}

void ScriptObjectThread::performFileRead(const QString& path, qint64 size, quint32 fileOperationNumber)
{
    QFile dataFile(path);

    if (!dataFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        emit fileReadResult("", fileOperationNumber);
        return;
    }

    QByteArray data = dataFile.read(size);
    emit fileReadResult(data, fileOperationNumber);

    dataFile.close();
}

void ScriptObjectThread::performFileReadAll(const QString& path, quint32 fileOperationNumber)
{
    QFile dataFile(path);

    if (!dataFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        emit fileReadResult("", fileOperationNumber);
        return;
    }

    QByteArray data = dataFile.readAll();
    emit fileReadResult(data, fileOperationNumber);

    dataFile.close();
}
