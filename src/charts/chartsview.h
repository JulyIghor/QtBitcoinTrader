//  This file is part of Qt Bitcion Trader
//      https://github.com/JulyIGHOR/QtBitcoinTrader
//  Copyright (C) 2013-2016 July IGHOR <julyighor@gmail.com>
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

#ifndef CHARTSVIEW_H
#define CHARTSVIEW_H

#include <QWidget>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QTimer>
#include <QElapsedTimer>
#include "charts/chartsmodel.h"
//#include "charts/chartsdata.h"
#include "ui_chartsview.h"

class ChartsView : public QWidget
{
    Q_OBJECT

public:
    ChartsModel *chartsModel;
    //ChartsData *chartsData;
    bool isVisible;
    bool sizeIsChanged;

    ChartsView();
    ~ChartsView();
    void clearCharts();
    void comeNewData();

private:
    Ui::ChartsView ui;
    quint16 fontHeight;
    quint16 fontHeightHalf;
    QGraphicsScene *sceneCharts;
    QGraphicsScene *leftSceneCharts;
    QGraphicsScene *rightSceneCharts;
    QGraphicsScene *bottomSceneCharts;
    QTimer refreshTimer;
    QElapsedTimer perfomanceTimer;
    qint32 timeRefreshCharts;
    qint32 lastResize;
    qint32 lastNewData;

    void resizeEvent(QResizeEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void drawText(QGraphicsScene*,QString, qint16 x=0, qint16 y=0, QString style="");
    void explainColor(QString,qint16,qint16,QColor);
    bool prepareCharts();
    void drawRect(qint16,qint16);
    //void drawCandle(ChartsItem *,qint16,qint16);

public slots:
    void refreshCharts();
};

#endif // CHARTSVIEW_H
