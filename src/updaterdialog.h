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

#ifndef UPDATERDIALOG_H
#define UPDATERDIALOG_H

#include "julyhttp.h"
#include "ui_updaterdialog.h"
#include <QDialog>
#include <QTimer>

class UpdaterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UpdaterDialog(bool feedbackMessage);
    ~UpdaterDialog();

private:
    bool forceUpdate;
    QByteArray getMidData(const QString& a, QString b, QByteArray* data);
    bool downloaded100;
    bool feedbackMessage;
    QTimer* timeOutTimer;
    void downloadError(int);
    void downloadErrorFile(int);
    QString updateVersion;
    QByteArray updateSignature;
    QByteArray versionSignature;
    QString updateChangeLog;
    QString updateLink;

    int stateUpdate;
    bool autoUpdate;
    JulyHttp* httpGet;
    JulyHttp* httpGetFile;
    Ui::UpdaterDialog ui;
private slots:
    void invalidData(bool);
    void dataReceived(QByteArray, int, int);
    void exitSlot();
    void dataProgress(int);
    void buttonUpdate();
};

#endif // UPDATERDIALOG_H
