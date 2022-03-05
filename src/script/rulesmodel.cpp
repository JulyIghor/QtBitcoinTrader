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

#include "rulesmodel.h"
#include "exchange/exchange.h"
#include "main.h"
#include "rulescriptparser.h"
#include "timesync.h"
#include <QRandomGenerator>

RulesModel::RulesModel(const QString& _gName) : QAbstractItemModel(), groupName(_gName)
{
    runningCount = 0;
    lastRuleGroupIsRunning = false;
    lastRuleId = QRandomGenerator::global()->bounded(1, 1001);
    stateWidth = 80;
    isConcurrentMode = false;
    columnsCount = 2;
    connect(this, &RulesModel::setRuleTabRunning, baseValues_->mainWindow_, &QtBitcoinTrader::setRuleTabRunning);
}

RulesModel::~RulesModel()
{
    clear();
}

void RulesModel::currencyChanged()
{
    if (baseValues.currentExchange_->multiCurrencyTradeSupport)
        return;

    for (int n = 0; n < holderList.size(); n++)
        pauseList[n] = holderList.at(n).valueASymbolCode != baseValues.currentPair.symbol ||
                       holderList.at(n).valueBSymbolCode != baseValues.currentPair.symbol;

    emit dataChanged(index(0, 0), index(rowCount() - 1, columnsCount - 1));
}

void RulesModel::updateRule(int row, RuleHolder& holder, bool running)
{
    if (row < 0 || row >= holderList.size())
        return;

    if (stateList.at(row) > 0)
        setRuleStateByRow(row, 0);

    holderList[row] = holder;
    emit dataChanged(index(row, 0), index(row, columnsCount - 1));

    if (running)
        setRuleStateByRow(row, 1);
}

void RulesModel::writeLogSlot(const QString& text)
{
    emit writeLog(text);
}

void RulesModel::addRule(RuleHolder& holder, bool running)
{
    beginInsertRows(QModelIndex(), holderList.size(), holderList.size());
    holderList << holder;
    auto* newScript = new ScriptObject(QString::number(lastRuleId++));
    connect(newScript, &ScriptObject::runningChanged, this, &RulesModel::runningChanged);
    connect(newScript, &ScriptObject::setGroupDone, this, &RulesModel::setGroupDone);
    connect(newScript, &ScriptObject::writeLog, this, &RulesModel::writeLogSlot);

    scriptList << newScript;
    stateList << 0;
    pauseList << false;
    doneList << TimeSync::getTimeT();
    endInsertRows();

    if (running)
        setRuleStateByRow(holderList.size() - 1, 1);
}

void RulesModel::checkRuleGroupIsRunning()
{
    if ((runningCount > 0) != lastRuleGroupIsRunning)
    {
        lastRuleGroupIsRunning = runningCount > 0;
        emit setRuleTabRunning(groupName, lastRuleGroupIsRunning);
    }

    if (!isConcurrentMode)
        for (int n = 0; n < stateList.size(); n++)
        {
            if (stateList.at(n) == 1)
                return;

            if (stateList.at(n) == 2)
            {
                setRuleStateByRow(n, 1);
                return;
            }
        }
}

void RulesModel::runningChanged(bool on)
{
    auto* senderScript = static_cast<ScriptObject*>(sender());

    if (senderScript == nullptr)
        return;

    QString name = senderScript->scriptName;
    setStateByName(name, on ? 1 : 0);

    if (on)
        runningCount++;
    else
        runningCount--;

    if (runningCount < 0)
        runningCount = 0;

    checkRuleGroupIsRunning();
}

void RulesModel::setGroupDone(const QString& name)
{
    setStateByName(name, 3);
    runningCount--;

    if (runningCount < 0)
        runningCount = 0;

    checkRuleGroupIsRunning();
}

void RulesModel::setStateByName(const QString& name, int newState)
{
    for (int n = 0; n < scriptList.size(); n++)
        if (scriptList.at(n) && scriptList.at(n)->scriptName == name)
        {
            if (stateList.at(n) == 3 && newState == 0)
            {
                if (TimeSync::getTimeT() - doneList.at(n) > 1)
                    stateList[n] = newState;
            }
            else
                stateList[n] = newState;

            if (newState == 3)
            {
                emit ruleDone();
                doneList[n] = TimeSync::getTimeT();
            }

            if (!baseValues.currentExchange_->multiCurrencyTradeSupport)
                pauseList[n] = holderList.at(n).valueASymbolCode != baseValues.currentPair.symbol ||
                               holderList.at(n).valueBSymbolCode != baseValues.currentPair.symbol;

            emit dataChanged(index(n, 0), index(n, columnsCount - 1));
            return;
        }
}

