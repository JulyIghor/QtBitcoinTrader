//Created by July IGHOR
//Feel free to contact me: julyighor@gmail.com
//Bitcoin Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc

#include "tempwindow.h"
#include "main.h"

TempWindow::TempWindow(QWidget *parent)
	: QDialog()
{
	ui.setupUi(this);
	setWindowFlags(Qt::WindowCloseButtonHint);
	setFixedSize(size());
	setStyleSheet("QLabel {color: black;} QDoubleSpinBox {background: white;} QTextEdit {background: white;}");
	setWindowIcon(QIcon(":/Resources/QtBitcoinTrader.png"));
}

TempWindow::~TempWindow()
{

}

void TempWindow::copyAddress()
{
	mainWindow_->copyDonateButton();
}