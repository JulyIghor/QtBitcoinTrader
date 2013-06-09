// Copyright (C) 2013 July IGHOR.
// I want to create Bitcoin Trader application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form https://sourceforge.net/projects/bitcointrader/
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
	void writeLog(QByteArray);
	LogThread();
	~LogThread();

private:
	void run();
signals:
	void writeLogSignal(QByteArray);
public slots:
	void writeLogSlot(QByteArray);
};

#endif // LOGTHREAD_H
