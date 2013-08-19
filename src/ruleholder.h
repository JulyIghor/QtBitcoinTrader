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
	void setEnabled(bool on){enabled=on;}
	bool isEnabled(){return enabled;}
	bool invalidHolder;
	RuleHolder(){invalidHolder=true;enabled=true;}
	RuleHolder(int moreLessEqual, double price, double bitcoins, uint guid, bool isBuy, double sellPrice, int rulePriceType);
	bool isAchieved(double price);
	bool isBuying();
	int getRuleMoreLessEqual(){return ruleMoreLessEqual;}
	double getRuleBtc(){return ruleBtc;}
	double getRulePrice(){return rulePrice;}
	double getRuleCheckPrice(){return ruleCheckPrice;}
	uint getRuleGuid(){return ruleGuid;}
	void startWaitingLowLag();
	QString getDescriptionString();
	int getRulePriceType(){return rulePriceType;}
	QString getSellOrBuyString();
	QString getBitcoinsString();
	QString getPriceText();
private:
	bool enabled;
	int rulePriceType;
	bool waitingGoodLag;
	bool buying;
	double ruleBtc;
	uint ruleGuid;
	int ruleMoreLessEqual;
	double rulePrice;
	double ruleCheckPrice;
};

#endif // RULEHOLDER_H
