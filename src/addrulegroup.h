// Copyright (C) 2013 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form https://sourceforge.net/projects/bitcointrader/
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#ifndef ADDRULEGROUP_H
#define ADDRULEGROUP_H

#include <QDialog>
#include "ui_addrulegroup.h"

class AddRuleGroup : public QDialog
{
	Q_OBJECT

public:
	QString groupName;
	QString copyFromExistingGroup;
	AddRuleGroup(QWidget *parent = 0);
	~AddRuleGroup();

private:
	QStringList existingGroups;
	Ui::AddRuleGroup ui;

private slots:
	void onGroupContentChanged(bool);
	void on_groupName_textChanged(QString);
	void on_buttonAddRule_clicked();
};

#endif // ADDRULEGROUP_H
