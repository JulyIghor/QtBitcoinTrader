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

#include "depthmodel.h"
#include "julymath.h"
#include "main.h"
#include <QTimer>

DepthModel::DepthModel(QComboBox* _groupComboBox, bool isAskData) : QAbstractItemModel(), groupComboBox(_groupComboBox)
{
    widthPriceTitle = 75;
    widthVolumeTitle = 75;
    widthSizeTitle = 75;

    groupedPrice = 0.0;
    groupedVolume = 0.0;
    widthPrice = 75;
    widthVolume = 75;
    widthSize = 75;
    somethingChanged = false;
    isAsk = isAskData;
    originalIsAsk = isAsk;
    columnsCount = 5;
}

DepthModel::~DepthModel()
{
}

double& DepthModel::sizeListAt(int row)
{
    if (!originalIsAsk)
        return sizeList[sizeList.size() - row - 1];

    return sizeList[row];
}

double DepthModel::sizeListGet(int row) const
{
    if (!originalIsAsk)
        return sizeList[sizeList.size() - row - 1];

    return sizeList[row];
}

void DepthModel::sizeListRemoveAt(int row)
{
    if (!originalIsAsk)
        sizeList.removeAt(sizeList.size() - row - 1);
    else
        sizeList.removeAt(row);
}

double DepthModel::getPriceByVolume(double amount)
{
    if (sizeList.empty())
        return 0.0;

    int outside = 1;
    int currentIndex = std::lower_bound(sizeList.begin(), sizeList.end(), amount) - sizeList.begin();

    if (currentIndex < 0)
        return 0.0;

    if (currentIndex >= sizeList.size())
    {
        currentIndex = sizeList.size() - 1;
        outside = -1;
    }

    if (!originalIsAsk)
        currentIndex = priceList.size() - currentIndex - 1;

    return priceList.at(currentIndex) * outside;
}

double DepthModel::getVolumeByPrice(double price, bool isAsk)
{
    if (priceList.empty())
        return 0.0;

    int currentIndex;
    int outside = 1;

    if (isAsk)
    {
        currentIndex = std::upper_bound(priceList.begin(), priceList.end(), price) - priceList.begin();
        --currentIndex;

        if (currentIndex < 0)
            return 0.0;

        if (currentIndex >= priceList.size() - 1 && price > priceList.last())
            outside = -1;
    }
    else
    {
        currentIndex = std::lower_bound(priceList.begin(), priceList.end(), price) - priceList.begin();

        if (currentIndex >= priceList.size())
            return 0.0;

        if (currentIndex == 0 && price < priceList[0])
            outside = -1;
    }

    return sizeListAt(currentIndex) * outside;
}

int DepthModel::rowCount(const QModelIndex& /*parent*/) const
{
    return priceList.size() + grouped;
}

int DepthModel::columnCount(const QModelIndex& /*parent*/) const
{
    return columnsCount;
}

