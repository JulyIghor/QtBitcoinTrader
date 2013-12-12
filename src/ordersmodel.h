// Copyright (C) 2013 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form https://sourceforge.net/projects/bitcointrader/
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#ifndef ORDERSMODEL_H
#define ORDERSMODEL_H

#include <QAbstractItemModel>
#include <QStringList>
#include "orderitem.h"

class OrdersModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	quint32 getRowNum(int row);
	quint32 getRowDate(int row);
	int getRowType(int row);
	int getRowStatus(int row);
	double getRowPrice(int row);
	double getRowVolume(int row);
	double getRowTotal(int row);

	QMap<double,bool> currentAsksPrices;
	QMap<double,bool> currentBidsPrices;

	bool checkDuplicatedOID;
	void ordersCancelAll();
	void setOrderCanceled(QByteArray);

	void clear();

	OrdersModel();
	~OrdersModel();

	void ordersChanged(QList<OrderItem> *orders);

	void setHorizontalHeaderLabels(QStringList list);

	QModelIndex index(int row, int column, const QModelIndex &parent=QModelIndex()) const;
	QModelIndex parent(const QModelIndex &index) const;

	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	Qt::ItemFlags flags(const QModelIndex &index) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;

	QList<OrderItem> orders;

private:
	QHash<QByteArray,quint32> oidMapForCheckingDuplicates;
	QStringList textStatusList;
	QString textAsk;
	QString textBid;

	bool haveOrders;

	int countWidth;
	int columnsCount;
	int dateWidth;
	int typeWidth;
	int statusWidth;

	QStringList headerLabels;

	QList<QByteArray> oidList;

	QList<quint32> dateList;
	QStringList dateStrList;

	QList<bool> typesList;

	QList<int> statusList;

	QList<double> amountList;
	QStringList amountStrList;

	QList<double> priceList;
	QStringList priceStrList;

	QList<double> totalList;
	QStringList totalStrList;

	QStringList symbolList;

signals:
	void cancelOrder(QByteArray);
	void ordersIsAvailable();
	void volumeAmountChanged(double, double);
};

#endif // ORDERSMODEL_H
