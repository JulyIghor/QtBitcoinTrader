//Created by July IGHOR
//http://trader.uax.co
//Bitcoin Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc

#include <QDir>
#include <QPlastiqueStyle>
#include "main.h"
#include <QtGui/QApplication>
#include <QFileInfo>
#include <QSettings>
#include "bitcointraderupdater.h"
#include "passworddialog.h"
#include "newpassworddialog.h"
#include "julyaes256.h"
#include <QTextCodec>
#include <QDesktopServices>
#include <QMessageBox>

QByteArray *appDataDir_;
QMap<QByteArray,QByteArray> *currencySignMap;
QMap<QByteArray,QByteArray> *currencyNamesMap;
QByteArray *bitcoinSign_;
QByteArray *currencySign_;
QByteArray *currencyStr_;
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
bool *useSSL_;

int main(int argc, char *argv[])
{
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("utf8"));
	QTextCodec::setCodecForTr(QTextCodec::codecForName("utf8"));
	appDataDir_=new QByteArray();
	appVerStr_=new QByteArray("0.92");
	appVerReal_=new double(appVerStr_->toDouble());
	currencyStr_=new QByteArray();
	currencySign_=new QByteArray();
	validKeySign_=new bool(false);
	bitcoinSign_=new QByteArray("BTC");
	useSSL_=new bool(true);
	currencySignMap=new QMap<QByteArray,QByteArray>;
	currencyNamesMap=new QMap<QByteArray,QByteArray>;

	if(argc>1)
	{
		QApplication a(argc,argv);
		if(a.arguments().last()=="/checkupdate")
		{
			BitcoinTraderUpdater updater;
			return a.exec();
		}
	}

	QApplication a(argc,argv);

#ifndef Q_OS_WIN
	a.setStyle(new QPlastiqueStyle);
