// Copyright (C) 2014 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form http://qtopentrader.com
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#include "julytranslator.h"
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>
#include <QGroupBox>
#include <QDoubleSpinBox>
#include <QFile>
#include "main.h"

int JulyTranslator::loadFromFile(const QString &fileName)
{
	clearMaps();
	QFile loadFile(fileName);
	if(loadFile.open(QIODevice::ReadOnly))
	{
		fillMapsFromList(QString::fromUtf8(loadFile.readAll().replace("\r","")).split("\n"));
		loadFile.close();
		lastLangFile=fileName;
		emit languageChanged();
		return 0;
	}
	return 1;
}

void JulyTranslator::fillMapsFromList(const QStringList &list)
{
	for(int n=0;n<list.count();n++)
	{
		QString currentRow=list.at(n);
		if(currentRow.isEmpty()||!currentRow.at(0).isLetter())continue;
		if(fillMapsFromLine(&buttonMap,currentRow,"Button_"))
			if(fillMapsFromLine(&labelMap,currentRow,"Label_"))
				if(fillMapsFromLine(&checkBoxMap,currentRow,"CheckBox_"))
					if(fillMapsFromLine(&spinBoxMap,currentRow,"SpinBox_"))
						if(fillMapsFromLine(&stringMap,currentRow,"String_"))
							fillMapsFromLine(&groupBoxMap,currentRow,"GroupBox_");
	}
}

bool JulyTranslator::fillMapsFromLine(QMap<QString,QString> *map, QString line, const QString &prefix)
{
	if(!line.startsWith(prefix))return true;
	line.remove(0,prefix.length());
	int splitPos=line.indexOf('=');
	if(splitPos==-1||splitPos+1>=line.length())return true;
	QString currentTid=line.left(splitPos);
	line.remove(0,splitPos+1);
	(*map)[currentTid]=line;
	return false;
}

int JulyTranslator::saveToFile(const QString &fileName)
{
	QStringList resultList;
	resultList.append(getMapList(&buttonMap,"Button_"));
	resultList.append(getMapList(&labelMap,"Label_"));
	resultList.append(getMapList(&checkBoxMap,"CheckBox_"));
	resultList.append(getMapList(&groupBoxMap,"GroupBox_"));
	resultList.append(getMapList(&spinBoxMap,"SpinBox_"));
	resultList.append(getMapList(&stringMap,"String_"));
	if(resultList.isEmpty())return 1;
	resultList.sort();
	QFile writeFile(fileName);
	if(writeFile.open(QIODevice::WriteOnly|QIODevice::Truncate))
	{
		writeFile.write(QString(resultList.join("\r\n")+"\r\n").toUtf8());
		writeFile.close();
		return 0;
	}
	return 2;
}

QStringList JulyTranslator::getMapList(QMap<QString,QString> *map, QString prefix)
{
	QStringList mapTids=map->keys();
	for(int n=0;n<mapTids.count();n++)
	{
		mapTids[n]=prefix+mapTids.at(n)+"="+map->value(mapTids.at(n),"");
		mapTids[n].replace("\n","<br>");
		mapTids[n].replace("\r","");
		mapTids[n].replace("\t"," ");
	}
	return mapTids;
}

QString JulyTranslator::translateButton(const QString &tid, const QString &defaultText){return buttonMap.value(tid,defaultText);}
QString JulyTranslator::translateLabel(const QString &tid, const QString &defaultText){return labelMap.value(tid,defaultText);}
QString JulyTranslator::translateCheckBox(const QString &tid, const QString &defaultText){return checkBoxMap.value(tid,defaultText);}
QString JulyTranslator::translateGroupBox(const QString &tid, const QString &defaultText){return groupBoxMap.value(tid,defaultText);}
QString JulyTranslator::translateSpinBox(const QString &tid, const QString &defaultText){return spinBoxMap.value(tid,defaultText);}

