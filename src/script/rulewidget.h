//  This file is part of Qt Bitcoin Trader
//      https://github.com/JulyIGHOR/QtBitcoinTrader
//  Copyright (C) 2013-2022 July Ighor <julyighor@gmail.com>
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

#ifndef RULEWIDGET_H
#define RULEWIDGET_H

#include "rulesmodel.h"
#include "ui_rulewidget.h"
#include <QTime>
#include <QWidget>

class RuleWidget : public QWidget
{
    Q_OBJECT

public:
    void addRuleByHolder(RuleHolder& holder, bool isEnabled);
    bool isBeepOnDone();
    void currencyChanged() const;
    void updateStyleSheets();
    void saveRulesData();
    bool haveWorkingRules() const;
    bool haveAnyRules() const;
    bool removeGroup();
    void languageChanged();
    Ui::RuleWidget ui;
    RulesModel* rulesModel;
    explicit RuleWidget(const QString& filePath);
    ~RuleWidget();

private:
    bool agreeRuleImmediately(QString);
    QTime ordersCancelTime;
    QMenu* rulesEnableDisableMenu;
    QString groupName;
    QString filePath;
private slots:
    void ruleEnableAllSlot();
public slots:
    void writeLog(QString);
    void on_limitRowsValue_valueChanged(int);
    void on_buttonSave_clicked();
    void ruleDone();
    void on_ruleUp_clicked();
    void on_ruleDown_clicked();
    void rulesMenuRequested(const QPoint&);
    void ruleDisableEnableMenuFix();
    void on_ruleConcurrentMode_toggled(bool) const;
    void ruleEnableSelected();
    void ruleDisableSelected();
    void ruleEnableAll();
    void ruleDisableAll();
    void on_ruleAddButton_clicked();
    void on_ruleEditButton_clicked();
    void on_ruleRemoveAll_clicked();
    void checkValidRulesButtons();
    void on_ruleRemove_clicked();
    void on_ruleSave_clicked();
};

#endif // RULEWIDGET_H
