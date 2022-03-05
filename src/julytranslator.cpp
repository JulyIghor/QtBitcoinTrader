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

#include "julytranslator.h"
#include "main.h"
#include <QCheckBox>
#include <QDockWidget>
#include <QDoubleSpinBox>
#include <QFile>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>

int JulyTranslator::loadFromFile(const QString& fileName)
{
    clearMaps();
    QFile loadFile(fileName);

    if (loadFile.open(QIODevice::ReadOnly))
    {
        fillMapsFromList(QString::fromUtf8(loadFile.readAll().replace("\r", "")).split("\n"));
        loadFile.close();
        lastLangFile = fileName;
        emit languageChanged();
        return 0;
    }

    return 1;
}

void JulyTranslator::fillMapsFromList(const QStringList& list)
{
    for (int n = 0; n < list.size(); n++)
    {
        QString currentRow = list.at(n);

        if (currentRow.isEmpty() || !currentRow.at(0).isLetter())
            continue;

        if (fillMapsFromLine(&buttonMap, currentRow, "Button_"))
            if (fillMapsFromLine(&labelMap, currentRow, "Label_"))
                if (fillMapsFromLine(&checkBoxMap, currentRow, "CheckBox_"))
                    if (fillMapsFromLine(&spinBoxMap, currentRow, "SpinBox_"))
                        if (fillMapsFromLine(&stringMap, currentRow, "String_"))
                            fillMapsFromLine(&groupBoxMap, currentRow, "GroupBox_");
    }
}

bool JulyTranslator::fillMapsFromLine(QMap<QString, QString>* map, QString line, const QString& prefix)
{
    if (!line.startsWith(prefix))
        return true;

    line.remove(0, prefix.length());
    int splitPos = line.indexOf('=');

    if (splitPos == -1 || splitPos + 1 >= line.length())
        return true;

    QString currentTid = line.left(splitPos);
    line.remove(0, splitPos + 1);
    (*map)[currentTid] = line;
    return false;
}

int JulyTranslator::saveToFile(const QString& fileName)
{
    QStringList resultList;
    resultList.append(getMapList(&buttonMap, "Button_"));
    resultList.append(getMapList(&labelMap, "Label_"));
    resultList.append(getMapList(&checkBoxMap, "CheckBox_"));
    resultList.append(getMapList(&groupBoxMap, "GroupBox_"));
    resultList.append(getMapList(&spinBoxMap, "SpinBox_"));
    resultList.append(getMapList(&stringMap, "String_"));

    if (resultList.isEmpty())
        return 1;

    resultList.sort();
    QFile writeFile(fileName);

    if (writeFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        writeFile.write(QString(resultList.join("\r\n") + "\r\n").toUtf8());
        writeFile.close();
        return 0;
    }

    return 2;
}

QStringList JulyTranslator::getMapList(QMap<QString, QString>* map, QString prefix)
{
    QStringList mapTids = map->keys();

    for (int n = 0; n < mapTids.size(); n++)
    {
        mapTids[n] = prefix + mapTids.at(n) + "=" + map->value(mapTids.at(n), "");
        mapTids[n].replace("\n", "<br>");
        mapTids[n].replace("\r", "");
        mapTids[n].replace("\t", " ");
    }

    return mapTids;
}

QString JulyTranslator::translateButton(const QString& tid, const QString& defaultText) const
{
    return buttonMap.value(tid, defaultText);
}
QString JulyTranslator::translateLabel(const QString& tid, const QString& defaultText) const
{
    return labelMap.value(tid, defaultText);
}
QString JulyTranslator::translateCheckBox(const QString& tid, const QString& defaultText) const
{
    return checkBoxMap.value(tid, defaultText);
}
QString JulyTranslator::translateGroupBox(const QString& tid, const QString& defaultText) const
{
    return groupBoxMap.value(tid, defaultText);
}
QString JulyTranslator::translateSpinBox(const QString& tid, const QString& defaultText) const
{
    return spinBoxMap.value(tid, defaultText);
}

QString JulyTranslator::translateString(const QString& tid, const QString& defaultText)
{
    QString result = stringMap.value(tid, defaultText);

    if (stringMap.find(tid) == stringMap.end())
        stringMap[tid] = defaultText;

    return result;
}

