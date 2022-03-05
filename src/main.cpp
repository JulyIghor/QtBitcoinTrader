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

#include "main.h"
#include "config/config_manager.h"
#include "datafolderchusedialog.h"
#include "exchange/exchange.h"
#include "iniengine.h"
#include "julyaes256.h"
#include "julylockfile.h"
#include "login/allexchangesdialog.h"
#include "login/featuredexchangesdialog.h"
#include "login/newpassworddialog.h"
#include "login/passworddialog.h"
#include "login/qttraderinform.h"
#include "qsystemdetection.h"
#include "timesync.h"
#include "translationdialog.h"
#include "updaterdialog.h"
#include "utils/utils.h"
#include <QApplication>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QLibraryInfo>
#include <QLoggingCategory>
#include <QMessageBox>
#include <QMetaEnum>
#include <QNetworkProxy>
#include <QNetworkProxyFactory>
#include <QProcess>
#include <QSettings>
#include <QStandardPaths>
#include <QStyle>
#include <QStyleFactory>
#include <QThread>
#include <QTranslator>
#include <QUrl>
#include <QUuid>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#ifdef Q_OS_UNIX
#include <initializer_list>
#include <signal.h>
void quitOnSignals(std::initializer_list<int> quitSignals)
{
    auto handler = [](int sig) -> void
    {
        qDebug().noquote() << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << "Shutdown signal received" << sig;
        QCoreApplication::quit();
    };

    sigset_t blocking_mask;
    sigemptyset(&blocking_mask);

    for (auto sig : quitSignals)
        sigaddset(&blocking_mask, sig);

    struct sigaction sa;
    sa.sa_handler = handler;
    sa.sa_mask = blocking_mask;
    sa.sa_flags = 0;

    for (auto sig : quitSignals)
        sigaction(sig, &sa, nullptr);
}
#endif

BaseValues* baseValues_;

BaseValues::BaseValues()
{
    forceDotInSpinBoxes = true;
    scriptsThatUseOrderBookCount = 0;
    trafficSpeed = 0;
    trafficTotal = 0;
    trafficTotalType = 0;
    feeDecimals = 2;
    currentExchange_ = nullptr;
    currentTheme = 0;
    gzipEnabled = true;
    appVerIsBeta = false;
    jlScriptVersion = 1.0;
    appVerStr = "1.4100";
    appVerReal = appVerStr.toDouble();

    if (appVerStr.size() > 4)
    {
        if (appVerStr.size() == 7)
            appVerStr.remove(6, 1);

        appVerStr.insert(4, ".");
    }

    appVerLastReal = appVerReal;

    logThread_ = nullptr;

    highResolutionDisplay = true;
    timeFormat = QLocale().timeFormat(QLocale::LongFormat).replace(" ", "").replace("t", "");
    dateTimeFormat = QLocale().dateFormat(QLocale::ShortFormat) + " " + timeFormat;
    depthCountLimit = 100;
    depthCountLimitStr = "100";
    uiUpdateInterval = 100;
    supportsUtfUI = true;
    debugLevel_ = 0;

#if QT_VERSION < 0x060000
#ifdef Q_WS_WIN

    if (QSysInfo::windowsVersion() <= QSysInfo::WV_XP)
        supportsUtfUI = false;

#endif
#endif

    upArrow = QByteArray::fromBase64("4oaR");
    downArrow = QByteArray::fromBase64("4oaT");

    if (supportsUtfUI)
    {
        upArrowNoUtf8 = upArrow;
        downArrowNoUtf8 = downArrow;
    }
    else
    {
        upArrowNoUtf8 = ">";
        downArrowNoUtf8 = "<";
    }

    httpRequestInterval = 400;
    minimumRequestInterval = 400;
    httpRequestTimeout = 5000;
    minimumRequestTimeout = 5000;
    httpRetryCount = 5;
    apiDownCount = 0;
    groupPriceValue = 0.0;
    defaultHeightForRow_ = 22;

    tempLocation = QStandardPaths::standardLocations(QStandardPaths::TempLocation).first().replace('\\', '/') + "/";
    desktopLocation = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first().replace('\\', '/') + "/";
    logFileName = QLatin1String("QtBitcoinTrader.log");
    iniFileName = QLatin1String("QtBitcoinTrader.ini");

    selectSystemLanguage();
}

