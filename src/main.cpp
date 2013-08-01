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
#include "datafolderchusedialog.h"

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
double *appVerLastReal_;
bool *appVerIsBeta_;
QByteArray *appVerStr_;
bool *validKeySign_;
JulyTranslator *julyTranslator;
QString *defaultLangFile_;
QString *dateTimeFormat_;
QString *timeFormat_;
QString *exchangeName_;
QByteArray *currencyRequest_;
int *btcDecimals_;
int *usdDecimals_;
int *priceDecimals_;
double *minTradePrice_;
double *minTradeVolume_;
int *httpRequestInterval_;
int *httpRequestTimeout_;
bool *httpSplitPackets_;
int *depthCountLimit_;
int *uiUpdateInterval_;
int *apiDownCount_;
QFontMetrics *fontMetrics_;

void pickDefaultLangFile()
{
	QString sysLocale=QLocale().name();
	if(sysLocale.startsWith("de"))defaultLangFile=":/Resources/Language/German.lng";
	else 
	if(sysLocale.startsWith("ru"))defaultLangFile=":/Resources/Language/Russian.lng";
	else 
	if(sysLocale.startsWith("uk"))defaultLangFile=":/Resources/Language/Ukrainian.lng";
	else 
	if(sysLocale.startsWith("pl"))defaultLangFile=":/Resources/Language/Polish.lng";
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
	appVerIsBeta_=new bool(false);
	appVerStr_=new QByteArray("1.0757");
	appVerReal_=new double(appVerStr.toDouble());
	if(appVerStr.size()>4)
	{ 
		appVerStr.insert(4,".");
		if(appVerStr.at(appVerStr.size()-1)!='0')appVerIsBeta=true;
	}
	appVerLastReal_=new double(appVerReal);
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
	timeFormat_=new QString(QLocale().timeFormat(QLocale::ShortFormat));
	exchangeName_=new QString("Mt.Gox");
	btcDecimals_=new int(8);
	usdDecimals_=new int(5);
	priceDecimals_=new int(5);
	depthCountLimit_=new int(100);
	uiUpdateInterval_=new int(100);

	minTradePrice_=new double(0.01);
	minTradeVolume_=new double(0.01);
	httpRequestInterval_=new int(400);
	httpRequestTimeout_=new int(5000);
	httpSplitPackets_=new bool(true);
	logEnabled_=new bool(false);
	apiDownCount_=new int(0);

	const QString globalStyleSheet="QGroupBox {background: rgba(255,255,255,190); border: 1px solid gray;border-radius: 3px;margin-top: 7px;} QGroupBox:title {background: qradialgradient(cx: 0.5, cy: 0.5, fx: 0.5, fy: 0.5, radius: 0.7, stop: 0 #fff, stop: 1 transparent); border-radius: 2px; padding: 1 4px; top: -7; left: 7px;} QLabel {color: black;} QDoubleSpinBox {background: white;} QTextEdit {background: white;} QPlainTextEdit {background: white;} QCheckBox {color: black;} QLineEdit {color: black; background: white; border: 1px solid gray;}";

	QApplication a(argc,argv);
	fontMetrics_=new QFontMetrics(a.font());

#ifdef Q_OS_WIN
	{
	QString appLocalStorageDir=a.applicationDirPath()+QLatin1String("/QtBitcoinTrader/");
	appDataDir=QDesktopServices::storageLocation(QDesktopServices::DataLocation).replace("\\","/").toAscii()+"/QtBitcoinTrader/";
	if(!QFile::exists(appLocalStorageDir)&&!QFile::exists(appDataDir))
	{
		julyTranslator->loadFromFile(defaultLangFile);
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
	
    if(argc>1)
	{
        if(a.arguments().last().startsWith("/checkupdate"))
		{
			a.setStyleSheet(globalStyleSheet);

			QSettings settings(appDataDir+"/Settings.set",QSettings::IniFormat);
			QString langFile=settings.value("LanguageFile","").toString();
			if(langFile.isEmpty()||!langFile.isEmpty()&&!QFile::exists(langFile))langFile=defaultLangFile;
			julyTranslator->loadFromFile(langFile);

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
		QSettings settings(appDataDir+"/Settings.set",QSettings::IniFormat);

		bool plastiqueStyle=false;
#ifndef Q_OS_WIN
		plastiqueStyle=true;
#endif
		plastiqueStyle=settings.value("PlastiqueStyle",plastiqueStyle).toBool();
		settings.setValue("PlastiqueStyle",plastiqueStyle);
		if(plastiqueStyle)a.setStyle(new QPlastiqueStyle);

		settings.beginGroup("Proxy");

		bool proxyEnabled=settings.value("Enabled",true).toBool();
		bool proxyAuto=settings.value("Auto",true).toBool();
		QString proxyHost=settings.value("Host","127.0.0.1").toString();
		quint16 proxyPort=settings.value("Port",1234).toInt();
		QString proxyUser=settings.value("User","username").toString();
		QString proxyPassword=settings.value("Password","password").toString();


		settings.setValue("Enabled",proxyEnabled);
		settings.setValue("Auto",proxyAuto);
		settings.setValue("Host",proxyHost);
		settings.setValue("Port",proxyPort);
		settings.setValue("User",proxyUser);
		settings.setValue("Password",proxyPassword);

		settings.endGroup();
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
		appVerLastReal=settings.value("Version",1.0).toDouble();
		settings.setValue("Version",appVerReal);
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
			logFileName=iniFileName;logFileName.replace(".ini",".log",Qt::CaseInsensitive);
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

	isLogEnabled=settings.value("LogEnabled",false).toBool();
	settings.setValue("LogEnabled",isLogEnabled);
	currencyASign=currencySignMap->value("BTC","BTC");
	currencyBStr=settings.value("Currency","USD").toString().toAscii();
	currencyBSign=currencySignMap->value(currencyBStr,"$");

	if(isLogEnabled)
	{
		logThread=new LogThread;
		logThread->writeLog("Proxy settings: "+proxy.hostName().toAscii()+":"+QByteArray::number(proxy.port())+" "+proxy.user().toAscii());
	}
	a.setQuitOnLastWindowClosed(false);
	mainWindow_=new QtBitcoinTrader;
	QObject::connect(mainWindow_,SIGNAL(quit()),&a,SLOT(quit()));
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
