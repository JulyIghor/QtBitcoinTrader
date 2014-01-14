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
#include "addrulewindow.h"
#include <QMessageBox>
#include <QtCore/qmath.h>
#include <QFileDialog>
#include <QDesktopServices>

RuleWidget::RuleWidget(int gID, QString gName, RuleWidget *copyFrom, QString restorableString)
	: QWidget()
{
	ordersCancelTime=QTime(1,0,0,0);
	ui.setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose,true);

	updateStyleSheets();

	ui.rulesNoMessage->setVisible(true);
	ui.rulesTable->setVisible(false);

	rulesModel=new RulesModel;
	rulesModel->setParent(this);
	ui.rulesTable->setModel(rulesModel);
	ui.rulesTable->horizontalHeader()->setResizeMode(0,QHeaderView::ResizeToContents);
	ui.rulesTable->horizontalHeader()->setResizeMode(1,QHeaderView::Stretch);
	ui.rulesTable->horizontalHeader()->setResizeMode(2,QHeaderView::ResizeToContents);
	ui.rulesTable->horizontalHeader()->setResizeMode(3,QHeaderView::ResizeToContents);
	ui.rulesTable->horizontalHeader()->setResizeMode(4,QHeaderView::ResizeToContents);
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

	setRuleGroupId(gID);

	languageChanged();

	groupName=gName;

	if(copyFrom)restorableString=copyFrom->rulesModel->saveRulesToString();
	   else
	if(restorableString.isEmpty())
	{
		mainWindow.iniSettings->beginGroup("Rules");
		restorableString=mainWindow.iniSettings->value(ruleGroupIdStr,"").toString();
		mainWindow.iniSettings->endGroup();
	}

	QStringList settingsParams=restorableString.split(":");
	if(settingsParams.count()>1)
	{
		restorableString=settingsParams.first();
		settingsParams.removeFirst();
		ui.ruleBeep->setChecked(settingsParams.first().toInt()==1);
	}

	if(settingsParams.count()>1)
	{
		if(groupName.isEmpty())
		{
		settingsParams.removeFirst();
		groupName=settingsParams.first();
		}
	}
	setWindowTitle(groupName);

	rulesModel->restoreRulesFromString(restorableString);

	saveRulesData();

	checkValidRulesButtons();
}

RuleWidget::~RuleWidget()
{
	if(!groupName.isEmpty())saveRulesData();
}

void RuleWidget::setRuleGroupId(int id)
{
	ui.groupID->setValue(id);
	ruleGroupIdStr=QString::number(id);
	while(ruleGroupIdStr.length()<3)ruleGroupIdStr.prepend("0");
}

QString RuleWidget::getRuleGroupIdStr()
{
	return ruleGroupIdStr;
}

int RuleWidget::getRuleGroupId()
{
	return ui.groupID->value();
}

void RuleWidget::updateStyleSheets()
{
	ui.rulesNoMessage->setStyleSheet("border: 1px solid gray; background: "+baseValues.appTheme.white.name()+"; color: "+baseValues.appTheme.gray.name());
}

void RuleWidget::removeGroup()
{
	mainWindow.iniSettings->remove("Rules/"+ruleGroupIdStr);
	mainWindow.iniSettings->sync();
	groupName.clear();
}

void RuleWidget::languageChanged()
{
	julyTranslator.translateUi(this);

	rulesModel->setHorizontalHeaderLabels(QStringList()<<julyTr("RULES_T_STATE","State")<<julyTr("RULES_T_DESCR","Description")<<julyTr("RULES_T_ACTION","Action")<<julyTr("ORDERS_AMOUNT","Amount")<<julyTr("RULES_T_PRICE","Price"));

	rulesEnableDisableMenu->actions().at(0)->setText(julyTr("RULE_ENABLE","Enable Selected"));
	rulesEnableDisableMenu->actions().at(1)->setText(julyTr("RULE_DISABLE","Disable Selected"));
	rulesEnableDisableMenu->actions().at(3)->setText(julyTr("RULE_ENABLE_ALL","Enable All"));
	rulesEnableDisableMenu->actions().at(4)->setText(julyTr("RULE_DISABLE_ALL","Disable All"));

	mainWindow.fixAllChildButtonsAndLabels(this);
}

