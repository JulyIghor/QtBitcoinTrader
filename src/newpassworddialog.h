#ifndef NEWPASSWORDDIALOG_H
#define NEWPASSWORDDIALOG_H

#include <QDialog>
#include "ui_newpassworddialog.h"

class NewPasswordDialog : public QDialog
{
	Q_OBJECT

public:
	QString selectedProfileName();
	void updateIniFileName();
	QString getRestSign();
	QString getRestKey();
	QString getPassword();
	NewPasswordDialog();
	~NewPasswordDialog();

private:
	Ui::NewPasswordDialog ui;
private slots:
	void checkToEnableButton();
	void getApiKeySecretButton();
};

#endif // NEWPASSWORDDIALOG_H
