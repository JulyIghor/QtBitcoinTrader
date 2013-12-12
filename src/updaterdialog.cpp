// Copyright (C) 2013 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form https://sourceforge.net/projects/bitcointrader/
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#include "updaterdialog.h"
#include <QMessageBox>
#include "main.h"
#include "julyrsa.h"
#include <QCryptographicHash>
#include <QMessageBox>
#include <QClipboard>
#include <QDesktopServices>
#include <QUrl>
#include <QFile>
#include "donatepanel.h"

UpdaterDialog::UpdaterDialog(bool fbMess)
	: QDialog()
{
	feedbackMessage=fbMess;
	stateUpdate=0;
	ui.setupUi(this);
	setWindowFlags(Qt::WindowCloseButtonHint|Qt::WindowStaysOnTopHint);
	httpGet=new JulyHttp("raw.github.com",0,this,true,false);
	timeOutTimer=new QTimer(this);
	connect(timeOutTimer,SIGNAL(timeout()),this,SLOT(exitSlot()));
	connect(httpGet,SIGNAL(dataReceived(QByteArray,int)),this,SLOT(dataReceived(QByteArray,int)));

	ui.donateGroupBox->layout()->addWidget(new DonatePanel(this));

	if(baseValues.appVerIsBeta)httpGet->sendData(320,"GET /JulyIGHOR/QtBitcoinTrader/master/versionsbeta.txt");
			else	httpGet->sendData(320,"GET /JulyIGHOR/QtBitcoinTrader/master/versions.txt");
	timeOutTimer->start(30000);
}

UpdaterDialog::~UpdaterDialog()
{
}

void UpdaterDialog::dataReceived(QByteArray dataReceived,int)
{
	if(httpGet)httpGet->blockSignals(true);
	timeOutTimer->stop();

	if(stateUpdate==0)
	{
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
		if(updateVersion.toDouble()<=baseValues.appVerReal)
		{
			if(feedbackMessage)
				QMessageBox::information(0,"Qt Bitcoin Trader",julyTr("UP_TO_DATE","Your version of Qt Bitcoin Trader is up to date."));
			exitSlot();
			return;
		}
		stateUpdate=1;
		ui.autoUpdateGroupBox->setVisible(canAutoUpdate);
		ui.changeLogText->setHtml(updateChangeLog);
		ui.versionLabel->setText("v"+updateVersion);

		julyTranslator.translateUi(this);
		ui.iconLabel->setPixmap(QPixmap(":/Resources/QtBitcoinTrader.png"));
		QSize minSizeHint=minimumSizeHint();
		if(mainWindow.isValidSize(&minSizeHint))setFixedSize(minimumSizeHint());
		show();
	}
	else
		if(stateUpdate==1)
		{
			QByteArray fileSha1=QCryptographicHash::hash(dataReceived,QCryptographicHash::Sha1);
			QFile readPublicKey(":/Resources/Public.key");
			if(!readPublicKey.open(QIODevice::ReadOnly)){QMessageBox::critical(this,windowTitle(),"Public.key is missing");return;}
			QByteArray publicKey=readPublicKey.readAll();
			QByteArray decrypted=JulyRSA::getSignature(updateSignature,publicKey);
			if(decrypted==fileSha1)
			{
				QString curBin=QApplication::applicationFilePath();
				QString updBin=curBin+".upd";
				QString bkpBin=curBin+".bkp";
				if(QFile::exists(updBin))QFile::remove(updBin);
				if(QFile::exists(bkpBin))QFile::remove(bkpBin);
				if(QFile::exists(updBin)||QFile::exists(bkpBin)){downloadError(1);return;}
				{
					QFile wrFile(updBin);
					if(wrFile.open(QIODevice::WriteOnly|QIODevice::Truncate))
					{
						wrFile.write(dataReceived);
						wrFile.close();
					}else {downloadError(2);return;}
				}
				QByteArray fileData;
				{
					QFile opFile(updBin);
					if(opFile.open(QIODevice::ReadOnly))fileData=opFile.readAll();
					opFile.close();
				}
				if(QCryptographicHash::hash(fileData,QCryptographicHash::Sha1)!=fileSha1){downloadError(3);return;}
				QFile::rename(curBin,bkpBin);
				if(!QFile::exists(bkpBin)){downloadError(4);return;}
				QFile::rename(updBin,curBin);
				if(!QFile::exists(curBin)){QMessageBox::critical(this,windowTitle(),"Critical error. Please reinstall application. Download it from http://sourceforge.net/projects/bitcointrader/<br>File not exists: "+curBin+"<br>"+updBin);downloadError(5);return;}
#ifdef Q_OS_MAC
				QFile(curBin).setPermissions(QFile(bkpBin).permissions());
#endif
				QMessageBox::information(this,windowTitle(),julyTr("UPDATED_SUCCESSFULLY","Application updated successfully. Please restart application to apply changes."));
				exitSlot();
			}
		}
}

void UpdaterDialog::exitSlot()
{
	QCoreApplication::quit();
}

void UpdaterDialog::buttonUpdate()
{
	ui.buttonUpdate->setEnabled(false);
	if(httpGet)delete httpGet;
	QStringList tempList=updateLink.split("//");
	if(tempList.count()!=2){downloadError(6);return;}
	QString protocol=tempList.first();
	tempList=tempList.last().split("/");
	if(tempList.count()==0){downloadError(7);return;}
	QString domain=tempList.first();
	int removeLength=domain.length()+protocol.length()+2;
	if(updateLink.length()<=removeLength){downloadError(8);return;}
	updateLink.remove(0,removeLength);

	httpGet=new JulyHttp(domain,0,this,protocol.startsWith("https"),false);
	connect(httpGet,SIGNAL(apiDown(bool)),this,SLOT(invalidData(bool)));
	connect(httpGet,SIGNAL(dataProgress(int)),this,SLOT(dataProgress(int)));
	connect(httpGet,SIGNAL(dataReceived(QByteArray,int)),this,SLOT(dataReceived(QByteArray,int)));

	httpGet->sendData(320,"GET "+updateLink.toAscii());
}

void UpdaterDialog::invalidData(bool err)
{
	if(err)downloadError(9);
}

void UpdaterDialog::downloadError(int val)
{
	QMessageBox::warning(this,windowTitle(),julyTr("DOWNLOAD_ERROR","Download error. Please try again.")+"<br>"+httpGet->errorString()+"<br>CODE: "+QString::number(val));
	exitSlot();
}

void UpdaterDialog::dataProgress(int precent)
{
	if(httpGet->getCurrentPacketContentLength()>20000000)downloadError(10);
	ui.progressBar->setValue(precent);
}