void RuleWidget::saveRulesData()
{
	mainWindow.iniSettings->beginGroup("Rules");
	mainWindow.iniSettings->setValue(ruleGroupIdStr,rulesModel->saveRulesToString()+":"+QString::number((ui.ruleBeep->isChecked()?1:0))+":"+groupName);
	mainWindow.iniSettings->endGroup();
	mainWindow.iniSettings->sync();
}

void RuleWidget::on_ruleAddButton_clicked()
{
	AddRuleWindow addRule(this);
	if(!mainWindow.isDetachedRules)addRule.setWindowFlags(mainWindow.windowFlags());
	else addRule.setWindowFlags(windowFlags());
	if(addRule.exec()!=QDialog::Accepted)return;
	RuleHolder *newHolder=new RuleHolder(addRule.getRuleHolder());
	//newHolder->setRuleState(1);
	rulesModel->addRule(newHolder);

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

	AddRuleWindow addRule(this);
	addRule.setWindowFlags(windowFlags());
	RuleHolder *curHolder=rulesModel->getRuleHolderByRow(curRow);
	if(curHolder==0||curHolder->invalidHolder)return;
	addRule.fillByRuleHolder(curHolder);
	if(addRule.exec()!=QDialog::Accepted)return;
	RuleHolder updatedRule=addRule.getRuleHolder();
	rulesModel->updateHolderByRow(curRow,&updatedRule);
	checkValidRulesButtons();
	saveRulesData();
}

void RuleWidget::on_ruleRemoveAll_clicked()
{
	QMessageBox msgBox(this);
	msgBox.setIcon(QMessageBox::Question);
	msgBox.setWindowTitle("Qt Bitcoin Trader");
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
	msgBox.setWindowTitle("Qt Bitcoin Trader");
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
		ifSelectedOneRuleIsItEnabled=rulesModel->getRuleHolderByRow(selectedRows.first().row())->getRuleState()==1;

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
}

void RuleWidget::on_ruleDown_clicked()
{
	QModelIndexList selectedRows=ui.rulesTable->selectionModel()->selectedRows();
	if(selectedRows.count()==0)return;
	int curRow=selectedRows.first().row();
	if(curRow>=rulesModel->rowCount()-1)return;

	rulesModel->moveRowDown(curRow);
}

