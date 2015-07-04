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

#include "config_manager.h"

#include <QDebug>
#include <QString>
#include <QApplication>
#include <QDesktopWidget>
#include "main.h"


static const QLatin1String CONFIG_PREFIX("Config.");
static const QLatin1String CONFIG_NAME("ConfigName");
static const QLatin1String CONFIG_NAMES("Config/Names");


static QString sectionName(const QString& name)
{
    return CONFIG_PREFIX + name;
}

ConfigManager* config;

// --- ConfigManager ----------------------------------------------------------

ConfigManager::ConfigManager(const QString& configFileName, QObject* parent) :
    QObject     (parent),
    settings    (configFileName, QSettings::IniFormat)
{
    defaultNames.append("Default Workspace");
    static const char geometryBuffer0[]="\x1\xd9\xd0\xcb\0\x2\0\0\0\0\0\0\0\0\0\0\0\0\x6\r\0\0\x3I\0\0\0\0\0\0\0\0\0\0\x6\r\0\0\x3I\0\0\0\0\0\0\0\0\x4\0";
    defaultGeometry.append(QByteArray::fromRawData(&geometryBuffer0[0],50));
    static const char stateBuffer0[]="\0\0\0\xff\0\0\0\0\xfd\0\0\0\x3\0\0\0\0\0\0\0\0\0\0\0\0\xfc\x2\0\0\0\x1\xfc\0\0\0\0\xff\xff\xff\xff\0\0\x1K\0\xff\xff\xff\xfc\x1\0\0\0\x2\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x33\x1\0\0\0\0\xff\xff\xff\xff\0\0\x1K\0\0\x2X\xfc\0\0\0\0\xff\xff\xff\xff\0\0\x1\xe4\0\xff\xff\xff\xfa\0\0\0\x4\x1\0\0\0\x5\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x34\x1\0\0\0\0\xff\xff\xff\xff\0\0\x1W\0\xff\xff\xff\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x35\x1\0\0\0\0\xff\xff\xff\xff\0\0\x1\x32\0\xff\xff\xff\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x36\x1\0\0\0\0\xff\xff\xff\xff\0\0\x1\xe4\0\xff\xff\xff\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x37\x1\0\0\0\0\xff\xff\xff\xff\0\0\x1N\0\xff\xff\xff\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x38\x1\0\0\0\0\xff\xff\xff\xff\0\0\x1\x90\0\xff\xff\xff\0\0\0\x2\0\0\0\0\0\0\0\0\xfc\x1\0\0\0\a\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x31\x1\0\0\0\0\xff\xff\xff\xff\0\0\0\xe0\0\0\x1^\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x32\x1\0\0\0\0\xff\xff\xff\xff\0\0\0W\0\0\0\xa0\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x33\x1\0\0\0\0\xff\xff\xff\xff\0\0\0W\0\0\0\xa0\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x34\x1\0\0\0\0\xff\xff\xff\xff\0\0\0W\0\0\0\xb4\xfb\0\0\0\x10\0\x64\0o\0\x63\0k\0N\0U\0L\0L\x1\0\0\0\0\xff\xff\xff\xff\0\0\0\0\0\xff\xff\xff\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x35\x1\0\0\0\0\xff\xff\xff\xff\0\0\x1G\0\0\x1\xf4\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x36\x1\0\0\0\0\xff\xff\xff\xff\0\0\0\x98\0\0\0\xc8\0\0\0\x3\0\0\0\0\0\0\0\0\xfc\x1\0\0\0\x4\xfc\0\0\0\0\xff\xff\xff\xff\0\0\x1\xc4\0\0\x2\b\xfc\x2\0\0\0\x2\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x37\x1\0\0\0\0\xff\xff\xff\xff\0\0\0\xa2\0\0\0\xdd\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x38\x1\0\0\0\0\xff\xff\xff\xff\0\0\0\x43\0\0\0\x65\xfc\0\0\0\0\xff\xff\xff\xff\0\0\x1\xcf\0\0\x2\b\xfc\x2\0\0\0\x2\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x39\x1\0\0\0\0\xff\xff\xff\xff\0\0\0\xa2\0\0\0\xdd\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x30\x1\0\0\0\0\xff\xff\xff\xff\0\0\0\x43\0\0\0\x65\xfb\0\0\0\x16\0\x64\0o\0\x63\0k\0H\0S\0p\0\x61\0\x63\0\x65\0r\x1\0\0\0\0\xff\xff\xff\xff\0\0\0\0\0\xff\xff\xff\xfc\0\0\0\0\xff\xff\xff\xff\0\0\0\xaa\0\0\x1\x18\xfc\x2\0\0\0\x2\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x31\x1\0\0\0\0\xff\xff\xff\xff\0\0\0;\0\0\0\x8d\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x32\x1\0\0\0\0\xff\xff\xff\xff\0\0\0\x46\0\0\0\x8d\0\0\0\0\0\0\0\0\0\0\0\x4\0\0\0\x4\0\0\0\b\0\0\0\b\xfc\0\0\0\0";
    defaultState.append(QByteArray::fromRawData(&stateBuffer0[0],891));

    defaultNames.append("Default Workspace - low resolution screen");
    static const char geometryBuffer1[]="\x1\xd9\xd0\xcb\0\x2\0\0\xff\xff\xff\xfc\xff\xff\xff\xfc\0\0\x4\x3\0\0\x2\xe7\0\0\0]\0\0\0\x32\0\0\x3\xfa\0\0\x2\xde\0\0\0\0\x2\0\0\0\x4\0";
    defaultGeometry.append(QByteArray::fromRawData(&geometryBuffer1[0],50));
    static const char stateBuffer1[]="\0\0\0\xff\0\0\0\0\xfd\0\0\0\x3\0\0\0\0\0\0\x3\xf2\0\0\x1o\xfc\x2\0\0\0\x1\xfc\0\0\0\x90\0\0\x1o\0\0\x1K\0\xff\xff\xff\xfc\x1\0\0\0\x2\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x33\x1\0\0\0\x4\0\0\x1\x66\0\0\x1\x66\0\0\x2X\xfc\0\0\x1p\0\0\x2\x86\0\0\x1\xd7\0\xff\xff\xff\xfa\0\0\0\x4\x1\0\0\0\x5\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x34\x1\0\0\0\0\xff\xff\xff\xff\0\0\x1W\0\xff\xff\xff\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x35\x1\0\0\0\0\xff\xff\xff\xff\0\0\x1\x32\0\xff\xff\xff\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x36\x1\0\0\0\0\xff\xff\xff\xff\0\0\x1\xd7\0\xff\xff\xff\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x37\x1\0\0\0\0\xff\xff\xff\xff\0\0\x1g\0\xff\xff\xff\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x38\x1\0\0\0\0\xff\xff\xff\xff\0\0\x1\x90\0\xff\xff\xff\0\0\0\x2\0\0\x3\xf8\0\0\0r\xfc\x1\0\0\0\x5\xfc\0\0\0\x4\0\0\0\x9b\0\0\0\x88\0\0\0\xa0\xfa\0\0\0\0\x2\0\0\0\x2\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x32\x1\0\0\0\0\xff\xff\xff\xff\0\0\0W\0\0\0\x65\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x31\x1\0\0\0\x18\0\0\0Y\0\0\0Y\0\0\0\x65\xfc\0\0\0\xa5\0\0\0\xb4\0\0\0\xb4\0\0\0\xfd\xfa\0\0\0\0\x2\0\0\0\x2\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x33\x1\0\0\0\0\xff\xff\xff\xff\0\0\0W\0\0\0\x65\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x34\x1\0\0\0\x18\0\0\0\x65\0\0\0W\0\0\0\x65\xfb\0\0\0\x10\0\x64\0o\0\x63\0k\0N\0U\0L\0L\x1\0\0\x1_\0\0\0\x66\0\0\0\0\0\xff\xff\xff\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x35\x1\0\0\x1\xcb\0\0\x1\x82\0\0\x1t\0\0\x1\xf4\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x36\x1\0\0\x3S\0\0\0\xa9\0\0\0\x98\0\0\0\xc8\0\0\0\x3\0\0\x3\xf8\0\0\0\xc8\xfc\x1\0\0\0\x3\xfc\0\0\0\x4\0\0\x2\b\0\0\x1\xee\0\0\x2\b\xfa\0\0\0\0\x2\0\0\0\x2\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x37\x1\0\0\0\0\xff\xff\xff\xff\0\0\0\xaf\0\0\0\xdd\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x39\x1\0\0\0\0\xff\xff\xff\xff\0\0\0\xaf\0\0\0\xdd\xfc\0\0\x2\x12\0\0\x1\xe4\0\0\x1\x9c\0\0\x2\b\xfc\x2\0\0\0\x2\xfc\0\0\x2\x5\0\0\0h\0\0\0\\\0\0\0~\xfa\0\0\0\0\x1\0\0\0\x2\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x38\x1\0\0\0\0\xff\xff\xff\xff\0\0\x1\x8a\0\0\x2\b\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x30\x1\0\0\x2\x12\0\0\x2\b\0\0\x1\x9c\0\0\x2\b\xfc\0\0\x2s\0\0\0Z\0\0\0\x46\0\0\0\x8d\xfc\x1\0\0\0\x2\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x32\x1\0\0\x2\x12\0\0\0\xda\0\0\0\xaa\0\0\x1\x18\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x31\x1\0\0\x2\xf2\0\0\x1\x4\0\0\0\x94\0\0\x1\x18\xfb\0\0\0\x16\0\x64\0o\0\x63\0k\0H\0S\0p\0\x61\0\x63\0\x65\0r\x1\0\0\x3\xfc\0\0\0\0\0\0\0\0\0\xff\xff\xff\0\0\0\0\0\0\x1o\0\0\0\x4\0\0\0\x4\0\0\0\b\0\0\0\b\xfc\0\0\0\0";
    defaultState.append(QByteArray::fromRawData(&stateBuffer1[0],976));

    defaultNames.append("Monitoring");
    static const char geometryBuffer2[]="\x1\xd9\xd0\xcb\0\x2\0\0\xff\xff\xff\xfc\xff\xff\xff\xfc\0\0\x6\x93\0\0\x4\x1\0\0\0\x99\0\0\0\x82\0\0\x3\xf6\0\0\x2\xfc\0\0\0\0\x2\0\0\0\x6\x90";
    defaultGeometry.append(QByteArray::fromRawData(&geometryBuffer2[0],50));
    static const char stateBuffer2[]="\0\0\0\xff\0\0\0\0\xfd\0\0\0\x3\0\0\0\0\0\0\x6\x82\0\0\x1\xbe\xfc\x2\0\0\0\x2\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x38\x1\0\0\0w\0\0\x1\xbe\0\0\0\xc8\0\xff\xff\xff\xfc\0\0\0\x83\0\0\x1\xca\0\0\0\0\0\xff\xff\xff\xfc\x1\0\0\0\x2\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x33\0\0\0\0\x4\0\0\x1\x66\0\0\x1\x66\0\0\x2X\xfc\0\0\0\x4\0\0\x6\x82\0\0\0\0\0\xff\xff\xff\xfa\xff\xff\xff\xff\x1\0\0\0\x2\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x34\0\0\0\0\0\xff\xff\xff\xff\0\0\x1W\0\xff\xff\xff\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x35\0\0\0\0\0\xff\xff\xff\xff\0\0\x1\x32\0\xff\xff\xff\0\0\0\x2\0\0\x6\x88\0\0\0Y\xfc\x1\0\0\0\x6\xfc\0\0\0\x4\0\0\0\xc8\0\0\0\x88\0\0\0\xc8\xfa\0\0\0\x1\x2\0\0\0\x2\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x32\0\0\0\0\0\xff\xff\xff\xff\0\0\0W\0\0\0\x65\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x31\x1\0\0\0\x18\0\0\0Y\0\0\0Y\0\0\0\x65\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x32\x1\0\0\0\xd2\0\0\x1\x18\0\0\0\xaa\0\0\x1\x18\xfc\0\0\0\xa5\0\0\0\xb4\0\0\0\0\0\xff\xff\xff\xfa\xff\xff\xff\xff\x2\0\0\0\x2\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x33\0\0\0\0\0\xff\xff\xff\xff\0\0\0W\0\0\0\x65\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x34\0\0\0\0\x18\0\0\0\x65\0\0\0W\0\0\0\x65\xfb\0\0\0\x10\0\x64\0o\0\x63\0k\0N\0U\0L\0L\x1\0\0\x1\xf0\0\0\x2\x84\0\0\0\0\0\xff\xff\xff\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x35\x1\0\0\x4z\0\0\x1t\0\0\x1t\0\0\x1\xf4\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x36\x1\0\0\x5\xf4\0\0\0\x98\0\0\0\x98\0\0\0\xc8\0\0\0\x3\0\0\x6\x88\0\0\x1\xac\xfc\x1\0\0\0\x5\xfc\0\0\0\x4\0\0\x2\b\0\0\0\0\0\xff\xff\xff\xfa\xff\xff\xff\xff\x2\0\0\0\x2\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x37\0\0\0\0\0\xff\xff\xff\xff\0\0\0\xaf\0\0\0\xdd\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x39\0\0\0\0\0\xff\xff\xff\xff\0\0\0\xaf\0\0\0\xdd\xfc\0\0\0\x4\0\0\x3\b\0\0\0\0\0\xff\xff\xff\xfc\x2\0\0\0\x2\xfc\0\0\x3\x8b\0\0\0\\\0\0\0\0\0\xff\xff\xff\xfa\xff\xff\xff\xff\x1\0\0\0\x2\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x38\0\0\0\0\0\xff\xff\xff\xff\0\0\x1\x8a\0\0\x2\b\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x30\0\0\0\x2\x12\0\0\x2\b\0\0\x1\x9c\0\0\x2\b\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x31\0\0\0\x2s\0\0\0Z\0\0\0;\0\0\0\x8d\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x36\x1\0\0\0\x4\0\0\x3\x9c\0\0\x1\xd7\0\xff\xff\xff\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x37\x1\0\0\x3\xa6\0\0\x2\xe0\0\0\x1\x61\0\xff\xff\xff\xfb\0\0\0\x16\0\x64\0o\0\x63\0k\0H\0S\0p\0\x61\0\x63\0\x65\0r\x1\0\0\x6\x8c\0\0\0\0\0\0\0\0\0\xff\xff\xff\0\0\0\0\0\0\x1\xbe\0\0\0\x4\0\0\0\x4\0\0\0\b\0\0\0\b\xfc\0\0\0\0";
    defaultState.append(QByteArray::fromRawData(&stateBuffer2[0],953));

    defaultNames.append("Monitoring - low resolution screen");
    static const char geometryBuffer3[]="\x1\xd9\xd0\xcb\0\x2\0\0\xff\xff\xff\xfc\xff\xff\xff\xfc\0\0\x4\x3\0\0\x2\xe7\0\0\0\x4\0\0\0\x19\0\0\x4\a\0\0\x2\xdc\0\0\0\0\x2\0\0\0\x4\0";
    defaultGeometry.append(QByteArray::fromRawData(&geometryBuffer3[0],50));
    static const char stateBuffer3[]="\0\0\0\xff\0\0\0\0\xfd\0\0\0\x3\0\0\0\0\0\0\x3\xf2\0\0\x1\xf8\xfc\x2\0\0\0\x1\xfc\0\0\0w\0\0\x1\xf8\0\0\x1K\0\xff\xff\xff\xfc\x1\0\0\0\x2\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x33\0\0\0\0\x4\0\0\x1\xc9\0\0\x1\x66\0\0\x2X\xfc\0\0\0\x4\0\0\x3\xf2\0\0\x1\xd7\0\xff\xff\xff\xfa\0\0\0\x4\x1\0\0\0\x5\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x34\0\0\0\0\0\xff\xff\xff\xff\0\0\x1W\0\xff\xff\xff\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x35\0\0\0\0\0\xff\xff\xff\xff\0\0\x1\x32\0\xff\xff\xff\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x36\x1\0\0\0\0\xff\xff\xff\xff\0\0\x1\xd7\0\xff\xff\xff\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x37\x1\0\0\0\0\xff\xff\xff\xff\0\0\x1\x61\0\xff\xff\xff\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x38\x1\0\0\0\0\xff\xff\xff\xff\0\0\x1\x90\0\xff\xff\xff\0\0\0\x2\0\0\x3\xf8\0\0\0Y\xfc\x1\0\0\0\a\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x31\x1\0\0\0\x4\0\0\0\x88\0\0\0\x88\0\0\0\xc8\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x32\0\0\0\0\x9b\0\0\0j\0\0\0\x64\0\0\0\xa0\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x33\0\0\0\0\xab\0\0\0\xb4\0\0\0\x99\0\0\0\xfd\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x34\0\0\0\0\xcd\0\0\x1\x2\0\0\0\xb4\0\0\x1\x18\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x35\x1\0\0\0\x92\0\0\x1\x80\0\0\x1t\0\0\x1\xf4\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x36\x1\0\0\x2\x18\0\0\0\x98\0\0\0\x98\0\0\0\xc8\xfb\0\0\0\x10\0\x64\0o\0\x63\0k\0N\0U\0L\0L\x1\0\0\x2\xb6\0\0\x1\x46\0\0\0\0\0\xff\xff\xff\0\0\0\x3\0\0\x3\xf8\0\0\0X\xfc\x1\0\0\0\x4\xfc\0\0\0\x4\0\0\x1\x8a\0\0\0\0\0\xff\xff\xff\xfc\x2\0\0\0\x2\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x37\0\0\0\x1\xd5\0\0\0\xaf\0\0\0\xaf\0\0\0\xdd\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x38\0\0\0\x1\xd5\0\0\0\xf8\0\0\0\x43\0\0\0\x65\xfc\0\0\0\x4\0\0\0\xc1\0\0\0\x94\0\0\x1\x18\xfc\x2\0\0\0\x2\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x39\0\0\0\x1\xd5\0\0\0\xaf\0\0\0\xaf\0\0\0\xdd\xfc\0\0\x2u\0\0\0X\0\0\0;\0\0\0\x8d\xfc\x1\0\0\0\x2\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x31\x1\0\0\0\x4\0\0\0\xc1\0\0\0\x94\0\0\x1\x18\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x30\0\0\0\0\x9e\0\0\x1\x9c\0\0\x1\x9c\0\0\x2\b\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x32\x1\0\0\0\xcb\0\0\x1\b\0\0\0\xaa\0\0\x1\x18\xfb\0\0\0\x16\0\x64\0o\0\x63\0k\0H\0S\0p\0\x61\0\x63\0\x65\0r\x1\0\0\x1\xd9\0\0\x2#\0\0\0\0\0\xff\xff\xff\0\0\0\0\0\0\x1\xf8\0\0\0\x4\0\0\0\x4\0\0\0\b\0\0\0\b\xfc\0\0\0\0";
    defaultState.append(QByteArray::fromRawData(&stateBuffer3[0],891));

    translateDefaultNames();
}

