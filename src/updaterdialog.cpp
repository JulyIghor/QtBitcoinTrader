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
	downloaded100=false;
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
			{
				QMessageBox msgb;
				msgb.setWindowFlags(Qt::WindowCloseButtonHint|Qt::WindowStaysOnTopHint);
				msgb.setWindowTitle("Qt Bitcoin Trader");
				msgb.setIcon(QMessageBox::Information);
				msgb.setText(julyTr("UP_TO_DATE","Your version of Qt Bitcoin Trader is up to date."));
				msgb.exec();
			}
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
			downloaded100=true;
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
	if(downloaded100)return;
	QMessageBox::warning(this,windowTitle(),julyTr("DOWNLOAD_ERROR","Download error. Please try again.")+"<br>"+httpGet->errorString()+"<br>CODE: "+QString::number(val));
	exitSlot();
}

void UpdaterDialog::dataProgress(int precent)
{
	if(httpGet->getCurrentPacketContentLength()>20000000)downloadError(10);
	ui.progressBar->setValue(precent);
}
