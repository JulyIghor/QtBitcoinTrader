//Created by July IGHOR
//http://trader.uax.co
//Bitcoin Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc

#include "logthread.h"
//#include <QDebug>
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