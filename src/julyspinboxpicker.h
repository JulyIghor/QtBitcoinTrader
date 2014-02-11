// Copyright (C) 2014 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form http://qtopentrader.com
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#ifndef JULYSPINBOXPICKER_H
#define JULYSPINBOXPICKER_H

#include <QLabel>
class QDoubleSpinBox;
class QMouseEvent;

class JulySpinBoxPicker : public QLabel
{
	Q_OBJECT

public:
	JulySpinBoxPicker(QDoubleSpinBox *parent, double *forceMinimumValue=0, double internalMinimumValue=0);
	~JulySpinBoxPicker();

private:
	double *forceMinimumValue;
	double internalMinimumValue;
	double maximumValue;
	int scrollDirection;
	void setIcon(int);
	double minimumValue;
	bool j_debugMode;
	QRect currentScreenRect;
	QPoint j_cursorLastPos;
	QPoint j_cursorLastMove;
	bool j_isPressing;
	QPixmap j_simpleState;
	QPixmap j_pressedState;
	QPixmap j_disabledState;

	void mouseMoveEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	QDoubleSpinBox *parentSpinBox;
};

#endif // JULYSPINBOXPICKER_H
