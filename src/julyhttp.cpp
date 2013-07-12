// Copyright (C) 2013 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form https://sourceforge.net/projects/bitcointrader/
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#include "julyhttp.h"
#include <openssl/hmac.h>
#include "main.h"
#include <QTimer>

JulyHttp::JulyHttp(const QString &hostN, const QByteArray &restLine, QObject *parent)
	: QObject(parent)
{
	incomingPacketsCount=0;

	for(int n=0;n<httpConnectionsCount;n++)
	{
		socketsList<<new QSslSocket(this);
		setupSocket(socketsList.last());
	}

	requestTimeOut.restart();
	requestDelay.restart();
	hostName=hostN;
	httpHeader.append(" HTTP/1.1\r\n");
	httpHeader.append("User-Agent: Qt Bitcoin Trader v"+appVerStr+"\r\n");
	httpHeader.append("Host: "+hostName+"\r\n");
	httpHeader.append("Connection: keep-alive\r\n");
	apiDownState=false;
	apiDownCount=0;
	pendingRequest=0;
	restKeyLine=restLine;

	QTimer *secondTimer=new QTimer(this);
	connect(secondTimer,SIGNAL(timeout()),this,SLOT(sendPendingData()));
	secondTimer->start(300);
}

JulyHttp::~JulyHttp()
{
	for(int n=0;n<socketsList.count();n++)socketsList.at(n)->abort();
}

void JulyHttp::setupSocket(QSslSocket *socket)
{
	socket->setSocketOption(QAbstractSocket::LowDelayOption,true);
	socket->setSocketOption(QAbstractSocket::KeepAliveOption,true);
	connect(socket,SIGNAL(readyRead()),SLOT(readSocket()));
	connect(socket,SIGNAL(connected()),SLOT(connected()));
	connect(socket,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(errorSlot(QAbstractSocket::SocketError)));
	connect(socket,SIGNAL(sslErrors(const QList<QSslError> &)),this,SLOT(sslErrorsSlot(const QList<QSslError> &)));
}

void JulyHttp::clearPendingData()
{
	for(int n=requestList.count()-1;n>=0;n--)takeRequestAt(n);
	reConnect();
}

void JulyHttp::reConnect(bool mastAbort)
{
	softLagChanged(requestDelay.elapsed());
	for(int n=0;n<socketsList.count();n++)reconnectSocket(socketsList.at(n),mastAbort);
	retryRequest();
}

void JulyHttp::reconnectSocket(QSslSocket *socket, bool mastAbort)
{
	if(mastAbort)socket->abort();
	if(socket->state()==QAbstractSocket::UnconnectedState||socket->state()==QAbstractSocket::UnconnectedState)
		socket->connectToHostEncrypted(hostName, 443, QIODevice::ReadWrite);
}

void JulyHttp::setApiDown(bool httpError)
{
	if(httpError)apiDownCount++;else apiDownCount=0;

	bool currentApiDownState=apiDownCount>5;
	if(apiDownState!=currentApiDownState)
	{
		apiDownState=currentApiDownState;
		emit apiDown(apiDownState);
	}
}

