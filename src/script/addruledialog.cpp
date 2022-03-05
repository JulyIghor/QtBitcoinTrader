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

#include "addruledialog.h"
#include "exchange/exchange.h"
#include "iniengine.h"
#include "julymath.h"
#include "julyspinboxfix.h"
#include "main.h"
#include "percentpicker.h"
#include "rulescriptparser.h"
#include "rulewidget.h"
#include "scriptobject.h"
#include "ui_addruledialog.h"
#include "utils/traderspinbox.h"
#include <QComboBox>
#include <QFileDialog>
#include <QMessageBox>

AddRuleDialog::AddRuleDialog(const QString& grName, QWidget* par) :
    QDialog(par), saveClicked(false), groupName(grName), pendingFix(true), ruleIsEnabled(false), ui(new Ui::AddRuleDialog)
{
    ui->setupUi(this);
    ui->buttonSaveRule->setVisible(false);
    ui->scriptCodeGroupbox->setVisible(false);
    on_thanAmountPercent_toggled(false);
    on_thanPricePercent_toggled(false);
    on_variableBPercent_toggled(false);

    ui->groupBoxWhen->setTitle(julyTr("WHEN", "When"));

    ui->sayCode->insertItem(ui->sayCode->count(), julyTr("NOT_USED", "Not Used"), "");

    for (QDoubleSpinBox* spinBox : mainWindow.findChildren<QDoubleSpinBox*>())
    {
        QString scriptName = spinBox->whatsThis();

        if (scriptName.isEmpty())
            continue;

        QString translatedName = julyTranslator.translateString(
            "INDICATOR_" + (scriptName.startsWith("Balance") ? "BALANCE" : scriptName.toUpper()), scriptName);

        if (scriptName.startsWith("BalanceA"))
            translatedName = translatedName.arg(baseValues.currentPair.currAStr);
        else
        {
            if (scriptName.startsWith("BalanceB"))
                translatedName = translatedName.arg(baseValues.currentPair.currBStr);
        }

        ui->variableA->insertItem(ui->variableA->count(), translatedName, scriptName);
        ui->variableB->insertItem(ui->variableB->count(), translatedName, scriptName);
        ui->sayCode->insertItem(ui->sayCode->count(), translatedName, scriptName);

        if (spinBox->accessibleName() == "PRICE")
            ui->thanPriceType->insertItem(ui->thanPriceType->count(), translatedName, scriptName);

        usedSpinBoxes << spinBox;
    }

    ui->variableA->insertItem(ui->variableA->count(), julyTr("RULE_IMMEDIATELY_EXECUTION", "Execute Immediately"), "IMMEDIATELY");

    ui->variableA->insertItem(ui->variableB->count(), julyTr("RULE_MY_LASTTRADE_CHANGED", "My order sold or bought"), "MyLastTrade");
    ui->variableA->insertItem(ui->variableB->count(), julyTr("RULE_LASTTRADE_CHANGED", "Market order sold or bought"), "LastTrade");

    ui->variableB->insertItem(ui->variableB->count(), julyTr("RULE_EXACT_VALUE", "Exact value"), "EXACT");
    ui->thanPriceType->insertItem(ui->thanPriceType->count(), julyTr("RULE_EXACT_VALUE", "Exact value"), "EXACT");

    int lastPriceInt = ui->variableA->findData("LastPrice");

    if (lastPriceInt > -1)
        ui->variableA->setCurrentIndex(lastPriceInt);

    lastPriceInt = ui->variableB->findData("LastPrice");

    if (lastPriceInt > -1)
        ui->variableB->setCurrentIndex(lastPriceInt);

    lastPriceInt = ui->thanPriceType->findData("LastPrice");

    if (lastPriceInt > -1)
        ui->thanPriceType->setCurrentIndex(lastPriceInt);

    ui->thanType->insertItem(ui->thanType->count(), julyTr("RULE_THAN_SELL", "Sell %1").arg(baseValues.currentPair.currAStr), "TRADE");
    ui->thanType->insertItem(ui->thanType->count(), julyTr("RULE_THAN_BUY", "Buy %1").arg(baseValues.currentPair.currAStr), "TRADE");

    ui->thanType->insertItem(
        ui->thanType->count(), julyTr("RULE_THAN_RECEIVE", "Receive %1").arg(baseValues.currentPair.currBStr), "TRADE");
    ui->thanType->insertItem(ui->thanType->count(), julyTr("RULE_THAN_SPEND", "Spend %1").arg(baseValues.currentPair.currBStr), "TRADE");

    ui->thanType->insertItem(ui->thanType->count(), julyTr("RULE_THAN_CANCEL_ALL", "Cancel All Orders"), "NOPARAMS");
    ui->thanType->insertItem(ui->thanType->count(), julyTr("RULE_THAN_CANCEL_ASKS", "Cancel Asks"), "NOPARAMS");
    ui->thanType->insertItem(ui->thanType->count(), julyTr("RULE_THAN_CANCEL_BIDS", "Cancel Bids"), "NOPARAMS");
    ui->thanType->insertItem(ui->thanType->count(), julyTr("RULE_THAN_START_GROUP", "Start Group or Script"), "NAME");
    ui->thanType->insertItem(ui->thanType->count(), julyTr("RULE_THAN_STOP_GROUP", "Stop Group or Script"), "NAME");
    ui->thanType->insertItem(ui->thanType->count(), julyTr("RULE_THAN_BEEP", "Beep"), "BEEP");
    ui->thanType->insertItem(ui->thanType->count(), julyTr("RULE_THAN_PLAY", "Play Sound"), "PLAY");
    ui->thanType->insertItem(ui->thanType->count(), julyTr("RULE_THAN_START_APP", "Start Application"), "PROGRAM");
    ui->thanType->insertItem(ui->thanType->count(), julyTr("RULE_THAN_SAY_TEXT", "Say Text"), "SAY");

    ui->variableBMode->setItemText(0, julyTr("RULE_TYPE_REALTIME", "Realtime comparison"));
    ui->variableBMode->setItemText(1, julyTr("RULE_TYPE_SAVEONSTART", "Fixed. Save base value once at rule starts"));
    ui->variableBMode->setItemText(2, julyTr("RULE_TYPE_FIXED", "Trailing. Save base value on opposide direction"));

    ui->variableBFee->setItemText(0, julyTr("RULE_NOFEE", "No Fee"));
    ui->variableBFee->setItemText(1, julyTr("RULE_PLUSFEE", "+ Fee"));
    ui->variableBFee->setItemText(2, julyTr("RULE_MINUSFEE", "- Fee"));

    ui->thanAmountFee->setItemText(0, julyTr("RULE_NOFEE", "No Fee"));
    ui->thanAmountFee->setItemText(1, julyTr("RULE_PLUSFEE", "+ Fee"));
    ui->thanAmountFee->setItemText(2, julyTr("RULE_MINUSFEE", "- Fee"));

    ui->thanPriceFee->setItemText(0, julyTr("RULE_NOFEE", "No Fee"));
    ui->thanPriceFee->setItemText(1, julyTr("RULE_PLUSFEE", "+ Fee"));
    ui->thanPriceFee->setItemText(2, julyTr("RULE_MINUSFEE", "- Fee"));

    on_thanType_currentIndexChanged(ui->thanType->currentIndex());
    on_variableA_currentIndexChanged(ui->variableA->currentIndex());
    on_variableB_currentIndexChanged(ui->variableB->currentIndex());
    on_valueBSymbol_currentIndexChanged(ui->valueBSymbol->currentIndex());
    on_thanSymbol_currentIndexChanged(ui->thanSymbol->currentIndex());
    on_valueASymbol_currentIndexChanged(ui->valueASymbol->currentIndex());

    for (QDoubleSpinBox* spinBox : findChildren<QDoubleSpinBox*>())
        new JulySpinBoxFix(spinBox);

    QString baseSymbol = baseValues.currentPair.symbolSecond();

    for (QComboBox* comboBox : findChildren<QComboBox*>())
    {
        if (comboBox->accessibleName() != "SYMBOL")
            continue;

        int selectedRow = -1;

        for (int n = 0; n < IniEngine::getPairsCount(); ++n)
        {
            QString curSymbol = IniEngine::getPairSymbolSecond(n);

            if (curSymbol == baseSymbol)
                selectedRow = n;

            // else if(!currentExchange->multiCurrencyTradeSupport)continue;
            comboBox->insertItem(comboBox->count(), IniEngine::getPairName(n), curSymbol);
        }

        if (selectedRow > -1)
            comboBox->setCurrentIndex(selectedRow);
    }

    ui->thanAmountFee->setCurrentIndex(2);

    for (QComboBox* comboBox : findChildren<QComboBox*>())
        connect(comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AddRuleDialog::reCacheCode);

    for (QDoubleSpinBox* spinBox : findChildren<QDoubleSpinBox*>())
        connect(spinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &AddRuleDialog::reCacheCode);

    for (QCheckBox* checkBox : findChildren<QCheckBox*>())
        connect(checkBox, &QCheckBox::toggled, this, &AddRuleDialog::reCacheCode);

    for (QRadioButton* checkBox : findChildren<QRadioButton*>())
        connect(checkBox, &QRadioButton::toggled, this, &AddRuleDialog::reCacheCode);

    setWindowFlags(Qt::WindowCloseButtonHint);
    setWindowTitle(julyTranslator.translateButton("ADD_RULE", "Add Rule"));

    julyTranslator.translateUi(this);

    int selectedRow = -1;
    auto* parentRuleWidget = qobject_cast<RuleWidget*>(par);
    auto* parentScriptWidget = qobject_cast<ScriptWidget*>(par);

    for (RuleWidget* currentGroup : mainWindow.ui.tabRules->findChildren<RuleWidget*>())
    {
        if (currentGroup == nullptr)
            continue;

        if (currentGroup == parentRuleWidget)
            selectedRow = ui->groupName->count();

        ui->groupName->addItem(currentGroup->windowTitle(), currentGroup->property("FileName").toString());
    }

    for (ScriptWidget* currentGroup : mainWindow.ui.tabRules->findChildren<ScriptWidget*>())
    {
        if (currentGroup == nullptr)
            continue;

        if (currentGroup == parentScriptWidget)
            selectedRow = ui->groupName->count();

        ui->groupName->addItem(currentGroup->windowTitle(), currentGroup->property("FileName").toString());
    }

    if (selectedRow < 0)
        selectedRow = ui->groupName->findText(groupName);

    if (selectedRow > -1)
        ui->groupName->setCurrentIndex(selectedRow);
    else
        ui->groupName->addItem(getFreeGroupName(), "");

    QTimer::singleShot(200, this, SLOT(reCacheCode()));
    QTimer::singleShot(201, this, SLOT(fixSizeWindow()));
}

