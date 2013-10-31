// Copyright (C) 2013 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form https://sourceforge.net/projects/bitcointrader/
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#include "julyhttp.h"
#include "main.h"
#include <QTimer>
#include <zlib.h>

JulyHttp::JulyHttp(const QString &hostN, const QByteArray &restLine, QObject *parent, const bool &secure, const bool &keepAlive, const QByteArray &contentType)
	: QObject(parent)
{
	secureConnection=secure;
	isDataPending=false;
	httpState=999;
	connectionClose=false;
	bytesDone=0;
	contentLength=0;
	chunkedSize=-1;
	readingHeader=false;
	waitingReplay=false;
	isDisabled=false;
	outGoingPacketsCount=0;
	contentGzipped=false;

	socket=new QSslSocket(this);
	setupSocket(socket);
	
	requestTimeOut.restart();
	hostName=hostN;
	httpHeader.append(" HTTP/1.1\r\n");
	httpHeader.append("User-Agent: Qt Bitcoin Trader v"+appVerStr+"\r\n");
	httpHeader.append("Host: "+hostName+"\r\n");
	httpHeader.append("Accept-Encoding: gzip\r\n");
	httpHeader.append("Content-Type: "+contentType+"\r\n");
	if(keepAlive)httpHeader.append("Connection: keep-alive\r\n");
	else httpHeader.append("Connection: close\r\n");
	apiDownState=false;
	apiDownCounter=0;
	restKeyLine=restLine;

	QTimer *secondTimer=new QTimer(this);
	connect(secondTimer,SIGNAL(timeout()),this,SLOT(sendPendingData()));
	secondTimer->start(300);
}

JulyHttp::~JulyHttp()
{
	abortSocket();
}

void JulyHttp::setupSocket(QSslSocket *socket)
{
	static QList<QSslCertificate> certs;
	if(certs.count()==0)
	{
		QFile readCerts(":/Resources/CertBase.cer");
		if(readCerts.open(QIODevice::ReadOnly))
		{
			QByteArray certData=readCerts.readAll()+"{SPLIT}";
			readCerts.close();
			do 
			{
			int nextCert=certData.indexOf("{SPLIT}");
			if(nextCert>-1)
			{
				QByteArray currentCert=certData.left(nextCert);
				QSslCertificate derCert(currentCert,QSsl::Der);
				if(derCert.isValid())certs<<derCert;
				certData.remove(0,nextCert+7);
			}
			else certData.clear();
			}while(certData.size());
		}
	}

	if(certs.count())
	{
	socket->addCaCertificates(certs);
	socket->addDefaultCaCertificates(certs);
	}

	socket->setPeerVerifyMode(QSslSocket::VerifyPeer);
	socket->setSocketOption(QAbstractSocket::KeepAliveOption,true);
	connect(socket,SIGNAL(readyRead()),SLOT(readSocket()));
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
	if(isDisabled)return;
	reconnectSocket(socket,mastAbort);
	retryRequest();
}

void JulyHttp::abortSocket()
{
	if(socket==0)return;
	socket->blockSignals(true);
	socket->abort();
	socket->blockSignals(false);
}

void JulyHttp::reconnectSocket(QSslSocket *socket, bool mastAbort)
{
	if(isDisabled)return;
	if(socket==0)return;
	if(mastAbort)abortSocket();
	if(socket->state()==QAbstractSocket::UnconnectedState||socket->state()==QAbstractSocket::UnconnectedState)
	{
		if(secureConnection)socket->connectToHostEncrypted(hostName, 443, QIODevice::ReadWrite);
		else socket->connectToHost(hostName,80,QIODevice::ReadWrite);
	}
}

void JulyHttp::setApiDown(bool httpError)
{
	if(httpError)apiDownCounter++;else apiDownCounter=0;

	bool currentApiDownState=apiDownCounter>apiDownCount;
	if(apiDownState!=currentApiDownState)
	{
		apiDownState=currentApiDownState;
		emit apiDown(apiDownState);
	}
}

