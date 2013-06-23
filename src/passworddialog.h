// Copyright (C) 2013 July IGHOR.
// I want to create Bitcoin Trader application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form https://sourceforge.net/projects/bitcointrader/
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#ifndef PASSWORDDIALOG_H
#define PASSWORDDIALOG_H

#include <QDialog>
#include "ui_passworddialog.h"

class PasswordDialog : public QDialog
{
	Q_OBJECT

public:
	QString lockFilePath(QString);
	bool isProfileLocked(QString);
	QString getPassword();
	bool resetData;
	bool newProfile;
	QString getIniFilePath();
	PasswordDialog(QWidget *parent = 0);
	~PasswordDialog();
private slots:
	void resetDataSlot();
	void addNewProfile();
	void checkToEnableButton(QString);
private: 
	void accept();
	Ui::PasswordDialog ui;
};

#endif // PASSWORDDIALOG_H
