//Created by July IGHOR
//Feel free to contact me: julyighor@gmail.com
//Bitcoin Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc

#include "socketthread.h"
#include "main.h"

SocketThread::SocketThread(int threadId_)
	: QThread()
{
	waitingNewData=false;
	sendState=0;
	//traffic=0;
	//lastTraffic=0;
	threadId=threadId_;
	isLoopRunning=true;
	moveToThread(this);
	start();
	if(isLogEnabled)logThread->writeLog("Program started");
}

SocketThread::~SocketThread()
{
	isLoopRunning=false;
}

void SocketThread::run()
{
	{
	queryTime.restart();
	if(isLogEnabled)logThread->writeLog("SocketThread Started");
	sslSocket=new QSslSocket;
	connect(this,SIGNAL(sendToApiSignal(QByteArray, QByteArray)),this,SLOT(sendToApiSlot(QByteArray, QByteArray)));
	connect(this,SIGNAL(reconnectApiSignal()),this,SLOT(reconnectApiSlot()));
	connect(sslSocket,SIGNAL(readyRead()),SLOT(readSocket()));

	secondTimer=new QTimer;
	connect(secondTimer,SIGNAL(timeout()),this,SLOT(secondSlot()));

	if(useSSL)sslSocket->connectToHostEncrypted(hostName, 443);
		else  sslSocket->connectToHost(hostName, 80);
	sslSocket->waitForConnected();
	if(isLogEnabled)
	{
		logThread->writeLog("SSL Socket state:"+sslSocket->errorString().toAscii()+". Supported: "+QByteArray::number(sslSocket->supportsSsl()));
	}
	
	secondTimer->start(100);
	sendPendingData();
	}
	exec();
}

void SocketThread::reconnectApi()
{
	emit reconnectApiSignal();
}

void SocketThread::reconnectApiSlot()
{
	sslSocket->disconnectFromHost();
}

void SocketThread::secondSlot()
{
	checkSocketConnected();
	if(queryTime.elapsed()>800)sendPendingData();
}

void SocketThread::sendPendingData()
{
	while(listToSend.count())
	{
		QByteArray dataToSend;
		getDataFromList(&listToSend,&dataToSend);
		checkSocketConnected();
		sslSocket->write(dataToSend);
		sslSocket->waitForBytesWritten();
		if(listToSend.count()>100)listToSend.clear();
	}

	if(queryTime.elapsed()<200)return;
	if(sendState==0)sendToApiNow("BTCUSD/money/order/lag","");
	else
		switch(sendState)
		{
			case 1: sendToApiNow("BTCUSD/money/info",""); break;
			case 2: sendToApiNow("BTCUSD/money/ticker",""); break;
			case 3: sendToApiNow("BTCUSD/money/orders",""); break;
			default: break;
		}

	if(++sendState>3)sendState=0;
	queryTime.restart();
}

void SocketThread::sendToApiNow(QByteArray request, QByteArray command)
{
	QByteArray dataToSend;
	getDataFromPair(&request,&command,&dataToSend);
	if(!dataToSend.isEmpty())
	{
		checkSocketConnected();
		sslSocket->write(dataToSend);
		sslSocket->waitForBytesWritten();
	}
}

QByteArray SocketThread::hmacSha512(QByteArray key, QByteArray baseString)
{
	return QByteArray(reinterpret_cast<const char *>(HMAC(EVP_sha512(),key.constData(), key.size(), reinterpret_cast<const unsigned char *>(baseString.constData()), baseString.size(), 0, 0)),64).toBase64();
}

void SocketThread::sendToApi(QByteArray request, QByteArray command)
{
	emit sendToApiSignal(request,command);
}
void SocketThread::sendToApiSlot(QByteArray request, QByteArray command)
{
	QPair<QByteArray,QByteArray>newPair;
	newPair.first=request;
	newPair.second=command;
	listToSend<<newPair;
}

void SocketThread::checkDataAndSend(QByteArray* data)
{
	if(data->size()<3)return;
	if(data->at(0)!='{'&&data->at(data->size()-1)!='}')return;

	int leftl=0;
	int rightl=0;

	for(int n=0;n<data->size();n++)
	{
		if(data->at(n)=='{')leftl++;
		else
		if(data->at(n)=='}')rightl++;
	}

	if(leftl&&leftl==rightl)
	{
		emit dataReceived(*data);
		data->clear();
	}
}

void SocketThread::readSocket()
{
	QByteArray currentAllData=sslSocket->readAll();
	while(currentAllData.size())
	{
		QByteArray currentData;
		int lrlnlrln=currentAllData.indexOf("\r\n\r\n");

		if(lrlnlrln>-1){currentData=currentAllData.left(lrlnlrln-1);currentAllData.remove(0,lrlnlrln+4);}
		else {currentData=currentAllData;currentAllData.clear();}
		if(currentData.size()<2)continue;

		if(currentData.startsWith("HTTP"))
		{
			if(currentData.startsWith("HTTP/1.1 50")){emit apiDown();waitingNewData=false;return;}
			if(waitingNewData)
			{
				checkDataAndSend(&dataBuffer);
				if(isLogEnabled&&dataBuffer.size())logThread->writeLog("Ignored Corrupt Data: "+dataBuffer);
				dataBuffer.clear();
				waitingNewData=false;
			}
			continue;
		}

		if(currentData.right(2)=="\r\n")currentData.remove(currentData.size()-2,2);

		int firstlRlN=currentData.indexOf("\r\n");
		if(firstlRlN>-1&&currentData.size()>=firstlRlN+2)currentData.remove(0,firstlRlN+2);

		checkDataAndSend(&currentData);
		if(!currentData.isEmpty())
		{
			dataBuffer.append(currentData);
			waitingNewData=true;
			checkDataAndSend(&dataBuffer);
			if(dataBuffer.isEmpty())
			{
				waitingNewData=false;
				sendPendingData();
			}
		}
	}
}

void SocketThread::checkSocketConnected()
{
	if(sslSocket->state()==0)
	{
		if(useSSL)sslSocket->connectToHostEncrypted(hostName, 443);
		else  sslSocket->connectToHost(hostName, 80);
		sslSocket->waitForConnected();
	}
}

void SocketThread::getDataFromList(QList< QPair<QByteArray,QByteArray> >* list, QByteArray *dataToReturn)
{
	if(!list->count())return;

	QByteArray metod=list->first().first;
	QByteArray commands=list->first().second;
	list->removeFirst();
	getDataFromPair(&metod,&commands,dataToReturn);
}

void SocketThread::getDataFromPair(QByteArray *metod, QByteArray *commands, QByteArray *dataToReturn)
{
	QByteArray postData="nonce="+QByteArray::number(++nonce)+"000000"+*commands;

	QByteArray forHash=postData;
	if(apiId=="2")
	{
		forHash.prepend(*metod+"0");
		forHash[metod->size()]=0;
	}
*dataToReturn="POST /api/"+apiId+"/"+*metod+" HTTP/1.1\r\n"
	 "User-Agent: Qt Bitcoin Trader v"+appVerStr+"\r\n"
	 "Host: "+hostName+"\r\n"
	 "Rest-Key: "+restKey+"\r\n"
	 "Rest-Sign: "+hmacSha512(restSign,forHash)+"\r\n"
	 "Content-Type: application/x-www-form-urlencoded\r\n"
	 "Content-Length: "+QByteArray::number(postData.size())+"\r\n\r\n"+postData;
}