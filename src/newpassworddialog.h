#ifndef NEWPASSWORDDIALOG_H
#define NEWPASSWORDDIALOG_H

#include <QDialog>
#include "ui_newpassworddialog.h"

class NewPasswordDialog : public QDialog
{
	Q_OBJECT

public:
	QString getRestSign();
	QString getRestKey();
	QString getPassword();
	NewPasswordDialog(QWidget *parent = 0);
	~NewPasswordDialog();

private:
	Ui::NewPasswordDialog ui;
private slots:
	void checkToEnableButton();
	void getApiKeySecretButton();
};

#endif // NEWPASSWORDDIALOG_H
