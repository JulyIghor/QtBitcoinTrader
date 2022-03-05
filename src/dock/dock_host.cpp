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

#include "dock_host.h"
#include <QAction>
#include <QEvent>
#include <QLayout>
#include <QTabBar>

DockHost::DockHost(QObject* parent) : QObject(parent), docks(), dockToggling(nullptr), lastLock(false), staysOnTop(false)
{
    //
}

DockHost::~DockHost()
{
    //
}

QDockWidget* DockHost::createDock(QWidget* parent, QWidget* widget, const QString& title)
{
    static int counter = 0;
    auto* dock = new QDockWidget(parent);
    dock->setObjectName(QString("dock%1").arg(++counter));
    dock->setProperty("Title", title);

    QMargins widgetMargins = widget->layout()->contentsMargins();
    widgetMargins.setTop(widgetMargins.top() + 2);
    widget->layout()->setContentsMargins(widgetMargins);
    dock->setWidget(widget);
    dock->setWindowTitle(title);

    QObject::connect(dock, &QDockWidget::topLevelChanged, this, &DockHost::onDockTopLevelChanged);

    QAction* action = dock->toggleViewAction();
    action->disconnect();
    QObject::connect(action, &QAction::triggered, this, &DockHost::onDockActionTriggered);

    dock->installEventFilter(this);

    docks << dock;
    return dock;
}

void DockHost::lockDocks(bool lock)
{
    if (lock == lastLock)
        return;

    lastLock = lock;

    for (QDockWidget* dock : docks)
    {
        if (dock)
        {
            if (lock)
            {
                if (dock->isFloating())
                {
                    dock->setFeatures(QDockWidget::DockWidgetFloatable);
                    dock->setAllowedAreas(Qt::NoDockWidgetArea);
                }
                else
                {
                    dock->setFeatures(QDockWidget::NoDockWidgetFeatures);
                    dock->setAllowedAreas(Qt::AllDockWidgetAreas);
                }
            }
            else
            {
                dock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
                dock->setAllowedAreas(Qt::AllDockWidgetAreas);
            }

            adjustFloatingWindowFlags(dock);
        }
    }
}

void DockHost::setFloatingVisible(bool visible)
{
    for (QDockWidget* dock : docks)
    {
        if (dock && dock->isFloating())
            dock->setVisible(visible);
    }
}

bool DockHost::eventFilter(QObject* obj, QEvent* event)
{
    auto* dock = static_cast<QDockWidget*>(obj);

    if (dock && dock->isFloating() && event->type() == QEvent::NonClientAreaMouseButtonDblClick)
    {
        if (!isConstrained(dock))
            return true; // handled. allow to minimize/maximize dock

        if (dock->allowedAreas() == Qt::NoDockWidgetArea)
            return true; // handled. forbid floating toggling.
    }

    return QObject::eventFilter(obj, event);
}

void DockHost::onDockActionTriggered(bool checked)
{
    Q_UNUSED(checked);
    auto* action = static_cast<QAction*>(sender());
    auto* dock = static_cast<QDockWidget*>(action->parent());

    if (dock)
    {
        if (dock->isVisible())
        {
            dock->close();
        }
        else
        {
            dockToggling = dock;
            dock->show();
            dockToggling = nullptr;
        }
    }
}

void DockHost::onDockTopLevelChanged()
{
    auto* dock = static_cast<QDockWidget*>(sender());

    if (dock->isFloating())
    {
        adjustFloatingWindowFlags(dock);
        dock->setWindowTitle(dock->property("Title").toString() + " - Qt Bitcoin Trader");
    }
    else
        dock->setWindowTitle(dock->property("Title").toString());
}

void DockHost::adjustFloatingWindowFlags(QDockWidget* dock)
{
    if (dock->isFloating())
    {
        Qt::WindowFlags flags = Qt::Window | Qt::CustomizeWindowHint | Qt::WindowTitleHint;

        if (dock->features() & QDockWidget::DockWidgetClosable)
            flags |= Qt::WindowCloseButtonHint;

        if (!isConstrained(dock))
            flags |= Qt::WindowMinMaxButtonsHint;

        if (staysOnTop)
            flags |= Qt::WindowStaysOnTopHint;

        bool isDockVisible = dock->isVisible();
        dock->setWindowFlags(flags);

        if (isDockVisible)
            dock->show();
    }
}

bool DockHost::isConstrained(QDockWidget* dock)
{
    QSize size = dock->widget()->maximumSize();
    return size.width() != QWIDGETSIZE_MAX || size.height() != QWIDGETSIZE_MAX;
}

void DockHost::hideFloatingWindow()
{
    for (QDockWidget* dock : docks)
    {
        if (dock && dock->isFloating())
            dock->hide();
    }
}

void DockHost::setStaysOnTop(bool state)
{
    if (state == staysOnTop)
        return;

    staysOnTop = state;

    for (QDockWidget* dock : docks)
    {
        adjustFloatingWindowFlags(dock);
    }
}
