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

#include "rulewidget.h"
#include "addruledialog.h"
#include "exchange/exchange.h"
#include "main.h"
#include "rulescriptparser.h"
#include "timesync.h"
#include "utils/utils.h"
#include <QDesktopServices>
#include <QFileDialog>
#include <QMessageBox>
#include <QtCore/qmath.h>

RuleWidget::RuleWidget(const QString& fileName) : QWidget()
{
    ui.setupUi(this);
    ui.rulesTabs->setCurrentIndex(0);

    filePath = fileName;

    setProperty("FileName", filePath);
    setProperty("GroupType", "Rule");

    QSettings loadRule(fileName, QSettings::IniFormat);
    loadRule.beginGroup("JLRuleGroup");
    groupName = loadRule.value("Name", "Unknown").toString();
    rulesModel = new RulesModel(groupName);
    ui.limitRowsValue->setValue(loadRule.value("LogRowsCount", 20).toInt());
    ui.ruleBeep->setChecked(loadRule.value("BeepOnDone", false).toBool());
    ui.notes->setPlainText(loadRule.value("Notes", "").toString());
    ui.ruleConcurrentMode->setChecked(loadRule.value("ConcurrentMode", false).toBool());
    loadRule.endGroup();

    ordersCancelTime = QTime(1, 0, 0, 0);
    setAttribute(Qt::WA_DeleteOnClose, true);

    updateStyleSheets();

    ui.rulesNoMessage->setVisible(true);
    ui.rulesTabs->setVisible(false);

    connect(rulesModel, &RulesModel::writeLog, this, &RuleWidget::writeLog);
    rulesModel->setParent(this);
    ui.rulesTable->setModel(rulesModel);
    mainWindow.setColumnResizeMode(ui.rulesTable, 0, QHeaderView::ResizeToContents);
    mainWindow.setColumnResizeMode(ui.rulesTable, 1, QHeaderView::Stretch);

    connect(rulesModel, &RulesModel::ruleDone, this, &RuleWidget::ruleDone);

    connect(ui.rulesTable->selectionModel(), &QItemSelectionModel::selectionChanged, this, &RuleWidget::checkValidRulesButtons);
    ui.rulesTable->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui.rulesTable, &QTableView::customContextMenuRequested, this, &RuleWidget::rulesMenuRequested);

    rulesEnableDisableMenu = new QMenu;
    rulesEnableDisableMenu->addAction("Enable Selected");
    connect(rulesEnableDisableMenu->actions().last(), &QAction::triggered, this, &RuleWidget::ruleEnableSelected);
    rulesEnableDisableMenu->addAction("Disable Selected");
    connect(rulesEnableDisableMenu->actions().last(), &QAction::triggered, this, &RuleWidget::ruleDisableSelected);
    rulesEnableDisableMenu->addSeparator();
    rulesEnableDisableMenu->addAction("Enable All");
    connect(rulesEnableDisableMenu->actions().last(), &QAction::triggered, this, &RuleWidget::ruleEnableAll);
    rulesEnableDisableMenu->addAction("Disable All");
    connect(rulesEnableDisableMenu->actions().last(), &QAction::triggered, this, &RuleWidget::ruleDisableAll);
    ui.ruleEnableDisable->setMenu(rulesEnableDisableMenu);
    connect(rulesEnableDisableMenu, &QMenu::aboutToShow, this, &RuleWidget::ruleDisableEnableMenuFix);

    languageChanged();

    setWindowTitle(groupName);

    for (const QString& group : loadRule.childGroups())
    {
        if (!group.startsWith("Rule_"))
            continue;

        RuleHolder holder = RuleScriptParser::readHolderFromSettings(loadRule, group);

        if (holder.isValid())
            rulesModel->addRule(holder);
    }

    saveRulesData();

    checkValidRulesButtons();

    mainWindow.fixTableViews(this);

    QSettings iniSettings(baseValues.iniFileName, QSettings::IniFormat, this);

    if (iniSettings.value("UI/OptimizeInterface", false).toBool())
        recursiveUpdateLayouts(this);
}

