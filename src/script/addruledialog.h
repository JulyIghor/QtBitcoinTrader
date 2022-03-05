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

#ifndef ADDRULEDIALOG_H
#define ADDRULEDIALOG_H

#include <QDialog>
#include "ruleholder.h"

namespace Ui
{
class AddRuleDialog;
}
class QComboBox;
class QDoubleSpinBox;

class AddRuleDialog : public QDialog
{
    Q_OBJECT

public:
    bool saveClicked;
    QString getGroupName();
    bool isRuleEnabled() const;
    RuleHolder getRuleHolder();
    void fillByHolder(RuleHolder&, bool enabled);
    explicit AddRuleDialog(const QString& groupName, QWidget* parent = 0);
    ~AddRuleDialog();

private slots:
    void reCacheCode();
    void fixSizeWindow();
    void on_variableA_currentIndexChanged(int index);
    void on_variableB_currentIndexChanged(int index);
    void on_thanAmountPercent_toggled(bool checked);
    void on_thanType_currentIndexChanged(int index);
    void on_playButton_clicked();
    void on_thanTextBrowse_clicked();
    void on_thanPriceType_currentIndexChanged(int index);
    void on_variableBPercent_toggled(bool checked);
    void on_thanPricePercent_toggled(bool checked);
    void on_variableBPercentButton_clicked();
    void on_thanAmountPercentButton_clicked();
    void on_thanPricePercentButton_clicked();
    void on_variableBFee_currentIndexChanged(int index);
    void on_thanAmountFee_currentIndexChanged(int index);
    void on_thanPriceFee_currentIndexChanged(int index);
    void on_thanText_textChanged(const QString& arg1);
    void on_codePreview_toggled(bool checked);
    void on_buttonAddRule_clicked();
    void on_buttonSaveRule_clicked();
    void on_fillFromBuyPanel_clicked();
    void on_fillFromSellPanel_clicked();
    void on_valueBSymbol_currentIndexChanged(int index);
    void on_thanSymbol_currentIndexChanged(int index);
    void on_valueASymbol_currentIndexChanged(int index);

    void on_sayCode_currentIndexChanged(int index);

private:
    QString groupName;
    QString getFreeGroupName();
    void fixSize(bool fitToWindow = false);
    bool pendingFix;
    bool ruleIsEnabled;
    QList<QDoubleSpinBox*> usedSpinBoxes;
    RuleHolder lastHolder;
    QString comboData(QComboBox* list, int row);
    QString comboCurrentData(QComboBox*);
    void setComboIndex(QComboBox* list, int& row);
    void setComboIndex(QComboBox* list, QString& text);
    void setComboIndexByData(QComboBox* list, QString& data);
    QString currentThanType;
    Ui::AddRuleDialog* ui;
};

#endif // ADDRULEDIALOG_H
