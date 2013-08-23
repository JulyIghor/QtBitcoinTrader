// Copyright (C) 2013 July IGHOR.
// I want to create trading application that can be configured for any rule and strategy.
// If you want to help me please Donate: 1d6iMwjjNo8ZGYeJBZKXgcgVk9o7fXcjc
// For any questions please use contact form https://sourceforge.net/projects/bitcointrader/
// Or send e-mail directly to julyighor@gmail.com
//
// You may use, distribute and copy the Qt Bitcion Trader under the terms of
// GNU General Public License version 3

#include "depthmodel.h"
#include "main.h"
#define grouped (groupPriceValue>0.0?2:0)

DepthModel::DepthModel(bool isAskData)
	: QAbstractItemModel()
{
	groupedPrice=0.0;
	groupedVolume=0.0;
	widthPrice=100;
	widthVolume=100;
	widthSize=100;
	somethingChanged=true;
	isAsk=isAskData;
	columnsCount=4;
}

DepthModel::~DepthModel()
{

}

int DepthModel::rowCount(const QModelIndex &) const
{
	return priceList.count()+grouped;
}

int DepthModel::columnCount(const QModelIndex &) const
{
	return columnsCount;
}

QVariant DepthModel::data(const QModelIndex &index, int role) const
{
	if(!index.isValid())return QVariant();
	int currentRow=index.row();

	if(role!=Qt::DisplayRole&&role!=Qt::ToolTipRole&&role!=Qt::ForegroundRole)return QVariant();

	int indexColumn=index.column();
	if(isAsk)indexColumn=columnsCount-indexColumn-1;

	if(grouped&&currentRow<2)
	{
		if(role==Qt::ForegroundRole)return Qt::black;
		if(currentRow==1||groupedPrice==0.0)return QVariant();
		QString firstRowText;
		switch(indexColumn)
		{
		case 0: firstRowText=currencyBSign+QLatin1String(" ")+mainWindow.numFromDouble(groupedPrice); break; //Price
		case 1: firstRowText=currencyASign+QLatin1String(" ")+mainWindow.numFromDouble(groupedVolume); break; //Volume
		}
		if(firstRowText.isEmpty())return QVariant();
		return firstRowText;
	}

	if(grouped)currentRow-=grouped;
	if(currentRow<0||currentRow>=priceList.count())return QVariant();

	if(!isAsk)currentRow=priceList.count()-currentRow-1;

	if(role==Qt::ForegroundRole)
	{
		if(indexColumn==1)
		{
		double volume=volumeList.at(currentRow);
		if(volume<1.0)return QColor(0,0,0,155+volume*100.0);
		else if(volume<100.0)return Qt::black;
		else if(volume<1000.0)return Qt::blue;
		else return Qt::red;
		return Qt::black;
		}
		return QVariant();
	}

	double requestedPrice=priceList.at(currentRow);
	if(requestedPrice<=0.0)return QVariant();
	QString returnText;

	switch(indexColumn)
	{
	case 0:	returnText=currencyBSign+QLatin1String(" ")+mainWindow.numFromDouble(requestedPrice); break;//Price
	case 1:
		{//Volume
		double requestedVolume=volumeList.at(currentRow);
		if(requestedVolume<=0.0)return QVariant();
		returnText=currencyASign+QLatin1String(" ")+mainWindow.numFromDouble(requestedVolume);
		}
		break;
	case 2:
		{//Size
		double requestedSize=sizeList.at(currentRow);
		if(requestedSize<=0.0)return QVariant();
		returnText=currencyASign+QLatin1String(" ")+mainWindow.numFromDouble(requestedSize);
		}
		break;
	default: break;
	}
	if(!returnText.isEmpty())return returnText;
	return QVariant();
}

void DepthModel::calculateSize()
{
	if(!somethingChanged)return;
	somethingChanged=true;

	uint currentPriceWidth=9;
	uint currentVolumeWidth=9;
	uint currentSizeWidth=9;

	double totalSize=0.0;
	if(isAsk)
	{
		for(int n=0;n<priceList.count();n++)
		{
			totalSize+=volumeList.at(n);
			sizeList[n]=totalSize;

			while(currentPriceWidth<priceList.at(n)){currentPriceWidth*=10;currentPriceWidth+=9;}
			while(currentVolumeWidth<volumeList.at(n)){currentVolumeWidth*=10;currentVolumeWidth+=9;}
			while(currentSizeWidth<sizeList.at(n)){currentSizeWidth*=10;currentSizeWidth+=9;}
		}
	}
	else
	{
		for(int n=priceList.count()-1;n>=0;n--)
		{
			totalSize+=volumeList.at(n);
			sizeList[n]=totalSize;

			while(currentPriceWidth<priceList.at(n)){currentPriceWidth*=10;currentPriceWidth+=9;}
			while(currentVolumeWidth<volumeList.at(n)){currentVolumeWidth*=10;currentVolumeWidth+=9;}
			while(currentSizeWidth<sizeList.at(n)){currentSizeWidth*=10;currentSizeWidth+=9;}
		}
	}

	QString curAPudding=QLatin1String(".")+QString('9').repeated(btcDecimals);
	QString curBPudding=QLatin1String(".")+QString('9').repeated(usdDecimals);

	widthPrice=10+textWidth(currencyBSign+QLatin1String(" ")+QString::number(currentPriceWidth)+curBPudding);
	widthVolume=10+textWidth(currencyASign+QLatin1String(" ")+QString::number(currentVolumeWidth)+curAPudding);
	widthSize=10+textWidth(currencyASign+QLatin1String(" ")+QString::number(currentSizeWidth)+curAPudding);

	int sizeColumn=2;
	if(isAsk)sizeColumn=1;
	emit dataChanged(index(0,sizeColumn),index(priceList.count(),sizeColumn));
}

