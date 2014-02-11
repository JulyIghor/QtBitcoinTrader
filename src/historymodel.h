// Copyright (C) 2014 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form http://qtopentrader.com
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#ifndef HISTORYMODEL_H
#define HISTORYMODEL_H

#include <QAbstractItemModel>
#include <QStringList>
#include "historyitem.h"

class HistoryModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	HistoryModel();
	~HistoryModel();
	double getRowPrice(int);
	double getRowVolume(int);
	int getRowType(int);
	void clear();

	void historyChanged(QList<HistoryItem> *histList);

	void setHorizontalHeaderLabels(QStringList list);

	QModelIndex index(int row, int column, const QModelIndex &parent=QModelIndex()) const;
	QModelIndex parent(const QModelIndex &index) const;

	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	Qt::ItemFlags flags(const QModelIndex &index) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;
private:
	int dateWidth;
	int typeWidth;
	quint32 lastDate;
	int columnsCount;
	QStringList headerLabels;
	QStringList typesLabels;

	QList<HistoryItem> itemsList;
	//typesList; 0=General, 1=Buy, 2=Sell, 3=Widthdraw, 4=Found
signals:
	void accLastSellChanged(QByteArray,double);
	void accLastBuyChanged(QByteArray,double);
};

#endif // HISTORYMODEL_H
