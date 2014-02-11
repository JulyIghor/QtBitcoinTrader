// Copyright (C) 2014 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form http://qtopentrader.com
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#include "datafolderchusedialog.h"
#include "main.h"

DataFolderChuseDialog::DataFolderChuseDialog(QString systemPath, QString localPath)
	: QDialog()
{
	isPortable=false;
	ui.setupUi(this);
	setWindowFlags(Qt::WindowCloseButtonHint);
	ui.buttonUseSystemFolder->setText(julyTr("USE_SYSTEM_FOLDER","Store your data in system folder."));
	ui.buttonUseSystemFolder->setToolTip(systemPath.replace("/","\\"));
	ui.buttonUsePortableMode->setText(julyTr("USE_PORTABLE_MODE","Enable portable mode. Store your data in same folder as executable file."));
	ui.buttonUsePortableMode->setToolTip(localPath.replace("/","\\"));
	setFixedSize(minimumSizeHint().width()+40,qMax(minimumSizeHint().height(),150));
}

DataFolderChuseDialog::~DataFolderChuseDialog()
{

}

void DataFolderChuseDialog::on_buttonUsePortableMode_clicked()
{
	isPortable=true;
	accept();
}