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

#include "newpassworddialog.h"
#include "main.h"
#include <QDesktopServices>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QSettings>
#include <QUrl>
#include <QtCore/qmath.h>

static const QRegularExpression validChars("[^a-zA-Z0-9+-=_]");

NewPasswordDialog::NewPasswordDialog(qint32 num) : QDialog()
{
    exchangeNum = num;
    ui.setupUi(this);
    ui.okButton->setEnabled(false);
    setWindowTitle(windowTitle() + " v" + baseValues.appVerStr);
    setWindowFlags(Qt::WindowCloseButtonHint | Qt::MSWindowsFixedSizeDialogHint);
    on_advSettingsTool_toggled(false);
    julyTranslator.translateUi(this);

    QSettings listSettings(":/Resources/Exchanges/List.ini", QSettings::IniFormat);
    QString exchangeIdText = QByteArray::number(exchangeNum);

    if (exchangeIdText.length() == 1)
        exchangeIdText.prepend("0");

    exchangeName = listSettings.value(exchangeIdText + "/Name").toString().replace(' ', "");
    QString logo = listSettings.value(exchangeIdText + "/Logo").toString();
    getApiUrl = listSettings.value(exchangeIdText + "/GetApiUrl").toString();
    clientIdEnabled = listSettings.value(exchangeIdText + "/ClientID", false).toBool();

    setDiffBar(0);

    logo = logo.insert(logo.lastIndexOf("."), "_Big");
    ui.exchangeLogoLabel->setPixmap(QPixmap(":/Resources/Exchanges/Logos/" + logo));
    exchangeChanged(exchangeName);

    QSize minSizeHint = minimumSizeHint();

    if (mainWindow.isValidSize(&minSizeHint))
        setFixedSize(minSizeHint);
}

NewPasswordDialog::~NewPasswordDialog()
{
}

void NewPasswordDialog::on_advSettingsTool_toggled(bool status)
{
    ui.domainLabel->setVisible(status);
    ui.domainLine->setVisible(status);
    ui.clearDomain->setVisible(status);
    ui.portLabel->setVisible(status);
    ui.portSpinBox->setVisible(status);
    ui.sslLabel->setVisible(status);
    ui.sslCheckBox->setVisible(status);

    QSize minSizeHint = minimumSizeHint();

    if (mainWindow.isValidSize(&minSizeHint))
        setFixedSize(minSizeHint);

    checkToEnableButton();
}

void NewPasswordDialog::exchangeChanged(const QString& name)
{
    if (exchangeNum == 0)
    {
        ui.groupBoxApiKeyAndSecret->hide();
        ui.getApiKeySecretButton->hide();
        ui.mainGridLayout->removeWidget(ui.groupBoxApiKeyAndSecret);
        ui.mainGridLayout->removeWidget(ui.profileGroupBox);
        ui.mainGridLayout->addWidget(ui.profileGroupBox, 0, 0, 1, 2);
        return;
    }

    ui.groupBoxApiKeyAndSecret->setTitle(julyTr("API_KEY_AND_SECRET", "%1 API key and Secret").arg(name));

    if (clientIdEnabled)
    {
        ui.labelClientID->setVisible(true);
        ui.clientIdLine->setVisible(true);
        ui.clearClientIdLine->setVisible(true);
    }
    else
    {
        ui.labelClientID->setVisible(false);
        ui.clientIdLine->setVisible(false);
        ui.clearClientIdLine->setVisible(false);
    }
}

QString NewPasswordDialog::getPassword()
{
    return ui.passwordEdit->text();
}

QString NewPasswordDialog::getRestSign()
{
    return ui.restSignLine->text().remove(validChars);
}

QString NewPasswordDialog::getRestKey()
{
    if (clientIdEnabled)
        return ui.clientIdLine->text().remove(validChars) + ":" + ui.restKeyLine->text().remove(validChars); // ClientID visible

    return ui.restKeyLine->text().remove(validChars);
}

void NewPasswordDialog::getApiKeySecretButton()
{
    QDesktopServices::openUrl(QUrl(getApiUrl));
}

int NewPasswordDialog::getExchangeId() const
{
    return exchangeNum;
}

QString NewPasswordDialog::getFileName()
{
    QString fileName = exchangeName + "_" + ui.profileNameEdit->text();
    fileName.replace(' ', '_');
    return fileName;
}

void NewPasswordDialog::setDiffBar(int val)
{
    QString style;
    QString styleWhite = "background: #FFFFFF; border: 1px solid #999999; border-radius: 1px";

    switch (val)
    {
    case 0:
        style = styleWhite;
        break;

    case 1:
        style = "background: #FFAAAA; border: 1px solid #999999; border-radius: 1px";
        break;

    case 2:
        style = "background: #FFFF66; border: 1px solid #999999; border-radius: 1px";
        break;

    case 3:
        style = "background: #66FF66; border: 1px solid #999999; border-radius: 1px";
        break;
    }

    ui.bar1->setStyleSheet(val > 0 ? style : styleWhite);
    ui.bar2->setStyleSheet(val > 1 ? style : styleWhite);
    ui.bar3->setStyleSheet(val > 2 ? style : styleWhite);
}

