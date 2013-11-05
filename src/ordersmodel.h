#ifndef ORDERSMODEL_H
#define ORDERSMODEL_H

#include <QAbstractItemModel>
#include <QStringList>
#include "orderitem.h"

class OrdersModel : public QAbstractItemModel
{
	Q_OBJECT

public:
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
	QHash<QByteArray,qint64> oidMapForCheckingDuplicates;
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
	QList<qint64> dateList;
	QList<bool> typesList;
	QList<int> statusList;
	QList<double> amountList;
	QList<double> priceList;
	QStringList symbolList;
	QStringList itemSignList;
	QStringList priceSignList;
signals:
	void cancelOrder(QByteArray);
	void ordersIsAvailable();
	void volumeAmountChanged(double, double);
};

#endif // ORDERSMODEL_H
