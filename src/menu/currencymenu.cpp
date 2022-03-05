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

#include <QScreen>
#include <QToolButton>
#include <QScrollBar>
#include "currencymenucell.h"
#include "currencymenu.h"
#include "ui_currencymenu.h"

CurrencyMenu::CurrencyMenu(QToolButton* _parentButton) :
    QMenu(),
    ui(new Ui::CurrencyMenu),
    parentButton(_parentButton),
    currentIndex(0),
    setCurrencyVisible(false)
{
    ui->setupUi(this);
    parentButton->setMenu(this);
    ui->filterWidget->setFixedHeight(ui->filterWidget->minimumSizeHint().height());
}

CurrencyMenu::~CurrencyMenu()
{
    delete ui;
}

void CurrencyMenu::on_filterLine_textChanged(const QString& filter)
{
    if (currencyPairs.size() != ui->currencyLayout->count())
        return;

    setCurrencyVisible = false;
    ui->scrollArea->hide();

    for (int i = 0; i < currencyPairs.size(); ++i)
    {
        bool bufferVisible = currencyPairs.at(i).contains(filter, Qt::CaseInsensitive);
        ui->currencyLayout->itemAt(i)->widget()->setVisible(bufferVisible);

        if (bufferVisible && !setCurrencyVisible)
            setCurrencyVisible = true;
    }

    currencyResize();
}

void CurrencyMenu::setPairs(const QStringList& newCurrencyPairs)
{
    if (newCurrencyPairs.isEmpty())
        return;

    for (int i = 0; i < currencyPairs.size(); ++i)
        ui->currencyLayout->itemAt(i)->widget()->deleteLater();

    currentIndex = 0;
    currencyPairs = newCurrencyPairs;

    displayPairs();
}

void CurrencyMenu::displayPairs()
{
    QPalette palette;
    QString highlight       = palette.color(QPalette::Highlight).name();
    QString highlightedText = palette.color(QPalette::HighlightedText).name();
    QString currencyStyle = "QWidget:hover{background-color:" + highlight + ";color:" + highlightedText + "}";

    for (int i = 0; i < currencyPairs.size(); ++i)
    {
        ui->currencyLayout->addWidget(new CurrencyMenuCell(currencyPairs.at(i), currencyStyle, i, this));
    }
}

int CurrencyMenu::count()
{
    return currencyPairs.size();
}

int CurrencyMenu::getCurrentIndex() const
{
    return currentIndex;
}

void CurrencyMenu::setCurrentIndex(const int newCurrentIndex)
{
    if (newCurrentIndex < 0 || newCurrentIndex > currencyPairs.size())
        return;

    currentIndex = newCurrentIndex;
    parentButton->setText(currencyPairs.at(currentIndex));
    parentButton->setFixedWidth(parentButton->minimumSizeHint().width());
    emit currencyMenuChanged(currentIndex);
}

void CurrencyMenu::showEvent(QShowEvent* /*event*/)
{
    currencyResize();
}

void CurrencyMenu::currencyResize()
{
    QPoint pos = parentButton->mapToGlobal(QPoint(0, 0));
    int width = ui->currencyLayout->minimumSize().width();
    int height = 0;
    int maxHeight = ui->scrollAreaWidgetContents->minimumSizeHint().height() + ui->filterWidget->height() + 2;

    for (QScreen* screen : QGuiApplication::screens())
    {
        QRect screenRect = screen->geometry();

        if (screenRect.contains(pos))
        {
            height = screenRect.bottom() - pos.y() - parentButton->height() - 10;
        }
    }

    if (height > maxHeight)
    {
        height = maxHeight;
        ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }
    else
    {
        ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        width += ui->scrollArea->verticalScrollBar()->width();
    }

    if (width < 58)
        width = 58;

    if (height < 28)
        height = 28;

    setFixedSize(width, height);

    if (setCurrencyVisible)
    {
        ui->scrollArea->show();
    }
}

void CurrencyMenu::currencySelect(const int newCurrentIndex)
{
    setCurrentIndex(newCurrentIndex);
    hide();
}

void CurrencyMenu::mouseReleaseEvent(QMouseEvent* /*unused*/)
{
}