QVariant DepthModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int currentRow = index.row();

    if (role == Qt::WhatsThisRole)
    {
        return baseValues.currentPair.currBSign + priceListStr.at(currentRow) + " " + baseValues.currentPair.currASign +
               volumeListStr.at(currentRow) + " " + baseValues.currentPair.currASign + sizeListStr.at(currentRow);
    }

    if (role != Qt::DisplayRole && role != Qt::ToolTipRole && role != Qt::StatusTipRole && role != Qt::ForegroundRole &&
        role != Qt::BackgroundRole && role != Qt::TextAlignmentRole)
        return QVariant();

    int indexColumn = index.column();

    if (isAsk)
        indexColumn = columnsCount - indexColumn - 1;

    if (role == Qt::TextAlignmentRole)
    {
        if (indexColumn == 0)
            return 0x0081;

        if (indexColumn == 1)
            return 0x0082;

        if (indexColumn == 3)
            return 0x0082;

        return 0x0084;
    }

    if (grouped && currentRow < 2)
    {
        if (role == Qt::ForegroundRole)
            return baseValues.appTheme.black;

        if (currentRow == 1 || groupedPrice == 0.0)
            return QVariant();

        QString firstRowText;

        switch (indexColumn)
        {
        case 0: // Price
            firstRowText = JulyMath::textFromDouble(groupedPrice);

            if (role == Qt::ToolTipRole)
                firstRowText.prepend(baseValues.currentPair.currBSign);

            break;

        case 1: // Volume
            firstRowText = JulyMath::textFromDouble(groupedVolume, baseValues.currentPair.currADecimals);

            if (role == Qt::ToolTipRole)
                firstRowText.prepend(baseValues.currentPair.currASign);

            break;
        }

        if (firstRowText.isEmpty())
            return QVariant();

        return firstRowText;
    }

    if (grouped)
        currentRow -= grouped;

    if (currentRow < 0 || currentRow >= priceList.size())
        return QVariant();

    if (!originalIsAsk)
        currentRow = priceList.size() - currentRow - 1;

    if (role == Qt::StatusTipRole)
    {
        QString direction;

        switch (directionList.at(currentRow))
        {
        case -1:
            direction = downArrowStr + "\t";
            break;

        case 1:
            direction = upArrowStr + "\t";
            break;
        }

        return baseValues.currentPair.currBSign + priceListStr.at(currentRow) + "\t" + baseValues.currentPair.currASign +
               volumeListStr.at(currentRow) + "\t" + direction + baseValues.currentPair.currASign + sizeListStr.at(currentRow);
    }

    if (role == Qt::ForegroundRole)
    {
        if (indexColumn == 1)
        {
            double volume = volumeList.at(currentRow);
            double smallValue = baseValues.currentPair.currAInfo.valueSmall;

            if (volume <= smallValue)
                return baseValues.appTheme.gray;

            smallValue *= 10.0;

            if (volume <= smallValue)
                return baseValues.appTheme.black;

            smallValue *= 10.0;

            if (volume <= smallValue)
                return baseValues.appTheme.darkGreen;

            smallValue *= 10.0;

            if (volume <= smallValue)
                return baseValues.appTheme.darkRedBlue;

            return baseValues.appTheme.red;
        }

        return baseValues.appTheme.black;
    }

    double requestedPrice = priceList.at(currentRow);

    if (requestedPrice <= 0.0)
        return QVariant();

    if (role == Qt::BackgroundRole)
    {
        if (originalIsAsk)
        {
            if (mainWindow.ordersModel->currentAsksPrices.value(requestedPrice, false))
                return baseValues.appTheme.lightGreen;
        }
        else
        {
            if (mainWindow.ordersModel->currentBidsPrices.value(requestedPrice, false))
                return baseValues.appTheme.lightGreen;
        }

        return QVariant();
    }

    QString returnText;

    switch (indexColumn)
    {
    case 0: // Price
        if (role == Qt::ToolTipRole)
            baseValues.currentPair.currBSign + priceListStr.at(currentRow);

        return priceListStr.at(currentRow);
        break;

    case 1:
        {
            // Volume
            if (volumeList.at(currentRow) <= 0.0)
                return QVariant();

            if (role == Qt::ToolTipRole)
                baseValues.currentPair.currASign + volumeListStr.at(currentRow);

            return volumeListStr.at(currentRow);
        }
        break;

    case 2:
        {
            // Direction
            switch (directionList.at(currentRow))
            {
            case -1:
                return downArrowStr;

            case 1:
                return upArrowStr;

            default:
                return QVariant();
            }
        }

    case 3:
        {
            // Size
            if (sizeListGet(currentRow) <= 0.0)
                return QVariant();

            if (role == Qt::ToolTipRole)
                baseValues.currentPair.currASign + sizeListStr.at(currentRow);

            return sizeListStr.at(currentRow);
        }
        break;

    default:
        break;
    }

    if (!returnText.isEmpty())
        return returnText;

    return QVariant();
}

void DepthModel::reloadVisibleItems()
{
    QTimer::singleShot(100, this, SLOT(delayedReloadVisibleItems()));
}

