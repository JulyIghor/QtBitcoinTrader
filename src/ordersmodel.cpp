#include "ordersmodel.h"
#include "main.h"

OrdersModel::OrdersModel()
	: QAbstractItemModel()
{
	checkDuplicatedOID=false;
	haveOrders=false;
	columnsCount=7;
	dateWidth=100;
	typeWidth=100;
	countWidth=20;
	statusWidth=100;

	textStatusList<<"CANCELED"<<"OPEN"<<"PENDING"<<"POST-PENDING"<<"INVALID";
}

OrdersModel::~OrdersModel()
{

}

int OrdersModel::rowCount(const QModelIndex &) const
{
	return oidList.count();
}

int OrdersModel::columnCount(const QModelIndex &) const
{
	return columnsCount;
}

void OrdersModel::clear()
{
	if(oidList.count()==0)return;

	beginResetModel();
	oidList.clear();
	dateList.clear();
	typesList.clear();
	statusList.clear();
	amountList.clear();
	priceList.clear();
	symbolList.clear();

	itemSignList.clear();
	priceSignList.clear();
	haveOrders=false;
	if(checkDuplicatedOID)oidMapForCheckingDuplicates.clear();

	emit volumeAmountChanged(0.0, 0.0);
	endResetModel();
}

void OrdersModel::ordersChanged(QList<OrderItem> *orders)
{
	if(orders->count()==0)
	{
		delete orders;
		emit volumeAmountChanged(0.0,0.0);
		clear();
		return;
	}

	QHash<QByteArray,bool> existingOids;

	double volumeTotal=0.0;
	double amountTotal=0.0;
	double decValue=0.0;

	if(mainWindow.exchangeId==2)decValue=0.01;//Bitstamp exception

	for(int n=0;n<orders->count();n++)
	{
		bool isAsk=orders->at(n).type;
		QByteArray orderSymbol=orders->at(n).symbol;

		if(orderSymbol.startsWith(currencyAStr)&&isAsk)volumeTotal+=orders->at(n).amount-decValue;
		if(orderSymbol.endsWith(currencyBStr)&&!isAsk)amountTotal+=(orders->at(n).amount-decValue)*orders->at(n).price;

		existingOids.insert(orders->at(n).oid,true);
		if(checkDuplicatedOID)(*orders)[n].date=oidMapForCheckingDuplicates.value(orders->at(n).oid,orders->at(n).date);

		int currentIndex=qLowerBound(dateList.begin(),dateList.end(),orders->at(n).date)-dateList.begin();
		bool matchListRang=currentIndex>-1&&dateList.count()>currentIndex;

		if(matchListRang&&oidList.at(currentIndex)==orders->at(n).oid)
		{//Update
		 if(statusList.at(currentIndex)&&
			 (statusList.at(currentIndex)!=orders->at(n).status||
			  amountList.at(currentIndex)!=orders->at(n).amount||
			   priceList.at(currentIndex)!=orders->at(n).price))
			{
			statusList[currentIndex]=orders->at(n).status;
			amountList[currentIndex]=orders->at(n).amount;
			priceList[currentIndex]=orders->at(n).price;

			emit dataChanged(index(currentIndex,0),index(currentIndex,columnsCount-1));
			}
		}
		else
		{//Insert
			beginInsertRows(QModelIndex(), currentIndex, currentIndex);

			oidList.insert(currentIndex,orders->at(n).oid);
			dateList.insert(currentIndex,orders->at(n).date);
			typesList.insert(currentIndex,orders->at(n).type);
			statusList.insert(currentIndex,orders->at(n).status);
			amountList.insert(currentIndex,orders->at(n).amount);
			priceList.insert(currentIndex,orders->at(n).price);
			symbolList.insert(currentIndex,orders->at(n).symbol);

			if(checkDuplicatedOID)
				oidMapForCheckingDuplicates.insert(orders->at(n).oid,orders->at(n).date);

			itemSignList.insert(currentIndex,currencySignMap->value(orders->at(n).symbol.left(3),"$"));
			priceSignList.insert(currentIndex,currencySignMap->value(orders->at(n).symbol.right(3),"$"));
			
			endInsertRows();
		}
	}

	for(int n=oidList.count()-1;n>=0;n--)//Removing Order
		if(existingOids.value(oidList.at(n),false)==false)
		{
			beginRemoveRows(QModelIndex(), n, n);
			if(checkDuplicatedOID)oidMapForCheckingDuplicates.remove(oidList.at(n));
			oidList.removeAt(n);
			dateList.removeAt(n);
			typesList.removeAt(n);
			statusList.removeAt(n);
			amountList.removeAt(n);
			priceList.removeAt(n);
			symbolList.removeAt(n);

			itemSignList.removeAt(n);
			priceSignList.removeAt(n);
			endRemoveRows();
		}

	delete orders;

	if(haveOrders==false)
	{
		emit ordersIsAvailable();
		haveOrders=true;
	}
	countWidth=textFontWidth(QString::number(oidList.count()+1))+6;
	emit volumeAmountChanged(volumeTotal, amountTotal);
	emit layoutChanged();
}