void JulyHttp::readSocket()
{
	if(isDisabled)return;

	requestTimeOut.restart();

	if(!waitingReplay)
	{
		httpState=999;
		bytesDone=0;
		connectionClose=false;
		buffer.clear();
		contentGzipped=false;
		waitingReplay=true;
		readingHeader=true;
		contentLength=0;
	}

	while(readingHeader)
	{
		bool endFound=false;
		QByteArray currentLine;
		while(!endFound&&socket->canReadLine())
		{
			currentLine=socket->readLine();
			if(currentLine=="\r\n"||
			   currentLine=="\n"||
			   currentLine.isEmpty())endFound=true;
			else
			{
				QString currentLineLow=currentLine.toLower();
				if(currentLineLow.startsWith("http/1.1 "))
				{
					if(currentLineLow.length()>12)httpState=currentLineLow.mid(9,3).toInt();
				}
				else
				if(currentLineLow.startsWith("set-cookie"))
				{
					cookie=currentLine;
					cookie.remove(0,4);
				}
				else 
				if(currentLineLow.startsWith("transfer-encoding")&&
				   currentLineLow.endsWith("chunked\r\n"))chunkedSize=0;
				else
				if(currentLineLow.startsWith(QLatin1String("content-length")))
				{
					QStringList pairList=currentLineLow.split(":");
					if(pairList.count()==2)contentLength=pairList.last().trimmed().toUInt();
				}
				else
				if(currentLineLow.startsWith(QLatin1String("connection"))&&
					currentLineLow.endsWith(QLatin1String("close\r\n")))connectionClose=true;
				else
				if(currentLineLow.startsWith(QLatin1String("content-encoding"))&&
					currentLineLow.contains(QLatin1String("gzip")))contentGzipped=true;
			}
		}
		if(!endFound)
		{
			retryRequest();
			return;
		}
		readingHeader=false;
	}
	if(httpState<400)emit anyDataReceived();

	bool allDataReaded=false;

		qint64 readSize=socket->bytesAvailable();

		QByteArray *dataArray=0;
		if(chunkedSize!=-1)
		{
			while(true)
			{
				if(chunkedSize==0)
				{
					if(!socket->canReadLine())break;
					QString sizeString=socket->readLine();
					int tPos=sizeString.indexOf(QLatin1Char(';'));
					if(tPos!=-1)sizeString.truncate(tPos);
					bool ok;
					chunkedSize=sizeString.toInt(&ok,16);
					if(!ok)
					{
						if(isLogEnabled)logThread->writeLog("Invalid size");
						if(dataArray){delete dataArray;dataArray=0;}
						retryRequest();
						return;
					}
					if(chunkedSize==0)chunkedSize=-2;
				}

				while(chunkedSize==-2&&socket->canReadLine())
				{
					QString currentLine=socket->readLine();
					 if(currentLine==QLatin1String("\r\n")||
						currentLine==QLatin1String("\n"))chunkedSize=-1;
				}
				if(chunkedSize==-1)
				{
					allDataReaded=true;
					break;
				}

				readSize=socket->bytesAvailable();
				if(readSize==0)break;
				if(readSize==chunkedSize||readSize==chunkedSize+1)
				{
					readSize=chunkedSize-1;
					if(readSize==0)break;
				}

				qint64 bytesToRead=chunkedSize<0?readSize:qMin(readSize,chunkedSize);
				if(!dataArray)dataArray=new QByteArray;
				quint32 oldDataSize=dataArray->size();
				dataArray->resize(oldDataSize+bytesToRead);
				qint64 read=socket->read(dataArray->data()+oldDataSize,bytesToRead);
				dataArray->resize(oldDataSize+read);

				chunkedSize-=read;
				if(chunkedSize==0&&readSize-read>=2)
				{
					char twoBytes[2];
					socket->read(twoBytes,2);
					if(twoBytes[0]!='\r'||twoBytes[1]!='\n')
					{
						if(isLogEnabled)logThread->writeLog("Invalid HTTP chunked body");
						if(dataArray){delete dataArray;dataArray=0;}
						retryRequest();
						return;
					}
				}
			}
		} 
		else
			if(contentLength>0)
			{
			readSize=qMin(qint64(contentLength-bytesDone),readSize);
			if(readSize>0)
			{
				if(dataArray){delete dataArray;dataArray=0;}
				dataArray=new QByteArray;
				dataArray->resize(readSize);
				dataArray->resize(socket->read(dataArray->data(),readSize));
			}
			if(bytesDone+socket->bytesAvailable()+readSize==contentLength)allDataReaded=true;
			}
			else 
			if(readSize>0)
			{
			if(!dataArray)dataArray=new QByteArray(socket->readAll());
			}

		if(dataArray)
		{
				readSize=dataArray->size();
				if(readSize>0)buffer.append(*dataArray);
				if(dataArray){delete dataArray;dataArray=0;}
				if(contentLength>0)
				{
					bytesDone+=readSize;
					emit dataProgress(100*bytesDone/contentLength);
				}
		}
		if(dataArray){delete dataArray;dataArray=0;}

	if(allDataReaded)
	{
		if(!buffer.isEmpty()&&requestList.count())
		{
			if(contentGzipped)uncompress(&buffer);
			bool apiMaybeDown=buffer[0]=='<';
			setApiDown(apiMaybeDown);
			if(isLogEnabled)logThread->writeLog("RCV: "+buffer);
			if(!apiMaybeDown)emit dataReceived(buffer,requestList.first().second);
		}
		waitingReplay=false;
		readingHeader=true;
		takeFirstRequest();
		clearRequest();
		if(connectionClose)
		{
			if(isLogEnabled)logThread->writeLog("HTTP: connection closed");
			reConnect(true);
		}
		sendPendingData();
	}
}

