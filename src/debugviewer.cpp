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

DebugViewer::DebugViewer()
	: QWidget()
{
	ui.setupUi(this);
	ui.checkEnabled->setChecked(true);

	setWindowFlags(Qt::Window);
	setAttribute(Qt::WA_DeleteOnClose,true);

	if(logThread)
	{
		logThread->terminate();
		logThread->deleteLater();
		logThread=0;
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

void DebugViewer::sendLogSlot(QByteArray text)
{
	buffer.append(text);
}

void DebugViewer::secondSlot()
{
	static int counter=0;
	if(++counter>4)counter=0;
	if(counter!=4&&ui.radioDebug->isChecked())return;

	if(buffer.isEmpty())return;
	if(ui.checkEnabled->isChecked())
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
