//  This file is part of Qt Bitcion Trader
//      https://github.com/JulyIGHOR/QtBitcoinTrader
//  Copyright (C) 2013-2015 July IGHOR <julyighor@gmail.com>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  In addition, as a special exception, the copyright holders give
//  permission to link the code of portions of this program with the
//  OpenSSL library under certain conditions as described in each
//  individual source file, and distribute linked combinations including
//  the two.
//
//  You must obey the GNU General Public License in all respects for all
//  of the code used other than OpenSSL. If you modify file(s) with this
//  exception, you may extend this exception to your version of the
//  file(s), but you are not obligated to do so. If you do not wish to do
//  so, delete this exception statement from your version. If you delete
//  this exception statement from all source files in the program, then
//  also delete it here.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "chartsview.h"
#include "main.h"

ChartsView::ChartsView():QWidget()
{
    ui.setupUi(this);
    ui.graphicsView->scale(1,-1);

    sceneCharts=new QGraphicsScene;
    ui.graphicsView->setScene(sceneCharts);
    drawText(julyTr("CHARTS_WAITING_FOR_DATA","Waiting for data..."),0,0,"font-size:28px;color:"+baseValues.appTheme.gray.name());

    chartsModel=new ChartsModel;

    refreshTimer=new QTimer;
    connect(refreshTimer,SIGNAL(timeout()),this,SLOT(refreshCharts()));
    refreshTimer->start(5000);
    lastResize=0;
}

ChartsView::~ChartsView()
{
    delete chartsModel;
    delete sceneCharts;
    delete refreshTimer;
}

void ChartsView::resizeEvent(QResizeEvent *event)
{
    event->accept();
    qint32 nowTime=QTime::currentTime().msecsSinceStartOfDay();
    qint32 deltaTime=nowTime-lastResize;
    if(deltaTime>500 || deltaTime<0){
        refreshCharts();
        lastResize=nowTime;
    }
}

void ChartsView::drawText(QString text, qint16 x, qint16 y, QString style)
{
    QGraphicsTextItem *textItem = new QGraphicsTextItem(text);
    textItem->setPos(x,y);
    textItem->setTransform(QTransform::fromScale(1,-1));
    if(style.size())textItem->setHtml("<span style='"+style+"'>"+text+"</span>");
    sceneCharts->addItem(textItem);
}

void ChartsView::explainColor(QString text,qint16 x,qint16 y,QColor color)
{
    drawText(text,x,y);
    sceneCharts->addRect(x-11,y-16,10,10,QPen(Qt::black),QBrush(color));
}

void ChartsView::drawRect(qint16 x,qint16 h)
{
    int w=20;
    sceneCharts->addRect(x-w/2,1,w,h,QPen(Qt::lightGray),QBrush(Qt::lightGray));
}

/*void ChartsView::drawCandle(ChartsItem *chartsItem,qint16 number,qint16 halfWidth)
{
    sceneCharts->addLine(number,chartsItem->min,number,chartsItem->max);
    sceneCharts->addRect(number-halfWidth,chartsItem->first,halfWidth*2,chartsItem->last-chartsItem->first,QPen(Qt::black),QBrush(Qt::black));
}*/

bool ChartsView::prepareCharts()
{
    int width=ui.graphicsView->width()-4;
    int height=ui.graphicsView->height()-4;
    if(width<550)width=550;
    if(height<220)height=220;

    if(!chartsModel->prepareChartsData(width,height))return false;
    int graphWidth=chartsModel->chartsWidth;
    int graphHeight=chartsModel->chartsHeight;

    sceneCharts->clear();
    sceneCharts->setSceneRect(-chartsModel->widthAmountYAxis-2,-27,width,height);

    sceneCharts->addLine(0,0,graphWidth,0,QPen("#777777"));
    sceneCharts->addLine(0,0,0,graphHeight,QPen("#777777"));
    sceneCharts->addLine(graphWidth,0,graphWidth,graphHeight,QPen("#777777"));

    explainColor("- "+julyTr("CHARTS_BUY","Buy"),0.11*graphWidth,graphHeight+20,"#0000FF");
    explainColor("- "+julyTr("CHARTS_SELL","Sell"),0.36*graphWidth,graphHeight+20,"#FF0000");
    explainColor("- "+julyTr("CHARTS_SPREAD","Spread"),0.61*graphWidth,graphHeight+20,"#BFFFBF");
    explainColor("- "+julyTr("CHARTS_AMOUNT","Amount"),0.86*graphWidth,graphHeight+20,"#777777");

    for(qint32 i=0;i<chartsModel->graphDateText.count();i++){
        sceneCharts->addLine(chartsModel->graphDateTextX.at(i),-5,chartsModel->graphDateTextX.at(i),0,QPen("#777777"));
        drawText(chartsModel->graphDateText.at(i),chartsModel->graphDateTextX.at(i)-17,-5);
    }

    for(qint32 i=0;i<chartsModel->graphAmountText.count();i++){
        sceneCharts->addLine(-5,chartsModel->graphAmountTextY.at(i),0,chartsModel->graphAmountTextY.at(i),QPen("#777777"));
        drawText(chartsModel->graphAmountText.at(i),-chartsModel->widthAmountYAxis,chartsModel->graphAmountTextY.at(i)+11);
    }

    for(qint32 i=0;i<chartsModel->graphPriceText.count();i++){
        if(i>0)sceneCharts->addLine(1,chartsModel->graphPriceTextY.at(i),graphWidth-1,chartsModel->graphPriceTextY.at(i),QPen("#DDDDDD"));
        sceneCharts->addLine(graphWidth,chartsModel->graphPriceTextY.at(i),graphWidth+5,chartsModel->graphPriceTextY.at(i),QPen("#777777"));
        drawText(chartsModel->graphPriceText.at(i),graphWidth+7,chartsModel->graphPriceTextY.at(i)+11);
    }

    return true;
}

