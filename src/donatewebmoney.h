// Copyright (C) 2013 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form https://sourceforge.net/projects/bitcointrader/
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#ifndef DONATEWEBMONEY_H
#define DONATEWEBMONEY_H

#include <QMenu>
#include "ui_donatewebmoney.h"

class DonateWebMoney : public QMenu
{
	Q_OBJECT

public:
	DonateWebMoney(QPushButton *parent);
	~DonateWebMoney();
public slots:
	void aboutToHideWindow();
private:
	Ui::DonateWebMoney ui;
private slots:
	void on_wmzCopy_clicked();
	void on_wmrCopy_clicked();
	void on_wmeCopy_clicked();
	void on_wmuCopy_clicked();

signals:
	void setCheckedButton(bool);
};

#endif // DONATEWEBMONEY_H
