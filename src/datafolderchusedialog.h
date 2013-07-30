#ifndef DATAFOLDERCHUSEDIALOG_H
#define DATAFOLDERCHUSEDIALOG_H

#include <QDialog>
#include "ui_datafolderchusedialog.h"

class DataFolderChuseDialog : public QDialog
{
	Q_OBJECT

public:
	bool isPortable;
	DataFolderChuseDialog(QString systemPath, QString localPath);
	~DataFolderChuseDialog();

private:
	Ui::DataFolderChuseDialog ui;
private slots:
	void on_buttonUsePortableMode_clicked();
};

#endif // DATAFOLDERCHUSEDIALOG_H