AddRuleDialog::~AddRuleDialog()
{
    delete ui;
}

QString AddRuleDialog::getGroupName()
{
    return ui->groupName->currentText();
}

QString AddRuleDialog::getFreeGroupName()
{
    QString groupLabel = julyTr("RULE_GROUP", "Group");
    QString newGroupName = groupLabel;

    int incr = 0;
    QStringList existingGroups = mainWindow.getRuleGroupsNames();
    existingGroups << mainWindow.getScriptGroupsNames();

    while (existingGroups.contains(newGroupName))
    {
        newGroupName = groupLabel;

        if (incr > 0)
            newGroupName += " " + QString::number(incr);

        ++incr;
    }

    return newGroupName;
}

void AddRuleDialog::fillByHolder(RuleHolder& holder, bool running)
{
    ui->thanAmountPercent->setChecked(holder.thanAmountPercentChecked);
    ui->thanPricePercent->setChecked(holder.thanPricePercentChecked);
    ui->variableBPercent->setChecked(holder.variableBPercentChecked);

    setComboIndex(ui->thanAmountFee, holder.thanAmountFeeIndex);
    setComboIndex(ui->thanPriceFee, holder.thanPriceFeeIndex);
    setComboIndex(ui->thanType, holder.thanTypeIndex);
    setComboIndex(ui->variableBFee, holder.variableBFeeIndex);
    setComboIndex(ui->variableBMode, holder.variableBModeIndex);

    ui->thanAmount->setValue(holder.thanAmount);
    ui->thanPriceValue->setValue(holder.thanPrice);
    ui->variableBExact->setValue(holder.variableBExact);

    ui->comparation->setCurrentIndex(ui->comparation->findText(holder.comparationText));

    setComboIndex(ui->thanPricePlusMinus, holder.thanPricePlusMinusText);
    setComboIndex(ui->variableBplusMinus, holder.variableBplusMinus);

    setComboIndexByData(ui->thanPriceType, holder.thanPriceTypeCode);

    QString sayParseCode = holder.sayCode;
    sayParseCode.replace("Balance\",\"" + baseValues.currentPair.currAStr, "BalanceA");
    sayParseCode.replace("Balance\",\"" + baseValues.currentPair.currBStr, "BalanceB");
    setComboIndexByData(ui->sayCode, sayParseCode);

    ui->thanText->setText(holder.thanText);

    setComboIndexByData(ui->thanSymbol, holder.tradeSymbolCode);
    setComboIndexByData(ui->valueASymbol, holder.valueASymbolCode);
    setComboIndexByData(ui->valueBSymbol, holder.valueBSymbolCode);
    setComboIndexByData(ui->variableA, holder.variableACode);
    setComboIndexByData(ui->variableB, holder.variableBCode);
    setComboIndexByData(ui->valueBSymbol, holder.variableBSymbolCode);

    ui->descriptionText->setText(holder.description);

    ui->delayValue->setValue(holder.delayMilliseconds);

    ui->buttonSaveRule->setVisible(true);

    ruleIsEnabled = running;
}