RuleWidget::~RuleWidget()
{
    if (!filePath.isEmpty())
        saveRulesData();
}

void RuleWidget::writeLog(QString text)
{
    text.replace("\\n", "<br>");
    text.replace("\\t", "    ");
    text.prepend(QDateTime::fromSecsSinceEpoch(TimeSync::getTimeT()).time().toString(baseValues.timeFormat) + "> ");
    ui.consoleOutput->appendHtml(text);
}

void RuleWidget::on_buttonSave_clicked()
{
    saveRulesData();
}

bool RuleWidget::isBeepOnDone()
{
    return ui.ruleBeep->isChecked();
}

void RuleWidget::ruleDone()
{
    if (isBeepOnDone())
        mainWindow.beep();
}

void RuleWidget::updateStyleSheets()
{
    ui.rulesNoMessage->setStyleSheet("font-size:27px; border: 1px solid gray; background: " + baseValues.appTheme.white.name() +
                                     "; color: " + baseValues.appTheme.gray.name());
}

bool RuleWidget::removeGroup()
{
    bool removed = true;

    if (!filePath.isEmpty())
    {
        QFile::remove(filePath);
        removed = !QFile::exists(filePath);
    };

    filePath.clear();

    return removed;
}

void RuleWidget::languageChanged()
{
    julyTranslator.translateUi(this);

    ui.rulesTabs->setTabText(0, julyTr("TAB_RULES_FOR_ORDERS", "Rules"));
    ui.rulesTabs->setTabText(1, julyTr("CONSOLE_OUT", "Console output"));
    ui.rulesTabs->setTabText(2, julyTr("SCRIPT_NOTES", "Notes"));

    rulesModel->setHorizontalHeaderLabels(QStringList() << julyTr("RULES_T_STATE", "State") << julyTr("RULES_T_DESCR", "Description"));
    // Removed <<julyTr("RULES_T_ACTION","Action")<<julyTr("ORDERS_AMOUNT","Amount")<<julyTr("RULES_T_PRICE","Price"));

    rulesEnableDisableMenu->actions().at(0)->setText(julyTr("RULE_ENABLE", "Enable Selected"));
    rulesEnableDisableMenu->actions().at(1)->setText(julyTr("RULE_DISABLE", "Disable Selected"));
    rulesEnableDisableMenu->actions().at(3)->setText(julyTr("RULE_ENABLE_ALL", "Enable All"));
    rulesEnableDisableMenu->actions().at(4)->setText(julyTr("RULE_DISABLE_ALL", "Disable All"));

    mainWindow.fixAllChildButtonsAndLabels(this);
}

void RuleWidget::on_limitRowsValue_valueChanged(int val)
{
    ui.consoleOutput->setMaximumBlockCount(val);
    QSettings(filePath, QSettings::IniFormat).setValue("JLRuleGroup/LogRowsCount", val);
}

void RuleWidget::saveRulesData()
{
    ui.saveFon->setVisible(false);

    if (QFile::exists(filePath))
        QFile::remove(filePath);

    QSettings saveScript(filePath, QSettings::IniFormat);
    saveScript.beginGroup("JLRuleGroup");
    saveScript.setValue("Version", baseValues.jlScriptVersion);
    saveScript.setValue("Name", groupName);
    saveScript.setValue("LogRowsCount", ui.limitRowsValue->value());
    saveScript.setValue("Notes", ui.notes->toPlainText());
    saveScript.setValue("BeepOnDone", ui.ruleBeep->isChecked());
    saveScript.setValue("ConcurrentMode", ui.ruleConcurrentMode->isChecked());
    saveScript.endGroup();

    for (int n = 0; n < rulesModel->holderList.size(); n++)
        RuleScriptParser::writeHolderToSettings(rulesModel->holderList[n], saveScript, "Rule_" + QString::number(n + 101));

    saveScript.sync();
}

