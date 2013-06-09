// Copyright (C) 2013 July IGHOR.
// I want to create Bitcoin Trader application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form https://sourceforge.net/projects/bitcointrader/
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#ifndef TRANSLATIONLINE_H
#define TRANSLATIONLINE_H

#include <QTextEdit>
#include <QResizeEvent>
#include <QFocusEvent>

class TranslationLine : public QTextEdit
{
	Q_OBJECT

public:
	bool isChanged(){return toPlainText()!=defaultText;}
	QString getValidText();
	void setItemText(QString);
	void setDefaultText(QString defText);
	TranslationLine(QWidget *parent=0);
	~TranslationLine();
private:
	void focusInEvent(QFocusEvent *e);
	void focusOutEvent(QFocusEvent *e);
	QString defaultText;
	bool fixingSize;
	void resizeEvent(QResizeEvent *event);
	void fixSize();
signals:
	void lineTextChanged();
private slots:
	void textChangedSlot();	
};

#endif // TRANSLATIONLINE_H