bool AddRuleDialog::isRuleEnabled() const
{
    return ruleIsEnabled;
}

void AddRuleDialog::setComboIndexByData(QComboBox* list, QString& data)
{
    if (list == nullptr)
        return;

    int find = list->findData(data);

    if (find < 0)
    {
        // qDebug()<<"Critical error. Can't find:"<<data;
        return;
    }

    list->setCurrentIndex(find);
}

void AddRuleDialog::setComboIndex(QComboBox* list, QString& text)
{
    if (list == nullptr)
        return;

    int find = list->findText(text);

    if (find < 0)
    {
        // qDebug()<<"Critical error. Can't find:"<<text;
        return;
    }

    list->setCurrentIndex(find);
}

void AddRuleDialog::setComboIndex(QComboBox* list, int& row)
{
    if (list == nullptr)
        return;

    if (row < 0 || row >= list->count())
        return;

    list->setCurrentIndex(row);
}

RuleHolder AddRuleDialog::getRuleHolder()
{
    if (!isVisible())
        return lastHolder;

    lastHolder.thanAmountPercentChecked = ui->thanAmountPercent->isChecked();
    lastHolder.thanPricePercentChecked = ui->thanPricePercent->isChecked();
    lastHolder.variableBPercentChecked = ui->variableBPercent->isVisible() && ui->variableBPercent->isChecked();
    lastHolder.thanAmountFeeIndex = ui->thanAmountFee->currentIndex();
    lastHolder.thanPriceFeeIndex = ui->thanPriceFee->currentIndex();
    lastHolder.thanTypeIndex = ui->thanType->currentIndex();
    lastHolder.variableBFeeIndex = !ui->variableBFee->isVisible() ? -1 : ui->variableBFee->currentIndex();
    lastHolder.variableBModeIndex = ui->variableBMode->currentIndex();
    lastHolder.thanAmount = ui->thanAmount->value();
    lastHolder.thanPrice = ui->thanPriceValue->value();
    lastHolder.variableBExact = ui->variableBExact->value();
    lastHolder.comparationText = ui->comparation->currentText();
    lastHolder.thanPricePlusMinusText = ui->thanPricePlusMinus->currentText();
    lastHolder.thanPriceTypeCode = comboCurrentData(ui->thanPriceType);

    QString grName = ui->thanText->text();

    if (grName.isEmpty())
        grName = groupName;

    lastHolder.thanText = grName;

    lastHolder.tradeSymbolCode = comboCurrentData(ui->thanSymbol);
    lastHolder.valueASymbolCode = comboCurrentData(ui->valueASymbol);
    lastHolder.valueBSymbolCode = comboCurrentData(ui->valueBSymbol);
    lastHolder.variableACode = comboCurrentData(ui->variableA);
    lastHolder.variableBCode = comboCurrentData(ui->variableB);
    lastHolder.variableBplusMinus = ui->variableBplusMinus->currentText();
    lastHolder.variableBSymbolCode = comboCurrentData(ui->valueBSymbol);
    lastHolder.description = ui->descriptionText->text();
    lastHolder.delayMilliseconds = (int)(ui->delayValue->value() * 1000.0) / 1000.0;
    lastHolder.sayCode = comboCurrentData(ui->sayCode);

    if (lastHolder.sayCode == "BalanceA")
        lastHolder.sayCode = "Balance\",\"" + baseValues.currentPair.currAStr;

    if (lastHolder.sayCode == "BalanceB")
        lastHolder.sayCode = "Balance\",\"" + baseValues.currentPair.currBStr;

    return lastHolder;
}