void JulyHttp::readSocket()
{
	QSslSocket *socketSender=qobject_cast<QSslSocket *>(QObject::sender());

	if(++incomingPacketsCount>50)swapSockets();
	
	int currentLineDataType=0;
	emit softLagChanged(requestDelay.elapsed());
	while(socketSender->bytesAvailable())
	{
		static	QByteArray currentLine;
		currentLine=socketSender->readLine();

		if(isLogEnabled)logThread->writeLog("RCV: "+currentLine);

		if(currentLine.isEmpty())continue;
		if(currentLine.startsWith("HTTP/1"))
		{
			if(!buffer.isEmpty())
			{
				if(isLogEnabled)logThread->writeLog("Buffer not empty:"+buffer);
				buffer.clear();
			}
			currentLineDataType=1;
			endOfPacket=false;
			continue;
		}
		
		switch(currentLineDataType)
		{
		case 0:
			if(nextPacketMastBeSize&&currentLine.size()>1&&currentLine.size()<=8&&currentLine.right(2)=="\r\n")
			{
				currentLine.remove(currentLine.size()-2,2);
				quint16 currentChunkSize=currentLine.toUShort(0,16);
				if(currentChunkSize==0)endOfPacket=true;
				else packetChunkSize+=currentChunkSize;
				nextPacketMastBeSize=false;
				break;
			}
			else
			{
				if(currentLine.size()>1&&currentLine.right(2)=="\r\n")
				{
					currentLine.remove(currentLine.size()-2,2);
					nextPacketMastBeSize=true;
				}
				requestTimeOut.restart();
				buffer.append(currentLine);
				if(buffer.size()>5&&buffer.at(0)=='<'&&buffer.right(5)=="html>")buffer.clear();
			}
			break;
		case 1:
			if(currentLine.toLower().startsWith("set-cookie:"))cookie=currentLine;
			if(currentLine.endsWith("chunked")){packetIsChunked=true;break;}
			if(currentLine!="\r\n")break;
			currentLineDataType=0;
			nextPacketMastBeSize=true;
			break;
		}
		if(endOfPacket)
		{
			if(packetChunkSize<buffer.size())
			{
				if(isLogEnabled)logThread->writeLog("Overloaded packet: "+buffer);
				buffer.remove(0,buffer.size()-packetChunkSize);
			}
			
			bool unknownPacket=packetChunkSize!=buffer.size()||buffer.size()&&buffer[0]=='<';
			setApiDown(unknownPacket);
	
			requestDelay.restart();

			if(!unknownPacket&&requestList.count())
			{
				emit dataReceived(buffer,requestList.first().second);
				if(isLogEnabled)logThread->writeLog("RCV: "+buffer);
			}
			else if(isLogEnabled)logThread->writeLog("Unknown response: "+buffer);
			takeFirstRequest();
			clearRequest();
			sendPendingData();
		}
	}
}

bool JulyHttp::isReqTypePending(int val)
{
	return reqTypePending.value(val,0)>0;
}

void JulyHttp::retryRequest()
{
	if(requestRetryCount<=0)takeFirstRequest();
	else requestRetryCount--;
	sendPendingData();
}

void JulyHttp::clearRequest()
{
	requestRetryCount=0;
	packetIsChunked=false;
	buffer.clear();
	packetChunkSize=0;
	nextPacketMastBeSize=false;
	endOfPacket=false;
}

void JulyHttp::prepareData(int reqType, const QByteArray &method, QByteArray postData, const QByteArray &restSignLine)
{
	QByteArray *data=new QByteArray(method+httpHeader+cookie);
	if(!restSignLine.isEmpty())data->append(restKeyLine+restSignLine);
	if(!postData.isEmpty())
	{
		data->append("Content-Type: application/x-www-form-urlencoded\r\nContent-Length: "+QByteArray::number(postData.size())+"\r\n\r\n");
		data->append(postData);
	}
	else data->append("\r\n");

	QPair<QByteArray*,int> reqPair;
	reqPair.first=data;
	reqPair.second=reqType;
	preparedList<<reqPair;
	retryCountMap[data]=1;
	reqTypePending[reqType]=reqTypePending.value(reqType,0)+1;
}

void JulyHttp::prepareDataSend()
{
	if(preparedList.count()==0)return;

	for(int n=1;n<preparedList.count();n++)
	{
		preparedList.at(0).first->append(*(preparedList.at(n).first))+"\r\n\r\n";
		skipOnceMap[preparedList.at(n).first]=true;
	}
	for(int n=0;n<preparedList.count();n++)requestList<<preparedList.at(n);
	preparedList.clear();
}

void JulyHttp::prepareDataClear()
{
	for(int n=0;n<preparedList.count();n++)
	{
		QPair<QByteArray*,int> reqPair=preparedList.at(n);
		retryCountMap.remove(reqPair.first);
		reqTypePending[reqPair.second]=reqTypePending.value(reqPair.second,1)-1;
		if(reqPair.first)delete reqPair.first;
	}
	preparedList.clear();
}

void JulyHttp::sendData(int reqType, bool isVip, const QByteArray &method, int removeLowerReqTypes, QByteArray postData, const QByteArray &restSignLine)
{
	Q_UNUSED(removeLowerReqTypes);
	QByteArray *data=new QByteArray(method+httpHeader+cookie);
	if(!restSignLine.isEmpty())data->append(restKeyLine+restSignLine);
	if(!postData.isEmpty())
	{
		data->append("Content-Type: application/x-www-form-urlencoded\r\nContent-Length: "+QByteArray::number(postData.size())+"\r\n\r\n");
		data->append(postData);
	}
	else data->append("\r\n");

	QPair<QByteArray*,int> reqPair;
	reqPair.first=data;
	reqPair.second=reqType;
	//if(false&&removeLowerReqTypes>100)
	//{
	//	for(int n=requestList.count()-1;n>=1;n--)
	//		if(requestList.at(n).second<removeLowerReqTypes)takeRequestAt(n);
	//}
	if(isVip)retryCountMap[data]=2;
	else retryCountMap[data]=0;
	requestList<<reqPair;

	reqTypePending[reqType]=reqTypePending.value(reqType,0)+1;
	sendPendingData();
}

