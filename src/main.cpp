//  This file is part of Qt Bitcion Trader
//      https://github.com/JulyIGHOR/QtBitcoinTrader
//  Copyright (C) 2013-2014 July IGHOR <julyighor@gmail.com>
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

#include <QUrl>
#include <QNetworkProxyFactory>
#include <QNetworkProxy>
#include <QDir>
#include <QPlastiqueStyle>
#include "main.h"
#include <QtGui/QApplication>
#include <QFileInfo>
#include <QSettings>
#include "updaterdialog.h"
#include "passworddialog.h"
#include "newpassworddialog.h"
#include "julyaes256.h"
#include <QTextCodec>
#include <QDesktopServices>
#include "translationdialog.h"
#include <QMessageBox>
#include <QDateTime>
#include <QStyleFactory>
#include "datafolderchusedialog.h"
#include <QMetaEnum>
#include <QUuid>

BaseValues *baseValues_;

void selectSystemLanguage()
{
	QString sysLocale=QLocale().name().toLower();
	if(sysLocale.startsWith("de"))baseValues.defaultLangFile=":/Resources/Language/German.lng";
	else 
	if(sysLocale.startsWith("fr"))baseValues.defaultLangFile=":/Resources/Language/French.lng";
	else 
	if(sysLocale.startsWith("zh"))baseValues.defaultLangFile=":/Resources/Language/Chinese.lng";
	else 
	if(sysLocale.startsWith("ru"))baseValues.defaultLangFile=":/Resources/Language/Russian.lng";
	else 
	if(sysLocale.startsWith("uk"))baseValues.defaultLangFile=":/Resources/Language/Ukrainian.lng";
	else 
	if(sysLocale.startsWith("pl"))baseValues.defaultLangFile=":/Resources/Language/Polish.lng";
	else 
	if(sysLocale.startsWith("nl"))baseValues.defaultLangFile=":/Resources/Language/Dutch.lng";
	else 
	if(sysLocale.startsWith("es"))baseValues.defaultLangFile=":/Resources/Language/Spanish.lng";
	else 
	if(sysLocale.startsWith("nb"))baseValues.defaultLangFile=":/Resources/Language/Norwegian.lng";
	else 
	if(sysLocale.startsWith("bg"))baseValues.defaultLangFile=":/Resources/Language/Bulgarian.lng";
	else 
	if(sysLocale.startsWith("cs"))baseValues.defaultLangFile=":/Resources/Language/Czech.lng";
	else 
	if(sysLocale.startsWith("tr"))baseValues.defaultLangFile=":/Resources/Language/Turkish.lng";
	else 
	if(sysLocale.startsWith("it"))baseValues.defaultLangFile=":/Resources/Language/Italiano.lng";
	else baseValues.defaultLangFile=":/Resources/Language/English.lng";
}

void BaseValues::Construct()
{
	lastGroupID=0;
	forceDotInSpinBoxes=true;
	trafficSpeed=0;
	trafficTotal=0;
	trafficTotalType=0;
	currentExchange_=0;
	nightMode=false;
	rulesSafeMode=true;
	rulesSafeModeInterval=5000;
	gzipEnabled=true;
	appVerIsBeta=false;
	appVerStr="1.0798";
	appVerReal=appVerStr.toDouble();
	if(appVerStr.size()>4)
	{ 
		if(appVerStr.size()==7)appVerStr.remove(6,1);
		appVerStr.insert(4,".");
		if(appVerStr.at(appVerStr.size()-1)!='0')appVerIsBeta=true;
	}
	appVerLastReal=appVerReal;

	logThread=0;

	highResolutionDisplay=true;
	timeFormat=QLocale().timeFormat(QLocale::LongFormat).replace(" ","").replace("t","");
	dateTimeFormat=QLocale().dateFormat(QLocale::ShortFormat)+" "+timeFormat;
	depthCountLimit=100;
	depthCountLimitStr="100";
	uiUpdateInterval=100;
	supportsUtfUI=true;
	debugLevel_=0;

#ifdef Q_WS_WIN
	if(QSysInfo::windowsVersion()<=QSysInfo::WV_XP)supportsUtfUI=false;
#endif	

	upArrow=QByteArray::fromBase64("4oaR");
	downArrow=QByteArray::fromBase64("4oaT");

	if(baseValues.supportsUtfUI)
	{
		upArrowNoUtf8=upArrow;
		downArrowNoUtf8=downArrow;
	}
	else
	{
		upArrowNoUtf8=">";
		downArrowNoUtf8="<";
	}

	httpRequestInterval=400;
	httpRequestTimeout=5000;
	httpSplitPackets=true;
	httpRetryCount=5;
	apiDownCount=0;
	groupPriceValue=0.0;
	defaultHeightForRow_=22;

	selectSystemLanguage();
}

