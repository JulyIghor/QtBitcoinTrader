// Copyright (C) 2013 July IGHOR.
// I want to create Bitcoin Trader application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form https://sourceforge.net/projects/bitcointrader/
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#include "translationdialog.h"
#include "main.h"
#ifdef Q_OS_WIN
#include "qtwin.h"
#endif
#include <QMessageBox>
#include <QFileDialog>
#include <QDesktopServices>
#include <QTimer>

TranslationDialog::TranslationDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	ui.buttonSaveAs->setEnabled(false);
	setWindowFlags(Qt::Window);
	setAttribute(Qt::WA_DeleteOnClose,true);
	//setFixedSize(size());

#ifdef GENERATE_LANGUAGE_FILE
	julyTranslator->loadMapFromUi(this);
	julyTranslator->saveToFile("LanguageDefault.lng");
#endif

#ifdef Q_OS_WIN
	if(QtWin::isCompositionEnabled())QtWin::extendFrameIntoClientArea(this);
#endif

	julyTranslator->translateUi(this);

	ui.deleteTranslationButton->setEnabled(!julyTranslator->lastFile().startsWith(":/Resource"));

	ui.languageName->setText(julyTr("LANGUAGE_NAME","Invalid"));
	authorAbout=new TranslationLine;
	ui.authorLayout->addWidget(authorAbout);
	authorAbout->setItemText(julyTr("LANGUAGE_AUTHOR","Invalid"));

	gridLayout=new QGridLayout;
	fonWidget.setLayout(gridLayout);
	ui.scrollArea->setWidget(&fonWidget);

	JulyTranslator defaultTranslation;
	defaultTranslation.loadFromFile(":/Resources/Language/English.lng");

	fillLayoutByMap(&(julyTranslator->labelMap),"Label_",&(defaultTranslation.labelMap));
	fillLayoutByMap(&(julyTranslator->groupBoxMap),"GroupBox_",&(defaultTranslation.groupBoxMap));
	fillLayoutByMap(&(julyTranslator->checkBoxMap),"CheckBox_",&(defaultTranslation.checkBoxMap));
	fillLayoutByMap(&(julyTranslator->buttonMap),"Button_",&(defaultTranslation.buttonMap));
	fillLayoutByMap(&(julyTranslator->spinBoxMap),"SpinBox_",&(defaultTranslation.spinBoxMap));
	fillLayoutByMap(&(julyTranslator->stringMap),"String_",&(defaultTranslation.stringMap));

	setTabOrder(ui.languageName,authorAbout);

	int currentRow=0;
	QWidget *lastWidget=authorAbout;
	for(int n=0;n<lineEdits.count();n++)
		if(lineEdits[n]->isChanged()==false)
		{
			TranslationLine *nextWidget=lineEdits[n];
			gridLayout->addWidget(nextWidget, currentRow++, 0);
			setTabOrder(lastWidget,nextWidget);
			lastWidget=nextWidget;
		}

	for(int n=0;n<lineEdits.count();n++)
		if(lineEdits[n]->isChanged()==true)
		{
			TranslationLine *nextWidget=lineEdits[n];
			gridLayout->addWidget(nextWidget, currentRow++, 0);
			setTabOrder(lastWidget,nextWidget);
			lastWidget=nextWidget;
		}
		setTabOrder(lastWidget,ui.searchLineEdit);

        resize(640,640);
        fixLayout();
	if(mainWindow_)mainWindow.addPopupDialog(1);

    QTimer::singleShot(100,this,SLOT(fixLayout()));
}

TranslationDialog::~TranslationDialog()
{
	if(gridLayout)delete gridLayout;
	if(mainWindow_)mainWindow.addPopupDialog(-1);
}

void TranslationDialog::fixLayout()
{
	QSize minSizeHint=fonWidget.minimumSizeHint();
	if(mainWindow.isValidSize(&minSizeHint))fonWidget.setFixedHeight(fonWidget.minimumSizeHint().height());
}

void TranslationDialog::resizeEvent(QResizeEvent *event)
{
    event->accept();
	fixLayout();
}

