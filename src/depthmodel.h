// Copyright (C) 2013 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form https://sourceforge.net/projects/bitcointrader/
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#ifndef DEPTHMODEL_H
#define DEPTHMODEL_H

#include <QAbstractItemModel>
#include <QStringList>
#include "depthitem.h"

class DepthModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	int itemsCount(){return volumeList.count();}
	void calculateSize();
	void clear();
	void setHorizontalHeaderLabels(QStringList list);
	void setAsk(bool on){isAsk=on;};
	DepthModel(bool isAskData=true);
	~DepthModel();
	double rowPrice(int row);
	double rowVolume(int row);
	double rowSize(int row);

	QModelIndex index(int row, int column, const QModelIndex &parent=QModelIndex()) const;
	QModelIndex parent(const QModelIndex &index) const;

	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	Qt::ItemFlags flags(const QModelIndex &index) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;

	void depthUpdateOrders(QList<DepthItem> *items);
	void depthFirstOrder(double price, double volume);

private:
	void depthUpdateOrder(double price, double volume);
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
	QList<double> priceList;

protected:
};

#endif // DEPTHMODEL_H
