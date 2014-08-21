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

#include "rulewidget.h"
#include "main.h"
#include <QMessageBox>
#include <QtCore/qmath.h>
#include <QFileDialog>
#include <QDesktopServices>
#include "rulescriptparser.h"
#include "addruledialog.h"
#include "exchange.h"

RuleWidget::RuleWidget(QString fileName)
	: QWidget()
{
    ui.setupUi(this);

    filePath=fileName;

    setProperty("FileName",filePath);

    QSettings loadRule(fileName,QSettings::IniFormat);
    loadRule.beginGroup("JLRuleGroup");
    groupName=loadRule.value("Name","Unknown").toString();
    ui.ruleBeep->setChecked(loadRule.value("BeepOnDone",false).toBool());
    loadRule.endGroup();

    ordersCancelTime=QTime(1,0,0,0);
    setAttribute(Qt::WA_DeleteOnClose,true);

	updateStyleSheets();

	ui.rulesNoMessage->setVisible(true);
	ui.rulesTable->setVisible(false);

    rulesModel=new RulesModel(groupName);
	rulesModel->setParent(this);
	ui.rulesTable->setModel(rulesModel);
    mainWindow.setColumnResizeMode(ui.rulesTable,0,QHeaderView::ResizeToContents);
    mainWindow.setColumnResizeMode(ui.rulesTable,1,QHeaderView::Stretch);

	connect(ui.rulesTable->selectionModel(),SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),this,SLOT(checkValidRulesButtons()));
	ui.rulesTable->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui.rulesTable, SIGNAL(customContextMenuRequested(const QPoint&)), SLOT(rulesMenuRequested(const QPoint&)));

	rulesEnableDisableMenu=new QMenu;
	rulesEnableDisableMenu->addAction("Enable Selected");
	connect(rulesEnableDisableMenu->actions().last(),SIGNAL(triggered(bool)),this,SLOT(ruleEnableSelected()));
	rulesEnableDisableMenu->addAction("Disable Selected");
	connect(rulesEnableDisableMenu->actions().last(),SIGNAL(triggered(bool)),this,SLOT(ruleDisableSelected()));
	rulesEnableDisableMenu->addSeparator();
	rulesEnableDisableMenu->addAction("Enable All");
	connect(rulesEnableDisableMenu->actions().last(),SIGNAL(triggered(bool)),this,SLOT(ruleEnableAll()));
	rulesEnableDisableMenu->addAction("Disable All");
	connect(rulesEnableDisableMenu->actions().last(),SIGNAL(triggered(bool)),this,SLOT(ruleDisableAll()));
	ui.ruleEnableDisable->setMenu(rulesEnableDisableMenu);
	connect(rulesEnableDisableMenu,SIGNAL(aboutToShow()),this,SLOT(ruleDisableEnableMenuFix()));

	languageChanged();

    setWindowTitle(groupName);

    QStringList rulesList=loadRule.childGroups();
    Q_FOREACH(QString group, rulesList)
    {
        if(!group.startsWith("Rule_"))continue;
        RuleHolder holder=RuleScriptParser::readHolderFromSettings(loadRule,group);
        if(holder.isValid())rulesModel->addRule(holder);
    }

	saveRulesData();

	checkValidRulesButtons();

	mainWindow.fixTableViews(this);
}

RuleWidget::~RuleWidget()
{
    if(!filePath.isEmpty())saveRulesData();
}

bool RuleWidget::isBeepOnDone()
{
    return ui.ruleBeep->isChecked();
}

void RuleWidget::updateStyleSheets()
{
	ui.rulesNoMessage->setStyleSheet("border: 1px solid gray; background: "+baseValues.appTheme.white.name()+"; color: "+baseValues.appTheme.gray.name());
}

bool RuleWidget::removeGroup()
{
    bool removed=true;
    if(!filePath.isEmpty()){QFile::remove(filePath);removed=!QFile::exists(filePath);};
    filePath.clear();
    return removed;
}

void RuleWidget::languageChanged()
{
	julyTranslator.translateUi(this);

    rulesModel->setHorizontalHeaderLabels(QStringList()<<julyTr("RULES_T_STATE","State")<<julyTr("RULES_T_DESCR","Description"));
            //Removed <<julyTr("RULES_T_ACTION","Action")<<julyTr("ORDERS_AMOUNT","Amount")<<julyTr("RULES_T_PRICE","Price"));

	rulesEnableDisableMenu->actions().at(0)->setText(julyTr("RULE_ENABLE","Enable Selected"));
	rulesEnableDisableMenu->actions().at(1)->setText(julyTr("RULE_DISABLE","Disable Selected"));
	rulesEnableDisableMenu->actions().at(3)->setText(julyTr("RULE_ENABLE_ALL","Enable All"));
	rulesEnableDisableMenu->actions().at(4)->setText(julyTr("RULE_DISABLE_ALL","Disable All"));

	mainWindow.fixAllChildButtonsAndLabels(this);
}

