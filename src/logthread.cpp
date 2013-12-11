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

LogThread::LogThread(bool wrf)
	: QThread()
{
	writeFile=wrf;
	moveToThread(this);
	start();
}

LogThread::~LogThread()
{

}

void LogThread::run()
{
	connect(this,SIGNAL(writeLogSignal(QByteArray,int)),this,SLOT(writeLogSlot(QByteArray,int)));
	exec();
}

void LogThread::writeLog(QByteArray data, int dbLvl)
{
	if(debugLevel==0)return;

	if(debugLevel==2&&dbLvl!=2)return;//0: Disabled; 1: Debug; 2: Log

	emit writeLogSignal(data,dbLvl);
}

void LogThread::writeLogSlot(QByteArray data, int dbLvl)
{
	data="------------------\r\n"+QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss LVL:").toAscii()+QByteArray::number(dbLvl)+"\r\n"+data+"\r\n------------------\r\n\r\n";
	if(writeFile)
	{
		QFile logFile(baseValues.logFileName);
		if(logFile.open(QIODevice::Append))
		{
			logFile.write(data);
			logFile.close();
		}
	}
	else
	emit sendLogSignal(data);
}