void AddRuleDialog::reCacheCode()
{
    QString descriptionText;

    if (ui->delayValue->value() > 0)
        descriptionText = julyTr("DELAY_SEC", "Delay %1 sec").arg(JulyMath::textFromDoubleStr(ui->delayValue->value())) + " ";

    QString currentAType = comboCurrentData(ui->variableA);

    if (currentAType == QLatin1String("IMMEDIATELY"))
    {
        if (descriptionText.isEmpty())
            descriptionText = julyTr("RULE_IMMEDIATELY_EXECUTION", "Execute immediately");
        else
            descriptionText.remove(descriptionText.size() - 1, 1);
    }
    else
    {
        bool requiresBaseValue = currentAType != QLatin1String("MyLastTrade") && currentAType != QLatin1String("LastTrade");
        descriptionText += julyTr("WHEN", "When") + " " + ui->variableA->currentText();

        if (ui->valueASymbol->isVisible())
            descriptionText += " (" + ui->valueASymbol->currentText() + ")";

        if (requiresBaseValue)
        {
            descriptionText += " " + ui->comparation->currentText() + " " + ui->variableB->currentText();

            if (ui->valueBSymbol->isVisible())
                descriptionText += " (" + ui->valueBSymbol->currentText() + ")";

            bool bExact = comboCurrentData(ui->variableB) == "EXACT";

            if (ui->variableBExact->value() != 0.0 || bExact)
            {
                if (!bExact)
                    descriptionText += " " + ui->variableBplusMinus->currentText();

                descriptionText += " " + JulyMath::textFromDouble(ui->variableBExact->value(), 8, 0);

                if (ui->variableBPercent->isChecked())
                    descriptionText += "%";
            }

            if (ui->variableBFee->currentIndex() > 0)
            {
                if (ui->variableBFee->currentIndex() == 1)
                    descriptionText += " + ";
                else
                    descriptionText += " - ";

                descriptionText += julyTr("LOG_FEE", "fee");
            }

            if (ui->variableBMode->isVisible())
                descriptionText += " (" + ui->variableBMode->currentText() + ")";
        }
    }

    descriptionText += " " + julyTr("THEN", "then") + " " + ui->thanType->currentText();

    if (ui->thanAmount->isVisible())
    {
        QString sign;
        CurrencyPairItem pairItem;
        pairItem = baseValues.currencyPairMap.value(comboCurrentData(ui->valueBSymbol), pairItem);

        if (!pairItem.symbolSecond().isEmpty())
            sign = pairItem.currASign;

        descriptionText += " " + sign + JulyMath::textFromDouble(ui->thanAmount->value(), 8, 0);

        if (ui->thanAmountPercent->isChecked())
            descriptionText += "%";

        if (ui->thanAmountFee->currentIndex() > 0)
        {
            if (ui->thanAmountFee->currentIndex() == 1)
                descriptionText += " + ";
            else
                descriptionText += " - ";

            descriptionText += julyTr("LOG_FEE", "fee");
        }
    }

    if (ui->thanPriceType->isVisible())
    {
        QString atPrice;
        bool priceExact = comboCurrentData(ui->thanPriceType) == QLatin1String("EXACT");

        if (!priceExact)
            atPrice = ui->thanPriceType->currentText();

        if (ui->thanPriceValue->isVisible() && ui->thanPriceValue->value() != 0.0)
        {
            if (!priceExact)
                atPrice += " " + ui->thanPricePlusMinus->currentText();

            QString sign;
            CurrencyPairItem pairItem;
            pairItem = baseValues.currencyPairMap.value(comboCurrentData(ui->thanSymbol), pairItem);

            if (!pairItem.symbolSecond().isEmpty())
                sign = pairItem.currBSign;

            atPrice += " " + sign + JulyMath::textFromDouble(ui->thanPriceValue->value(), 8, 0);
        }

        if (ui->thanPriceFee->currentIndex() > 0)
        {
            if (ui->thanPriceFee->currentIndex() == 1)
                atPrice += " + ";
            else
                atPrice += " - ";

            atPrice += julyTr("LOG_FEE", "fee");
        }

        descriptionText += " " + julyTr("AT", "at %1").arg(atPrice);
    }

    QString grName = ui->thanText->text();

    if (grName.isEmpty())
        grName = groupName;

    if (ui->thanText->isVisible())
        descriptionText += " (" + grName + ")";

    ui->descriptionText->setText(descriptionText);

    RuleHolder holder = getRuleHolder();
    ui->scriptCodePreview->setPlainText(RuleScriptParser::holderToScript(holder));
}

