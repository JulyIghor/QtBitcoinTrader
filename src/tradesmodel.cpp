// Copyright (C) 2013 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form https://sourceforge.net/projects/bitcointrader/
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#include "tradesmodel.h"
#include "main.h"

TradesModel::TradesModel()
	: QAbstractItemModel()
{
	lastPrecentBids=0.0;
	lastRemoveDate=0;
	lastPrice=0.0;
	columnsCount=8;
	dateWidth=100;
	typeWidth=100;
}

TradesModel::~TradesModel()
{

}

void TradesModel::clear()
{
	if(itemsList.isEmpty())return;
	beginResetModel();
	lastPrice=0.0;
	itemsList.clear();
	endResetModel();
}

int TradesModel::rowCount(const QModelIndex &) const
{
	return itemsList.count();
}

int TradesModel::columnCount(const QModelIndex &) const
{
	return columnsCount;
}

void TradesModel::removeFirst()
{
	if(itemsList.count()==0)return;
	itemsList.removeFirst();
}

void TradesModel::removeDataOlderThen(quint32 date)
{
	lastRemoveDate=date;
	if(itemsList.count()==0){updateTotalBTC();return;}

	int removeUpToIndex=-1;
	for(int n=0;n<itemsList.count();n++)
	{
		if(date<=itemsList.at(n).date)break;
		removeUpToIndex=n;
	}

	if(removeUpToIndex==-1)return;

	beginRemoveRows(QModelIndex(),0,removeUpToIndex);
	for(int n=0;n<=removeUpToIndex;n++)itemsList.removeFirst();
	endRemoveRows();

	if(itemsList.count()==0)clear();
	updateTotalBTC();
	emit layoutChanged();
}

QVariant TradesModel::data(const QModelIndex &index, int role) const
{
	if(!index.isValid())return QVariant();
	int currentRow=itemsList.count()-index.row()-1;
	if(currentRow<0||currentRow>=itemsList.count())return QVariant();

	if(role==Qt::WhatsThisRole)
	{
		QString typeText;
		switch(itemsList.at(currentRow).orderType)
		{
		case -1: typeText=textBid; break;
		case 1: typeText=textAsk; break;
		}
		return itemsList.at(currentRow).dateStr+" "+baseValues.currentPair.currASign+itemsList.at(currentRow).amountStr+" "+typeText+" "+(itemsList.at(currentRow).direction==1?upArrowStr:downArrowStr)+" "+baseValues.currentPair.currBSign+itemsList.at(currentRow).priceStr+" "+baseValues.currentPair.currBSign+itemsList.at(currentRow).totalStr;
	}

	if(role!=Qt::DisplayRole&&role!=Qt::ToolTipRole&&role!=Qt::ForegroundRole&&role!=Qt::BackgroundRole&&role!=Qt::TextAlignmentRole)return QVariant();

	int indexColumn=index.column();

	if(role==Qt::TextAlignmentRole)
	{
		if(indexColumn==1)return 0x0082;
		if(indexColumn==2)return 0x0082;
		if(indexColumn==5)return 0x0081;
		if(indexColumn==6)return 0x0082;
		return 0x0084;
	}

	if(role==Qt::BackgroundRole)
	{
		return itemsList.at(currentRow).backGray?baseValues.appTheme.altRowColor:baseValues.appTheme.white;
	}

	if(role==Qt::ForegroundRole)
	{
		switch(indexColumn)
		{
		case 1: return baseValues.appTheme.gray; break;
		case 2:
			{
			double amount=itemsList.at(currentRow).amount;
			if(amount<1.0)
			{
				//if(baseValues.appTheme.nightMode) return QColor(255,255,255,255-(155+amount*100.0)).lighter(200).lighter(200);
				//else 
					return QColor(0,0,0,155+amount*100.0);
			}
			else if(amount<100.0)return baseValues.appTheme.black;
			else if(amount<1000.0)return baseValues.appTheme.blue;
			else return baseValues.appTheme.red;
			return baseValues.appTheme.black;
			}
			break;
		case 3:
			switch(itemsList.at(currentRow).orderType)
			{
			case -1: return baseValues.appTheme.blue;
			case 1: return baseValues.appTheme.red;
			default: return baseValues.appTheme.black;
			}
		default: break;
		}
		return baseValues.appTheme.black;
	}

	double requestedPrice=itemsList.at(currentRow).price;
	if(requestedPrice<=0.0)return QVariant();

	switch(indexColumn)
	{
	case 1:
		 {//Date
			if(role==Qt::ToolTipRole||itemsList.at(currentRow).displayFullDate)return itemsList.at(currentRow).dateStr;
			return itemsList.at(currentRow).timeStr; break;
		 }
	case 2:
		{//Volume
			if(itemsList.at(currentRow).amount<=0.0)return QVariant();
			if(role==Qt::ToolTipRole)return baseValues.currentPair.currASign+itemsList.at(currentRow).amountStr;
			return itemsList.at(currentRow).amountStr;
		}
		break;
	case 3:
		{//Type
			switch(itemsList.at(currentRow).orderType)
			{
			case -1: return textBid;
			case 1: return textAsk;
			default: return QVariant();
			}
		}
		break;
	case 4:
		{//Direction
			if(itemsList.at(currentRow).price<=0.0)return QVariant();
			if(itemsList.at(currentRow).direction)
			{
				if(itemsList.at(currentRow).direction==1)return upArrowStr;
				else return downArrowStr;
			}
			return QVariant();
		}
		break;
	case 5:
		{//Price
			if(itemsList.at(currentRow).price<=0.0)return QVariant();
			if(role==Qt::ToolTipRole)return baseValues.currentPair.currBSign+itemsList.at(currentRow).priceStr;
			return itemsList.at(currentRow).priceStr;
		}
		break;
	case 6:
		{//Total
			if(itemsList.at(currentRow).price<=0.0)return QVariant();
			if(role==Qt::ToolTipRole)return baseValues.currentPair.currBSign+itemsList.at(currentRow).totalStr;
			return itemsList.at(currentRow).totalStr;
		}
	default: break;
	}
	return QVariant();
}

