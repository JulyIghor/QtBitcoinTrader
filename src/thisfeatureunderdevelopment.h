// Copyright (C) 2014 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form http://qtopentrader.com
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#ifndef THISFEATUREUNDERDEVELOPMENT_H
#define THISFEATUREUNDERDEVELOPMENT_H

#include <QDialog>
#include "ui_thisfeatureunderdevelopment.h"

class ThisFeatureUnderDevelopment : public QDialog
{
	Q_OBJECT

public:
	ThisFeatureUnderDevelopment(QWidget *parent=0);
	~ThisFeatureUnderDevelopment();

private:
	Ui::ThisFeatureUnderDevelopment ui;
private slots:
	void themeChanged();
};

#endif // THISFEATUREUNDERDEVELOPMENT_H
