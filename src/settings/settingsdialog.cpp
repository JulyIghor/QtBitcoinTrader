//  This file is part of Qt Bitcoin Trader
//      https://github.com/JulyIGHOR/QtBitcoinTrader
//  Copyright (C) 2013-2020 July Ighor <julyighor@gmail.com>
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

#include <QMessageBox>
#include "settingsdialog.h"
#include "main.h"

SettingsDialog::SettingsDialog()
    : QDialog()
{
    ui.setupUi(this);
    ui.scrollAreaWidgetContents->setBackgroundRole(QPalette::Base);
    setWindowFlags(Qt::WindowCloseButtonHint | Qt::MSWindowsFixedSizeDialogHint);
    listLayout = new QVBoxLayout;
    setWindowTitle(julyTr("SETTINGS_TITLE", "Settings Dialog"));

    settingsGeneral = new SettingsGeneral(this);
    addDialog(julyTr("GENERAL", "General"), "settings_global_32x32.png", settingsGeneral);
    settingsNetworkProxy = new SettingsNetworkProxy(this);
    addDialog(julyTr("NETWORK", "Network"), "settings_network_32x32.png", settingsNetworkProxy);
    settingsDecimals = new SettingsDecimals(this);
    addDialog(julyTr("DECIMALS", "Decimals"), "settings_decimals_32x32.png", settingsDecimals);

    configureNameList();

    julyTranslator.translateUi(this);
}

SettingsDialog::~SettingsDialog()
{
    delete listLayout;
    listListElement.clear();
    delete settingsGeneral;
    delete settingsNetworkProxy;
    delete settingsDecimals;
}

bool SettingsDialog::isSettingsSaved()
{
    switch (ui.settingsStackedWidget->currentIndex())
    {
        case 0:
            return !settingsGeneral->ui.saveButton->isEnabled();

        case 1:
            return !settingsNetworkProxy->ui.saveButton->isEnabled();

        case 2:
            return !settingsDecimals->ui.saveButton->isEnabled();

        default:
            break;
    }

    return true;
}

void SettingsDialog::settingsSave()
{
    switch (ui.settingsStackedWidget->currentIndex())
    {
        case 0:
            settingsGeneral->on_saveButton_clicked();
            return;

        case 1:
            settingsNetworkProxy->on_saveButton_clicked();
            return;

        case 2:
            settingsDecimals->on_saveButton_clicked();
            return;

        default:
            return;
    }
}

void SettingsDialog::settingsDiscard()
{
    switch (ui.settingsStackedWidget->currentIndex())
    {
        case 0:
            settingsGeneral->on_revertChangesButton_clicked();
            return;

        case 1:
            settingsNetworkProxy->on_revertChangesButton_clicked();
            return;

        case 2:
            settingsDecimals->on_revertChangesButton_clicked();
            return;

        default:
            return;
    }
}

void SettingsDialog::resizeNameList()
{
    quint32 maxWidth = 0;

    for (int i = 0; i < listListElement.size(); i++)
    {
        if (maxWidth < listListElement.at(i)->width)
            maxWidth = listListElement.at(i)->width;
    }

    ui.scrollArea->setFixedWidth(maxWidth + 70);
}

void SettingsDialog::configureNameList()
{
    listLayout->setSpacing(0);
    listLayout->setContentsMargins(5, 5, 5, 5);
    listLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
    ui.scrollAreaWidgetContents->setLayout(listLayout);

    if (listListElement.size())
        listListElement.at(0)->selectedItem();

    resizeNameList();
}

void SettingsDialog::addDialog(QString name, QString icon, QWidget* widget)
{
    ui.settingsStackedWidget->addWidget(widget);

    SettingsDialogListElement* listElement = new SettingsDialogListElement(this, listListElement.size(), name, icon);
    listListElement.append(listElement);
    listLayout->addWidget(listElement);
}

void SettingsDialog::clickOnList(qint32 index)
{
    if (index < 0 || index >= listListElement.size())
        return;

    if (index == ui.settingsStackedWidget->currentIndex())
        return;

    if (listListElement.size() != ui.settingsStackedWidget->count())
        return;

    bool settingsSaved = isSettingsSaved();

    if (!settingsSaved)
    {
        QMessageBox closeMsgBox;
        closeMsgBox.setWindowTitle(julyTr("SETTINGS_MODIFIED", "The settings has been modified"));
        closeMsgBox.setText(julyTr("SETTINGS_WANT_SAVE_CHANGES", "Do you want to save your changes?"));
        closeMsgBox.setStandardButtons(QMessageBox::Save | QMessageBox::No | QMessageBox::Cancel);
        closeMsgBox.setDefaultButton(QMessageBox::Save);
        closeMsgBox.setButtonText(QMessageBox::Save, julyTr("YES", "Save"));
        closeMsgBox.setButtonText(QMessageBox::No, julyTr("NO", "Discard"));
        closeMsgBox.setButtonText(QMessageBox::Cancel, julyTranslator.translateButton("TRCANCEL", "Cancel"));
        closeMsgBox.setIcon(QMessageBox::Warning);

        switch (closeMsgBox.exec())
        {
            case QMessageBox::Save:
                settingsSave();
                break;

            case QMessageBox::Discard:
                settingsDiscard();
                break;

            case QMessageBox::Cancel:
                return;

            default:
                break;
        }
    }

    listListElement.at(ui.settingsStackedWidget->currentIndex())->clearSelection();
    listListElement.at(index)->selectedItem();
    ui.settingsStackedWidget->setCurrentIndex(index);
}

void SettingsDialog::closeEvent(QCloseEvent* event)
{
    bool settingsSaved = isSettingsSaved();

    if (settingsSaved)
        return;

    QMessageBox closeMsgBox;
    closeMsgBox.setWindowTitle(julyTr("SETTINGS_MODIFIED", "The settings has been modified"));
    closeMsgBox.setText(julyTr("SETTINGS_WANT_SAVE_CHANGES", "Do you want to save your changes?"));
    closeMsgBox.setStandardButtons(QMessageBox::Save | QMessageBox::No | QMessageBox::Cancel);
    closeMsgBox.setDefaultButton(QMessageBox::Save);
    closeMsgBox.setButtonText(QMessageBox::Save, julyTr("YES", "Save"));
    closeMsgBox.setButtonText(QMessageBox::No, julyTr("NO", "Discard"));
    closeMsgBox.setButtonText(QMessageBox::Cancel, julyTranslator.translateButton("TRCANCEL", "Cancel"));
    closeMsgBox.setIcon(QMessageBox::Warning);

    switch (closeMsgBox.exec())
    {
        case QMessageBox::Save:
            settingsSave();
            break;

        case QMessageBox::No:
            break;

        case QMessageBox::Cancel:
            event->ignore();
            break;

        default:
            break;
    }
}

void SettingsDialog::disableTranslateButton()
{
    settingsGeneral->ui.showTranslationButton->setEnabled(false);
}
