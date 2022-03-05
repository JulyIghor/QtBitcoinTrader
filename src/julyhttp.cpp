//  This file is part of Qt Bitcoin Trader
//      https://github.com/JulyIGHOR/QtBitcoinTrader
//  Copyright (C) 2013-2022 July Ighor <julyighor@gmail.com>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  In addition, as a special exception, the copyright holders give
//  permission to link the code of portions of this program with the
//  OpenSSL library under certain conditions as described in each
//  individual source file, and distribute linked combinations including
//  the two.
//
//  You must obey the GNU General Public License in all respects for all
//  of the code used other than OpenSSL. If you modify file(s) with this
//  exception, you may extend this exception to your version of the
//  file(s), but you are not obligated to do so. If you do not wish to do
//  so, delete this exception statement from your version. If you delete
//  this exception statement from all source files in the program, then
//  also delete it here.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "julyhttp.h"
#include "main.h"
#include <QFile>
#include <QMutex>
#include <QTimer>
#include <QWaitCondition>
#include <zlib.h>

#ifdef Q_OS_WIN
#include <winsock2.h>
#else
#include <fcntl.h>
#include <net/if.h>
#include <netdb.h>
#include <sys/socket.h>
#endif

JulyHttp::JulyHttp(const QString& hostN,
                   const QByteArray& restLine,
                   QObject* parent,
                   const bool& secure,
                   const bool& keepAlive,
                   const QByteArray& contentType) :
    QSslSocket(parent), ignoreError(false), waitForReadyReadCount(0)
{
    destroyClass = false;
    noReconnect = false;
    forcedPort = 0U;
    noReconnectCount = 0;
    secureConnection = secure;
    isDataPending = false;
    httpState = 999;
    connectionClose = false;
    bytesDone = 0;
    contentLength = 0;
    chunkedSize = -1;
    readingHeader = false;
    waitingReplay = false;
    isDisabled = false;
    outGoingPacketsCount = 0;
    contentGzipped = false;

    setupSocket();

    requestTimeOut.restart();
    hostName = hostN;
    httpHeader.append(" HTTP/1.1\r\n");

    if (baseValues.customUserAgent.length() > 0)
        httpHeader.append("User-Agent: " + baseValues.customUserAgent.toLatin1() + "\r\n");
    else
        httpHeader.append("User-Agent: Qt Bitcoin Trader v" + baseValues.appVerStr + "\r\n");

    httpHeader.append("Host: " + hostName.toLatin1() + "\r\n");
    httpHeader.append("Accept: */*\r\n");

    if (baseValues.gzipEnabled)
        httpHeader.append("Accept-Encoding: gzip\r\n");

    contentTypeLine = "Content-Type: " + contentType + "\r\n";

    if (keepAlive)
        httpHeader.append("Connection: keep-alive\r\n");
    else
        httpHeader.append("Connection: close\r\n");

    apiDownState = false;
    apiDownCounter = 0;
    restKeyLine = restLine;

    if (baseValues.customCookies.length() > 0)
    {
        baseValues.customCookies.append(" ");
        QStringList cookieListStr = baseValues.customCookies.split("; ");

        for (int n = 0; n < cookieListStr.size(); n++)
        {
            QStringList nameValue = cookieListStr.at(n).split("=");

            if (nameValue.size() != 2 || nameValue.first().isEmpty() || nameValue.last().isEmpty())
                continue;

            cookiesMap.insert(nameValue.first().toLatin1(), nameValue.last().toLatin1());
        }
    }

    saveCookies();

    secondTimer = new QTimer(this);
    connect(secondTimer, &QTimer::timeout, this, &JulyHttp::sendPendingData);
    secondTimer->start(300);
}

JulyHttp::~JulyHttp()
{
    delete secondTimer;
    abortSocket();
}

