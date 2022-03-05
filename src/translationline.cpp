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

#include "translationline.h"
#include "main.h"
#include <QTextDocument>

TranslationLine::TranslationLine(QWidget* parent) : QTextEdit(parent)
{
    fixingSize = false;
    setWordWrapMode(QTextOption::WrapAnywhere);
    setTabChangesFocus(true);
    setAcceptRichText(false);
    setMinimumWidth(100);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    connect(this, &TranslationLine::textChanged, this, &TranslationLine::textChangedSlot);
}

TranslationLine::~TranslationLine()
{
}

void TranslationLine::focusInEvent(QFocusEvent* e)
{
    QTextEdit::focusInEvent(e);
    selectAll();
}

void TranslationLine::focusOutEvent(QFocusEvent* e)
{
    QTextEdit::focusOutEvent(e);
    QTextCursor cursor = textCursor();
    cursor.clearSelection();
    setTextCursor(cursor);
}

void TranslationLine::textChangedSlot()
{
    fixSize();

    if (isChanged())
        setStyleSheet("color: " + baseValues.appTheme.black.name());
    else
        setStyleSheet("color: " + baseValues.appTheme.red.name());

    emit lineTextChanged();
}

void TranslationLine::fixSize()
{
    if (fixingSize)
        return;

    fixingSize = true;
    int docHeight = document()->size().height();

    if (docHeight > 0)
        setFixedHeight(docHeight);

    fixingSize = false;
}

void TranslationLine::setDefaultText(const QString& defText)
{
    if (defText != "yyyy-MM-dd HH:mm:ss" && defText != baseValues.exchangeName + ":")
    {
        defaultText = defText;
        defaultText.replace("<br>", "\n");
    }
}

void TranslationLine::setItemText(QString text)
{
    if (text.isEmpty())
        text = defaultText;

    text.replace("<br>", "\n");
    setPlainText(text);
    document()->adjustSize();
    fixSize();
}

void TranslationLine::resizeEvent(QResizeEvent* event)
{
    QTextEdit::resizeEvent(event);
    fixSize();
}

QString TranslationLine::getValidText()
{
    QString validText = toPlainText();
    validText.replace("\r", "");
    validText.replace("\n", "<br>");

    QString lastText;

    while (lastText != validText && !validText.isEmpty())
    {
        validText.replace("<br> ", "<br>");
        lastText = validText;
    }

    lastText.clear();

    while (lastText != validText && !validText.isEmpty())
    {
        validText.replace(" <br>", "<br>");
        lastText = validText;
    }

    return validText;
}