void BaseValues::selectSystemLanguage()
{
    QString sysLocale = QLocale().name().toLower();

    if (sysLocale.startsWith("de"))
        defaultLangFile = ":/Resources/Language/German.lng";
    else if (sysLocale.startsWith("fr"))
        defaultLangFile = ":/Resources/Language/French.lng";
    else if (sysLocale.startsWith("zh"))
        defaultLangFile = ":/Resources/Language/Chinese.lng";
    else if (sysLocale.startsWith("ru"))
        defaultLangFile = ":/Resources/Language/Russian.lng";
    else if (sysLocale.startsWith("uk"))
        defaultLangFile = ":/Resources/Language/Ukrainian.lng";
    else if (sysLocale.startsWith("pl"))
        defaultLangFile = ":/Resources/Language/Polish.lng";
    else if (sysLocale.startsWith("nl"))
        defaultLangFile = ":/Resources/Language/Dutch.lng";
    else if (sysLocale.startsWith("es"))
        defaultLangFile = ":/Resources/Language/Spanish.lng";
    else if (sysLocale.startsWith("nb"))
        defaultLangFile = ":/Resources/Language/Norwegian.lng";
    else if (sysLocale.startsWith("bg"))
        defaultLangFile = ":/Resources/Language/Bulgarian.lng";
    else if (sysLocale.startsWith("cs"))
        defaultLangFile = ":/Resources/Language/Czech.lng";
    else if (sysLocale.startsWith("tr"))
        defaultLangFile = ":/Resources/Language/Turkish.lng";
    else if (sysLocale.startsWith("it"))
        defaultLangFile = ":/Resources/Language/Italiano.lng";
    else
        defaultLangFile = ":/Resources/Language/English.lng";
}

void BaseValues::initHiDpi()
{
#ifdef Q_OS_LINUX
    defaultEnableHiDPI = false;
#else
    defaultEnableHiDPI = true;
#endif

    QSettings hiDpiSettings("Centrabit", "Qt Bitcoin Trader");

    if (hiDpiSettings.value("HiDPI", defaultEnableHiDPI).toBool())
    {
        QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
        QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    }
    else
        QApplication::setAttribute(Qt::AA_Use96Dpi);
}

