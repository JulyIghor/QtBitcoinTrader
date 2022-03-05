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

#ifndef DEPTHMODEL_H
#define DEPTHMODEL_H

#include "depthitem.h"
#include <QAbstractItemModel>
#include <QStringList>

class QComboBox;

class DepthModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    void reloadVisibleItems();
    void fixTitleWidths();
    int itemsCount()
    {
        return volumeList.size();
    }
    void calculateSize();
    void clear();
    void setHorizontalHeaderLabels(QStringList list);
    void setAsk(bool on)
    {
        isAsk = on;
    };
    explicit DepthModel(QComboBox* _groupComboBox, bool isAskData = true);
    ~DepthModel();
    double rowPrice(int row);
    double rowVolume(int row);
    double rowSize(int row);

    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex& index) const;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex& index) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    int columnCount(const QModelIndex& parent = QModelIndex()) const;

    void depthUpdateOrders(QList<DepthItem>* items);
    void depthFirstOrder(double price, double volume);

    double getVolumeByPrice(double, bool);
    double getPriceByVolume(double);
    void initGroupList(double price);

private slots:
    void delayedReloadVisibleItems();

private:
    void depthUpdateOrder(DepthItem item);
    QString chopLastZero(const QString& text);

    bool originalIsAsk;
    bool somethingChanged;
    double groupedPrice;
    double groupedVolume;

    int widthPrice;
    int widthVolume;
    int widthSize;

    int widthPriceTitle;
    int widthVolumeTitle;
    int widthSizeTitle;

    int columnsCount;
    QStringList headerLabels;
    bool isAsk;
    QList<double> volumeList;
    QList<double> sizeList;
    double& sizeListAt(int);
    double sizeListGet(int) const;
    void sizeListRemoveAt(int);
    QList<double> sizePriceList;
    QList<double> priceList;
    QStringList volumeListStr;
    QStringList sizeListStr;
    QStringList priceListStr;
    QList<int> directionList;
    QComboBox* groupComboBox;
};

#endif // DEPTHMODEL_H
