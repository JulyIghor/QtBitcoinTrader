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

#include "julyaes256.h"
#include <openssl/aes.h>
#include <openssl/evp.h>

QByteArray JulyAES256::sha256(const QByteArray& text)
{
    unsigned int outLen = 0;
    QByteArray dataBuff;
    dataBuff.resize(EVP_MAX_MD_SIZE);
#if OPENSSL_VERSION_NUMBER < 0x10100000L
    EVP_MD_CTX* evpMdCtx = EVP_MD_CTX_create();
#else
    EVP_MD_CTX* evpMdCtx = EVP_MD_CTX_new();
#endif
    EVP_DigestInit(evpMdCtx, EVP_sha256());
    EVP_DigestUpdate(evpMdCtx, text.data(), static_cast<size_t>(text.size()));
    EVP_DigestFinal_ex(evpMdCtx, reinterpret_cast<unsigned char*>(dataBuff.data()), &outLen);
#if OPENSSL_VERSION_NUMBER < 0x10100000L
    EVP_MD_CTX_cleanup(evpMdCtx);
    OPENSSL_free(evpMdCtx);
#else
    EVP_MD_CTX_free(evpMdCtx);
#endif
    dataBuff.resize(static_cast<int>(outLen));
    return dataBuff.toHex();
}

QByteArray JulyAES256::encrypt(const QByteArray& data, const QByteArray& password)
{
    int outLen = 0;
    QByteArray dataBuff;
    dataBuff.resize(data.size() + AES_BLOCK_SIZE);
    EVP_CIPHER_CTX* evpCipherCtx = EVP_CIPHER_CTX_new();
    EVP_CIPHER_CTX_init(evpCipherCtx);
    EVP_EncryptInit(evpCipherCtx,
                    EVP_aes_256_cbc(),
                    reinterpret_cast<const unsigned char*>(sha256(password).data()),
                    reinterpret_cast<const unsigned char*>(sha256("JulyAES" + password).data()));
    EVP_EncryptUpdate(evpCipherCtx,
                      reinterpret_cast<unsigned char*>(dataBuff.data()),
                      &outLen,
                      reinterpret_cast<const unsigned char*>(data.data()),
                      data.size());
    int tempLen = outLen;
    EVP_EncryptFinal(evpCipherCtx, reinterpret_cast<unsigned char*>(dataBuff.data()) + tempLen, &outLen);
    tempLen += outLen;
    EVP_CIPHER_CTX_cleanup(evpCipherCtx);
    EVP_CIPHER_CTX_free(evpCipherCtx);
    dataBuff.resize(tempLen);
    return dataBuff;
}

QByteArray JulyAES256::decrypt(const QByteArray& data, const QByteArray& password)
{
    int outLen = 0;
    QByteArray dataBuff;
    dataBuff.resize(data.size() + AES_BLOCK_SIZE);
    EVP_CIPHER_CTX* evpCipherCtx = EVP_CIPHER_CTX_new();
    EVP_CIPHER_CTX_init(evpCipherCtx);
    EVP_DecryptInit(evpCipherCtx,
                    EVP_aes_256_cbc(),
                    reinterpret_cast<const unsigned char*>(sha256(password).data()),
                    reinterpret_cast<const unsigned char*>(sha256("JulyAES" + password).data()));
    EVP_DecryptUpdate(evpCipherCtx,
                      reinterpret_cast<unsigned char*>(dataBuff.data()),
                      &outLen,
                      reinterpret_cast<const unsigned char*>(data.data()),
                      data.size());
    int tempLen = outLen;
    EVP_DecryptFinal(evpCipherCtx, reinterpret_cast<unsigned char*>(dataBuff.data()) + tempLen, &outLen);
    tempLen += outLen;
    EVP_CIPHER_CTX_cleanup(evpCipherCtx);
    EVP_CIPHER_CTX_free(evpCipherCtx);
    dataBuff.resize(tempLen);
    return dataBuff;
}
