#include "dock_host.h"

#include <QAction>
#include <QTabBar>
#include <QEvent>
#include <QLayout>


DockHost::DockHost(QObject* parent) :
    QObject         (parent),
    widgets         (),
    dockToggling    (NULL)
{
    //
}

DockHost::~DockHost()
{
    //
}

QDockWidget* DockHost::createDock(QWidget* parent, QWidget* widget, const QString &title)
{
    static int counter = 0;
    QDockWidget* dock = new QDockWidget(parent);
    dock->setObjectName(QString("dock%1").arg(++counter));

    QMargins widgetMargins = widget->layout()->contentsMargins();
    widgetMargins.setTop(widgetMargins.top()+2);
    widget->layout()->setContentsMargins(widgetMargins);
    dock->setWidget(widget);
    dock->setWindowTitle(title);

    QObject::connect(dock, &QDockWidget::topLevelChanged, this, &DockHost::onDockTopLevelChanged);
    QObject::connect(dock, &QDockWidget::visibilityChanged, this, &DockHost::onDockVisibilityChanged);
    QObject::connect(dock, &QDockWidget::featuresChanged, this, &DockHost::onDockFeaturesChanged);

    QAction* action = dock->toggleViewAction();
    action->disconnect();
    QObject::connect(action, &QAction::triggered, this, &DockHost::onDockActionTriggered);

    dock->installEventFilter(this);

    widgets << widget;
    return dock;
}

void DockHost::lockDocks(bool lock)
{
    Q_FOREACH(QWidget* widget, widgets) {
        QDockWidget* dock = static_cast<QDockWidget*>(widget->parentWidget());
        if (dock) {
            if (lock) {
                if (dock->isFloating()) {
                    dock->setFeatures(QDockWidget::DockWidgetFloatable);
                    dock->setAllowedAreas(Qt::NoDockWidgetArea);
                } else {
                    dock->setFeatures(QDockWidget::NoDockWidgetFeatures);
                    dock->setAllowedAreas(Qt::AllDockWidgetAreas);
                }
            } else {
                dock->setFeatures(QDockWidget::AllDockWidgetFeatures);
                dock->setAllowedAreas(Qt::AllDockWidgetAreas);
            }
            adjustFloatingWindowFlags(dock);
        }
    }
}

void DockHost::setFloatingVisible(bool visible)
{
    Q_FOREACH(QWidget* widget, widgets) {
        QDockWidget* dock = static_cast<QDockWidget*>(widget->parentWidget());
        if (dock && dock->isFloating()) {
            dock->setVisible(visible);
        }
    }
}

bool DockHost::eventFilter(QObject* obj, QEvent* event)
{
    QDockWidget* dock = static_cast<QDockWidget*>(obj);
    if (dock && dock->isFloating() && event->type() == QEvent::NonClientAreaMouseButtonDblClick) {
        if (!isConstrained(dock)) {
            return true; // handled. allow to minimize/maximize dock
        }
        if (dock->allowedAreas() == Qt::NoDockWidgetArea) {
            return true; // handled. forbid floating toggling.
        }
    }
    return QObject::eventFilter(obj, event);
}

void DockHost::onDockActionTriggered(bool checked)
{
    Q_UNUSED(checked);
    QAction* action = static_cast<QAction*>(sender());
    QDockWidget* dock = static_cast<QDockWidget*>(action->parent());
    if (dock) {
        if (dock->isVisible()) {
            dock->close();
        } else {
            dockToggling = dock;
            dock->show();
            dockToggling = NULL;
        }
    }
}

void DockHost::onDockTopLevelChanged()
{
    QDockWidget* dock = static_cast<QDockWidget*>(sender());
    if (!dock->isFloating()) {
        selectTab(dock);
    } else {
        adjustFloatingWindowFlags(dock);
    }
}

void DockHost::onDockVisibilityChanged()
{
    QDockWidget* dock = static_cast<QDockWidget*>(sender());
    if (!dock->isFloating() && dock->isVisible() && dockToggling == dock) {
        selectTab(dock);
    }
}

void DockHost::onDockFeaturesChanged(QDockWidget::DockWidgetFeatures features)
{
    Q_UNUSED(features)
    QDockWidget* dock = static_cast<QDockWidget*>(sender());
    if (dock->isFloating() && dock->isVisible()) {
        adjustFloatingWindowFlags(dock);
    }
}

void DockHost::selectTab(QDockWidget* dock)
{
    Q_FOREACH(QTabBar* tabBar, parent()->findChildren<QTabBar*>()) {
        for (int i = 0; i < tabBar->count(); ++i) {
            if (dock == (QWidget*) tabBar->tabData(i).toULongLong()) {
                tabBar->setCurrentIndex(i);
                tabBar->sizePolicy().setVerticalPolicy(QSizePolicy::Expanding);
                return;
            }
        }
    }
}

void DockHost::adjustFloatingWindowFlags(QDockWidget* dock)
{
    if (dock->isFloating()) {
        Qt::WindowFlags flags = Qt::Window | Qt::CustomizeWindowHint | Qt::WindowTitleHint;
        if (dock->features() & QDockWidget::DockWidgetClosable) {
            flags |= Qt::WindowCloseButtonHint;
        }
        if (!isConstrained(dock)) {
            flags |= Qt::WindowMinMaxButtonsHint;
        }
        dock->setWindowFlags(flags);
        dock->show();
    }
}

bool DockHost::isConstrained(QDockWidget* dock)
{
    QSize size = dock->widget()->maximumSize();
    return size.width() != QWIDGETSIZE_MAX || size.height() != QWIDGETSIZE_MAX;
}
