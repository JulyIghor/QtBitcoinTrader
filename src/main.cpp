// Copyright (C) 2013 July IGHOR.
// I want to create Bitcoin Trader application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form https://sourceforge.net/projects/bitcointrader/
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

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

QByteArray *appDataDir_;
QMap<QByteArray,QByteArray> *currencySignMap;
QMap<QByteArray,QByteArray> *currencyNamesMap;
QByteArray *currencyASign_;
QByteArray *currencyBSign_;
QByteArray *currencyAStr_;
QByteArray *currencyBStr_;
QByteArray *currencyAStrLow_;
QByteArray *currencyBStrLow_;
QByteArray *restKey_;
QByteArray *restSign_;
LogThread *logThread;
QtBitcoinTrader *mainWindow_;
quint64 *nonce_;
bool *logEnabled_;
QString *iniFileName_;
QString *logFileName_;
double *appVerReal_;
QByteArray *appVerStr_;
bool *validKeySign_;
JulyTranslator *julyTranslator;
QString *defaultLangFile_;
QString *dateTimeFormat_;
QString *timeFormat_;
QString *exchangeName_;
QByteArray *currencyRequest_;

void pickDefaultLangFile()
{
	QString sysLocale=QLocale().name();
	if(sysLocale.startsWith("de"))defaultLangFile=":/Resources/Language/German.lng";
	else 
	if(sysLocale.startsWith("ru"))defaultLangFile=":/Resources/Language/Russian.lng";
	else 
	if(sysLocale.startsWith("uk"))defaultLangFile=":/Resources/Language/Ukrainian.lng";
	else 
	if(sysLocale.startsWith("nl"))defaultLangFile=":/Resources/Language/Dutch.lng";
	else 
	if(sysLocale.startsWith("es"))defaultLangFile=":/Resources/Language/Spanish.lng";
	else 
	if(sysLocale.startsWith("nb"))defaultLangFile=":/Resources/Language/Norwegian.lng";
	else defaultLangFile=":/Resources/Language/English.lng";
}

int main(int argc, char *argv[])
{
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("utf8"));
	QTextCodec::setCodecForTr(QTextCodec::codecForName("utf8"));

	julyTranslator=new JulyTranslator;
	appDataDir_=new QByteArray();
	appVerStr_=new QByteArray("1.041");
	appVerReal_=new double(appVerStr.toDouble());
	currencyBStr_=new QByteArray("USD");
	currencyBStrLow_=new QByteArray("usd");
	currencyBSign_=new QByteArray("USD");
	validKeySign_=new bool(false);
	currencyASign_=new QByteArray("BTC");
	currencyAStr_=new QByteArray("BTC");
	currencyAStrLow_=new QByteArray("btc");
	currencyRequest_=new QByteArray("BTCUSD");
	defaultLangFile_=new QString();pickDefaultLangFile();
	currencySignMap=new QMap<QByteArray,QByteArray>;
	currencyNamesMap=new QMap<QByteArray,QByteArray>;
	dateTimeFormat_=new QString(QLocale().dateTimeFormat(QLocale::ShortFormat));
	timeFormat_=new QString(QLocale().timeFormat(QLocale::ShortFormat));// "HH:mm:ss");
	exchangeName_=new QString("Mt.Gox");
	QString globalStyleSheet="QGroupBox {background: rgba(255,255,255,160); border: 1px solid gray;border-radius: 3px;margin-top: 7px;} QGroupBox:title {background: qradialgradient(cx: 0.5, cy: 0.5, fx: 0.5, fy: 0.5, radius: 0.7, stop: 0 #fff, stop: 1 transparent); border-radius: 2px; padding: 1 4px; top: -7; left: 7px;} QLabel {color: black;} QDoubleSpinBox {background: white;} QTextEdit {background: white;} QPlainTextEdit {background: white;} QCheckBox {color: black;} QLineEdit {color: black; background: white; border: 1px solid gray;}";

