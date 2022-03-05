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

#include "rulescriptparser.h"
#include "julymath.h"
#include "main.h"
#include <QSettings>

RuleScriptParser::RuleScriptParser()
{
}

bool RuleScriptParser::writeHolderToSettings(RuleHolder& holder, QSettings& settings, QString section)
{
    if (!holder.isValid())
        return false;

    settings.beginGroup(section);
    settings.setValue("ComparationText", holder.comparationText);
    settings.setValue("ThanAmount", holder.thanAmount);
    settings.setValue("ThanAmountFeeIndex", holder.thanAmountFeeIndex);
    settings.setValue("ThanAmountPercentChecked", holder.thanAmountPercentChecked);
    settings.setValue("ThanPrice", holder.thanPrice);
    settings.setValue("ThanPriceFeeIndex", holder.thanPriceFeeIndex);
    settings.setValue("ThanPricePercentChecked", holder.thanPricePercentChecked);
    settings.setValue("ThanPricePlusMinusText", holder.thanPricePlusMinusText);
    settings.setValue("ThanPriceTypeCode", holder.thanPriceTypeCode);
    settings.setValue("ThanText", holder.thanText);
    settings.setValue("ThanTypeIndex", holder.thanTypeIndex);
    settings.setValue("TradeSymbolCode", holder.tradeSymbolCode);
    settings.setValue("ValueASymbolCode", holder.valueASymbolCode);
    settings.setValue("ValueBSymbolCode", holder.valueBSymbolCode);
    settings.setValue("VariableACode", holder.variableACode);
    settings.setValue("VariableBCode", holder.variableBCode);
    settings.setValue("VariableBExact", holder.variableBExact);
    settings.setValue("VariableBFeeIndex", holder.variableBFeeIndex);
    settings.setValue("VariableBModeIndex", holder.variableBModeIndex);
    settings.setValue("VariableBPercentChecked", holder.variableBPercentChecked);
    settings.setValue("VariableBplusMinus", holder.variableBplusMinus);
    settings.setValue("VariableBSymbolCode", holder.variableBSymbolCode);
    settings.setValue("Description", holder.description);
    settings.setValue("Delay", holder.delayMilliseconds);
    settings.endGroup();
    settings.sync();
    return true;
}

bool RuleScriptParser::writeHolderToFile(RuleHolder& holder, QString& file, QString section)
{
    QSettings settings(file, QSettings::IniFormat);
    return writeHolderToSettings(holder, settings, section);
}

RuleHolder RuleScriptParser::readHolderFromSettings(QSettings& settings, QString section)
{
    RuleHolder holder;
    settings.beginGroup(section);
    holder.comparationText = settings.value("ComparationText", "").toString();
    holder.thanAmount = settings.value("ThanAmount", 0.0).toDouble();
    holder.thanAmountFeeIndex = settings.value("ThanAmountFeeIndex", -1).toInt();
    holder.thanAmountPercentChecked = settings.value("ThanAmountPercentChecked", false).toBool();
    holder.thanPrice = settings.value("ThanPrice", 0.0).toDouble();
    holder.thanPriceFeeIndex = settings.value("ThanPriceFeeIndex", -1).toInt();
    holder.thanPricePercentChecked = settings.value("ThanPricePercentChecked", false).toBool();
    holder.thanPricePlusMinusText = settings.value("ThanPricePlusMinusText", "").toString();
    holder.thanPriceTypeCode = settings.value("ThanPriceTypeCode", "").toString();
    holder.thanText = settings.value("ThanText", "").toString();
    holder.thanTypeIndex = settings.value("ThanTypeIndex", -1).toInt();
    holder.tradeSymbolCode = settings.value("TradeSymbolCode", "").toString();
    holder.valueASymbolCode = settings.value("ValueASymbolCode", "").toString();
    holder.valueBSymbolCode = settings.value("ValueBSymbolCode", "").toString();
    holder.variableACode = settings.value("VariableACode", "").toString();
    holder.variableBCode = settings.value("VariableBCode", "").toString();
    holder.variableBExact = settings.value("VariableBExact", 0.0).toDouble();
    holder.variableBFeeIndex = settings.value("VariableBFeeIndex", -1).toInt();
    holder.variableBModeIndex = settings.value("VariableBModeIndex", -1).toInt();
    holder.variableBPercentChecked = settings.value("VariableBPercentChecked", false).toBool();
    holder.variableBplusMinus = settings.value("VariableBplusMinus", "").toString();
    holder.variableBSymbolCode = settings.value("VariableBSymbolCode", "").toString();
    holder.description = settings.value("Description", "").toString();
    holder.delayMilliseconds = JulyMath::cutDoubleDecimalsCopy(settings.value("Delay", 0.0).toReal(), 3, false);
    settings.endGroup();
    return holder;
}

RuleHolder RuleScriptParser::readHolderFromFile(QString& file, QString section)
{
    QSettings settings(file, QSettings::IniFormat);
    return readHolderFromSettings(settings, section);
}