void RuleWidget::saveRulesData()
{
    if(QFile::exists(filePath))QFile::remove(filePath);
    QSettings saveScript(filePath,QSettings::IniFormat);
    saveScript.beginGroup("JLRuleGroup");
    saveScript.setValue("Version",baseValues.jlScriptVersion);
    saveScript.setValue("Name",groupName);
    saveScript.setValue("BeepOnDone",ui.ruleBeep->isChecked());
    saveScript.endGroup();

    for(int n=0;n<rulesModel->holderList.count();n++)
        RuleScriptParser::writeHolderToSettings(rulesModel->holderList[n],saveScript,"Rule_"+QString::number(n+101));
    saveScript.sync();
}

void RuleWidget::on_ruleAddButton_clicked()
{
    AddRuleDialog ruleWindow(this);
    if(!mainWindow.isDetachedRules)ruleWindow.setWindowFlags(mainWindow.windowFlags());
    if(ruleWindow.exec()==QDialog::Rejected)return;

    RuleHolder holder=ruleWindow.getRuleHolder();
    if(!holder.isValid())return;

    rulesModel->addRule(holder,ruleWindow.isRuleEnabled());

    ui.rulesNoMessage->setVisible(false);
    ui.rulesTable->setVisible(true);
    checkValidRulesButtons();
    saveRulesData();
}

void RuleWidget::on_ruleConcurrentMode_toggled(bool on)
{
	rulesModel->isConcurrentMode=on;
}

void RuleWidget::on_ruleEditButton_clicked()
{
	QModelIndexList selectedRows=ui.rulesTable->selectionModel()->selectedRows();
	if(selectedRows.count()==0)return;
	int curRow=selectedRows.first().row();
	if(curRow<0)return;

    AddRuleDialog ruleWindow(this);
    if(!mainWindow.isDetachedRules)ruleWindow.setWindowFlags(mainWindow.windowFlags());
    ruleWindow.fillByHolder(rulesModel->holderList[curRow]);
    if(ruleWindow.exec()==QDialog::Rejected)return;

    RuleHolder holder=ruleWindow.getRuleHolder();
    if(!holder.isValid())return;

    rulesModel->setRuleStateByRow(curRow,0);
    rulesModel->updateRule(curRow,holder,ruleWindow.isRuleEnabled());

	checkValidRulesButtons();
	saveRulesData();
}

void RuleWidget::on_ruleRemoveAll_clicked()
{
	QMessageBox msgBox(this);
	msgBox.setIcon(QMessageBox::Question);
    msgBox.setWindowTitle(julyTr("APPLICATION_TITLE",windowTitle()));
	msgBox.setText(julyTr("RULE_CONFIRM_REMOVE_ALL","Are you sure to remove all rules?"));
	msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	msgBox.setDefaultButton(QMessageBox::Yes);
	msgBox.setButtonText(QMessageBox::Yes,julyTr("YES","Yes"));
	msgBox.setButtonText(QMessageBox::No,julyTr("NO","No"));
	if(msgBox.exec()!=QMessageBox::Yes)return;

	rulesModel->clear();
	checkValidRulesButtons();
	saveRulesData();
}

void RuleWidget::on_ruleRemove_clicked()
{
	QMessageBox msgBox(this);
	msgBox.setIcon(QMessageBox::Question);
    msgBox.setWindowTitle(julyTr("APPLICATION_TITLE",windowTitle()));
	msgBox.setText(julyTr("RULE_CONFIRM_REMOVE","Are you sure to remove this rule?"));
	msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	msgBox.setDefaultButton(QMessageBox::Yes);
	msgBox.setButtonText(QMessageBox::Yes,julyTr("YES","Yes"));
	msgBox.setButtonText(QMessageBox::No,julyTr("NO","No"));
	if(msgBox.exec()!=QMessageBox::Yes)return;

	QModelIndexList selectedRows=ui.rulesTable->selectionModel()->selectedRows();
	if(selectedRows.count()==0)return;
	int curRow=selectedRows.first().row();
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
	bool haveRules_=rulesModel->rowCount()>0;
	QModelIndexList selectedRows=ui.rulesTable->selectionModel()->selectedRows();

	int selectedRulesCount=selectedRows.count();
	bool ifSelectedOneRuleIsItEnabled=selectedRulesCount==1;
	if(ifSelectedOneRuleIsItEnabled)
	{
        ifSelectedOneRuleIsItEnabled=rulesModel->getStateByRow(selectedRows.first().row())==1;
		rulesEnableDisableMenu->actions().at(0)->setEnabled(!ifSelectedOneRuleIsItEnabled);
		rulesEnableDisableMenu->actions().at(1)->setEnabled(ifSelectedOneRuleIsItEnabled);
	}
	else
	{
		rulesEnableDisableMenu->actions().at(0)->setEnabled(selectedRulesCount>0);
		rulesEnableDisableMenu->actions().at(1)->setEnabled(selectedRulesCount>0);
	}
	rulesEnableDisableMenu->actions().at(2)->setEnabled(haveRules_);
	rulesEnableDisableMenu->actions().at(3)->setEnabled(haveRules_);
}

