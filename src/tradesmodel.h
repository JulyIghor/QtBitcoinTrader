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

#ifndef TRADESMODEL_H
#define TRADESMODEL_H

#include "tradesitem.h"
#include <QAbstractItemModel>
#include <QStringList>

class TradesModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    TradesModel();
    ~TradesModel();
    void updateTotalBTC();
    double getRowPrice(int);
    double getRowVolume(int);
    int getRowType(int);
    void clear();
    void removeDataOlderThen(qint64);
    void addNewTrades(QList<TradesItem>*);

    void setHorizontalHeaderLabels(QStringList list);

    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex& index) const;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex& index) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    int columnCount(const QModelIndex& parent = QModelIndex()) const;

private:
    double lastPrecentBids;
    qint64 lastRemoveDate;
    void removeFirst();
    QString textBid;
    QString textAsk;

    double lastPrice;
    int columnsCount;
    int dateWidth;
    int typeWidth;

    QStringList headerLabels;

    QList<TradesItem> itemsList;
signals:
    void precentBidsChanged(double);
    void trades10MinVolumeChanged(double);
    void addChartsTrades(QList<TradesItem>*);
    void addChartsData(QList<TradesItem>*);
};

#endif // TRADESMODEL_H