void RuleWidget::addRuleByHolder(RuleHolder& holder, bool isEnabled)
{
    rulesModel->addRule(holder, isEnabled);

    ui.rulesNoMessage->setVisible(false);
    ui.rulesTabs->setVisible(true);
    checkValidRulesButtons();
    saveRulesData();
}

void RuleWidget::on_ruleAddButton_clicked()
{
    AddRuleDialog ruleWindow(groupName, this);
    ruleWindow.setWindowFlags(mainWindow.windowFlags());

    if (ruleWindow.exec() == QDialog::Rejected)
        return;

    RuleHolder holder = ruleWindow.getRuleHolder();

    if (!holder.isValid())
        return;

    mainWindow.addRuleByHolder(holder, ruleWindow.isRuleEnabled(), ruleWindow.getGroupName());
}

void RuleWidget::on_ruleConcurrentMode_toggled(bool on) const
{
    rulesModel->isConcurrentMode = on;
}

void RuleWidget::on_ruleEditButton_clicked()
{
    QModelIndexList selectedRows = ui.rulesTable->selectionModel()->selectedRows();

    if (selectedRows.empty())
        return;

    int curRow = selectedRows.first().row();

    if (curRow < 0)
        return;

    AddRuleDialog ruleWindow(groupName, baseValues.mainWindow_);
    ruleWindow.setWindowFlags(mainWindow.windowFlags());
    ruleWindow.fillByHolder(rulesModel->holderList[curRow], rulesModel->getStateByRow(curRow) == 1);

    if (ruleWindow.exec() == QDialog::Rejected)
        return;

    RuleHolder holder = ruleWindow.getRuleHolder();

    if (!holder.isValid())
        return;

    if (ruleWindow.saveClicked)
    {
        rulesModel->setRuleStateByRow(curRow, 0);
        rulesModel->updateRule(curRow, holder, ruleWindow.isRuleEnabled());

        checkValidRulesButtons();
        saveRulesData();
    }
    else
        mainWindow.addRuleByHolder(holder, ruleWindow.isRuleEnabled(), ruleWindow.getGroupName());
}

void RuleWidget::on_ruleRemoveAll_clicked()
{
    QMessageBox msgBox(baseValues.mainWindow_);
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setWindowTitle(julyTr("APPLICATION_TITLE", windowTitle()));
    msgBox.setText(julyTr("RULE_CONFIRM_REMOVE_ALL", "Are you sure to remove all rules?"));

    auto buttonYes = msgBox.addButton(julyTr("YES", "Yes"), QMessageBox::YesRole);
    msgBox.addButton(julyTr("NO", "No"), QMessageBox::NoRole);
    msgBox.exec();
    if (msgBox.clickedButton() != buttonYes)
        return;

    rulesModel->clear();
    checkValidRulesButtons();
    saveRulesData();
}

void RuleWidget::on_ruleRemove_clicked()
{
    QMessageBox msgBox(baseValues.mainWindow_);
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setWindowTitle(julyTr("APPLICATION_TITLE", windowTitle()));
    msgBox.setText(julyTr("RULE_CONFIRM_REMOVE", "Are you sure to remove this rule?"));

    auto buttonYes = msgBox.addButton(julyTr("YES", "Yes"), QMessageBox::YesRole);
    msgBox.addButton(julyTr("NO", "No"), QMessageBox::NoRole);
    msgBox.exec();
    if (msgBox.clickedButton() != buttonYes)
        return;

    QModelIndexList selectedRows = ui.rulesTable->selectionModel()->selectedRows();

    if (selectedRows.empty())
        return;

    int curRow = selectedRows.first().row();
    rulesModel->removeRuleByRow(curRow);
    checkValidRulesButtons();
    saveRulesData();
}

void RuleWidget::rulesMenuRequested(const QPoint& point)
{
    rulesEnableDisableMenu->exec(ui.rulesTable->viewport()->mapToGlobal(point));
}

