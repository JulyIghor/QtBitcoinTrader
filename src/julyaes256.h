//Created by July IGHOR
//Feel free to contact me: julyighor@gmail.com
//Bitcoin Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc

#ifndef JULYAES256_H
#define JULYAES256_H

#include <QByteArray>

class JulyAES256
{
public:
	static QByteArray sha256(const QByteArray &text);
	static QByteArray decrypt(const QByteArray &data, const QByteArray &password);
	static QByteArray encrypt(const QByteArray &data, const QByteArray &password);
};

#endif // JULYAES256_H
