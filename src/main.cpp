// Copyright (C) 2013 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form https://sourceforge.net/projects/bitcointrader/
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

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
	else baseValues.defaultLangFile=":/Resources/Language/English.lng";
}

QColor swapColor(QColor color)
{
	//return color.darker(150);
	//int alpha=255-color.alpha();
	return QColor(255-color.red(),255-color.green(),255-color.blue(),color.alpha()).lighter(200).lighter(200);
}

void BaseValues::Construct()
{
	gzipEnabled=true;
	appVerIsBeta=false;
	appVerStr="1.07961";
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
	dateTimeFormat=QLocale().dateTimeFormat(QLocale::ShortFormat);
	timeFormat=QLocale().timeFormat(QLocale::ShortFormat);
	depthCountLimit=100;
	depthCountLimitStr="100";
	uiUpdateInterval=100;
	depthRefreshBlocked=false;
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
	defaultHeightForRow_=26;


	baseValues.appThemeLight.altRowColor=QColor(240,240,240);
	baseValues.appThemeLight.gray=Qt::gray;
	baseValues.appThemeLight.red=Qt::red;
	baseValues.appThemeLight.green=Qt::green;
	baseValues.appThemeLight.blue=Qt::blue;
	baseValues.appThemeLight.lightRed.setRgb(255,200,200);
	baseValues.appThemeLight.lightGreen.setRgb(200,255,200);
	baseValues.appThemeLight.lightBlue.setRgb(200,200,255);
	baseValues.appThemeLight.lightGreenBlue.setRgb(200,255,255);
	baseValues.appThemeLight.lightRedBlue.setRgb(255,200,255);
	baseValues.appThemeLight.lightRedGreen.setRgb(255,255,200);
	baseValues.appThemeLight.darkRed=Qt::darkRed;
	baseValues.appThemeLight.darkGreen=Qt::darkGreen;
	baseValues.appThemeLight.darkBlue=Qt::darkBlue;
	baseValues.appThemeLight.black=Qt::black;
	baseValues.appThemeLight.white=Qt::white;

	baseValues.appThemeDark.altRowColor=QColor(20,20,20);
	baseValues.appThemeDark.gray=Qt::gray;
	baseValues.appThemeDark.red=Qt::red;
	baseValues.appThemeDark.green=Qt::green;
	baseValues.appThemeDark.blue=QColor(20,20,100);
	baseValues.appThemeDark.lightRed.setRgb(0,55,55);
	baseValues.appThemeDark.lightGreen.setRgb(55,0,55);
	baseValues.appThemeDark.lightBlue.setRgb(55,55,0);
	baseValues.appThemeDark.lightGreenBlue.setRgb(55,0,0);
	baseValues.appThemeDark.lightRedBlue.setRgb(0,55,0);
	baseValues.appThemeDark.lightRedGreen.setRgb(0,0,55);
	baseValues.appThemeDark.darkRed=Qt::darkRed;
	baseValues.appThemeDark.darkGreen=Qt::darkGreen;
	baseValues.appThemeDark.darkBlue=Qt::darkBlue;
	baseValues.appThemeDark.black=Qt::white;
	baseValues.appThemeDark.white=Qt::black;

	selectSystemLanguage();
}

AppTheme::AppTheme()
{
	nightMode=false;
	gray=Qt::gray;
	altRowColor=QColor(240,240,240);
	lightGray=Qt::lightGray;
	red=Qt::red;
	green=Qt::green;
	blue=Qt::blue;
	lightRed.setRgb(255,200,200);
	lightGreen.setRgb(200,255,200);
	lightBlue.setRgb(200,200,255);
	lightGreenBlue.setRgb(200,255,255);
	lightRedBlue.setRgb(255,200,255);
	lightRedGreen.setRgb(255,255,200);
	darkRed=Qt::darkRed;
	darkGreen=Qt::darkGreen;
	darkBlue=Qt::darkBlue;
	black=Qt::black;
	white=Qt::white;
}

int main(int argc, char *argv[])
{
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("utf8"));
	QTextCodec::setCodecForTr(QTextCodec::codecForName("utf8"));
	
	baseValues_=new BaseValues;
	baseValues.Construct();

	QString globalStyleSheet="QGroupBox {background: rgba(255,255,255,190); border: 1px solid gray;border-radius: 3px;margin-top: 7px;} QGroupBox:title {background: qradialgradient(cx: 0.5, cy: 0.5, fx: 0.5, fy: 0.5, radius: 0.7, stop: 0 #fff, stop: 1 transparent); border-radius: 2px; padding: 1 4px; top: -7; left: 7px;} QLabel {color: black;} QDoubleSpinBox {background: white;} QTextEdit {background: white;} QPlainTextEdit {background: white;} QCheckBox {color: black;} QLineEdit {color: black; background: white; border: 1px solid gray;}";

	QApplication a(argc,argv);
	baseValues_->fontMetrics_=new QFontMetrics(a.font());

