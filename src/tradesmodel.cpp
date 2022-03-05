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

#include "tradesmodel.h"
#include "main.h"

TradesModel::TradesModel() : QAbstractItemModel()
{
    lastPrecentBids = 0.0;
    lastRemoveDate = 0;
    lastPrice = 0.0;
    columnsCount = 8;
    dateWidth = 100;
    typeWidth = 100;
}

TradesModel::~TradesModel()
{
}

void TradesModel::clear()
{
    if (itemsList.isEmpty())
        return;

    beginResetModel();
    lastPrice = 0.0;
    itemsList.clear();
    endResetModel();
}

int TradesModel::rowCount(const QModelIndex& /*parent*/) const
{
    return itemsList.size();
}

int TradesModel::columnCount(const QModelIndex& /*parent*/) const
{
    return columnsCount;
}

void TradesModel::removeFirst()
{
    if (itemsList.empty())
        return;

    itemsList.removeFirst();
}

void TradesModel::removeDataOlderThen(qint64 date)
{
    lastRemoveDate = date;

    if (itemsList.empty())
    {
        updateTotalBTC();
        return;
    }

    int removeUpToIndex = -1;

    for (int n = 0; n < itemsList.size(); n++)
    {
        if (date <= itemsList.at(n).date)
            break;

        removeUpToIndex = n;
    }

    if (removeUpToIndex == -1)
        return;

    beginRemoveRows(QModelIndex(), 0, removeUpToIndex);

    for (int n = 0; n <= removeUpToIndex; n++)
        itemsList.removeFirst();

    endRemoveRows();

    if (itemsList.empty())
        clear();

    updateTotalBTC();
}

