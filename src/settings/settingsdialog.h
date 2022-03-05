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

#ifndef SETTINGS_H
#define SETTINGS_H

#include "settingsdecimals.h"
#include "settingsdialoglistelement.h"
#include "settingsgeneral.h"
#include "settingsnetworkproxy.h"
#include "ui_settingsdialog.h"
#include <QDialog>
#include <QStandardItemModel>

class SettingsDialogListElement;

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    SettingsDialog();
    ~SettingsDialog();
    void reject(){};

    void clickOnList(qint32);
    void disableTranslateButton();

private:
    Ui::SettingsDialog ui;
    QVBoxLayout* listLayout;
    QList<SettingsDialogListElement*> listListElement;
    SettingsGeneral* settingsGeneral;
    SettingsNetworkProxy* settingsNetworkProxy;
    SettingsDecimals* settingsDecimals;
    void resizeNameList();
    void configureNameList();
    void addDialog(const QString&, const QString&, QWidget*);
    void closeEvent(QCloseEvent*);
    bool isSettingsSaved();
    void settingsSave();
    void settingsDiscard();
};

#endif // SETTINGS_H
