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

#include "addrulegroup.h"
#include "main.h"
#include "thisfeatureunderdevelopment.h"
#include <QDesktopServices>
#include <QFileDialog>
#include <QDebug>
AddRuleGroup::AddRuleGroup(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	setWindowFlags(Qt::WindowCloseButtonHint);

	foreach(RuleWidget* currentGroup, mainWindow.ui.tabRules->findChildren<RuleWidget*>())
	{
		existingGroupsIDs<<currentGroup->getRuleGroupId();
		existingGroups<<currentGroup->windowTitle();
	}

	existingGroups.sort();
	ui.groupName->setText(julyTr("RULE_GROUP","Group"));
	ui.existingRulesList->addItems(existingGroups);

	ui.checkExistingRule->setEnabled(existingGroups.count());
	ui.existingRulesList->setEnabled(existingGroups.count());

	julyTranslator.translateUi(this);

	setWindowTitle(julyTranslator.translateButton("ADD_RULES_GROUP","Add Rules Group"));

	mainWindow.fixAllChildButtonsAndLabels(this);

	resize(width(),minimumSizeHint().height());
	setFixedHeight(height());
	onGroupContentChanged(true);
	ui.groupID->setMaximum(baseValues.lastGroupID+1);
	ui.groupID->setValue(ui.groupID->maximum());

	bool haveTemplate=true;
	int templateId=1;
	while(haveTemplate)
	{
		QString templateIdStr=QString::number(templateId);
		if(templateIdStr.length()==1)templateIdStr.prepend("00");
		if(templateIdStr.length()==2)templateIdStr.prepend("0");
		QString currentRule=":/Resources/Templates/"+templateIdStr+".qbtrule";
		haveTemplate=QFile::exists(currentRule);
		if(haveTemplate)
		{
			templateId++;
			QByteArray currentRuleData;
			QFile readRule(currentRule);
			readRule.open(QIODevice::ReadOnly);
			currentRuleData=readRule.readLine();
			if(!currentRuleData.startsWith("Qt Bitcoin Trader Rules"))continue;

			QStringList rulePair=QString(readRule.readAll()).split("==>");
			if(rulePair.count()!=2||rulePair.first().isEmpty())continue;
			templatesList<<currentRule;
			ui.useRulesGroupTemplateList->addItem(rulePair.first());
		}
	}
}

AddRuleGroup::~AddRuleGroup()
{

}

void AddRuleGroup::onGroupContentChanged(bool on)
{
	if(!on)return;

	if(ui.checkExistingRule->isEnabled())ui.existingRulesList->setEnabled(ui.checkExistingRule->isChecked());
	if(ui.checkUseTemplate->isEnabled())ui.useRulesGroupTemplateList->setEnabled(ui.checkUseTemplate->isChecked());

	ui.rulesFile->setEnabled(ui.checkUseFile->isChecked());
	ui.ruleOpen->setEnabled(ui.checkUseFile->isChecked());

	if(ui.checkUseFile->isChecked())on_ruleOpen_clicked();
	else ui.groupNameGroupbox->setEnabled(true);

	checkValidButton();
}

bool AddRuleGroup::loadGroupFromFile(QString fileName)
{
	QByteArray rulesData;
	QFile validateRule(fileName);
	if(validateRule.open(QIODevice::ReadOnly))rulesData=validateRule.readAll();
	if(rulesData.startsWith("Qt Bitcoin Trader Rules\n"))rulesData.remove(0,24);
	else return false;
	groupsList=QString(rulesData).split("\n");
	return true;
}

void AddRuleGroup::on_ruleOpen_clicked()
{
	QString lastRulesDir=mainWindow.iniSettings->value("UI/LastRulesPath",QDesktopServices::storageLocation(QDesktopServices::DesktopLocation)).toString();

	QString fileName=QFileDialog::getOpenFileName(this, julyTr("OPEN_GOUP","Open Rules Group"),lastRulesDir,"(*.qbtrule)");
	if(fileName.isEmpty())
	{
		ui.checkEmptyRule->setChecked(true);
		return;
	}

	if(!loadGroupFromFile(fileName))
	{
		ui.checkEmptyRule->setChecked(true);
		return;
	}
	
#ifdef Q_OS_WIN
	fileName.replace("/","\\");
#endif

	ui.rulesFile->setText(fileName);
	mainWindow.iniSettings->setValue("UI/LastRulesPath",QFileInfo(fileName).dir().path());
	mainWindow.iniSettings->sync();


	ui.groupNameGroupbox->setEnabled(groupsList.count()<=1);
	if(groupsList.count()==1)ui.groupName->setText(groupsList.first().split("==>").first());
}

void AddRuleGroup::on_buttonAddRule_clicked()
{
	groupName=ui.groupName->text();
	if(ui.checkExistingRule->isChecked())copyFromExistingGroup=existingGroupsIDs.at(ui.existingRulesList->currentIndex());
	if(ui.checkUseTemplate->isChecked())loadGroupFromFile(templatesList.at(ui.useRulesGroupTemplateList->currentIndex()));
	else
	if(!ui.checkUseFile->isChecked())groupsList.clear();
	accept();
}

void AddRuleGroup::on_groupName_textChanged(QString)
{
	checkValidButton();
}

void AddRuleGroup::checkValidButton()
{
	bool isAbleToSave=!existingGroups.contains(":");
	if(isAbleToSave&&ui.checkUseFile->isChecked())
	{
		if(ui.rulesFile->text().length()<3)isAbleToSave=false;
		else isAbleToSave=QFile::exists(ui.rulesFile->text());
	}
	ui.buttonAddRule->setEnabled(isAbleToSave);
}