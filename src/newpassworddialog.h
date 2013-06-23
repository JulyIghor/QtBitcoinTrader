// Copyright (C) 2013 July IGHOR.
// I want to create Bitcoin Trader application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form https://sourceforge.net/projects/bitcointrader/
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#ifndef NEWPASSWORDDIALOG_H
#define NEWPASSWORDDIALOG_H

#include <QDialog>
#include "ui_newpassworddialog.h"

class NewPasswordDialog : public QDialog
{
	Q_OBJECT

public:
	int getExchangeId();
	QString selectedProfileName();
	void updateIniFileName();
	QString getRestSign();
	QString getRestKey();
	QString getPassword();
	NewPasswordDialog();
	~NewPasswordDialog();

private:
	Ui::NewPasswordDialog ui;
private slots:
	void exchangeChanged(QString);
	void checkToEnableButton();
	void getApiKeySecretButton();
};

#endif // NEWPASSWORDDIALOG_H