#endif
	{
	nonce_=new quint64(0);
	logEnabled_=new bool(false);

#ifdef Q_OS_WIN
	appDataDir=QDesktopServices::storageLocation(QDesktopServices::DataLocation).toAscii()+"/QtBitcoinTrader/";
	if(!QFile::exists(appDataDir))QDir().mkpath(appDataDir);
	QString oldIni=QApplication::applicationDirPath()+"/"+QFileInfo(a.applicationFilePath()).completeBaseName()+".ini";

	if(QFile::exists(oldIni))
	{
		QFile::copy(oldIni,appDataDir+QFileInfo(a.applicationFilePath()).completeBaseName()+".ini");
		QFile::remove(oldIni);
	}
#else
	appDataDir=QDesktopServices::storageLocation(QDesktopServices::HomeLocation).toAscii()+"/.config/QtBitcoinTrader/";
	if(!QFile::exists(appDataDir))QDir().mkpath(appDataDir);
#endif
	a.setStyleSheet("QGroupBox {background: rgba(255,255,255,160); border: 1px solid gray;border-radius: 3px;margin-top: 7px;} QGroupBox:title {background: qradialgradient(cx: 0.5, cy: 0.5, fx: 0.5, fy: 0.5, radius: 0.7, stop: 0 #fff, stop: 1 transparent); border-radius: 2px; padding: 1 4px; top: -7; left: 7px;} QLabel {color: black;} QDoubleSpinBox {background: white;} QTextEdit {background: white;} QCheckBox {color: black;} QLineEdit {color: black; background: white; border: 1px solid gray;}");

	logFileName_=new QString("QtBitcoinTrader.log");
	iniFileName_=new QString("QtBitcoinTrader.ini");

	nonce=QDateTime::currentDateTime().toMSecsSinceEpoch();
	restKey_=new QByteArray;
	restSign_=new QByteArray;
	{
		QFile currencyFile("://Resources/Currencies.map");
		currencyFile.open(QIODevice::ReadOnly);
		QStringList currencyList=QString(currencyFile.readAll()).split("\r\n");
		currencyFile.close();
		for(int n=0;n<currencyList.count();n++)
		{
			QStringList currencyName=currencyList.at(n).split("=");
			if(currencyName.count()!=3)continue;
			currencyNamesMap->insert(currencyName.at(0).toAscii(),currencyName.at(1).toAscii());
			currencySignMap->insert(currencyName.at(0).toAscii(),currencyName.at(2).toAscii());
		}
	}

	bool tryDecrypt=true;
	bool showNewPasswordDialog=false;
	while(tryDecrypt)
	{
		QString tryPassword;
		restKey_->clear();
		restSign_->clear();

		if(QDir(appDataDir,"*.ini").entryList().isEmpty()||showNewPasswordDialog)
		{
			NewPasswordDialog newPassword;
			if(newPassword.exec()==QDialog::Rejected)return 0;
			tryPassword=newPassword.getPassword();
			newPassword.updateIniFileName();
			restKey=newPassword.getRestKey().toAscii();
			restSign=QByteArray::fromBase64(newPassword.getRestSign().toAscii());
			QByteArray cryptedData=JulyAES256::encrypt("Qt Bitcoin Trader\r\n"+restKey+"\r\n"+restSign_->toBase64(),tryPassword.toAscii());
			QSettings settings(iniFileName,QSettings::IniFormat);
			settings.setValue("CryptedData",QString(cryptedData.toBase64()));
			settings.setValue("ProfileName",newPassword.selectedProfileName());
			settings.remove("RestSign");
			settings.remove("RestKey");
			settings.sync();

			showNewPasswordDialog=false;
		}
			PasswordDialog enterPassword;
			if(enterPassword.exec()==QDialog::Rejected)return 0;
			if(enterPassword.resetData){QFile::remove(enterPassword.getIniFilePath());continue;}
			if(enterPassword.newProfile){showNewPasswordDialog=true;continue;}
			tryPassword=enterPassword.getPassword();

		if(!tryPassword.isEmpty())
		{
			iniFileName=enterPassword.getIniFilePath();

			QString lockFilePath(QDesktopServices::storageLocation(QDesktopServices::TempLocation)+"/QtBitcoinTrader_lock_"+QString(QCryptographicHash::hash(iniFileName_->toAscii(),QCryptographicHash::Sha1).toHex()));
		
			QFile::remove(lockFilePath);
			if(QFile::exists(lockFilePath))
			{
				QMessageBox::warning(0,"Qt Bitcoin Trader","This profile is already used by another instance.\nAPI does not allow to run two instances with same key sign pair.\nPlease create new profile if you want to use two instances.");
				tryPassword.clear();
			}
			else
			{
				QFile *lockFile=new QFile(lockFilePath);
				lockFile->open(QIODevice::WriteOnly);
				lockFile->write("Qt Bitcoin Trader Lock File");

				QSettings settings(iniFileName,QSettings::IniFormat);
				QStringList decryptedList=QString(JulyAES256::decrypt(QByteArray::fromBase64(settings.value("CryptedData","").toString().toAscii()),tryPassword.toAscii())).split("\r\n");

				if(decryptedList.count()==3&&decryptedList.first()=="Qt Bitcoin Trader")
				{
					restKey=decryptedList.at(1).toAscii();
					restSign=QByteArray::fromBase64(decryptedList.last().toAscii());
					tryDecrypt=false;
				}
			}
		}
	}

	QSettings settings(iniFileName,QSettings::IniFormat);
	isLogEnabled=settings.value("LogEnabled",false).toBool();
	settings.setValue("LogEnabled",isLogEnabled);
	bitcoinSign=currencySignMap->value("BTC","BTC");
	currencyStr=settings.value("Currency","USD").toString().toAscii();
	currencySign=currencySignMap->value(currencyStr,"$");

	if(isLogEnabled)logThread=new LogThread;

	mainWindow_=new QtBitcoinTrader;
	QObject::connect(mainWindow_,SIGNAL(quit()),&a,SLOT(quit()));
	}
	mainWindow_->show();
	return a.exec();
}