ConfigManager::~ConfigManager()
{
    //
}

void ConfigManager::translateDefaultNames()
{
    defaultNamesTr.clear();
    defaultNamesTr.append(julyTr("DEFAULT_WORKSPACE",defaultNames[0]));
    defaultNamesTr.append(julyTr("LOW_RESOLUTION_SCREEN",defaultNames[1]));
    defaultNamesTr.append(julyTr("MONITORING_HIGH_RESOLUTION_SCREEN",defaultNames[2]));
    defaultNamesTr.append(julyTr("MONITORING_LOW_RESOLUTION_SCREEN",defaultNames[3]));
}

QStringList ConfigManager::getConfigNames()
{
    QStringList names = defaultNamesTr;
    names.append(settings.value(CONFIG_NAMES).toStringList());
    names.removeOne(""); // remove default config
    return names;
}

void ConfigManager::sync()
{
    settings.sync();
}

void ConfigManager::save(const QString& name)
{
    QStringList names = settings.value(CONFIG_NAMES).toStringList();
    if (names.contains(name)) {
        settings.remove(sectionName(name));
        names.removeOne(name);
    }

    names.append(name);
    settings.setValue(CONFIG_NAMES, names);

    settings.beginGroup(sectionName(name));
    settings.setValue(CONFIG_NAME, name);
    settings.setValue("Geometry", baseValues_->mainWindow_->saveGeometry());
    settings.setValue("State", baseValues_->mainWindow_->saveState());
    settings.endGroup();

    settings.sync();
    emit onChanged();
}

