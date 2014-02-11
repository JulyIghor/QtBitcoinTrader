// Copyright (C) 2014 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form http://qtopentrader.com
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#include "historymodel.h"
#include "main.h"

HistoryModel::HistoryModel()
	: QAbstractItemModel()
{
	typeWidth=75;
	columnsCount=7;
	lastDate=0;
	typesLabels<<""<<"Bought"<<"Sell"<<"Buy"<<"Fee"<<"Deposit"<<"Withdraw";//0=General, 1=Sell, 2=Buy, 3=Fee, 4=Deposit, 5=Withdraw
}

HistoryModel::~HistoryModel()
{

}

void HistoryModel::clear()
{
	if(itemsList.isEmpty())return;
	beginResetModel();
	lastDate=0;
	itemsList.clear();
	endResetModel();
}

void HistoryModel::historyChanged(QList<HistoryItem> *histList)
{
	bool haveLastBuy=false;
	bool haveLastSell=false;
	for(int n=0;n<histList->count();n++)
	if(histList->at(n).symbol==baseValues.currentPair.currSymbol)
	{
		if(histList->at(n).type==1)
		{
			emit accLastSellChanged(histList->at(n).symbol.right(3),histList->at(n).price);
			haveLastSell=true;
		}
		else
		if(histList->at(n).type==2)
		{
			emit accLastBuyChanged(histList->at(n).symbol.right(3),histList->at(n).price);
			haveLastBuy=true;
		}
		if(haveLastSell&&haveLastBuy)break;
	}

	while(histList->count()&&histList->last().dateTimeInt<=lastDate)histList->removeLast();
	if(histList->count()==0){delete histList; return;}

	beginInsertRows(QModelIndex(), 0, histList->count());

	if(histList->count()&&itemsList.count())
	{
		(*histList)[histList->count()-1].displayFullDate=histList->at(histList->count()-1).dateInt!=itemsList.last().dateInt;
	}
	quint32 maxListDate=0;
	for(int n=histList->count()-1;n>=0;n--)
	{
		if(maxListDate<histList->at(n).dateTimeInt)maxListDate=histList->at(n).dateTimeInt;

		if(n!=histList->count()-1)(*histList)[n].displayFullDate=histList->at(n).dateInt!=histList->at(n+1).dateInt;

		itemsList<<histList->at(n);
	}
	delete histList;
	if(maxListDate>lastDate)lastDate=maxListDate;
	endInsertRows();
}

double HistoryModel::getRowPrice(int row)
{
	row=itemsList.count()-row-1;
	if(row<0||row>=itemsList.count())return 0.0;
	return itemsList.at(row).price;
}

double HistoryModel::getRowVolume(int row)
{
	row=itemsList.count()-row-1;
	if(row<0||row>=itemsList.count())return 0.0;
	return itemsList.at(row).volume;
}

int HistoryModel::getRowType(int row)
{
	row=itemsList.count()-row-1;
	if(row<0||row>=itemsList.count())return 0.0;
	return itemsList.at(row).type;
}


int HistoryModel::rowCount(const QModelIndex &) const
{
	return itemsList.count();
}

int HistoryModel::columnCount(const QModelIndex &) const
{
	return columnsCount;
}

