// Copyright (C) 2013 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form https://sourceforge.net/projects/bitcointrader/
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#ifndef JULYHTTP_H
#define JULYHTTP_H

#include <QObject>
#include <QSslSocket>
#include <QTime>

class JulyHttp : public QObject
{
	Q_OBJECT

public:
	void clearPendingData();
	void reConnect(bool mastAbort=true);
	bool isReqTypePending(int);
	void sendData(int reqType, bool highPriority, const QByteArray &method, int removeLowerReqTypes=-1, QByteArray postData=0, const QByteArray &restSignLine=0);

	void prepareData(int reqType, const QByteArray &method, QByteArray postData=0, const QByteArray &restSignLine=0);
	void prepareDataSend();
	void prepareDataClear();

	JulyHttp(const QString &hostName, const QByteArray &restKeyLine, QObject *parent);
	~JulyHttp();

private:
	QByteArray cookie;
	int incomingPacketsCount;
	void swapSockets();
	void pickNextConnectedSocket();
	QSslSocket *getStableSocket();
	void setupSocket(QSslSocket *socket);
	bool isSocketConnected(QSslSocket *socket);
	void reconnectSocket(QSslSocket *socket, bool mastAbort);
	QList<QSslSocket *>socketsList;
	void setApiDown(bool);
	QTime requestDelay;
	bool apiDownState;
	int apiDownCount;
	int requestRetryCount;
	bool packetIsChunked;
	QByteArray buffer;
	qint32 packetChunkSize;
	bool nextPacketMastBeSize;
	bool endOfPacket;
	void clearRequest();
	void retryRequest();

	QTime requestTimeOut;
	QByteArray *pendingRequest;
	QList<QPair<QByteArray*,int>>requestList;
	QMap<QByteArray*,int> retryCountMap;
	QMap<int,int> reqTypePending;
	QMap<QByteArray*,bool> skipOnceMap;

	QList<QPair<QByteArray*,int>>preparedList;

	void takeFirstRequest();
	void takeRequestAt(int);
	QByteArray restKeyLine;
	QByteArray httpHeader;
	QString hostName;
private slots:
	void sslErrorsSlot(const QList<QSslError> &);
	void connected();
	void errorSlot(QAbstractSocket::SocketError);
	void sendPendingData();
	void readSocket();
signals:
	void sslErrors(const QList<QSslError> &);
	void softLagChanged(int);
	void apiDown(bool);
	void dataReceived(QByteArray,int);
};

#endif // JULYHTTP_H
