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

#include "percentpicker.h"

PercentPicker::PercentPicker(QDoubleSpinBox* _spinBox, double _maxValue) : QMenu()
{
    maxValue = _maxValue;
    spinBox = _spinBox;

    if (maxValue == 0.0)
        maxValue = 0.000000001;

    ui.setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose, true);
    setFixedSize(minimumSizeHint().width(), 200);
    ui.verticalSlider->setValue(spinBox->value() * 100.0 / maxValue);
    ui.spinBox->setFocus();
    ui.spinBox->selectAll();
}

PercentPicker::~PercentPicker()
{
}

void PercentPicker::mouseReleaseEvent(QMouseEvent* event)
{
    event->ignore();
}

void PercentPicker::keyPressEvent(QKeyEvent* event)
{
    event->accept();

    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter || event->key() == Qt::Key_Escape)
        close();
}

void PercentPicker::on_percentTo1_clicked()
{
    if (ui.percentTo1->text() == "1%")
    {
        ui.verticalSlider->setValue(1);
        ui.percentTo1->setText("100%");
    }
    else if (ui.percentTo1->text() == "100%")
    {
        ui.verticalSlider->setValue(100);
        ui.percentTo1->setText("1%");
    }
}

void PercentPicker::on_verticalSlider_valueChanged(int val)
{
    if (val == 1)
        ui.percentTo1->setText("100%");
    else if (val == 100)
        ui.percentTo1->setText("1%");

    if (isVisible())
        spinBox->setValue((double)val * maxValue / 100.0);
}
