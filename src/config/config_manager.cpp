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

#include "config_manager.h"

#include "main.h"
#include "timesync.h"
#include <QApplication>
#include <QScreen>
#include <QString>

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
    QObject(parent), settings(configFileName, QSettings::IniFormat)
{
    defaultNames.append("Default Workspace");
    static const char geometryBuffer0[] =
        "\x1\xd9\xd0\xcb\0\x2\0\0\xff\xff\xff\xfc\xff\xff\xff\xfc\0\0\x6\x93\0\0\x4\x1\0\0\0\x4\0\0\0\x17\0\0\x6\x11\0\0\x3`\0\0\0\0\x2\0\0\0\x6\x90";
    defaultGeometry.append(QByteArray::fromRawData(&geometryBuffer0[0], sizeof(geometryBuffer0) - 1));
    static const char stateBuffer0[] =
        "\0\0\0\xff\0\0\0\0\xfd\0\0\0\x3\0\0\0\0\0\0\x6\x82\0\0\x2r\xfc\x2\0\0\0\x1\xfc\0\0\0w\0\0\x2r\0\0\x1\\\0\xff\xff\xff\xfc\x1\0\0\0\x2\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x33\x1\0\0\0\x4\0\0\x2\b\0\0\x1\x66\0\xff\xff\xff\xfc\0\0\x2\x12\0\0\x4t\0\0\x2<\0\xff\xff\xff\xfa\0\0\0\x2\x1\0\0\0\x6\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x34\x1\0\0\0\0\xff\xff\xff\xff\0\0\x1W\0\xff\xff\xff\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x35\x1\0\0\0\0\xff\xff\xff\xff\0\0\x1\x32\0\xff\xff\xff\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x36\x1\0\0\0\0\xff\xff\xff\xff\0\0\x1\xd7\0\xff\xff\xff\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x37\x1\0\0\0\0\xff\xff\xff\xff\0\0\x1\x61\0\xff\xff\xff\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x38\x1\0\0\0\0\xff\xff\xff\xff\0\0\x2<\0\xff\xff\xff\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x39\x1\0\0\0\0\xff\xff\xff\xff\0\0\0\x45\0\xff\xff\xff\0\0\0\x2\0\0\x6\x88\0\0\0Y\xfc\x1\0\0\0\a\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x31\x1\0\0\0\x4\0\0\0\xc8\0\0\0\x88\0\0\0\xc8\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x32\x1\0\0\0\xd2\0\0\0\xa0\0\0\0\x64\0\0\0\xa0\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x33\x1\0\0\x1x\0\0\0\xfd\0\0\0\x99\0\0\0\xfd\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x34\x1\0\0\x2{\0\0\x1\x18\0\0\0\xb4\0\0\x1\x18\xfb\0\0\0\x10\0\x64\0o\0\x63\0k\0N\0U\0L\0L\x1\0\0\x3\x99\0\0\0+\0\0\0\0\0\xff\xff\xff\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x35\x1\0\0\x3\xca\0\0\x1\xf4\0\0\x1\x80\0\0\x1\xf4\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x36\x1\0\0\x5\xc4\0\0\0\xc8\0\0\0\x98\0\0\0\xc8\0\0\0\x3\0\0\x6\x88\0\0\0\xf8\xfc\x1\0\0\0\x4\xfc\0\0\0\x4\0\0\x2\b\0\0\x1\xee\0\0\x2\b\xfc\x2\0\0\0\x2\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x37\x1\0\0\x2\xef\0\0\0\xaf\0\0\0\xaf\0\0\0\xdd\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x38\x1\0\0\x3\xa4\0\0\0\x43\0\0\0\x43\0\0\0\x65\xfc\0\0\x2\x12\0\0\x2\b\0\0\x1\xee\0\0\x2\b\xfc\x2\0\0\0\x2\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x39\x1\0\0\x2\xef\0\0\0\xaf\0\0\0\xaf\0\0\0\xdd\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x30\x1\0\0\x3\xa4\0\0\0\x43\0\0\0\x43\0\0\0\x65\xfb\0\0\0\x16\0\x64\0o\0\x63\0k\0H\0S\0p\0\x61\0\x63\0\x65\0r\x1\0\0\x4 \0\0\x1N\0\0\0\0\0\xff\xff\xff\xfc\0\0\x5t\0\0\x1\x18\0\0\0\xaa\0\0\x1\x18\xfc\x2\0\0\0\x2\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x31\x1\0\0\x2\xef\0\0\0o\0\0\0;\0\0\0\x8d\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x32\x1\0\0\x3\x64\0\0\0\x83\0\0\0\x46\0\0\0\x8d\0\0\0\0\0\0\x2r\0\0\0\x4\0\0\0\x4\0\0\0\b\0\0\0\b\xfc\0\0\0\0";
    defaultState.append(QByteArray::fromRawData(&stateBuffer0[0], sizeof(stateBuffer0) - 1));

    defaultNames.append("Default Workspace - low resolution screen");
    static const char geometryBuffer1[] =
        "\x1\xd9\xd0\xcb\0\x2\0\0\xff\xff\xff\xfc\xff\xff\xff\xfc\0\0\x6\x93\0\0\x4\x1\0\0\0\x4\0\0\0\x17\0\0\x6\x11\0\0\x3`\0\0\0\0\x2\0\0\0\x6\x90";
    defaultGeometry.append(QByteArray::fromRawData(&geometryBuffer1[0], sizeof(geometryBuffer1) - 1));
    static const char stateBuffer1[] =
        "\0\0\0\xff\0\0\0\0\xfd\0\0\0\x3\0\0\0\0\0\0\x6\x82\0\0\x2\x89\xfc\x2\0\0\0\x1\xfc\0\0\0\x90\0\0\x2\x89\0\0\x1\\\0\xff\xff\xff\xfc\x1\0\0\0\x2\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x33\x1\0\0\0\x4\0\0\x1\xee\0\0\x1\x66\0\xff\xff\xff\xfc\0\0\x1\xf8\0\0\x4\x8e\0\0\x2<\0\xff\xff\xff\xfa\0\0\0\x2\x1\0\0\0\x6\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x34\x1\0\0\0\0\xff\xff\xff\xff\0\0\x1W\0\xff\xff\xff\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x35\x1\0\0\0\0\xff\xff\xff\xff\0\0\x1\x32\0\xff\xff\xff\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x36\x1\0\0\0\0\xff\xff\xff\xff\0\0\x1\xd7\0\xff\xff\xff\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x37\x1\0\0\0\0\xff\xff\xff\xff\0\0\x1g\0\xff\xff\xff\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x38\x1\0\0\0\0\xff\xff\xff\xff\0\0\x2<\0\xff\xff\xff\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x39\x1\0\0\0\0\xff\xff\xff\xff\0\0\0\x45\0\xff\xff\xff\0\0\0\x2\0\0\x6\x88\0\0\0r\xfc\x1\0\0\0\x5\xfc\0\0\0\x4\0\0\0\xa0\0\0\0\x88\0\0\0\xa0\xfa\0\0\0\0\x2\0\0\0\x2\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x32\x1\0\0\0\0\xff\xff\xff\xff\0\0\0W\0\0\0\x65\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x31\x1\0\0\0\x18\0\0\0Y\0\0\0Y\0\0\0\x65\xfc\0\0\0\xaa\0\0\0\xfd\0\0\0\xb4\0\0\0\xfd\xfa\0\0\0\0\x2\0\0\0\x2\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x33\x1\0\0\0\0\xff\xff\xff\xff\0\0\0W\0\0\0\x65\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x34\x1\0\0\0\x18\0\0\0\x65\0\0\0W\0\0\0\x65\xfb\0\0\0\x10\0\x64\0o\0\x63\0k\0N\0U\0L\0L\x1\0\0\x1\xad\0\0\x2\x17\0\0\0\0\0\xff\xff\xff\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x35\x1\0\0\x3\xca\0\0\x1\xf4\0\0\x1t\0\0\x1\xf4\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x36\x1\0\0\x5\xc4\0\0\0\xc8\0\0\0\x98\0\0\0\xc8\0\0\0\x3\0\0\x6\x88\0\0\0\xc8\xfc\x1\0\0\0\x3\xfc\0\0\0\x4\0\0\x1\xee\0\0\x1\xee\0\0\x2\b\xfa\0\0\0\0\x2\0\0\0\x2\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x37\x1\0\0\0\0\xff\xff\xff\xff\0\0\0\xaf\0\0\0\xdd\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x39\x1\0\0\x3\x38\0\0\0\xaf\0\0\0\xaf\0\0\0\xdd\xfc\0\0\x1\xf8\0\0\x1\x9c\0\0\x1\x9c\0\0\x2\b\xfc\x2\0\0\0\x2\xfc\0\0\x3\x1f\0\0\0i\0\0\0\\\0\0\0~\xfa\0\0\0\0\x2\0\0\0\x2\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x38\x1\0\0\0\0\xff\xff\xff\xff\0\0\0\x43\0\0\0\x65\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x30\x1\0\0\x3\x1f\0\0\0\x65\0\0\0\x43\0\0\0\x65\xfc\0\0\x3\x8e\0\0\0Y\0\0\0\x46\0\0\0\x8d\xfc\x1\0\0\0\x2\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x31\x1\0\0\x1\xf8\0\0\0\x94\0\0\0\x94\0\0\x1\x18\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x32\x1\0\0\x2\x92\0\0\x1\x2\0\0\0\xaa\0\0\x1\x18\xfb\0\0\0\x16\0\x64\0o\0\x63\0k\0H\0S\0p\0\x61\0\x63\0\x65\0r\x1\0\0\x3\x9a\0\0\x2\xf2\0\0\0\0\0\xff\xff\xff\0\0\0\0\0\0\x2\x89\0\0\0\x4\0\0\0\x4\0\0\0\b\0\0\0\b\xfc\0\0\0\0";
    defaultState.append(QByteArray::fromRawData(&stateBuffer1[0], sizeof(stateBuffer1) - 1));

    defaultNames.append("Monitoring");
    static const char geometryBuffer2[] =
        "\x1\xd9\xd0\xcb\0\x2\0\0\xff\xff\xff\xfc\xff\xff\xff\xfc\0\0\x6\x93\0\0\x4\x1\0\0\0\x4\0\0\0\x17\0\0\x6\x11\0\0\x3`\0\0\0\0\x2\0\0\0\x6\x90";
    defaultGeometry.append(QByteArray::fromRawData(&geometryBuffer2[0], sizeof(geometryBuffer2) - 1));
    static const char stateBuffer2[] =
        "\0\0\0\xff\0\0\0\0\xfd\0\0\0\x3\0\0\0\0\0\0\x6\x82\0\0\x1\xaf\xfc\x2\0\0\0\x1\xfc\0\0\0w\0\0\x1\xaf\0\0\x1\t\0\xff\xff\xff\xfc\x1\0\0\0\x2\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x33\0\0\0\0\x4\0\0\x2\b\0\0\x1\x63\0\xff\xff\xff\xfc\0\0\0\x4\0\0\x6\x82\0\0\x2<\0\xff\xff\xff\xfa\0\0\0\x2\x1\0\0\0\x4\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x34\0\0\0\0\0\xff\xff\xff\xff\0\0\x1W\0\xff\xff\xff\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x35\0\0\0\0\0\xff\xff\xff\xff\0\0\x1\x32\0\xff\xff\xff\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x38\x1\0\0\0\0\xff\xff\xff\xff\0\0\x2<\0\xff\xff\xff\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x39\0\0\0\0\0\xff\xff\xff\xff\0\0\0\x45\0\xff\xff\xff\0\0\0\x2\0\0\x6\x88\0\0\0Y\xfc\x1\0\0\0\t\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x31\x1\0\0\0\x4\0\0\0\xc8\0\0\0\x85\0\0\0\xc8\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x32\0\0\0\0\xd2\0\0\0\xa0\0\0\0\x64\0\0\0\xa0\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x33\0\0\0\0\xd2\0\0\0\xfd\0\0\0\x99\0\0\0\xfd\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x34\0\0\0\0\xd2\0\0\x1\x18\0\0\0\xb4\0\0\x1\x18\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x31\x1\0\0\0\xd2\0\0\x1\x18\0\0\0\x94\0\0\x1\x18\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x32\x1\0\0\x1\xf0\0\0\x1\x18\0\0\0\xaa\0\0\x1\x18\xfb\0\0\0\x10\0\x64\0o\0\x63\0k\0N\0U\0L\0L\x1\0\0\x3\xe\0\0\0\xb6\0\0\0\0\0\xff\xff\xff\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x35\x1\0\0\x3\xca\0\0\x1\xf4\0\0\x1t\0\0\x1\xf4\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x36\x1\0\0\x5\xc4\0\0\0\xc8\0\0\0\x98\0\0\0\xc8\0\0\0\x3\0\0\x6\x88\0\0\x1\xbb\xfc\x1\0\0\0\x4\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x36\x1\0\0\0\x4\0\0\x3\xb5\0\0\x1\xd7\0\xff\xff\xff\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x37\x1\0\0\x3\xbf\0\0\x2\xc7\0\0\x1\x61\0\xff\xff\xff\xfc\0\0\x3\xc6\0\0\x2\b\0\0\0\0\0\xff\xff\xff\xfc\x2\0\0\0\x2\xfc\0\0\x2\xb5\0\0\0\xcd\0\0\0\0\0\xff\xff\xff\xfa\xff\xff\xff\xff\x1\0\0\0\x2\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x37\0\0\0\0\0\xff\xff\xff\xff\0\0\x1\xee\0\0\x2\b\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x39\0\0\0\x2\x12\0\0\x2\b\0\0\x1\xee\0\0\x2\b\xfc\0\0\x2\xb5\0\0\x1\x32\0\0\0\0\0\xff\xff\xff\xfa\xff\xff\xff\xff\x1\0\0\0\x2\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x38\0\0\0\0\0\xff\xff\xff\xff\0\0\x1\x8a\0\0\x2\b\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x30\0\0\0\x3q\0\0\x1\x9c\0\0\x1\x9c\0\0\x2\b\xfb\0\0\0\x16\0\x64\0o\0\x63\0k\0H\0S\0p\0\x61\0\x63\0\x65\0r\x1\0\0\x6\x8c\0\0\0\0\0\0\0\0\0\xff\xff\xff\0\0\0\0\0\0\x1\xaf\0\0\0\x4\0\0\0\x4\0\0\0\b\0\0\0\b\xfc\0\0\0\0";
    defaultState.append(QByteArray::fromRawData(&stateBuffer2[0], sizeof(stateBuffer2) - 1));

    defaultNames.append("Monitoring - low resolution screen");
    static const char geometryBuffer3[] =
        "\x1\xd9\xd0\xcb\0\x2\0\0\xff\xff\xff\xfc\xff\xff\xff\xfc\0\0\x6\x93\0\0\x4\x1\0\0\0\x4\0\0\0\x17\0\0\x6\x11\0\0\x3`\0\0\0\0\x2\0\0\0\x6\x90";
    defaultGeometry.append(QByteArray::fromRawData(&geometryBuffer3[0], sizeof(geometryBuffer3) - 1));
    static const char stateBuffer3[] =
        "\0\0\0\xff\0\0\0\0\xfd\0\0\0\x3\0\0\0\0\0\0\x6\x82\0\0\x3\x1a\xfc\x2\0\0\0\x1\xfc\0\0\0w\0\0\x3\x1a\0\0\x1K\0\xff\xff\xff\xfc\x1\0\0\0\x2\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x33\0\0\0\0\x4\0\0\x2\b\0\0\x1\x63\0\xff\xff\xff\xfc\0\0\0\x4\0\0\x6\x82\0\0\x2<\0\xff\xff\xff\xfa\0\0\0\x2\x1\0\0\0\x6\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x34\0\0\0\0\0\xff\xff\xff\xff\0\0\x1W\0\xff\xff\xff\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x35\0\0\0\0\0\xff\xff\xff\xff\0\0\x1\x32\0\xff\xff\xff\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x36\x1\0\0\0\0\xff\xff\xff\xff\0\0\x1\xd7\0\xff\xff\xff\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x37\x1\0\0\0\0\xff\xff\xff\xff\0\0\x1[\0\xff\xff\xff\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x38\x1\0\0\0\0\xff\xff\xff\xff\0\0\x2<\0\xff\xff\xff\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x39\0\0\0\0\0\xff\xff\xff\xff\0\0\0\x45\0\xff\xff\xff\0\0\0\x2\0\0\x6\x88\0\0\0Y\xfc\x1\0\0\0\a\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x31\x1\0\0\0\x4\0\0\0\xc8\0\0\0\x85\0\0\0\xc8\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x32\0\0\0\0\xd2\0\0\0\xa0\0\0\0\x64\0\0\0\xa0\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x33\0\0\0\0\xd2\0\0\0\xfd\0\0\0\x99\0\0\0\xfd\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x34\0\0\0\0\xd2\0\0\x1\x18\0\0\0\xb4\0\0\x1\x18\xfb\0\0\0\x10\0\x64\0o\0\x63\0k\0N\0U\0L\0L\x1\0\0\0\xd2\0\0\x2\xf2\0\0\0\0\0\xff\xff\xff\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x35\x1\0\0\x3\xca\0\0\x1\xf4\0\0\x1\x8c\0\0\x1\xf4\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x36\x1\0\0\x5\xc4\0\0\0\xc8\0\0\0\x98\0\0\0\xc8\0\0\0\x3\0\0\x6\x88\0\0\0P\xfc\x1\0\0\0\x5\xfc\0\0\0\x4\0\0\x1\xee\0\0\0\0\0\xff\xff\xff\xfc\x2\0\0\0\x2\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x37\0\0\0\x2\xef\0\0\0\xaf\0\0\0\xaf\0\0\0\xdd\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x38\0\0\0\x2\xef\0\0\0\xf8\0\0\0\x43\0\0\0\x65\xfc\0\0\0\x4\0\0\x2\b\0\0\0\0\0\xff\xff\xff\xfc\x2\0\0\0\x2\xfb\0\0\0\n\0\x64\0o\0\x63\0k\0\x39\0\0\0\x2\xef\0\0\0\xaf\0\0\0\xaf\0\0\0\xdd\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x30\0\0\0\x3\x82\0\0\0\x65\0\0\0\x43\0\0\0\x65\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x31\x1\0\0\0\x4\0\0\x1\x18\0\0\0\x94\0\0\x1\x18\xfb\0\0\0\f\0\x64\0o\0\x63\0k\0\x31\0\x32\x1\0\0\x1\"\0\0\x1\x18\0\0\0\xaa\0\0\x1\x18\xfb\0\0\0\x16\0\x64\0o\0\x63\0k\0H\0S\0p\0\x61\0\x63\0\x65\0r\x1\0\0\x2@\0\0\x4L\0\0\0\0\0\xff\xff\xff\0\0\0\0\0\0\x3\x1a\0\0\0\x4\0\0\0\x4\0\0\0\b\0\0\0\b\xfc\0\0\0\0";
    defaultState.append(QByteArray::fromRawData(&stateBuffer3[0], sizeof(stateBuffer3) - 1));

    translateDefaultNames();
    lastRestoreState.clear();
    lastRestoreTime = 0;
}

