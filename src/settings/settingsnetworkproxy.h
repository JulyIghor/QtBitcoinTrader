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

#ifndef SETTINGSNETWORKPROXY_H
#define SETTINGSNETWORKPROXY_H

#include "ui_settingsnetworkproxy.h"
#include <QSettings>
#include <QWidget>

class SettingsNetworkProxy : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsNetworkProxy(QWidget* parent);
    ~SettingsNetworkProxy();
    Ui::SettingsNetworkProxy ui;

private:
    QSettings* proxySettings;
    QSettings* networkSettings;
    void allSetStatus();
    void loadProxy();
    void saveProxy();
    void activateProxy();
    void loadNetwork();
    void saveNetwork();
    void activateNetwork();

public slots:
    void on_saveButton_clicked();
    void on_revertChangesButton_clicked();

private slots:
    void on_enabledCheckBox_stateChanged(int);
    void on_autoCheckBox_stateChanged(int);
    void on_restoreDefaultsButton_clicked();
    void anyValueChanged();
};

#endif // NETWORKPROXY_H
