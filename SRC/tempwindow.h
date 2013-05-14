//Created by July IGHOR
//Feel free to contact me: julyighor@gmail.com
//Bitcoin Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc

#ifndef TEMPWINDOW_H
#define TEMPWINDOW_H

#include <QDialog>
#include "ui_tempwindow.h"

class TempWindow : public QDialog
{
	Q_OBJECT

public:
	TempWindow(QWidget *parent = 0);
	~TempWindow();

private:
	Ui::TempWindow ui;
private slots:
	void copyAddress();
};

#endif // TEMPWINDOW_H