void JulyTranslator::loadMapFromUi(QWidget* par)
{
    for (QPushButton* curButton : par->findChildren<QPushButton*>())
        if (!curButton->accessibleName().isEmpty())
            buttonMap[curButton->accessibleName()] = curButton->text().replace("\n", "<br>").replace("\r", "");

    for (QToolButton* curButton : par->findChildren<QToolButton*>())
        if (!curButton->accessibleName().isEmpty())
            buttonMap[curButton->accessibleName()] = curButton->text().replace("\n", "<br>").replace("\r", "");

    for (QCheckBox* curCheckBox : par->findChildren<QCheckBox*>())
        if (!curCheckBox->accessibleName().isEmpty())
            checkBoxMap[curCheckBox->accessibleName()] = curCheckBox->text().replace("\n", "<br>").replace("\r", "");

    for (QRadioButton* curCheckBox : par->findChildren<QRadioButton*>())
        if (!curCheckBox->accessibleName().isEmpty())
            checkBoxMap[curCheckBox->accessibleName()] = curCheckBox->text().replace("\n", "<br>").replace("\r", "");

    for (QLabel* curLabel : par->findChildren<QLabel*>())
        if (!curLabel->accessibleName().isEmpty())
            labelMap[curLabel->accessibleName()] = curLabel->text().replace("\n", "<br>").replace("\r", "");

    for (QGroupBox* curGroupBox : par->findChildren<QGroupBox*>())
        if (!curGroupBox->accessibleName().isEmpty())
            groupBoxMap[curGroupBox->accessibleName()] = curGroupBox->title().replace("\n", "<br>").replace("\r", "");

    for (QDoubleSpinBox* curSpinBox : par->findChildren<QDoubleSpinBox*>())
        if (!curSpinBox->accessibleName().isEmpty())
            spinBoxMap[curSpinBox->accessibleName()] = curSpinBox->suffix();
}

void JulyTranslator::translateUi(QWidget* par)
{
    if (par == nullptr)
        return;

    for (QPushButton* curButton : par->findChildren<QPushButton*>())
        if (!curButton->accessibleName().isEmpty())
            curButton->setText(translateButton(curButton->accessibleName(), curButton->text()));

    for (QToolButton* curButton : par->findChildren<QToolButton*>())
        if (!curButton->accessibleName().isEmpty())
            curButton->setText(translateButton(curButton->accessibleName(), curButton->text()));

    for (QCheckBox* curCheckBox : par->findChildren<QCheckBox*>())
        if (!curCheckBox->accessibleName().isEmpty())
            curCheckBox->setText(translateCheckBox(curCheckBox->accessibleName(), curCheckBox->text()));

    for (QRadioButton* curCheckBox : par->findChildren<QRadioButton*>())
        if (!curCheckBox->accessibleName().isEmpty())
            curCheckBox->setText(translateCheckBox(curCheckBox->accessibleName(), curCheckBox->text()));

    for (QLabel* curLabel : par->findChildren<QLabel*>())
        if (!curLabel->accessibleName().isEmpty())
            curLabel->setText(translateLabel(curLabel->accessibleName(), curLabel->text()));

    for (QGroupBox* curGroupBox : par->findChildren<QGroupBox*>())
        if (!curGroupBox->accessibleName().isEmpty())
            curGroupBox->setTitle(translateGroupBox(curGroupBox->accessibleName(), curGroupBox->title()));

    for (QDoubleSpinBox* curSpinBox : par->findChildren<QDoubleSpinBox*>())
        if (!curSpinBox->accessibleName().isEmpty())
            curSpinBox->setSuffix(translateSpinBox(curSpinBox->accessibleName(), curSpinBox->suffix()));

    for (QWidget* curWidget : par->findChildren<QWidget*>())
    {
        auto* dock = static_cast<QDockWidget*>(curWidget->parentWidget());

        if (dock == nullptr || curWidget->accessibleName().isEmpty())
            continue;

        dock->setWindowTitle(translateGroupBox(curWidget->accessibleName(), dock->windowTitle()));
    }
}
