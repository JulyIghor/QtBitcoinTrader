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

#include "allexchangesdialog.h"
#include "main.h"
#include <QDesktopServices>
#include <QSettings>

AllExchangesDialog::AllExchangesDialog(int featuredExchangesNum) : QDialog()
{
    startIndex = 0;
    exchangeNum = -1;
    ui.setupUi(this);
    ui.okButton->setEnabled(false);
    setWindowFlags(Qt::WindowCloseButtonHint);
    setWindowTitle("Qt Bitcoin Trader v" + baseValues.appVerStr + " - " + julyTr("ALL_EXCHANGES", "All Exchanges"));

    if (featuredExchangesNum == -3)
        ui.backButton->hide();

    QSettings listSettings(":/Resources/Exchanges/List.ini", QSettings::IniFormat);
    QStringList exchangesList = listSettings.childGroups();

    allExchangesModel = new AllExchangesModel;
    ui.exchangesTableView->setModel(allExchangesModel);

    for (int n = startIndex; n < exchangesList.size(); n++)
    {
        QString currentName = listSettings.value(exchangesList.at(n) + "/Name").toString();

        if (currentName.isEmpty() || exchangesList.at(n) == "RSA2048Sign")
            continue;

        allExchangesModel->addExchange(exchangesList.at(n).toUInt(), currentName, loadCurrencies(currentName));
    }

    mainWindow.setColumnResizeMode(ui.exchangesTableView, 0, QHeaderView::ResizeToContents);
    mainWindow.setColumnResizeMode(ui.exchangesTableView, 1, QHeaderView::Stretch);

    connect(ui.exchangesTableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &AllExchangesDialog::selectionExchange);

    julyTranslator.translateUi(this);

    mainWindow.fixTableViews(this);

    ui.exchangesTableView->setMinimumHeight(ui.exchangesTableView->verticalHeader()->defaultSectionSize() * exchangesList.size());
}

AllExchangesDialog::~AllExchangesDialog()
{
    delete allExchangesModel;
}

void AllExchangesDialog::on_okButton_clicked()
{
    if (exchangeNum == 0)
    {
        QDesktopServices::openUrl(QUrl("https://qttrader.com/"));
        return;
    }

    accept();
}

void AllExchangesDialog::on_backButton_clicked()
{
    exchangeNum = -2;
    accept();
}

void AllExchangesDialog::on_exchangesTableView_doubleClicked()
{
    selectionExchange();

    if (exchangeNum == 0)
    {
        QDesktopServices::openUrl(QUrl("https://qttrader.com/"));
        return;
    }

    accept();
}

QString AllExchangesDialog::loadCurrencies(const QString& name)
{
    QString nameIni = name;
    nameIni.remove(" ");
    nameIni.remove("-");
    nameIni.remove(".");
    QSettings listSettings(":/Resources/Exchanges/" + nameIni + ".ini", QSettings::IniFormat);
    QStringList pairsList = listSettings.childGroups();
    QString currencies = "";
    QString currencies12;
    QString currencies1;
    QString currencies2;

    for (int n = 0; n < pairsList.size(); n++)
    {
        currencies12 = listSettings.value(pairsList.at(n) + "/Symbol").toString();
        int posSplitter = currencies12.indexOf('/');

        if (posSplitter == -1)
        {
            currencies1 = currencies12.left(3);
            currencies2 = currencies12.right(3);
        }
        else
        {
            currencies1 = currencies12.left(posSplitter);
            currencies2 = currencies12.right(currencies12.size() - posSplitter - 1);
        }

        if (!currencies.contains(currencies1))
            currencies += currencies1 + ", ";

        if (!currencies.contains(currencies2))
            currencies += currencies2 + ", ";
    }

    currencies.chop(2);
    return currencies;
}

void AllExchangesDialog::selectionExchange()
{
    QModelIndexList selectedRows = ui.exchangesTableView->selectionModel()->selectedRows();

    if (selectedRows.empty())
        return;

    int curRow = selectedRows.first().row();

    if (curRow < 0)
        return;

    exchangeNum = allExchangesModel->getExchangeId(curRow);
    ui.okButton->setEnabled(true);
}