void ConfigManager::load(const QString& name)
{
    qint16 index=defaultNamesTr.indexOf(name);
    if(index>-1){
        baseValues_->mainWindow_->restoreGeometry(defaultGeometry[index]);
        baseValues_->mainWindow_->restoreState(defaultState[index]);
        return;
    }

    QStringList names = settings.value(CONFIG_NAMES).toStringList();
    if(names.isEmpty()){if(QApplication::desktop()->screenGeometry().width()<12100)load(defaultNamesTr[1]);return;}
    if(!names.contains(name)){onError("Config not found: "+name);return;}
    if(!settings.childGroups().contains(sectionName(name))){onError("Config section not found: "+name);return;}

    settings.beginGroup(sectionName(name));
    baseValues_->mainWindow_->restoreGeometry(settings.value("Geometry",defaultGeometry[0]).toByteArray());
    baseValues_->mainWindow_->restoreState(settings.value("State",defaultState[0]).toByteArray());
    QString storedName = settings.value(CONFIG_NAME).toString();
    settings.endGroup();
    if (storedName != name) {
        emit onError(QString("Config name dismatch: stored: '%1', expected: '%2'").arg(storedName).arg(name));
    }
}

void ConfigManager::remove(const QString& name)
{
    QStringList names = settings.value(CONFIG_NAMES).toStringList();
    if (!names.contains(name)) {
        onError("Config not found: " + name);
        return;
    }

    settings.remove(sectionName(name));
    names.removeOne(name);

    settings.setValue(CONFIG_NAMES, names);
    settings.sync();
    emit onChanged();
}
