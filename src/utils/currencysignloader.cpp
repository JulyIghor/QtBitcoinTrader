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

#include "currencysignloader.h"
#include <QFile>
#include <QFontMetrics>
#include <QPainter>

CurrencySignLoader::CurrencySignLoader() : QObject()
{
}

CurrencySignLoader::~CurrencySignLoader()
{
}

void CurrencySignLoader::createIcon(const QString& text)
{
    int fontSize;
    QString textTop;
    QString textBottom;

    if (text.size() < 4)
    {
        for (fontSize = 12; fontSize > 3; --fontSize)
        {
            QFont font("Arial", fontSize);
            QFontMetrics fm(font);

            if (fm.boundingRect(text).width() <= 20)
                break;
        }
    }
    else
    {
        int half = (text.size() + 1) / 2;
        textTop = text.left(half);
        textBottom = text.right(text.size() - half);

        for (fontSize = 9; fontSize > 3; --fontSize)
        {
            QFont font("Arial", fontSize);
            QFontMetrics fm(font);

            if (fm.boundingRect(textTop).width() <= 20 && fm.boundingRect(textBottom).width())
                break;
        }
    }

    QPixmap pixmap(20, 20);
    pixmap.fill(QColor(0, 0, 0, 0));
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QColor(255, 255, 255, 255));
    painter.setFont(QFont("Arial", fontSize));
    // painter.drawRect(0, 0, 20, 20);

    if (text.size() < 4)
    {
        QFont font("Arial", fontSize);
        QFontMetrics fm(font);

        int x = (20 - fm.boundingRect(text).width()) / 2;
        int y = (20 - fontSize) / 2;

        painter.drawText(x + 1, 21 - y, text);
        painter.setPen(QColor(0, 0, 0, 255));
        painter.drawText(x, 20 - y, text);
    }
    else
    {
        QFont font("Arial", fontSize);
        QFontMetrics fm(font);

        int x = (20 - fm.boundingRect(textTop).width()) / 2;
        int y = (9 - fontSize) / 2;
        painter.drawText(x + 1, 10 - y, textTop);

        int x2 = (20 - fm.boundingRect(textBottom).width()) / 2;
        int y2 = (9 - fontSize) / 2;
        painter.drawText(x2 + 1, 21 - y2, textBottom);

        painter.setPen(QColor(0, 0, 0, 255));
        painter.drawText(x, 9 - y, textTop);
        painter.drawText(x2, 20 - y2, textBottom);
    }

    currencySign.insert(text, pixmap);
}

void CurrencySignLoader::getCurrencySign(const QString& currency, QPixmap& sign)
{
    QString currencyFile = "://Resources/CurrencySign/" + currency + ".png";

    if (currencySign.contains(currency))
        sign = currencySign.value(currency);
    else if (QFile(currencyFile).exists())
    {
        sign.load(currencyFile);
        currencySign.insert(currency, sign);
    }
    else
    {
        createIcon(currency);
        sign = currencySign.value(currency);
    }
}
