#ifndef NEWPASSWORDDIALOG_H
#define NEWPASSWORDDIALOG_H

#include <QDialog>
#include "ui_newpassworddialog.h"

class NewPasswordDialog : public QDialog
{
	Q_OBJECT

public:
	QByteArray getSelectedCurrency();
	QString getRestSign();
	QString getRestKey();
	QString getPassword();
	NewPasswordDialog(QMap<QByteArray,QByteArray> *names, QMap<QByteArray,QByteArray> *signs);
	~NewPasswordDialog();

private:
	Ui::NewPasswordDialog ui;
private slots:
	void checkToEnableButton();
	void getApiKeySecretButton();
};

#endif // NEWPASSWORDDIALOG_H
