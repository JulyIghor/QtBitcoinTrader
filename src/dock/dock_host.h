#pragma once

#include <QObject>
#include <QDockWidget>


class QDockWidget;


class DockHost : public QObject
{
    Q_OBJECT

public:
    DockHost(QObject* parent = NULL);
    ~DockHost();

public:
    QDockWidget* createDock(QWidget* parent, QWidget* widget, const QString& title);
    void lockDocks(bool lock);
    void setFloatingVisible(bool visible);

protected:
    bool eventFilter(QObject* obj, QEvent* event) Q_DECL_OVERRIDE;

private Q_SLOTS:
    void onDockActionTriggered(bool checked = false);
    void onDockTopLevelChanged();
    void onDockVisibilityChanged();
    void onDockFeaturesChanged(QDockWidget::DockWidgetFeatures features);

private:
    void selectTab(QDockWidget* dock);
    void adjustFloatingWindowFlags(QDockWidget* dock);
    bool isConstrained(QDockWidget* dock);

private:
    QList<QWidget*>     widgets;
    QDockWidget*        dockToggling;
};