#ifdef Q_OS_WIN
	{
	QString appLocalStorageDir=a.applicationDirPath()+QLatin1String("/QtBitcoinTrader/");
	appDataDir=QDesktopServices::storageLocation(QDesktopServices::DataLocation).replace("\\","/").toAscii()+"/QtBitcoinTrader/";
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
	appDataDir=QDesktopServices::storageLocation(QDesktopServices::HomeLocation).toAscii()+"/.config/QtBitcoinTrader/";
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
			a.setStyleSheet(globalStyleSheet);

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
		else
		{
			baseValues.appTheme.nightMode=false;
			if(baseValues.appTheme.nightMode)
			{
			a.setStyle(new QPlastiqueStyle);
			baseValues.appTheme=baseValues.appThemeDark;

			QPalette darkPalette=a.palette();
			for(int n=0; n<3; n++)
			{
				if(n==3)continue;
				for(int i=0; i<20; i++)
				{
					darkPalette.setColor(QPalette::ColorGroup(n),QPalette::ColorRole(i),swapColor(darkPalette.color(QPalette::ColorGroup(n),QPalette::ColorRole(i))));
				}
			}


			a.setPalette(darkPalette);
			}
		}
		a.setStyleSheet(globalStyleSheet);

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
		QFile currencyFile("://Resources/Currencies.map");
		currencyFile.open(QIODevice::ReadOnly);
		QStringList currencyList=QString(currencyFile.readAll().replace("\r","")).split("\n");
		currencyFile.close();
		for(int n=0;n<currencyList.count();n++)
		{
			QStringList currencyName=currencyList.at(n).split("=");
			if(currencyName.count()<3)continue;
			baseValues.currencyNamesMap.insert(currencyName.at(0).toAscii(),currencyName.at(1).toAscii());

			if(currencyName.count()==4&&!baseValues.supportsUtfUI)baseValues.currencySignMap.insert(currencyName.at(0).toAscii(),currencyName.at(3).toAscii());
			else baseValues.currencySignMap.insert(currencyName.at(0).toAscii(),currencyName.at(2).toAscii());
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
				encryptedData=JulyAES256::encrypt("Qt Bitcoin Trader\r\n"+baseValues.restKey+"\r\n"+baseValues.restSign.toBase64(),tryPassword.toAscii());
				}
				break;
			case 1:
				{//BTC-e
				baseValues.restSign=newPassword.getRestSign().toAscii();
				encryptedData=JulyAES256::encrypt("Qt Bitcoin Trader\r\n"+baseValues.restKey+"\r\n"+baseValues.restSign.toBase64(),tryPassword.toAscii());
				}
				break;
			case 2:
				{//Bitstamp
				baseValues.restSign=newPassword.getRestSign().toAscii();
				encryptedData=JulyAES256::encrypt("Qt Bitcoin Trader\r\n"+baseValues.restKey+"\r\n"+baseValues.restSign.toBase64(),tryPassword.toAscii());
				}
				break;
			case 3:
				{//BTC China
				baseValues.restSign=newPassword.getRestSign().toAscii();
				encryptedData=JulyAES256::encrypt("Qt Bitcoin Trader\r\n"+baseValues.restKey+"\r\n"+baseValues.restSign.toBase64(),tryPassword.toAscii());
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

				if(decryptedList.count()==3&&decryptedList.first()=="Qt Bitcoin Trader")
				{
					baseValues.restKey=decryptedList.at(1).toAscii();
					baseValues.restSign=QByteArray::fromBase64(decryptedList.last().toAscii());

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
	baseValues.currentPair.currASign=baseValues.currencySignMap.value("BTC","BTC");
	baseValues.currentPair.currBStr=iniSettings.value("Profile/Currency","USD").toString().toAscii();
	baseValues.currentPair.currBSign=baseValues.currencySignMap.value(baseValues.currentPair.currBStr,"$");

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
	mainWindow.loadUiSettings();
	a.exec();

	if(lockFile)
	{
		lockFile->close();
		lockFile->remove();
		delete lockFile;
	}
	return 0;
}