void JulyHttp::setupSocket()
{
    //    static QMutex mutex;
    //    mutex.lock();
    //    static QList<QSslCertificate> certs;

    //    if (certs.size() == 0)
    //    {
    //        QFile readCerts(":/Resources/CertBase.cer");

    //        if (readCerts.open(QIODevice::ReadOnly))
    //        {
    //            QByteArray certData = readCerts.readAll() + "{SPLIT}";
    //            readCerts.close();

    //            do
    //            {
    //                int nextCert = certData.indexOf("{SPLIT}");

    //                if (nextCert > -1)
    //                {
    //                    QByteArray currentCert = certData.left(nextCert);
    //                    QSslCertificate derCert(currentCert, QSsl::Der);

    //                    if (!derCert.isNull())
    //                        certs << derCert;

    //                    certData.remove(0, nextCert + 7);
    //                }
    //                else
    //                    certData.clear();
    //            }
    //            while (certData.size());
    //        }
    //    }

    //    setCaCertificates(certs);

    setPeerVerifyMode(QSslSocket::VerifyPeer);
    connect(this, &JulyHttp::readyRead, this, &JulyHttp::readSocket);
    connect(this, &JulyHttp::errorOccurred, this, &JulyHttp::errorSlot, Qt::QueuedConnection);
    connect(this, QOverload<const QList<QSslError>&>::of(&JulyHttp::sslErrors), this, &JulyHttp::sslErrorsSlot, Qt::QueuedConnection);

    //    mutex.unlock();
}

void JulyHttp::clearPendingData()
{
    for (int n = requestList.size() - 1; n >= 0; n--)
        takeRequestAt(n);

    reConnect();
}

void JulyHttp::reConnect(bool forceAbort)
{
    if (isDisabled)
        return;

    reconnectSocket(forceAbort);
}

void JulyHttp::abortSocket()
{
    blockSignals(true);
    abort();
    blockSignals(false);
}

void JulyHttp::reconnectSocket(bool forceAbort)
{
    if (destroyClass)
        return; //{qDebug("delete reconnectSocket1"); delete this; qDebug("delete reconnectSocket2");}

    if (isDisabled)
        return;

    if (forceAbort)
        abortSocket();

    if (state() == QAbstractSocket::UnconnectedState)
    {
        if (secureConnection)
            connectToHostEncrypted(hostName, forcedPort ? forcedPort : 443, QIODevice::ReadWrite);
        else
            connectToHost(hostName, forcedPort ? forcedPort : 80, QIODevice::ReadWrite);

        waitForConnected(((noReconnect && noReconnectCount++ > 5) ? 1000 : baseValues.httpRequestTimeout));

#ifdef Q_OS_WIN
        setsockopt(static_cast<SOCKET>(this->socketDescriptor()),
                   SOL_SOCKET,
                   SO_RCVTIMEO,
                   (const char*)&baseValues.httpRequestTimeout,
                   sizeof(int));
#else
        struct timeval vtime;
        vtime.tv_sec = baseValues.httpRequestTimeout / 1000;
        vtime.tv_usec = baseValues.httpRequestTimeout * 1000 - vtime.tv_sec * 1000000;
        setsockopt(this->socketDescriptor(), SOL_SOCKET, SO_RCVTIMEO, &vtime, sizeof(struct timeval));
#endif
    }
}

void JulyHttp::setApiDown(bool httpError)
{
    if (httpError)
        apiDownCounter++;
    else
        apiDownCounter = 0;

    bool currentApiDownState = apiDownCounter > baseValues.apiDownCount;

    if (apiDownState != currentApiDownState)
    {
        apiDownState = currentApiDownState;
        emit apiDown(apiDownState);
    }
}

void JulyHttp::saveCookies()
{
    cookieLine.clear();
    int cookiesCount = 0;

    for (const QByteArray& name : cookiesMap.keys())
    {
        if (cookiesCount > 0)
            cookieLine.append("; ");

        cookieLine.append(name + "=" + cookiesMap.value(name));
        cookiesCount++;
    }

    if (!cookieLine.isEmpty())
    {
        cookieLine.prepend("Cookie: ");
        cookieLine.append("\r\n");
    }
}

