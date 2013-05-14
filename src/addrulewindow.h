//Created by July IGHOR
//Feel free to contact me: julyighor@gmail.com
//Bitcoin Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc

#ifndef ADDRULEWINDOW_H
#define ADDRULEWINDOW_H

#include <QDialog>
#include "ui_addrulewindow.h"

class AddRuleWindow : public QDialog
{
	Q_OBJECT

public:
	Ui::AddRuleWindow ui;
	AddRuleWindow(QWidget *parent = 0);
	~AddRuleWindow();
	QString getIfString();
	QString getGoesString();
	QString getPriceString();
	QString getSellBuyString();
	QString getBitcoinsString();
private:
public slots:
	void buttonAddRule();
};

#endif // ADDRULEWINDOW_H
