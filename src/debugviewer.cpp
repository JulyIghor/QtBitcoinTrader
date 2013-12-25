// Copyright (C) 2013 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form https://sourceforge.net/projects/bitcointrader/
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#include "debugviewer.h"
#include <QScrollBar>
#include "main.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QSysInfo>

DebugViewer::DebugViewer()
	: QWidget()
{
	savingFile=false;
	ui.setupUi(this);
	ui.checkEnabled->setChecked(true);

	setWindowFlags(Qt::Window);
	setAttribute(Qt::WA_DeleteOnClose,true);

	if(baseValues.logThread_)
	{
		baseValues.logThread_->terminate();
		baseValues.logThread_->deleteLater();
		baseValues.logThread_=0;
	}
	
	logThread=new LogThread(false);
	connect(logThread,SIGNAL(sendLogSignal(QByteArray)),this,SLOT(sendLogSlot(QByteArray)));
	debugLevel=2;
	secondTimer.setParent(this);
	connect(&secondTimer,SIGNAL(timeout()),this,SLOT(secondSlot()));
	secondTimer.start(1000);
	show();
}

DebugViewer::~DebugViewer()
{
	debugLevel=0;
	if(logThread)
	{
		logThread->terminate();
		logThread->deleteLater();
		logThread=0;
	}
}

void DebugViewer::on_buttonSaveAs_clicked()
{
	QDateTime currentdate;
	savingFile=true;
#if QT_VERSION >= 0x040700
	currentdate = QDateTime::currentDateTimeUtc();
#else
	currentdate = QDateTime::currentDateTime().toUTC();
#endif
	QString fileName=QFileDialog::getSaveFileName(this, "Save Debug Information",currentdate.toString("yyyy-MM-dd HH.mm.ss")+".log","Log file (*.log)");
	if(fileName.isEmpty()){savingFile=false;return;}
	
	QFile writeLog(fileName);
	if(writeLog.open(QIODevice::WriteOnly))
	{
		writeLog.write("Qt Bitcoin Trader "+baseValues.appVerStr+"\r\n");

		QByteArray osLine;
#ifdef Q_OS_WIN
		osLine="OS: Windows "+QByteArray::number(QSysInfo::windowsVersion())+"\r\n";
#endif

#ifdef Q_OS_MAC
		osLine="OS: Mac OS "+QByteArray::number(QSysInfo::MacintoshVersion)+"\r\n";
#endif
		if(osLine.isEmpty())
			osLine="OS: Some Linux\r\n";

		writeLog.write(osLine);
		writeLog.write(ui.debugText->toPlainText().toAscii());
		writeLog.close();
	}
	else
		QMessageBox::critical(this,windowTitle(),"Cannot save log file");

	savingFile=false;
}

void DebugViewer::sendLogSlot(QByteArray text)
{
	QStringList filterData(QString(text).split("\r\n"));
	for(int n=0;n<filterData.count();n++)
		if(filterData.at(n).startsWith("Cookie",Qt::CaseInsensitive))
			filterData[n]="Cookie: THERE_WAS_A_COOKIE";

	buffer.append(filterData.join("\r\n"));
}

void DebugViewer::secondSlot()
{
	static int counter=0;
	if(++counter>4)counter=0;
	if(counter!=4&&ui.radioDebug->isChecked())return;

	if(buffer.isEmpty())return;
	if(savingFile==false&&ui.checkEnabled->isChecked())
	{
		ui.debugText->setPlainText(ui.debugText->toPlainText()+"\n"+buffer);

		ui.debugText->verticalScrollBar()->setValue(ui.debugText->verticalScrollBar()->maximum());
	}
	buffer.clear();
}

void DebugViewer::on_radioDebug_toggled(bool debugEnabled)
{
	if(debugEnabled)debugLevel=1;
	else debugLevel=2;
	ui.debugText->setPlainText("");
	buffer.clear();
}
