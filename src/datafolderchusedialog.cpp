//  This file is part of Qt Bitcoin Trader
//      https://github.com/JulyIGHOR/QtBitcoinTrader
//  Copyright (C) 2013-2022 July Ighor <julyighor@gmail.com>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  In addition, as a special exception, the copyright holders give
//  permission to link the code of portions of this program with the
//  OpenSSL library under certain conditions as described in each
//  individual source file, and distribute linked combinations including
//  the two.
//
//  You must obey the GNU General Public License in all respects for all
//  of the code used other than OpenSSL. If you modify file(s) with this
//  exception, you may extend this exception to your version of the
//  file(s), but you are not obligated to do so. If you do not wish to do
//  so, delete this exception statement from your version. If you delete
//  this exception statement from all source files in the program, then
//  also delete it here.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "datafolderchusedialog.h"
#include "main.h"
#include <QStandardPaths>

DataFolderChuseDialog::DataFolderChuseDialog(QString systemPath, QString localPath) : QDialog()
{
    isPortable = false;
    ui.setupUi(this);
    setWindowFlags(Qt::WindowCloseButtonHint);

#ifdef Q_OS_WIN
    systemPath.replace('/', '\\');
    localPath.replace('/', '\\');
#else

    QString homeDir = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first();

    if (systemPath.startsWith(homeDir))
    {
        systemPath.remove(0, homeDir.size());
        systemPath.prepend("~");
    }

#endif

    ui.buttonUseSystemFolder->setText(julyTr("USE_SYSTEM_FOLDER", "Store your data in system folder") + "\n\n" + systemPath);
    ui.buttonUsePortableMode->setToolTip(systemPath);
    ui.buttonUsePortableMode->setText(
        julyTr("USE_PORTABLE_MODE", "Enable portable mode. Store your data in same folder as executable file"));
    ui.buttonUsePortableMode->setToolTip(localPath);
    setFixedSize(minimumSizeHint().width() + 40, qMax(minimumSizeHint().height(), 150));
}

DataFolderChuseDialog::~DataFolderChuseDialog()
{
}

void DataFolderChuseDialog::on_buttonUsePortableMode_clicked()
{
    isPortable = true;
    accept();
}
