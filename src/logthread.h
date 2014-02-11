// Copyright (C) 2014 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form http://qtopentrader.com
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#ifndef LOGTHREAD_H
#define LOGTHREAD_H

#include <QThread>

class LogThread : public QThread
{
	Q_OBJECT

public:
	void writeLog(QByteArray,int debugLevel=0);
	void writeLogB(QString mess,int dLevel=0){writeLog(mess.toAscii(),dLevel);}
	LogThread(bool writeFile=true);
	~LogThread();

private:
	bool writeFile;
	void run();
signals:
	void writeLogSignal(QByteArray,int);
	void sendLogSignal(QByteArray);
public slots:
	void writeLogSlot(QByteArray,int);
};

#endif // LOGTHREAD_H
