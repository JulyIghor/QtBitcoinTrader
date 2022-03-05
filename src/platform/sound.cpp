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

#include "sound.h"

#ifdef Q_OS_WIN
#include <windows.h>
#else
#if QT_VERSION <= 0x060000
#include <QSound>
#else
#include <QDebug>
#endif
#endif
#include "procdestructor.h"

namespace Platform
{
    void playSound(const QString& path)
    {
#ifdef Q_OS_WIN
        PlaySound((LPCWSTR)path.utf16(), NULL, SND_ASYNC);
#else
#if QT_VERSION <= 0x060000
        static QSound soundInstance("", nullptr);
        soundInstance.stop();
        soundInstance.play(path);
#else
#ifdef Q_OS_MAC
        std::unique_ptr<QProcess> proc(new QProcess);
        proc->start("afplay", QStringList() << path);
        if (proc->waitForStarted(3000))
        {
            static ProcDestructor procHolder;
            procHolder.addProc(proc.release());
        }
        else
            qDebug().noquote() << "Failed to play" << path;
#else
        std::unique_ptr<QProcess> proc(new QProcess);
        proc->start("play", QStringList() << "-q" << path);
        if (proc->waitForStarted(3000))
        {
            static ProcDestructor procHolder;
            procHolder.addProc(proc.release());
        }
        else
            qDebug().noquote() << "Failed to play" << path;
#endif
#endif
#endif
    }

} // namespace Platform
