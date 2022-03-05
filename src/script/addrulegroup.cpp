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

#include "addrulegroup.h"
#include "main.h"
#include "rulescriptparser.h"
#include "thisfeatureunderdevelopment.h"
#include <QFileDialog>
#include <QMessageBox>

AddRuleGroup::AddRuleGroup(QWidget* parent) : QDialog(parent)
{
    ui.setupUi(this);
    setWindowFlags(Qt::WindowCloseButtonHint);

    for (RuleWidget* currentGroup : mainWindow.ui.tabRules->findChildren<RuleWidget*>())
        ui.existingRulesList->addItem(currentGroup->windowTitle(), currentGroup->property("FileName").toString());

    QString groupLabel = julyTr("RULE_GROUP", "Group");
    QString newGroupName = groupLabel;

    int incr = 0;
    QStringList existingGroups = mainWindow.getRuleGroupsNames();

    while (existingGroups.contains(newGroupName))
    {
        newGroupName = groupLabel;

        if (incr > 0)
            newGroupName += " " + QString::number(incr);

        incr++;
    }

    ui.groupName->setText(newGroupName);

    ui.checkExistingRule->setEnabled(ui.existingRulesList->count());
    ui.existingRulesList->setEnabled(ui.existingRulesList->count());

    setWindowTitle(julyTranslator.translateButton("ADD_RULES_GROUP", "Add Rules Group"));

    mainWindow.fixAllChildButtonsAndLabels(this);
    julyTranslator.translateUi(this);

    resize(width(), minimumSizeHint().height());
    setFixedHeight(height());
    onGroupContentChanged(true);

    bool haveTemplate = true;
    int templateId = 1;

    while (haveTemplate)
    {
        QString templateIdStr = QString::number(templateId);

        if (templateIdStr.length() == 1)
            templateIdStr.prepend("00");

        if (templateIdStr.length() == 2)
            templateIdStr.prepend("0");

        QString currentRule = ":/Resources/Templates/" + templateIdStr + ".JLR";
        haveTemplate = QFile::exists(currentRule);

        if (haveTemplate)
        {
            templateId++;
            QSettings loadScript(currentRule, QSettings::IniFormat);
            QString currentName = loadScript.value("JLRuleGroup/Name").toString();

            if (currentName != "")
                ui.useRulesGroupTemplateList->addItem(currentName, currentRule);
        }
    }

    ui.checkUseTemplate->setEnabled(ui.useRulesGroupTemplateList->count());
}

AddRuleGroup::~AddRuleGroup()
{
}

void AddRuleGroup::onGroupContentChanged(bool on)
{
    if (!on)
        return;

    if (ui.checkExistingRule->isEnabled())
        ui.existingRulesList->setEnabled(ui.checkExistingRule->isChecked());

    if (ui.checkUseTemplate->isEnabled())
        ui.useRulesGroupTemplateList->setEnabled(ui.checkUseTemplate->isChecked());

    ui.rulesFile->setEnabled(ui.checkUseFile->isChecked());
    ui.ruleOpen->setEnabled(ui.checkUseFile->isChecked());

    if (ui.checkUseFile->isChecked())
        on_ruleOpen_clicked();
    else
        ui.groupNameGroupbox->setEnabled(true);

    checkValidButton();
}

void AddRuleGroup::on_ruleOpen_clicked()
{
    QString lastRulesDir = mainWindow.iniSettings->value("UI/LastRulesPath", baseValues.desktopLocation).toString();

    if (!QFile::exists(lastRulesDir))
        lastRulesDir = baseValues.desktopLocation;

    QString fileNameOpen = QFileDialog::getOpenFileName(this, julyTr("OPEN_GOUP", "Open Rules Group"), lastRulesDir, "(*.JLR)");

    if (fileNameOpen.isEmpty())
    {
        ui.checkEmptyRule->setChecked(true);
        return;
    }

    QString scriptNameFile = QSettings(fileNameOpen, QSettings::IniFormat).value("JLRuleGroup/Name", "").toString();
    {
        // Validate saved file
        if (scriptNameFile.isEmpty())
        {
            QMessageBox::warning(this, windowTitle(), julyTr("OPEN_INVALID_SCRIPT", "Invalid script \"%1\"").arg(fileNameOpen));
            ui.checkEmptyRule->setChecked(true);
            return;
        }
    }

#ifdef Q_OS_WIN
    fileName.replace("/", "\\");
#endif

    ui.rulesFile->setText(fileNameOpen);
    mainWindow.iniSettings->setValue("UI/LastRulesPath", QFileInfo(fileNameOpen).dir().path());
    mainWindow.iniSettings->sync();

    ui.groupNameGroupbox->setEnabled(groupsList.size() <= 1);

    if (groupsList.size() == 1)
        ui.groupName->setText(groupsList.first().split("==>").first());
}

void AddRuleGroup::on_buttonAddRule_clicked()
{
    QString filePath;
    groupName = ui.groupName->text();

    if (ui.checkExistingRule->isChecked() && ui.existingRulesList->currentIndex() > -1)
        filePath = ui.existingRulesList->itemData(ui.existingRulesList->currentIndex()).toString();
    else if (ui.checkUseTemplate->isChecked() && ui.useRulesGroupTemplateList->currentIndex() > -1)
        filePath = ui.useRulesGroupTemplateList->itemData(ui.useRulesGroupTemplateList->currentIndex()).toString();
    else if (ui.checkUseFile->isChecked())
        filePath = ui.rulesFile->text();

    QList<RuleHolder> holderList;

    if (!filePath.isEmpty())
    {
        QSettings loadRules(filePath, QSettings::IniFormat);
        QStringList rulesList = loadRules.childGroups();

        for (const QString& group : rulesList)
        {
            if (!group.startsWith("Rule_"))
                continue;

            RuleHolder holder = RuleScriptParser::readHolderFromSettings(loadRules, group);

            if (holder.isValid())
                holderList << holder;
        }
    }

    QString nameTemplate(baseValues.scriptFolder + "Rule_%1.JLR");
    int ruleN = 1;

    while (QFile::exists(nameTemplate.arg(ruleN)))
        ruleN++;

    filePath = nameTemplate.arg(ruleN);
    QSettings saveScript(filePath, QSettings::IniFormat);
    saveScript.beginGroup("JLRuleGroup");
    saveScript.setValue("Version", baseValues.jlScriptVersion);
    saveScript.setValue("Name", groupName);
    saveScript.endGroup();

    for (int n = 0; n < holderList.size(); n++)
        RuleScriptParser::writeHolderToSettings(holderList[n], saveScript, "Rule_" + QString::number(n + 101));

    saveScript.sync();

    if (!QFile::exists(filePath))
        QMessageBox::warning(this, windowTitle(), "Can't write file: " + filePath);
    else
    {
        fileName = filePath;
        accept();
    }
}

void AddRuleGroup::on_groupName_textChanged(QString /*unused*/)
{
    checkValidButton();
}

void AddRuleGroup::checkValidButton()
{
    bool isAbleToSave = ui.groupName->text().length() > 0;

    if (isAbleToSave && ui.checkUseFile->isChecked())
    {
        if (ui.rulesFile->text().length() < 3)
            isAbleToSave = false;
        else
            isAbleToSave = QFile::exists(ui.rulesFile->text());
    }

    ui.buttonAddRule->setEnabled(isAbleToSave);
}
