// Copyright (C) 2013 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form https://sourceforge.net/projects/bitcointrader/
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#include "rulewidget.h"
#include "main.h"
#include "addrulewindow.h"
#include <QMessageBox>
#include <QtCore/qmath.h>

RuleWidget::RuleWidget(QString gName, RuleWidget *copyFrom)
	: QWidget()
{
	groupName=gName;
	ui.setupUi(this);
	setWindowTitle(groupName);
	setAttribute(Qt::WA_DeleteOnClose,true);

	ui.rulesNoMessage->setStyleSheet("border: 1px solid gray; background: "+baseValues.appTheme.white.name()+"; color: "+baseValues.appTheme.gray.name());

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
	connect(rulesModel,SIGNAL(rulesCountChanged()),baseValues.mainWindow_,SLOT(rulesCountChanged()));

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

	QString restorableString;

	if(copyFrom)restorableString=copyFrom->rulesModel->saveRulesToString();
	   else
	{
		mainWindow.iniSettings->beginGroup("Rules");
		restorableString=mainWindow.iniSettings->value(groupName,"").toString();
		mainWindow.iniSettings->endGroup();
	}

	QStringList settingsParams=restorableString.split(":");
	if(settingsParams.count()>1)
	{
		restorableString=settingsParams.first();
		settingsParams.removeFirst();
		ui.ruleBeep->setChecked(settingsParams.first().toInt()==1);
	}
	
	rulesModel->restoreRulesFromString(restorableString);

	saveRulesData();

	checkValidRulesButtons();
}

RuleWidget::~RuleWidget()
{
	if(!groupName.isEmpty())saveRulesData();
}

void RuleWidget::removeGroup()
{
	mainWindow.iniSettings->remove("Rules/"+groupName);
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
	mainWindow.iniSettings->setValue(groupName,rulesModel->saveRulesToString()+":"+QString::number((ui.ruleBeep->isChecked()?1:0)));
	mainWindow.iniSettings->endGroup();
	mainWindow.iniSettings->sync();
}

void RuleWidget::on_ruleAddButton_clicked()
{
	AddRuleWindow addRule(this);
	addRule.setWindowFlags(windowFlags());
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
		if(!mainWindow.isValidSoftLag){achievedHolderList.at(n)->startWaitingLowLag();continue;}

		double ruleBtc=achievedHolderList.at(n)->getRuleBtc();
		bool isBuying=achievedHolderList.at(n)->isBuying();
		double priceToExec=achievedHolderList.at(n)->getRulePrice();

		if(ruleBtc<0)
		{
			if(ruleBtc==-1.0)ruleBtc=mainWindow.getAvailableBTC();else//"Sell All my BTC"
				if(ruleBtc==-2.0)ruleBtc=mainWindow.getAvailableBTC()/2.0;else//"Sell Half my BTC"
					if(ruleBtc==-3.0)ruleBtc=mainWindow.getAvailableUSDtoBTC(priceToExec);else//"Spend All my Funds"
						if(ruleBtc==-4.0)ruleBtc=mainWindow.getAvailableUSDtoBTC(priceToExec)/2.0;else//"Spend Half my Funds"
							if(ruleBtc==-5.0)//"Cancel All Orders"
							{
								if(ui.ruleConcurrentMode->isChecked())
								{
									mainWindow.ordersCancelAll();
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
										static QTime ordersCancelTime(1,0,0,0);
										if(ordersCancelTime.elapsed()>5000)mainWindow.ordersCancelAll();
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
		}

		if(priceToExec<0)
		{
			if(priceToExec==-1.0)priceToExec=mainWindow.ui.marketLast->value();
			if(priceToExec==-2.0)priceToExec=mainWindow.ui.marketBuy->value();
			if(priceToExec==-3.0)priceToExec=mainWindow.ui.marketSell->value();
			if(priceToExec==-4.0)priceToExec=mainWindow.ui.marketHigh->value();
			if(priceToExec==-5.0)priceToExec=mainWindow.ui.marketLow->value();
			if(priceToExec==-6.0)priceToExec=mainWindow.ui.ordersLastBuyPrice->value();
			if(priceToExec==-7.0)priceToExec=mainWindow.ui.ordersLastSellPrice->value();
		}			

		if(ruleBtc>=baseValues.currentPair.tradeVolumeMin)
		{
			if(isBuying)mainWindow.apiBuySend(ruleBtc,priceToExec);
			else mainWindow.apiSellSend(ruleBtc,priceToExec);
		}
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