QVariant TradesModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int currentRow = itemsList.size() - index.row() - 1;

    if (currentRow < 0 || currentRow >= itemsList.size())
        return QVariant();

    if (role == Qt::WhatsThisRole)
    {
        QString typeText;

        switch (itemsList.at(currentRow).orderType)
        {
        case -1:
            typeText = textBid;
            break;

        case 1:
            typeText = textAsk;
            break;
        }

        return itemsList.at(currentRow).dateStr + " " + baseValues.currentPair.currASign + itemsList.at(currentRow).amountStr + " " +
               typeText + " " + (itemsList.at(currentRow).direction == 1 ? upArrowStr : downArrowStr) + " " +
               baseValues.currentPair.currBSign + itemsList.at(currentRow).priceStr + " " + baseValues.currentPair.currBSign +
               itemsList.at(currentRow).totalStr;
    }

    if (role == Qt::StatusTipRole)
    {
        QString lineText;
        lineText += itemsList.at(currentRow).dateStr + "\t";
        lineText += baseValues.currentPair.currASign + itemsList.at(currentRow).amountStr;

        switch (itemsList.at(currentRow).orderType)
        {
        case -1:
            lineText += "\t" + textBid;
            break;

        case 1:
            lineText += "\t" + textAsk;
            break;
        }

        if (itemsList.at(currentRow).price > 0.0)
        {
            if (itemsList.at(currentRow).direction)
            {
                if (itemsList.at(currentRow).direction == 1)
                    lineText += "\t" + upArrowStr;
                else
                    lineText += "\t" + downArrowStr;
            }

            lineText += "\t" + baseValues.currentPair.currBSign + itemsList.at(currentRow).priceStr + "\t";
            lineText += baseValues.currentPair.currBSign + itemsList.at(currentRow).totalStr;
        }

        return lineText;
    }

    if (role != Qt::DisplayRole && role != Qt::ToolTipRole && role != Qt::ForegroundRole && role != Qt::BackgroundRole &&
        role != Qt::TextAlignmentRole)
        return QVariant();

    int indexColumn = index.column();

    if (role == Qt::TextAlignmentRole)
    {
        if (indexColumn == 1)
            return 0x0082;

        if (indexColumn == 2)
            return 0x0082;

        if (indexColumn == 5)
            return 0x0081;

        if (indexColumn == 6)
            return 0x0082;

        return 0x0084;
    }

    if (role == Qt::BackgroundRole)
    {
        return itemsList.at(currentRow).backGray ? baseValues.appTheme.altRowColor : QVariant();
    }

    if (role == Qt::ForegroundRole)
    {
        switch (indexColumn)
        {
        case 1:
            return baseValues.appTheme.gray;
            break;

        case 2:
            {
                double amount = itemsList.at(currentRow).amount;
                double smallValue = baseValues.currentPair.currAInfo.valueSmall;

                if (amount <= smallValue)
                    return baseValues.appTheme.gray;

                smallValue *= 10.0;

                if (amount <= smallValue)
                    return baseValues.appTheme.black;

                smallValue *= 10.0;

                if (amount <= smallValue)
                    return baseValues.appTheme.darkGreen;

                smallValue *= 10.0;

                if (amount <= smallValue)
                    return baseValues.appTheme.darkRedBlue;

                return baseValues.appTheme.red;
            }
            break;

        case 3:
            switch (itemsList.at(currentRow).orderType)
            {
            case -1:
                return baseValues.appTheme.blue;

            case 1:
                return baseValues.appTheme.red;

            default:
                return baseValues.appTheme.black;
            }

        default:
            break;
        }

        return baseValues.appTheme.black;
    }

    double requestedPrice = itemsList.at(currentRow).price;

    if (requestedPrice <= 0.0)
        return QVariant();

    switch (indexColumn)
    {
    case 1:
        {
            // Date
            if (role == Qt::ToolTipRole || itemsList.at(currentRow).displayFullDate)
                return itemsList.at(currentRow).dateStr;

            return itemsList.at(currentRow).timeStr;
            break;
        }

    case 2:
        {
            // Volume
            if (itemsList.at(currentRow).amount <= 0.0)
                return QVariant();

            if (role == Qt::ToolTipRole)
                return baseValues.currentPair.currASign + itemsList.at(currentRow).amountStr;

            return itemsList.at(currentRow).amountStr;
        }
        break;

    case 3:
        {
            // Type
            switch (itemsList.at(currentRow).orderType)
            {
            case -1:
                return textBid;

            case 1:
                return textAsk;

            default:
                return QVariant();
            }
        }
        break;

    case 4:
        {
            // Direction
            if (itemsList.at(currentRow).price <= 0.0)
                return QVariant();

            if (itemsList.at(currentRow).direction)
            {
                if (itemsList.at(currentRow).direction == 1)
                    return upArrowStr;
                return downArrowStr;
            }

            return QVariant();
        }
        break;

    case 5:
        {
            // Price
            if (itemsList.at(currentRow).price <= 0.0)
                return QVariant();

            if (role == Qt::ToolTipRole)
                return baseValues.currentPair.currBSign + itemsList.at(currentRow).priceStr;

            return itemsList.at(currentRow).priceStr;
        }
        break;

    case 6:
        {
            // Total
            if (itemsList.at(currentRow).price <= 0.0)
                return QVariant();

            if (role == Qt::ToolTipRole)
                return baseValues.currentPair.currBSign + itemsList.at(currentRow).totalStr;

            return itemsList.at(currentRow).totalStr;
        }

    default:
        break;
    }

    return QVariant();
}

QVariant TradesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal)
        return QVariant();

    if (role == Qt::TextAlignmentRole)
    {
        if (section == 2)
            return 0x0082;

        if (section == 5)
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

    switch (section)
    {
    case 2:
        return headerLabels.at(section) + " " + baseValues.currentPair.currASign;

    case 5:
    case 6:
        return headerLabels.at(section) + " " + baseValues.currentPair.currBSign;

    default:
        break;
    }

    return headerLabels.at(section);
}