bool RuleWidget::haveWorkingRules()
{
	return rulesModel->haveWorkingRule();
}

bool RuleWidget::haveAnyRules()
{
	return rulesModel->rowCount()>0;
}

bool RuleWidget::haveAnyTradingRules()
{
	return rulesModel->haveAnyTradingRules();
}

void RuleWidget::currencyChanged()
{
    if(baseValues.currentExchange_->multiCurrencyTradeSupport)return;
    rulesModel->currencyChanged();
}

void RuleWidget::checkValidRulesButtons()
{
	int selectedCount=ui.rulesTable->selectionModel()->selectedRows().count();
	ui.ruleEditButton->setEnabled(selectedCount==1);
	ui.ruleRemove->setEnabled(selectedCount);
	rulesEnableDisableMenu->actions().at(0)->setEnabled(selectedCount);
	rulesEnableDisableMenu->actions().at(1)->setEnabled(selectedCount);
	ui.ruleEnableDisable->setEnabled(rulesModel->rowCount());
	ui.ruleRemoveAll->setEnabled(rulesModel->rowCount());
	ui.ruleConcurrentMode->setEnabled(rulesModel->rowCount()==0||rulesModel->allDisabled);
	ui.ruleSequencialMode->setEnabled(rulesModel->rowCount()==0||rulesModel->allDisabled);

	ui.rulesNoMessage->setVisible(rulesModel->rowCount()==0);
	ui.rulesTable->setVisible(rulesModel->rowCount());

	ui.ruleUp->setEnabled(ui.ruleEditButton->isEnabled()&&rulesModel->rowCount()>1);
	ui.ruleDown->setEnabled(ui.ruleEditButton->isEnabled()&&rulesModel->rowCount()>1);
}

void RuleWidget::on_ruleUp_clicked()
{
	QModelIndexList selectedRows=ui.rulesTable->selectionModel()->selectedRows();
	if(selectedRows.count()==0)return;
	int curRow=selectedRows.first().row();
	if(curRow<1)return;
	rulesModel->moveRowUp(curRow);
    ui.rulesTable->selectRow(curRow-1);
}

void RuleWidget::on_ruleDown_clicked()
{
	QModelIndexList selectedRows=ui.rulesTable->selectionModel()->selectedRows();
	if(selectedRows.count()==0)return;
	int curRow=selectedRows.first().row();
	if(curRow>=rulesModel->rowCount()-1)return;

	rulesModel->moveRowDown(curRow);
    ui.rulesTable->selectRow(curRow+1);
}

void RuleWidget::ruleEnableSelected()
{
	QModelIndexList selectedRows=ui.rulesTable->selectionModel()->selectedRows();
	if(selectedRows.count()==0)return;
	int curRow=selectedRows.first().row();
	rulesModel->setRuleStateByRow(curRow,1);//Enable
	checkValidRulesButtons();
}

void RuleWidget::ruleDisableSelected()
{
	QModelIndexList selectedRows=ui.rulesTable->selectionModel()->selectedRows();
	if(selectedRows.count()==0)return;
	int curRow=selectedRows.first().row();
	rulesModel->setRuleStateByRow(curRow,0);//Disable
	checkValidRulesButtons();
}

void RuleWidget::ruleEnableAll()
{
	rulesModel->enableAll();
	checkValidRulesButtons();
}

void RuleWidget::ruleDisableAll()
{
	rulesModel->disableAll();
	checkValidRulesButtons();
}

void RuleWidget::on_ruleSave_clicked()
{
    QString lastRulesDir=mainWindow.iniSettings->value("UI/LastRulesPath",baseValues.desktopLocation).toString();
    if(!QFile::exists(lastRulesDir))lastRulesDir=baseValues.desktopLocation;

    QString fileName=QFileDialog::getSaveFileName(this, julyTr("SAVE_GOUP","Save Rules Group"),lastRulesDir+"/"+groupName+".JLR","(*.JLR)");
	if(fileName.isEmpty())return;
	mainWindow.iniSettings->setValue("UI/LastRulesPath",QFileInfo(fileName).dir().path());
	mainWindow.iniSettings->sync();
	if(QFile::exists(fileName))QFile::remove(fileName);

    QSettings saveScript(fileName,QSettings::IniFormat);
    saveScript.beginGroup("JLRuleGroup");
    saveScript.setValue("Version",baseValues.jlScriptVersion);
    saveScript.setValue("Name",groupName);
    saveScript.endGroup();

    for(int n=0;n<rulesModel->holderList.count();n++)
        RuleScriptParser::writeHolderToSettings(rulesModel->holderList[n],saveScript,"Rule_"+QString::number(n+101));
    saveScript.sync();

    if(!QFile::exists(fileName))
    {
        QMessageBox::warning(this,windowTitle(),"Can not write file");
        return;
    }
}