void JulyHttp::takeRequestAt(int pos)
{
	if(requestList.count()<=pos)return;
	QPair<QByteArray*,int> reqPair=requestList.at(pos);
	reqTypePending[reqPair.second]=reqTypePending.value(reqPair.second,1)-1;
	retryCountMap.remove(reqPair.first);
	if(isLogEnabled)logThread->writeLog("Data taken: "+*reqPair.first);
	delete reqPair.first;
	reqPair.first=0;
	requestList.removeAt(pos);
	if(pos==0)pendingRequest=0;
	if(requestList.count()==0)
	{
		reqTypePending.clear();
		retryCountMap.clear();
	}
}

void JulyHttp::takeFirstRequest()
{
	if(requestList.count()==0)return;
	takeRequestAt(0);
}

void JulyHttp::swapSockets()
{
	incomingPacketsCount=0;
	for(int n=1;n<socketsList.count();n++)socketsList.swap(n-1,n);
}

void JulyHttp::pickNextConnectedSocket()
{
	swapSockets();
	while(socketsList.first()->state()!=QAbstractSocket::ConnectedState)swapSockets();
}

void JulyHttp::connected()
{
	QSslSocket *socketSender=qobject_cast<QSslSocket *>(QObject::sender());
	if(socketSender!=socketsList.first())pickNextConnectedSocket();
}

void JulyHttp::errorSlot(QAbstractSocket::SocketError socketError)
{
	QSslSocket *socketSender=qobject_cast<QSslSocket *>(QObject::sender());
	if(isLogEnabled)logThread->writeLog("SocetError: "+socketSender->errorString().toAscii());
	if(socketError==QAbstractSocket::RemoteHostClosedError)reconnectSocket(socketSender,false);
}

bool JulyHttp::isSocketConnected(QSslSocket *socket)
{
	return socket->state()==QAbstractSocket::ConnectedState;
}

QSslSocket *JulyHttp::getStableSocket()
{
	for(int n=0;n<socketsList.count();n++)
	if(isSocketConnected(socketsList.at(n)))return socketsList.at(n);
	else reconnectSocket(socketsList.at(n),false);

	if(socketsList.first()->state()!=QAbstractSocket::UnconnectedState)socketsList.first()->waitForConnected();
	if(socketsList.first()->state()!=QAbstractSocket::ConnectedState)
	{
		reConnect();
		socketsList.first()->waitForConnected();
	}
	else reconnectSocket(socketsList.first(),false);
	return socketsList.first();
}

void JulyHttp::sendPendingData()
{
	if(requestList.count()==0)return;

	QSslSocket *currentSocket=getStableSocket();

	if(pendingRequest==requestList.first().first)
	{
		if(requestTimeOut.elapsed()<httpRequestTimeout)
		{
			softLagChanged(requestDelay.elapsed());
			return;
		}
		else
		{
			if(isLogEnabled)logThread->writeLog(QString("Request timeout: %0<%1").arg(requestTimeOut.elapsed()).arg(httpRequestTimeout).toAscii());
			if(requestRetryCount>0){retryRequest();return;}
		}
	}
	else pendingRequest=requestList.first().first;
	clearRequest();
	softLagChanged(requestDelay.elapsed());
	requestRetryCount=retryCountMap.value(pendingRequest,1);
	if(requestRetryCount<1||requestRetryCount>100)requestRetryCount=1;
	requestDelay.restart();
	requestTimeOut.restart();
	if(isLogEnabled)logThread->writeLog("SND: "+*pendingRequest);

	if(pendingRequest)
	{
		if(skipOnceMap.value(pendingRequest,false)==true)skipOnceMap.remove(pendingRequest);
		else
		{
			currentSocket->write(*pendingRequest);
			currentSocket->flush();
			currentSocket->waitForBytesWritten();
		}
	}
	else if(isLogEnabled)logThread->writeLog("PendingRequest pointer not exist");
}

void JulyHttp::sslErrorsSlot(const QList<QSslError> &val)
{
	emit sslErrors(val);
}