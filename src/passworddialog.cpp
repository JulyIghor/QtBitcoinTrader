#include "passworddialog.h"
#include "main.h"

#ifdef Q_OS_WIN
#include "qtwin.h"
#endif
#include <QDir>
#include <QSettings>
#include <QMessageBox>

PasswordDialog::PasswordDialog(QWidget *parent)
	: QDialog(parent)
{
	resetData=false;
	newProfile=false;
	ui.setupUi(this);
	setWindowTitle(windowTitle()+" v"+appVerStr);
	setFixedSize(minimumSizeHint());
	setWindowFlags(Qt::WindowCloseButtonHint);
	ui.okButton->setEnabled(false);
#ifdef Q_OS_WIN
	if(QtWin::isCompositionEnabled())
		QtWin::extendFrameIntoClientArea(this);
#endif
	setWindowIcon(QIcon(":/Resources/QtBitcoinTrader.png"));

	QStringList settingsList=QDir(appDataDir,"*.ini").entryList();
	for(int n=0;n<settingsList.count();n++)
		ui.profileComboBox->addItem(QSettings(appDataDir+settingsList.at(n),QSettings::IniFormat).value("ProfileName",QFileInfo(settingsList.at(n)).completeBaseName()).toString(),settingsList.at(n));
	if(ui.profileComboBox->count()==0)ui.profileComboBox->addItem("Default Profile");
}

PasswordDialog::~PasswordDialog()
{

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
	msgBox.setText("Are you sure to delete \""+ui.profileComboBox->currentText()+"\" profile?");
	msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	msgBox.setDefaultButton(QMessageBox::Yes);
	if(msgBox.exec()!=QMessageBox::Yes)return;

	resetData=true;
	accept();
}

void PasswordDialog::checkToEnableButton(QString pass)
{
	if(pass.length()<8){ui.okButton->setEnabled(pass.length()>=8);return;}

	static QString allowedChars="!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";
	bool containsLetter=false;
	bool containsDigit=false;
	bool containsSpec=false;
	for(int n=0;n<pass.length();n++)
	{
		if(!containsLetter&&pass.at(n).isLetter())containsLetter=true;
		if(!containsDigit&&pass.at(n).isDigit())containsDigit=true;
		if(!containsSpec&&allowedChars.contains(pass.at(n)))containsSpec=true;
		if(containsSpec&&containsDigit&&containsSpec)break;
	}
	ui.okButton->setEnabled(containsLetter&&containsDigit&&containsSpec);
}