// Copyright (C) 2013 July IGHOR.
// I want to create Bitcoin Trader application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form https://sourceforge.net/projects/bitcointrader/
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#include "newpassworddialog.h"
#include "main.h"
#ifdef Q_OS_WIN
#include "qtwin.h"
#endif
#include <QDesktopServices>
#include <QUrl>
#include <QFile>

NewPasswordDialog::NewPasswordDialog()
	: QDialog()
{
	ui.setupUi(this);
	setWindowTitle(windowTitle()+" v"+appVerStr);
	ui.okButton->setEnabled(false);
	QSize minSizeHint=minimumSizeHint();
	if(mainWindow.isValidSize(&minSizeHint))setFixedSize(minimumSizeHint());
	setWindowFlags(Qt::WindowCloseButtonHint);
#ifdef Q_OS_WIN
	if(QtWin::isCompositionEnabled())
		QtWin::extendFrameIntoClientArea(this);
#endif

#ifdef GENERATE_LANGUAGE_FILE
	julyTranslator->loadMapFromUi(this);
	julyTranslator->saveToFile("LanguageDefault.lng");
#endif

	julyTranslator->translateUi(this);
}

NewPasswordDialog::~NewPasswordDialog()
{

}

QString NewPasswordDialog::getPassword()
{
	return ui.passwordEdit->text();
}

QString NewPasswordDialog::getRestSign()
{
	return ui.restSignLine->text();
}

QString NewPasswordDialog::getRestKey()
{
	return ui.restKeyLine->text();
}

void NewPasswordDialog::getApiKeySecretButton()
{
	QDesktopServices::openUrl(QUrl("https://www.mtgox.com/security"));
}

void NewPasswordDialog::checkToEnableButton()
{
	QString profileName=ui.profileNameEdit->text();
	if(!profileName.isEmpty())
	{
		static QString allowedNameChars="()+,-.;=@[]^_`{}~ ";
		QString allowedPName;
		for(int n=0;n<profileName.length();n++)
			if(profileName.at(n).isLetterOrNumber()||allowedNameChars.contains(profileName.at(n)))
				allowedPName.append(profileName.at(n));

		if(profileName!=allowedPName)
		{
			ui.profileNameEdit->setText(allowedPName);
			return;
		}
	}

	if(ui.restSignLine->text().isEmpty()||ui.restKeyLine->text().isEmpty()){ui.okButton->setEnabled(false);return;}

	QString pass=ui.passwordEdit->text();
	if(pass.length()<8){ui.okButton->setEnabled(false);return;}
	if(pass!=ui.confirmEdit->text()){ui.confirmLabel->setStyleSheet("color: red;");ui.okButton->setEnabled(false);return;}
	ui.confirmLabel->setStyleSheet("");

	static QString allowedPassChars="!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";
	bool containsLetter=false;
	bool containsDigit=false;
	bool containsSpec=false;
	for(int n=0;n<pass.length();n++)
	{
		if(!containsLetter&&pass.at(n).isLetter())containsLetter=true;
		if(!containsDigit&&pass.at(n).isDigit())containsDigit=true;
		if(!containsSpec&&allowedPassChars.contains(pass.at(n)))containsSpec=true;
		if(containsSpec&&containsDigit&&containsSpec)break;
	}
	ui.okButton->setEnabled(containsLetter&&containsDigit&&containsSpec);
}

void NewPasswordDialog::updateIniFileName()
{
	if(ui.profileNameEdit->text().isEmpty())
	iniFileName=appDataDir+"QtBitcoinTrader.ini";
	else
	iniFileName=appDataDir+ui.profileNameEdit->text().toAscii()+".ini";
}

QString NewPasswordDialog::selectedProfileName()
{
	if(ui.profileNameEdit->text().isEmpty())return "Default Profile";
	return ui.profileNameEdit->text();
}