QVariant TradesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if(orientation!=Qt::Horizontal)return QVariant();
	if(role==Qt::TextAlignmentRole)
	{
		if(section==2)return 0x0082;
		if(section==5)return 0x0081;
		return 0x0084;
	}

	if(role==Qt::SizeHintRole)
	{
		switch(section)
		{
		case 1: return QSize(dateWidth,defaultHeightForRow);//Date
		case 3: return QSize(typeWidth,defaultHeightForRow);//Type
		}
		return QVariant();
	}

	if(role!=Qt::DisplayRole)return QVariant();
	if(headerLabels.count()!=columnsCount)return QVariant();

	switch(section)
	{
	case 2: return headerLabels.at(section)+baseValues.currentPair.currASign;
	case 5: 
	case 6: return headerLabels.at(section)+baseValues.currentPair.currBSign;
	default: break;
	}
	return headerLabels.at(section);
}

void TradesModel::updateTotalBTC()
{
	double summ=0.0;
	double bidsSumm=0.0;
	for(int n=0;n<itemsList.count();n++)
	{
		summ+=itemsList.at(n).amount;
		if(itemsList.at(n).orderType==-1)bidsSumm+=itemsList.at(n).amount;
	}
	bidsSumm=100.0*bidsSumm/summ;
	if(bidsSumm!=lastPrecentBids)
	{
		lastPrecentBids=bidsSumm;
		emit precentBidsChanged(lastPrecentBids);
	}
	emit trades10MinVolumeChanged(summ);
}

Qt::ItemFlags TradesModel::flags(const QModelIndex &) const
{
	return Qt::ItemIsSelectable|Qt::ItemIsEnabled;
}

void TradesModel::setHorizontalHeaderLabels(QStringList list)
{
	if(list.count()!=columnsCount)return;

	textAsk=julyTr("ORDER_TYPE_ASK","ask");
	textBid=julyTr("ORDER_TYPE_BID","bid");
	dateWidth=qMax(qMax(textFontWidth(QDateTime(QDate(2000,12,30),QTime(23,59,59,999)).toString(baseValues.dateTimeFormat)),textFontWidth(QDateTime(QDate(2000,12,30),QTime(12,59,59,999)).toString(baseValues.dateTimeFormat))),textFontWidth(list.at(0)))+10;
	typeWidth=qMax(qMax(textFontWidth(textAsk),textFontWidth(textBid)),textFontWidth(list.at(2)))+10;

	headerLabels=list;
	emit headerDataChanged(Qt::Horizontal, 0, columnsCount-1);
	emit layoutChanged();
}

QModelIndex TradesModel::index(int row, int column, const QModelIndex &parent) const
{
	if(!hasIndex(row, column, parent))return QModelIndex();
	return createIndex(row,column);
}

QModelIndex TradesModel::parent(const QModelIndex &) const
{
	return QModelIndex();
}

void TradesModel::addNewTrades(QList<TradesItem> *newItems)
{
	QList<TradesItem> verifedItems;

	for(int n=0;n<newItems->count();n++)
	{
	if(newItems->at(n).date<200||newItems->at(n).symbol!=baseValues.currentPair.currSymbol||newItems->at(n).date<=lastRemoveDate)continue;
	if(lastPrice>newItems->at(n).price)(*newItems)[n].direction=-1;
	if(lastPrice<newItems->at(n).price)(*newItems)[n].direction=1;
	lastPrice=newItems->at(n).price;

	if(newItems->at(n).orderType==0)
	{
		if(newItems->at(n).date>mainWindow.currencyChangedDate)
		{
			if(newItems->at(n).price<mainWindow.meridianPrice)(*newItems)[n].orderType=1;
			else (*newItems)[n].orderType=-1;
		}
		
	}
	static bool backSwitcher=false;

	(*newItems)[n].backGray=backSwitcher;
	verifedItems<<newItems->at(n);
	backSwitcher=!backSwitcher;
	}

	if(verifedItems.count()>0)
	{
	verifedItems[verifedItems.count()-1].displayFullDate=true;
	beginInsertRows(QModelIndex(),0,verifedItems.count()-1);
	itemsList<<verifedItems;
	endInsertRows();
	}

	delete newItems;
}

double TradesModel::getRowPrice(int row)
{
	row=itemsList.count()-row-1;
	if(row<0||row>=itemsList.count())return 0.0;
	return itemsList.at(row).price;
}

double TradesModel::getRowVolume(int row)
{
	row=itemsList.count()-row-1;
	if(row<0||row>=itemsList.count())return 0.0;
	return itemsList.at(row).amount;
}

int TradesModel::getRowType(int row)
{
	row=itemsList.count()-row-1;
	if(row<0||row>=itemsList.count())return true;
	return itemsList.at(row).orderType;
}
