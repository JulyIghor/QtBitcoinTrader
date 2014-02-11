// Copyright (C) 2014 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form http://qtopentrader.com
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#ifndef PERCENTPICKER_H
#define PERCENTPICKER_H

#include <QMenu>
#include <QPushButton>
#include <QDoubleSpinBox>
#include "ui_percentpicker.h"
#include <QMouseEvent>

class PercentPicker : public QMenu
{
	Q_OBJECT

public:
	PercentPicker(QPushButton *button, QDoubleSpinBox *spinBox, double maxValue);
	~PercentPicker();
	void setValue(int val){ui.verticalSlider->setValue(val);}
private:
	void mouseReleaseEvent(QMouseEvent *);
	QDoubleSpinBox *spinBox;
	QPushButton *button;
	double maxValue;
	Ui::PercentPicker ui;
private slots:
	void on_percentTo1_clicked();
	void on_verticalSlider_valueChanged(int);
};

#endif // PERCENTPICKER_H
