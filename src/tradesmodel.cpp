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
	lastRemoveDate=0;
	lastPrice=0.0;
	columnsCount=4;
	dateWidth=100;
	typeWidth=100;
	upArrow=QByteArray::fromBase64("4oaR");
	downArrow=QByteArray::fromBase64("4oaT");
}

TradesModel::~TradesModel()
{

}

void TradesModel::clear()
{
	if(priceList.isEmpty())return;
	beginResetModel();
	lastPrice=0.0;
	dateList.clear();
	volumeList.clear();
	priceList.clear();
	directionList.clear();
	directionList.clear();
	endResetModel();
}

int TradesModel::rowCount(const QModelIndex &) const
{
	return priceList.count();
}

int TradesModel::columnCount(const QModelIndex &) const
{
	return columnsCount;
}

void TradesModel::removeFirst()
{
	if(dateList.count()==0)return;
	dateList.removeFirst();
	volumeList.removeFirst();
	priceList.removeFirst();
	typesList.removeFirst();
	directionList.removeFirst();
}

void TradesModel::removeDataOlderThen(qint64 date)
{
	lastRemoveDate=date;
	if(priceList.count()==0){updateTotalBTC();return;}
	int lowerIndex=qLowerBound(dateList.begin(),dateList.end(),date)-dateList.begin();
	if(lowerIndex==0)
	{
		if(date>=dateList.first())
		{
			beginRemoveRows(QModelIndex(),0,0);
			removeFirst();
			endRemoveRows();
			updateTotalBTC();
		}
		return;
	}
	beginRemoveRows(QModelIndex(), lowerIndex, priceList.count());
	for(int n=0;n<lowerIndex;n++)removeFirst();
	endRemoveRows();
	if(priceList.count()==0)clear();
	updateTotalBTC();
}

QVariant TradesModel::data(const QModelIndex &index, int role) const
{
	if(!index.isValid())return QVariant();
	int currentRow=priceList.count()-index.row()-1;
	if(currentRow<0||currentRow>=priceList.count())return QVariant();

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
		case 1:
			{
			double volume=volumeList.at(currentRow);
			if(volume<1.0)return QColor(0,0,0,155+volume*100.0);
			else if(volume<100.0)return Qt::black;
			else if(volume<1000.0)return Qt::blue;
			else return Qt::red;
			return Qt::black;
			}
			break;
		case 2:
			if(typesList.at(currentRow))return Qt::red;
			return Qt::blue;
			break;
		default: break;
		}
		return Qt::black;
	}

	double requestedPrice=priceList.at(currentRow);
	if(requestedPrice<=0.0)return QVariant();

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
	case 2:
		{//Type
			if(typesList.at(currentRow))return textAsk;
			else return textBid;
		}
		break;
	case 3:
		{//Price
			double requestedPrice=priceList.at(currentRow);
			if(requestedPrice<=0.0)return QVariant();
			QString returnPrice=currencyBSign+QLatin1String(" ")+mainWindow.numFromDouble(requestedPrice);
			if(directionList.at(currentRow))
			{
				if(directionList.at(currentRow)==1)
					returnPrice.append(QLatin1String(" ")+upArrow);
				else returnPrice.append(QLatin1String(" ")+downArrow);
			}
			return returnPrice;
		}
		break;
	default: break;
	}
	return QVariant();
}

QVariant TradesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if(orientation!=Qt::Horizontal)return QVariant();
	if(role==Qt::TextAlignmentRole)
	{
		if(section==1)return 0x0082;
		if(section==3)return 0x0081;
		return 0x0084;
	}

	if(role==Qt::SizeHintRole)
	{
		switch(section)
		{
		case 0: return QSize(dateWidth,defaultSectionSize);//Date
		case 2: return QSize(typeWidth,defaultSectionSize);//Type
		}
		return QVariant();
	}

	if(role!=Qt::DisplayRole)return QVariant();
	if(headerLabels.count()!=columnsCount)return QVariant();

	return headerLabels.at(section);
}

void TradesModel::updateTotalBTC()
{
	double summ=0.0;
	foreach(double vol, volumeList)summ+=vol;
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
	dateWidth=qMax(qMax(textWidth(QDateTime(QDate(2000,12,30),QTime(23,59,59,999)).toString(localDateTimeFormat)),textWidth(QDateTime(QDate(2000,12,30),QTime(12,59,59,999)).toString(localDateTimeFormat))),textWidth(list.at(0)))+10;
	typeWidth=qMax(qMax(textWidth(textAsk),textWidth(textBid)),textWidth(list.at(2)))+10;

	headerLabels=list;
	emit headerDataChanged(Qt::Horizontal, 0, columnsCount-1);
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

void TradesModel::addNewTrade(qint64 dateT, double volumeT, double priceT, QByteArray symbol, bool isSell)
{
	if(symbol!=currencySymbol||dateT<=lastRemoveDate)return;
	beginInsertRows(QModelIndex(),0,0);
	dateList<<dateT;
	volumeList<<volumeT;
	priceList<<priceT;
	typesList<<isSell;
	int currentDirection=0;
	if(lastPrice>priceT)currentDirection=-1;
	if(lastPrice<priceT)currentDirection=1;
	lastPrice=priceT;
	directionList<<currentDirection;
	endInsertRows();
}

double TradesModel::getRowPrice(int row)
{
	row=priceList.count()-row-1;
	if(row<0||row>=priceList.count())return 0.0;
	return priceList.at(row);
}

double TradesModel::getRowVolume(int row)
{
	row=volumeList.count()-row-1;
	if(row<0||row>=volumeList.count())return 0.0;
	return volumeList.at(row);
}

bool TradesModel::getRowType(int row)
{
	row=typesList.count()-row-1;
	if(row<0||row>=typesList.count())return 0.0;
	return typesList.at(row);
}
