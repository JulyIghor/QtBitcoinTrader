// Copyright (C) 2013 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form https://sourceforge.net/projects/bitcointrader/
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#ifndef UPDATERDIALOG_H
#define UPDATERDIALOG_H

#include <QDialog>
#include "ui_updaterdialog.h"
#include "julyhttp.h"
#include <QTimer>

class UpdaterDialog : public QDialog
{
	Q_OBJECT

public:
	UpdaterDialog(bool feedbackMessage);
	~UpdaterDialog();

private:
	bool feedbackMessage;
	QTimer *timeOutTimer;
	void downloadError();
	QString updateVersion;
	QByteArray updateSignature;
	QString updateChangeLog;
	QString updateLink;
	
	int stateUpdate;
	JulyHttp *httpGet;
	Ui::UpdaterDialog ui;
private slots:
	void invalidData(bool);
	void dataReceived(QByteArray,int);
	void copyDonateButton();
	void exitSlot();
	void dataProgress(int);
	void buttonUpdate();
};

#endif // UPDATERDIALOG_H