bool BaseValues::initAppDataDir(QApplication& a)
{
    bool portableModeSupported(false);
#ifdef Q_OS_MAC

    if (!a.applicationDirPath().startsWith("/Applications/"))
        portableModeSupported = true;

#endif
#ifdef Q_OS_WIN
    portableModeSupported = true;
#endif
#ifdef QTBUILDTARGETLINUX64
    portableModeSupported = true;
#endif
    QString systemAppDataDir(QStandardPaths::standardLocations(QStandardPaths::AppLocalDataLocation).first());
#ifdef Q_OS_WIN
    systemAppDataDir.replace('\\', '/');
#endif

    if (portableModeSupported)
    {
        QString portableAppDataDir(a.applicationDirPath() + QLatin1String("/QtBitcoinTrader.data"));
#ifdef Q_OS_WIN

        if (QFile::exists(a.applicationDirPath() + QLatin1String("/QtBitcoinTrader")) &&
            QFileInfo(a.applicationDirPath() + QLatin1String("/QtBitcoinTrader")).isDir())
            QFile::rename(a.applicationDirPath() + QLatin1String("/QtBitcoinTrader"), portableAppDataDir);

#endif
        appDataDir = systemAppDataDir;

        if (!QFile::exists(portableAppDataDir) && !QFile::exists(appDataDir))
        {
            julyTranslator.loadFromFile(defaultLangFile);
            DataFolderChuseDialog chuseStorageLocation(appDataDir, portableAppDataDir);

            if (chuseStorageLocation.exec() == QDialog::Rejected)
                return false;

            if (chuseStorageLocation.isPortable)
                QDir().mkdir(portableAppDataDir);
            else
            {
                QDir().mkdir(systemAppDataDir);
                QString installedBin = systemAppDataDir + "/QtBitcoinTrader";
                const QString desktopLocation(QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first());
#ifdef Q_OS_WIN
                installedBin.append(".exe");
#else
                installedBin.append(".bin");
#endif

                QFile::Permissions selfPerms = QFile(a.applicationFilePath()).permissions();

                if (a.applicationFilePath().startsWith(desktopLocation))
                    QFile::rename(a.applicationFilePath(), installedBin);
                else
                    QFile::copy(a.applicationFilePath(), installedBin);

                if (QFile::exists(installedBin))
                {
                    QFile(installedBin).setPermissions(selfPerms);
#ifdef Q_OS_WIN
                    {
                        QString desktopFile(desktopLocation + "/Qt Bitcoin Trader.lnk");
                        QFile::link(installedBin, desktopFile);
                        QProcess proc;
                        proc.startDetached(installedBin, QStringList() << "/installed");
                        proc.waitForStarted(3000);
                        return false;
                    }
#endif
#ifdef Q_OS_LINUX
                    {
                        QString desktopFile(desktopLocation + "/Qt Bitcoin Trader.desktop");
                        QString desktopIconFile(systemAppDataDir + "/QtBitcoinTrader.png");
                        {
                            QByteArray iconData;
                            QFile rF(":/Resources/QtBitcoinTrader.png");
                            rF.open(QFile::ReadOnly);
                            QFile wF(desktopIconFile);

                            if (wF.open(QFile::WriteOnly))
                            {
                                wF.write(rF.readAll());
                                wF.close();
                            }

                            rF.close();
                        }
                        {
                            QFile wF(desktopFile);

                            if (wF.open(QFile::WriteOnly))
                            {
                                wF.write("[Desktop Entry]\n"
                                         "Encoding=UTF-8\n"
                                         "Name=Qt Bitcoin Trader\n"
                                         "GenericName=Secure Multi Trading Client\n"
                                         "Exec=\"" +
                                         installedBin.toUtf8() +
                                         "\"\n"
                                         "Icon=" +
                                         desktopIconFile.toUtf8() +
                                         "\n"
                                         "Terminal=false\n"
                                         "Type=Application\n"
                                         "Categories=Qt;Office;Finance;\n");
                                wF.close();
                            }
                        }
                        QProcess proc;
                        proc.startDetached(installedBin, QStringList() << "/installed");
                        proc.waitForStarted(3000);
                        return false;
                    }
#endif
                }
            }
        }

        if (QFile::exists(portableAppDataDir))
        {
            portableMode = true;
            appDataDir = portableAppDataDir;
        }

        if (!QFile::exists(appDataDir + "/Language"))
            QDir().mkpath(appDataDir + "/Language");

        if (!QFile::exists(appDataDir))
        {
            QMessageBox::warning(
                nullptr, "Qt Bitcoin Trader", julyTr("CAN_NOT_WRITE_TO_FOLDER", "Can not write to folder") + ": \"" + appDataDir + "\"");
            return false;
        }
    }
    else
    {
        appDataDir = systemAppDataDir;
        QString oldAppDataDir = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first() + "/.config/QtBitcoinTrader";

        if (!QFile::exists(appDataDir) && oldAppDataDir != appDataDir && QFile::exists(oldAppDataDir))
        {
            QFile::rename(oldAppDataDir, appDataDir);

            if (QFile::exists(oldAppDataDir))
            {
                if (!QFile::exists(appDataDir))
                    QDir().mkpath(appDataDir);

                QStringList fileList = QDir(oldAppDataDir).entryList();

                for (int n = 0; n < fileList.size(); n++)
                    if (fileList.at(n).length() > 2)
                    {
                        QFile::copy(oldAppDataDir + fileList.at(n), appDataDir + fileList.at(n));

                        if (QFile::exists(oldAppDataDir + fileList.at(n)))
                            QFile::remove(oldAppDataDir + fileList.at(n));
                    }
            }
        }

        if (!QFile::exists(appDataDir))
            QDir().mkpath(appDataDir);

        if (!QFile::exists(appDataDir))
        {
            QMessageBox::warning(
                nullptr, "Qt Bitcoin Trader", julyTr("CAN_NOT_WRITE_TO_FOLDER", "Can not write to folder") + ": \"" + appDataDir + "\"");
            return false;
        }
    }

    return true;
}

