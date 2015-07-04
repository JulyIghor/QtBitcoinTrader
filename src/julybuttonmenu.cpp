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

#include "julybuttonmenu.h"
#include <QToolButton>
#include <QMouseEvent>
#include <QCursor>

JulyButtonMenu::JulyButtonMenu(QToolButton *_parentButton, Align _position) :
    QMenu()
{
    position=_position;
    parentButton=_parentButton;
    widgetUnderButton=parentButton->parentWidget();
    parentWindow=widgetUnderButton;
    while(parentWindow->parentWidget())
        parentWindow=parentWindow->parentWidget();

    parentButton->setCheckable(true);
    connect(this,SIGNAL(setCheckedButton(bool)),parentButton,SLOT(setChecked(bool)));
    connect(this,SIGNAL(aboutToHide()),this,SLOT(aboutToHideWindow()));
    connect(parentButton,SIGNAL(clicked()),this,SLOT(displayMenuClicked()));
}

void JulyButtonMenu::displayMenuClicked()
{
    resize(minimumSizeHint());
    QPoint pointToShow;
    if(widgetUnderButton==0)pointToShow=QCursor::pos();
    else
    {
    if(position==Left)
        pointToShow=parentButton->geometry().bottomLeft();
    else
    {
        pointToShow=parentButton->geometry().bottomRight();
        pointToShow.setX(pointToShow.x()-this->width());
    }

    pointToShow=widgetUnderButton->mapToGlobal(pointToShow);
    exec(pointToShow);return;
    if(pointToShow.x()<parentWindow->geometry().x())pointToShow.setX(parentWindow->geometry().x());
    if(pointToShow.y()<parentWindow->geometry().y())pointToShow.setY(parentWindow->geometry().y());
    if(pointToShow.x()>parentWindow->geometry().right()-this->width())pointToShow.setX(parentWindow->geometry().right()-this->width());
    if(pointToShow.y()>parentWindow->geometry().bottom()-this->height())pointToShow.setY(parentWindow->geometry().bottom()-this->height());
    }
    exec(pointToShow);
}

void JulyButtonMenu::aboutToHideWindow()
{
    emit setCheckedButton(false);
}

void JulyButtonMenu::mouseReleaseEvent(QMouseEvent *event)
{
    event->ignore();
}
