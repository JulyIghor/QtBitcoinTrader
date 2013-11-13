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

struct PacketItem
{
	QByteArray *data;
	int reqType;
	int retryCount;
	bool skipOnce;
};

class JulyHttp : public QSslSocket
{
	Q_OBJECT

public:
	uint getCurrentPacketContentLength(){return contentLength;}
	void clearPendingData();
	void reConnect(bool mastAbort=true);
	bool isReqTypePending(int);
	void sendData(int reqType, const QByteArray &method, QByteArray postData=0, const QByteArray &restSignLine=0, const int &forceRetryCount=-1);

	void prepareData(int reqType, const QByteArray &method, QByteArray postData=0, const QByteArray &restSignLine=0, const int &forceRetryCount=-1);
	void prepareDataSend();
	void prepareDataClear();

	JulyHttp(const QString &hostName, const QByteArray &restKeyLine, QObject *parent, const bool &secure=true, const bool &keepAlive=true, const QByteArray &contentType="application/x-www-form-urlencoded");
	~JulyHttp();

private:
	bool secureConnection;
	bool isDataPending;
	void uncompress(QByteArray *data);
	bool contentGzipped;
	QByteArray *currentPendingRequest;
	bool connectionClose;
	int httpState;
	qint64 bytesDone;
	uint contentLength;
	bool waitingReplay;
	bool readingHeader;
	qint64 chunkedSize;

	void abortSocket();
	bool isDisabled;
	QByteArray cookie;
	int outGoingPacketsCount;
	void setupSocket();
	bool isSocketConnected();
	void reconnectSocket(bool mastAbort);
	void setApiDown(bool);
	bool apiDownState;
	int apiDownCounter;
	bool packetIsChunked;
	QByteArray buffer;
	bool nextPacketMastBeSize;
	bool endOfPacket;
	void clearRequest();
	void retryRequest();

	QTime requestTimeOut;
	QList<PacketItem>requestList;
	QMap<int,int> reqTypePending;

	QList<PacketItem>preparedList;

	void takeFirstRequest();
	void takeRequestAt(int);
	QByteArray restKeyLine;
	QByteArray httpHeader;
	QString hostName;
private slots:
	void sslErrorsSlot(const QList<QSslError> &);
	void errorSlot(QAbstractSocket::SocketError);
	void sendPendingData();
	void readSocket();
signals:
	void setDataPending(bool);
	void dataProgress(int);
	void anyDataReceived();
	void errorSignal(QString);
	void sslErrorSignal(const QList<QSslError> &);
	void apiDown(bool);
	void dataReceived(QByteArray,int);
};

#endif // JULYHTTP_H