QModelIndex DepthModel::index(int row, int column, const QModelIndex &parent) const
{
	if(!hasIndex(row, column, parent))return QModelIndex();
	return createIndex(row,column);
}

QModelIndex DepthModel::parent(const QModelIndex &) const
{
	return QModelIndex();
}

void DepthModel::clear()
{
	if(priceList.isEmpty())return;
	groupedPrice=0.0;
	groupedVolume=0.0;
	priceList.clear();
	volumeList.clear();
	sizeList.clear();
	reset();
	somethingChanged=false;
}

Qt::ItemFlags DepthModel::flags(const QModelIndex &index) const
{
	if(!index.isValid())return 0;
	if(grouped)
	{
		if(index.row()==1||groupedPrice==0.0&&priceList.isEmpty())return Qt::NoItemFlags;
	}
	return Qt::ItemIsSelectable|Qt::ItemIsEnabled;
}

QVariant DepthModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	int indexColumn=section;
	if(isAsk)indexColumn=columnsCount-indexColumn-1;

	if(role==Qt::SizeHintRole)
	{
		switch(indexColumn)
		{
		case 0: return QSize(widthPrice,defaultSectionSize);//Price
		case 1: return QSize(widthVolume,defaultSectionSize);//Volume
		case 2: return QSize(widthSize,defaultSectionSize);//Size
		}
		return QVariant();
	}

	if(role!=Qt::DisplayRole)return QVariant();
	if(orientation!=Qt::Horizontal)return QVariant();
	if(headerLabels.count()!=columnsCount)return QVariant();

	return headerLabels.at(indexColumn);
}

void DepthModel::setHorizontalHeaderLabels(QStringList list)
{
	if(list.count()!=columnsCount)return;
	headerLabels=list;
	emit headerDataChanged(Qt::Horizontal, 0, columnsCount);
}

void DepthModel::depthFirstOrder(double price, double volume)
{
	if(!grouped)return;
	if(price==groupedPrice&&groupedVolume==volume)return;
	groupedPrice=price;
	groupedVolume=volume;
	if(isAsk)
		emit dataChanged(index(0,2),index(0,3));
	else 
		emit dataChanged(index(0,0),index(0,1));
}

void DepthModel::depthUpdateOrder(double price, double volume)
{
	if(price==0.0)return;
	int currentIndex=qLowerBound(priceList.begin(),priceList.end(),price)-priceList.begin();
	bool matchListRang=currentIndex>-1&&priceList.count()>currentIndex;

	if(volume==0.0)
	{//Remove item
		if(matchListRang)
		{
			beginRemoveRows(QModelIndex(), currentIndex+grouped, currentIndex+grouped);
			priceList.removeAt(currentIndex);
			volumeList.removeAt(currentIndex);
			sizeList.removeAt(currentIndex);
			endRemoveRows();
			somethingChanged=true;
		}
		return;
	}
	if(matchListRang&&priceList.at(currentIndex)==price)
	{//Update
		if(volumeList.at(currentIndex)==volume)return;
		volumeList[currentIndex]=volume;
		sizeList[currentIndex]=0.0;
		somethingChanged=true;
		emit dataChanged(index(currentIndex+grouped,0),index(currentIndex+grouped,columnsCount));
	}
	else
	{//Insert
		beginInsertRows(QModelIndex(), currentIndex+grouped, currentIndex+grouped);
		priceList.insert(currentIndex,price);
		volumeList.insert(currentIndex,volume);
		sizeList.insert(currentIndex,volume);
		endInsertRows();
		somethingChanged=true;
	}
}

double DepthModel::rowPrice(int row)
{
	if(grouped&&row<2)
	{
		if(row==0)return groupedPrice;
		return 0.0;
	}
	row-=grouped;
	if(!isAsk)row=priceList.count()-row-1;
	if(row<0||row>=priceList.count())return 0.0;
	return priceList.at(row);
}

double DepthModel::rowVolume(int row)
{
	if(grouped&&row<2)
	{
		if(row==0)return groupedVolume;
		return 0.0;
	}
	row-=grouped;
	if(!isAsk)row=priceList.count()-row-1;
	if(row<0||row>=priceList.count())return 0.0;
	return volumeList.at(row);
}

double DepthModel::rowSize(int row)
{
	if(grouped&&row<2)return 0.0;
	row-=grouped;
	if(!isAsk)row=priceList.count()-row-1;
	if(row<0||row>=priceList.count())return 0.0;
	return sizeList.at(row);
}

