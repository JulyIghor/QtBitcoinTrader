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

#include "updaterdialog.h"
#include "julymath.h"
#include "julyrsa.h"
#include "logobutton.h"
#include "main.h"
#include <QClipboard>
#include <QCryptographicHash>
#include <QDesktopServices>
#include <QFile>
#include <QMessageBox>
#include <QProcess>
#include <QUrl>

#ifdef Q_OS_WIN
#include "windows.h"
#endif

UpdaterDialog::UpdaterDialog(bool fbMess) : QDialog()
{
    forceUpdate = false;

#ifdef Q_OS_WIN
#ifndef QTBUILDTARGETWIN64

#if QT_VERSION < 0x060000
    if (QSysInfo::windowsVersion() > QSysInfo::WV_XP)
#endif
    {
        _SYSTEM_INFO sysinfo;
        GetNativeSystemInfo(&sysinfo);

        if (sysinfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
            forceUpdate = true;
    }

#endif
#endif

    QSettings settings(appDataDir + "/QtBitcoinTrader.cfg", QSettings::IniFormat);
    int updateCheckRetryCount = settings.value("UpdateCheckRetryCount", 0).toInt();
    settings.setValue("UpdateCheckRetryCount", updateCheckRetryCount);

    if (updateCheckRetryCount > 10)
    {
        settings.setValue("UpdateCheckRetryCount", 0);
        updateCheckRetryCount = 0;
    }

    downloaded100 = false;
    feedbackMessage = fbMess;
    stateUpdate = 0;
    autoUpdate = settings.value("AutoUpdate", false).toBool();
    ui.setupUi(this);
    ui.againAutoUpdateCheckBox->setChecked(autoUpdate);
    setWindowFlags(Qt::WindowCloseButtonHint | Qt::WindowStaysOnTopHint);

    for (QGroupBox* groupBox : this->findChildren<QGroupBox*>())
    {
        if (groupBox->accessibleName() == "LOGOBUTTON")
        {
            QLayout* groupboxLayout = groupBox->layout();

            if (groupboxLayout == nullptr)
            {
                groupboxLayout = new QGridLayout;
                groupboxLayout->setContentsMargins(0, 0, 0, 0);
                groupboxLayout->setSpacing(0);
                groupBox->setLayout(groupboxLayout);
                auto* logoButton = new LogoButton(true);
                groupboxLayout->addWidget(logoButton);
            }
        }
    }

    if (updateCheckRetryCount > 3)
        httpGet = new JulyHttp("api.qtbitcointrader.com", nullptr, this, false, false);
    else
        httpGet = new JulyHttp("qbtapi.centrabit.com", nullptr, this, false, false);

    httpGet->noReconnect = true;
    timeOutTimer = new QTimer(this);
    connect(timeOutTimer, &QTimer::timeout, this, &UpdaterDialog::exitSlot);
    connect(httpGet, &JulyHttp::dataReceived, this, &UpdaterDialog::dataReceived);

#ifdef Q_OS_WIN
    QByteArray osString = "Win";

#if QT_VERSION < 0x060000
    if (QSysInfo::windowsVersion() > QSysInfo::WV_XP)
#endif
    {
        _SYSTEM_INFO sysinfo;
        GetNativeSystemInfo(&sysinfo);

        if (sysinfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
            osString = "Win64";
    }

#else
#ifdef Q_OS_MAC
    QByteArray osString = "Mac";
#else
#ifdef Q_OS_LINUX
#ifdef QTBUILDTARGETLINUX64
    QByteArray osString = "Linux64";
#else
    QByteArray osString = "Linux";
#endif
#else
    QByteArray osString = "Undefined";
#endif
#endif
#endif

    QByteArray reqStr = "Beta=";

    if (baseValues.appVerIsBeta)
        reqStr.append("true");
    else
        reqStr.append("false");

    if (forceUpdate)
        reqStr.append("&Version=" + JulyMath::byteArrayFromDouble(baseValues.appVerReal * 100000 - 100, 0));
    else
        reqStr.append("&Version=" + JulyMath::byteArrayFromDouble(baseValues.appVerReal * 100000, 0));

    reqStr.append("&OS=" + osString);
    reqStr.append("&Locale=" + QLocale().name().toLatin1());

    QString md5;
    QFile readSelf(QApplication::applicationFilePath());

    if (readSelf.open(QIODevice::ReadOnly))
    {
        md5 = QCryptographicHash::hash(readSelf.readAll(), QCryptographicHash::Md5).toHex();
        readSelf.close();
    }

    reqStr.append("&MD5=" + md5.toLatin1());
    httpGet->sendData(140, 0, "POST /", reqStr);

    timeOutTimer->start(60000);
}

UpdaterDialog::~UpdaterDialog()
{
    QSettings settings(appDataDir + "/QtBitcoinTrader.cfg", QSettings::IniFormat);
    settings.setValue("AutoUpdate", ui.againAutoUpdateCheckBox->isChecked());
}

QByteArray UpdaterDialog::getMidData(const QString& a, QString b, QByteArray* data)
{
    QByteArray rez;

    if (b.isEmpty())
        b = "\",";

    int startPos = data->indexOf(a.toLatin1(), 0);

    if (startPos > -1)
    {
        int endPos = data->indexOf(b.toLatin1(), startPos + a.length());

        if (endPos > -1)
            rez = data->mid(startPos + a.length(), endPos - startPos - a.length());
    }

    return rez;
}

void UpdaterDialog::dataReceived(QByteArray dataReceived, int reqType, int /*unused*/)
{
    timeOutTimer->stop();

    if (stateUpdate == 0)
    {
        if (dataReceived.size() > 50000)
            exitSlot();

        bool canAutoUpdate = false;
#ifdef Q_OS_MAC
        canAutoUpdate = true;
#endif
#ifdef Q_OS_WIN
        canAutoUpdate = true;
#endif
#ifdef QTBUILDTARGETLINUX64
        canAutoUpdate = true;
#endif

        if (reqType == 140)
        {
            QString os = "Src";
#ifdef Q_OS_LINUX
#ifdef QTBUILDTARGETLINUX64
            os = "Linux64";
#else
            os = "Linux";
#endif
#endif
#ifdef Q_OS_MAC
            os = "Mac";
#endif
#ifdef Q_OS_WIN
            os = "Win";

#if QT_VERSION < 0x060000
            if (QSysInfo::windowsVersion() > QSysInfo::WV_XP)
#endif
            {
                _SYSTEM_INFO sysinfo;
                GetNativeSystemInfo(&sysinfo);

                if (sysinfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
                    os = "Win64";
            }

#endif

            updateVersion = getMidData("Version\":\"", "\"", &dataReceived);

            if (updateVersion.size() > 2)
                updateVersion.insert(1, ".");

            updateSignature = getMidData("Hash\":\"", "\"", &dataReceived);

            if (!updateSignature.isEmpty())
                updateSignature = QByteArray::fromBase64(updateSignature);

            updateChangeLog = getMidData("ChangeLog\":\"", "\"", &dataReceived);
            updateLink = getMidData("Binary\":\"", "\"", &dataReceived).replace("\\/", "/");

            versionSignature = getMidData("VersionHash\":\"", "\"", &dataReceived);

            if (!versionSignature.isEmpty())
                versionSignature = QByteArray::fromBase64(versionSignature);

            QByteArray versionSha1 = QCryptographicHash::hash(
                os.toUtf8() + updateVersion.toUtf8() + updateChangeLog.toUtf8() + updateLink.toUtf8(), QCryptographicHash::Sha1);

            QFile readPublicKey(":/Resources/Public.key");

            if (!readPublicKey.open(QIODevice::ReadOnly))
            {
                QMessageBox::critical(this, windowTitle(), "Public.key is missing");
                return;
            }

            QByteArray publicKey = readPublicKey.readAll();

            QByteArray decrypted = JulyRSA::getSignature(versionSignature, publicKey);

            if (versionSignature.isEmpty() || versionSha1 != decrypted)
            {
                exitSlot();
                return;
            }
        }

        if (reqType == 120)
        {
            QMap<QString, QString> versionsMap;
            QStringList dataList = QString(dataReceived).split("\n");

            for (int n = 0; n < dataList.size(); n++)
            {
                QString varData = dataList.at(n);
                int splitPos = varData.indexOf('=');

                if (splitPos > -1)
                {
                    QString varName = varData.left(splitPos);
                    varData.remove(0, splitPos + 1);
                    versionsMap[varName] = varData;
                }
            }

#ifdef Q_OS_MAC
            QString os = "Mac";
#else
#ifdef Q_OS_WIN
            QString os = "Win32";

#if QT_VERSION < 0x060000
            if (QSysInfo::windowsVersion() > QSysInfo::WV_XP)
#endif
            {
                _SYSTEM_INFO sysinfo;
                GetNativeSystemInfo(&sysinfo);

                if (sysinfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
                    os = "Win64";
            }

#else
#ifdef Q_OS_LINUX
#ifdef QTBUILDTARGETLINUX64
            QString os = "Linux64";
#else
            QString os = "Linux";
#endif
#else
            QString os = "Src";
#endif
#endif
#endif
            updateVersion = versionsMap.value(os + "Ver");
            updateSignature = versionsMap.value(os + "Signature").toLatin1();

            if (!updateSignature.isEmpty())
                updateSignature = QByteArray::fromBase64(updateSignature);

            updateChangeLog = versionsMap.value(os + "ChangeLog");
            updateLink = versionsMap.value(os + "Bin");
        }

        if (!forceUpdate)
            if (updateVersion.toDouble() <= baseValues.appVerReal)
            {
                if (feedbackMessage)
                {
                    QMessageBox msgb;
                    msgb.setWindowFlags(Qt::WindowCloseButtonHint | Qt::WindowStaysOnTopHint);
                    msgb.setWindowTitle("Qt Bitcoin Trader");
                    msgb.setIcon(QMessageBox::Information);
                    msgb.setText(julyTr("UP_TO_DATE", "Your version of Qt Bitcoin Trader is up to date."));
                    msgb.exec();
                }

                exitSlot();
                return;
            }

        ui.againAutoUpdateCheckBox->setChecked(autoUpdate);
        ui.autoUpdateGroupBox->setVisible(canAutoUpdate);
        ui.changeLogText->setHtml(updateChangeLog);
        ui.versionLabel->setText("v" + updateVersion);

        julyTranslator.translateUi(this);
        ui.iconLabel->setPixmap(QPixmap(":/Resources/QtBitcoinTrader.png"));
        QSize minSizeHint = minimumSizeHint();

        if (mainWindow.isValidSize(&minSizeHint))
            setFixedSize(minimumSizeHint());

        if (autoUpdate)
            ui.buttonUpdate->click();
        else
            show();
    }
    else if (stateUpdate == 1)
    {
        downloaded100 = true;
        QByteArray fileSha1 = QCryptographicHash::hash(dataReceived, QCryptographicHash::Sha1);
        QFile readPublicKey(":/Resources/Public.key");

        if (!readPublicKey.open(QIODevice::ReadOnly))
        {
            QMessageBox::critical(this, windowTitle(), "Public.key is missing");
            return;
        }

        QByteArray publicKey = readPublicKey.readAll();
        QByteArray decrypted = JulyRSA::getSignature(updateSignature, publicKey);

        if (decrypted == fileSha1)
        {
            QString curBin = QApplication::applicationFilePath();
            QString updBin = curBin + ".upd";
            QString bkpBin = curBin + ".bkp";

            if (QFile::exists(updBin))
                QFile::remove(updBin);

            if (QFile::exists(bkpBin))
                QFile::remove(bkpBin);

            if (QFile::exists(updBin) || QFile::exists(bkpBin))
            {
                downloadErrorFile(1);
                return;
            }

            {
                QFile wrFile(updBin);

                if (wrFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
                {
                    wrFile.write(dataReceived);
                    wrFile.close();
                }
                else
                {
                    downloadErrorFile(2);
                    return;
                }
            }

            QByteArray fileData;
            {
                QFile opFile(updBin);

                if (opFile.open(QIODevice::ReadOnly))
                    fileData = opFile.readAll();

                opFile.close();
            }

            if (QCryptographicHash::hash(fileData, QCryptographicHash::Sha1) != fileSha1)
            {
                QFile::remove(updBin);
                downloadErrorFile(3);
                return;
            }

#ifdef Q_OS_LINUX
            QFile(updBin).setPermissions(QFile(curBin).permissions());
            {
                QProcess testProc;
                testProc.setProcessChannelMode(QProcess::MergedChannels);
                testProc.start(updBin, QStringList() << "/test");
                testProc.waitForFinished(30000);
                QByteArray testOut = testProc.readAll();

                if (!testOut.contains("(-: OK :-)"))
                {
                    QFile::remove(updBin);
                    QMessageBox::critical(
                        this,
                        windowTitle(),
                        "Looks like new version of app have dependency problems. Please download latest version from http://sourceforge.net/projects/bitcointrader and fix it manually\n\n" +
                            QString::fromUtf8(testOut));
                    downloadErrorFile(11);
                    return;
                }
            }
#endif

            QFile::rename(curBin, bkpBin);

            if (!QFile::exists(bkpBin))
            {
                downloadErrorFile(4);
                return;
            }

            QFile::rename(updBin, curBin);

            if (!QFile::exists(curBin))
            {
                QMessageBox::critical(
                    this,
                    windowTitle(),
                    "Critical error. Please reinstall application. Download it from http://sourceforge.net/projects/bitcointrader/<br>File not exists: " +
                        curBin + "<br>" + updBin);
                downloadErrorFile(5);
                return;
            }

#ifdef Q_OS_MAC
            QFile(curBin).setPermissions(QFile(bkpBin).permissions());
#endif
#ifdef Q_OS_LINUX
            QFile(curBin).setPermissions(QFile(bkpBin).permissions());
#endif

            if (!autoUpdate)
                QMessageBox::information(
                    this,
                    windowTitle(),
                    julyTr("UPDATED_SUCCESSFULLY", "Application updated successfully. Please restart application to apply changes."));

            QSettings settings(appDataDir + "/QtBitcoinTrader.cfg", QSettings::IniFormat);
            settings.setValue("UpdateCheckRetryCount", 0);
            exitSlot();
        }
        else
            downloadErrorFile(12);
    }
}

void UpdaterDialog::exitSlot()
{
    QCoreApplication::quit();
}

void UpdaterDialog::buttonUpdate()
{
    stateUpdate = 1;
    ui.buttonUpdate->setEnabled(false);
    QStringList tempList = updateLink.split("//");

    if (tempList.size() != 2)
    {
        downloadError(6);
        return;
    }

    QString protocol = tempList.first();
    tempList = tempList.last().split("/");

    if (tempList.empty())
    {
        downloadError(7);
        return;
    }

    QString domain = tempList.first();
    int removeLength = domain.length() + protocol.length() + 2;

    if (updateLink.length() <= removeLength)
    {
        downloadError(8);
        return;
    }

    updateLink.remove(0, removeLength);

    httpGetFile = new JulyHttp(domain, nullptr, this, protocol.startsWith("https"), false);
    connect(httpGetFile, &JulyHttp::apiDown, this, &UpdaterDialog::invalidData);
    connect(httpGetFile, &JulyHttp::dataProgress, this, &UpdaterDialog::dataProgress);
    connect(httpGetFile, &JulyHttp::dataReceived, this, &UpdaterDialog::dataReceived);
    httpGetFile->noReconnect = true;

    httpGetFile->sendData(120, 0, "GET " + updateLink.toLatin1());
}

void UpdaterDialog::invalidData(bool err)
{
    if (err)
        downloadErrorFile(9);
}

void UpdaterDialog::downloadError(int val)
{
    if (downloaded100)
        return;

    QMessageBox::warning(this,
                         windowTitle(),
                         julyTr("DOWNLOAD_ERROR", "Download error. Please try again.") + "<br>" + httpGet->errorString() +
                             "<br>CODE: " + QString::number(val));

    QSettings settings(appDataDir + "/QtBitcoinTrader.cfg", QSettings::IniFormat);
    settings.setValue("UpdateCheckRetryCount", settings.value("UpdateCheckRetryCount", 0).toInt() + 1);

    exitSlot();
}

void UpdaterDialog::downloadErrorFile(int val)
{
    if (downloaded100)
        return;

    QMessageBox::warning(this,
                         windowTitle(),
                         julyTr("DOWNLOAD_ERROR", "Download error. Please try again.") + "<br>" + httpGetFile->errorString() +
                             "<br>CODE: " + QString::number(val));

    QSettings settings(appDataDir + "/QtBitcoinTrader.cfg", QSettings::IniFormat);
    settings.setValue("UpdateCheckRetryCount", settings.value("UpdateCheckRetryCount", 0).toInt() + 1);

    exitSlot();
}

void UpdaterDialog::dataProgress(int precent)
{
    if (httpGetFile->getCurrentPacketContentLength() > 300000000)
        downloadErrorFile(10);

    ui.progressBar->setValue(precent);
}