void JulyHttp::uncompress(QByteArray *data)
{
	if(data->size()<=4)
	{
		if(isLogEnabled)logThread->writeLog("GZIP: Input data is truncated");
		return;
	}

	QByteArray result;

	static const int CHUNK_SIZE=1024;
	char out[CHUNK_SIZE];

	z_stream strm;
	strm.zalloc=Z_NULL;
	strm.zfree=Z_NULL;
	strm.opaque=Z_NULL;
	strm.avail_in=data->size();
	strm.next_in=(Bytef*)(data->data());

	int ret=inflateInit2(&strm,47);
	if(ret!=Z_OK)return;

	do
	{
		strm.avail_out=CHUNK_SIZE;
		strm.next_out=(Bytef*)(out);

		ret=inflate(&strm,Z_NO_FLUSH);
		Q_ASSERT(ret!=Z_STREAM_ERROR);
		switch(ret)
		{
		case Z_NEED_DICT: ret=Z_DATA_ERROR;
		case Z_DATA_ERROR:
		case Z_MEM_ERROR: (void)inflateEnd(&strm);
			return;
		}
		result.append(out, CHUNK_SIZE-strm.avail_out);
	} 
	while(strm.avail_out==0);

	inflateEnd(&strm);
	(*data)=result;
}

bool JulyHttp::isReqTypePending(int val)
{
	return reqTypePending.value(val,0)>0;
}

void JulyHttp::retryRequest()
{
	if(isDisabled)return;
	if(requestRetryCount<=0)takeFirstRequest();
	else requestRetryCount--;
	sendPendingData();
}

void JulyHttp::clearRequest()
{
	requestRetryCount=0;
	buffer.clear();
	chunkedSize=-1;
	nextPacketMastBeSize=false;
	endOfPacket=false;
}

void JulyHttp::prepareData(int reqType, const QByteArray &method, QByteArray postData, const QByteArray &restSignLine, const int &forceRetryCount)
{
	if(isDisabled)return;
	QByteArray *data=new QByteArray(method+httpHeader+cookie);
	if(!restSignLine.isEmpty())data->append(restKeyLine+restSignLine);
	if(!postData.isEmpty())
	{
		data->append("Content-Length: "+QByteArray::number(postData.size())+"\r\n\r\n");
		data->append(postData);
	}
	else data->append("\r\n");

	QPair<QByteArray*,int> reqPair;
	reqPair.first=data;
	reqPair.second=reqType;
	preparedList<<reqPair;
	if(forceRetryCount==-1)
	{
	if(reqType>300)retryCountMap[data]=httpRetryCount-1;
	else retryCountMap[data]=0;
	}
	else retryCountMap[data]=forceRetryCount;
	reqTypePending[reqType]=reqTypePending.value(reqType,0)+1;
}

void JulyHttp::prepareDataSend()
{
	if(isDisabled)return;
	if(preparedList.count()==0)return;

	for(int n=1;n<preparedList.count();n++)
	{
		preparedList.at(0).first->append(*(preparedList.at(n).first))+"\r\n\r\n";
		skipOnceMap[preparedList.at(n).first]=true;
	}
	for(int n=0;n<preparedList.count();n++)requestList<<preparedList.at(n);
	preparedList.clear();
	if(isDataPending!=true)
	{
		emit setDataPending(true);
		isDataPending=true;
	}
}

void JulyHttp::prepareDataClear()
{
	if(isDisabled)return;
	for(int n=0;n<preparedList.count();n++)
	{
		QPair<QByteArray*,int> reqPair=preparedList.at(n);
		retryCountMap.remove(reqPair.first);
		reqTypePending[reqPair.second]=reqTypePending.value(reqPair.second,1)-1;
		if(reqPair.first)delete reqPair.first;
	}
	preparedList.clear();
}