void DepthModel::delayedReloadVisibleItems()
{
    emit dataChanged(index(0, 0), index(priceList.size() - 1, columnsCount - 1));
}

void DepthModel::calculateSize()
{
    if (!somethingChanged)
        return;

    double maxPrice = 0.0;
    double maxVolume = 0.0;
    double maxTotal = 0.0;

    double totalSize = 0.0;
    double totalPrice = 0.0;

    if (originalIsAsk)
    {
        for (int n = 0; n < priceList.size(); n++)
        {
            int currentRow = n;

            if (!originalIsAsk)
                currentRow = priceList.size() - currentRow - 1;

            totalSize += volumeList.at(currentRow);
            totalPrice += volumeList.at(currentRow) * priceList.at(currentRow);

            sizeListAt(currentRow) = totalSize;
            sizePriceList[currentRow] = totalPrice;
            sizeListStr[currentRow] =
                JulyMath::textFromDouble(totalSize, qMin(baseValues.currentPair.currADecimals, baseValues.decimalsTotalOrderBook));

            maxPrice = qMax(maxPrice, priceList.at(currentRow));
            maxVolume = qMax(maxVolume, volumeList.at(currentRow));
            maxTotal = qMax(maxTotal, sizeListAt(currentRow));
        }
    }
    else
    {
        for (int n = priceList.size() - 1; n >= 0; n--)
        {
            int currentRow = n;

            if (originalIsAsk)
                currentRow = priceList.size() - currentRow - 1;

            totalSize += volumeList.at(currentRow);
            totalPrice += volumeList.at(currentRow) * priceList.at(currentRow);
            sizeListAt(currentRow) = totalSize;
            sizePriceList[currentRow] = totalPrice;
            sizeListStr[currentRow] =
                JulyMath::textFromDouble(totalSize, qMin(baseValues.currentPair.currADecimals, baseValues.decimalsTotalOrderBook));

            maxPrice = qMax(maxPrice, priceList.at(currentRow));
            maxVolume = qMax(maxVolume, volumeList.at(currentRow));
            maxTotal = qMax(maxTotal, sizeListAt(currentRow));
        }
    }

    widthPrice = 10 + textFontWidth(JulyMath::textFromDouble(maxPrice, baseValues.currentPair.priceDecimals));
    widthVolume = 10 + textFontWidth(JulyMath::textFromDouble(maxVolume, baseValues.currentPair.currADecimals));
    widthSize = 10 + textFontWidth(JulyMath::textFromDouble(maxTotal, baseValues.currentPair.currADecimals));

    widthPrice = qMax(widthPrice, widthPriceTitle);
    widthVolume = qMax(widthVolume, widthVolumeTitle);
    widthSize = qMax(widthSize, widthSizeTitle);

    int sizeColumn = 2;

    if (isAsk)
        sizeColumn = 1;

    emit dataChanged(index(0, sizeColumn), index(priceList.size() - 1, sizeColumn));
}

QModelIndex DepthModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column);
}

QModelIndex DepthModel::parent(const QModelIndex& /*child*/) const
{
    return QModelIndex();
}

void DepthModel::clear()
{
    if (priceList.isEmpty())
        return;

    beginResetModel();
    groupedPrice = 0.0;
    groupedVolume = 0.0;
    directionList.clear();
    priceList.clear();
    volumeList.clear();
    sizeList.clear();
    sizePriceList.clear();
    priceListStr.clear();
    volumeListStr.clear();
    sizeListStr.clear();
    endResetModel();
    somethingChanged = false;
}

Qt::ItemFlags DepthModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    if (grouped)
    {
        if (index.row() == 1 || (groupedPrice == 0.0 && priceList.isEmpty()))
            return Qt::NoItemFlags;
    }

    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

