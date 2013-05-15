#include "passworddialog.h"

#ifdef Q_OS_WIN
#include "qtwin.h"
#endif

PasswordDialog::PasswordDialog(QWidget *parent)
	: QDialog(parent)
{
	resetData=false;
	ui.setupUi(this);
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

PasswordDialog::~PasswordDialog()
{

}

QString PasswordDialog::getPassword()
{
	return ui.passwordEdit->text();
}

void PasswordDialog::resetDataSlot()
{
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