#ifndef ORDERSTABLECANCELBUTTON_H
#define ORDERSTABLECANCELBUTTON_H

#include <QItemDelegate>
class QTableView;

class OrdersTableCancelButton : public QItemDelegate
{
	Q_OBJECT

public:
	OrdersTableCancelButton(QObject *parent);
	~OrdersTableCancelButton();

private:
	QTableView* parentTable;
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // ORDERSTABLECANCELBUTTON_H
