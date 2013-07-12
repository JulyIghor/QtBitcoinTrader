// Copyright (C) 2013 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form https://sourceforge.net/projects/bitcointrader/
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#ifndef TRANSLATIONABOUT_H
#define TRANSLATIONABOUT_H

#include <QDialog>
#include "ui_translationabout.h"

class TranslationAbout : public QDialog
{
	Q_OBJECT

public:
	TranslationAbout(QWidget *parent = 0);
	~TranslationAbout();

private:
	Ui::TranslationAbout ui;
private slots:
	void createTranslation();
	void buttonCheckUpdates();
};

#endif // TRANSLATIONABOUT_H