void JulyHttp::readSocket()
{
    if (isDisabled || requestList.isEmpty())
        return;

    requestTimeOut.restart();

    if (!waitingReplay)
    {
        httpState = 999;
        bytesDone = 0;
        connectionClose = false;
        buffer.clear();
        contentGzipped = false;
        waitingReplay = true;
        readingHeader = true;
        contentLength = 0;
    }

    bool lineReaded = false;

    while (readingHeader)
    {
        bool endFound = false;
        QByteArray currentLine;

        while (!endFound && canReadLine())
        {
            lineReaded = true;

            if (outBuffer.size())
            {
                currentLine = outBuffer + readLine();
                outBuffer.clear();
            }
            else
                currentLine = readLine();

            if (currentLine == "\r\n" || currentLine == "\n" || currentLine.isEmpty())
                endFound = true;
            else
            {
                QString currentLineLow = currentLine.toLower();

                if (currentLineLow.startsWith("http/1.1 "))
                {
                    if (currentLineLow.length() > 12)
                        httpState = currentLineLow.mid(9, 3).toInt();

                    if (debugLevel && httpState != 200)
                    {
                        logThread->writeLog(currentLine + readAll());
                        takeFirstRequest();
                        clearRequest();
                        return;
                    }
                }
                else if (currentLineLow.startsWith("set-cookie"))
                {
                    int cookieNameEnds = currentLine.indexOf(";");

                    if (cookieNameEnds == -1)
                        cookieNameEnds = currentLine.size() - 2;

                    if (cookieNameEnds > 13)
                    {
                        int cookieSplitPos = currentLine.indexOf("=");

                        if (cookieSplitPos != -1 && 0 < cookieSplitPos - 12 && 0 < cookieNameEnds - cookieSplitPos)
                        {
                            QByteArray cookieName = currentLine.mid(12, cookieSplitPos - 12);
                            QByteArray cookieValue = currentLine.mid(cookieSplitPos + 1, cookieNameEnds - cookieSplitPos - 1);

                            if (cookiesMap.value(cookieName) != cookieValue)
                            {
                                if (cookieValue.isEmpty())
                                    cookiesMap.remove(cookieName);
                                else
                                    cookiesMap[cookieName] = cookieValue;

                                saveCookies();
                            }
                        }
                    }
                }
                else if (currentLineLow.startsWith("transfer-encoding") && currentLineLow.endsWith("chunked\r\n"))
                    chunkedSize = 0;
                else if (currentLineLow.startsWith(QLatin1String("content-length")))
                {
                    QStringList pairList = currentLineLow.split(":");

                    if (pairList.size() == 2)
                        contentLength = pairList.last().trimmed().toUInt();
                }
                else if (currentLineLow.startsWith(QLatin1String("connection")) && currentLineLow.endsWith(QLatin1String("close\r\n")))
                    connectionClose = true;
                else if (currentLineLow.startsWith(QLatin1String("content-encoding")) && currentLineLow.contains(QLatin1String("gzip")))
                    contentGzipped = true;
            }
        }

        if (!endFound)
        {
            if (!lineReaded)
            {
                if (outBuffer.size() > 30000)
                    outBuffer.clear();

                outBuffer.append(readAll());
                return;
            }

            ++waitForReadyReadCount;

            if (waitForReadyReadCount > baseValues.httpRetryCount)
                retryRequest();

            return;
        }

        waitForReadyReadCount = 0;
        readingHeader = false;
    }

    if (httpState < 400)
        emit anyDataReceived();

    bool allDataReaded = false;

    qint64 readSize = bytesAvailable();

    addSpeedSize(readSize);

    QScopedPointer<QByteArray> dataArray;

    if (chunkedSize != -1)
    {
        while (true)
        {
            if (chunkedSize == 0)
            {
                if (!canReadLine())
                    break;

                QString sizeString = readLine();
                int tPos = sizeString.indexOf(QLatin1Char(';'));

                if (tPos != -1)
                    sizeString.truncate(tPos);

                bool ok;
                chunkedSize = sizeString.toInt(&ok, 16);

                if (!ok)
                {
                    if (debugLevel)
                        logThread->writeLog("Invalid size", 2);

                    retryRequest();
                    return;
                }

                if (chunkedSize == 0)
                    chunkedSize = -2;
            }

            while (chunkedSize == -2 && canReadLine())
            {
                QString currentLine = readLine();

                if (currentLine == QLatin1String("\r\n") || currentLine == QLatin1String("\n"))
                    chunkedSize = -1;
            }

            if (chunkedSize == -1)
            {
                allDataReaded = true;
                break;
            }

            readSize = bytesAvailable();

            if (readSize == 0)
                break;

            if (readSize == chunkedSize || readSize == chunkedSize + 1)
            {
                readSize = chunkedSize - 1;

                if (readSize == 0)
                    break;
            }

            qint64 bytesToRead = chunkedSize < 0 ? readSize : qMin(readSize, chunkedSize);

            if (!dataArray)
                dataArray.reset(new QByteArray);

            qint64 oldDataSize = dataArray->size();
            dataArray->resize(static_cast<int>(oldDataSize + bytesToRead));
            qint64 read = this->read(dataArray->data() + oldDataSize, bytesToRead);
            dataArray->resize(static_cast<int>(oldDataSize + read));

            chunkedSize -= read;

            if (chunkedSize == 0 && readSize - read >= 2)
            {
                char twoBytes[2];
                this->read(twoBytes, 2);

                if (twoBytes[0] != '\r' || twoBytes[1] != '\n')
                {
                    if (debugLevel)
                        logThread->writeLog("Invalid HTTP chunked body", 2);

                    retryRequest();
                    return;
                }
            }
        }
    }
    else if (contentLength > 0)
    {
        readSize = qMin(qint64(contentLength - bytesDone), readSize);

        if (readSize > 0)
        {
            dataArray.reset(new QByteArray);
            dataArray->resize(static_cast<int>(readSize));
            dataArray->resize(static_cast<int>(read(dataArray->data(), readSize)));
        }

        if (bytesDone + /* bytesAvailable() + */ readSize == contentLength)
            allDataReaded = true;
    }
    else if (readSize > 0)
    {
        if (!dataArray)
        {
            dataArray.reset(new QByteArray);
            dataArray->resize(static_cast<int>(readSize));
            dataArray->resize(static_cast<int>(read(dataArray->data(), readSize)));
        }
    }

    if (dataArray)
    {
        readSize = dataArray->size();

        if (readSize > 0)
            buffer.append(*dataArray);

        dataArray.reset();

        if (contentLength > 0)
        {
            bytesDone += readSize;
            emit dataProgress(static_cast<int>(100 * bytesDone / contentLength));
        }
    }

    dataArray.reset();

    if (allDataReaded)
    {
        if (!buffer.isEmpty() && !requestList.empty())
        {
            if (contentGzipped)
                gzipUncompress(&buffer);

            bool apiMaybeDown = buffer[0] == '<';
            setApiDown(apiMaybeDown);

            if (debugLevel && buffer.isEmpty())
                logThread->writeLog("Response is EMPTY", 2);

            emit dataReceived(buffer, requestList.first().reqType, requestList.first().pairChangeCount);
        }

        waitingReplay = false;
        readingHeader = true;

        if (!requestList.empty())
            requestList[0].retryCount = 0;

        takeFirstRequest();
        clearRequest();

        if (connectionClose)
        {
            if (debugLevel)
                logThread->writeLog("HTTP: connection closed");

            reConnect(true);
        }

        sendPendingData();
    }
}

