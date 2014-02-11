// Copyright (C) 2014 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form http://qtopentrader.com
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#ifndef JULYLIGHTCHANGES_H
#define JULYLIGHTCHANGES_H

#include <QObject>
#include <QDoubleSpinBox>
#include <QTimer>

class JulyLightChanges : public QObject
{
	Q_OBJECT

public:
	JulyLightChanges(QDoubleSpinBox *parent);
	~JulyLightChanges();
private:
	double lastValue;
	QDoubleSpinBox *parentSpinBox;
	QTimer *changeTimer;
private slots:
	void changeTimerSlot();
public slots:
	void valueChanged(double);
	
};

#endif // JULYLIGHTCHANGES_H
