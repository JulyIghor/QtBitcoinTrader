//Created by July IGHOR
//http://trader.uax.co
//Bitcoin Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc

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