void JulyHttp::gzipUncompress(QByteArray* data)
{
    if (data->size() <= 4)
    {
        if (debugLevel)
            logThread->writeLog("GZIP: Input data is truncated", 2);

        return;
    }

    QByteArray result;

    static const int CHUNK_SIZE = 1024;
    char out[CHUNK_SIZE];

    z_stream strm;
    strm.zalloc = nullptr;
    strm.zfree = nullptr;
    strm.opaque = nullptr;
    strm.avail_in = static_cast<unsigned int>(data->size());
    strm.next_in = (Bytef*)(data->data());

    int ret = inflateInit2(&strm, 47);

    if (ret != Z_OK)
        return;

    do
    {
        strm.avail_out = CHUNK_SIZE;
        strm.next_out = (Bytef*)(out);

        ret = inflate(&strm, Z_NO_FLUSH);
        Q_ASSERT(ret != Z_STREAM_ERROR);

        switch (ret)
        {
        case Z_NEED_DICT:
            ret = Z_DATA_ERROR;
            break;

        case Z_DATA_ERROR:
        case Z_MEM_ERROR:
            (void)inflateEnd(&strm);
            return;
        }

        result.append(out, CHUNK_SIZE - static_cast<int>(strm.avail_out));
    } while (strm.avail_out == 0);

    inflateEnd(&strm);
    (*data) = result;
}

bool JulyHttp::isReqTypePending(int val)
{
    return reqTypePending.value(val, 0) > 0;
}

void JulyHttp::retryRequest()
{
    if (isDisabled || requestList.isEmpty())
        return;

    if (requestList.first().retryCount <= 0)
        takeFirstRequest();
    else
    {
        if (debugLevel)
            logThread->writeLog("Warning: Request resent due timeout", 2);

        requestList[0].retryCount--;
    }

    sendPendingData();
}

void JulyHttp::clearRequest()
{
    buffer.clear();
    chunkedSize = -1;
    nextPacketMustBeSize = false;
    endOfPacket = false;
}

