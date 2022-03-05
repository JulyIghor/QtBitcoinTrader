//  This file is part of Qt Bitcoin Trader
//      https://github.com/JulyIGHOR/QtBitcoinTrader
//  Copyright (C) 2013-2022 July Ighor <julyighor@gmail.com>
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

#include "charts/chartsview.h"
#include "charts/chartsmodel.h"
#include "main.h"
#include "ui_chartsview.h"
#include <QElapsedTimer>
#include <QGraphicsScene>
#include <QGraphicsTextItem>

ChartsView::ChartsView() :
    QWidget(),
    ui(new Ui::ChartsView),
    isChartsVisible(false),
    sizeIsChanged(false),
    refreshTimer(new QTimer),
    perfomanceTimer(new QElapsedTimer),
    timeRefreshCharts(5000),
    lastResize(0),
    lastNewData(0)
{
    ui->setupUi(this);
    ui->chartsBorder->setMinimumSize(550, 220);

    QFontMetrics fontMetrics(QApplication::font());
    fontHeight = fontMetrics.height();
    fontHeightHalf = fontHeight / 2 + 1;

    setStyleSheet("background: " + baseValues.appTheme.white.name());
    QPixmap descriptionPixmap(QSize(11, 11));
    QPainter descriptionPainter(&descriptionPixmap);
    descriptionPainter.setPen(QPen(Qt::black));

    descriptionPainter.setBrush(QBrush("#0000FF"));
    descriptionPainter.drawRect(0, 0, 10, 10);
    ui->descriptionBuyImgLabel->setPixmap(descriptionPixmap);
    ui->descriptionBuyLabel->setText("- " + julyTr("CHARTS_BUY", "Buy"));

    descriptionPainter.setBrush(QBrush("#FF0000"));
    descriptionPainter.drawRect(0, 0, 10, 10);
    ui->descriptionSellImgLabel->setPixmap(descriptionPixmap);
    ui->descriptionSellLabel->setText("- " + julyTr("CHARTS_SELL", "Sell"));

    descriptionPainter.setBrush(QBrush("#BFFFBF"));
    descriptionPainter.drawRect(0, 0, 10, 10);
    ui->descriptionSpreadImgLabel->setPixmap(descriptionPixmap);
    ui->descriptionSpreadLabel->setText("- " + julyTr("CHARTS_SPREAD", "Spread"));

    descriptionPainter.setBrush(QBrush("#777777"));
    descriptionPainter.drawRect(0, 0, 10, 10);
    ui->descriptionAmountImgLabel->setPixmap(descriptionPixmap);
    ui->descriptionAmountLabel->setText("- " + julyTr("CHARTS_AMOUNT", "Amount"));

    sceneCharts.reset(new QGraphicsScene);
    ui->graphicsView->scale(1, -1);
    ui->graphicsView->setScene(sceneCharts.data());
    leftSceneCharts.reset(new QGraphicsScene);
    ui->leftGraphicsView->scale(1, -1);
    ui->leftGraphicsView->setScene(leftSceneCharts.data());
    ui->leftGraphicsView->setFixedWidth(0);
    rightSceneCharts.reset(new QGraphicsScene);
    ui->rightGraphicsView->scale(1, -1);
    ui->rightGraphicsView->setScene(rightSceneCharts.data());
    ui->rightGraphicsView->setFixedWidth(0);
    bottomSceneCharts.reset(new QGraphicsScene);
    ui->bottomGraphicsView->scale(1, -1);
    ui->bottomGraphicsView->setScene(bottomSceneCharts.data());
    ui->bottomGraphicsView->setFixedHeight(fontHeight + 3);
    drawText(sceneCharts.data(),
             julyTr("CHARTS_WAITING_FOR_DATA", "Waiting for data..."),
             0,
             0,
             "font-size:28px;color:" + baseValues.appTheme.gray.name());

    chartsModel.reset(new ChartsModel);

    connect(refreshTimer.data(), &QTimer::timeout, this, &ChartsView::refreshCharts);
    refreshTimer->start(timeRefreshCharts);
}

ChartsView::~ChartsView()
{
}

void ChartsView::visibilityChanged(bool visible)
{
    isChartsVisible = visible;

    if (visible)
    {
        if (sizeIsChanged)
        {
            sizeIsChanged = false;
            refreshCharts();
        }
        else
            comeNewData();
    }
}