void TranslationDialog::deleteTranslationButton()
{
	QMessageBox msgBox(this);
	msgBox.setIcon(QMessageBox::Question);
	msgBox.setWindowTitle(julyTr("MESSAGE_CONFIRM_DELETE_TRANSLATION","Please confirm removing file"));
	msgBox.setText(julyTr("MESSAGE_CONFIRM_DELETE_TRANSLATION_TEXT","Are you sure to delete translation file?"));
	msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	msgBox.setDefaultButton(QMessageBox::Yes);
	msgBox.setButtonText(QMessageBox::Yes,julyTr("YES","Yes"));
	msgBox.setButtonText(QMessageBox::No,julyTr("NO","No"));
	if(msgBox.exec()!=QMessageBox::Yes)return;
	if(QFile::exists(julyTranslator->lastFile()))QFile::remove(julyTranslator->lastFile());
	ui.deleteTranslationButton->setEnabled(QFile::exists(julyTranslator->lastFile()));
	mainWindow.reloadLanguageList();
	close();
}

void TranslationDialog::fillLayoutByMap(QMap<QString,QString>* cMap, QString subName, QMap<QString,QString>* dMap)
{
	QStringList currentIdList=dMap->keys();

	for(int n=0;n<currentIdList.count();n++)
	{
		if(currentIdList.at(n).startsWith("LANGUAGE_"))continue;
		TranslationLine *newEdit=new TranslationLine;
		newEdit->setToolTip(subName+currentIdList.at(n));
		newEdit->setDefaultText(dMap->value(currentIdList.at(n),""));
		newEdit->setItemText(cMap->value(currentIdList.at(n),""));
		connect(newEdit,SIGNAL(lineTextChanged()),this,SLOT(lineTextChanged()));
		lineEdits<<newEdit;
	}
}

void TranslationDialog::lineTextChanged()
{
	fixLayout();
	ui.buttonApply->setEnabled(true);
}

void TranslationDialog::applyButton()
{
	QStringList resultList;
	for(int n=0;n<lineEdits.count();n++)
	{
		QString curText=lineEdits.at(n)->getValidText();
		if(curText.isEmpty())
		{
			ui.buttonSaveAs->setEnabled(false);
			QMessageBox::warning(this,windowTitle(),julyTr("LANGUAGE_NOT_APPROVED","Please fill empty fields"));
			return;
		}
		resultList<<lineEdits.at(n)->toolTip()+"="+curText;
	}
	resultList<<"String_LANGUAGE_NAME="+ui.languageName->text();
	resultList<<"String_LANGUAGE_AUTHOR="+authorAbout->getValidText();
	QString localeName=locale().name();//if(localeName.contains("_"))localeName.split("_").first();
	resultList<<"String_LANGUAGE_LOCALE="+localeName;
	QFile writeFile(appDataDir+"Language/Custom.lng");
	writeFile.open(QIODevice::WriteOnly|QIODevice::Truncate);
	writeFile.write(resultList.join("\r\n").toUtf8());
	writeFile.close();

	if(mainWindow_)mainWindow.reloadLanguageList(appDataDir+"Language/Custom.lng");
	ui.buttonSaveAs->setEnabled(true);
	ui.buttonApply->setEnabled(false);
	ui.deleteTranslationButton->setEnabled(QFile::exists(appDataDir+"Language/Custom.lng"));
}

void TranslationDialog::saveAsButton()
{
	applyButton();
	if(ui.buttonSaveAs->isEnabled()==false)return;

	QString fileName=QFileDialog::getSaveFileName(this, julyTr("SAVE_TRANSLATION","Save Translation"),QDesktopServices::storageLocation(QDesktopServices::DesktopLocation)+"/"+ui.languageName->text().replace("/","_").replace("\\","").replace(":","").replace("?","")+".lng","(*.lng)");
	if(fileName.isEmpty())return;
	if(QFile::exists(fileName))QFile::remove(fileName);
	QFile::copy(julyTranslator->lastFile(),fileName);
}

void TranslationDialog::searchLang(QString filterText)
{
	if(filterText.isEmpty())
	{
		for(int n=0;n<lineEdits.count();n++)lineEdits[n]->setVisible(true);
	}
	else
	{
		QStringList langFilter=filterText.split(" ");
		for(int n=0;n<lineEdits.count();n++)
		{
			QString curText=lineEdits[n]->getValidText();
			bool containsText=true;
			for(int k=0;k<langFilter.count();k++)if(!curText.contains(langFilter.at(k),Qt::CaseInsensitive))containsText=false;
			lineEdits[n]->setVisible(containsText);
		}
	}
	fonWidget.setFixedHeight(fonWidget.minimumSizeHint().height());
}