void BaseValues::initValues(QApplication& a)
{
    qRegisterMetaType<QAbstractSocket::SocketError>("QAbstractSocket::SocketError");
    qRegisterMetaType<QList<QSslError>>("QList<QSslError>");

    QThread::currentThread()->setObjectName("Main Thread");

    a.setWindowIcon(QIcon(":/Resources/QtBitcoinTrader.png"));
    a.setApplicationVersion(appVerStr);

    QTranslator qTranslator;
    qTranslator.load("qt_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    a.installTranslator(&qTranslator);

#ifdef Q_OS_WIN // DPI Fix
    QFont font = a.font();
    font.setPixelSize(11);
    a.setFont(font);
#endif

    fontMetrics_ = new QFontMetrics(a.font());
    scriptFolder = appDataDir + "/Scripts/";

    if (QFile::exists(a.applicationFilePath() + ".upd"))
        QFile::remove(a.applicationFilePath() + ".upd");

    if (QFile::exists(a.applicationFilePath() + ".bkp"))
        QFile::remove(a.applicationFilePath() + ".bkp");
}

void BaseValues::initThemes(QApplication& a)
{
    a.setStyle(QStyleFactory::create("Fusion"));

    if (QFile::exists(appDataDir + "/Themes"))
    {
        themeFolder = appDataDir + "/Themes";

        if (!QFile::exists(themeFolder + "/Dark.thm"))
            QFile::copy("://Resources/Themes/Dark.thm", themeFolder + "/Dark.thm");

        if (!QFile::exists(themeFolder + "/Light.thm"))
            QFile::copy("://Resources/Themes/Light.thm", themeFolder + "/Light.thm");

        if (!QFile::exists(themeFolder + "/Gray.thm"))
            QFile::copy("://Resources/Themes/Gray.thm", themeFolder + "/Gray.thm");
    }
    else
        themeFolder = "://Resources/Themes";

    appThemeLight.palette = a.palette();
    appThemeDark.palette = a.palette();
    appThemeGray.palette = a.palette();

    appThemeLight.loadTheme("Light");
    appThemeDark.loadTheme("Dark");
    appThemeGray.loadTheme("Gray");
    appTheme = appThemeLight;

    osStyle = a.style()->objectName();

    a.setPalette(appTheme.palette);
    a.setStyleSheet(appTheme.styleSheet);
}

void BaseValues::initSettings()
{
    QSettings settingsMain(appDataDir + "/QtBitcoinTrader.cfg", QSettings::IniFormat);

    settingsMain.beginGroup("Proxy");
    bool proxyEnabled = settingsMain.value("Enabled", true).toBool();
    bool proxyAuto = settingsMain.value("Auto", true).toBool();
    QString proxyHost = settingsMain.value("Host", "127.0.0.1").toString();
    quint16 proxyPort = static_cast<quint16>(settingsMain.value("Port", 1234).toUInt());
    QString proxyUser = settingsMain.value("User", "username").toString();
    QString proxyPassword = settingsMain.value("Password", "password").toString();
    QNetworkProxy::ProxyType proxyType;

    if (settingsMain.value("Type", "HttpProxy").toString() == "Socks5Proxy")
        proxyType = QNetworkProxy::Socks5Proxy;
    else
        proxyType = QNetworkProxy::HttpProxy;

    settingsMain.setValue("Enabled", proxyEnabled);
    settingsMain.setValue("Auto", proxyAuto);
    settingsMain.setValue("Host", proxyHost);
    settingsMain.setValue("Port", proxyPort);
    settingsMain.setValue("User", proxyUser);
    settingsMain.setValue("Password", proxyPassword);
    settingsMain.endGroup();

    QNetworkProxy proxy;

    if (proxyEnabled)
    {
        if (proxyAuto)
        {
            QList<QNetworkProxy> proxyList = QNetworkProxyFactory::systemProxyForQuery(QNetworkProxyQuery(QUrl("https://")));

            if (!proxyList.empty())
                proxy = proxyList.first();
        }
        else
        {
            proxy.setHostName(proxyHost);
            proxy.setUser(proxyUser);
            proxy.setPort(proxyPort);
            proxy.setPassword(proxyPassword);
            proxy.setType(proxyType);
        }

        QNetworkProxy::setApplicationProxy(proxy);
    }

    appVerLastReal = settingsMain.value("Version", 1.0).toDouble();

    if (!qFuzzyCompare(appVerLastReal + 1.0, appVerReal + 1.0))
    {
        settingsMain.setValue("Version", appVerReal);
        QStringList cacheFiles = QDir(appDataDir + "/cache").entryList(QStringList("*.cache"), QDir::Files);

        for (int i = 0; i < cacheFiles.size(); ++i)
            QFile(appDataDir + "/cache/" + cacheFiles.at(i)).remove();

        if (qFuzzyIsNull(appVerLastReal))
            appVerLastReal = appVerReal;
    }

    appVerIsBeta = settingsMain.value("CheckForUpdatesBeta", false).toBool();
    use24HourTimeFormat = settingsMain.value("Use24HourTimeFormat", true).toBool();

    settingsMain.beginGroup("Decimals");
    decimalsAmountMyTransactions = settingsMain.value("AmountMyTransactions", 8).toInt();
    decimalsPriceMyTransactions = settingsMain.value("PriceMyTransactions", 8).toInt();
    decimalsTotalMyTransactions = settingsMain.value("TotalMyTransactions", 8).toInt();
    decimalsAmountOrderBook = settingsMain.value("AmountOrderBook", 8).toInt();
    decimalsPriceOrderBook = settingsMain.value("PriceOrderBook", 8).toInt();
    decimalsTotalOrderBook = settingsMain.value("TotalOrderBook", 8).toInt();
    decimalsAmountLastTrades = settingsMain.value("AmountLastTrades", 8).toInt();
    decimalsPriceLastTrades = settingsMain.value("PriceLastTrades", 8).toInt();
    decimalsTotalLastTrades = settingsMain.value("TotalLastTrades", 8).toInt();
    settingsMain.endGroup();

    {
        if (!QFile::exists(appDataDir + "/Language"))
            QDir().mkpath(appDataDir + "/Language");

        QString langFile = settingsMain.value("LanguageFile", "").toString();

        if (langFile.isEmpty() || !QFile::exists(langFile))
            langFile = defaultLangFile;

        julyTranslator.loadFromFile(langFile);
    }
}

int main(int argc, char* argv[])
{
    if (QSslSocket::sslLibraryVersionString().isEmpty())
    {
        QMessageBox::critical(nullptr, "Qt Bitcoin Trader", julyTr("CANT_LOAD_OPENSSL", "Can't load OpenSSL libraries"));
        return 0;
    }

    QScopedPointer<JulyLockFile> julyLock(nullptr);
    QLoggingCategory::setFilterRules("qt.network.ssl.warning=false");
    baseValues_ = new BaseValues();
    baseValues.initHiDpi();
    QApplication a(argc, argv);

    if (argc > 1)
    {
#ifdef Q_OS_LINUX

        if (a.arguments().contains("/test"))
        {
            qDebug().noquote() << "(-: OK :-)";
            return 0;
        }

#endif

        if (a.arguments().contains("/uninstall"))
        {
            QThread::msleep(2000);
#ifndef Q_OS_MAC
            QString tmpDir = QStandardPaths::standardLocations(QStandardPaths::TempLocation).first();

            if (!a.applicationFilePath().startsWith(tmpDir))
                return 0;

#ifdef Q_OS_WIN
            QString desktopShortcut = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first() + "/Qt Bitcoin Trader.lnk";

            if (QFile::exists(desktopShortcut))
                DeleteFile(reinterpret_cast<LPCWSTR>(desktopShortcut.replace('/', '\\').utf16()));

#else
            QString desktopShortcut =
                QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first() + "/Qt Bitcoin Trader.desktop";

            if (!desktopShortcut.isEmpty() && QFile::exists(desktopShortcut))
                QFile::remove(desktopShortcut);

#endif

#endif
            QDir appdataDir(QStandardPaths::standardLocations(QStandardPaths::AppLocalDataLocation).first());

            if (appdataDir.exists())
                appdataDir.removeRecursively();

            QMessageBox::information(
                nullptr, "Qt Bitcoin Trader", julyTr("QT_BITCOIN_TRADER_UNINSTALLED", "Qt Bitcoin Trader completely uninstalled"));
#ifndef Q_OS_MAC
            QFile::remove(a.applicationFilePath());
#endif
            return 0;
        }

        if (a.arguments().contains("/installed"))
        {
            QMessageBox::information(nullptr,
                                     "Qt Bitcoin Trader",
                                     julyTr("QT_BITCOIN_TRADER_INSTALLED",
                                            "Qt Bitcoin Trader installed into system folder<br>"
                                            "Launch shortcut have been placed in your Desktop folder<br>"
                                            "To uninstall it you can use menu Help->Uninstall<br>"
                                            "You can now delete installation file")
                                         .replace("<br>", "<br><br>"));
        }
    }

    a.setApplicationName("QtBitcoinTrader");

    if (!baseValues.initAppDataDir(a))
        return 0;

    baseValues.initValues(a);
    baseValues.initThemes(a);
    baseValues.initSettings();

    if (argc > 1)
    {
        if (a.arguments().last().startsWith("/checkupdate"))
        {
            QSettings settings(appDataDir + "/QtBitcoinTrader.cfg", QSettings::IniFormat);
            QString langFile = settings.value("LanguageFile", "").toString();

            if (langFile.isEmpty() || !QFile::exists(langFile))
                langFile = baseValues.defaultLangFile;

            julyTranslator.loadFromFile(langFile);
            QTimer::singleShot(1000000, &a, &QCoreApplication::quit);
            UpdaterDialog updater(a.arguments().last() != "/checkupdate");
#ifdef Q_OS_UNIX
            quitOnSignals({SIGHUP, SIGQUIT, SIGABRT, SIGTERM, SIGXCPU, SIGXFSZ, SIGINT});
#endif
            return a.exec();
        }
    }

    IniEngine::global();
    TimeSync::global();

    {
        bool tryDecrypt = true;
        bool showNewPasswordDialog = false;

        while (tryDecrypt)
        {
            QString tryPassword;
            baseValues.restKey.clear();
            baseValues.restSign.clear();
            bool noProfiles = QDir(appDataDir, "*.ini").entryList().isEmpty();

            if (noProfiles || showNewPasswordDialog)
            {
                FeaturedExchangesDialog featuredExchanges;

                if (featuredExchanges.exchangeNum != -3)
                {
                    int execResult = featuredExchanges.exec();

                    if (noProfiles && execResult == QDialog::Rejected)
                        return 0;

                    if (featuredExchanges.exchangeNum != -2)
                        if (execResult == QDialog::Rejected || featuredExchanges.exchangeNum == -1)
                        {
                            showNewPasswordDialog = false;
                            continue;
                        }
                }

                qint32 exchangeNumber = 0;

                if (featuredExchanges.exchangeNum >= 0)
                    exchangeNumber = featuredExchanges.exchangeNum;
                else
                {
                    AllExchangesDialog allExchanges(featuredExchanges.exchangeNum);

                    if (allExchanges.exec() == QDialog::Rejected)
                    {
                        showNewPasswordDialog = false;
                        continue;
                    }

                    if (allExchanges.exchangeNum == -1)
                    {
                        showNewPasswordDialog = false;
                        continue;
                    }

                    if (allExchanges.exchangeNum == -2)
                        continue;

                    exchangeNumber = allExchanges.exchangeNum;
                }

                if (exchangeNumber == 0)
                {
                    QMessageBox msgBox;
                    msgBox.setIcon(QMessageBox::Question);
                    msgBox.setWindowTitle("Qt Bitcoin Trader");
                    msgBox.setText(julyTr("AGREE_CLOSED_SOURCE", "Do you agree to download the enclosed application?"));
                    auto buttonYes = msgBox.addButton(julyTr("YES", "Yes"), QMessageBox::YesRole);
                    msgBox.addButton(julyTr("NO", "No"), QMessageBox::NoRole);
                    msgBox.exec();
                    if (msgBox.clickedButton() != buttonYes)
                        continue;
                }

                NewPasswordDialog newPassword(exchangeNumber);

                if (newPassword.exec() == QDialog::Accepted)
                {
                    tryPassword = newPassword.getPassword();
                    newPassword.updateIniFileName();
                    baseValues.restKey = newPassword.getRestKey().toLatin1();
                    QSettings settings(baseValues.iniFileName, QSettings::IniFormat);
                    settings.setValue("Profile/ExchangeId", newPassword.getExchangeId());
                    settings.sync();

                    if (!QFile::exists(baseValues.iniFileName))
                        QMessageBox::warning(nullptr, "Qt Bitcoin Trader", "Can't write file: \"" + baseValues.iniFileName + "\"");

                    QByteArray encryptedData;

                    switch (newPassword.getExchangeId())
                    {
                    case 0:
                        {
                            // Qt Trader Exchange
                            baseValues.restSign = newPassword.getRestSign().toLatin1();
                            encryptedData =
                                JulyAES256::encrypt("Qt Bitcoin Trader\r\n" + baseValues.restKey + "\r\n" + baseValues.restSign.toBase64() +
                                                        "\r\n" + QUuid::createUuid().toString().toLatin1(),
                                                    tryPassword.toUtf8());
                        }
                        break;

                    case 2:
                        {
                            // Bitstamp
                            baseValues.restSign = newPassword.getRestSign().toLatin1();
                            encryptedData =
                                JulyAES256::encrypt("Qt Bitcoin Trader\r\n" + baseValues.restKey + "\r\n" + baseValues.restSign.toBase64() +
                                                        "\r\n" + QUuid::createUuid().toString().toLatin1(),
                                                    tryPassword.toUtf8());
                        }
                        break;

                    case 4:
                        {
                            // Bitfinex
                            baseValues.restSign = newPassword.getRestSign().toLatin1();
                            encryptedData =
                                JulyAES256::encrypt("Qt Bitcoin Trader\r\n" + baseValues.restKey + "\r\n" + baseValues.restSign.toBase64() +
                                                        "\r\n" + QUuid::createUuid().toString().toLatin1(),
                                                    tryPassword.toUtf8());
                        }
                        break;

                    case 6:
                        {
                            // Indacoin
                            baseValues.restSign = newPassword.getRestSign().toLatin1();
                            encryptedData =
                                JulyAES256::encrypt("Qt Bitcoin Trader\r\n" + baseValues.restKey + "\r\n" + baseValues.restSign.toBase64() +
                                                        "\r\n" + QUuid::createUuid().toString().toLatin1(),
                                                    tryPassword.toUtf8());
                        }
                        break;

                    case 10:
                        {
                            // YObit
                            baseValues.restSign = newPassword.getRestSign().toLatin1();
                            encryptedData =
                                JulyAES256::encrypt("Qt Bitcoin Trader\r\n" + baseValues.restKey + "\r\n" + baseValues.restSign.toBase64() +
                                                        "\r\n" + QUuid::createUuid().toString().toLatin1(),
                                                    tryPassword.toUtf8());
                        }
                        break;

                    case 11:
                        {
                            // Binance
                            baseValues.restSign = newPassword.getRestSign().toLatin1();
                            encryptedData =
                                JulyAES256::encrypt("Qt Bitcoin Trader\r\n" + baseValues.restKey + "\r\n" + baseValues.restSign.toBase64() +
                                                        "\r\n" + QUuid::createUuid().toString().toLatin1(),
                                                    tryPassword.toUtf8());
                        }
                        break;

                    case 12:
                        {
                            // Bittrex
                            baseValues.restSign = newPassword.getRestSign().toLatin1();
                            encryptedData =
                                JulyAES256::encrypt("Qt Bitcoin Trader\r\n" + baseValues.restKey + "\r\n" + baseValues.restSign.toBase64() +
                                                        "\r\n" + QUuid::createUuid().toString().toLatin1(),
                                                    tryPassword.toUtf8());
                        }
                        break;

                    case 13:
                        {
                            // HitBTC
                            baseValues.restSign = newPassword.getRestSign().toLatin1();
                            encryptedData =
                                JulyAES256::encrypt("Qt Bitcoin Trader\r\n" + baseValues.restKey + "\r\n" + baseValues.restSign.toBase64() +
                                                        "\r\n" + QUuid::createUuid().toString().toLatin1(),
                                                    tryPassword.toUtf8());
                        }
                        break;

                    case 14:
                        {
                            // Poloniex
                            baseValues.restSign = newPassword.getRestSign().toLatin1();
                            encryptedData =
                                JulyAES256::encrypt("Qt Bitcoin Trader\r\n" + baseValues.restKey + "\r\n" + baseValues.restSign.toBase64() +
                                                        "\r\n" + QUuid::createUuid().toString().toLatin1(),
                                                    tryPassword.toUtf8());
                        }
                        break;

                    default:
                        break;
                    }

                    settings.setValue("Profile/Name", newPassword.selectedProfileName());
                    settings.setValue("EncryptedData/ApiKeySign", QString(encryptedData.toBase64()));
                    settings.sync();

                    showNewPasswordDialog = false;
                }
                else if (noProfiles)
                    continue;
            }

            PasswordDialog enterPassword;

            if (enterPassword.exec() == QDialog::Rejected)
                return 0;

            if (enterPassword.resetData)
                continue;

            if (enterPassword.newProfile)
            {
                showNewPasswordDialog = true;
                continue;
            }

            tryPassword = enterPassword.getPassword();

            if (!tryPassword.isEmpty())
            {
                baseValues.iniFileName = enterPassword.getIniFilePath();
                baseValues.logFileName = baseValues.iniFileName;
                baseValues.logFileName.replace(".ini", ".log", Qt::CaseInsensitive);

                if (julyLock)
                    julyLock->free();

                julyLock.reset(new JulyLockFile(baseValues.iniFileName, appDataDir));
                bool profileLocked = julyLock->isLocked();

                if (profileLocked)
                {
                    QMessageBox msgBox(nullptr);
                    msgBox.setIcon(QMessageBox::Question);
                    msgBox.setWindowTitle("Qt Bitcoin Trader");
                    msgBox.setText(julyTr(
                        "THIS_PROFILE_ALREADY_USED",
                        "This profile is already used by another instance.<br>API does not allow to run two instances with same key sign pair.<br>Please create new profile if you want to use two instances."));
                    msgBox.setStandardButtons(QMessageBox::Ok);
                    msgBox.setDefaultButton(QMessageBox::Ok);
                    msgBox.exec();

                    tryPassword.clear();
                }

                if (!profileLocked)
                {
                    QSettings settings(baseValues.iniFileName, QSettings::IniFormat);
                    QStringList decryptedList =
                        QString(JulyAES256::decrypt(
                                    QByteArray::fromBase64(settings.value("EncryptedData/ApiKeySign", "").toString().toLatin1()),
                                    tryPassword.toUtf8()))
                            .split("\r\n");

                    if (decryptedList.size() < 3 || decryptedList.first() != "Qt Bitcoin Trader")
                    {
                        decryptedList =
                            QString(JulyAES256::decrypt(
                                        QByteArray::fromBase64(settings.value("EncryptedData/ApiKeySign", "").toString().toLatin1()),
                                        tryPassword.toLatin1()))
                                .split("\r\n");
                    }

                    if (decryptedList.size() >= 3 && decryptedList.first() == "Qt Bitcoin Trader")
                    {
                        baseValues.restKey = decryptedList.at(1).toLatin1();
                        baseValues.restSign = QByteArray::fromBase64(decryptedList.at(2).toLatin1());

                        if (decryptedList.size() == 3)
                        {
                            decryptedList << QUuid::createUuid().toString().toLatin1();
                            settings.setValue(
                                "EncryptedData/ApiKeySign",
                                QString(JulyAES256::encrypt("Qt Bitcoin Trader\r\n" + decryptedList.at(1).toLatin1() + "\r\n" +
                                                                decryptedList.at(2).toLatin1() + "\r\n" + decryptedList.at(3).toLatin1(),
                                                            tryPassword.toUtf8())
                                            .toBase64()));
                            settings.sync();
                        }

                        baseValues.randomPassword = decryptedList.at(3).toLatin1();
                        tryDecrypt = false;
                    }
                }
            }
        }

        baseValues.scriptFolder += QFileInfo(baseValues.iniFileName).completeBaseName() + "/";

        QSettings iniSettings(baseValues.iniFileName, QSettings::IniFormat);

        if (iniSettings.value("Debug/LogEnabled", false).toBool())
            debugLevel = 1;

        iniSettings.setValue("Debug/LogEnabled", debugLevel > 0);
        baseValues.logThread_ = nullptr;

        if (debugLevel)
        {
            baseValues.logThread_ = new LogThread;
            logThread->writeLog("Proxy settings: " + QNetworkProxy::applicationProxy().hostName().toUtf8() + ":" +
                                QByteArray::number(QNetworkProxy::applicationProxy().port()) + " " +
                                QNetworkProxy::applicationProxy().user().toUtf8());
        }

        QSettings settingsMain(appDataDir + "/QtBitcoinTrader.cfg", QSettings::IniFormat);

        if (settingsMain.value("ShowQtTraderInform2", true).toBool())
        {
            QtTraderInform inform;
            int informRez = inform.exec();

            if (inform.dontShowAgain())
            {
                settingsMain.setValue("ShowQtTraderInform2", false);
                settingsMain.sync();
            }

            if (informRez == QDialog::Accepted)
                QDesktopServices::openUrl(QUrl("https://qttrader.com/"));
        }

        ::config = new ConfigManager(slash(appDataDir, "QtBitcoinTrader.ws.cfg"), &a);
        baseValues.mainWindow_ = new QtBitcoinTrader;
    }

    baseValues.mainWindow_->setupClass();

#ifdef Q_OS_UNIX
    quitOnSignals({SIGHUP, SIGQUIT, SIGABRT, SIGTERM, SIGXCPU, SIGXFSZ, SIGINT});
#endif
    int rezult = a.exec();
    return rezult;
}