QVariant HistoryModel::data(const QModelIndex &index, int role) const
{
	int currentRow=itemsList.count()-index.row()-1;
	if(currentRow<0||currentRow>=itemsList.count())return QVariant();

	if(role==Qt::WhatsThisRole)
	{
		return itemsList.at(currentRow).dateTimeStr+" "+typesLabels.at(itemsList.at(currentRow).type)+" "+itemsList.at(currentRow).priceStr+" "+itemsList.at(currentRow).totalStr;
	}

	if(role==Qt::StatusTipRole)
	{
		return itemsList.at(currentRow).dateTimeStr+"\t"+itemsList.at(currentRow).volumeStr+"\t"+typesLabels.at(itemsList.at(currentRow).type)+"\t"+itemsList.at(currentRow).priceStr+"\t"+itemsList.at(currentRow).totalStr;
	}

	if(role!=Qt::DisplayRole&&role!=Qt::ToolTipRole&&role!=Qt::ForegroundRole&&role!=Qt::TextAlignmentRole)return QVariant();

	int indexColumn=index.column();

	if(role==Qt::TextAlignmentRole)
	{
		if(indexColumn==1)return 0x0082;
		if(indexColumn==2)return 0x0082;
		if(indexColumn==4)return 0x0081;
		if(indexColumn==5)return 0x0082;
		if(indexColumn==6)return 0x0081;
		return 0x0084;
	}

	if(role==Qt::ForegroundRole)
	{
		switch(indexColumn)
		{
		case 1: return baseValues.appTheme.gray; break;
		case 3:
			switch(itemsList.at(currentRow).type)
			{
			case 1: return baseValues.appTheme.red;
			case 2: return baseValues.appTheme.blue;
			case 3: return baseValues.appTheme.darkGreen;
			case 4: return baseValues.appTheme.darkRed;
			case 5: return baseValues.appTheme.darkRedBlue;
			default: break;
			}
			break;
		case 6: return baseValues.appTheme.gray; break;
		default: break;
		}
		return baseValues.appTheme.black;
	}

	switch(indexColumn)
	{
	case 1:
		 {//Date
			if(role==Qt::ToolTipRole||itemsList.at(currentRow).displayFullDate)
				return itemsList.at(currentRow).dateTimeStr;//DateTime
			return itemsList.at(currentRow).timeStr;//Time
		 }
	case 2: return itemsList.at(currentRow).volumeStr;//Volume
	case 3: return typesLabels.at(itemsList.at(currentRow).type);//Type
	case 4: return itemsList.at(currentRow).priceStr;//Price
	case 5: return itemsList.at(currentRow).totalStr;//Total
	case 6: return itemsList.at(currentRow).description;//Description
	default: break;
	}
	return QVariant();
}

QVariant HistoryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if(orientation!=Qt::Horizontal)return QVariant();
	if(role==Qt::TextAlignmentRole)
	{
		if(section==2)return 0x0082;
		if(section==4)return 0x0081;
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

	return headerLabels.at(section);
}


Qt::ItemFlags HistoryModel::flags(const QModelIndex &) const
{
	return Qt::ItemIsSelectable|Qt::ItemIsEnabled;
}

void HistoryModel::setHorizontalHeaderLabels(QStringList list)
{
	if(list.count()!=columnsCount)return;

	typesLabels[1]=julyTr("LOG_SOLD","sold");
	typesLabels[2]=julyTr("LOG_BOUGHT","bought");
	typesLabels[3]=julyTr("LOG_FEE","fee");
	typesLabels[4]=julyTr("LOG_DEPOSIT","deposit");
	typesLabels[5]=julyTr("LOG_WITHDRAW","withdraw");

	typeWidth=0;
	for(int n=11;n<typesLabels.count();n++)
		typeWidth=qMax(typeWidth,textFontWidth(typesLabels.at(n)));
	typeWidth=qMax(typeWidth,textFontWidth(list.at(2)));
	typeWidth+=10;

	dateWidth=qMax(qMax(textFontWidth(QDateTime(QDate(2000,12,30),QTime(23,59,59,999)).toString(baseValues.dateTimeFormat)),textFontWidth(QDateTime(QDate(2000,12,30),QTime(12,59,59,999)).toString(baseValues.dateTimeFormat))),textFontWidth(list.at(0)))+10;
	
	headerLabels=list;
	emit headerDataChanged(Qt::Horizontal, 0, columnsCount-1);
}

QModelIndex HistoryModel::index(int row, int column, const QModelIndex &parent) const
{
	if(!hasIndex(row, column, parent))return QModelIndex();
	return createIndex(row,column);
}

QModelIndex HistoryModel::parent(const QModelIndex &) const
{
	return QModelIndex();
}