void RuleWidget::checkAndExecuteRule(int ruleType, double price)
{
	QList<RuleHolder *> achievedHolderList=rulesModel->getAchievedRules(ruleType,price);
	for(int n=0;n<achievedHolderList.count();n++)
	{
		if(baseValues.rulesSafeMode)
		{
		if(mainWindow.lastRuleExecutedTime.elapsed()<baseValues.rulesSafeModeInterval)continue;
		mainWindow.lastRuleExecutedTime.restart();
		}

		if(!mainWindow.isValidSoftLag){achievedHolderList.at(n)->startWaitingLowLag();continue;}

		double ruleBtc=achievedHolderList.at(n)->getRuleBtc();
		bool isBuying=achievedHolderList.at(n)->isBuying();
		double priceToExec=achievedHolderList.at(n)->getRuleExecutePrice();
		double amountPercentage=achievedHolderList.at(n)->getRuleAmountPercentage();
		if(priceToExec<0)priceToExec=achievedHolderList.at(n)->getCurrentExecPrice();

		if(ruleBtc<0)
		{
			if(ruleBtc==-1.0)ruleBtc=mainWindow.getAvailableBTC()*amountPercentage;else//"Sell All my BTC"
				if(ruleBtc==-2.0)ruleBtc=mainWindow.getAvailableBTC()/2.0;else//"Sell Half my BTC"
					if(ruleBtc==-3.0)ruleBtc=mainWindow.getAvailableUSDtoBTC(priceToExec)*amountPercentage;else//"Spend All my Funds"
						if(ruleBtc==-4.0)ruleBtc=mainWindow.getAvailableUSDtoBTC(priceToExec)/2.0;else//"Spend Half my Funds"
							if(ruleBtc==-5.0)//"Cancel All Orders"
							{
								if(ui.ruleConcurrentMode->isChecked())
								{
									mainWindow.cancelAllCurrentPairOrders();
									if(ui.ruleBeep->isChecked())
										mainWindow.beep();
									continue;
								}
								else
								{
									if(mainWindow.ordersModel->rowCount()==0)
									{
										if(ui.ruleBeep->isChecked())mainWindow.beep();
										rulesModel->setRuleStateByHolder(achievedHolderList.at(n),2);
										return;
									}
									else
									{
										if(ordersCancelTime.elapsed()>5000)mainWindow.cancelAllCurrentPairOrders();
										ordersCancelTime.restart();
										continue;
									}
								}
							}
							else
							if(ruleBtc==-6.0)//Enable All Rules
							{
								ruleEnableAll();
								if(ui.ruleBeep->isChecked())mainWindow.beep();continue;
							}
							else
							if(ruleBtc==-7.0)//Disable All Rules
							{
								ruleDisableAll();
								if(ui.ruleBeep->isChecked())mainWindow.beep();continue;
							}
							else
							if(ruleBtc==-8.0)//Enable All Rules in Group #
							{
								mainWindow.enableGroupId(achievedHolderList.at(n)->getRuleGroupId());
								if(ui.ruleBeep->isChecked())mainWindow.beep();
								rulesModel->setRuleStateByHolder(achievedHolderList.at(n),2);continue;
							}
							else
							if(ruleBtc==-9.0)//Disable All Rules in Group #
							{
								mainWindow.disableGroupId(achievedHolderList.at(n)->getRuleGroupId());
								if(ui.ruleBeep->isChecked())mainWindow.beep();
								rulesModel->setRuleStateByHolder(achievedHolderList.at(n),2);continue;
							}
							else
							if(ruleBtc==-10.0)//Beep
							{
								mainWindow.beep(true);
								rulesModel->setRuleStateByHolder(achievedHolderList.at(n),2);continue;
							}
							else
							if(ruleBtc==-11.0)//Play Sound
							{
								mainWindow.playWav(achievedHolderList.at(n)->getRuleWavFile(),true);
										rulesModel->setRuleStateByHolder(achievedHolderList.at(n),2);continue;
							}
		}


		if(ruleBtc<baseValues.currentPair.tradeVolumeMin)
			if(debugLevel)logThread->writeLog("Volume too low");

		if(isBuying)mainWindow.apiBuySend(ruleBtc,priceToExec);
		else mainWindow.apiSellSend(ruleBtc,priceToExec);

		rulesModel->setRuleStateByHolder(achievedHolderList.at(n),2);
		if(ui.ruleBeep->isChecked())mainWindow.beep();
	}
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
	QString lastRulesDir=mainWindow.iniSettings->value("UI/LastRulesPath",QDesktopServices::storageLocation(QDesktopServices::DesktopLocation)).toString();

	QString fileName=QFileDialog::getSaveFileName(this, julyTr("SAVE_GOUP","Save Rules Group"),lastRulesDir+"/"+groupName+".qbtrule","(*.qbtrule)");
	if(fileName.isEmpty())return;
	mainWindow.iniSettings->setValue("UI/LastRulesPath",QFileInfo(fileName).dir().path());
	mainWindow.iniSettings->sync();
	if(QFile::exists(fileName))QFile::remove(fileName);

	QFile saveRule(fileName);
	if(!saveRule.open(QIODevice::WriteOnly))
	{
		QMessageBox::warning(this,windowTitle(),"Can not write file");
		return;
	}
	saveRule.write("Qt Bitcoin Trader Rules\n"+groupName.toAscii()+"==>"+rulesModel->saveRulesToString().toAscii());
	saveRule.close();
}