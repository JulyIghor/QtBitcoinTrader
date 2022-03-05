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

#include "julyspinboxpicker.h"
#include "main.h"
#include <QApplication>
#include <QDoubleSpinBox>
#include <QMouseEvent>
#include <QScreen>
#include <QtCore/qmath.h>

JulySpinBoxPicker::JulySpinBoxPicker(QDoubleSpinBox* parent, double* forceMinValue, double intMinV) : QLabel()
{
    internalMinimumValue = intMinV;

    if (internalMinimumValue > 0.0 && forceMinValue == nullptr)
        forceMinimumValue = &internalMinimumValue;
    else
        forceMinimumValue = forceMinValue;

    scrollDirection = 0;
    parentSpinBox = parent;
    setScaledContents(true);
    setFixedSize(11, 16);
    j_debugMode = false;
    setIcon(0);
    setCursor(QCursor(Qt::OpenHandCursor));
}

JulySpinBoxPicker::~JulySpinBoxPicker()
{
}

void JulySpinBoxPicker::setIcon(int state)
{
    static QPixmap mouse("://Resources/Mouse.png");
    static QPixmap mouseDrag("://Resources/MouseDrag.png");
    static QPixmap mouseDragLeftRight("://Resources/MouseDragLeftRight.png");
    static QPixmap mouseDragUpDown("://Resources/MouseDragUpDown.png");

    switch (state)
    {
    case -1:
        setPixmap(mouseDragUpDown);
        break;

    case 0:
        setPixmap(mouse);
        break;

    case 1:
        setPixmap(mouseDragLeftRight);
        break;

    case 2:
        setPixmap(mouseDrag);
        break;
    }
}

void JulySpinBoxPicker::mousePressEvent(QMouseEvent* event)
{
    event->accept();

    if (event->button() == Qt::LeftButton)
    {
        if (!j_debugMode)
            setCursor(QCursor(Qt::BlankCursor));

        j_cursorLastPos = QCursor::pos();
        j_cursorLastMove = QCursor::pos();
        j_isPressing = true;
        scrollDirection = 0;
        setIcon(2);

        currentScreenRect = QApplication::screenAt(j_cursorLastPos)->geometry();

        maximumValue = 10.0;

        if (parentSpinBox->accessibleName() == "USD" || parentSpinBox->accessibleName() == "PRICE")
            maximumValue = baseValues.currentPair.currBInfo.valueSmall;

        if (parentSpinBox->accessibleName() == "BTC")
            maximumValue = baseValues.currentPair.currAInfo.valueSmall;

        if (forceMinimumValue)
            minimumValue = *forceMinimumValue;
        else
            minimumValue = qPow(0.1, parentSpinBox->decimals());
    }
}

void JulySpinBoxPicker::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        j_isPressing = false;
        setIcon(0);
        QCursor::setPos(j_cursorLastPos);

        if (!j_debugMode)
            setCursor(QCursor(Qt::OpenHandCursor));
    }

    event->accept();
}

void JulySpinBoxPicker::mouseMoveEvent(QMouseEvent* event)
{
    if (j_isPressing)
    {
        int t_x = QCursor::pos().x();
        int t_y = QCursor::pos().y();
        int t_deltaX = t_x - j_cursorLastMove.x();
        int t_deltaY = t_y - j_cursorLastMove.y();

        j_cursorLastMove.setX(t_x);
        j_cursorLastMove.setY(t_y);

        if ((qAbs(t_deltaX) < 100) && (qAbs(t_deltaY) < 100))
        {
            if (scrollDirection == 0)
            {
                if (qAbs(t_deltaX) > qAbs(t_deltaY))
                    scrollDirection = 1;
                else
                    scrollDirection = -1;

                setIcon(scrollDirection);
            }

            if (scrollDirection == -1)
            {
                int tempValue = t_deltaX;
                t_deltaX = -t_deltaY;
                t_deltaY = tempValue;
            }

            double valueToChange = t_deltaX;

            if (QApplication::keyboardModifiers() == Qt::NoModifier)
                valueToChange /= 100.0;

            valueToChange *= maximumValue;

            parentSpinBox->setValue(parentSpinBox->value() + valueToChange);
        }

        int t_deltaXY = 100;

        if ((t_x < currentScreenRect.left() + t_deltaXY) || (t_y < currentScreenRect.top() + t_deltaXY) ||
            (t_x > currentScreenRect.right() - t_deltaXY) || (t_y > currentScreenRect.bottom() - t_deltaXY))
            QCursor::setPos(currentScreenRect.center());
    }

    event->accept();
}
