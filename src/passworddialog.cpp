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

#include "passworddialog.h"
#include "main.h"
#include <QDir>
#include <QSettings>
#include <QMessageBox>
#include <QDesktopServices>
#include <QCryptographicHash>

PasswordDialog::PasswordDialog(QWidget *parent)
	: QDialog(parent)
{
	resetData=false;
	newProfile=false;
	ui.setupUi(this);
    setWindowTitle(windowTitle()+" v"+baseValues.appVerStr);
	setWindowFlags(Qt::WindowCloseButtonHint);
	//ui.updateCheckBox->setStyleSheet("QCheckBox {background: qradialgradient(cx: 0.5, cy: 0.5, fx: 0.5, fy: 0.5, radius: 0.7, stop: 0 #fff, stop: 1 transparent)}");
	ui.okButton->setEnabled(false);

	QSettings settings(appDataDir+"/QtBitcoinTrader.cfg",QSettings::IniFormat);
	ui.updateCheckBox->setChecked(settings.value("CheckForUpdates",true).toBool());
	QString lastProfile=settings.value("LastProfile","").toString();
	int lastProfileIndex=-1;
	int firstUnlockedProfileIndex=-1;

	QMap<int,QString> logosMap;
	QSettings listSettings(":/Resources/Exchanges/List.ini",QSettings::IniFormat);
	QStringList exchangesList=listSettings.childGroups();
	for(int n=0;n<exchangesList.count();n++)
	{
		QString currentLogo=listSettings.value(exchangesList.at(n)+"/Logo").toString();
		if(currentLogo.isEmpty())continue;
		logosMap.insert(n+1,":/Resources/Exchanges/Logos/"+currentLogo);
	}

	QStringList settingsList=QDir(appDataDir,"*.ini").entryList();
	for(int n=0;n<settingsList.count();n++)
	{
		QSettings settIni(appDataDir+settingsList.at(n),QSettings::IniFormat);

		if(baseValues.appVerLastReal<1.0772)
		{
			QString cryptedData=settIni.value("CryptedData","").toString();
			if(!cryptedData.isEmpty())settIni.setValue("EncryptedData/ApiKeySign",cryptedData);

			QString profileNameOld=settIni.value("ProfileName","").toString();
			if(!profileNameOld.isEmpty())
			{
				settIni.remove("ProfileName");
				settIni.setValue("Profile/Name",profileNameOld);
			}
			QString profileExchangeIdOld=settIni.value("ExchangeId","").toString();
			if(!profileExchangeIdOld.isEmpty())
			{
				settIni.remove("ExchangeId");
				settIni.setValue("Profile/ExchangeId",profileExchangeIdOld);
			}
			settIni.sync();
		}

		if(settIni.value("EncryptedData/ApiKeySign","").toString().isEmpty())
		{
			QFile::remove(appDataDir+settingsList.at(n));
			continue;
		}
		int currentProfileExchangeId=settIni.value("Profile/ExchangeId",0).toInt();

		if(baseValues.appVerLastReal<1.0775)
		{
			if(currentProfileExchangeId==2)
			{
				QFile::remove(appDataDir+settingsList.at(n));
				static bool haveBitstampProfile=false;
				if(!haveBitstampProfile)
				{
					haveBitstampProfile=true;
					QMessageBox::warning(0,windowTitle(),"From now Bitstamp support API keys with permissions. To ensure the security of your Bitstamp account you must create new API keys and add a new profile of Bitstamp to the Qt Bitcoin Trader.");
				}
				continue;
			}
		}
		QString currentLogo=logosMap.value(currentProfileExchangeId);
		if(!QFile::exists(currentLogo))currentLogo=":/Resources/Exchanges/Logos/Unknown.png";
		ui.profileComboBox->addItem(QIcon(currentLogo),settIni.value("Profile/Name",QFileInfo(settingsList.at(n)).completeBaseName()).toString(),settingsList.at(n));
		bool isProfLocked=isProfileLocked(settingsList.at(n));

		if(!isProfLocked&&lastProfileIndex==-1&&lastProfile==settingsList.at(n))lastProfileIndex=n;
		if(firstUnlockedProfileIndex==-1&&!isProfLocked)firstUnlockedProfileIndex=n;
	}
	if(ui.profileComboBox->count()==0)ui.profileComboBox->addItem(julyTr("DEFAULT_PROFILE_NAME","Default Profile"));
	if(firstUnlockedProfileIndex!=-1&&lastProfileIndex==-1)lastProfileIndex=firstUnlockedProfileIndex;
	if(lastProfileIndex>-1)ui.profileComboBox->setCurrentIndex(lastProfileIndex);

	julyTranslator.translateUi(this);

	foreach(QCheckBox* checkBoxes, findChildren<QCheckBox*>())
		checkBoxes->setMinimumWidth(qMin(checkBoxes->maximumWidth(),textFontWidth(checkBoxes->text())+20));
	QSize minSizeHint=minimumSizeHint();
	if(mainWindow.isValidSize(&minSizeHint))setFixedSize(minimumSizeHint());
}

PasswordDialog::~PasswordDialog()
{
	QSettings settings(appDataDir+"/QtBitcoinTrader.cfg",QSettings::IniFormat);
	settings.setValue("CheckForUpdates",ui.updateCheckBox->isChecked());
}

QString PasswordDialog::lockFilePath(QString name)
{
	return QDesktopServices::storageLocation(QDesktopServices::TempLocation)+"/QtBitcoinTrader_lock_"+QString(QCryptographicHash::hash(appDataDir+"/"+QFileInfo(name).fileName().toAscii(),QCryptographicHash::Sha1).toHex());
}

bool PasswordDialog::isProfileLocked(QString name)
{
	QString lockFileP=lockFilePath(name);

#ifdef Q_OS_WIN
	if(QFile::exists(lockFileP))QFile::remove(lockFileP);
#endif
	return QFile::exists(lockFileP);
}

void PasswordDialog::accept()
{
	QSettings settings(appDataDir+"/QtBitcoinTrader.cfg",QSettings::IniFormat);
	int currIndex=ui.profileComboBox->currentIndex();
	if(currIndex>0)settings.setValue("LastProfile",ui.profileComboBox->itemData(currIndex).toString());
	QDialog::accept();
}

QString PasswordDialog::getIniFilePath()
{
	int currIndex=ui.profileComboBox->currentIndex();
	if(currIndex==-1)return appDataDir+"QtBitcoinTrader.ini";
	return appDataDir+ui.profileComboBox->itemData(currIndex).toString();
}

void PasswordDialog::addNewProfile()
{
	newProfile=true;
	accept();
}

QString PasswordDialog::getPassword()
{
	return ui.passwordEdit->text();
}

void PasswordDialog::resetDataSlot()
{
	QMessageBox msgBox(this);
	msgBox.setIcon(QMessageBox::Question);
	msgBox.setWindowTitle(windowTitle());
	msgBox.setText(julyTr("CONFIRM_DELETE_PROFILE","Are you sure to delete \"%1\" profile?").arg(ui.profileComboBox->currentText()));
	msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	msgBox.setDefaultButton(QMessageBox::Yes);
	msgBox.setButtonText(QMessageBox::Yes,julyTr("YES","Yes"));
	msgBox.setButtonText(QMessageBox::No,julyTr("NO","No"));
	if(msgBox.exec()!=QMessageBox::Yes)return;

	resetData=true;
	accept();
}

void PasswordDialog::checkToEnableButton(QString pass)
{
	if(pass.length()<8){ui.okButton->setEnabled(false);return;}

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
	ui.okButton->setEnabled(isValidPassword);
}