void TradesModel::updateTotalBTC()
{
    double summ = 0.0;
    double bidsSumm = 0.0;

    for (int n = 0; n < itemsList.size(); n++)
    {
        summ += itemsList.at(n).amount;

        if (itemsList.at(n).orderType == -1)
            bidsSumm += itemsList.at(n).amount;
    }

    bidsSumm = 100.0 * bidsSumm / summ;

    if (bidsSumm != lastPrecentBids)
    {
        lastPrecentBids = bidsSumm;
        emit precentBidsChanged(lastPrecentBids);
    }

    emit trades10MinVolumeChanged(summ);
}

Qt::ItemFlags TradesModel::flags(const QModelIndex& /*index*/) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

void TradesModel::setHorizontalHeaderLabels(QStringList list)
{
    if (list.size() != columnsCount)
        return;

    textAsk = julyTr("ORDER_TYPE_ASK", "sell");
    textBid = julyTr("ORDER_TYPE_BID", "buy");
    dateWidth = qMax(qMax(textFontWidth(QDateTime(QDate(2000, 12, 30), QTime(23, 59, 59, 999)).toString(baseValues.dateTimeFormat)),
                          textFontWidth(QDateTime(QDate(2000, 12, 30), QTime(12, 59, 59, 999)).toString(baseValues.dateTimeFormat))),
                     textFontWidth(list.at(0))) +
                10;
    typeWidth = qMax(qMax(textFontWidth(textAsk), textFontWidth(textBid)), textFontWidth(list.at(2))) + 10;

    headerLabels = list;
    emit headerDataChanged(Qt::Horizontal, 0, columnsCount - 1);
    emit layoutChanged();
}

QModelIndex TradesModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column);
}

QModelIndex TradesModel::parent(const QModelIndex& /*child*/) const
{
    return QModelIndex();
}

void TradesModel::addNewTrades(QList<TradesItem>* newItems)
{
    QList<TradesItem> verifedItems;

    for (int n = 0; n < newItems->size(); n++)
    {
        if (newItems->at(n).date < 200 || newItems->at(n).symbol != baseValues.currentPair.symbol || newItems->at(n).date <= lastRemoveDate)
            continue;

        if (lastPrice > newItems->at(n).price)
            (*newItems)[n].direction = -1;

        if (lastPrice < newItems->at(n).price)
            (*newItems)[n].direction = 1;

        lastPrice = newItems->at(n).price;

        if (newItems->at(n).orderType == 0)
        {
            if (newItems->at(n).date > mainWindow.currencyChangedDate)
            {
                if (newItems->at(n).price < mainWindow.meridianPrice)
                    (*newItems)[n].orderType = 1;
                else
                    (*newItems)[n].orderType = -1;
            }
        }

        static bool backSwitcher = false;

        (*newItems)[n].backGray = backSwitcher;
        verifedItems << newItems->at(n);
        static QMap<QString, quint32> lastDateMap;

        if (lastDateMap.value(verifedItems.last().symbol, 0UL) <= verifedItems.last().date)
        {
            lastDateMap[verifedItems.last().symbol] = verifedItems.last().date;
            mainWindow.sendIndicatorEvent(verifedItems.last().symbol, QLatin1String("LastTrade"), verifedItems.last().amount);
        }

        backSwitcher = !backSwitcher;
    }

    if (!verifedItems.empty())
    {
        verifedItems[verifedItems.size() - 1].displayFullDate = true;
        beginInsertRows(QModelIndex(), 0, verifedItems.size() - 1);
        itemsList << verifedItems;
        endInsertRows();
    }

    // emit addChartsData(newItems);
    emit addChartsTrades(newItems);
}

double TradesModel::getRowPrice(int row)
{
    row = itemsList.size() - row - 1;

    if (row < 0 || row >= itemsList.size())
        return 0.0;

    return itemsList.at(row).price;
}

double TradesModel::getRowVolume(int row)
{
    row = itemsList.size() - row - 1;

    if (row < 0 || row >= itemsList.size())
        return 0.0;

    return itemsList.at(row).amount;
}

int TradesModel::getRowType(int row)
{
    row = itemsList.size() - row - 1;

    if (row < 0 || row >= itemsList.size())
        return true;

    return itemsList.at(row).orderType;
}