void JulyHttp::prepareData(int reqType,
                           int pairChangeCount,
                           const QByteArray& method,
                           QByteArray postData,
                           const QByteArray& restSignLine,
                           const int& forceRetryCount)
{
    if (isDisabled)
        return;

    auto* data = new QByteArray(method + httpHeader + cookieLine);

    if (!restSignLine.isEmpty())
        data->append(restKeyLine + restSignLine);

    if (!postData.isEmpty())
    {
        data->append(contentTypeLine);
        data->append("Content-Length: " + QByteArray::number(postData.size()) + "\r\n\r\n");
        data->append(postData);
    }

    PacketItem newPacket;
    newPacket.data = data;
    newPacket.reqType = reqType;
    newPacket.pairChangeCount = pairChangeCount;
    newPacket.retryCount = 0;
    newPacket.skipOnce = false;

    if (forceRetryCount == -1)
    {
        if (reqType > 300)
            newPacket.retryCount = baseValues.httpRetryCount - 1;
    }
    else
        newPacket.retryCount = forceRetryCount;

    reqTypePending[reqType] = reqTypePending.value(reqType, 0) + 1;

    preparedList << newPacket;
}

void JulyHttp::prepareDataSend()
{
    if (isDisabled || preparedList.isEmpty())
        return;

    for (int n = 1; n < preparedList.size(); n++)
    {
        preparedList[0].data->append(*(preparedList[n].data)) + "\r\n\r\n";
        preparedList[n].skipOnce = true;
    }

    for (int n = 0; n < preparedList.size(); n++)
        requestList << preparedList.at(n);

    preparedList.clear();

    if (!isDataPending)
    {
        emit setDataPending(true);
        isDataPending = true;
    }
}

void JulyHttp::prepareDataClear()
{
    if (isDisabled)
        return;

    for (int n = 0; n < preparedList.size(); n++)
    {
        PacketItem preparingPacket = preparedList.at(n);
        reqTypePending[preparingPacket.reqType] = reqTypePending.value(preparingPacket.reqType, 1) - 1;

        delete preparingPacket.data;
    }

    preparedList.clear();
}

void JulyHttp::sendData(int reqType,
                        int pairChangeCount,
                        const QByteArray& method,
                        QByteArray postData,
                        const QByteArray& restSignLine,
                        const int& forceRetryCount)
{
    if (isDisabled)
        return;

    auto* data = new QByteArray(method + httpHeader + cookieLine);

    if (!restSignLine.isEmpty())
        data->append(restKeyLine + restSignLine);

    if (!postData.isEmpty())
    {
        data->append(contentTypeLine);
        data->append("Content-Length: " + QByteArray::number(postData.size()) + "\r\n\r\n");
        data->append(postData);
    }
    else
        data->append("\r\n");

    if (reqType > 300)
        for (int n = requestList.size() - 1; n >= 1; n--)
            if (requestList.at(n).reqType < 300 && !requestList[n].skipOnce)
                takeRequestAt(n);

    PacketItem newPacket;
    newPacket.data = data;
    newPacket.reqType = reqType;
    newPacket.pairChangeCount = pairChangeCount;
    newPacket.retryCount = 0;
    newPacket.skipOnce = false;

    if (forceRetryCount == -1)
    {
        if (reqType > 300)
            newPacket.retryCount = baseValues.httpRetryCount - 1;
        else
            newPacket.retryCount = 0;
    }
    else
        newPacket.retryCount = forceRetryCount;

    if (newPacket.retryCount > 0 && debugLevel && newPacket.reqType > 299)
        logThread->writeLog("Added to Query RetryCount=" + QByteArray::number(newPacket.retryCount), 2);

    requestList << newPacket;

    if (!isDataPending)
    {
        emit setDataPending(true);
        isDataPending = true;
    }

    reqTypePending[reqType] = reqTypePending.value(reqType, 0) + 1;
    sendPendingData();
}

void JulyHttp::takeRequestAt(int pos)
{
    if (requestList.size() <= pos)
        return;

    if (pos == 0)
        currentPendingRequest = nullptr;

    PacketItem packetTake = requestList.at(pos);
    reqTypePending[packetTake.reqType] = reqTypePending.value(packetTake.reqType, 1) - 1;

    delete packetTake.data;
    packetTake.data = nullptr;
    requestList.removeAt(pos);

    if (requestList.isEmpty())
    {
        reqTypePending.clear();

        if (isDataPending)
        {
            emit setDataPending(false);
            isDataPending = false;
        }
    }
}

void JulyHttp::takeFirstRequest()
{
    if (requestList.isEmpty())
        return;

    takeRequestAt(0);
}