QVariant DepthModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    int indexColumn = section;

    if (isAsk)
        indexColumn = columnsCount - indexColumn - 1;

    if (orientation != Qt::Horizontal)
        return QVariant();

    if (role == Qt::TextAlignmentRole)
    {
        if (indexColumn == 0)
            return 0x0081;

        if (indexColumn == 1)
            return 0x0082;

        if (indexColumn == 2)
            return 0x0082;

        return 0x0084;
    }

    if (role == Qt::SizeHintRole)
    {
        switch (indexColumn)
        {
        case 0:
            return QSize(widthPrice, defaultHeightForRow); // Price

        case 1:
            return QSize(widthVolume, defaultHeightForRow); // Volume

        case 3:
            return QSize(widthSize, defaultHeightForRow); // Size
        }

        return QVariant();
    }

    if (role != Qt::DisplayRole)
        return QVariant();

    if (headerLabels.size() != columnsCount)
        return QVariant();

    switch (indexColumn)
    {
    case 0:
        return headerLabels.at(indexColumn) + QLatin1String(" ") + baseValues.currentPair.currBSign;

    case 1:
        return headerLabels.at(indexColumn) + QLatin1String(" ") + baseValues.currentPair.currASign;

    case 3:
        return headerLabels.at(indexColumn) + QLatin1String(" ") + baseValues.currentPair.currASign;
    }

    return headerLabels.at(indexColumn);
}

void DepthModel::fixTitleWidths()
{
    int curASize = textFontWidth(" " + baseValues.currentPair.currASign);
    int curBSize = textFontWidth(" " + baseValues.currentPair.currBSign);
    widthPriceTitle = textFontWidth(headerLabels.at(0)) + 20 + curBSize;
    widthVolumeTitle = textFontWidth(headerLabels.at(1)) + 20 + curASize;
    widthSizeTitle = textFontWidth(headerLabels.at(3)) + 20 + curASize;

    if (widthPriceTitle > widthPrice)
        widthPrice = widthPriceTitle;

    if (widthVolumeTitle > widthVolume)
        widthVolume = widthVolumeTitle;

    if (widthSizeTitle > widthSize)
        widthSize = widthSizeTitle;
}

void DepthModel::setHorizontalHeaderLabels(QStringList list)
{
    if (list.size() != columnsCount)
        return;

    headerLabels = list;
    fixTitleWidths();
    emit headerDataChanged(Qt::Horizontal, 0, columnsCount - 1);
}

void DepthModel::depthFirstOrder(double price, double volume)
{
    if (!grouped)
        return;

    if (price == groupedPrice && groupedVolume == volume)
        return;

    groupedPrice = price;
    groupedVolume = volume;

    if (isAsk)
        emit dataChanged(index(0, 3), index(0, 4));
    else
        emit dataChanged(index(0, 0), index(0, 1));
}

QString DepthModel::chopLastZero(const QString& text)
{
    if (text.isEmpty())
        return text;

    int i = text.size() - 1;

    for (; i >= 0; --i)
        if (text.at(i) != '0')
            break;

    if (i > 0)
        if (text.at(i) == '.')
            --i;

    return text.left(i + 1);
}

void DepthModel::initGroupList(double price)
{
    if (groupComboBox->count() > 2)
        return;

    int degree = 0;
    double minVal = 1E-8;
    price /= minVal;

    while (price >= 10)
    {
        ++degree;
        price /= 10;
    }

    if (price >= 5)
        price = 5;
    else if (price >= 2)
        price = 2;
    else
        price = 1;

    double lastStep = -1.0;
    int uiIndex = -1;
    groupComboBox->blockSignals(true);

    if (groupComboBox->count() == 2)
    {
        lastStep = groupComboBox->itemData(1).toDouble();
        groupComboBox->removeItem(1);
    }
    else if (groupComboBox->count() == 0)
        groupComboBox->addItem(julyTr("DONT_GROUP", "None"), double(0));

    for (int i = degree; i >= 0 && groupComboBox->count() < 12; --i)
    {
        for (int j = qRound(price); j > 0; --j)
        {
            double priceD = j * minVal;

            for (int k = 0; k < i; ++k)
                priceD *= 10;

            groupComboBox->insertItem(1, chopLastZero(QString::number(priceD, 'f', 8)), priceD);

            if (priceD == lastStep)
                uiIndex = groupComboBox->count() - 1;

            if (j == 5)
                j -= 2;
        }

        price = 5.0;
    }

    if (uiIndex > 0 && uiIndex < groupComboBox->count())
        groupComboBox->setCurrentIndex(uiIndex);

    groupComboBox->blockSignals(false);
    mainWindow.fixWidthComboBoxGroupByPrice();
}

