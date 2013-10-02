// Copyright (C) 2013 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form https://sourceforge.net/projects/bitcointrader/
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#ifndef RULESMODEL_H
#define RULESMODEL_H

#include <QAbstractItemModel>
#include "ruleholder.h"
#include <QStringList>

class RulesModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	QString saveRulesToString();
	void restoreRulesFromString(QString);
	bool haveWorkingRule();
	bool allDisabled;
	bool isConcurrentMode;
	RulesModel();
	~RulesModel();

	void disableAll();
	void enableAll();

	QList<RuleHolder *> getAchievedRules(int type, double value);
	RuleHolder *getRuleHolderByRow(int row);
	void updateHolderByRow(int row, RuleHolder *holder);
	void moveRowUp(int row);
	void moveRowDown(int row);

	void setRuleStateByHolder(RuleHolder *holder, int state);
	void setRuleStateByRow(int row, int state);
	void clear();
	void addRule(RuleHolder *holder);
	void removeRuleByRow(int row);

	void setHorizontalHeaderLabels(QStringList list);

	QModelIndex index(int row, int column, const QModelIndex &parent=QModelIndex()) const;
	QModelIndex parent(const QModelIndex &index) const;

	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	Qt::ItemFlags flags(const QModelIndex &index) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;
private:
	int stateWidth;
	RuleHolder *firstQueringHolder;
	RuleHolder *lastQueringHolder;
	QStringList headerLabels;
	int columnsCount;
	QList<RuleHolder*> holderList;
};

#endif // RULESMODEL_H
