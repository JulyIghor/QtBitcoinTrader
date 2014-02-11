// Copyright (C) 2014 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form http://qtopentrader.com
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#ifndef MAIN_H
#define MAIN_H
//#include <QDebug>////////////
//#define showDebug(text) qDebug()<<QTime::currentTime().toString(localTimeFormat)+"> "+text//////////
#include "qtextstream.h"
#include <QFontMetrics>
#include "julytranslator.h"
#include "logthread.h"

#define USE_QTMULTIMEDIA

#define textFontWidth(text) baseValues_->fontMetrics_->width(text)
#define debugLevel (baseValues_->debugLevel_)
#define appDataDir (baseValues_->appDataDir_)
#define grouped (baseValues_->groupPriceValue>0.0?2:0)
#define mainWindow (*baseValues_->mainWindow_)
#define logThread (baseValues_->logThread_)

#define julyTr baseValues_->julyTranslator_.translateString
#define julyTranslator baseValues_->julyTranslator_
#define currentExchange baseValues_->currentExchange_

#define defaultHeightForRow baseValues_->defaultHeightForRow_
#define upArrowStr baseValues_->upArrow
#define downArrowStr baseValues_->downArrow
#define upArrowNoUtfStr baseValues_->upArrowNoUtf8
#define downArrowNoUtfStr baseValues_->downArrowNoUtf8

#include "qtbitcointrader.h"

#define hmacSha512(key, baseString) QByteArray(reinterpret_cast<const char *>(HMAC(EVP_sha512(),key.constData(), key.size(), reinterpret_cast<const unsigned char *>(baseString.constData()), baseString.size(), 0, 0)),64)
#define hmacSha384(key, baseString) QByteArray(reinterpret_cast<const char *>(HMAC(EVP_sha384(),key.constData(), key.size(), reinterpret_cast<const unsigned char *>(baseString.constData()), baseString.size(), 0, 0)),48)
#define hmacSha256(key, baseString) QByteArray(reinterpret_cast<const char *>(HMAC(EVP_sha256(),key.constData(), key.size(), reinterpret_cast<const unsigned char *>(baseString.constData()), baseString.size(), 0, 0)),32)
#define hmacSha1(key, baseString) QByteArray(reinterpret_cast<const char *>(HMAC(EVP_sha1(),key.constData(), key.size(), reinterpret_cast<const unsigned char *>(baseString.constData()), baseString.size(), 0, 0)),20)

#include "currencyinfo.h"
#include "apptheme.h"

class Exchange;

#include "currencypairitem.h"

struct BaseValues
{
	void Construct();

	QString osStyle;
	int lastGroupID;
	bool forceDotInSpinBoxes;

	int trafficSpeed;
	qint64 trafficTotal;
	int trafficTotalType;

	CurrencyPairItem currentPair;

	bool nightMode;
	bool rulesSafeMode;
	int rulesSafeModeInterval;
	
	QByteArray upArrow;
	QByteArray downArrow;
	QByteArray upArrowNoUtf8;
	QByteArray downArrowNoUtf8;

	QString customUserAgent;
	QString customCookies;
	bool gzipEnabled;
	AppTheme appThemeLight;
	AppTheme appThemeDark;
	AppTheme appTheme;
	int debugLevel_;//0: Disabled; 1: Debug; 2: Log
	bool supportsUtfUI;
	bool highResolutionDisplay;
	int defaultHeightForRow_;
	double groupPriceValue;
	QFontMetrics *fontMetrics_;
	int apiDownCount;
	int uiUpdateInterval;
	QByteArray depthCountLimitStr;
	int depthCountLimit;
	int httpRetryCount;
	bool httpSplitPackets;
	int httpRequestInterval;
	int httpRequestTimeout;
	Exchange *currentExchange_;
	QString exchangeName;
	QString timeFormat;
	QString dateTimeFormat;
	QString defaultLangFile;
	JulyTranslator julyTranslator_;
	QByteArray appDataDir_;
	QByteArray appVerStr;
	LogThread *logThread_;
	QByteArray restKey;
	QByteArray restSign;
	QByteArray randomPassword;
	QtBitcoinTrader *mainWindow_;
	QString logFileName;
	QString iniFileName;
	double appVerReal;
	double appVerLastReal;
	bool appVerIsBeta;
	QMap<QByteArray,QByteArray> currencyMapSign;
	QMap<QByteArray,CurencyInfo> currencyMap;
};

#define baseValues (*baseValues_)
extern BaseValues *baseValues_;

#endif // MAIN_H
