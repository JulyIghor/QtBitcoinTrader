//  This file is part of Qt Bitcion Trader
//      https://github.com/JulyIGHOR/QtBitcoinTrader
//  Copyright (C) 2013-2015 July IGHOR <julyighor@gmail.com>
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