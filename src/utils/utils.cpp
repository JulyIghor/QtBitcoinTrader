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

#include "utils.h"

#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QLayout>
#include <QScreen>
#include <QWidget>

QString changeFileExt(const QString& fileName, const QString& ext)
{
    QFileInfo fileInfo(fileName);
    return fileInfo.path() + "/" + fileInfo.baseName() + ext;
}

QString adjustPathSeparators(const QString& path)
{
    return QDir::fromNativeSeparators(path);
}

QString slash(const QString& path1, const QString& path2)
{
    QString s1 = path1;
    QString s2 = path2;

    while (s1.endsWith('/') || s1.endsWith('\\'))
    {
        s1.truncate(s1.length() - 1);
    }

    while (s2.startsWith('/') || s2.startsWith('\\'))
    {
        s1.remove(0, 1);
    }

    return s1 + "/" + s2;
}

QString slash(const QString& path1, const QString& path2, const QString& path3)
{
    return slash(slash(path1, path2), path3);
}

void adjustWidgetGeometry(QWidget* widget)
{
    QRect workarea = QApplication::screenAt(widget->mapToGlobal(widget->geometry().center()))->availableGeometry();
    QRect bounds = widget->frameGeometry();
    int delta;

    if ((delta = (workarea.right() - bounds.right())) < 0)
    {
        bounds.translate(delta, 0);
    }

    if ((delta = (workarea.bottom() - bounds.bottom())) < 0)
    {
        bounds.translate(0, delta);
    }

    if ((delta = (workarea.left() - bounds.left())) > 0)
    {
        bounds.translate(delta, 0);
    }

    if ((delta = (workarea.top() - bounds.top())) > 0)
    {
        bounds.translate(0, delta);
    }

    widget->move(bounds.topLeft());
}

void recursiveUpdateLayouts(const QObject* object)
{
    const QWidget* widget = qobject_cast<const QWidget*>(object);

    if (widget->layout())
    {
        if (widget->layout()->spacing() > 0)
            widget->layout()->setSpacing(widget->layout()->spacing() / 2);

        if (widget->layout()->contentsMargins().top() > 0)
            widget->layout()->setContentsMargins(3, 3, 3, 3);

        if (widget->layout()->contentsMargins().top() == -1)
            widget->layout()->setContentsMargins(3, 5, 3, 3);
    }

    for (const QObject* child : object->children())
        if (const QWidget* wd = qobject_cast<const QWidget*>(child))
            recursiveUpdateLayouts(wd);
}