void AddRuleDialog::fixSizeWindow()
{
    fixSize(true);
    fixSize(false);
}

void AddRuleDialog::fixSize(bool fitToWindow)
{
    if (fitToWindow)
    {
        mainWindow.fixAllChildButtonsAndLabels(this);
        setMinimumWidth(mainWindow.minimumSizeHint().width());
        setMinimumHeight(300);
    }

    QSize preferedSize = minimumSizeHint();

    if (pendingFix)
    {
        pendingFix = false;
        fitToWindow = true;
    }

    int minWidth = fitToWindow ? (mainWindow.width() - 20) : 0;

    if (ui->scriptCodeGroupbox->isVisible())
        resize(qMax(preferedSize.width(), minWidth), height());
    else
        resize(qMax(preferedSize.width(), minWidth), qMax(preferedSize.height(), 250));
}

void AddRuleDialog::on_variableA_currentIndexChanged(int /*unused*/)
{
    QString variableAType = comboCurrentData(ui->variableA);

    bool immediately = variableAType == QLatin1String("IMMEDIATELY");
    bool tradeEvent = variableAType == QLatin1String("MyLastTrade") || variableAType == QLatin1String("LastTrade");

    ui->whenValueGroupBox->setEnabled(!immediately && !tradeEvent);
    ui->comparation->setEnabled(!immediately && !tradeEvent);

    ui->valueALabel->setVisible(!immediately);
    ui->valueASymbol->setVisible(!immediately);
    fixSize();
}

void AddRuleDialog::on_variableB_currentIndexChanged(int index)
{
    QString varBType = comboData(ui->variableB, index);
    bool exact = varBType == "EXACT";
    ui->variableBplusMinus->setVisible(!exact);
    ui->variableBFee->setVisible(!exact);
    ui->modeFon->setVisible(!exact);
    ui->variableBModeLabel->setVisible(!exact);
    ui->variableBPercent->setVisible(!exact);
    on_variableBPercent_toggled(ui->variableBPercent->isChecked());

    fixSize();
}

