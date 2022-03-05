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

#include "settingsdialoglistelement.h"
#include <QPixmap>

SettingsDialogListElement::SettingsDialogListElement(SettingsDialog* tempParent,
                                                     qint32 tempIndex,
                                                     const QString& name,
                                                     const QString& icon) :
    QWidget(tempParent)
{
    ui.setupUi(this);
    ui.textListLabel->setText(name);
    ui.iconListLabel->setPixmap(QPixmap(":/Resources/" + icon));
    setCursor(Qt::PointingHandCursor);

    parent = tempParent;
    index = tempIndex;

    QFontMetrics fontMetrics(ui.textListLabel->font());
#if QT_VERSION < 0x060000
    width = fontMetrics.horizontalAdvance(name) + ui.iconListLabel->pixmap()->width() + 17;
    setFixedHeight(qMax(fontMetrics.height(), ui.iconListLabel->pixmap()->height()) + 10);
#else
    width = fontMetrics.horizontalAdvance(name) + ui.iconListLabel->pixmap().width() + 17;
    setFixedHeight(qMax(fontMetrics.height(), ui.iconListLabel->pixmap().height()) + 10);
#endif
}

SettingsDialogListElement::~SettingsDialogListElement()
{
}

void SettingsDialogListElement::mouseReleaseEvent(QMouseEvent* event)
{
    event->accept();

    if (event->button() == Qt::LeftButton)
    {
        parent->clickOnList(index);
    }
}

void SettingsDialogListElement::selectedItem()
{
    setStyleSheet("background:#3399ff;color:white");
}

void SettingsDialogListElement::clearSelection()
{
    setStyleSheet("");
}
