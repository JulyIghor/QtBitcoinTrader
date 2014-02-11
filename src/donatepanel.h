// Copyright (C) 2014 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form http://qtopentrader.com
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#ifndef DONATEPANEL_H
#define DONATEPANEL_H

#include <QWidget>
#include "ui_donatepanel.h"
#include <QMenu>

class DonatePanel : public QWidget
{
	Q_OBJECT

public:
	DonatePanel(QWidget *parent);
	~DonatePanel();

private:
	QWidget *parentWindow;
	QPoint getFixedPoint(QWidget *, QWidget *);
	Ui::DonatePanel ui;
private slots:
	void on_buttonBitcoin_clicked();
	void on_buttonLitecoin_clicked();
	void on_buttonWebMoney_clicked();
};

#endif // DONATEPANEL_H