#ifdef Q_OS_WIN
	if(QFile::exists("./QtBitcoinTrader"))
	{
		appDataDir="./QtBitcoinTrader/";
		QDir().mkpath(appDataDir+"Language");
		if(!QFile::exists(appDataDir+"Language"))appDataDir.clear();
	}
	if(appDataDir.isEmpty())
	{
	appDataDir=QDesktopServices::storageLocation(QDesktopServices::DataLocation).replace("\\","/").toAscii()+"/QtBitcoinTrader/";
	if(!QFile::exists(appDataDir))QDir().mkpath(appDataDir);
	}
#else
	appDataDir=QDesktopServices::storageLocation(QDesktopServices::HomeLocation).toAscii()+"/.config/QtBitcoinTrader/";
	if(!QFile::exists(appDataDir))QDir().mkpath(appDataDir);
#endif
	
    if(argc>1)
	{
		QApplication a(argc,argv);
        if(a.arguments().last().startsWith("/checkupdate"))
		{
#ifndef Q_OS_WIN
			a.setStyle(new QPlastiqueStyle);
#endif
			a.setStyleSheet(globalStyleSheet);

			QSettings settings(appDataDir+"/Settings.set",QSettings::IniFormat);
			QString langFile=settings.value("LanguageFile","").toString();
			if(langFile.isEmpty()||!langFile.isEmpty()&&!QFile::exists(langFile))langFile=defaultLangFile;
			julyTranslator->loadFromFile(langFile);

			UpdaterDialog updater(a.arguments().last()!="/checkupdate");
			return a.exec();
		}
	}

	QApplication a(argc,argv);
#ifndef Q_OS_WIN
	a.setStyle(new QPlastiqueStyle);