QString JulyTranslator::translateString(const QString &tid, const QString &defaultText)
{
	QString result=stringMap.value(tid,defaultText);
	if(stringMap.values(tid).count()==0)stringMap[tid]=defaultText;
	return result;
}

void JulyTranslator::loadMapFromUi(QWidget *par)
{
	foreach(QPushButton* curButton, par->findChildren<QPushButton*>())
		if(!curButton->accessibleName().isEmpty())
			buttonMap[curButton->accessibleName()]=curButton->text().replace("\n","<br>").replace("\r","");

	foreach(QToolButton* curButton, par->findChildren<QToolButton*>())
		if(!curButton->accessibleName().isEmpty())
			buttonMap[curButton->accessibleName()]=curButton->text().replace("\n","<br>").replace("\r","");
		
	foreach(QCheckBox* curCheckBox, par->findChildren<QCheckBox*>())
		if(!curCheckBox->accessibleName().isEmpty())
			checkBoxMap[curCheckBox->accessibleName()]=curCheckBox->text().replace("\n","<br>").replace("\r","");

		foreach(QRadioButton* curCheckBox, par->findChildren<QRadioButton*>())
			if(!curCheckBox->accessibleName().isEmpty())
				checkBoxMap[curCheckBox->accessibleName()]=curCheckBox->text().replace("\n","<br>").replace("\r","");

	foreach(QLabel* curLabel, par->findChildren<QLabel*>())
		if(!curLabel->accessibleName().isEmpty())
			labelMap[curLabel->accessibleName()]=curLabel->text().replace("\n","<br>").replace("\r","");
		
	foreach(QGroupBox* curGroupBox, par->findChildren<QGroupBox*>())
		if(!curGroupBox->accessibleName().isEmpty())
			groupBoxMap[curGroupBox->accessibleName()]=curGroupBox->title().replace("\n","<br>").replace("\r","");

	foreach(QDoubleSpinBox* curSpinBox, par->findChildren<QDoubleSpinBox*>())
		if(!curSpinBox->accessibleName().isEmpty())
			spinBoxMap[curSpinBox->accessibleName()]=curSpinBox->suffix();
}

void JulyTranslator::translateUi(QWidget *par)
{
	foreach(QPushButton* curButton, par->findChildren<QPushButton*>())
		if(!curButton->accessibleName().isEmpty())
			curButton->setText(translateButton(curButton->accessibleName(),curButton->text()));

	foreach(QToolButton* curButton, par->findChildren<QToolButton*>())
		if(!curButton->accessibleName().isEmpty())
			curButton->setText(translateButton(curButton->accessibleName(),curButton->text()));

	foreach(QCheckBox* curCheckBox, par->findChildren<QCheckBox*>())
		if(!curCheckBox->accessibleName().isEmpty())
			curCheckBox->setText(translateCheckBox(curCheckBox->accessibleName(),curCheckBox->text()));

	foreach(QRadioButton* curCheckBox, par->findChildren<QRadioButton*>())
		if(!curCheckBox->accessibleName().isEmpty())
			curCheckBox->setText(translateCheckBox(curCheckBox->accessibleName(),curCheckBox->text()));

	foreach(QLabel* curLabel, par->findChildren<QLabel*>())
		if(!curLabel->accessibleName().isEmpty())
			curLabel->setText(translateLabel(curLabel->accessibleName(),curLabel->text()));

	foreach(QGroupBox* curGroupBox, par->findChildren<QGroupBox*>())
		if(!curGroupBox->accessibleName().isEmpty())
			curGroupBox->setTitle(translateGroupBox(curGroupBox->accessibleName(),curGroupBox->title()));

	foreach(QDoubleSpinBox* curSpinBox, par->findChildren<QDoubleSpinBox*>())
		if(!curSpinBox->accessibleName().isEmpty())
			curSpinBox->setSuffix(translateSpinBox(curSpinBox->accessibleName(),curSpinBox->suffix()));
}