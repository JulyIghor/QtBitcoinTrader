// Copyright (C) 2014 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form http://qtopentrader.com
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#ifndef DONATEBTC_H
#define DONATEBTC_H

#include <QMenu>
#include "ui_donatebtc.h"
#include <QPushButton>
#include <QMouseEvent>

class DonateBTC : public QMenu
{
	Q_OBJECT

public:
	DonateBTC(QPushButton *button, bool btc);
	~DonateBTC();
public slots:
	void aboutToHideWindow();
private:
	void mouseReleaseEvent(QMouseEvent *event);
	bool isBtc;
	Ui::DonateBTC ui;
signals:
	void setCheckedButton(bool);
private slots:
	void on_copyDonateButton_clicked();
};

#endif // DONATEBTC_H
