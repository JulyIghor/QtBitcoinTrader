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
#include <QTimer>

DepthModel::DepthModel(bool isAskData)
	: QAbstractItemModel()
{
	widthPriceTitle=75;
	widthVolumeTitle=75;
	widthSizeTitle=75;

	groupedPrice=0.0;
	groupedVolume=0.0;
	widthPrice=75;
	widthVolume=75;
	widthSize=75;
	somethingChanged=true;
	isAsk=isAskData;
	originalIsAsk=isAsk;
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

	if(role!=Qt::DisplayRole&&role!=Qt::ToolTipRole&&role!=Qt::ForegroundRole&&role!=Qt::BackgroundRole&&role!=Qt::TextAlignmentRole)return QVariant();

	int indexColumn=index.column();
	if(isAsk)indexColumn=columnsCount-indexColumn-1;

	if(role==Qt::TextAlignmentRole)
	{
		if(indexColumn==0)return 0x0081;
		if(indexColumn==1)return 0x0082;
		if(indexColumn==2)return 0x0082;
		return 0x0084;
	}

	if(grouped&&currentRow<2)
	{
		if(role==Qt::ForegroundRole)return Qt::black;
		if(currentRow==1||groupedPrice==0.0)return QVariant();
		QString firstRowText;
		switch(indexColumn)
		{
		case 0: //Price
				firstRowText=mainWindow.numFromDouble(groupedPrice);
				if(role==Qt::ToolTipRole)firstRowText.prepend(currencyBSign+QLatin1String(" "));	
				break; 
		case 1: //Volume
				firstRowText=QString::number(groupedVolume,'f',btcDecimals);  
				if(role==Qt::ToolTipRole)firstRowText.prepend(currencyASign+QLatin1String(" "));
				break;
		}
		if(firstRowText.isEmpty())return QVariant();
		return firstRowText;
	}

	if(grouped)currentRow-=grouped;
	if(currentRow<0||currentRow>=priceList.count())return QVariant();

	if(!originalIsAsk)currentRow=priceList.count()-currentRow-1;

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

	if(role==Qt::BackgroundRole)
	{
		if(!isAsk)
		{
			if(mainWindow.ordersModel->currentAsksPrices.value(requestedPrice,false))return QColor(200,255,200);
		}
		else
		{
			if(mainWindow.ordersModel->currentBidsPrices.value(requestedPrice,false))return QColor(200,255,200);
		}
		return QVariant();
	}

	QString returnText;

	switch(indexColumn)
	{
	case 0://Price
		returnText=mainWindow.numFromDouble(requestedPrice);
		if(role==Qt::ToolTipRole)returnText.prepend(currencyBSign+QLatin1String(" "));
		break;
	case 1:
		{//Volume
		double requestedVolume=volumeList.at(currentRow);
		if(requestedVolume<=0.0)return QVariant();
		returnText=QString::number(requestedVolume,'f',btcDecimals);
		if(role==Qt::ToolTipRole)returnText.prepend(currencyASign+QLatin1String(" "));
		}
		break;
	case 2:
		{//Size
		double requestedSize=sizeList.at(currentRow);
		if(requestedSize<=0.0)return QVariant();
		returnText=QString::number(requestedSize,'f',btcDecimals);
		if(role==Qt::ToolTipRole)returnText.prepend(currencyASign+QLatin1String(" "));
		}
		break;
	default: break;
	}
	if(!returnText.isEmpty())return returnText;
	return QVariant();
}

void DepthModel::reloadVisibleItems()
{
	QTimer::singleShot(100,this,SLOT(delayedReloadVisibleItems()));
}

void DepthModel::delayedReloadVisibleItems()
{
	emit dataChanged(index(0,0),index(priceList.count()-1,columnsCount-1));
	emit layoutChanged();
}

