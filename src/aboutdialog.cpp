// Copyright (C) 2013 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form https://sourceforge.net/projects/bitcointrader/
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#include "aboutdialog.h"
#include "main.h"
#include "translationdialog.h"

TranslationAbout::TranslationAbout(QWidget *par)
	: QDialog()
{
	ui.setupUi(this);
	setWindowFlags(Qt::WindowCloseButtonHint|par->windowFlags());
	setWindowModality(Qt::ApplicationModal);
	setAttribute(Qt::WA_DeleteOnClose,true);
	//setFixedSize(size());

	julyTranslator->translateUi(this);
	ui.languageField->setText(julyTr("LANGUAGE_NAME","Invalid Language"));
	ui.translationAuthor->setText(julyTr("LANGUAGE_AUTHOR","Invalid About"));
	ui.aboutBitcoinTraderGroupBox->setTitle(julyTr("ABOUT_QT_BITCOIN_TRADER","About %1").arg(windowTitle()));
	ui.aboutTextLabel->setText(julyTr("ABOUT_QT_BITCOIN_TRADER_TEXT","Qt Bitcoin Trader is a free Open Source project<br>developed on C++ Qt and OpenSSL.<br>If you want to help make project better please donate: %1<br>Feel free to send me recommendations and fixes to: %2").arg("<a href=\"bitcoin:1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc\">1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc</a>").arg("<a href=\"mailto:julyighor@gmail.com\">julyighor@gmail.com</a>"));
	if(mainWindow_)mainWindow.addPopupDialog(1);
}

TranslationAbout::~TranslationAbout()
{
	if(mainWindow_)mainWindow.addPopupDialog(-1);
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