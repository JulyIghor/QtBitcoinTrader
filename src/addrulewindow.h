// Copyright (C) 2014 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form http://qtopentrader.com
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#ifndef ADDRULEWINDOW_H
#define ADDRULEWINDOW_H

#include <QDialog>
#include "ui_addrulewindow.h"
#include "ruleholder.h"
#include "rulewidget.h"

class AddRuleWindow : public QDialog
{
	Q_OBJECT

public:
	Ui::AddRuleWindow ui;
	AddRuleWindow(RuleWidget *parent = 0);
	~AddRuleWindow();
	RuleHolder getRuleHolder();
	void fillByRuleHolder(RuleHolder *holder);
private:
	bool wasTrailingVisible;
	int getRuleIfType();
	bool changingSound;
	QString sound;
	double getSelectedIfValue();
	void fixWidth();
	RuleWidget *parentRuleGroup;
	int thanType();
	bool checkIsValidRule();
public slots:
	void languageChanged();
public slots:
	void on_playSoundBeep_clicked();
	void on_playSound_clicked();
	void on_valueCheckValuePercentageToZero_clicked();
	void on_checkCheckUsePercentageValue_toggled(bool);
	void on_valueValuePercentage2_3_clicked();
	void on_valueValuePercentage50_clicked();
	void on_valueValuePercentage1_3_clicked();
	void on_exactPrice_toggled(bool);
	void on_valueValuePercentage100_clicked();
	void on_thanValuePercentageToZero_clicked();
	void thanTypeChanged();
	void on_fillFromBuyPanel_clicked();
	void on_fillFromSellPanel_clicked();

	void ifChanged(bool);
	void amountChanged();
	void buttonAddRule();
	void checkToEnableButtons();
};

#endif // ADDRULEWINDOW_H
