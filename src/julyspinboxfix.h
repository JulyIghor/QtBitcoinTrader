// Copyright (C) 2013 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form https://sourceforge.net/projects/bitcointrader/
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#ifndef JULYSPINBOXFIX_H
#define JULYSPINBOXFIX_H

#include <QObject>
#include <QDoubleSpinBox>
#include <QFontMetrics>

class JulySpinBoxFix : public QObject
{
	Q_OBJECT

public:
	JulySpinBoxFix(QDoubleSpinBox *parent, int minimumWidth=0);
	~JulySpinBoxFix();

private:
	int pMinimumWidth;
	int spinMargin;
	QDoubleSpinBox *parentSpinBox;
private slots:
	void valueChanged(QString);
};

#endif // JULYSPINBOXFIX_H