void DepthModel::calculateSize()
{
	if(!somethingChanged)return;
	somethingChanged=true;

	double maxPrice=0.0;
	double maxVolume=0.0;
	double maxTotal=0.0;

	double totalSize=0.0;
	if(isAsk)
	{
		for(int n=0;n<priceList.count();n++)
		{
			int currentRow=n;
			if(!originalIsAsk)currentRow=priceList.count()-currentRow-1;

			totalSize+=volumeList.at(currentRow);
			sizeList[currentRow]=totalSize;

			maxPrice=qMax(maxPrice,priceList.at(currentRow));
			maxVolume=qMax(maxVolume,volumeList.at(currentRow));
			maxTotal=qMax(maxTotal,sizeList.at(currentRow));
		}
	}
	else
	{
		for(int n=priceList.count()-1;n>=0;n--)
		{
			int currentRow=n;
			if(originalIsAsk)currentRow=priceList.count()-currentRow-1;
			totalSize+=volumeList.at(currentRow);
			sizeList[currentRow]=totalSize;

			maxPrice=qMax(maxPrice,priceList.at(currentRow));
			maxVolume=qMax(maxVolume,volumeList.at(currentRow));
			maxTotal=qMax(maxTotal,sizeList.at(currentRow));
		}
	}

	widthPrice=10+textFontWidth(mainWindow.numFromDouble(maxPrice,priceDecimals));
	widthVolume=10+textFontWidth(QString::number(maxVolume,'f',btcDecimals));
	widthSize=10+textFontWidth(QString::number(maxTotal,'f',btcDecimals));
	
	widthPrice=qMax(widthPrice,widthPriceTitle);
	widthVolume=qMax(widthVolume,widthVolumeTitle);
	widthSize=qMax(widthSize,widthSizeTitle);

	int sizeColumn=2;
	if(isAsk)sizeColumn=1;
	emit dataChanged(index(0,sizeColumn),index(priceList.count()-1,sizeColumn));
	emit layoutChanged();
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
	beginResetModel();
	groupedPrice=0.0;
	groupedVolume=0.0;
	priceList.clear();
	volumeList.clear();
	sizeList.clear();
	endResetModel();
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

	if(orientation!=Qt::Horizontal)return QVariant();

	if(role==Qt::TextAlignmentRole)
	{
		if(indexColumn==0)return 0x0081;
		if(indexColumn==1)return 0x0082;
		if(indexColumn==2)return 0x0082;
		return 0x0084;
	}

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
	if(headerLabels.count()!=columnsCount)return QVariant();

	switch(indexColumn)
	{
	case 0: return headerLabels.at(indexColumn)+QLatin1String(" ")+currencyBSign;
	case 1: return headerLabels.at(indexColumn)+QLatin1String(" ")+currencyASign;
	case 2: return headerLabels.at(indexColumn)+QLatin1String(" ")+currencyASign;
	}

	return headerLabels.at(indexColumn);
}

void DepthModel::fixTitleWidths()
{
	int curASize=textFontWidth(" "+currencyASign);
	int curBSize=textFontWidth(" "+currencyBSign);
	widthPriceTitle=textFontWidth(headerLabels.at(0))+20+curBSize;
	widthVolumeTitle=textFontWidth(headerLabels.at(1))+20+curASize;
	widthSizeTitle=textFontWidth(headerLabels.at(2))+20+curASize;
	emit layoutChanged();
}

void DepthModel::setHorizontalHeaderLabels(QStringList list)
{
	if(list.count()!=columnsCount)return;
	headerLabels=list;
	fixTitleWidths();
	emit headerDataChanged(Qt::Horizontal, 0, columnsCount-1);
	emit layoutChanged();
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

void DepthModel::depthUpdateOrders(QList<DepthItem> *items)
{
	if(items==0)return;
	for(int n=0;n<items->count();n++)depthUpdateOrder(items->at(n).price,items->at(n).volume);
	delete items;
	calculateSize();
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
		emit dataChanged(index(currentIndex+grouped,0),index(currentIndex+grouped,columnsCount-1));
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

