#pragma once

#include <QString>

class QWidget;


QString changeFileExt(const QString& fileName, const QString& ext);
QString adjustPathSeparators(const QString& path);
QString slash(const QString& path1, const QString& path2);
QString slash(const QString& path1, const QString& path2, const QString& path3);
void adjustWidgetGeometry(QWidget* widget);
