// Copyright (C) 2014 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form http://qtopentrader.com
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#include "aboutdialog.h"
#include "main.h"
#include "translationdialog.h"
#include "donatepanel.h"

TranslationAbout::TranslationAbout(QWidget *par)
	: QDialog()
{
	ui.setupUi(this);
	setWindowFlags(Qt::WindowCloseButtonHint|par->windowFlags());
	setWindowModality(Qt::ApplicationModal);
	setAttribute(Qt::WA_DeleteOnClose,true);
	//setFixedSize(size());
	ui.aboutTextLabel->setStyleSheet("QLabel {color: "+baseValues.appTheme.black.name()+"; border: 1px solid "+baseValues.appTheme.gray.name()+"; background: "+baseValues.appTheme.white.name()+"; padding:6px}");
	ui.translationAuthor->setStyleSheet(ui.aboutTextLabel->styleSheet());
}

TranslationAbout::~TranslationAbout()
{
	if(baseValues.mainWindow_)mainWindow.addPopupDialog(-1);
}

void TranslationAbout::showWindow()
{
	ui.donateGroupBoxBottom->layout()->addWidget(new DonatePanel(this));

	julyTranslator.translateUi(this);
	ui.languageField->setText(julyTr("LANGUAGE_NAME","Invalid Language"));
	ui.translationAuthor->setText(julyTr("LANGUAGE_AUTHOR","Invalid About"));
	ui.aboutBitcoinTraderGroupBox->setTitle(julyTr("ABOUT_QT_BITCOIN_TRADER","About %1").arg(windowTitle()));
	ui.aboutTextLabel->setText(julyTr("ABOUT_QT_BITCOIN_TRADER_TEXT","Qt Bitcoin Trader is a free Open Source project<br>developed on C++ Qt and OpenSSL.<br>If you want to help make project better please donate.<br>Feel free to send me recommendations and fixes to: %1").arg("<a href=\"mailto:julyighor@gmail.com\">julyighor@gmail.com</a>"));
	if(baseValues.mainWindow_)mainWindow.addPopupDialog(1);
	show();
}

void TranslationAbout::createTranslation()
{
	accept();
	TranslationDialog *translationDialog=new TranslationDialog;
	translationDialog->setWindowFlags(windowFlags());
	translationDialog->show();
}

void TranslationAbout::buttonCheckUpdates()
{
	mainWindow.checkUpdate();
}