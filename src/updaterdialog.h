// Copyright (C) 2013 July IGHOR.
// I want to create Bitcoin Trader application that can be configured for any rule and strategy.
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
#include <QHttp>

class UpdaterDialog : public QDialog
{
	Q_OBJECT

public:
	UpdaterDialog(QWidget *parent = 0);
	~UpdaterDialog();

private:
	void downloadError();
	QString updateVersion;
	QByteArray updateSignature;
	QString updateChangeLog;
	QString updateLink;
	
	int stateUpdate;
	QHttp *httpGet;
	Ui::UpdaterDialog ui;
private slots:
	void dataReadProgress(int done,int total);
	void buttonUpdate();
	void httpDone(bool);
};

#endif // UPDATERDIALOG_H