#endif

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
		//QSettings settingsProxy(a.applicationDirPath()+"/Proxy.ini",QSettings::IniFormat);
		//QString proxyHost=settingsProxy.value("Host","").toString();settingsProxy.setValue("Host",proxyHost);
		//bool proxyEnabled=settingsProxy.value("Enabled",false).toBool();settingsProxy.setValue("Enabled",proxyEnabled);
		//qint16 proxyPort=settingsProxy.value("Port",8990).toUInt();settingsProxy.setValue("Port",proxyPort);
		//QString proxyLogin=settingsProxy.value("Login","").toString();settingsProxy.setValue("Login",proxyLogin);
		//QString proxyPassword=settingsProxy.value("Password","").toString();settingsProxy.setValue("Password",proxyPassword);
		//int proxyType=settingsProxy.value("Type","").toInt();settingsProxy.setValue("Type",proxyType);
		//if(proxyEnabled)
		//{
		//QNetworkProxy proxy;
		//switch(proxyType)
		//{
		//	case 1:	proxy.setType(QNetworkProxy::Socks5Proxy); break;
		//	case 3:	proxy.setType(QNetworkProxy::HttpProxy); break;
		//	case 4:	proxy.setType(QNetworkProxy::HttpCachingProxy); break;
		//	case 5:	proxy.setType(QNetworkProxy::FtpCachingProxy); break;
		//	default:	proxy.setType(QNetworkProxy::DefaultProxy); break;
		//}
		//proxy.setHostName(proxyHost);
		//proxy.setPort(proxyPort);
		//proxy.setUser(proxyLogin);
		//proxy.setPassword(proxyPassword);
		//QNetworkProxy::setApplicationProxy(proxy);
		//}
	logEnabled_=new bool(false);

	a.setStyleSheet(globalStyleSheet);

	logFileName_=new QString("QtBitcoinTrader.log");
	iniFileName_=new QString("QtBitcoinTrader.ini");

	restKey_=new QByteArray;
	restSign_=new QByteArray;
	{
		QFile currencyFile("://Resources/Currencies.map");
		currencyFile.open(QIODevice::ReadOnly);
		QStringList currencyList=QString(currencyFile.readAll().replace("\r","")).split("\n");
		currencyFile.close();
		for(int n=0;n<currencyList.count();n++)
		{
			QStringList currencyName=currencyList.at(n).split("=");
			if(currencyName.count()!=3)continue;
			currencyNamesMap->insert(currencyName.at(0).toAscii(),currencyName.at(1).toAscii());
			currencySignMap->insert(currencyName.at(0).toAscii(),currencyName.at(2).toAscii());
		}
		if(!QFile::exists(appDataDir+"Language"))QDir().mkpath(appDataDir+"Language");
		QSettings settings(appDataDir+"/Settings.set",QSettings::IniFormat);
		QString langFile=settings.value("LanguageFile","").toString();
		if(langFile.isEmpty()||!langFile.isEmpty()&&!QFile::exists(langFile))langFile=defaultLangFile;
			julyTranslator->loadFromFile(langFile);
	}

	bool tryDecrypt=true;
	bool showNewPasswordDialog=false;
	while(tryDecrypt)
	{
		QString tryPassword;
		restKey.clear();
		restSign.clear();

		if(QDir(appDataDir,"*.ini").entryList().isEmpty()||showNewPasswordDialog)
		{
			NewPasswordDialog newPassword;
			if(newPassword.exec()==QDialog::Accepted)
			{
			tryPassword=newPassword.getPassword();
			newPassword.updateIniFileName();
			restKey=newPassword.getRestKey().toAscii();
			QSettings settings(iniFileName,QSettings::IniFormat);
			settings.setValue("ExchangeId",newPassword.getExchangeId());
			QByteArray cryptedData;
			if(newPassword.getExchangeId()==0)
			{
			restSign=QByteArray::fromBase64(newPassword.getRestSign().toAscii());
			cryptedData=JulyAES256::encrypt("Qt Bitcoin Trader\r\n"+restKey+"\r\n"+restSign.toBase64(),tryPassword.toAscii());
			}
			else
			if(newPassword.getExchangeId()==1)
			{
				restSign=newPassword.getRestSign().toAscii();
				cryptedData=JulyAES256::encrypt("Qt Bitcoin Trader\r\n"+restKey+"\r\n"+restSign.toBase64(),tryPassword.toAscii());
			}
			settings.setValue("CryptedData",QString(cryptedData.toBase64()));
			settings.setValue("ProfileName",newPassword.selectedProfileName());
			settings.remove("RestSign");
			settings.remove("RestKey");
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
			iniFileName=enterPassword.getIniFilePath();
			bool profileLocked=enterPassword.isProfileLocked(iniFileName);
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
				QSettings settings(iniFileName,QSettings::IniFormat);
				QStringList decryptedList=QString(JulyAES256::decrypt(QByteArray::fromBase64(settings.value("CryptedData","").toString().toAscii()),tryPassword.toAscii())).split("\r\n");

				if(decryptedList.count()==3&&decryptedList.first()=="Qt Bitcoin Trader")
				{
					restKey=decryptedList.at(1).toAscii();
					restSign=QByteArray::fromBase64(decryptedList.last().toAscii());
                    tryDecrypt=false;
					lockFile=new QFile(enterPassword.lockFilePath(iniFileName));
                    lockFile->open(QIODevice::WriteOnly|QIODevice::Truncate);
					lockFile->write("Qt Bitcoin Trader Lock File");
				}
			}
		}
	}

	QSettings settings(iniFileName,QSettings::IniFormat);
	isLogEnabled=settings.value("LogEnabled",false).toBool();
	settings.setValue("LogEnabled",isLogEnabled);
	currencyASign=currencySignMap->value("BTC","BTC");
	currencyBStr=settings.value("Currency","USD").toString().toAscii();
	currencyBSign=currencySignMap->value(currencyBStr,"$");

	if(isLogEnabled)logThread=new LogThread;

	mainWindow_=new QtBitcoinTrader;
	QObject::connect(mainWindow_,SIGNAL(quit()),&a,SLOT(quit()));
	}
	mainWindow.show();
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
