// Copyright (C) 2013 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form https://sourceforge.net/projects/bitcointrader/
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#include "logthread.h"
#include <QDateTime>
#include <QFile>
#include <QApplication>
#include "main.h"

LogThread::LogThread()
	: QThread()
{
	moveToThread(this);
	start();
}

LogThread::~LogThread()
{

}

void LogThread::run()
{
	connect(this,SIGNAL(writeLogSignal(QByteArray)),this,SLOT(writeLogSlot(QByteArray)));
	exec();
}

void LogThread::writeLog(QByteArray data)
{
	emit writeLogSignal(data);
}

void LogThread::writeLogSlot(QByteArray data)
{
	QFile logFile(logFileName);
	if(logFile.open(QIODevice::Append))
	{
		logFile.write("------------------\r\n"+QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss\n").toAscii()+data+"\r\n------------------\r\n\r\n");
		logFile.close();
	}
}