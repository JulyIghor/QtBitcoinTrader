// Copyright (C) 2013 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form https://sourceforge.net/projects/bitcointrader/
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

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
