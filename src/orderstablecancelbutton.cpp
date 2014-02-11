#include "orderstablecancelbutton.h"
#include <QPushButton>
#include <QDebug>
#include "main.h"
#include <QTableView>

OrdersTableCancelButton::OrdersTableCancelButton(QObject *parent)
	: QItemDelegate(parent)
{
	parentTable=dynamic_cast<QTableView *>(parent);
}

OrdersTableCancelButton::~OrdersTableCancelButton()
{

}

void OrdersTableCancelButton::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	if(parentTable&&!mainWindow.currentlyAddingOrders)
	{
		QPushButton* buttonCancel=dynamic_cast<QPushButton*>(parentTable->indexWidget(index));
		if(!buttonCancel)
		{
			QString oID=index.data(Qt::EditRole).toString();
			if(!oID.isEmpty())
			{
			buttonCancel=new QPushButton("X");
			buttonCancel->setFixedWidth(defaultHeightForRow);
			buttonCancel->setFixedHeight(defaultHeightForRow);
			connect(buttonCancel,SIGNAL(clicked()),baseValues.mainWindow_,SLOT(cancelOrderByXButton()));
			parentTable->setIndexWidget(index, buttonCancel);
			}
		}	
	}
	QItemDelegate::paint(painter, option, index);
}