int RulesModel::getStateByRow(int row)
{
    if (row < 0 || row >= stateList.size())
        return 0;

    return stateList.at(row);
}

bool RulesModel::haveWorkingRule() const
{
    return runningCount > 0;
}

void RulesModel::moveRowUp(int row)
{
    if (row - 1 < 0)
        return;

    if (!isConcurrentMode && stateList.at(row - 1) == 1 && stateList.at(row) == 2)
    {
        setRuleStateByRow(row - 1, 0);
        stateList[row - 1] = 2;
        swapRows(row, row - 1);
        setRuleStateByRow(row - 1, 0);
        setRuleStateByRow(row - 1, 1);
    }
    else
        swapRows(row, row - 1);

    emit dataChanged(index(row - 1, 0), index(row, columnsCount - 1));
}

void RulesModel::moveRowDown(int row)
{
    if (row + 1 >= stateList.size())
        return;

    if (!isConcurrentMode && stateList.at(row) == 1 && stateList.at(row + 1) == 2)
    {
        setRuleStateByRow(row, 0);
        stateList[row] = 2;
        swapRows(row, row + 1);
        setRuleStateByRow(row, 0);
        setRuleStateByRow(row, 1);
    }
    else
        swapRows(row, row + 1);

    emit dataChanged(index(row, 0), index(row + 1, columnsCount - 1));
}

void RulesModel::swapRows(int a, int b)
{
    pauseList.swapItemsAt(a, b);
    stateList.swapItemsAt(a, b);
    scriptList.swapItemsAt(a, b);
    holderList.swapItemsAt(a, b);
    doneList.swapItemsAt(a, b);
}

bool RulesModel::isRowPaused(int curRow)
{
    if (curRow < 0 || stateList.size() <= curRow)
        return false;

    return pauseList.at(curRow);
}

bool RulesModel::testRuleByRow(int curRow, bool forceTest)
{
    if (curRow < 0 || stateList.size() <= curRow)
        return false;

    if (stateList.at(curRow) == 1 && !forceTest)
        return false;

    if (!forceTest && !isConcurrentMode)
    {
        for (int n = 0; n < curRow; n++)
            if (stateList.at(n) != 3 && stateList.at(n) != 0)
                return false;
    }

    QString code = RuleScriptParser::holderToScript(holderList[curRow], true);
    ScriptObject tempScript("Test");
    tempScript.executeScript(code, true);
    tempScript.stopScript();
    return tempScript.testResult == 1;
}

void RulesModel::setRuleStateByRow(int curRow, int state)
{
    if (curRow < 0 || stateList.size() <= curRow)
        return;

    if (state == 0)
    {
        scriptList[curRow]->stopScript();
        stateList[curRow] = state;
    }
    else
    {
        if (isConcurrentMode)
        {
            scriptList[curRow]->stopScript();
            scriptList[curRow]->executeScript(RuleScriptParser::holderToScript(holderList[curRow]), false);
        }
        else
        {
            int firstWorking = -1;
            int firstPending = -1;

            for (int n = 0; n < scriptList.size(); n++)
            {
                if (firstWorking == -1 && scriptList.at(n)->isRunning())
                    firstWorking = n;

                if (firstPending == -1 && stateList.at(n) == 2)
                    firstPending = n;

                if (firstWorking != -1 || firstPending != -1)
                    break;
            }

            if (firstPending > -1)
            {
                scriptList[curRow]->stopScript();

                if (curRow < firstPending)
                    scriptList[curRow]->executeScript(RuleScriptParser::holderToScript(holderList[curRow]), false);
                else
                    scriptList[firstPending]->executeScript(RuleScriptParser::holderToScript(holderList[curRow]), false);
            }
            else if (firstWorking == -1)
            {
                scriptList[curRow]->stopScript();
                scriptList[curRow]->executeScript(RuleScriptParser::holderToScript(holderList[curRow]), false);
            }
            else
            {
                if (curRow < firstWorking)
                {
                    scriptList[firstWorking]->stopScript();
                    stateList[firstWorking] = 2;
                    scriptList[curRow]->executeScript(RuleScriptParser::holderToScript(holderList[curRow]), false);
                    emit dataChanged(index(firstWorking, 0), index(firstWorking, columnsCount - 1));
                }
                else
                {
                    stateList[curRow] = 2;
                }
            }
        }
    }

    if (!baseValues.currentExchange_->multiCurrencyTradeSupport)
        pauseList[curRow] = holderList.at(curRow).valueASymbolCode != baseValues.currentPair.symbolSecond() ||
                            holderList.at(curRow).valueBSymbolCode != baseValues.currentPair.symbolSecond();

    emit dataChanged(index(curRow, 0), index(curRow, columnsCount - 1));
}

