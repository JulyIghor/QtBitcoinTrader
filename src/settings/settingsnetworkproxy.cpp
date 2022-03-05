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

#include "settingsnetworkproxy.h"
#include "main.h"
#include <QNetworkProxy>
#include <QNetworkProxyFactory>
#include <QUrl>

SettingsNetworkProxy::SettingsNetworkProxy(QWidget* parent) : QWidget(parent)
{
    ui.setupUi(this);
    ui.typeComboBox->addItem("Http", "HttpProxy");
    ui.typeComboBox->addItem("Socks5", "Socks5Proxy");
    proxySettings = new QSettings(appDataDir + "/QtBitcoinTrader.cfg", QSettings::IniFormat);
    networkSettings = new QSettings(baseValues.iniFileName, QSettings::IniFormat);

    loadProxy();
    loadNetwork();

    ui.revertChangesButton->setEnabled(false);
    ui.saveButton->setEnabled(false);
}

SettingsNetworkProxy::~SettingsNetworkProxy()
{
    delete proxySettings;
    delete networkSettings;
}

void SettingsNetworkProxy::allSetStatus()
{
    bool enabledStatus = bool(ui.enabledCheckBox->checkState());
    bool manualProxy = !bool(ui.autoCheckBox->checkState());

    if (!enabledStatus)
        manualProxy = false;

    ui.autoLabel->setEnabled(enabledStatus);
    ui.autoCheckBox->setEnabled(enabledStatus);
    ui.hostLabel->setEnabled(manualProxy);
    ui.hostLineEdit->setEnabled(manualProxy);
    ui.portLabel->setEnabled(manualProxy);
    ui.portLineEdit->setEnabled(manualProxy);
    ui.userLabel->setEnabled(manualProxy);
    ui.userLineEdit->setEnabled(manualProxy);
    ui.passwordLabel->setEnabled(manualProxy);
    ui.passwordLineEdit->setEnabled(manualProxy);
    ui.typeLabel->setEnabled(manualProxy);
    ui.typeComboBox->setEnabled(manualProxy);
}

void SettingsNetworkProxy::on_enabledCheckBox_stateChanged(int /*unused*/)
{
    allSetStatus();
}

void SettingsNetworkProxy::on_autoCheckBox_stateChanged(int /*unused*/)
{
    allSetStatus();
}

void SettingsNetworkProxy::loadProxy()
{
    ui.enabledCheckBox->setChecked(proxySettings->value("Proxy/Enabled", true).toBool());
    ui.autoCheckBox->setChecked(proxySettings->value("Proxy/Auto", true).toBool());
    allSetStatus();
    ui.hostLineEdit->setText(proxySettings->value("Proxy/Host", "127.0.0.1").toString());
    ui.portLineEdit->setText(proxySettings->value("Proxy/Port", "1234").toString());
    ui.userLineEdit->setText(proxySettings->value("Proxy/User", "username").toString());
    ui.passwordLineEdit->setText(proxySettings->value("Proxy/Password", "password").toString());

    if (proxySettings->value("Proxy/Type", "HttpProxy").toString() == "Socks5Proxy")
        ui.typeComboBox->setCurrentIndex(1);
    else
        ui.typeComboBox->setCurrentIndex(0);
}

void SettingsNetworkProxy::saveProxy()
{
    proxySettings->setValue("Proxy/Enabled", ui.enabledCheckBox->isChecked());
    proxySettings->setValue("Proxy/Auto", ui.autoCheckBox->isChecked());
    proxySettings->setValue("Proxy/Host", ui.hostLineEdit->text());
    proxySettings->setValue("Proxy/Port", ui.portLineEdit->text());
    proxySettings->setValue("Proxy/User", ui.userLineEdit->text());
    proxySettings->setValue("Proxy/Password", ui.passwordLineEdit->text());
    proxySettings->setValue("Proxy/Type", ui.typeComboBox->itemData(ui.typeComboBox->currentIndex()).toString());
}