void AddRuleDialog::on_thanAmountPercent_toggled(bool checked)
{
    ui->thanAmountPercentButton->setVisible(checked);

    if (ui->thanAmountPercentButton->isVisible())
    {
        ui->thanAmount->setMaximum(200.0);
        ui->thanAmount->setMinimum(0.00000001);
    }
    else
    {
        ui->thanAmount->setMaximum(9999999999.9);
        ui->thanAmount->setMinimum(-9999999999.9);
    }

    fixSize();
}

void AddRuleDialog::on_thanType_currentIndexChanged(int index)
{
    currentThanType = comboData(ui->thanType, index);
    bool trade = currentThanType == "TRADE";
    bool noParams = currentThanType == "NOPARAM";
    bool name = currentThanType == "NAME";
    bool beep = currentThanType == "BEEP";
    bool wav = currentThanType == "PLAY";
    bool say = currentThanType == "SAY";
    bool program = currentThanType == "PROGRAM";

    ui->doubleValueFon->setVisible(!noParams && trade);
    ui->thanTextBrowse->setVisible(!noParams && (wav || program));
    ui->thanText->setVisible(!noParams && (wav || program || name || say));
    ui->clearTextButton->setVisible(ui->thanText->isVisible());
    ui->nameLabel->setVisible(name);
    ui->thanSymbol->setVisible(trade);
    ui->thanSymbolLabel->setVisible(trade);
    ui->sayCode->setVisible(say);
    ui->sayLabelPlus->setVisible(say);

    ui->playButton->setVisible(!noParams && (wav || beep || say));

    fixSize();

    if (wav || program)
        on_thanTextBrowse_clicked();
}

void AddRuleDialog::on_playButton_clicked()
{
    if (currentThanType == "SAY")
    {
        QString sayText = ui->thanText->text();

        if (ui->sayCode->currentIndex() > 0)
        {
            QString currentCode = comboCurrentData(ui->sayCode);

            for (QDoubleSpinBox* spin : usedSpinBoxes)
                if (spin->whatsThis() == currentCode)
                {
                    int detectDoublePoint = 0;

                    bool decimalComma = QLocale().decimalPoint() == QChar(',');

                    if (!decimalComma)
                        decimalComma = QLocale().country() == QLocale::Ukraine || QLocale().country() == QLocale::RussianFederation;

                    if (decimalComma)
                        detectDoublePoint = 2;
                    else
                        detectDoublePoint = 1;

                    QString number = JulyMath::textFromDouble(spin->value(), 8, 0);
                    int crop = number.size() - 1;

                    while (crop > 0 && number.at(crop) == QLatin1Char('0'))
                        crop--;

                    if (crop > 0 && number.at(crop) == QLatin1Char('.'))
                        crop--;

                    if (crop >= 0 && crop < number.size())
                        number.resize(crop + 1);

                    if (detectDoublePoint == 2)
                        number.replace(QLatin1Char('.'), QLatin1Char(','));

                    sayText.append(" " + number);
                }
        }

        mainWindow.sayText(sayText);
    }
    else if (currentThanType == "BEEP")
        mainWindow.beep();
    else if (currentThanType == "PLAY")
        mainWindow.play(ui->thanText->text());
}

void AddRuleDialog::on_thanTextBrowse_clicked()
{
    QString lastRulesDir = mainWindow.iniSettings->value("UI/LastRulesPath", baseValues.desktopLocation).toString();

    if (!QFile::exists(lastRulesDir))
        lastRulesDir = baseValues.desktopLocation;

    QString description;
    QString mask = "*";

    if (currentThanType == "PLAY")
    {
        description = julyTr("PLAY_SOUND_WAV", "Open WAV file");
        mask = "*.wav";
    }

    if (currentThanType == "PROGRAM")
    {
        description = julyTr("OPEN_ANY_FILE", "Open any file");
        mask = "*";
    }

    QString fileName = QFileDialog::getOpenFileName(this, description, lastRulesDir, "(" + mask + ")");

    if (fileName.isEmpty())
        return;

    mainWindow.iniSettings->setValue("UI/LastRulesPath", QFileInfo(fileName).dir().path());
    mainWindow.iniSettings->sync();

#ifdef Q_OS_WIN
    fileName.replace("/", "\\");
#endif

    ui->thanText->setText(fileName);
}

void AddRuleDialog::on_thanPriceType_currentIndexChanged(int index)
{
    QString thanPriceType = comboData(ui->thanPriceType, index);
    bool exactPrice = thanPriceType == "EXACT";

    ui->thanPricePlusMinus->setVisible(!exactPrice);
    ui->thanPricePercent->setVisible(!exactPrice);

    if (ui->thanPricePercentButton->isVisible())
    {
        ui->thanPriceValue->setMaximum(200.0);
        ui->thanPriceValue->setMinimum(0.00000001);
    }
    else
    {
        ui->thanPriceValue->setMaximum(9999999999.9);
        ui->thanPriceValue->setMinimum(-9999999999.9);
    }

    ui->thanPriceFee->setVisible(!exactPrice);

    on_thanPricePercent_toggled(ui->thanPricePercent->isChecked());
    fixSize();
}