void RuleWidget::ruleDisableEnableMenuFix()
{
    bool haveRules_ = rulesModel->rowCount() > 0;
    QModelIndexList selectedRows = ui.rulesTable->selectionModel()->selectedRows();

    int selectedRulesCount = selectedRows.size();
    bool ifSelectedOneRuleIsItEnabled = selectedRulesCount == 1;

    if (ifSelectedOneRuleIsItEnabled)
    {
        int state = rulesModel->getStateByRow(selectedRows.first().row());
        ifSelectedOneRuleIsItEnabled = state == 1 || state == 2;
        rulesEnableDisableMenu->actions().at(0)->setEnabled(!ifSelectedOneRuleIsItEnabled || state == 3);
        rulesEnableDisableMenu->actions().at(1)->setEnabled(ifSelectedOneRuleIsItEnabled || state == 3);
    }
    else
    {
        rulesEnableDisableMenu->actions().at(0)->setEnabled(selectedRulesCount > 0);
        rulesEnableDisableMenu->actions().at(1)->setEnabled(selectedRulesCount > 0);
    }

    rulesEnableDisableMenu->actions().at(2)->setEnabled(haveRules_);
    rulesEnableDisableMenu->actions().at(3)->setEnabled(haveRules_);
}

bool RuleWidget::haveWorkingRules() const
{
    return rulesModel->haveWorkingRule();
}

bool RuleWidget::haveAnyRules() const
{
    return rulesModel->rowCount() > 0;
}

void RuleWidget::currencyChanged() const
{
    if (baseValues.currentExchange_->multiCurrencyTradeSupport)
        return;

    rulesModel->currencyChanged();
}

void RuleWidget::checkValidRulesButtons()
{
    int selectedCount = ui.rulesTable->selectionModel()->selectedRows().size();
    ui.ruleEditButton->setEnabled(selectedCount == 1);
    ui.ruleRemove->setEnabled(selectedCount);
    rulesEnableDisableMenu->actions().at(0)->setEnabled(selectedCount);
    rulesEnableDisableMenu->actions().at(1)->setEnabled(selectedCount);
    ui.ruleEnableDisable->setEnabled(rulesModel->rowCount());
    ui.ruleRemoveAll->setEnabled(rulesModel->rowCount());
    ui.ruleConcurrentMode->setEnabled(rulesModel->rowCount() == 0 || !rulesModel->haveWorkingRule());
    ui.ruleSequencialMode->setEnabled(ui.ruleConcurrentMode->isEnabled());

    ui.rulesNoMessage->setVisible(rulesModel->rowCount() == 0);
    ui.rulesTabs->setVisible(rulesModel->rowCount());

    ui.ruleUp->setEnabled(ui.ruleEditButton->isEnabled() && rulesModel->rowCount() > 1);
    ui.ruleDown->setEnabled(ui.ruleEditButton->isEnabled() && rulesModel->rowCount() > 1);
}

void RuleWidget::on_ruleUp_clicked()
{
    QModelIndexList selectedRows = ui.rulesTable->selectionModel()->selectedRows();

    if (selectedRows.empty())
        return;

    int curRow = selectedRows.first().row();

    if (curRow < 1)
        return;

    rulesModel->moveRowUp(curRow);
    ui.rulesTable->selectRow(curRow - 1);
}

void RuleWidget::on_ruleDown_clicked()
{
    QModelIndexList selectedRows = ui.rulesTable->selectionModel()->selectedRows();

    if (selectedRows.empty())
        return;

    int curRow = selectedRows.first().row();

    if (curRow >= rulesModel->rowCount() - 1)
        return;

    rulesModel->moveRowDown(curRow);
    ui.rulesTable->selectRow(curRow + 1);
}

bool RuleWidget::agreeRuleImmediately(QString text)
{
    text.replace(QLatin1Char('<'), QLatin1String("&#60;"));
    text.replace(QLatin1Char('='), QLatin1String("&#61;"));
    text.replace(QLatin1Char('>'), QLatin1String("&#62;"));

    QMessageBox msgBox(baseValues.mainWindow_);
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setWindowTitle(windowTitle());
    msgBox.setText(
        julyTr("INVALID_RULE_CHECK",
               "This rule will be executed instantly.<br>This means that you make a mistake.<br>Please check values you entered.") +
        "<br><br>\"" + text + "\"");
    auto buttonYes = msgBox.addButton(julyTr("RULE_ENABLE", "Enable"), QMessageBox::YesRole);
    msgBox.addButton(julyTr("TRCANCEL", "Cancel"), QMessageBox::NoRole);
    msgBox.exec();
    return msgBox.clickedButton() == buttonYes;
}

