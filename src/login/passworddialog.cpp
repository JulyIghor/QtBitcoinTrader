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

#include "passworddialog.h"
#include "julyrsa.h"
#include "logobutton.h"
#include "main.h"
#include "timesync.h"
#include <QCryptographicHash>
#include <QDesktopServices>
#include <QDir>
#include <QMessageBox>
#include <QSettings>

PasswordDialog::PasswordDialog(QWidget* parent) : QDialog(parent)
{
    resetData = false;
    newProfile = false;
    ui.setupUi(this);
    setWindowTitle(windowTitle() + " v" + baseValues.appVerStr);
    setWindowFlags(Qt::WindowCloseButtonHint | Qt::MSWindowsFixedSizeDialogHint);
    ui.okButton->setEnabled(false);

    QSettings settings(appDataDir + "/QtBitcoinTrader.cfg", QSettings::IniFormat);
    QString lastProfile = settings.value("LastProfile", "").toString();
    int lastProfileIndex = -1;
    int firstUnlockedProfileIndex = -1;

    QMap<int, QString> logosMap;

    if (!JulyRSA::isIniFileSigned(":/Resources/Exchanges/List.ini"))
    {
        QMessageBox::warning(
            nullptr,
            windowTitle(),
            julyTr("PROGRAM_CORRUPTED", "The program is corrupted. Download from the official site https://centrabit.com."));
        exit(0);
    }

    QSettings listSettings(":/Resources/Exchanges/List.ini", QSettings::IniFormat);
    QStringList exchangesList = listSettings.childGroups();

    for (int n = 0; n < exchangesList.size(); n++)
    {
        QString currentLogo = listSettings.value(exchangesList.at(n) + "/Logo").toString();

        if (currentLogo.isEmpty())
            continue;

        logosMap.insert(exchangesList.at(n).toInt(), ":/Resources/Exchanges/Logos/" + currentLogo);
    }

    QStringList scriptsOldPlace = QDir(baseValues.scriptFolder)
                                      .entryList(QStringList() << "*.JLR"
                                                               << "*.JLS");
    QStringList iniNames;

    QStringList settingsList = QDir(appDataDir, "*.ini").entryList();

    for (int n = 0; n < settingsList.size(); n++)
    {
        if (!scriptsOldPlace.empty())
            iniNames << QFileInfo(settingsList.at(n)).completeBaseName();

        QSettings settIni(appDataDir + "/" + settingsList.at(n), QSettings::IniFormat);

        if (settIni.value("EncryptedData/ApiKeySign", "").toString().isEmpty())
        {
            QFile::remove(appDataDir + "/" + settingsList.at(n));
            continue;
        }

        int exchangeId = settIni.value("Profile/ExchangeId", -1).toInt();
        QString currentLogo = logosMap.value(exchangeId);

        if (!QFile::exists(currentLogo))
            currentLogo = ":/Resources/Exchanges/Logos/Unknown.png";

        ui.profileComboBox->addItem(
            QIcon(currentLogo), settIni.value("Profile/Name", QFileInfo(settingsList.at(n)).fileName()).toString(), settingsList.at(n));
        bool isProfLocked = isProfileLocked(settingsList.at(n));

        if (!isProfLocked && lastProfileIndex == -1 && lastProfile == settingsList.at(n))
            lastProfileIndex = n;

        if (firstUnlockedProfileIndex == -1 && !isProfLocked)
            firstUnlockedProfileIndex = n - 1;
    }

    if (!iniNames.empty())
    {
        for (const QString& scriptFolderName : iniNames)
        {
            QDir().mkpath(baseValues.scriptFolder + scriptFolderName);

            for (const QString& curScript : scriptsOldPlace)
                QFile::copy(baseValues.scriptFolder + curScript, baseValues.scriptFolder + scriptFolderName + "/" + curScript);
        }

        for (const QString& curScript : scriptsOldPlace)
            QFile::remove(baseValues.scriptFolder + curScript);
    }

    if (ui.profileComboBox->count() == 0)
        ui.profileComboBox->addItem(julyTr("DEFAULT_PROFILE_NAME", "Default Profile"));

    if (firstUnlockedProfileIndex != -1 && lastProfileIndex == -1)
        lastProfileIndex = firstUnlockedProfileIndex;

    if (lastProfileIndex > -1)
        ui.profileComboBox->setCurrentIndex(lastProfileIndex);

    ui.label_info->setText("Centrabit AG, Zug\nreg. CHE-114.254.375\nVersion: " + baseValues.appVerStr);

    julyTranslator.translateUi(this);

    foreach (QCheckBox* checkBoxes, findChildren<QCheckBox*>())
        checkBoxes->setMinimumWidth(qMin(checkBoxes->maximumWidth(), textFontWidth(checkBoxes->text()) + 20));

    QLayout* groupboxLayout = ui.LogoGroupBox->layout();

    if (groupboxLayout == nullptr)
    {
        groupboxLayout = new QGridLayout;
        groupboxLayout->setContentsMargins(0, 0, 0, 0);
        groupboxLayout->setSpacing(0);
        ui.LogoGroupBox->setLayout(groupboxLayout);
        auto* logoButton = new LogoButton(true);
        groupboxLayout->addWidget(logoButton);
    }

    if (settings.value("HidePasswordDescription", false).toBool())
        ui.descriptionGroupBox->setChecked(false);

    connect(TimeSync::global(), &TimeSync::warningMessage, this, &PasswordDialog::showTimeMessage);
    TimeSync::syncNow();

    QSize minSizeHint = minimumSizeHint();

    if (mainWindow.isValidSize(&minSizeHint))
        setFixedSize(minimumSizeHint());
}