void JulyHttp::errorSlot(QAbstractSocket::SocketError socketError)
{
    if (ignoreError)
        return;

    if (noReconnect && noReconnectCount++ > 5)
    {
        isDisabled = true;
        return;
    }

    if (socketError != QAbstractSocket::RemoteHostClosedError && socketError != QAbstractSocket::UnfinishedSocketOperationError)
        setApiDown(true);

    if (debugLevel)
        logThread->writeLog("SocketError: " + errorString().toUtf8(), 2);

    if (socketError == QAbstractSocket::ProxyAuthenticationRequiredError)
    {
        isDisabled = true;
        emit errorSignal(errorString());
        abortSocket();
    }
    else
    {
        QMutex mutex;
        mutex.lock();

        QWaitCondition waitCondition;
        waitCondition.wait(&mutex, 1000);

        mutex.unlock();

        reconnectSocket(false);
    }
}

bool JulyHttp::isSocketConnected()
{
    return state() == QAbstractSocket::ConnectedState;
}

void JulyHttp::sendPendingData()
{
    if (isDisabled || requestList.isEmpty())
        return;

    if (!isSocketConnected())
        reconnectSocket(false);

    if (state() != QAbstractSocket::UnconnectedState)
    {
        if (state() == QAbstractSocket::ConnectingState || state() == QAbstractSocket::HostLookupState)
            waitForConnected(((noReconnect && noReconnectCount++ > 5) ? 1000 : baseValues.httpRequestTimeout + 1000));
    }

    if (!noReconnect)
    {
        if (!isSocketConnected())
        {
            setApiDown(true);

            if (debugLevel)
                logThread->writeLog("Socket state: " + errorString().toUtf8(), 2);

            reconnectSocket(false);

            if (state() == QAbstractSocket::ConnectingState)
                waitForConnected(baseValues.httpRequestTimeout + 1000);
        }
        else
            reconnectSocket(false);
    }

    if (!isSocketConnected())
        return;

    if (currentPendingRequest == requestList.first().data)
    {
        if (requestTimeOut.elapsed() < ((noReconnect && noReconnectCount++ > 5) ? 1000 : baseValues.httpRequestTimeout))
            return;

        if (debugLevel)
            logThread->writeLog(
                QString("Request timeout: %0>%1").arg(requestTimeOut.elapsed()).arg(baseValues.httpRequestTimeout).toLatin1(), 2);

        reconnectSocket(true);
        setApiDown(true);

        if (requestList.first().retryCount > 0)
        {
            retryRequest();
            return;
        }
    }
    else
    {
        currentPendingRequest = requestList.first().data;

        if (debugLevel && requestList.first().reqType > 299)
            logThread->writeLog("Sending request ID: " + QByteArray::number(requestList.first().reqType), 2);
    }

    clearRequest();

    requestTimeOut.restart();

    if (debugLevel && currentPendingRequest)
        logThread->writeLog("SND: " + QByteArray(*currentPendingRequest).replace(baseValues.restKey, "REST_KEY"));

    if (currentPendingRequest)
    {
        if (requestList.first().skipOnce)
            requestList[0].skipOnce = false;
        else
        {
            if (bytesAvailable())
            {
                if (debugLevel)
                    logThread->writeLog("Cleared previous data: " + readAll());
                else
                    readAll();
            }

            waitingReplay = false;
            addSpeedSize(currentPendingRequest->size());
            write(*currentPendingRequest);
            flush();
        }
    }
    else if (debugLevel)
        logThread->writeLog("PendingRequest pointer not exist", 2);
}

void JulyHttp::addSpeedSize(qint64 size)
{
    baseValues.trafficSpeed += size;
}

void JulyHttp::sslErrorsSlot(const QList<QSslError>& val)
{
    emit sslErrorSignal(val);
}

bool JulyHttp::requestWait(const QUrl& url, QByteArray& result, QString* errorString)
{
    if (errorString)
        errorString->clear();

    QNetworkAccessManager mng;

    QNetworkRequest request(url);
    request.setRawHeader("Accept", "*/*");
    request.setRawHeader("Accept-Language", "en");
    request.setHeader(QNetworkRequest::UserAgentHeader, QCoreApplication::applicationName());

    QNetworkReply* reply = mng.get(request);

    if (reply == nullptr)
        return false;

    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    result = reply->readAll();

    if (result.isEmpty() && errorString)
        *errorString = reply->errorString();

    reply->deleteLater();

    return !result.isEmpty();
}
