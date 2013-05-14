//Created by July IGHOR
//Feel free to contact me: julyighor@gmail.com
//Bitcoin Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc

#ifndef SOCKETTHREAD_H
#define SOCKETTHREAD_H

#include <QThread>
#include <QNetworkRequest>
#include <QUrl>
#include <QSslSocket>
#include <QDateTime>
#include <openssl/hmac.h>
#include <QTimer>

class SocketThread : public QThread
{
	Q_OBJECT

public:
	void reconnectApi();
	void sendToApi(QByteArray, QByteArray);
	void stopLoop(){isLoopRunning=false;}
	SocketThread(int threadId);
	~SocketThread();

private:
	bool waitingNewData;
	void sendToApiNow(QByteArray,QByteArray);
	void checkDataAndSend(QByteArray*);
	void sendPendingData();

	int threadId;
	QTime queryTime;
	QSslSocket *sslSocket;
	QByteArray dataBuffer;
	QTimer *secondTimer;
	QTimer *sendTimer;
	int sendState;
	QList< QPair<QByteArray,QByteArray> > listToSend;
	void getDataFromList(QList< QPair<QByteArray,QByteArray> >*, QByteArray*);
	void getDataFromPair(QByteArray*,QByteArray*,QByteArray*);
	bool isLoopRunning;
	void run();
	QByteArray hmacSha512(QByteArray key, QByteArray baseString);
signals:
	void apiDown();
	void reconnectApiSignal();
	void sendToApiSignal(QByteArray, QByteArray);
	void dataReceived(QByteArray);
public slots:
	void secondSlot();
	void readSocket();
	void reconnectApiSlot();
	void checkSocketConnected();
	void sendToApiSlot(QByteArray, QByteArray);
};

#endif // SOCKETTHREAD_H