int main(int argc, char *argv[])
{
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("utf8"));
	QTextCodec::setCodecForTr(QTextCodec::codecForName("utf8"));

	baseValues_=new BaseValues;
	baseValues.Construct();

	QApplication a(argc,argv);
	a.setApplicationName("QtBitcoinTrader");
	a.setApplicationVersion(baseValues.appVerStr);

	baseValues.appThemeLight.palette=a.palette();
	baseValues.appThemeDark.palette=a.palette();

	baseValues.appThemeLight.loadTheme("Light");
	baseValues.appThemeDark.loadTheme("Dark");
	baseValues.appTheme=baseValues.appThemeLight;

	baseValues_->fontMetrics_=new QFontMetrics(a.font());

#ifdef Q_OS_WIN
	{
	QString appLocalStorageDir=a.applicationDirPath()+QLatin1String("/QtBitcoinTrader/");
	appDataDir=QDesktopServices::storageLocation(QDesktopServices::DataLocation).replace("\\","/").toAscii()+"/";
	if(!QFile::exists(appLocalStorageDir)&&!QFile::exists(appDataDir))
	{
		julyTranslator.loadFromFile(baseValues.defaultLangFile);
		DataFolderChuseDialog chuseStorageLocation(appDataDir,appLocalStorageDir);
		if(chuseStorageLocation.exec()==QDialog::Rejected)return 0;
		if(chuseStorageLocation.isPortable)QDir().mkdir(appLocalStorageDir);
	}
	if(QFile::exists(appLocalStorageDir))
	{
		appDataDir=appLocalStorageDir.toAscii();
		QDir().mkpath(appDataDir+"Language");
		if(!QFile::exists(appDataDir+"Language"))appDataDir.clear();
	}
	if(!QFile::exists(appDataDir+"Language"))QDir().mkpath(appDataDir+"Language");
	}
#else
	appDataDir=QDesktopServices::storageLocation(QDesktopServices::DataLocation).replace("\\","/").toAscii()+"/QtBitcoinTrader/";
	QString oldAppDataDir=QDesktopServices::storageLocation(QDesktopServices::HomeLocation).toAscii()+"/.config/QtBitcoinTrader/";

	if(!QFile::exists(appDataDir)&&oldAppDataDir!=appDataDir&&QFile::exists(oldAppDataDir))
	{
		QFile::rename(oldAppDataDir,appDataDir);
		if(QFile::exists(oldAppDataDir))
		{
			if(!QFile::exists(appDataDir))QDir().mkpath(appDataDir);
			QStringList fileList=QDir(oldAppDataDir).entryList();
			for(int n=0;n<fileList.count();n++)
				if(fileList.at(n).length()>2)
				{
					QFile::copy(oldAppDataDir+fileList.at(n),appDataDir+fileList.at(n));
					if(QFile::exists(oldAppDataDir+fileList.at(n)))
						QFile::remove(oldAppDataDir+fileList.at(n));
				}
		}
	}

	if(!QFile::exists(appDataDir))QDir().mkpath(appDataDir);
#endif
	if(baseValues.appVerLastReal<1.0763)
	{
		QFile::rename(appDataDir+"/Settings.set",appDataDir+"/QtBitcoinTrader.cfg");
	}

    if(argc>1)
	{
        if(a.arguments().last().startsWith("/checkupdate"))
		{
			a.setStyleSheet(baseValues.appTheme.styleSheet);

			QSettings settings(appDataDir+"/QtBitcoinTrader.cfg",QSettings::IniFormat);
			QString langFile=settings.value("LanguageFile","").toString();
			if(langFile.isEmpty()||!langFile.isEmpty()&&!QFile::exists(langFile))langFile=baseValues.defaultLangFile;
			julyTranslator.loadFromFile(langFile);

			UpdaterDialog updater(a.arguments().last()!="/checkupdate");
			return a.exec();
		}
	}

