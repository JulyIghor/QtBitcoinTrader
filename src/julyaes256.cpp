// Copyright (C) 2013 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form https://sourceforge.net/projects/bitcointrader/
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#include "julyaes256.h"
#include <openssl/evp.h>
#include <openssl/aes.h>

QByteArray JulyAES256::sha256(const QByteArray &text)
{
	unsigned int outLen=0;
	QByteArray dataBuff; dataBuff.resize(EVP_MAX_MD_SIZE);
	EVP_MD_CTX evpMdCtx;
	EVP_DigestInit(&evpMdCtx, EVP_sha256());
	EVP_DigestUpdate(&evpMdCtx, text.data(), text.size());
	EVP_DigestFinal_ex(&evpMdCtx, (unsigned char *)dataBuff.data(), &outLen);
	EVP_MD_CTX_cleanup(&evpMdCtx);
	dataBuff.resize(outLen);
	return dataBuff.toHex();
}

QByteArray JulyAES256::encrypt(const QByteArray &data, const QByteArray &password)
{
	int outLen=0;
	QByteArray dataBuff;dataBuff.resize(data.size()+AES_BLOCK_SIZE);
	EVP_CIPHER_CTX evpCipherCtx;
	EVP_CIPHER_CTX_init(&evpCipherCtx);
	EVP_EncryptInit(&evpCipherCtx,EVP_aes_256_cbc(),(const unsigned char*)sha256(password).data(),(const unsigned char*)sha256("JulyAES"+password).data());
	EVP_EncryptUpdate(&evpCipherCtx,(unsigned char*)dataBuff.data(),&outLen,(const unsigned char*)data.data(),data.size());
	int tempLen=outLen;
	EVP_EncryptFinal(&evpCipherCtx,(unsigned char*)dataBuff.data()+tempLen,&outLen);
	tempLen+=outLen;
	EVP_CIPHER_CTX_cleanup(&evpCipherCtx);
	dataBuff.resize(tempLen);
	return dataBuff;
}

QByteArray JulyAES256::decrypt(const QByteArray &data, const QByteArray &password)
{
	int outLen=0;
	QByteArray dataBuff;dataBuff.resize(data.size()+AES_BLOCK_SIZE);
	EVP_CIPHER_CTX evpCipherCtx;
	EVP_CIPHER_CTX_init(&evpCipherCtx);
	EVP_DecryptInit(&evpCipherCtx,EVP_aes_256_cbc(),(const unsigned char*)sha256(password).data(),(const unsigned char*)sha256("JulyAES"+password).data());
	EVP_DecryptUpdate(&evpCipherCtx,(unsigned char*)dataBuff.data(),&outLen,(const unsigned char*)data.data(),data.size());
	int tempLen=outLen;
	EVP_DecryptFinal(&evpCipherCtx,(unsigned char*)dataBuff.data()+tempLen,&outLen);
	tempLen+=outLen;
	EVP_CIPHER_CTX_cleanup(&evpCipherCtx);
	dataBuff.resize(tempLen);
	return dataBuff;
}