// Copyright (C) 2013 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form https://sourceforge.net/projects/bitcointrader/
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
	columnsCount=4;
	lastDate=0;
	typesLabels<<""<<"Bought"<<"Sell"<<"Buy"<<"Fee"<<"Deposit"<<"Withdraw";//0=General, 1=Sell, 2=Buy, 3=Fee, 4=Deposit, 5=Withdraw
}

HistoryModel::~HistoryModel()
{

}

void HistoryModel::historyChanged(QList<HistoryItem> *histList)
{
	while(histList->count()&&histList->last().date<=lastDate)histList->removeLast();
	if(histList->count()==0){delete histList; return;}

	beginInsertRows(QModelIndex(), 0, histList->count());
	qint64 maxListDate=0;
	for(int n=histList->count()-1;n>=0;n--)
	{
		qint64 curItemDate=histList->at(n).date;
		if(maxListDate<curItemDate)maxListDate=curItemDate;
		dateList<<curItemDate;
		volumeList<<histList->at(n).volume;
		priceList<<histList->at(n).price;
		symbolList<<histList->at(n).symbol;
		typesList<<histList->at(n).type;
		if(histList->at(n).type==1)emit accLastSellChanged(histList->at(n).symbol.right(3),histList->at(n).price);
		else
		if(histList->at(n).type==2)emit accLastBuyChanged(histList->at(n).symbol.right(3),histList->at(n).price);
	}
	delete histList;
	if(maxListDate>lastDate)lastDate=maxListDate;
	endInsertRows();
}

double HistoryModel::getRowPrice(int row)
{
	row=priceList.count()-row-1;
	if(row<0||row>=priceList.count())return 0.0;
	return priceList.at(row);
}

double HistoryModel::getRowVolume(int row)
{
	row=volumeList.count()-row-1;
	if(row<0||row>=volumeList.count())return 0.0;
	return volumeList.at(row);
}

int HistoryModel::getRowType(int row)
{
	row=typesList.count()-row-1;
	if(row<0||row>=typesList.count())return 0.0;
	return typesList.at(row);
}


int HistoryModel::rowCount(const QModelIndex &) const
{
	return dateList.count();
}

int HistoryModel::columnCount(const QModelIndex &) const
{
	return columnsCount;
}

QVariant HistoryModel::data(const QModelIndex &index, int role) const
{
	int currentRow=dateList.count()-index.row()-1;
	if(currentRow<0||currentRow>=dateList.count())return QVariant();

	if(role!=Qt::DisplayRole&&role!=Qt::ToolTipRole&&role!=Qt::ForegroundRole&&role!=Qt::TextAlignmentRole)return QVariant();

	int indexColumn=index.column();

	if(role==Qt::TextAlignmentRole)
	{
		if(indexColumn==1)return 0x0082;
		if(indexColumn==3)return 0x0081;
		return 0x0084;
	}

	if(role==Qt::ForegroundRole)
	{
		switch(indexColumn)
		{
		case 0: return Qt::gray; break;
		case 2:
			switch(typesList.at(currentRow))
			{
			case 1: return Qt::red;
			case 2: return Qt::blue;
			case 3: return Qt::darkGreen;
			case 4: return Qt::darkRed;
			case 5: return Qt::darkBlue;
			default: break;
			}
		default: break;
		}
		return Qt::black;
	}

	switch(indexColumn)
	{
	case 0:	return QDateTime::fromTime_t(dateList.at(currentRow)).toString(localDateTimeFormat); break;//Date
	case 1:
		{//Volume
			double requestedVolume=volumeList.at(currentRow);
			if(requestedVolume<=0.0)return QVariant();
			return currencyASign+QLatin1String(" ")+mainWindow.numFromDouble(requestedVolume);
		}
		break;
	case 2://Type
		{
			return typesLabels.at(typesList.at(currentRow));
		}
	case 3:
		{//Price
			double requestedPrice=priceList.at(currentRow);
			if(requestedPrice<=0.0)return QVariant();
			QString returnPrice=currencyBSign+QLatin1String(" ")+mainWindow.numFromDouble(requestedPrice);
			return returnPrice;
		}
		break;
	default: break;
	}
	return QVariant();
}

QVariant HistoryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if(orientation!=Qt::Horizontal)return QVariant();
	if(role==Qt::TextAlignmentRole)
	{
		if(section==1)return 0x0082;
		if(section==3)return 0x0081;
		return 0x0084;
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
	headerLabels=list;
	emit headerDataChanged(Qt::Horizontal, 0, columnsCount-1);
	emit layoutChanged();
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