#ifdef  Q_OS_WIN
	if(QFile::exists(a.applicationFilePath()+".upd"))QFile::remove(a.applicationFilePath()+".upd");
	if(QFile::exists(a.applicationFilePath()+".bkp"))QFile::remove(a.applicationFilePath()+".bkp");
#endif

#ifdef  Q_OS_MAC
	if(QFile::exists(a.applicationFilePath()+".upd"))QFile::remove(a.applicationFilePath()+".upd");
	if(QFile::exists(a.applicationFilePath()+".bkp"))QFile::remove(a.applicationFilePath()+".bkp");
#endif

	a.setWindowIcon(QIcon(":/Resources/QtBitcoinTrader.png"));
	QFile *lockFile=0;

	{
		QNetworkProxy proxy;
		QSettings settingsMain(appDataDir+"/QtBitcoinTrader.cfg",QSettings::IniFormat);

		bool plastiqueStyle=false;
#ifndef Q_OS_WIN
		plastiqueStyle=true;
#endif
		plastiqueStyle=settingsMain.value("PlastiqueStyle",plastiqueStyle).toBool();
		settingsMain.setValue("PlastiqueStyle",plastiqueStyle);
		if(plastiqueStyle)a.setStyle(new QPlastiqueStyle);
		baseValues.osStyle=a.style()->objectName();

		a.setStyleSheet(baseValues.appTheme.styleSheet);

		settingsMain.beginGroup("Proxy");

		bool proxyEnabled=settingsMain.value("Enabled",true).toBool();
		bool proxyAuto=settingsMain.value("Auto",true).toBool();
		QString proxyHost=settingsMain.value("Host","127.0.0.1").toString();
		quint16 proxyPort=settingsMain.value("Port",1234).toInt();
		QString proxyUser=settingsMain.value("User","username").toString();
		QString proxyPassword=settingsMain.value("Password","password").toString();

		settingsMain.setValue("Enabled",proxyEnabled);
		settingsMain.setValue("Auto",proxyAuto);
		settingsMain.setValue("Host",proxyHost);
		settingsMain.setValue("Port",proxyPort);
		settingsMain.setValue("User",proxyUser);
		settingsMain.setValue("Password",proxyPassword);

		settingsMain.endGroup();
		if(proxyEnabled)
		{
			if(proxyAuto)
			{
				QList<QNetworkProxy> proxyList=QNetworkProxyFactory::systemProxyForQuery(QNetworkProxyQuery(QUrl("https://")));
				if(proxyList.count())proxy=proxyList.first();
			}
			else
			{
				proxy.setHostName(proxyHost);
				proxy.setUser(proxyUser);
				proxy.setPort(proxyPort);
				proxy.setPassword(proxyPassword);
				proxy.setType(QNetworkProxy::HttpProxy);
			}
			QNetworkProxy::setApplicationProxy(proxy);
		}

	baseValues.logFileName=QLatin1String("QtBitcoinTrader.log");
	baseValues.iniFileName=QLatin1String("QtBitcoinTrader.ini");

	{

		QSettings settingsCurrencies("://Resources/Currencies.ini",QSettings::IniFormat); 
		QStringList currenciesList=settingsCurrencies.childGroups();
		for(int n=0;n<currenciesList.count();n++)
		{
			if(currenciesList.at(n).length()!=3)continue;
			CurencyInfo newCurr;
			newCurr.name=settingsCurrencies.value(currenciesList.at(n)+"/Name","").toString().toAscii();
			if(baseValues.supportsUtfUI)newCurr.sign=settingsCurrencies.value(currenciesList.at(n)+"/Sign","").toString().toAscii();
			else newCurr.sign=settingsCurrencies.value(currenciesList.at(n)+"/SignNonUTF8","").toString().toAscii();
			newCurr.valueStep=settingsCurrencies.value(currenciesList.at(n)+"/ValueStep",0.01).toDouble();
			newCurr.valueSmall=settingsCurrencies.value(currenciesList.at(n)+"/ValueSmall",0.1).toDouble();
			if(newCurr.isValid())
			{
				baseValues.currencyMap.insert(currenciesList.at(n).toAscii(),newCurr);
				baseValues.currencyMapSign.insert(newCurr.sign,currenciesList.at(n).toAscii());
			}
		}

		if(!QFile::exists(appDataDir+"Language"))QDir().mkpath(appDataDir+"Language");
		QString langFile=settingsMain.value("LanguageFile","").toString();
		baseValues.appVerLastReal=settingsMain.value("Version",1.0).toDouble();
		settingsMain.setValue("Version",baseValues.appVerReal);
		if(langFile.isEmpty()||!langFile.isEmpty()&&!QFile::exists(langFile))langFile=baseValues.defaultLangFile;
			julyTranslator.loadFromFile(langFile);
	}
	
	bool tryDecrypt=true;
	bool showNewPasswordDialog=false;
	while(tryDecrypt)
	{
		QString tryPassword;
		baseValues.restKey.clear();
		baseValues.restSign.clear();

		if(QDir(appDataDir,"*.ini").entryList().isEmpty()||showNewPasswordDialog)
		{
			NewPasswordDialog newPassword;
			if(newPassword.exec()==QDialog::Accepted)
			{
			tryPassword=newPassword.getPassword();
			newPassword.updateIniFileName();
			baseValues.restKey=newPassword.getRestKey().toAscii();
			QSettings settings(baseValues.iniFileName,QSettings::IniFormat);
			settings.setValue("Profile/ExchangeId",newPassword.getExchangeId());
			QByteArray encryptedData;
			switch(newPassword.getExchangeId())
			{
			case 0:
				{//Mt.Gox
				baseValues.restSign=QByteArray::fromBase64(newPassword.getRestSign().toAscii());
				encryptedData=JulyAES256::encrypt("Qt Bitcoin Trader\r\n"+baseValues.restKey+"\r\n"+baseValues.restSign.toBase64()+"\r\n"+QUuid::createUuid().toByteArray(),tryPassword.toAscii());
				}
				break;
			case 1:
				{//BTC-e
				baseValues.restSign=newPassword.getRestSign().toAscii();
				encryptedData=JulyAES256::encrypt("Qt Bitcoin Trader\r\n"+baseValues.restKey+"\r\n"+baseValues.restSign.toBase64()+"\r\n"+QUuid::createUuid().toByteArray(),tryPassword.toAscii());
				}
				break;
			case 2:
				{//Bitstamp
				baseValues.restSign=newPassword.getRestSign().toAscii();
				encryptedData=JulyAES256::encrypt("Qt Bitcoin Trader\r\n"+baseValues.restKey+"\r\n"+baseValues.restSign.toBase64()+"\r\n"+QUuid::createUuid().toByteArray(),tryPassword.toAscii());
				}
				break;
			case 3:
				{//BTC China
					baseValues.restSign=newPassword.getRestSign().toAscii();
					encryptedData=JulyAES256::encrypt("Qt Bitcoin Trader\r\n"+baseValues.restKey+"\r\n"+baseValues.restSign.toBase64()+"\r\n"+QUuid::createUuid().toByteArray(),tryPassword.toAscii());
				}
				break;
			case 4:
				{//Bitfinex
					baseValues.restSign=newPassword.getRestSign().toAscii();
					encryptedData=JulyAES256::encrypt("Qt Bitcoin Trader\r\n"+baseValues.restKey+"\r\n"+baseValues.restSign.toBase64()+"\r\n"+QUuid::createUuid().toByteArray(),tryPassword.toAscii());
				}
				break;
			default: break;
			}
			settings.setValue("EncryptedData/ApiKeySign",QString(encryptedData.toBase64()));
			settings.setValue("Profile/Name",newPassword.selectedProfileName());
			settings.sync();

			showNewPasswordDialog=false;
			}
		}
			PasswordDialog enterPassword;
			if(enterPassword.exec()==QDialog::Rejected)return 0;
			if(enterPassword.resetData)
			{
				if(QFile::exists(enterPassword.getIniFilePath()))
					QFile::remove(enterPassword.getIniFilePath());
				continue;
			}
			if(enterPassword.newProfile){showNewPasswordDialog=true;continue;}
			tryPassword=enterPassword.getPassword();

		if(!tryPassword.isEmpty())
		{
			baseValues.iniFileName=enterPassword.getIniFilePath();
			baseValues.logFileName=baseValues.iniFileName;baseValues.logFileName.replace(".ini",".log",Qt::CaseInsensitive);
			bool profileLocked=enterPassword.isProfileLocked(baseValues.iniFileName);
			if(profileLocked)
			{
				QMessageBox msgBox(0);
				msgBox.setIcon(QMessageBox::Question);
				msgBox.setWindowTitle("Qt Bitcoin Trader");
				msgBox.setText(julyTr("THIS_PROFILE_ALREADY_USED","This profile is already used by another instance.<br>API does not allow to run two instances with same key sign pair.<br>Please create new profile if you want to use two instances."));
#ifdef Q_OS_WIN
				msgBox.setStandardButtons(QMessageBox::Ok);
				msgBox.setDefaultButton(QMessageBox::Ok);
				msgBox.exec();
#else
				msgBox.setStandardButtons(QMessageBox::Ignore|QMessageBox::Ok);
				msgBox.setDefaultButton(QMessageBox::Ok);
				if(msgBox.exec()==QMessageBox::Ignore)profileLocked=false;
#endif
				if(profileLocked)tryPassword.clear();
			}

			if(!profileLocked)
			{
				QSettings settings(baseValues.iniFileName,QSettings::IniFormat);

				QStringList decryptedList=QString(JulyAES256::decrypt(QByteArray::fromBase64(settings.value("EncryptedData/ApiKeySign","").toString().toAscii()),tryPassword.toAscii())).split("\r\n");

				if(decryptedList.count()>=3&&decryptedList.first()=="Qt Bitcoin Trader")
				{
					baseValues.restKey=decryptedList.at(1).toAscii();
					baseValues.restSign=QByteArray::fromBase64(decryptedList.at(2).toAscii());
					if(decryptedList.count()==3)
					{
						decryptedList<<QUuid::createUuid().toByteArray();
						settings.setValue("EncryptedData/ApiKeySign",QString(JulyAES256::encrypt("Qt Bitcoin Trader\r\n"+decryptedList.at(1).toAscii()+"\r\n"+decryptedList.at(2).toAscii()+"\r\n"+decryptedList.at(3).toAscii(),tryPassword.toAscii()).toBase64()));
						settings.sync();
					}
					baseValues.randomPassword=decryptedList.at(3).toAscii();
                    tryDecrypt=false;
					lockFile=new QFile(enterPassword.lockFilePath(baseValues.iniFileName));
                    lockFile->open(QIODevice::WriteOnly|QIODevice::Truncate);
					lockFile->write("Qt Bitcoin Trader Lock File");
				}
			}
		}
	}

	QSettings iniSettings(baseValues.iniFileName,QSettings::IniFormat);
	if(iniSettings.value("Debug/LogEnabled",false).toBool())debugLevel=1;
	iniSettings.setValue("Debug/LogEnabled",debugLevel>0);
	baseValues.currentPair.currASign=baseValues.currencyMap.value("BTC",CurencyInfo("BTC")).sign;
	baseValues.currentPair.currBStr=iniSettings.value("Profile/Currency","USD").toString().toAscii();
	baseValues.currentPair.currBSign=baseValues.currencyMap.value(baseValues.currentPair.currBStr,CurencyInfo("$")).sign;

	baseValues.logThread_=0;
	if(debugLevel)
	{
		baseValues.logThread_=new LogThread;
		logThread->writeLog("Proxy settings: "+proxy.hostName().toAscii()+":"+QByteArray::number(proxy.port())+" "+proxy.user().toAscii());
	}
	a.setQuitOnLastWindowClosed(false);
	baseValues.mainWindow_=new QtBitcoinTrader;
	QObject::connect(baseValues.mainWindow_,SIGNAL(quit()),&a,SLOT(quit()));
	}
	mainWindow.setupClass();
	a.exec();

	if(lockFile)
	{
		lockFile->close();
		lockFile->remove();
		delete lockFile;
	}
	return 0;
}
