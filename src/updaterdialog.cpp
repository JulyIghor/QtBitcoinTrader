// Copyright (C) 2013 July IGHOR.
// I want to create Bitcoin Trader application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form https://sourceforge.net/projects/bitcointrader/
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#include "updaterdialog.h"
#include <QSslSocket>
#include <QMessageBox>
#include "main.h"
#include "julyrsa.h"
#ifdef Q_OS_WIN
#include "qtwin.h"
#endif
#include <QCryptographicHash>
#include <QMessageBox>

UpdaterDialog::UpdaterDialog(QWidget *parent)
	: QDialog(parent)
{
	stateUpdate=0;
	ui.setupUi(this);
	setWindowFlags(Qt::WindowCloseButtonHint|Qt::WindowStaysOnTopHint);
	httpGet=new QHttp("raw.github.com", QHttp::ConnectionModeHttps,443,this);
	timeOutTimer=new QTimer(this);
	connect(timeOutTimer,SIGNAL(timeout()),this,SLOT(exitSlot()));
	connect(httpGet,SIGNAL(done(bool)),this,SLOT(httpDone(bool)));

	httpGet->get("/JulyIGHOR/QtBitcoinTrader/master/versions.txt");
	timeOutTimer->start(30000);
}

UpdaterDialog::~UpdaterDialog()
{
}

void UpdaterDialog::exitSlot()
{
	QCoreApplication::quit();
}

void UpdaterDialog::httpDone(bool error)
{
	timeOutTimer->stop();
	if(error)
	{
		if(isVisible())
		{
			if(httpGet->errorString()=="Request aborted")return;
			downloadError();
			return;
		}
		exitSlot();
		return;
	}

	if(stateUpdate==0)
	{
		QByteArray dataReceived(httpGet->readAll().replace("\r",""));
		if(dataReceived.size()>10245)exitSlot();
		QMap<QString,QString>versionsMap;
		QStringList dataList=QString(dataReceived).split("\n");
		for(int n=0;n<dataList.count();n++)
		{
			QString varData=dataList.at(n);
			int splitPos=varData.indexOf('=');
			if(splitPos>-1)
			{
				QString varName=varData.left(splitPos);
				varData.remove(0,splitPos+1);
				versionsMap[varName]=varData;
			}
		}

QString os="Src";
bool canAutoUpdate=false;
#ifdef Q_OS_MAC
		os="Mac";
		canAutoUpdate=true;
#endif
#ifdef Q_OS_WIN
		os="Win32";
		canAutoUpdate=true;
#endif
		updateVersion=versionsMap.value(os+"Ver");
		updateSignature=versionsMap.value(os+"Signature").toAscii();
		if(!updateSignature.isEmpty())updateSignature=QByteArray::fromBase64(updateSignature);
		updateChangeLog=versionsMap.value(os+"ChangeLog");
		updateLink=versionsMap.value(os+"Bin");
		if(updateVersion.toDouble()<=appVerReal){exitSlot();return;}
		stateUpdate=1;
		ui.autoUpdateGroupBox->setVisible(canAutoUpdate);
		ui.changeLogText->setHtml(updateChangeLog);
		ui.versionLabel->setText("v"+updateVersion);

#ifdef Q_OS_WIN
		if(QtWin::isCompositionEnabled())
			QtWin::extendFrameIntoClientArea(this);
#endif

#ifdef GENERATE_LANGUAGE_FILE
		julyTranslator->loadMapFromUi(this);
		julyTranslator->saveToFile("LanguageDefault.lng");
#endif
		julyTranslator->translateUi(this);
		ui.iconLabel->setPixmap(QPixmap(":/Resources/QtBitcoinTrader.png"));
		QSize minSizeHint=minimumSizeHint();
		if(mainWindow.isValidSize(&minSizeHint))setFixedSize(minimumSizeHint());
		show();
	}
	else
	if(stateUpdate==1)
	{
		QByteArray binData=httpGet->readAll();
		QByteArray fileSha1=QCryptographicHash::hash(binData,QCryptographicHash::Sha1);
		QFile readPublicKey(":/Resources/Public.key");
		if(!readPublicKey.open(QIODevice::ReadOnly)){QMessageBox::critical(this,windowTitle(),"Public.key is missing");return;}
		QByteArray publicKey=readPublicKey.readAll();
		QByteArray decrypted=JulyRSA::getSignature(updateSignature,publicKey);
		if(decrypted==fileSha1)
		{
			QString curBin=QApplication::applicationFilePath();
			QString updBin=curBin+".upd";
			QString bkpBin=curBin+".bkp";
			QFile::remove(updBin);
			QFile::remove(bkpBin);
			if(QFile::exists(updBin)||QFile::exists(bkpBin)){downloadError();return;}
			{
				QFile wrFile(updBin);
				if(wrFile.open(QIODevice::WriteOnly|QIODevice::Truncate))
				{
					wrFile.write(binData);
					wrFile.close();
				}else {downloadError();return;}
			}
			QByteArray fileData;
			{
			QFile opFile(updBin);
			if(opFile.open(QIODevice::ReadOnly))fileData=opFile.readAll();
			opFile.close();
			}
			if(QCryptographicHash::hash(fileData,QCryptographicHash::Sha1)!=fileSha1){downloadError();return;}
			QFile::rename(curBin,bkpBin);
			if(!QFile::exists(bkpBin)){downloadError();return;}
			QFile::rename(updBin,curBin);
			if(!QFile::exists(curBin)){QMessageBox::critical(this,windowTitle(),"Critical error. Please reinstall application. Download it from http://sourceforge.net/projects/bitcointrader/<br>File not exists: "+curBin+"\n"+updBin);downloadError();return;}
#ifdef Q_OS_MAC
            QFile(curBin).setPermissions(QFile(bkpBin).permissions());
#endif
            QMessageBox::information(this,windowTitle(),julyTr("UPDATED_SUCCESSFULLY","Application updated successfully. Please restart application to apply changes."));
			exitSlot();
		}
	}
}

void UpdaterDialog::buttonUpdate()
{
	if(httpGet)delete httpGet;
	httpGet=new QHttp;
	connect(httpGet,SIGNAL(done(bool)),this,SLOT(httpDone(bool)));
	connect(httpGet,SIGNAL(dataReadProgress(int,int)),this,SLOT(dataReadProgress(int,int)));
	QStringList tempList=updateLink.split("//");
	if(tempList.count()!=2){downloadError();return;}
	QString protocol=tempList.first();
	tempList=tempList.last().split("/");
	if(tempList.count()==0){downloadError();return;}
	QString domain=tempList.first();
	int removeLength=domain.length()+protocol.length()+2;
	if(updateLink.length()<=removeLength){downloadError();return;}
	updateLink.remove(0,removeLength);
	if(protocol.startsWith("https"))
	{
		httpGet->setSocket(new QSslSocket(httpGet));
		httpGet->setHost(domain, QHttp::ConnectionModeHttps,443);
	}
	else httpGet->setHost(domain,80);
	httpGet->get(updateLink);
}

void UpdaterDialog::downloadError()
{
	QMessageBox::warning(this,windowTitle(),julyTr("DOWNLOAD_ERROR","Download error. Please try again.")+"<br>"+httpGet->errorString());
	exitSlot();
}

void UpdaterDialog::dataReadProgress(int done,int total)
{
	ui.buttonUpdate->setEnabled(false);
	if(total>15000000)downloadError();
	ui.progressBar->setValue(done*100/total);
}