void ChartsView::resizeEvent(QResizeEvent* event)
{
    event->accept();

    if (!isChartsVisible)
    {
        sizeIsChanged = true;
        return;
    }

    qint32 nowTime = QTime::currentTime().msecsSinceStartOfDay();
    qint32 deltaTime = nowTime - lastResize;

    if (deltaTime > 500 || deltaTime < 0)
    {
        refreshTimer->stop();
        refreshCharts();
        lastResize = nowTime;
        refreshTimer->start(timeRefreshCharts);
    }
}

void ChartsView::comeNewData()
{
    if (!isChartsVisible)
        return;

    qint32 nowTime = QTime::currentTime().msecsSinceStartOfDay();
    qint32 deltaTime = nowTime - lastNewData;

    if (deltaTime > 1000 || deltaTime < 0)
    {
        refreshTimer->stop();
        refreshCharts();
        lastNewData = nowTime;
        refreshTimer->start(timeRefreshCharts);
    }
}

void ChartsView::drawText(QGraphicsScene* scene, QString text, qint16 x, qint16 y, QString style)
{
    if (style.size())
    {
        auto* textItem = new QGraphicsTextItem();
        textItem->setHtml("<span style='" + style + "'>" + text + "</span>");
        textItem->setPos(x, y);
        textItem->setTransform(QTransform::fromScale(1, -1));
        scene->addItem(textItem);
    }
    else
    {
        auto* textItem = new QGraphicsSimpleTextItem(text);
        textItem->setPos(x, y);
        textItem->setTransform(QTransform::fromScale(1, -1));
        scene->addItem(textItem);
    }
}

void ChartsView::drawRect(qint16 x, qint16 h)
{
    int w = 20;
    sceneCharts->addRect(x - w / 2, 1, w, h, QPen(Qt::lightGray), QBrush(Qt::lightGray));
}

bool ChartsView::prepareCharts()
{
    if (!chartsModel->prepareChartsData(ui->bottomGraphicsView->width(), ui->graphicsView->height() - fontHeightHalf))
        return false;

    bottomSceneCharts->clear();
    bottomSceneCharts->setSceneRect(1, -fontHeight - 5, ui->bottomGraphicsView->width(), ui->bottomGraphicsView->height());

    for (qint32 i = 0; i < chartsModel->graphDateText.size(); ++i)
    {
        drawText(bottomSceneCharts.data(),
                 chartsModel->graphDateText.at(i),
                 chartsModel->graphDateTextX.at(i) - 12 + chartsModel->widthAmountYAxis,
                 -4);
    }

    leftSceneCharts->clear();
    ui->leftGraphicsView->setFixedWidth(chartsModel->widthAmountYAxis);
    leftSceneCharts->setSceneRect(1, -5, ui->leftGraphicsView->width(), ui->leftGraphicsView->height());

    for (qint32 i = 0; i < chartsModel->graphAmountText.size(); ++i)
    {
        leftSceneCharts->addLine(ui->leftGraphicsView->width() - 4,
                                 chartsModel->graphAmountTextY.at(i),
                                 ui->leftGraphicsView->width(),
                                 chartsModel->graphAmountTextY.at(i),
                                 QPen("#777777"));
        drawText(leftSceneCharts.data(), chartsModel->graphAmountText.at(i), 5, chartsModel->graphAmountTextY.at(i) + 7);
    }

    rightSceneCharts->clear();
    ui->rightGraphicsView->setFixedWidth(chartsModel->widthPriceYAxis);
    rightSceneCharts->setSceneRect(1, -5, ui->rightGraphicsView->width(), ui->rightGraphicsView->height());

    for (qint32 i = 0; i < chartsModel->graphPriceText.size(); ++i)
    {
        rightSceneCharts->addLine(1, chartsModel->graphPriceTextY.at(i), 5, chartsModel->graphPriceTextY.at(i), QPen("#777777"));
        drawText(rightSceneCharts.data(), chartsModel->graphPriceText.at(i), 11, chartsModel->graphPriceTextY.at(i) + 7);
    }

    ui->graphicsView->setScene(nullptr);
    sceneCharts.reset(new QGraphicsScene);
    ui->graphicsView->setScene(sceneCharts.data());

    int graphWidth = chartsModel->chartsWidth;
    int graphHeight = chartsModel->chartsHeight;
    sceneCharts->setSceneRect(0, -5, graphWidth + 1, ui->graphicsView->height());
    sceneCharts->addLine(0, 0, graphWidth, 0, QPen("#777777"));
    sceneCharts->addLine(0, 0, 0, graphHeight, QPen("#777777"));
    sceneCharts->addLine(graphWidth, 0, graphWidth, graphHeight, QPen("#777777"));

    for (qint32 i = 0; i < chartsModel->graphDateText.size(); ++i)
    {
        sceneCharts->addLine(chartsModel->graphDateTextX.at(i), -5, chartsModel->graphDateTextX.at(i), 0, QPen("#777777"));
    }

    for (qint32 i = 1; i < chartsModel->graphPriceText.size(); ++i)
    {
        sceneCharts->addLine(1, chartsModel->graphPriceTextY.at(i), graphWidth - 1, chartsModel->graphPriceTextY.at(i), QPen("#DDDDDD"));
    }

    return true;
}

