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

#include "historymodel.h"
#include "main.h"

HistoryModel::HistoryModel() : QAbstractItemModel()
{
    typeWidth = 75;
    columnsCount = 7;
    lastDate = 0;
    typesLabels << ""
                << "Bought"
                << "Sell"
                << "Buy"
                << "Fee"
                << "Deposit"
                << "Withdraw"; // 0=General, 1=Sell, 2=Buy, 3=Fee, 4=Deposit, 5=Withdraw
}

HistoryModel::~HistoryModel()
{
}

void HistoryModel::clear()
{
    if (itemsList.isEmpty())
        return;

    beginResetModel();
    lastDate = 0;
    itemsList.clear();
    endResetModel();
}

void HistoryModel::loadLastPrice()
{
    bool haveLastBuy = false;
    bool haveLastSell = false;

    for (int n = 0; n < itemsList.size(); n++)
        if (itemsList.at(n).symbol == baseValues.currentPair.symbol)
        {
            if (!haveLastSell && itemsList.at(n).type == 1)
            {
                emit accLastSellChanged(itemsList.at(n).symbol.right(3), itemsList.at(n).price);
                haveLastSell = true;
            }

            if (!haveLastBuy && itemsList.at(n).type == 2)
            {
                emit accLastBuyChanged(itemsList.at(n).symbol.right(3), itemsList.at(n).price);
                haveLastBuy = true;
            }

            if (haveLastSell && haveLastBuy)
                break;
        }
}

void HistoryModel::historyChanged(QList<HistoryItem>* histList)
{
    bool haveLastBuy = false;
    bool haveLastSell = false;

    for (int n = 0; n < histList->size(); n++)
        if (histList->at(n).symbol == baseValues.currentPair.symbol)
        {
            if (!haveLastSell && histList->at(n).type == 1)
            {
                emit accLastSellChanged(histList->at(n).symbol.right(3), histList->at(n).price);
                haveLastSell = true;
            }

            if (!haveLastBuy && histList->at(n).type == 2)
            {
                emit accLastBuyChanged(histList->at(n).symbol.right(3), histList->at(n).price);
                haveLastBuy = true;
            }

            if (haveLastSell && haveLastBuy)
                break;
        }

    while (!histList->empty() && histList->last().dateTimeInt <= lastDate)
        histList->removeLast();

    if (histList->empty())
    {
        delete histList;
        return;
    }

    beginInsertRows(QModelIndex(), 0, histList->size() - 1);

    if (!histList->empty() && !itemsList.empty())
    {
        (*histList)[histList->size() - 1].displayFullDate = histList->at(histList->size() - 1).dateInt != itemsList.last().dateInt;
    }

    if (itemsList.isEmpty() && !histList->empty())
        (*histList)[histList->size() - 1].displayFullDate = true;

    qint64 maxListDate = 0;

    for (int n = histList->size() - 1; n >= 0; n--)
    {
        if (maxListDate < histList->at(n).dateTimeInt)
            maxListDate = histList->at(n).dateTimeInt;

        if (n != histList->size() - 1)
            (*histList)[n].displayFullDate = histList->at(n).dateInt != histList->at(n + 1).dateInt;

        itemsList << histList->at(n);

        static QMap<QString, quint32> lastDateMap;

        if (lastDateMap.value(itemsList.last().symbol, 0UL) <= itemsList.last().dateInt)
        {
            lastDateMap[itemsList.last().symbol] = itemsList.last().dateInt;
            mainWindow.sendIndicatorEvent(
                itemsList.last().symbol + itemsList.last().currRequestSecond, QLatin1String("MyLastTrade"), itemsList.last().volume);
        }
    }

    delete histList;

    if (maxListDate > lastDate)
        lastDate = maxListDate;

    endInsertRows();
}

double HistoryModel::getRowPrice(int row)
{
    row = itemsList.size() - row - 1;

    if (row < 0 || row >= itemsList.size())
        return 0.0;

    return itemsList.at(row).price;
}

double HistoryModel::getRowVolume(int row)
{
    row = itemsList.size() - row - 1;

    if (row < 0 || row >= itemsList.size())
        return 0.0;

    return itemsList.at(row).volume;
}

int HistoryModel::getRowType(int row)
{
    row = itemsList.size() - row - 1;

    if (row < 0 || row >= itemsList.size())
        return 0.0;

    return itemsList.at(row).type;
}

int HistoryModel::rowCount(const QModelIndex& /*parent*/) const
{
    return itemsList.size();
}

int HistoryModel::columnCount(const QModelIndex& /*parent*/) const
{
    return columnsCount;
}

