#include "datafolderchusedialog.h"
#include "main.h"

DataFolderChuseDialog::DataFolderChuseDialog(QString systemPath, QString localPath)
	: QDialog()
{
	isPortable=false;
	ui.setupUi(this);
	setWindowFlags(Qt::WindowCloseButtonHint);
	ui.buttonUseSystemFolder->setText(julyTr("USE_SYSTEM_FOLDER","Store your data in system folder."));
	ui.buttonUseSystemFolder->setToolTip(systemPath.replace("/","\\"));
	ui.buttonUsePortableMode->setText(julyTr("USE_PORTABLE_MODE","Enable portable mode. Store your data in same folder as executable file."));
	ui.buttonUsePortableMode->setToolTip(localPath.replace("/","\\"));
	setFixedSize(minimumSizeHint().width()+40,qMax(minimumSizeHint().height(),150));
}

DataFolderChuseDialog::~DataFolderChuseDialog()
{

}

void DataFolderChuseDialog::on_buttonUsePortableMode_clicked()
{
	isPortable=true;
	accept();
}