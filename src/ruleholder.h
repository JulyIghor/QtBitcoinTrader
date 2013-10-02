// Copyright (C) 2013 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form https://sourceforge.net/projects/bitcointrader/
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#ifndef RULEHOLDER_H
#define RULEHOLDER_H

#include <QObject>

class RuleHolder
{
public:
	bool invalidHolder;
	RuleHolder(){invalidHolder=true;ruleState=0;waitingGoodLag=false;}
	RuleHolder(int moreLessEqual, double price, double bitcoins, bool isBuy, double sellPrice, int rulePriceType, bool enabled=true);
	RuleHolder(QString strData);
	bool isAchieved(double price);
	bool isBuying();
	int getRuleMoreLessEqual(){return ruleMoreLessEqual;}
	double getRuleBtc(){return ruleBtc;}
	double getRulePrice(){return rulePrice;}
	double getRuleCheckPrice(){return ruleCheckPrice;}
	void startWaitingLowLag();
	QString getDescriptionString();
	int getRulePriceType(){return rulePriceType;}
	QString getSellOrBuyString();
	QString getBitcoinsString();
	QString getPriceText();
	int getRuleState();
	void setRuleState(int);

	QString generateSavableData();
	
private:
	int ruleState;
	int rulePriceType;
	bool waitingGoodLag;
	bool buying;
	double ruleBtc;
	int ruleMoreLessEqual;
	double rulePrice;
	double ruleCheckPrice;
};

#endif // RULEHOLDER_H
