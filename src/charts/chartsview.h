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

#ifndef CHARTSVIEW_H
#define CHARTSVIEW_H

#include <QElapsedTimer>
#include <QWidget>

class QGraphicsScene;
class QElapsedTimer;
class ChartsModel;

namespace Ui
{
    class ChartsView;
}

class ChartsView : public QWidget
{
    Q_OBJECT
public:
    QScopedPointer<ChartsModel> chartsModel;

    ChartsView();
    ~ChartsView();

    void clearCharts();
    void comeNewData();

public slots:
    void refreshCharts();
    void visibilityChanged(bool visible);

private:
    QScopedPointer<Ui::ChartsView> ui;
    bool isChartsVisible;
    bool sizeIsChanged;
    int fontHeight;
    int fontHeightHalf;
    QScopedPointer<QGraphicsScene> sceneCharts;
    QScopedPointer<QGraphicsScene> leftSceneCharts;
    QScopedPointer<QGraphicsScene> rightSceneCharts;
    QScopedPointer<QGraphicsScene> bottomSceneCharts;
    QScopedPointer<QTimer> refreshTimer;
    QScopedPointer<QElapsedTimer> perfomanceTimer;
    qint32 timeRefreshCharts;
    qint32 lastResize;
    qint32 lastNewData;

    void resizeEvent(QResizeEvent* event);
    void drawText(QGraphicsScene*, QString, qint16 x = 0, qint16 y = 0, QString style = "");
    void explainColor(QString, qint16, qint16, QColor);
    bool prepareCharts();
    void drawRect(qint16, qint16);
};

#endif // CHARTSVIEW_H
