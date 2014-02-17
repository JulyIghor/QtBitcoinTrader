//  This file is part of Qt Bitcion Trader
//      https://github.com/JulyIGHOR/QtBitcoinTrader
//  Copyright (C) 2013-2014 July IGHOR <julyighor@gmail.com>
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
	int getRuleMoreLessEqual(){return ruleMoreLessEqualChanged;}
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
	int ruleMoreLessEqualChanged;
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