void DepthModel::depthUpdateOrders(QList<DepthItem>* items)
{
    if (items == nullptr)
        return;

    bool somethingChangedBefore = somethingChanged;

    for (int n = 0; n < items->size(); n++)
        depthUpdateOrder(items->at(n));

    if (!somethingChangedBefore && somethingChanged && !items->empty())
        initGroupList(items->first().price);

    delete items;
    calculateSize();
}

void DepthModel::depthUpdateOrder(DepthItem item)
{
    double price = item.price;
    double volume = item.volume;

    if (price == 0.0)
        return;

    int currentIndex = std::lower_bound(priceList.begin(), priceList.end(), price) - priceList.begin();
    bool matchListRang = currentIndex > -1 && priceList.size() > currentIndex;

    if (volume == 0.0)
    {
        // Remove item
        if (matchListRang)
        {
            beginRemoveRows(QModelIndex(), currentIndex + grouped, currentIndex + grouped);
            directionList.removeAt(currentIndex);
            priceList.removeAt(currentIndex);
            volumeList.removeAt(currentIndex);
            sizeListRemoveAt(currentIndex);
            sizePriceList.removeAt(currentIndex);
            priceListStr.removeAt(currentIndex);
            volumeListStr.removeAt(currentIndex);
            sizeListStr.removeAt(currentIndex);
            endRemoveRows();
            somethingChanged = true;
        }

        return;
    }

    if (matchListRang && priceList.at(currentIndex) == price)
    {
        // Update
        if (volumeList.at(currentIndex) == volume)
            return;

        directionList[currentIndex] = volumeList.at(currentIndex) < volume ? 1 : -1;
        volumeList[currentIndex] = volume;
        sizeListAt(currentIndex) = 0.0;
        sizePriceList[currentIndex] = 0.0;
        priceListStr[currentIndex] = item.priceStr;
        volumeListStr[currentIndex] = item.volumeStr;
        sizeListStr[currentIndex] = "0.0";
        somethingChanged = true;
        emit dataChanged(index(currentIndex + grouped, 0), index(currentIndex + grouped, columnsCount - 1));
    }
    else
    {
        // Insert
        beginInsertRows(QModelIndex(), currentIndex + grouped, currentIndex + grouped);
        priceList.insert(currentIndex, price);
        volumeList.insert(currentIndex, volume);
        sizeList.insert(currentIndex, volume);
        sizePriceList.insert(currentIndex, volume * price);
        directionList.insert(currentIndex, 0);
        priceListStr.insert(currentIndex, item.priceStr);
        volumeListStr.insert(currentIndex, item.volumeStr);
        sizeListStr.insert(currentIndex, item.volumeStr);
        endInsertRows();
        somethingChanged = true;
    }
}

double DepthModel::rowPrice(int row)
{
    if (grouped && row < 2)
    {
        if (row == 0)
            return groupedPrice;

        return 0.0;
    }

    row -= grouped;

    if (!originalIsAsk)
        row = priceList.size() - row - 1;

    if (row < 0 || row >= priceList.size())
        return 0.0;

    return priceList.at(row);
}

double DepthModel::rowVolume(int row)
{
    if (grouped && row < 2)
    {
        if (row == 0)
            return groupedVolume;

        return 0.0;
    }

    row -= grouped;

    if (!originalIsAsk)
        row = priceList.size() - row - 1;

    if (row < 0 || row >= priceList.size())
        return 0.0;

    return volumeList.at(row);
}

double DepthModel::rowSize(int row)
{
    if (grouped && row < 2)
        return 0.0;

    row -= grouped;

    if (!originalIsAsk)
        row = priceList.size() - row - 1;

    if (row < 0 || row >= priceList.size())
        return 0.0;

    return sizeListAt(row);
}