QVariant HistoryModel::data(const QModelIndex& index, int role) const
{
    int currentRow = itemsList.size() - index.row() - 1;

    if (currentRow < 0 || currentRow >= itemsList.size())
        return QVariant();

    if (role == Qt::WhatsThisRole)
    {
        return itemsList.at(currentRow).dateTimeStr + " " + typesLabels.at(itemsList.at(currentRow).type) + " " +
               itemsList.at(currentRow).priceStr + " " + itemsList.at(currentRow).totalStr;
    }

    if (role == Qt::StatusTipRole)
    {
        return itemsList.at(currentRow).dateTimeStr + "\t" + itemsList.at(currentRow).volumeStr + "\t" +
               typesLabels.at(itemsList.at(currentRow).type) + "\t" + itemsList.at(currentRow).priceStr + "\t" +
               itemsList.at(currentRow).totalStr;
    }

    if (role != Qt::DisplayRole && role != Qt::ToolTipRole && role != Qt::ForegroundRole && role != Qt::TextAlignmentRole)
        return QVariant();

    int indexColumn = index.column();

    if (role == Qt::TextAlignmentRole)
    {
        if (indexColumn == 1)
            return 0x0082;

        if (indexColumn == 2)
            return 0x0082;

        if (indexColumn == 4)
            return 0x0081;

        if (indexColumn == 5)
            return 0x0082;

        if (indexColumn == 6)
            return 0x0081;

        return 0x0084;
    }

    if (role == Qt::ForegroundRole)
    {
        switch (indexColumn)
        {
        case 1:
            return baseValues.appTheme.gray;
            break;

        case 3:
            switch (itemsList.at(currentRow).type)
            {
            case 1:
                return baseValues.appTheme.red;

            case 2:
                return baseValues.appTheme.blue;

            case 3:
                return baseValues.appTheme.darkGreen;

            case 4:
                return baseValues.appTheme.darkRed;

            case 5:
                return baseValues.appTheme.darkRedBlue;

            default:
                break;
            }

            break;

        case 6:
            return baseValues.appTheme.gray;
            break;

        default:
            break;
        }

        return baseValues.appTheme.black;
    }

    switch (indexColumn)
    {
    case 1:
        {
            // Date
            if (role == Qt::ToolTipRole || itemsList.at(currentRow).displayFullDate)
                return itemsList.at(currentRow).dateTimeStr; // DateTime

            return itemsList.at(currentRow).timeStr; // Time
        }

    case 2:
        return itemsList.at(currentRow).volumeStr; // Volume

    case 3:
        return typesLabels.at(itemsList.at(currentRow).type); // Type

    case 4:
        return itemsList.at(currentRow).priceStr; // Price

    case 5:
        return itemsList.at(currentRow).totalStr; // Total

    case 6:
        return itemsList.at(currentRow).description; // Description

    default:
        break;
    }

    return QVariant();
}

QVariant HistoryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal)
        return QVariant();

    if (role == Qt::TextAlignmentRole)
    {
        if (section == 2)
            return 0x0082;

        if (section == 4)
            return 0x0081;

        return 0x0084;
    }

    if (role == Qt::SizeHintRole)
    {
        switch (section)
        {
        case 1:
            return QSize(dateWidth, defaultHeightForRow); // Date

        case 3:
            return QSize(typeWidth, defaultHeightForRow); // Type
        }

        return QVariant();
    }

    if (role != Qt::DisplayRole)
        return QVariant();

    if (headerLabels.size() != columnsCount)
        return QVariant();

    return headerLabels.at(section);
}

Qt::ItemFlags HistoryModel::flags(const QModelIndex& /*index*/) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

void HistoryModel::setHorizontalHeaderLabels(QStringList list)
{
    if (list.size() != columnsCount)
        return;

    typesLabels[1] = julyTr("LOG_SOLD", "sell");
    typesLabels[2] = julyTr("LOG_BOUGHT", "buy");
    typesLabels[3] = julyTr("LOG_FEE", "fee");
    typesLabels[4] = julyTr("LOG_DEPOSIT", "deposit");
    typesLabels[5] = julyTr("LOG_WITHDRAW", "withdraw");

    typeWidth = 0;

    for (int n = 1; n < typesLabels.size(); n++)
        typeWidth = qMax(typeWidth, textFontWidth(typesLabels.at(n)));

    typeWidth = qMax(typeWidth, textFontWidth(list.at(2)));
    typeWidth += 10;

    dateWidth = qMax(qMax(textFontWidth(QDateTime(QDate(2000, 12, 30), QTime(23, 59, 59, 999)).toString(baseValues.dateTimeFormat)),
                          textFontWidth(QDateTime(QDate(2000, 12, 30), QTime(12, 59, 59, 999)).toString(baseValues.dateTimeFormat))),
                     textFontWidth(list.at(0))) +
                10;

    headerLabels = list;
    emit headerDataChanged(Qt::Horizontal, 0, columnsCount - 1);
}

QModelIndex HistoryModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column);
}

QModelIndex HistoryModel::parent(const QModelIndex& /*child*/) const
{
    return QModelIndex();
}
