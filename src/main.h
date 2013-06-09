// Copyright (C) 2013 July IGHOR.
// I want to create Bitcoin Trader application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form https://sourceforge.net/projects/bitcointrader/
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#ifndef MAIN_H
#define MAIN_H

#include "qtbitcointrader.h"
#include "logthread.h"
#include "julytranslator.h"

//#define GENERATE_LANGUAGE_FILE//Used for make default language file

#define julyTr julyTranslator->translateString

#define hostName QByteArray("data.mtgox.com")
#define apiId QByteArray("2")
#define restKey (*restKey_)
#define restSign (*restSign_)
#define mainWindow (*mainWindow_)
#define nonce (*nonce_)
#define isLogEnabled (*logEnabled_)
#define iniFileName (*iniFileName_)
#define logFileName (*logFileName_)
#define appVerReal (*appVerReal_)
#define appVerStr (*appVerStr_)
#define useSSL (*useSSL_)
#define currencyStr (*currencyStr_)
#define currencySign (*currencySign_)
#define bitcoinSign (*bitcoinSign_)
#define appDataDir (*appDataDir_)
#define defaultLangFile (*defaultLangFile_)

extern QString *defaultLangFile_;
extern JulyTranslator *julyTranslator;
extern QByteArray *appDataDir_;
extern QByteArray *bitcoinSign_;
extern QByteArray *currencyStr_;
extern QByteArray *currencySign_;
extern bool *useSSL_;
extern QByteArray *appVerStr_;
extern LogThread *logThread;
extern QByteArray *restKey_;
extern QByteArray *restSign_;
extern QtBitcoinTrader *mainWindow_;
extern quint64 *nonce_;
extern bool *logEnabled_;
extern QString *logFileName_;
extern QString *iniFileName_;
extern double *appVerReal_;
extern QMap<QByteArray,QByteArray> *currencySignMap;
extern QMap<QByteArray,QByteArray> *currencyNamesMap;
//
//quint64 getNextNonce()
//{
//	quint64 nextNonce=QDateTime::currentDateTime().toMSecsSinceEpoch()*1000000;
//	if(nonce>=nextNonce-1000000)nextNonce+=1000000;
//	nonce=nextNonce;
//	return nextNonce;
//}
#endif // MAIN_H