void ChartsView::clearCharts()
{
    refreshTimer->stop();
    sceneCharts->clear();
    drawText(julyTr("CHARTS_WAITING_FOR_DATA","Waiting for data..."),0,0,"font-size:28px;color:"+baseValues.appTheme.gray.name());
    sceneCharts->setSceneRect(sceneCharts->itemsBoundingRect());
    refreshTimer->start(5000);
}

void ChartsView::refreshCharts()
{
    if(!prepareCharts()){
        sceneCharts->clear();
        drawText(julyTr("CHARTS_WAITING_FOR_DATA","Waiting for data..."),0,0,"font-size:28px;color:"+baseValues.appTheme.gray.name());
        return;
    }

    for(qint32 i=0;i<chartsModel->graphAmountY.count();i++){
        drawRect(chartsModel->graphAmountX.at(i),chartsModel->graphAmountY.at(i));
    }

    /*for(qint32 i=1;i<chartsModel->graphBoundsSellX.count();i++){
        sceneCharts->addLine(chartsModel->graphBoundsSellX.at(i-1),chartsModel->graphBoundsSellY.at(i-1),chartsModel->graphBoundsSellX.at(i),chartsModel->graphBoundsSellY.at(i),QPen("#000000"));
    }

    for(qint32 i=1;i<chartsModel->graphBoundsBuyX.count();i++){
        sceneCharts->addLine(chartsModel->graphBoundsBuyX.at(i-1),chartsModel->graphBoundsBuyY.at(i-1),chartsModel->graphBoundsBuyX.at(i),chartsModel->graphBoundsBuyY.at(i),QPen("#000000"));
    }*/
    QPolygonF boundsPolygon;
    for(qint32 i=0;i<chartsModel->graphBoundsSellX.count();i++){
        boundsPolygon << QPointF(chartsModel->graphBoundsSellX.at(i), chartsModel->graphBoundsSellY.at(i));
    }
    for(qint32 i=chartsModel->graphBoundsBuyX.count()-1;i>=0;i--){
        boundsPolygon << QPointF(chartsModel->graphBoundsBuyX.at(i), chartsModel->graphBoundsBuyY.at(i));
    }
    if(boundsPolygon.count())sceneCharts->addPolygon(boundsPolygon,QPen("#4000FF00"),QBrush("#4000FF00"));

    if(chartsModel->graphTradesX.count()){
        QPen pen;
        if(chartsModel->graphTradesType[0]==1)pen=QPen("#FF0000");else pen=QPen("#0000FF");
        sceneCharts->addEllipse(chartsModel->graphTradesX.at(0)-2,chartsModel->graphTradesY.at(0)-2,4,4,pen);
        for(qint32 i=1;i<chartsModel->graphTradesX.count();i++){
            if(chartsModel->graphTradesType[i]==1)pen=QPen("#FF0000");else pen=QPen("#0000FF");
            sceneCharts->addLine(chartsModel->graphTradesX.at(i-1),chartsModel->graphTradesY.at(i-1),chartsModel->graphTradesX.at(i),chartsModel->graphTradesY.at(i),pen);
            sceneCharts->addEllipse(chartsModel->graphTradesX.at(i)-2,chartsModel->graphTradesY.at(i)-2,4,4,pen);
        }
    }
}
