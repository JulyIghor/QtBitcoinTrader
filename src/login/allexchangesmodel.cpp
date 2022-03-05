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

#include "allexchangesmodel.h"
#include "main.h"

AllExchangesModel::AllExchangesModel() : QAbstractItemModel()
{
    rowsCount = 0;
    columnsCount = 2;

    headerList.append(julyTr("NAME", "Name"));
    headerList.append(julyTr("SUPPORTED_CUEEENCIRS", "Supported currencies"));
}

AllExchangesModel::~AllExchangesModel()
{
}

QModelIndex AllExchangesModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column);
}

QModelIndex AllExchangesModel::parent(const QModelIndex& /*child*/) const
{
    return QModelIndex();
}

int AllExchangesModel::rowCount(const QModelIndex& /*parent*/) const
{
    return rowsCount;
}

int AllExchangesModel::columnCount(const QModelIndex& /*parent*/) const
{
    return columnsCount;
}

QVariant AllExchangesModel::data(const QModelIndex& index, int role) const
{
    if (role == Qt::TextAlignmentRole)
    {
        return 0x0084;
    }

    if (role == Qt::DisplayRole)
    {
        QString unswer = "";

        if (index.column() == 0)
            unswer = nameList.at(index.row());
        else if (index.column() == 1)
            unswer = currenciesList.at(index.row());

        return unswer;
    }

    return QVariant();
}

QVariant AllExchangesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (headerList.size() != columnsCount)
        return QVariant();

    if (orientation == Qt::Horizontal)
        return headerList.at(section);

    return section + 1;
}

void AllExchangesModel::addExchange(quint32 id, QString name, QString currencies)
{
    beginInsertRows(QModelIndex(), 0, 0);
    rowsCount++;
    exchangeIdList.append(id);
    nameList.append(name);
    currenciesList.append(currencies);
    endInsertRows();
}

int AllExchangesModel::getExchangeId(int rowId)
{
    return exchangeIdList.at(rowId);
}