void ChartsView::clearCharts()
{
    refreshTimer->stop();
    bottomSceneCharts->clear();
    leftSceneCharts->clear();
    rightSceneCharts->clear();
    sceneCharts->clear();
    drawText(sceneCharts.data(),
             julyTr("CHARTS_WAITING_FOR_DATA", "Waiting for data..."),
             0,
             0,
             "font-size:28px;color:" + baseValues.appTheme.gray.name());
    sceneCharts->setSceneRect(sceneCharts->itemsBoundingRect());
    refreshTimer->start(timeRefreshCharts);
}

void ChartsView::refreshCharts()
{
    if (!isChartsVisible)
        return;

    perfomanceTimer->start();

    if (!prepareCharts())
    {
        sceneCharts->clear();
        bottomSceneCharts->clear();
        leftSceneCharts->clear();
        rightSceneCharts->clear();
        drawText(sceneCharts.data(),
                 julyTr("CHARTS_WAITING_FOR_DATA", "Waiting for data..."),
                 0,
                 0,
                 "font-size:28px;color:" + baseValues.appTheme.gray.name());
        return;
    }

    for (qint32 i = 0; i < chartsModel->graphAmountY.size(); ++i)
    {
        drawRect(chartsModel->graphAmountX.at(i), chartsModel->graphAmountY.at(i));
    }

    QPolygonF boundsPolygon;

    for (qint32 i = 0; i < chartsModel->graphBoundsSellX.size(); ++i)
    {
        boundsPolygon << QPointF(chartsModel->graphBoundsSellX.at(i), chartsModel->graphBoundsSellY.at(i));
    }

    for (qint32 i = chartsModel->graphBoundsBuyX.size() - 1; i >= 0; --i)
    {
        boundsPolygon << QPointF(chartsModel->graphBoundsBuyX.at(i), chartsModel->graphBoundsBuyY.at(i));
    }

    if (!boundsPolygon.empty())
        sceneCharts->addPolygon(boundsPolygon, QPen("#4000FF00"), QBrush("#4000FF00"));

    QPolygonF tradesPolygon;

    if (!chartsModel->graphTradesX.empty())
    {
        for (qint32 i = 0; i < chartsModel->graphTradesX.size(); ++i)
        {
            tradesPolygon << QPointF(chartsModel->graphTradesX.at(i), chartsModel->graphTradesY.at(i));
        }
    }

    if (!tradesPolygon.empty())
    {
        QPainterPath tradesPath;
        tradesPath.addPolygon(tradesPolygon);
        sceneCharts->addPath(tradesPath, QPen("#555555"));
    }

    QPen pen;

    if (!chartsModel->graphTradesX.empty())
    {
        for (qint32 i = 0; i < chartsModel->graphTradesX.size(); ++i)
        {
            if (chartsModel->graphTradesType[i] == 1)
                pen = QPen("#FF0000");
            else
                pen = QPen("#0000FF");

            sceneCharts->addEllipse(chartsModel->graphTradesX.at(i) - 2, chartsModel->graphTradesY.at(i) - 2, 4, 4, pen);
        }
    }

    qint64 perfDeltaTime = perfomanceTimer->elapsed();

    if (perfDeltaTime > 50 && chartsModel->perfomanceStep < 10)
        ++chartsModel->perfomanceStep;
    else
    {
        if (perfDeltaTime < 30 && chartsModel->perfomanceStep > 1)
            --chartsModel->perfomanceStep;
    }
}
