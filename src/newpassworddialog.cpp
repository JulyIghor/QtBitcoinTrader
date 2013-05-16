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
	setFixedSize(minimumSizeHint());
	setWindowFlags(Qt::WindowCloseButtonHint);
#ifdef Q_OS_WIN
	if(QtWin::isCompositionEnabled())
	{
		QtWin::extendFrameIntoClientArea(this);
		setStyleSheet("QGroupBox {background: rgba(255,255,255,160); border: 1px solid gray;border-radius: 3px;margin-top: 7px;} QGroupBox:title {background: qradialgradient(cx: 0.5, cy: 0.5, fx: 0.5, fy: 0.5, radius: 0.7, stop: 0 #fff, stop: 1 transparent); border-radius: 2px; padding: 1 4px; top: -7; left: 7px;}");
	}
#endif

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
	if(ui.restSignLine->text().isEmpty()||ui.restKeyLine->text().isEmpty()){ui.okButton->setEnabled(false);return;}

	QString pass=ui.passwordEdit->text();
	if(pass.length()<8){ui.okButton->setEnabled(false);return;}
	if(pass!=ui.confirmEdit->text()){ui.confirmLabel->setStyleSheet("color: red;");ui.okButton->setEnabled(false);return;}
	ui.confirmLabel->setStyleSheet("");

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

QByteArray NewPasswordDialog::getSelectedCurrency()
{
	if(ui.currencyComboBox->currentIndex()==-1)return QByteArray("USD");
	return ui.currencyComboBox->itemData(ui.currencyComboBox->currentIndex()).toString().toAscii();
}

