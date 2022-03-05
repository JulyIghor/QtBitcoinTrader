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

#ifndef RULESMODEL_H
#define RULESMODEL_H

#include "ruleholder.h"
#include "scriptobject.h"
#include <QAbstractItemModel>
#include <QStringList>

class RulesModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    void currencyChanged();
    void addRule(RuleHolder& holder, bool running = false);
    void updateRule(int row, RuleHolder& holder, bool running = false);
    QList<RuleHolder> holderList;
    int getStateByRow(int row);
    bool isRowPaused(int row);
    void restoreRulesFromString(QString);
    bool haveWorkingRule() const;
    bool isConcurrentMode;
    explicit RulesModel(const QString& groupName);
    ~RulesModel();

    void disableAll();
    void enableAll();

    void moveRowUp(int row);
    void moveRowDown(int row);

    void setRuleStateByRow(int row, int state);
    bool testRuleByRow(int row, bool forceTest = false);
    void clear();
    void removeRuleByRow(int row);

    void setHorizontalHeaderLabels(QStringList list);

    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex& index) const;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex& index) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    int columnCount(const QModelIndex& parent = QModelIndex()) const;
private slots:
    void writeLogSlot(const QString&);
    void runningChanged(bool);
    void setGroupDone(const QString&);

private:
    void swapRows(int, int);
    bool lastRuleGroupIsRunning;
    int runningCount;
    void checkRuleGroupIsRunning();
    QString groupName;
    void setStateByName(const QString&, int);
    quint32 lastRuleId;
    QList<ScriptObject*> scriptList;
    QList<int> stateList;
    QList<bool> pauseList;
    QList<quint32> doneList;
    int stateWidth;
    QStringList headerLabels;
    int columnsCount;
signals:
    void writeLog(QString);
    void ruleDone();
    void groupDone();
    void setRuleTabRunning(QString, bool);
    void rulesCountChanged();
};

#endif // RULESMODEL_H