QString RuleScriptParser::holderToScript(RuleHolder& holder, bool testMode)
{
    bool execImmediately = holder.variableACode == QLatin1String("IMMEDIATELY");
    bool eventIsTrade = holder.variableACode == QLatin1String("LastTrade") || holder.variableACode == QLatin1String("MyLastTrade");

    QString script = "function executeRule()\n{\n";

    if (!execImmediately)
        script = "var executed=false;\n" + script + " executed=true;\n";

    if (!testMode)
    {
        if (holder.isTradingRule())
        {
            script += " if(trader.get(\"ApiLag\")>10)\n"
                      " {\n"
                      " trader.log(\"Api lag is to high\");\n"
                      " trader.delay(1,\"executeRule()\");\n"
                      " return;\n"
                      " }\n\n";
        }

        if (holder.thanTypeIndex < 4)
        {
            double amount = holder.thanAmount;

            if (holder.thanAmountPercentChecked)
            {
                amount /= 100.0;

                if (holder.thanTypeIndex == 0)
                    script += " var amount = trader.get(\"Balance\",\"" + baseValues.currentPair.currAStr + "\");\n";
                else
                    script += " var amount = trader.get(\"Balance\",\"" + baseValues.currentPair.currBStr + "\");\n";

                if (amount != 0.0 && amount != 1.0)
                    script += " amount *= " + JulyMath::textFromDouble(amount) + ";\n";
            }
            else
                script += " var amount = " + JulyMath::textFromDouble(amount) + ";\n";

            if (amount != 0.0)
            {
                if (holder.thanAmountFeeIndex == 1)
                    script += " amount *= (1.0 + trader.get(\"" + holder.valueBSymbolCode + "\" , \"Fee\") / 100.0);\n";

                if (holder.thanAmountFeeIndex == 2)
                    script += " amount *= (1.0 - trader.get(\"" + holder.valueBSymbolCode + "\" , \"Fee\") / 100.0);\n";
            }

            if (!script.isEmpty())
                script += "\n";

            if (holder.thanPriceTypeCode == "EXACT")
                script += " var price = " + JulyMath::textFromDouble(holder.thanPrice) + ";\n";
            else
            {
                script += " var price = trader.get(\"" + holder.tradeSymbolCode + "\" , \"" + holder.thanPriceTypeCode + "\");\n";

                if (holder.thanPricePercentChecked)
                    script += " price " + holder.thanPricePlusMinusText + "= price * " +
                              JulyMath::textFromDouble(holder.thanPrice / 100.0) + ";\n";
                else if (holder.thanPrice != 0.0)
                    script += " price " + holder.thanPricePlusMinusText + "= " + JulyMath::textFromDouble(holder.thanPrice) + ";\n";

                if (holder.thanPriceFeeIndex == 1)
                    script += " price *= ( 1.0 + trader.get(\"" + holder.valueBSymbolCode + "\" , \"Fee\") / 100.0 );\n";

                if (holder.thanPriceFeeIndex == 2)
                    script += " price *= ( 1.0 - trader.get(\"" + holder.valueBSymbolCode + "\" , \"Fee\") / 100.0 );\n";
            }

            if (!script.isEmpty())
                script += "\n";

            switch (holder.thanTypeIndex)
            {
            case 0: // Sell
                script += " trader.sell(\"" + holder.tradeSymbolCode + "\" , amount , price)";
                break;

            case 1: // Buy
                if (holder.thanAmountPercentChecked)
                    script += " trader.buy(\"" + holder.tradeSymbolCode + "\" , amount / price , price)";
                else
                    script += " trader.buy(\"" + holder.tradeSymbolCode + "\" , amount , price)";

                break;

            case 2: // Receive
                script += " trader.sell(\"" + holder.tradeSymbolCode + "\" , amount / price , price)";
                break;

            case 3: // Spend
                script += " trader.buy(\"" + holder.tradeSymbolCode + "\" , amount / price , price)";
                break;

            default:
                break;
            }

            if (!script.isEmpty())
                script += ";\n";
        }
        else
        {
            switch (holder.thanTypeIndex)
            {
            case 4: // Cancel all Orders
                script += " trader.cancelOrders();\n";
                break;

            case 5: // Cancel Asks
                script += " trader.cancelAsks();\n";
                break;

            case 6: // Cancel Bids
                script += " trader.cancelBids();\n";
                break;

            case 7: // Start group
                script += " trader.groupStart(\"" + holder.thanText + "\");\n";
                break;

            case 8: // Stop group
                script += " trader.groupStop(\"" + holder.thanText + "\");\n";
                break;

            case 9: // Beep
                script += " trader.beep();\n";
                break;

            case 10: // Play Sound
                script += " trader.playWav(\"" + holder.thanText + "\");\n";
                break;

            case 11: // Start app
                script += " trader.startApp(\"" + holder.thanText + "\");\n";
                break;

            case 12: // Say text
                {
                    QString sayText;

                    if (!holder.sayCode.isEmpty())
                        sayText = ", trader.get(\"" + holder.sayCode + "\")";

                    script += " trader.say(\"" + holder.thanText + "\"" + sayText + ");\n";
                }
                break;
            }
        }

        script += " trader.groupDone();\n";
    }
    else
        script += " trader.test(1);\n trader.groupStop();\n";

    script += "}";

    bool haveDelay = holder.delayMilliseconds > 0.0001 && !testMode;

    QString executeRuleLine;

    if (haveDelay)
        executeRuleLine = " trader.delay(" + JulyMath::textFromDoubleStr(holder.delayMilliseconds, 3, 0) + ",\"executeRule()\");";
    else
        executeRuleLine = " executeRule();";

    if (execImmediately)
    {
        script += "\n" + executeRuleLine;
    }
    else
    {
        QString indicatorBValue;
        QString indicatorB;
        QString realtime;
        QString ifLine;

        QString comparationText = holder.comparationText;

        if (comparationText == QLatin1String("="))
            comparationText = QLatin1String("==");
        else if (comparationText == QLatin1String("<>"))
            comparationText = QLatin1String("!=");

        if (!eventIsTrade)
        {
            if (holder.variableBCode == "EXACT")
            {
                ifLine = " if(value " + comparationText + " " + JulyMath::textFromDouble(holder.variableBExact) + ")";
            }
            else
            {
                indicatorBValue = "trader.get(\"" + holder.variableBSymbolCode + "\" , \"" + holder.variableBCode + "\")";
                indicatorB = indicatorBValue + ";\n";

                if (holder.variableBPercentChecked)
                    indicatorB += " baseVariable " + holder.variableBplusMinus + "= baseVariable*" +
                                  JulyMath::textFromDouble(holder.variableBExact / 100.0) + ";\n";
                else if (holder.variableBExact != 0.0)
                    indicatorB +=
                        " baseVariable " + holder.variableBplusMinus + "= " + JulyMath::textFromDouble(holder.variableBExact) + ";\n";

                if (holder.variableBFeeIndex > 0)
                {
                    QString sign = (holder.variableBFeeIndex == 1 ? "+" : "-");
                    indicatorB +=
                        " baseVariable *= (1.0 " + sign + " trader.get(\"" + holder.valueBSymbolCode + "\" , \"Fee\") / 100.0);\n";
                }

                if (holder.variableBModeIndex == 0)
                    realtime = " calcBaseVariable();\n";
                else if (holder.variableBModeIndex == 2)
                {
                    bool haveLessThan = comparationText.contains("<");
                    bool haveMoreThan = comparationText.contains(">");

                    if (haveLessThan)
                        realtime = " if(value > " + indicatorBValue + ")calcBaseVariable();\n";

                    if (haveMoreThan)
                        realtime = " if(value < " + indicatorBValue + ")calcBaseVariable();\n";
                }

                ifLine = " if(value " + comparationText + " baseVariable)";
                script += "\n\nvar baseVariable = calcBaseVariable();\n"
                          "function calcBaseVariable()\n"
                          "{\n"
                          " baseVariable = " +
                          indicatorB +
                          " return baseVariable;\n"
                          "}";
            }
        }

        QString eventName = holder.variableACode;

        if (!eventIsTrade && eventName.startsWith(QLatin1String("Balance"), Qt::CaseInsensitive))
        {
            QString currAStr, currBStr;
            int posSplitter = holder.valueASymbolCode.indexOf('/');

            if (posSplitter == -1)
            {
                currAStr = holder.valueASymbolCode.left(3);
                currBStr = holder.valueASymbolCode.mid(3, 3);
            }
            else
            {
                currAStr = holder.valueASymbolCode.left(posSplitter);
                currBStr = holder.valueASymbolCode.right(holder.valueASymbolCode.size() - posSplitter - 1);

                if (currBStr.size() > 7)
                {
                    fixCurrency(currBStr, "trading");
                    fixCurrency(currBStr, "exchange");
                }
            }

            if (eventName.endsWith("A", Qt::CaseInsensitive))
                eventName = "Balance\",\"" + currAStr;
            else if (eventName.endsWith("B", Qt::CaseInsensitive))
                eventName = "Balance\",\"" + currBStr;
        }

        script += "\n\ntrader.on(\"" + eventName +
                  "\").changed()\n"
                  "{\n"
                  " if(executed)return;\n"
                  " if(symbol != \"" +
                  holder.valueASymbolCode + "\")return;\n";
        script += realtime + ifLine + executeRuleLine;

        if (testMode)
            script += "\n else { trader.test(2); trader.stopGroup(); }\n";

        script += "\n}\n";
    }

    return script;
}

void RuleScriptParser::fixCurrency(QString& currency, const QString removedText)
{
    if (currency.endsWith(removedText))
        currency.chop(removedText.size());
}
