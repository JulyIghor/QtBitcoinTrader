// Copyright (C) 2014 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form http://qtopentrader.com
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
	double getCurrentCheckPrice();
	double getCurrentExecPrice();
	bool invalidHolder;
	RuleHolder(){invalidHolder=true;ruleState=0;waitingGoodLag=false;}
	RuleHolder(int moreLessEqual, double price, double bitcoins, bool isBuy, double sellPrice, int rulePriceType, double rulekPricePercentage, double rulePricePercentage, double ruleAmountPercentage, int ruleGroupId, QString sound, bool trailingStop, bool enabled=true);
	RuleHolder(QString strData);
	bool isAchieved(double price);
	bool isBuying();
	bool isTrading();
	int getRuleMoreLessEqual(){return ruleMoreLessEqualThreshold;}
	double getRuleBtc(){return ruleAmount;}
	double getRuleExecutePrice(){return ruleExecutePrice;}
	double getRuleCheckPrice(){return ruleCheckPrice;}
	void startWaitingLowLag();
	QString getDescriptionString();
	int getRulePriceType(){return rulePriceType;}
	QString getSellOrBuyString();
	QString getBitcoinsString();
	QString getPriceText();
	bool isTrailingEnabled(){return trailingEnabled;}
	int getRuleState();
	void setRuleState(int);
	void setRuleWavFile(QString wav){ruleWavFile=wav;}
	QString getRuleWavFile(){return ruleWavFile;}
	void setRuleGroupId(int id);
	int getRuleGroupId(){return ruleGroupId;}
	double getRuleAmountPercentage(){return ruleAmountPercentage;}
	double getRuleExecutePricePercentage(){return ruleExecutePricePercentage;}
	double getRuleCheckPricePercentage(){return ruleCheckPricePercentage;}
	QString generateSavableData();
	QString ruleWavFile;
	
private:
	bool trailingEnabled;
	void saveRuleCheckPrice();
	int ruleState;
	int rulePriceType;
	bool waitingGoodLag;
	bool buying;
	int ruleMoreLessEqualThreshold;
	double ruleAmount;
	double ruleAmountPercentage;
	double ruleExecutePrice;
	double ruleExecutePricePercentage;
	double ruleCheckPrice;
	double lastRuleCheckPrice;
	double ruleCheckPricePercentage;
	int ruleGroupId;
};

#endif // RULEHOLDER_H