void SettingsNetworkProxy::activateProxy()
{
    if (ui.enabledCheckBox->checkState() == Qt::Checked)
    {
        QNetworkProxy proxy;

        if (ui.autoCheckBox->checkState() == Qt::Checked)
        {
            QList<QNetworkProxy> proxyList = QNetworkProxyFactory::systemProxyForQuery(QNetworkProxyQuery(QUrl("https://")));

            if (!proxyList.empty())
                proxy = proxyList.first();
        }
        else
        {
            proxy.setHostName(ui.hostLineEdit->text());
            proxy.setPort(ui.portLineEdit->text().toUInt());
            proxy.setUser(ui.userLineEdit->text());
            proxy.setPassword(ui.passwordLineEdit->text());

            if (ui.typeComboBox->currentIndex() == 1)
                proxy.setType(QNetworkProxy::Socks5Proxy);
            else
                proxy.setType(QNetworkProxy::HttpProxy);
        }

        QNetworkProxy::setApplicationProxy(proxy);
    }
    else
        QNetworkProxy::setApplicationProxy(QNetworkProxy());
}

void SettingsNetworkProxy::loadNetwork()
{
    ui.apiDownCounterMaxSpinBox->setValue(networkSettings->value("Network/ApiDownCounterMax", 5).toInt());
    ui.httpRetryCountSpinBox->setValue(networkSettings->value("Network/HttpRetryCount", 8).toInt());
    ui.httpRequestsIntervalSpinBox->setValue(
        networkSettings->value("Network/HttpRequestsInterval", baseValues.minimumRequestInterval).toInt());
    ui.httpRequestsTimeoutSpinBox->setValue(
        networkSettings->value("Network/HttpRequestsTimeout", baseValues.minimumRequestTimeout).toInt());
}

void SettingsNetworkProxy::saveNetwork()
{
    networkSettings->setValue("Network/ApiDownCounterMax", ui.apiDownCounterMaxSpinBox->value());
    networkSettings->setValue("Network/HttpRetryCount", ui.httpRetryCountSpinBox->value());
    networkSettings->setValue("Network/HttpRequestsInterval", ui.httpRequestsIntervalSpinBox->value());
    networkSettings->setValue("Network/HttpRequestsTimeout", ui.httpRequestsTimeoutSpinBox->value());
}

void SettingsNetworkProxy::activateNetwork()
{
    baseValues.apiDownCount = ui.apiDownCounterMaxSpinBox->value();
    baseValues.httpRetryCount = ui.httpRetryCountSpinBox->value();
    baseValues.httpRequestInterval = ui.httpRequestsIntervalSpinBox->value();
    baseValues.httpRequestTimeout = ui.httpRequestsTimeoutSpinBox->value();
}

void SettingsNetworkProxy::on_revertChangesButton_clicked()
{
    loadProxy();
    loadNetwork();
    ui.revertChangesButton->setEnabled(false);
    ui.saveButton->setEnabled(false);
}

void SettingsNetworkProxy::on_restoreDefaultsButton_clicked()
{
    ui.enabledCheckBox->setChecked(true);
    ui.autoCheckBox->setChecked(true);
    allSetStatus();
    ui.hostLineEdit->setText("127.0.0.1");
    ui.portLineEdit->setText("1234");
    ui.userLineEdit->setText("username");
    ui.passwordLineEdit->setText("password");
    ui.typeComboBox->setCurrentIndex(0);

    ui.apiDownCounterMaxSpinBox->setValue(5);
    ui.httpRetryCountSpinBox->setValue(8);
    ui.httpRequestsIntervalSpinBox->setValue(baseValues.minimumRequestInterval);
    ui.httpRequestsTimeoutSpinBox->setValue(baseValues.minimumRequestTimeout);

    ui.restoreDefaultsButton->setEnabled(false);
}

void SettingsNetworkProxy::on_saveButton_clicked()
{
    activateProxy();
    saveProxy();
    activateNetwork();
    saveNetwork();

    ui.revertChangesButton->setEnabled(false);
    ui.saveButton->setEnabled(false);
}

void SettingsNetworkProxy::anyValueChanged()
{
    if (!ui.revertChangesButton->isEnabled())
        ui.revertChangesButton->setEnabled(true);

    if (!ui.restoreDefaultsButton->isEnabled())
        ui.restoreDefaultsButton->setEnabled(true);

    if (!ui.saveButton->isEnabled())
        ui.saveButton->setEnabled(true);
}