void RulesModel::clear()
{
    beginResetModel();
    holderList.clear();
    qDeleteAll(scriptList);
    scriptList.clear();
    stateList.clear();
    pauseList.clear();
    doneList.clear();
    endResetModel();
}

int RulesModel::rowCount(const QModelIndex& /*parent*/) const
{
    return holderList.size();
}

int RulesModel::columnCount(const QModelIndex& /*parent*/) const
{
    return columnsCount;
}

void RulesModel::removeRuleByRow(int row)
{
    beginRemoveRows(QModelIndex(), row, row);
    holderList.removeAt(row);
    scriptList[row]->stopScript();
    scriptList[row]->deleteLater();
    scriptList.removeAt(row);
    stateList.removeAt(row);
    pauseList.removeAt(row);
    doneList.removeAt(row);
    endRemoveRows();
}

QVariant RulesModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int currentRow = index.row();

    if (currentRow < 0 || currentRow >= holderList.size())
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::ToolTipRole && role != Qt::ForegroundRole && role != Qt::BackgroundRole &&
        role != Qt::TextAlignmentRole)
        return QVariant();

    int indexColumn = index.column();

    if (role == Qt::TextAlignmentRole)
        return 0x0084;

    if (role == Qt::ForegroundRole)
        return baseValues.appTheme.black;

    if (role == Qt::BackgroundRole)
    {
        switch (stateList.at(currentRow))
        {
        case 1:
            return QVariant();
            break;

        case 2:
            return baseValues.appTheme.lightRedGreen;
            break;

        case 3:
            return baseValues.appTheme.lightGreen;
            break;

        default:
            return baseValues.appTheme.lightRed;
            break;
        }

        return QVariant();
    }

    switch (indexColumn)
    {
    case 0: // State
        switch (stateList.at(currentRow))
        {
        case 1:
            {
                if (pauseList.at(currentRow))
                    return julyTr("RULE_STATE_PAUSED", "paused");

                return julyTr("RULE_STATE_PROCESSING", "processing");
            }
            break;

        case 2:
            return julyTr("RULE_STATE_PENDING", "pending");
            break;

        case 3:
            return julyTr("RULE_STATE_DONE", "done");
            break;

        default:
            return julyTr("RULE_STATE_DISABLED", "disabled");
            break;
        }

    case 1: // Description
        return holderList.at(currentRow).description;
        break;

    default:
        break;
    }

    return QVariant();
}

void RulesModel::disableAll()
{
    for (int n = holderList.size() - 1; n >= 0; n--)
        setRuleStateByRow(n, 0);
}

void RulesModel::enableAll()
{
    for (int n = 0; n < holderList.size(); n++)
        setRuleStateByRow(n, 1);
}

QVariant RulesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::TextAlignmentRole)
        return 0x0084;

    if (role == Qt::SizeHintRole && orientation == Qt::Horizontal)
    {
        switch (section)
        {
        case 0:
            return QSize(stateWidth, defaultHeightForRow); // State
            // case 2: return QSize(typeWidth,defaultSectionSize);//Type
        }

        return QVariant();
    }

    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Vertical)
        return section;

    if (headerLabels.size() != columnsCount)
        return QVariant();

    return headerLabels.at(section);
}

Qt::ItemFlags RulesModel::flags(const QModelIndex& /*index*/) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

void RulesModel::setHorizontalHeaderLabels(QStringList list)
{
    if (list.size() != columnsCount)
        return;

    headerLabels = list;
    stateWidth = qMax(textFontWidth(headerLabels.first()), textFontWidth(julyTr("RULE_STATE_PROCESSING", "processing")));
    stateWidth = qMax(stateWidth, textFontWidth(julyTr("RULE_STATE_PENDING", "pending")));
    stateWidth = qMax(stateWidth, textFontWidth(julyTr("RULE_STATE_DONE", "done")));
    stateWidth = qMax(stateWidth, textFontWidth(julyTr("RULE_STATE_DISABLED", "disabled")));
    stateWidth += 12;
    emit headerDataChanged(Qt::Horizontal, 0, columnsCount - 1);
}

QModelIndex RulesModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column);
}

QModelIndex RulesModel::parent(const QModelIndex& /*child*/) const
{
    return QModelIndex();
}
