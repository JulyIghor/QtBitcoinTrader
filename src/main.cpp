//Created by July IGHOR
//http://trader.uax.co
//Bitcoin Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc

#define USING_QTSINGLEAPPLICATION //QtSingleApplication uses only to prevent starting two programs at time. You can remove this line to not use this class.

#include "main.h"
#include <QtGui/QApplication>
#include <QFileInfo>
#include <QSettings>
#include "bitcointraderupdater.h"
#include "passworddialog.h"
#include "newpassworddialog.h"
#include "julyaes256.h"

#ifdef USING_QTSINGLEAPPLICATION
#include "qtsingleapplication.h"//https://github.com/connectedtable/qtsingleapplication
#endif

QByteArray *restKey_;
QByteArray *restSign_;
LogThread *logThread;
BitcoinTrader *mainWindow_;
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
	appVerStr_=new QByteArray("0.88");
	appVerReal_=new double(appVerStr_->toDouble());
	validKeySign_=new bool(false);
	useSSL_=new bool(true);

#ifdef Q_OS_WIN
	if(argc>1)
	{
#ifdef USING_QTSINGLEAPPLICATION
		QtSingleApplication a("BitcoinTraderUpdater",argc,argv);
		if(a.isRunning())return 0;
#else
		QApplication a(argc,argv);
#endif
		if(a.arguments().last()=="/checkupdate")
		{
			BitcoinTraderUpdater updater;
			return a.exec();
		}
	}
#endif

#ifdef USING_QTSINGLEAPPLICATION
	QtSingleApplication a(argc, argv);
	if(a.isRunning())
	{
		a.sendMessage("ActivateWindow");
		return 0;
	}
#else
	QApplication a(argc,argv);
#endif
	{
	nonce_=new quint64(0);
	logEnabled_=new bool(false);
	QString appFileName=a.applicationDirPath()+"/"+QFileInfo(a.applicationFilePath()).completeBaseName();
	iniFileName_=new QString(appFileName+".ini");
	logFileName_=new QString(appFileName+".log");

	QSettings settings(iniFileName,QSettings::IniFormat);
	isLogEnabled=settings.value("LogEnabled",false).toBool();
	settings.setValue("LogEnabled",isLogEnabled);
	if(isLogEnabled)logThread=new LogThread;
	nonce=QDateTime::currentDateTime().toMSecsSinceEpoch();
	restKey_=new QByteArray;
	restSign_=new QByteArray;

	bool tryDecrypt=true;
	while(tryDecrypt)
	{
		QByteArray cryptedData=QByteArray::fromBase64(settings.value("CryptedData","").toString().toAscii());
		QString tryPassword;
		restKey_->clear();
		restSign_->clear();

		if(cryptedData.isEmpty())
		{
			NewPasswordDialog newPassword;
			if(newPassword.exec()==QDialog::Rejected)return 0;
			tryPassword=newPassword.getPassword();
			restKey=newPassword.getRestKey().toAscii();
			restSign=QByteArray::fromBase64(newPassword.getRestSign().toAscii());
			cryptedData=JulyAES256::encrypt("Qt Bitcoin Trader\r\n"+restKey+"\r\n"+restSign,tryPassword.toAscii());
			settings.setValue("CryptedData",QString(cryptedData.toBase64()));
			settings.remove("RestSign");
			settings.remove("RestKey");
			settings.sync();
		}
		if(tryPassword.isEmpty())
		{
			PasswordDialog enterPassword;
			if(enterPassword.exec()==QDialog::Rejected)return 0;
			if(enterPassword.resetData){settings.remove("CryptedData");settings.sync();continue;}
			tryPassword=enterPassword.getPassword();
		}
		if(!tryPassword.isEmpty())
		{
			QStringList decryptedList=QString(JulyAES256::decrypt(cryptedData,tryPassword.toAscii())).split("\r\n");
			if(decryptedList.count()==3&&decryptedList.first()=="Qt Bitcoin Trader")
			{
				restKey=decryptedList.at(1).toAscii();
				restSign=QByteArray::fromBase64(decryptedList.last().toAscii());
				tryDecrypt=false;
			}
		}
	}
	mainWindow_=new BitcoinTrader;

#ifdef USING_QTSINGLEAPPLICATION
	a.setActivationWindow(mainWindow_);
	QObject::connect(mainWindow_,SIGNAL(finished(int)),&a,SLOT(quit()));
#endif
	}
	mainWindow_->show();
	return a.exec();
}
