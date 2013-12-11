// Copyright (C) 2013 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form https://sourceforge.net/projects/bitcointrader/
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#include "addrulegroup.h"
#include "main.h"
#include "thisfeatureunderdevelopment.h"

AddRuleGroup::AddRuleGroup(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	setWindowFlags(Qt::WindowCloseButtonHint);

	foreach(RuleWidget* currentGroup, mainWindow.ui.tabRules->findChildren<RuleWidget*>())
		existingGroups<<currentGroup->windowTitle();

	existingGroups.sort();
	int ruleInc=mainWindow.ui.rulesTabs->count();
	QString newGroupName;
	do newGroupName=julyTr("RULE_GROUP","Group %1").arg(++ruleInc);
	while(existingGroups.contains(newGroupName));
	ui.groupName->setText(newGroupName);
	ui.existingRulesList->addItems(existingGroups);

	ui.checkExistingRule->setEnabled(existingGroups.count());
	ui.existingRulesList->setEnabled(existingGroups.count());

	julyTranslator.translateUi(this);

	setWindowTitle(julyTranslator.translateButton("ADD_RULES_GROUP","Add Rules Group"));

	mainWindow.fixAllChildButtonsAndLabels(this);

	resize(width(),minimumSizeHint().height());
	setFixedHeight(height());
	onGroupContentChanged(true);
}

AddRuleGroup::~AddRuleGroup()
{

}

void AddRuleGroup::onGroupContentChanged(bool on)
{
	if(!on)return;
	if(ui.checkUseTemplate->isChecked())
	{
		ui.checkEmptyRule->setChecked(true);
		ThisFeatureUnderDevelopment featureNotAvailable;
		if(mainWindow.ui.widgetStaysOnTop->isChecked())featureNotAvailable.setWindowFlags(Qt::WindowCloseButtonHint|Qt::WindowStaysOnTopHint);
		else  featureNotAvailable.setWindowFlags(Qt::WindowCloseButtonHint);
		julyTranslator.translateUi(&featureNotAvailable);
		featureNotAvailable.setFixedSize(380,featureNotAvailable.minimumSizeHint().height());
		featureNotAvailable.exec();
		return;
	}
	if(ui.checkExistingRule->isEnabled())ui.existingRulesList->setEnabled(ui.checkExistingRule->isChecked());
	if(ui.checkUseTemplate->isEnabled())ui.useRulesGroupTemplateList->setEnabled(ui.checkUseTemplate->isChecked());
}

void AddRuleGroup::on_buttonAddRule_clicked()
{
	groupName=ui.groupName->text();
	if(ui.checkExistingRule->isChecked())copyFromExistingGroup=ui.existingRulesList->currentText();
	accept();
}

void AddRuleGroup::on_groupName_textChanged(QString text)
{
	ui.buttonAddRule->setEnabled(text.length()&&!existingGroups.contains(text));
}