ConfigManager::~ConfigManager()
{
}

void ConfigManager::translateDefaultNames()
{
    defaultNamesTr.clear();
    defaultNamesTr.append(julyTr("DEFAULT_WORKSPACE", defaultNames[0]));
    defaultNamesTr.append(julyTr("LOW_RESOLUTION_SCREEN", defaultNames[1]));
    defaultNamesTr.append(julyTr("MONITORING_HIGH_RESOLUTION_SCREEN", defaultNames[2]));
    defaultNamesTr.append(julyTr("MONITORING_LOW_RESOLUTION_SCREEN", defaultNames[3]));
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

void ConfigManager::save(const QString& name, bool initConfigMenu)
{
    QStringList names = settings.value(CONFIG_NAMES).toStringList();

    if (names.contains(name))
    {
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

    if (initConfigMenu)
        emit onChanged();
}

void ConfigManager::load(const QString& name)
{
    qint16 index = defaultNamesTr.indexOf(name);

    if (index > -1)
    {
        lastRestoreGeometry = defaultGeometry[index];
        lastRestoreState = defaultState[index];
        lastRestoreTime = TimeSync::getTimeT();

        if (baseValues_->mainWindow_->saveGeometry() != defaultGeometry[index])
        {
            baseValues_->mainWindow_->restoreGeometry(defaultGeometry[index]);
        }

        if (baseValues_->mainWindow_->saveState() != defaultState[index])
        {
            baseValues_->mainWindow_->restoreState(defaultState[index]);
        }

        return;
    }

    QStringList names = settings.value(CONFIG_NAMES).toStringList();

    if (names.isEmpty())
    {
        if (!QApplication::screens().isEmpty() && QApplication::screens().first()->geometry().width() < 1210)
            load(defaultNamesTr[1]);
        else
            load(defaultNamesTr[0]);

        return;
    }

    if (!names.contains(name))
    {
        onError("Config not found: " + name);
        return;
    }

    if (!settings.childGroups().contains(sectionName(name)))
    {
        onError("Config section not found: " + name);
        return;
    }

    settings.beginGroup(sectionName(name));
    QByteArray geometry = settings.value("Geometry", defaultGeometry[0]).toByteArray();
    QByteArray state = settings.value("State", defaultState[0]).toByteArray();
    lastRestoreGeometry = geometry;
    lastRestoreState = state;
    lastRestoreTime = TimeSync::getTimeT();

    if (baseValues_->mainWindow_->saveGeometry() != geometry)
    {
        baseValues_->mainWindow_->restoreGeometry(geometry);
    }

    if (baseValues_->mainWindow_->saveState() != state)
    {
        baseValues_->mainWindow_->restoreState(state);
    }

    QString storedName = settings.value(CONFIG_NAME).toString();
    settings.endGroup();

    if (storedName != name)
    {
        emit onError(QString("Config name dismatch: stored: '%1', expected: '%2'").arg(storedName).arg(name));
    }
}

void ConfigManager::restoreState()
{
    if (!lastRestoreState.isEmpty())
    {
        if (TimeSync::getTimeT() - lastRestoreTime < 2)
        {
            if (baseValues_->mainWindow_->saveGeometry() != lastRestoreGeometry)
            {
                baseValues_->mainWindow_->restoreGeometry(lastRestoreGeometry);
            }

            if (baseValues_->mainWindow_->saveState() != lastRestoreState)
            {
                baseValues_->mainWindow_->restoreState(lastRestoreState);
            }
        }
    }

    lastRestoreState.clear();
}

void ConfigManager::remove(const QString& name)
{
    QStringList names = settings.value(CONFIG_NAMES).toStringList();

    if (!names.contains(name))
    {
        onError("Config not found: " + name);
        return;
    }

    settings.remove(sectionName(name));
    names.removeOne(name);

    settings.setValue(CONFIG_NAMES, names);
    settings.sync();
    emit onChanged();
}