int NewPasswordDialog::difficulty(const QString& pass, bool* resive_PasswordIsGood, QString* resive_Message)
{
    QString Message = "";
    qint32 diff = 0UL;                 // Difficulty level
    qint32 passLength = pass.length(); // Password length

    if (passLength)
    {
        static QString allowedPassChars = "!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";
        bool containsDigit = false;
        bool containsSpec = false;
        bool containsUpCase = false;
        bool containsDownCase = false;

        for (int n = 0; n < pass.length(); n++)
        {
            if (!containsDigit && pass.at(n).isDigit())
                containsDigit = true;

            if (!containsSpec && allowedPassChars.contains(pass.at(n)))
                containsSpec = true;

            if (!containsUpCase && pass.at(n).isLetter() && pass.at(n).isUpper())
                containsUpCase = true;

            if (!containsDownCase && pass.at(n).isLetter() && pass.at(n).isLower())
                containsDownCase = true;
        }

        qint32 passDifficulty = 0;

        if (containsDownCase)
            passDifficulty += 26;

        if (containsUpCase)
            passDifficulty += 26;

        if (containsDigit)
            passDifficulty += 10;

        if (containsSpec)
            passDifficulty += 12;

        const int blockSize = 3;
        int repeatCount = 0;

        for (int n = 0; n < pass.length() - blockSize; ++n)
        {
            QString fragment = pass.mid(n, blockSize);

            if (pass.indexOf(fragment, n + 1) > -1)
            {
                ++repeatCount;
                n += blockSize - 1;
            }
        }

        passLength -= repeatCount;

        if (passLength > 20)
            passLength = 20;

        if (passDifficulty >= 26 && passLength > 14)
            passLength = 14;

        if (passDifficulty >= 36 && passLength > 13)
            passLength = 13;

        if (passDifficulty >= 52 && passLength > 12)
            passLength = 12;

        if (passDifficulty >= 62 && passLength > 11)
            passLength = 11;

        quint64 PasswordsPerSecond = 500000000;
        double crackTimeD = qPow(passDifficulty, passLength) / PasswordsPerSecond;
        quint64 crackTime = crackTimeD >= std::numeric_limits<quint64>::max() ? std::numeric_limits<quint64>::max() :
                                                                                static_cast<quint64>(crackTimeD);

        if (crackTime >= 1798389)
            *resive_PasswordIsGood = true;
        else
            *resive_PasswordIsGood = false;

        if (crackTime < 60)
        {
            Message = julyTr("ONE_DAY_DIFFICULTY", "Less than 1 day"); // les 1 day

            if (passLength < 3)
                diff = 1;
            else if (crackTime < 1)
                diff = 2;
            else
                diff = 3;
        }
        else
        {
            crackTime /= 60;

            if (crackTime < 180)
            {
                Message = julyTr("ONE_DAY_DIFFICULTY", "Less than 1 day");
                diff = 4;
            }
            else
            {
                crackTime /= 60;

                if (crackTime < 72)
                {
                    Message = julyTr("ONE_DAY_DIFFICULTY", "Less than 1 day");
                    diff = 5;
                }
                else
                {
                    crackTime /= 24;

                    if (crackTime < 90)
                    {
                        Message = julyTr("DAYS_DIFFICULTY", "%1 days").arg(crackTime);
                        diff = 6;
                    }
                    else
                    {
                        crackTime /= 30;

                        if (crackTime < 36)
                        {
                            Message = julyTr("MONTHS_DIFFICULTY", "%1 months").arg(crackTime);
                            diff = 7;
                        }
                        else
                        {
                            crackTime /= 12;

                            if (crackTime < 1000)
                            {
                                Message = julyTr("YEARS_DIFFICULTY", "%1 years").arg(crackTime);

                                if (crackTime < 100)
                                    diff = 8;
                                else
                                    diff = 9;
                            }
                            else
                            {
                                Message = julyTr("100_YEARS_DIFFICULTY", "More than 1000 years");
                                diff = 9999;
                            }
                        }
                    }
                }
            }
        }

        Message = julyTr("TO_CRACK_DIFFICULTY", "%1 to crack").arg(Message);
    }
    else
        Message = "";

    *resive_Message = Message;
    return diff;
}