void JulyHttp::sendData(int reqType, const QByteArray &method, QByteArray postData, const QByteArray &restSignLine, const int &forceRetryCount)
{
	if(isDisabled)return;
	QByteArray *data=new QByteArray(method+httpHeader+cookie);
	if(!restSignLine.isEmpty())data->append(restKeyLine+restSignLine);
	if(!postData.isEmpty())
	{
		data->append("Content-Length: "+QByteArray::number(postData.size())+"\r\n\r\n");
		data->append(postData);
	}
	else data->append("\r\n");

	if(reqType>300)
		for(int n=requestList.count()-1;n>=1;n--)
			if(requestList.at(n).second<300&&skipOnceMap.value(requestList.at(n).first,false)!=true)
				takeRequestAt(n);

	QPair<QByteArray*,int> reqPair;
	reqPair.first=data;
	reqPair.second=reqType;
	if(forceRetryCount==-1)
	{
	if(reqType>300)retryCountMap[data]=httpRetryCount-1;
	else retryCountMap[data]=0;
	}
	else retryCountMap[data]=forceRetryCount;
	requestList<<reqPair;

	if(isDataPending!=true)
	{
		emit setDataPending(true);
		isDataPending=true;
	}

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
	if(requestList.count()==0)
	{
		reqTypePending.clear();
		retryCountMap.clear();

		if(isDataPending!=false)
		{
			emit setDataPending(false);
			isDataPending=false;
		}
	}
}

void JulyHttp::takeFirstRequest()
{
	if(requestList.count()==0)return;
	takeRequestAt(0);
}

void JulyHttp::errorSlot(QAbstractSocket::SocketError socketError)
{
	if(socketError!=QAbstractSocket::RemoteHostClosedError||socketError!=QAbstractSocket::UnfinishedSocketOperationError)setApiDown(true);

	if(isLogEnabled)logThread->writeLog("SocketError: "+socket->errorString().toAscii());

	if(socketError==QAbstractSocket::ProxyAuthenticationRequiredError)
	{
		isDisabled=true;
		emit errorSignal(socket->errorString());
		abortSocket();
	}
	else reconnectSocket(socket,false);
}

bool JulyHttp::isSocketConnected(QSslSocket *socket)
{
	return socket->state()==QAbstractSocket::ConnectedState;
}

QSslSocket *JulyHttp::getStableSocket()
{
	if(isSocketConnected(socket))return socket;
	else reconnectSocket(socket,false);

	if(socket->state()!=QAbstractSocket::UnconnectedState)
	{
		if(socket->state()==QAbstractSocket::ConnectingState||socket->state()==QAbstractSocket::HostLookupState)socket->waitForConnected(5000);
	}
	if(socket->state()!=QAbstractSocket::ConnectedState)
	{
		setApiDown(true);
		if(isLogEnabled)logThread->writeLog("Socket error: "+socket->errorString().toAscii());
		reconnectSocket(socket,false);
		if(socket->state()==QAbstractSocket::ConnectingState)socket->waitForConnected(5000);
	}
	else reconnectSocket(socket,false);
	return socket;
}

void JulyHttp::sendPendingData()
{
	if(isDisabled)return;
	if(requestList.count()==0)return;

	QSslSocket *currentSocket=getStableSocket();

	if(currentSocket->state()!=QAbstractSocket::ConnectedState)return;

	QByteArray *pendingRequest=pendingRequestMap.value(currentSocket,0);
	if(pendingRequest==requestList.first().first)
	{
		if(requestTimeOut.elapsed()<httpRequestTimeout)return;
		else
		{
			if(isLogEnabled)logThread->writeLog(QString("Request timeout: %0>%1").arg(requestTimeOut.elapsed()).arg(httpRequestTimeout).toAscii());
			reconnectSocket(socket,true);
			if(requestRetryCount>0){retryRequest();return;}
		}
	}
	else
	{
		pendingRequestMap[currentSocket]=requestList.first().first;
		pendingRequest=pendingRequestMap.value(currentSocket,0);
	}
	clearRequest();
	requestRetryCount=retryCountMap.value(pendingRequest,0);
	if(requestRetryCount<0||requestRetryCount>100)requestRetryCount=0;
	requestTimeOut.restart();
	if(isLogEnabled)logThread->writeLog("SND: "+QByteArray(*pendingRequest).replace(restKey,"REST_KEY").replace(restSign,"REST_SIGN"));

	if(pendingRequest)
	{
		if(skipOnceMap.value(pendingRequest,false)==true)skipOnceMap.remove(pendingRequest);
		else
		{
			if(currentSocket->bytesAvailable())
			{
				if(isLogEnabled)logThread->writeLog("Cleared previous data: "+currentSocket->readAll());
				else currentSocket->readAll();
			}
			waitingReplay=false;
			currentSocket->write(*pendingRequest);
			currentSocket->flush();
		}
	}
	else if(isLogEnabled)logThread->writeLog("PendingRequest pointer not exist");
}

void JulyHttp::sslErrorsSlot(const QList<QSslError> &val)
{
	emit sslErrorSignal(val);
}