QVariant OrdersModel::data(const QModelIndex &index, int role) const
{
	if(!index.isValid())return QVariant();
	int currentRow=oidList.count()-index.row()-1;
	if(currentRow<0||currentRow>=oidList.count())return QVariant();

	if(role==Qt::UserRole)
	{
		if(statusList.at(currentRow))return oidList.at(currentRow);
		return QVariant();
	}

	if(role!=Qt::DisplayRole&&role!=Qt::ToolTipRole&&role!=Qt::ForegroundRole&&role!=Qt::TextAlignmentRole&&role!=Qt::BackgroundRole)return QVariant();

	int indexColumn=index.column()-1;

	if(role==Qt::TextAlignmentRole)return 0x0084;

	if(role==Qt::ForegroundRole)
	{
		switch(indexColumn)
		{
		case 1: return typesList.at(currentRow)?Qt::red:Qt::blue;
		default: break;
		}
		return Qt::black;
	}

	if(role==Qt::BackgroundRole)
	{
		switch(statusList.at(currentRow))
		{
			case 0: return QColor(255,200,200);
			case 2: //return QColor(200,255,200);
			case 3: return QColor(255,255,200);
			case 4: return Qt::red;
			default: break;
		}
		return QVariant();
	}

	switch(indexColumn)
	{
	case -1://Counter
		{
			if(role==Qt::ToolTipRole)return oidList.at(currentRow);
			return oidList.count()-currentRow;
		}
		break;
	case 0:
		{//Date
			//if(role==Qt::ToolTipRole)
				return QDateTime::fromTime_t(dateList.at(currentRow)).toString(localDateTimeFormat);
			//if(highResolutionDisplay)
			//	return QDateTime::fromTime_t(dateList.at(currentRow)).toString(localDateTimeFormat);
			//return QDateTime::fromTime_t(dateList.at(currentRow)).toString(localTimeFormat);
		}
		break;
	case 1:
		{//Type
			return typesList.at(currentRow)?textAsk:textBid;
		}
		break;
	case 2:
		{//Status
			switch(statusList.at(currentRow))
			{
			case 0: return textStatusList.at(0); break;
			case 1: return textStatusList.at(1); break;
			case 2: return textStatusList.at(2); break;
			case 3: return textStatusList.at(3); break;
			default: return textStatusList.at(4); break; 
			}
		}
		break;
	case 3:
		{//Amount
			return itemSignList.at(currentRow)+" "+mainWindow.numFromDouble(amountList.at(currentRow));
		}
		break;
	case 4:
		{//Price
			return priceSignList.at(currentRow)+" "+mainWindow.numFromDouble(priceList.at(currentRow));
		}
		break;
	case 5:
		{//Total
			return priceSignList.at(currentRow)+" "+mainWindow.numFromDouble(amountList.at(currentRow)*priceList.at(currentRow),usdDecimals);
		}
		break;
	default: break;
	}
	return QVariant();
}

void OrdersModel::ordersCancelAll()
{
	for(int n=oidList.count()-1;n>=0;n--)
		if(statusList.at(n))
			emit cancelOrder(oidList.at(n));
}

void OrdersModel::setOrderCanceled(QByteArray oid)
{
	for(int n=0;n<oidList.count();n++)
		if(oidList.at(n)==oid)
		{
			statusList[n]=0;
			emit dataChanged(index(n,0),index(n,columnsCount-1));
			break;
		}
}

QVariant OrdersModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if(orientation!=Qt::Horizontal)return QVariant();
	if(role==Qt::TextAlignmentRole)return 0x0084;

	if(role==Qt::SizeHintRole)
	{
		switch(section)
		{
		case 0: return QSize(countWidth,defaultSectionSize);//Counter
		case 1: return QSize(dateWidth,defaultSectionSize);//Date
		case 2: return QSize(typeWidth,defaultSectionSize);//Type
		case 3: return QSize(statusWidth,defaultSectionSize);//Status
		}
		return QVariant();
	}

	if(role!=Qt::DisplayRole)return QVariant();
	if(headerLabels.count()!=columnsCount)return QVariant();

	return headerLabels.at(section);
}

Qt::ItemFlags OrdersModel::flags(const QModelIndex &) const
{
	return Qt::ItemIsSelectable|Qt::ItemIsEnabled;
}

void OrdersModel::setHorizontalHeaderLabels(QStringList list)
{
	if(list.count()!=columnsCount)return;

	textAsk=julyTr("ORDER_TYPE_ASK","ask");
	textBid=julyTr("ORDER_TYPE_BID","bid");

	textStatusList[0]=julyTr("ORDER_STATE_CANCELED",textStatusList.at(0));
	textStatusList[1]=julyTr("ORDER_STATE_OPEN",textStatusList.at(1));
	textStatusList[2]=julyTr("ORDER_STATE_PENDING",textStatusList.at(2));
	textStatusList[3]=julyTr("ORDER_STATE_POST-PENDING",textStatusList.at(3));
	textStatusList[4]=julyTr("ORDER_STATE_INVALID",textStatusList.at(4));

	dateWidth=qMax(qMax(textFontWidth(QDateTime(QDate(2000,12,30),QTime(23,59,59,999)).toString(localDateTimeFormat)),textFontWidth(QDateTime(QDate(2000,12,30),QTime(12,59,59,999)).toString(localDateTimeFormat))),textFontWidth(list.at(0)))+10;
	typeWidth=qMax(qMax(textFontWidth(textAsk),textFontWidth(textBid)),textFontWidth(list.at(1)))+10;

	for(int n=0;n<4;n++)statusWidth=qMax(textFontWidth(textStatusList.at(n)),textFontWidth(textStatusList.at(n+1)));
	statusWidth+=10;

	headerLabels=list;
	emit headerDataChanged(Qt::Horizontal, 0, columnsCount-1);
	emit layoutChanged();
}

QModelIndex OrdersModel::index(int row, int column, const QModelIndex &parent) const
{
	if(!hasIndex(row, column, parent))return QModelIndex();
	return createIndex(row,column);
}

QModelIndex OrdersModel::parent(const QModelIndex &) const
{
	return QModelIndex();
}