PasswordDialog::~PasswordDialog()
{
}

QString PasswordDialog::lockFilePath(const QString& name)
{
    return baseValues.tempLocation + "/QtBitcoinTrader_lock_" +
           QString(
               QCryptographicHash::hash(QString(appDataDir + "/" + QFileInfo(name).fileName()).toUtf8(), QCryptographicHash::Sha1).toHex());
}

bool PasswordDialog::isProfileLocked(const QString& name)
{
    QString lockFileP = lockFilePath(name);

#ifdef Q_OS_WIN

    if (QFile::exists(lockFileP))
        QFile::remove(lockFileP);

#endif
    return QFile::exists(lockFileP);
}

void PasswordDialog::accept()
{
    QSettings settings(appDataDir + "/QtBitcoinTrader.cfg", QSettings::IniFormat);
    int currIndex = ui.profileComboBox->currentIndex();

    if (currIndex >= 0)
        settings.setValue("LastProfile", ui.profileComboBox->itemData(currIndex).toString());

    QDialog::accept();
}

QString PasswordDialog::getIniFilePath()
{
    int currIndex = ui.profileComboBox->currentIndex();

    if (currIndex == -1)
        return appDataDir + "/QtBitcoinTrader.ini";

    return appDataDir + "/" + ui.profileComboBox->itemData(currIndex).toString();
}

void PasswordDialog::addNewProfile()
{
    newProfile = true;
    accept();
}

QString PasswordDialog::getPassword()
{
    return ui.passwordEdit->text();
}

void PasswordDialog::resetDataSlot()
{
    QMessageBox msgBox(this);
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setWindowTitle(windowTitle());
    msgBox.setText(julyTr("CONFIRM_DELETE_PROFILE", "Are you sure to delete \"%1\" profile?").arg(ui.profileComboBox->currentText()));

    auto buttonYes = msgBox.addButton(julyTr("YES", "Yes"), QMessageBox::YesRole);
    msgBox.addButton(julyTr("NO", "No"), QMessageBox::NoRole);
    msgBox.exec();
    if (msgBox.clickedButton() != buttonYes)
        return;

    resetData = true;

    QString iniToRemove = getIniFilePath();

    if (QFile::exists(iniToRemove))
    {
        QSettings rmSettings(iniToRemove, QSettings::IniFormat);

        if (rmSettings.value("Profile/ExchangeId", -1).toInt() == 0)
        {
            int indexPoint = iniToRemove.lastIndexOf('.');
            QString rmFolder = indexPoint > -1 ? iniToRemove.left(indexPoint) : "";

            if (!rmFolder.isEmpty() && QFile::exists(rmFolder))
            {
                QString qtConfig = rmFolder + "/QtTrader.cfg";

                if (QFile::exists(qtConfig))
                    QFile::remove(qtConfig);

                QStringList qtIniToRemove = QDir(rmFolder).entryList(QStringList() << "*.ini");

                for (const QString& qtIniFile : qtIniToRemove)
                    QFile::remove(rmFolder + "/" + qtIniFile);
            }

            QDir().rmdir(rmFolder);
        }

        QFile::remove(iniToRemove);
        QString scriptFolder = baseValues.scriptFolder + "/" + QFileInfo(iniToRemove).completeBaseName() + "/";

        if (QFile::exists(scriptFolder))
        {
            for (const QString& curFile : QDir(scriptFolder)
                                              .entryList(QStringList() << "*.JLS"
                                                                       << "*.JLR"))
                QFile::remove(scriptFolder + curFile);

            QDir().rmdir(scriptFolder);
        }
    }

    accept();
}

void PasswordDialog::checkToEnableButton(const QString& pass)
{
    ui.okButton->setEnabled(pass.length());
}

void PasswordDialog::on_descriptionGroupBox_toggled(bool /*unused*/)
{
    ui.descriptionGroupBox->setVisible(false);

    QSettings settings(appDataDir + "/QtBitcoinTrader.cfg", QSettings::IniFormat);
    settings.setValue("HidePasswordDescription", true);

    QSize minSizeHint = minimumSizeHint();
    setFixedHeight(minSizeHint.height());
}

void PasswordDialog::showTimeMessage(const QString& message)
{
    QMessageBox::warning(this, julyTr("TIME_ERROR", "Time error"), message);
}