void RuleWidget::ruleEnableSelected()
{
    QModelIndexList selectedRows = ui.rulesTable->selectionModel()->selectedRows();

    if (selectedRows.empty())
        return;

    int curRow = selectedRows.first().row();

    if (rulesModel->testRuleByRow(curRow))
        if (!agreeRuleImmediately(rulesModel->holderList.at(curRow).description))
            return;

    rulesModel->setRuleStateByRow(curRow, 1); // Enable
    checkValidRulesButtons();
}

void RuleWidget::ruleDisableSelected()
{
    QModelIndexList selectedRows = ui.rulesTable->selectionModel()->selectedRows();

    if (selectedRows.empty())
        return;

    int curRow = selectedRows.first().row();

    if (!rulesModel->isConcurrentMode && curRow < rulesModel->rowCount() - 1 && rulesModel->getStateByRow(curRow + 1) == 1 &&
        !rulesModel->isRowPaused(curRow))
    {
        if (rulesModel->testRuleByRow(curRow + 1, true))
            if (!agreeRuleImmediately(rulesModel->holderList.at(curRow + 1).description))
                return;
    }

    rulesModel->setRuleStateByRow(curRow, 0); // Disable
    checkValidRulesButtons();
}

void RuleWidget::ruleEnableAll()
{
    QTimer::singleShot(100, this, SLOT(ruleEnableAllSlot()));
}

void RuleWidget::ruleEnableAllSlot()
{
    for (int curRow = 0; curRow < rulesModel->holderList.size(); curRow++)
    {
        if (rulesModel->getStateByRow(curRow) == 1)
            continue;

        if (qobject_cast<QAction*>(sender()) && rulesModel->testRuleByRow(curRow))
            if (!agreeRuleImmediately(rulesModel->holderList.at(curRow).description))
                continue;

        rulesModel->setRuleStateByRow(curRow, 1);
    }

    checkValidRulesButtons();
}

void RuleWidget::ruleDisableAll()
{
    mainWindow.clearPendingGroup(groupName);
    rulesModel->disableAll();
    checkValidRulesButtons();
}

void RuleWidget::on_ruleSave_clicked()
{
    QString lastRulesDir = mainWindow.iniSettings->value("UI/LastRulesPath", baseValues.desktopLocation).toString();

    if (!QFile::exists(lastRulesDir))
        lastRulesDir = baseValues.desktopLocation;

    QString fileName = QFileDialog::getSaveFileName(
        baseValues.mainWindow_,
        julyTr("SAVE_GOUP", "Save Rules Group"),
        lastRulesDir + "/" + QString(groupName).replace("/", "_").replace("\\", "").replace(":", "").replace("?", "") + ".JLR",
        "JL Ruels (*.JLR)");

    if (fileName.isEmpty())
        return;

    mainWindow.iniSettings->setValue("UI/LastRulesPath", QFileInfo(fileName).dir().path());
    mainWindow.iniSettings->sync();

    if (QFile::exists(fileName))
        QFile::remove(fileName);

    QSettings saveScript(fileName, QSettings::IniFormat);
    saveScript.beginGroup("JLRuleGroup");
    saveScript.setValue("Version", baseValues.jlScriptVersion);
    saveScript.setValue("Name", groupName);
    saveScript.endGroup();

    for (int n = 0; n < rulesModel->holderList.size(); n++)
        RuleScriptParser::writeHolderToSettings(rulesModel->holderList[n], saveScript, "Rule_" + QString::number(n + 101));

    saveScript.sync();

    if (!QFile::exists(fileName))
    {
        QMessageBox::warning(baseValues.mainWindow_, windowTitle(), "Can not write file");
        return;
    }
}
