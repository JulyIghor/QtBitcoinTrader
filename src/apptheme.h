// Copyright (C) 2014 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form http://qtopentrader.com
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#ifndef APPTHEME_H
#define APPTHEME_H

#include <QString>
#include <QColor>
#include <QPalette>
#include <QStyle>

struct AppTheme
{
	AppTheme();
	void loadTheme(QString);
	QString styleSheet;
	QColor getColor(QString);
	QPalette palette;
	QColor swapColor(QColor color);
	QColor altRowColor;
	QColor gray;
	QColor lightGray;
	QColor red;
	QColor green;
	QColor blue;
	QColor lightRed;
	QColor lightGreen;
	QColor lightBlue;
	QColor darkRed;
	QColor darkGreen;
	QColor darkBlue;
	QColor lightRedGreen;
	QColor lightGreenBlue;
	QColor lightRedBlue;
	QColor darkRedBlue;
	QColor black;
	QColor white;
};

#endif // APPTHEME_H
