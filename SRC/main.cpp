//Created by July IGHOR
//http://trader.uax.co
//Bitcoin Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc

#define USING_QTSINGLEAPPLICATION

#include "main.h"
#include <QtGui/QApplication>
#include <QFileInfo>
#include <QSettings>
#include "bitcointraderupdater.h"

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
	appVerStr_=new QByteArray("0.87");
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
	mainWindow_=new BitcoinTrader;

#ifdef USING_QTSINGLEAPPLICATION
	a.setActivationWindow(mainWindow_);
	QObject::connect(mainWindow_,SIGNAL(finished(int)),&a,SLOT(quit()));
#endif
	}
	mainWindow_->show();
	return a.exec();
}
