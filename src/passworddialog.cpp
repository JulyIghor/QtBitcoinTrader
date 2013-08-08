// Copyright (C) 2013 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form https://sourceforge.net/projects/bitcointrader/
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

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
    setWindowTitle(windowTitle()+" v"+appVerStr);
	setWindowFlags(Qt::WindowCloseButtonHint);
	ui.updateCheckBox->setStyleSheet("QCheckBox {background: qradialgradient(cx: 0.5, cy: 0.5, fx: 0.5, fy: 0.5, radius: 0.7, stop: 0 #fff, stop: 1 transparent)}");
	ui.okButton->setEnabled(false);

	QSettings settings(appDataDir+"/Settings.set",QSettings::IniFormat);
	ui.updateCheckBox->setChecked(settings.value("CheckForUpdates",true).toBool());
	QString lastProfile=settings.value("LastProfile","").toString();
	int lastProfileIndex=-1;
	int firstUnlockedProfileIndex=-1;
	QStringList settingsList=QDir(appDataDir,"*.ini").entryList();
	for(int n=0;n<settingsList.count();n++)
	{
		QSettings settIni(appDataDir+settingsList.at(n),QSettings::IniFormat);
		if(settIni.value("CryptedData","").toByteArray().isEmpty())
		{
			QFile::remove(appDataDir+settingsList.at(n));//I'll add ini backup function here
			continue;
		}
		int currentProfileExchangeId=settIni.value("ExchangeId",0).toInt();
		QString itemIcon;
		if(currentProfileExchangeId==0)itemIcon=":/Resources/Exchanges/Mt.Gox.png";
		if(currentProfileExchangeId==1)itemIcon=":/Resources/Exchanges/BTC-e.png";
		ui.profileComboBox->addItem(QIcon(itemIcon),settIni.value("ProfileName",QFileInfo(settingsList.at(n)).completeBaseName()).toString(),settingsList.at(n));
		bool isProfLocked=isProfileLocked(settingsList.at(n));

		if(!isProfLocked&&lastProfileIndex==-1&&lastProfile==settingsList.at(n))lastProfileIndex=n;
		if(firstUnlockedProfileIndex==-1&&!isProfLocked)firstUnlockedProfileIndex=n;
	}
	if(ui.profileComboBox->count()==0)ui.profileComboBox->addItem(julyTr("DEFAULT_PROFILE_NAME","Default Profile"));
	if(firstUnlockedProfileIndex!=-1&&lastProfileIndex==-1)lastProfileIndex=firstUnlockedProfileIndex;
	if(lastProfileIndex>-1)ui.profileComboBox->setCurrentIndex(lastProfileIndex);

	julyTranslator->translateUi(this);

	foreach(QCheckBox* checkBoxes, findChildren<QCheckBox*>())
		checkBoxes->setMinimumWidth(qMin(checkBoxes->maximumWidth(),textWidth(checkBoxes->text())+20));
	QSize minSizeHint=minimumSizeHint();
	if(mainWindow.isValidSize(&minSizeHint))setFixedSize(minimumSizeHint());
}

PasswordDialog::~PasswordDialog()
{
	QSettings settings(appDataDir+"/Settings.set",QSettings::IniFormat);
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
	QSettings settings(appDataDir+"/Settings.set",QSettings::IniFormat);
	int currIndex=ui.profileComboBox->currentIndex();
	if(currIndex>-1)settings.setValue("LastProfile",ui.profileComboBox->itemData(currIndex).toString());
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
