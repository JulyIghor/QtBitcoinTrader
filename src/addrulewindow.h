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
