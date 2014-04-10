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

#include "newpassworddialog.h"
#include "main.h"
#include <QDesktopServices>
#include <QUrl>
#include <QFile>
#include <QMessageBox>
#include <QSettings>

NewPasswordDialog::NewPasswordDialog()
	: QDialog()
{
	ui.setupUi(this);
	setWindowTitle(windowTitle()+" v"+baseValues.appVerStr);
	ui.okButton->setEnabled(false);
	QSize minSizeHint=minimumSizeHint();
	if(mainWindow.isValidSize(&minSizeHint))setFixedSize(minimumSizeHint());
	setWindowFlags(Qt::WindowCloseButtonHint);

	julyTranslator.translateUi(this);

	QSettings listSettings(":/Resources/Exchanges/List.ini",QSettings::IniFormat);
	QStringList exchangesList=listSettings.childGroups();
	for(int n=0;n<exchangesList.count();n++)
	{
		QString currentName=listSettings.value(exchangesList.at(n)+"/Name").toString();
		QString currentLogo=listSettings.value(exchangesList.at(n)+"/Logo").toString();
		bool currentClientIdEnabled=listSettings.value(exchangesList.at(n)+"/ClientID",false).toBool();
		QString currentGetApiUrl=listSettings.value(exchangesList.at(n)+"/GetApiUrl").toString();
		if(currentName.isEmpty()||currentLogo.isEmpty())continue;
		clientIdVisibleMap.insert(n,currentClientIdEnabled);
		getApiUrlMap.insert(n,currentGetApiUrl);
		ui.exchangeComboBox->addItem(QIcon(":/Resources/Exchanges/Logos/"+currentLogo),currentName,currentClientIdEnabled);
	}
	if(ui.exchangeComboBox->count()<4)return;
	if(QLocale().name().startsWith("zh"))ui.exchangeComboBox->setCurrentIndex(3);
	else
	exchangeChanged(ui.exchangeComboBox->currentText());
}

NewPasswordDialog::~NewPasswordDialog()
{

}

void NewPasswordDialog::exchangeChanged(QString name)
{
	ui.groupBoxApiKeyAndSecret->setTitle(julyTr("API_KEY_AND_SECRET","%1 API key and Secret").arg(name));

	if(ui.exchangeComboBox->itemData(ui.exchangeComboBox->currentIndex()).toBool())
	{
		ui.labelClientID->setVisible(true);
		ui.clientIdLine->setVisible(true);
		ui.clearClientIdLine->setVisible(true);
	}
	else
	{
		ui.labelClientID->setVisible(false);
		ui.clientIdLine->setVisible(false);
		ui.clearClientIdLine->setVisible(false);
	}
}

QString NewPasswordDialog::getPassword()
{
	return ui.passwordEdit->text();
}

QString NewPasswordDialog::getRestSign()
{
	return ui.restSignLine->text().remove(QRegExp("[^a-zA-Z0-9-+/=\\d]"));
}

QString NewPasswordDialog::getRestKey()
{
	if(ui.exchangeComboBox->itemData(ui.exchangeComboBox->currentIndex()).toBool())
		return ui.clientIdLine->text().remove(QRegExp("[^a-zA-Z0-9-+/=\\d]")) + ":" + ui.restKeyLine->text().remove(QRegExp("[^a-zA-Z0-9-+/=\\d]")); //ClientID visible
	return ui.restKeyLine->text().remove(QRegExp("[^a-zA-Z0-9-+/=\\d]"));
}

void NewPasswordDialog::getApiKeySecretButton()
{
	QDesktopServices::openUrl(QUrl(getApiUrlMap.value(ui.exchangeComboBox->currentIndex())));
}

int NewPasswordDialog::getExchangeId()
{
	return ui.exchangeComboBox->currentIndex()+1;
}

void NewPasswordDialog::checkToEnableButton()
{
	if(ui.passwordEdit->text()!=ui.confirmEdit->text()){ui.confirmLabel->setStyleSheet("color: red;");ui.okButton->setEnabled(false);return;}
	else ui.confirmLabel->setStyleSheet("");

	QString profileName=ui.profileNameEdit->text();
	if(!profileName.isEmpty())
	{
		static QString allowedNameChars="()+,-.;=@[]^_`{}~ ";
		QString allowedPName;
		for(int n=0;n<profileName.length();n++)
			if(profileName.at(n).isLetterOrNumber()||allowedNameChars.contains(profileName.at(n)))
				allowedPName.append(profileName.at(n));

		if(profileName!=allowedPName)ui.profileNameEdit->setText(allowedPName);
	}
	if(profileName.isEmpty())
	{
		ui.okButton->setEnabled(false);
		return;
	}

	if(ui.restSignLine->text().isEmpty()||ui.restKeyLine->text().isEmpty()||ui.clientIdLine->isVisible()&&ui.clientIdLine->text().isEmpty())
		{ui.okButton->setEnabled(false);return;}

	ui.okButton->setEnabled(true);
}

bool NewPasswordDialog::isValidPassword()
{
	QString pass=ui.passwordEdit->text();
	if(pass.length()<8)return false;
	if(pass!=ui.confirmEdit->text())return false;
	ui.confirmLabel->setStyleSheet("");

	static QString allowedPassChars="!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";
	bool isValidPassword=pass.length()>14;
	if(!isValidPassword)
	{
		bool containsLetter=false;
		bool containsDigit=false;
		bool containsSpec=false;
		bool containsUpCase=false;
		bool containsDownCase=false;
		for(int n=0;n<pass.length();n++)
		{
			if(!containsLetter&&pass.at(n).isLetter())containsLetter=true;
			if(!containsDigit&&pass.at(n).isDigit())containsDigit=true;
			if(!containsSpec&&allowedPassChars.contains(pass.at(n)))containsSpec=true;
			if(!containsUpCase&&pass.at(n).isLetter()&&pass.at(n).isUpper())containsUpCase=true;
			if(!containsDownCase&&pass.at(n).isLetter()&&pass.at(n).isLower())containsDownCase=true;
			if(containsLetter&&containsDigit&&containsSpec||containsLetter&&containsDigit&&containsUpCase&&containsDownCase)
			{isValidPassword=true;break;}
		}
	}
	return isValidPassword;
}

void NewPasswordDialog::updateIniFileName()
{
	if(ui.profileNameEdit->text().isEmpty())
	baseValues.iniFileName=appDataDir+"QtBitcoinTrader.ini";
	else
	{
		baseValues.iniFileName=ui.exchangeComboBox->currentText()+"_"+ui.profileNameEdit->text().toAscii()+".ini";
		baseValues.iniFileName.replace(' ','_');
		baseValues.iniFileName.prepend(appDataDir);
	}

	QSettings settings(appDataDir+"/QtBitcoinTrader.cfg",QSettings::IniFormat);
	settings.setValue("LastProfile",baseValues.iniFileName);
}

QString NewPasswordDialog::selectedProfileName()
{
	if(ui.profileNameEdit->text().isEmpty())return "Default Profile";
	return ui.profileNameEdit->text();
}

void NewPasswordDialog::okPressed()
{
	if(isValidPassword())accept();
	else
		QMessageBox::warning(this,"Qt Bitcoin Trader",julyTranslator.translateLabel("TR00100","Your password must be at least 8 characters and contain letters, digits, and special characters."));
}
