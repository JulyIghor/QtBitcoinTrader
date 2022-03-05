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

#include "apptheme.h"
#include "main.h"
#include <QSettings>

AppTheme::AppTheme()
{
    gray = Qt::gray;
    altRowColor = QColor(240, 240, 240);
    lightGray = Qt::lightGray;
    red = Qt::red;
    green = Qt::green;
    blue = Qt::blue;
    lightRed.setRgb(255, 200, 200);
    lightGreen.setRgb(200, 255, 200);
    lightBlue.setRgb(200, 200, 255);
    lightGreenBlue.setRgb(200, 255, 255);
    lightRedBlue.setRgb(255, 200, 255);
    darkRedBlue.setRgb(155, 100, 155);
    lightRedGreen.setRgb(255, 255, 200);
    darkRed = Qt::darkRed;
    darkGreen = Qt::darkGreen;
    darkBlue = Qt::darkBlue;
    black = Qt::black;
    white = Qt::white;
}

QColor AppTheme::getColor(const QString& str) const
{
    QStringList colorList = str.split(",");

    if (colorList.size() < 3)
        return Qt::black;

    if (colorList.size() < 4)
        colorList << "255";

    return QColor(colorList.at(0).toInt(), colorList.at(1).toInt(), colorList.at(2).toInt(), colorList.at(3).toInt());
}

void AppTheme::loadTheme(const QString& name)
{
    QSettings themeLoad(baseValues.themeFolder + "/" + name + ".thm", QSettings::IniFormat);

    themeLoad.beginGroup("Normal");
    QStringList colorList = themeLoad.childKeys();
    themeLoad.endGroup();

    for (int n = 0; n < colorList.size(); n++)
    {
        QStringList split_ = colorList.at(n).split("_");

        if (split_.size() < 2)
            continue;

        split_.removeFirst();
        int colNum = split_.first().toInt();

        if (colNum < 0 || colNum >= 20)
            continue;

        palette.setColor(QPalette::Normal, QPalette::ColorRole(colNum), getColor(themeLoad.value("Normal/" + colorList.at(n)).toString()));
    }

    themeLoad.beginGroup("Disabled");
    colorList = themeLoad.childKeys();
    themeLoad.endGroup();

    for (int n = 0; n < colorList.size(); n++)
    {
        QStringList split_ = colorList.at(n).split("_");

        if (split_.size() < 2)
            continue;

        split_.removeFirst();
        int colNum = split_.first().toInt();

        if (colNum < 0 || colNum >= 20)
            continue;

        palette.setColor(
            QPalette::Disabled, QPalette::ColorRole(colNum), getColor(themeLoad.value("Disabled/" + colorList.at(n)).toString()));
    }

    themeLoad.beginGroup("Inactive");
    colorList = themeLoad.childKeys();
    themeLoad.endGroup();

    for (int n = 0; n < colorList.size(); n++)
    {
        QStringList split_ = colorList.at(n).split("_");

        if (split_.size() < 2)
            continue;

        split_.removeFirst();
        int colNum = split_.first().toInt();

        if (colNum < 0 || colNum >= 20)
            continue;

        palette.setColor(
            QPalette::Inactive, QPalette::ColorRole(colNum), getColor(themeLoad.value("Inactive/" + colorList.at(n)).toString()));
    }

    altRowColor = palette.color(QPalette::AlternateBase);
    gray = getColor(themeLoad.value("Gray").toString());
    red = getColor(themeLoad.value("Red").toString());
    green = getColor(themeLoad.value("Green").toString());
    blue = getColor(themeLoad.value("Blue").toString());
    lightRed = getColor(themeLoad.value("LightRed").toString());
    lightGreen = getColor(themeLoad.value("LightGreen").toString());
    lightBlue = getColor(themeLoad.value("LightRedBlue").toString());
    lightGreenBlue = getColor(themeLoad.value("LightGreenBlue").toString());
    lightRedBlue = getColor(themeLoad.value("LightRedBlue").toString());
    darkRedBlue = getColor(themeLoad.value("DarkRedBlue").toString());
    lightRedGreen = getColor(themeLoad.value("LightRedGreen").toString());
    darkRed = getColor(themeLoad.value("DarkRed").toString());
    darkGreen = getColor(themeLoad.value("DarkGreen").toString());
    darkBlue = getColor(themeLoad.value("DarkBlue").toString());
    black = getColor(themeLoad.value("Black").toString());
    white = getColor(themeLoad.value("White").toString());

    palette.setColor(QPalette::Text, black);

    styleSheet = "QHeaderView::section {color: " + black.name() +
                 ";}"
                 "QToolButton {color: " +
                 black.name() +
                 ";}"
                 "QPushButton {color: " +
                 black.name() +
                 ";}"
                 "QGroupBox {background: rgba(255,255,255,60); color: " +
                 black.name() + "; border: 1px solid " + gray.name() +
                 ";border-radius: 3px;margin-top: 8px;}"
                 "QGroupBox:title {subcontrol-origin: margin; position: relative; left: 6px; color: " +
                 black.name() + "; background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 transparent, stop: 0.45 " +
                 white.name() + ", stop: 0.5 " + white.name() + ", stop: 0.55 " + white.name() +
                 ", stop: 1.0 transparent);  padding-left: 2px;} QLabel {color: " + black.name() +
                 ";}"
                 "QTabBar::tab {color: " +
                 black.name() +
                 ";}"
                 "QRadioButton {color: " +
                 black.name() +
                 ";}"
                 "QDoubleSpinBox {background: " +
                 white.name() +
                 ";}"
                 "QTextEdit {background: " +
                 white.name() +
                 ";}"
                 "QPlainTextEdit {background: " +
                 white.name() +
                 ";}"
                 "QCheckBox {color: " +
                 black.name() +
                 ";}"
                 "QLineEdit {color: " +
                 black.name() + "; background: " + white.name() + "; border: 1px solid " + gray.name() +
                 ";}"
                 "*[IsDockable] {background: rgba(255,255,255,40); border: 1px solid " +
                 gray.name() +
                 "; border-radius: 3px;margin-top:2px}"
                 "QDockWidget::title {background: rgba(255,255,255,100); border: 1px solid " +
                 gray.name() +
                 "; border-radius: 2px; padding:3px; text-align: left center;}"
                 "QDockWidget::close-button {top:2px;right:3px}"
                 "QDockWidget::float-button {top:2px;right:18px}";
}