void NewPasswordDialog::checkToEnableButton()
{
    bool diff_t = false;
    QString difficultyMessage;
    int diffff = difficulty(ui.passwordEdit->text(), &diff_t, &difficultyMessage);

    if (diffff < 4)
        setDiffBar(0);
    else if (diffff < 6)
        setDiffBar(1);
    else if (diffff < 9)
        setDiffBar(2);
    else
        setDiffBar(3);

    ui.label_difficulty->setText(difficultyMessage);

    if (diff_t)
        ui.label_difficulty->setStyleSheet("");
    else
        ui.label_difficulty->setStyleSheet("color: #A00;");

    if (ui.passwordEdit->text() != ui.confirmEdit->text())
    {
        ui.confirmLabel->setStyleSheet("color: #A00;");
        ui.okButton->setEnabled(false);
        return;
    }
    ui.confirmLabel->setStyleSheet("");

    if (!diff_t)
    {
        ui.okButton->setEnabled(false);
        return;
    }

    QString profileName = ui.profileNameEdit->text();

    if (!profileName.isEmpty())
    {
        static QString allowedNameChars = "()+,-.;=@[]^_`{}~ ";
        QString allowedPName;

        for (int n = 0; n < profileName.length(); n++)
            if (profileName.at(n).isLetterOrNumber() || allowedNameChars.contains(profileName.at(n)))
                allowedPName.append(profileName.at(n));

        if (profileName != allowedPName)
            ui.profileNameEdit->setText(allowedPName);
    }

    if (profileName.isEmpty())
    {
        ui.okButton->setEnabled(false);
        return;
    }

    if (ui.groupBoxApiKeyAndSecret->isVisible() &&
        (ui.restSignLine->text().isEmpty() || ui.restKeyLine->text().isEmpty() ||
         (ui.clientIdLine->isVisible() && !ui.advSettingsTool->isChecked() && ui.clientIdLine->text().isEmpty())))
    {
        ui.okButton->setEnabled(false);
        return;
    }

    if (ui.advSettingsTool->isChecked() && ui.domainLine->text().isEmpty())
    {
        ui.okButton->setEnabled(false);
        return;
    }

    ui.okButton->setEnabled(true);
}

bool NewPasswordDialog::isValidPassword()
{
    QString pass = ui.passwordEdit->text();

    if (pass.length() < 8)
        return false;

    if (pass != ui.confirmEdit->text())
        return false;

    ui.confirmLabel->setStyleSheet("");

    static QString allowedPassChars = "!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";
    bool isValidPassword = pass.length() > 14;

    if (!isValidPassword)
    {
        bool containsLetter = false;
        bool containsDigit = false;
        bool containsSpec = false;
        bool containsUpCase = false;
        bool containsDownCase = false;

        for (int n = 0; n < pass.length(); n++)
        {
            if (!containsLetter && pass.at(n).isLetter())
                containsLetter = true;

            if (!containsDigit && pass.at(n).isDigit())
                containsDigit = true;

            if (!containsSpec && allowedPassChars.contains(pass.at(n)))
                containsSpec = true;

            if (!containsUpCase && pass.at(n).isLetter() && pass.at(n).isUpper())
                containsUpCase = true;

            if (!containsDownCase && pass.at(n).isLetter() && pass.at(n).isLower())
                containsDownCase = true;

            if ((containsLetter && containsDigit && containsSpec) ||
                (containsLetter && containsDigit && containsUpCase && containsDownCase))
            {
                isValidPassword = true;
                break;
            }
        }
    }

    return isValidPassword;
}

void NewPasswordDialog::updateIniFileName()
{
    if (ui.profileNameEdit->text().isEmpty())
        baseValues.iniFileName = appDataDir + "/QtBitcoinTrader.ini";
    else
    {
        baseValues.iniFileName = getFileName() + ".ini";
        baseValues.iniFileName.prepend(appDataDir + "/");
    }

    QSettings settings(appDataDir + "/QtBitcoinTrader.cfg", QSettings::IniFormat);
    settings.setValue("LastProfile", QFileInfo(baseValues.iniFileName).fileName());

    if (ui.advSettingsTool->isChecked())
    {
        QSettings iniSettings(baseValues.iniFileName, QSettings::IniFormat);
        iniSettings.setValue("Network/Domain", ui.domainLine->text());
        iniSettings.setValue("Network/Port", ui.portSpinBox->value());
        iniSettings.setValue("Network/SSL", ui.sslCheckBox->isChecked());
    }
}

QString NewPasswordDialog::selectedProfileName()
{
    if (ui.profileNameEdit->text().isEmpty())
        return "Default Profile";

    return ui.profileNameEdit->text();
}

void NewPasswordDialog::okPressed()
{
    bool diff_t;
    QString difficultyMessage;
    difficulty(ui.passwordEdit->text(), &diff_t, &difficultyMessage);

    if (diff_t)
        accept();
    else
        QMessageBox::warning(
            this,
            "Qt Bitcoin Trader",
            julyTranslator.translateLabel(
                "TR00100", "Your password must be at least 8 characters and contain letters, digits, and special characters."));
}
