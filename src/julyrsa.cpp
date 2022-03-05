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

#include "julyrsa.h"
#include <QCryptographicHash>
#include <QFile>

QByteArray JulyRSA::getSignature(const QByteArray& data, const QByteArray& keyArray)
{
    BIO* bioKey = BIO_new(BIO_s_mem());
    BIO_puts(bioKey, keyArray.data());

    RSA* rsa = nullptr;
    rsa = PEM_read_bio_RSAPublicKey(bioKey, &rsa, nullptr, nullptr);

    if (rsa == nullptr)
    {
        BIO* errBIO = BIO_new(BIO_s_mem());
        ERR_print_errors(errBIO);
        // char* errData;
        // long errSize = BIO_get_mem_data(errBIO, &errData);
        //  QByteArray errorString(errData, static_cast<int>(errSize));
        BIO_free(errBIO);
        BIO_free(bioKey);
        RSA_free(rsa);
        return QByteArray();
    }

    int rsaSize = RSA_size(rsa);
    int dataLimit = rsaSize;

    QList<QByteArray> dataList;
    int curDataPos = 0;

    while (curDataPos < data.size())
    {
        int endPos = curDataPos + dataLimit - 1;

        if (endPos >= data.size())
            endPos = data.size() - 1;

        if (curDataPos <= endPos)
            dataList << data.mid(curDataPos, endPos - curDataPos + 1);
        else
            break;

        curDataPos = endPos + 1;
    }

    QByteArray result;

    for (int n = 0; n < dataList.size(); n++)
    {
        auto* finalData = static_cast<unsigned char*>(malloc(static_cast<size_t>(rsaSize)));
        int outSize =
            RSA_public_decrypt(dataList.at(n).size(), (unsigned char*)dataList.at(n).constData(), finalData, rsa, RSA_PKCS1_PADDING);
        result.append(QByteArray((char*)finalData, outSize));
        free(finalData);
    }

    BIO_free(bioKey);
    RSA_free(rsa);
    return result;
}

bool JulyRSA::isIniFileSigned(const QString& fileName)
{
    QList<QByteArray> fileDataList;
    {
        QFile fr(fileName);

        if (!fr.open(QFile::ReadOnly | QFile::Text))
            return false;

        fileDataList = fr.readAll().split('\n');
        fr.close();
    }

    QByteArray existingHash;
    int signGroupPos = -1;
    QByteArray signData;

    for (int n = fileDataList.size() - 1; n >= 0; n--)
    {
        if (fileDataList.at(n).startsWith("[RSA2048Sign]"))
        {
            signGroupPos = n;
            fileDataList.removeAt(n);
            continue;
        }

        if (fileDataList.at(n).startsWith("SignHash="))
        {
            existingHash = fileDataList.at(n);
            existingHash.remove(0, 9);
            fileDataList.removeAt(n);
            continue;
        }

        signData += fileDataList.at(n);
    }

    if (signGroupPos == -1)
        existingHash.clear();

    QFile readPublicKey(":/Resources/Public.key");

    if (!readPublicKey.open(QIODevice::ReadOnly))
        return false;

    QByteArray publicKey = readPublicKey.readAll();
    readPublicKey.close();

    return QCryptographicHash::hash(fileDataList.join('\n').trimmed(), QCryptographicHash::Sha1) ==
           JulyRSA::getSignature(QByteArray::fromBase64(existingHash), publicKey);
}
