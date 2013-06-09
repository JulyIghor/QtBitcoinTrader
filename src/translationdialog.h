// Copyright (C) 2013 July IGHOR.
// I want to create Bitcoin Trader application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form https://sourceforge.net/projects/bitcointrader/
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#ifndef TRANSLATIONDIALOG_H
#define TRANSLATIONDIALOG_H

#include <QDialog>
#include "translationline.h"
#include "ui_translationdialog.h"
#include <QResizeEvent>
#include "julytranslator.h"

class TranslationDialog : public QDialog
{
	Q_OBJECT

public:
	TranslationDialog(QWidget *parent = 0);
	~TranslationDialog();

private:
	void resizeEvent(QResizeEvent *event);
	QGridLayout *gridLayout;
	TranslationLine *authorAbout;
	void fillLayoutByMap(QMap<QString,QString>*, QString subName, QMap<QString,QString>* dMap);
	QList<TranslationLine*> lineEdits;
	QWidget fonWidget;
	Ui::TranslationDialog ui;
public slots:
	void deleteTranslationButton();
	void lineTextChanged();
	void fixLayout();
	void searchLang(QString);
	void applyButton();
	void saveAsButton();
};

#endif // TRANSLATIONDIALOG_H
