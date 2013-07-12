// Copyright (C) 2013 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form https://sourceforge.net/projects/bitcointrader/
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#include "julyrsa.h"

QByteArray JulyRSA::getSignature(QByteArray data, QByteArray keyArray)
{
	BIO *bioKey = BIO_new(BIO_s_mem());
	BIO_puts(bioKey,keyArray.data());

	RSA *rsa=NULL;
	rsa=PEM_read_bio_RSAPublicKey(bioKey,&rsa,NULL,NULL);

	if(rsa==NULL)
	{
		BIO *errBIO = BIO_new(BIO_s_mem());
		ERR_print_errors(errBIO);
			char *errData;
			long errSize=BIO_get_mem_data(errBIO, &errData);
		QByteArray errorString(errData,errSize);
		BIO_free(errBIO);
		BIO_free(bioKey);
		RSA_free(rsa);
		return QByteArray();
	}
	int rsaSize=RSA_size(rsa);
	int dataLimit=rsaSize;

	QList<QByteArray> dataList;
	int curDataPos=0;
	while(curDataPos<data.size())
	{
		int endPos=curDataPos+dataLimit-1;
		if(endPos>=data.size())endPos=data.size()-1;
		if(curDataPos<=endPos)
			dataList<<data.mid(curDataPos,endPos-curDataPos+1);
		else break;
		curDataPos=endPos+1;
	}
	QByteArray result;
	for(int n=0;n<dataList.count();n++)
	{
	unsigned char *finalData=(unsigned char *)malloc(rsaSize);
	int outSize=RSA_public_decrypt(dataList.at(n).size(), (unsigned char*)dataList.at(n).constData(), finalData, rsa, RSA_PKCS1_PADDING);
	result.append(QByteArray((char *)finalData,outSize));
	free(finalData);
	}
	BIO_free(bioKey);
	RSA_free(rsa);
	return result;
}