void AddRuleDialog::on_variableBPercent_toggled(bool checked)
{
    ui->variableBPercentButton->setVisible(checked && ui->variableBPercent->isVisible());

    if (ui->variableBPercentButton->isVisible())
    {
        ui->variableBExact->setMaximum(200.0);
        ui->variableBExact->setMinimum(0.00000001);
    }
    else
    {
        ui->variableBExact->setMaximum(9999999999.9);
        ui->variableBExact->setMinimum(-9999999999.9);
    }

    fixSize();
}

void AddRuleDialog::on_thanPricePercent_toggled(bool checked)
{
    ui->thanPricePercentButton->setVisible(checked);

    if (ui->thanPricePercentButton->isVisible())
    {
        ui->thanPriceValue->setMaximum(200.0);
        ui->thanPriceValue->setMinimum(0.00000001);
    }
    else
    {
        ui->thanPriceValue->setMaximum(9999999999.9);
        ui->thanPriceValue->setMinimum(-9999999999.9);
    }

    fixSize();
}

void AddRuleDialog::on_variableBPercentButton_clicked()
{
    auto* percentPicker = new PercentPicker(ui->variableBExact, 100.0);
    QPoint execPos = ui->whenValueGroupBox->mapToGlobal(ui->variableBPercentButton->geometry().center());
    execPos.setX(execPos.x() - percentPicker->width() / 2);
    execPos.setY(execPos.y() - percentPicker->width());
    percentPicker->exec(execPos);
}

void AddRuleDialog::on_thanAmountPercentButton_clicked()
{
    auto* percentPicker = new PercentPicker(ui->thanAmount, 100.0);
    QPoint execPos = ui->actionGroupBox->mapToGlobal(ui->thanAmountPercentButton->geometry().center());
    execPos.setX(execPos.x() - percentPicker->width() / 2);
    execPos.setY(execPos.y() - percentPicker->width());
    percentPicker->exec(execPos);
}

void AddRuleDialog::on_thanPricePercentButton_clicked()
{
    auto* percentPicker = new PercentPicker(ui->thanPriceValue, 100.0);
    QPoint execPos = ui->actionGroupBox->mapToGlobal(ui->thanPricePercentButton->geometry().center());
    execPos.setX(execPos.x() - percentPicker->width() / 2);
    execPos.setY(execPos.y() - percentPicker->width());
    percentPicker->exec(execPos);
}

void AddRuleDialog::on_variableBFee_currentIndexChanged(int /*unused*/)
{
    fixSize();
}

void AddRuleDialog::on_thanAmountFee_currentIndexChanged(int /*unused*/)
{
    fixSize();
}

void AddRuleDialog::on_thanPriceFee_currentIndexChanged(int /*unused*/)
{
    fixSize();
}

void AddRuleDialog::on_thanText_textChanged(const QString& /*unused*/)
{
    reCacheCode();
}

void AddRuleDialog::on_codePreview_toggled(bool checked)
{
    if (checked)
    {
        ui->addRuleGroupBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
        setMaximumHeight(1280);
        resize(width(), 640);
        fixSize();
    }
    else
    {
        ui->addRuleGroupBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        setMaximumHeight(600);
        fixSize();
    }
}

QString AddRuleDialog::comboData(QComboBox* list, int row)
{
    if (row < 0 || row >= list->count())
        return QLatin1String("");

    return list->itemData(row).toString();
}

QString AddRuleDialog::comboCurrentData(QComboBox* list)
{
    return comboData(list, list->currentIndex());
}

void AddRuleDialog::on_buttonAddRule_clicked()
{
    RuleHolder holder = getRuleHolder();

    if (!holder.isValid())
    {
        QMessageBox::warning(this, windowTitle(), julyTr("INVALID_RULE_VALUES", "Values you entered is invalid"));
        saveClicked = false;
        return;
    }

    if (ruleIsEnabled)
    {
        QString code = RuleScriptParser::holderToScript(holder, true);
        ScriptObject tempScript("Test");
        tempScript.executeScript(code, true);
        tempScript.stopScript();

        ruleIsEnabled = tempScript.testResult != 1;
    }

    if (!baseValues.currentExchange_->multiCurrencyTradeSupport)
    {
        QString currentSymbol = baseValues.currentPair.symbolSecond();

        if (comboCurrentData(ui->valueASymbol) != currentSymbol || comboCurrentData(ui->valueBSymbol) != currentSymbol ||
            comboCurrentData(ui->thanSymbol) != currentSymbol)
        {
            QMessageBox::warning(
                this,
                windowTitle(),
                julyTr(
                    "RULE_MULTITRADE_NOTSUPPORTED",
                    "Warning. Multi currency trading is not supported yet.\nRule will works only with the same currency pair as current.\nSet up all symbols as current main currency."));
            saveClicked = false;
            return;
        }
    }

    accept();
}

void AddRuleDialog::on_buttonSaveRule_clicked()
{
    saveClicked = true;
    on_buttonAddRule_clicked();
}

void AddRuleDialog::on_fillFromBuyPanel_clicked()
{
    QString tempSymbol = baseValues.currentPair.symbolSecond();
    setComboIndexByData(ui->thanSymbol, tempSymbol);
    ui->thanPricePercent->setChecked(false);
    ui->thanPriceFee->setCurrentIndex(0);
    ui->thanType->setCurrentIndex(1);
    ui->thanPriceType->setCurrentIndex(ui->thanPriceType->count() - 1);
    ui->thanAmount->setValue(mainWindow.buyTotalBtc->value());
    ui->thanPriceValue->setValue(mainWindow.buyPricePerCoin->value());
}

void AddRuleDialog::on_fillFromSellPanel_clicked()
{
    QString tempSymbol = baseValues.currentPair.symbolSecond();
    setComboIndexByData(ui->thanSymbol, tempSymbol);
    ui->thanPricePercent->setChecked(false);
    ui->thanPriceFee->setCurrentIndex(0);
    ui->thanType->setCurrentIndex(0);
    ui->thanPriceType->setCurrentIndex(ui->thanPriceType->count() - 1);
    ui->thanAmount->setValue(mainWindow.sellTotalBtc->value());
    ui->thanPriceValue->setValue(mainWindow.sellPricePerCoin->value());
}

void AddRuleDialog::on_valueBSymbol_currentIndexChanged(int index)
{
    QString symbol = ui->valueBSymbol->itemData(index).toString();

    CurrencyPairItem pairItem;
    pairItem = baseValues.currencyPairMap.value(symbol, pairItem);

    if (pairItem.symbolSecond().isEmpty())
        return;

    for (int n = 0; n < ui->variableB->count(); n++)
    {
        QString scriptName = ui->variableB->itemData(n).toString();

        if (!scriptName.startsWith("Balance"))
            continue;

        QString translatedName = julyTranslator.translateString("INDICATOR_BALANCE", scriptName);

        if (scriptName.startsWith("BalanceA"))
            translatedName = translatedName.arg(pairItem.currAStr);
        else if (scriptName.startsWith("BalanceB"))
            translatedName = translatedName.arg(pairItem.currBStr);

        ui->variableB->setItemText(n, translatedName);
    }
}

void AddRuleDialog::on_thanSymbol_currentIndexChanged(int index)
{
    QString symbol = ui->thanSymbol->itemData(index).toString();

    CurrencyPairItem pairItem;
    pairItem = baseValues.currencyPairMap.value(symbol, pairItem);

    if (pairItem.symbolSecond().isEmpty())
        return;

    ui->thanType->setItemText(0, julyTr("RULE_THAN_SELL", "Sell %1").arg(pairItem.currAStr));
    ui->thanType->setItemText(1, julyTr("RULE_THAN_BUY", "Buy %1").arg(pairItem.currAStr));
    ui->thanType->setItemText(2, julyTr("RULE_THAN_RECEIVE", "Receive %1").arg(pairItem.currBStr));
    ui->thanType->setItemText(3, julyTr("RULE_THAN_SPEND", "Spend %1").arg(pairItem.currBStr));
}

void AddRuleDialog::on_valueASymbol_currentIndexChanged(int index)
{
    QString symbol = comboData(ui->valueASymbol, index);

    CurrencyPairItem pairItem;
    pairItem = baseValues.currencyPairMap.value(symbol, pairItem);

    if (pairItem.symbolSecond().isEmpty())
        return;

    for (int n = 0; n < ui->variableA->count(); n++)
    {
        QString scriptName = comboData(ui->variableA, n);

        if (!scriptName.startsWith("Balance"))
            continue;

        QString translatedName = julyTranslator.translateString("INDICATOR_BALANCE", scriptName);

        if (scriptName.startsWith("BalanceA"))
            translatedName = translatedName.arg(pairItem.currAStr);
        else if (scriptName.startsWith("BalanceB"))
            translatedName = translatedName.arg(pairItem.currBStr);

        ui->variableA->setItemText(n, translatedName);
    }
}

void AddRuleDialog::on_sayCode_currentIndexChanged(int index)
{
    if (!ui->sayCode->isVisible())
        return;

    if (index == 0)
        ui->thanText->setText("");
    else
        ui->thanText->setText(ui->sayCode->itemText(index));
}
