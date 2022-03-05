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

#include "scriptwidget.h"
#include "addruledialog.h"
#include "main.h"
#include "scriptobject.h"
#include "timesync.h"
#include "ui_scriptwidget.h"
#include "utils/utils.h"
#include <QAction>
#include <QDesktopServices>
#include <QFileDialog>
#include <QMessageBox>
#include <QTabWidget>
#include <QToolButton>
#include <QUrl>

ScriptWidget::ScriptWidget(const QString& gName, const QString& _fileName, const QString& fileCopyFrom) :
    QWidget(), ui(new Ui::ScriptWidget)
{
    fileName = _fileName;
    ui->setupUi(this);
    ui->saveFon->setVisible(false);
    ui->clearFon->setVisible(false);
    ui->scriptTabWidget->setCurrentIndex(0);
    setAttribute(Qt::WA_DeleteOnClose, true);
    scriptName = gName;

    if (fileName.isEmpty() || !QFile::exists(fileName))
    {
        if (!QFile::exists(baseValues.scriptFolder))
            QDir().mkpath(QFileInfo(baseValues.scriptFolder).dir().path());

        int curNum = 1;

        do
        {
            fileName = baseValues.scriptFolder + "Script_" + QString::number(curNum++) + ".JLS";
        } while (QFile::exists(fileName));
    }

    setProperty("FileName", fileName);
    setProperty("GroupType", "Script");

    if (!fileCopyFrom.isEmpty() && QFile::exists(fileCopyFrom))
    {
        if (QFile::exists(fileName))
            QFile::remove(fileName);

        QFile::copy(fileCopyFrom, fileName);
    }

    if (fileName.isEmpty() || !QFile::exists(fileName))
    {
        QSettings saveScript(fileName, QSettings::IniFormat);
        saveScript.beginGroup("JLScript");
        saveScript.setValue("Version", baseValues.jlScriptVersion);
        saveScript.setValue("Encrypted", false);
        saveScript.sync();
    }
    else
    {
        QSettings loadScript(fileName, QSettings::IniFormat);
        loadScript.beginGroup("JLScript");
        ui->limitRowsValue->setValue(loadScript.value("LogRowsCount", 20).toInt());
        ui->notes->setPlainText(loadScript.value("Notes", "").toString());
        ui->sourceCode->setPlainText(loadScript.value("Data", ui->sourceCode->toPlainText()).toString());
        loadScript.sync();
    }

    scriptObject.reset(new ScriptObject(scriptName));
    connect(scriptObject.get(), &ScriptObject::writeLog, this, &ScriptWidget::writeConsole);
    connect(scriptObject.get(), &ScriptObject::logClearSignal, this, &ScriptWidget::clearLog);
    connect(scriptObject.get(), &ScriptObject::runningChanged, this, &ScriptWidget::setRunningUi);
    connect(scriptObject.get(), &ScriptObject::errorHappend, this, &ScriptWidget::errorHappend);
    connect(this, &ScriptWidget::setRuleTabRunning, baseValues_->mainWindow_, &QtBitcoinTrader::setRuleTabRunning);

    ui->insertEvents->setMenu(&insertEventMenu);
    ui->insertFunction->setMenu(&insertFunctionMenu);
    ui->insertCommand->setMenu(&insertCommandMenu);

    for (const QString& value : scriptObject->indicatorList)
        insertEventMenu.addAction(NewEventsAction(value));

    for (int n = 0; n < scriptObject->commandsList.size(); n++)
        insertCommandMenu.addAction(NewFunctionsAction(scriptObject->commandsList.at(n), scriptObject->argumentsList.at(n)));

    for (int n = 0; n < scriptObject->functionsList.size(); n++)
        insertFunctionMenu.addAction(NewFunctionsAction(scriptObject->functionsList.at(n)));

    menuButtons << ui->insertEvents << ui->insertFunction << ui->insertCommand;

    for (QToolButton* button : menuButtons)
        button->installEventFilter(this);

    setWindowTitle(scriptName);
    setRunningUi(false);

    on_limitRowsValue_valueChanged(ui->limitRowsValue->value());

    // QStringList eventList;
    // for(QAction *currentAction: insertEventMenu.actions())
    //   eventList<<currentAction->text();

    // QStringList functionList;
    // for(QAction *currentAction: insertFunctionMenu.actions())
    //   functionList<<currentAction->text();

    // QStringList commandList;
    // for(QAction *currentAction: insertCommandMenu.actions())
    //   commandList<<currentAction->text();

    // qDebug()<<"Events:\n"<<eventList.join("\n")<<"\nFunctions:\n"<<functionList.join("\n")<<"\nCommands:\n"<<commandList.join("\n");

    languageChanged();
    currencyChanged();

    QSettings iniSettings(baseValues.iniFileName, QSettings::IniFormat, this);

    if (iniSettings.value("UI/OptimizeInterface", false).toBool())
        recursiveUpdateLayouts(this);
}

ScriptWidget::~ScriptWidget()
{
    if (!fileName.isEmpty())
        saveScriptToFile(fileName);
}

void ScriptWidget::languageChanged()
{
    julyTranslator.translateUi(this);

    ui->scriptTabWidget->setTabText(0, " " + julyTr("SOURCE_CODE", "Source code"));
    ui->scriptTabWidget->setTabText(1, julyTr("CONSOLE_OUT", "Console output"));
    ui->scriptTabWidget->setTabText(2, julyTr("SCRIPT_NOTES", "Notes"));

    ui->insertEvents->setText(" " + julyTr("SCRIPT_EVENT", "Event") + " ");
    ui->insertFunction->setText(" " + julyTr("SCRIPT_FUNCTION", "Function") + " ");
    ui->insertCommand->setText(" " + julyTr("SCRIPT_COMMAND", "Command") + " ");

    ui->stateLabel->setText(isRunning() ? julyTr("SCRIPT_RUNNING", "Running") : julyTr("SCRIPT_STOPPED", "Stopped"));
    ui->buttonStartStop->setText(isRunning() ? julyTr("SCRIPT_STOP", "Stop") : julyTr("SCRIPT_START", "Start"));

    //    for(QAction *action: insertEventMenu.actions())
    //        action->setToolTip(julyTr("SCRIPT_ACTION_EVENT_"+action->property("TranslationName").toString(),action->text()));

    //    for(QAction *action: insertCommandMenu.actions())
    //        action->setToolTip(julyTr("SCRIPT_ACTION_"+action->property("TranslationName").toString(),action->text()));

    //    for(QAction *action: insertFunctionMenu.actions())
    //        action->setToolTip(julyTr("SCRIPT_ACTION_"+action->property("TranslationName").toString(),action->text()));
}

void ScriptWidget::currencyChanged()
{
    for (QAction* action : insertEventMenu.actions())
    {
        QString trString = action->property("TranslationName").toString();

        if (trString.contains("_BALANCEA_"))
        {
            action->setText("trader.on(\"Balance\",\"" + baseValues.currentPair.currAStr + "\").changed()");
            action->setProperty("ScriptName", action->text());
        }
        else if (trString.contains("_BALANCEB_"))
        {
            action->setText("trader.on(\"Balance\",\"" + baseValues.currentPair.currBStr + "\").changed()");
            action->setProperty("ScriptName", action->text());
        }
    }

    for (QAction* action : insertFunctionMenu.actions())
    {
        QString trString = action->property("TranslationName").toString();

        if (trString.contains("_BALANCEA"))
            action->setText("trader.get(\"Balance\",\"" + baseValues.currentPair.currAStr + "\")");
        else if (trString.contains("_BALANCEB"))
            action->setText("trader.get(\"Balance\",\"" + baseValues.currentPair.currBStr + "\")");
    }
}

void ScriptWidget::insertFilePath(const QString& description, const QString& mask)
{
    QString lastRulesDir = mainWindow.iniSettings->value("UI/LastRulesPath", baseValues.desktopLocation).toString();

    if (!QFile::exists(lastRulesDir))
        lastRulesDir = baseValues.desktopLocation;

    QString fileName = QFileDialog::getOpenFileName(this, description, lastRulesDir, "(" + mask + ")");

    if (fileName.isEmpty())
        return;

    mainWindow.iniSettings->setValue("UI/LastRulesPath", QFileInfo(fileName).dir().path());
    mainWindow.iniSettings->sync();

#ifdef Q_OS_WIN
    fileName.replace("/", "\\");
#endif

    ui->sourceCode->textCursor().removeSelectedText();
    ui->sourceCode->insertPlainText("\"" + fileName + "\"");
}

bool ScriptWidget::isRunning()
{
    return scriptObject->isRunning();
}

QAction* ScriptWidget::NewFunctionsAction(const QString& name, QString params)
{
    auto* action = new QAction(this);

    if (!params.isEmpty() || name.at(name.size() - 1) != QLatin1Char(')'))
        params = "(" + params + ")";

    action->setText(name + params);
    action->setToolTip(action->text());
    action->setProperty("TranslationName",
                        "SCRIPT_ACTION_" + name.toUpper().replace("\").", "_").replace(".", "_").replace("(\"", "_").replace("\")", ""));
    action->setProperty("ScriptName", name);
    connect(action, &QAction::triggered, this, &ScriptWidget::addFunctionClicked);
    return action;
}

QAction* ScriptWidget::NewEventsAction(const QString& name)
{
    auto* action = new QAction(this);
    action->setText(name + "()");
    action->setToolTip(action->text());
    action->setProperty("TranslationName",
                        "SCRIPT_ACTION_EVENT_" +
                            name.toUpper().replace("\").", "_").replace(".", "_").replace("(\"", "_").replace("\")", ""));
    action->setProperty("ScriptName", name);
    connect(action, &QAction::triggered, this, &ScriptWidget::addEventsClicked);
    return action;
}

void ScriptWidget::addFunctionClicked()
{
    auto* action = qobject_cast<QAction*>(sender());

    if (action == nullptr)
        return;

    QString command = action->property("ScriptName").toString();

    if (command.length() == 0)
        return;

    if (command == QLatin1String("trader.get(\"BalanceA\")"))
        command = QString("trader.get(\"Balance\",\"%1\")").arg(baseValues.currentPair.currAStr);
    else if (command == QLatin1String("trader.get(\"BalanceB\")"))
        command = QString("trader.get(\"Balance\",\"%1\")").arg(baseValues.currentPair.currBStr);

    ui->sourceCode->textCursor().removeSelectedText();

    QString plainText = ui->sourceCode->toPlainText();

    int cursorPos = ui->sourceCode->textCursor().anchor();

    bool validPlaceToInsert = cursorPos > 0 && cursorPos < plainText.length();

    if (!validPlaceToInsert)
    {
        int indexOfQuote = plainText.indexOf(QLatin1Char('{'), cursorPos);

        if (indexOfQuote < 0)
            indexOfQuote = plainText.length();
        else
            command.prepend("\n");

        if (indexOfQuote > -1)
        {
            QTextCursor textCursor = ui->sourceCode->textCursor();
            textCursor.setPosition(indexOfQuote);
            ui->sourceCode->setTextCursor(textCursor);
        }
    }

    if (command.size() && command.right(1) != QLatin1String(")"))
    {
        if (command.endsWith("buy") || command.endsWith("sell") || command.endsWith("log"))
            command.append("(");
        else
            command.append("()");
    }

    if (command.endsWith("startApp()"))
    {
        command.append(";");
        ui->sourceCode->insertPlainText(command);
        QTextCursor textCursor = ui->sourceCode->textCursor();
        textCursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, 2);
        ui->sourceCode->setTextCursor(textCursor);
        insertFilePath(julyTr("OPEN_ANY_FILE", "Open any file"), "*");
    }
    else if (command.contains("playSound"))
    {
        command.append(";");
        ui->sourceCode->insertPlainText(command);
        QTextCursor textCursor = ui->sourceCode->textCursor();
        textCursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, 2);
        ui->sourceCode->setTextCursor(textCursor);
        insertFilePath(julyTr("PLAY_SOUND_WAV", "Open WAV file"), "*.wav");
    }
    else
    {
        if (!command.startsWith(QLatin1String("trader.get")))
            command = action->text() + QLatin1String(";");

        ui->sourceCode->insertPlainText(command);
    }
}

void ScriptWidget::replaceScript(const QString& code)
{
    setRunning(false);
    ui->sourceCode->setPlainText(code);
}

void ScriptWidget::addEventsClicked()
{
    auto* action = qobject_cast<QAction*>(sender());

    if (action == nullptr)
        return;

    QString command = action->property("ScriptName").toString();

    if (command.length() == 0)
        return;

    if (command.at(command.size() - 1) != QLatin1Char(')'))
        command.append("()");

    ui->sourceCode->textCursor().removeSelectedText();

    QString plainText = ui->sourceCode->toPlainText();
    int existingEvent = plainText.indexOf(command);

    if (existingEvent > -1)
    {
        QTextCursor textCursor = ui->sourceCode->textCursor();
        textCursor.setPosition(existingEvent);
        ui->sourceCode->setTextCursor(textCursor);
        ui->sourceCode->setFocus();
        return;
    }

    command.append("\n{\n\n}\n");

    int cursorPos = ui->sourceCode->textCursor().anchor();

    if (cursorPos > 0)
        command.prepend("\n");

    if (cursorPos > 0 && cursorPos < plainText.length() - 1)
    {
        bool validPlaceToInsert = cursorPos > 0 && cursorPos < plainText.length() && plainText.at(cursorPos - 1) == QLatin1Char('\n') &&
                                  plainText.at(cursorPos) == QLatin1Char('\n');

        int countLeft = 0;
        int countRight = 0;

        for (int n = 0; n < qMin(cursorPos, command.length()); n++)
        {
            if (plainText.at(n) == QLatin1Char('{'))
                countLeft++;
            else if (plainText.at(n) == QLatin1Char('}'))
                countRight++;
        }

        if (countLeft != countRight || !validPlaceToInsert)
        {
            ui->sourceCode->moveCursor(QTextCursor::End);
        }
    }

    ui->sourceCode->insertPlainText(command);

    QTextCursor textCursor = ui->sourceCode->textCursor();
    textCursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, 3);
    ui->sourceCode->setTextCursor(textCursor);

    ui->sourceCode->setFocus();
}

void ScriptWidget::clearLog()
{
    ui->consoleOutput->clear();
}

bool ScriptWidget::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::Enter)
    {
        auto* enteredToolButton = qobject_cast<QToolButton*>(obj);

        if (enteredToolButton)
            for (QToolButton* button : menuButtons)
            {
                if (button == enteredToolButton)
                    continue;

                if (button->menu()->isVisible())
                {
                    button->menu()->hide();
                    enteredToolButton->click();
                    break;
                }
            }
    }

    return QObject::eventFilter(obj, event);
}

void ScriptWidget::setRunning(bool on)
{
    if (on == scriptObject->isRunning())
        return;

    if (!on)
        scriptObject->stopScript();
    else
        executeScript(false);
}

void ScriptWidget::on_buttonStartStop_clicked()
{
    if (scriptObject->stopScript())
        return;

    ui->buttonStartStop->setEnabled(false);

    if (!executeScript(true))
    {
        ui->buttonStartStop->setEnabled(true);
        return;
    }

    executeScript(false);
}

void ScriptWidget::setRunningUi(bool on)
{
    if (on)
        ui->sourceCode->textCursor().clearSelection();

    ui->insertEvents->setEnabled(!on);
    ui->insertFunction->setEnabled(!on);
    ui->insertCommand->setEnabled(!on);
    ui->stateLabel->setStyleSheet(on ? "color: green" : "color: red");
    ui->stateLabel->setText(on ? julyTr("SCRIPT_RUNNING", "Running") : julyTr("SCRIPT_STOPPED", "Stopped"));
    ui->buttonStartStop->setText(on ? julyTr("SCRIPT_STOP", "Stop") : julyTr("SCRIPT_START", "Start"));
    ui->sourceCode->setReadOnly(on);
    ui->buttonClear->setEnabled(!on);
    ui->validateButton->setEnabled(!on);

    ui->buttonStartStop->setEnabled(true);
    emit setRuleTabRunning(scriptName, on);
}

QString ScriptWidget::getFilePath()
{
    return fileName;
}

bool ScriptWidget::removeGroup()
{
    bool removed = true;

    if (!fileName.isEmpty())
    {
        QFile::remove(fileName);
        removed = !QFile::exists(fileName);
    }

    fileName.clear();
    return removed;
}

bool ScriptWidget::executeScript(bool testMode)
{
    QString script = ui->sourceCode->toPlainText();
    ui->statusLabel->setText("");

    bool success = scriptObject->executeScript(script, testMode);

    if (success)
    {
        saveScriptToFile(fileName);
    }

    if (!testMode && !ui->buttonStartStop->isEnabled())
        ui->buttonStartStop->setEnabled(true);

    return success;
}

void ScriptWidget::on_validateButton_clicked()
{
    if (executeScript(true))
        ui->statusLabel->setText("OK");

    saveScriptToFile(fileName);
}

void ScriptWidget::noteChanged()
{
    ui->saveFon->setVisible(true);
}

void ScriptWidget::writeConsole(QString text)
{
    text.replace("\\n", "<br>");
    text.replace("\\t", "    ");
    text.prepend(QDateTime::fromSecsSinceEpoch(TimeSync::getTimeT()).time().toString(baseValues.timeFormat) + "> ");
    ui->statusLabel->setText(text);
    ui->consoleOutput->appendHtml(text);
}

void ScriptWidget::on_buttonClear_clicked()
{
    if (ui->scriptTabWidget->currentIndex() == 0)
    {
        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setWindowTitle("Qt Bitcoin Trader");
        msgBox.setText(julyTr("MESSAGE_CONFIRM_CLEAR_SCRIPT_TEXT", "Do you really want to clear the script?"));

        auto buttonYes = msgBox.addButton(julyTr("YES", "Yes"), QMessageBox::YesRole);
        msgBox.addButton(julyTr("NO", "No"), QMessageBox::NoRole);
        msgBox.exec();
        if (msgBox.clickedButton() == buttonYes)
            ui->sourceCode->clear();
    }
    else
        ui->consoleOutput->clear();
}

void ScriptWidget::on_limitRowsValue_valueChanged(int val)
{
    ui->consoleOutput->setMaximumBlockCount(val);
    QSettings(fileName, QSettings::IniFormat).setValue("JLScript/LogRowsCount", val);
}

void ScriptWidget::errorHappend(int lineNumber, const QString& errorText)
{
    if (lineNumber > -1)
    {
        QTextCursor tempCursor = ui->sourceCode->textCursor();
        tempCursor.setPosition(0, QTextCursor::MoveAnchor);
        tempCursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, lineNumber);
        ui->sourceCode->setTextCursor(tempCursor);
        ui->sourceCode->setFocus();
    }
    else
    {
        ui->scriptTabWidget->setCurrentIndex(1);
    }

    writeConsole(errorText);
}

void ScriptWidget::on_scriptSave_clicked()
{
    QString lastDir = mainWindow.iniSettings->value("UI/LastRulesPath", baseValues.desktopLocation).toString();

    if (!QFile::exists(lastDir))
        lastDir = baseValues.desktopLocation;

    QString fileName = QFileDialog::getSaveFileName(
        this,
        julyTr("SAVE_SCRIPT", "Save Script"),
        lastDir + "/" + QString(scriptName).replace("/", "_").replace("\\", "").replace(":", "").replace("?", "") + ".JLS",
        "JL Script (*.JLS)");

    if (fileName.isEmpty())
        return;

    mainWindow.iniSettings->setValue("UI/LastRulesPath", QFileInfo(fileName).dir().path());
    mainWindow.iniSettings->sync();

    if (QFile::exists(fileName))
        QFile::remove(fileName);

    if (!saveScriptToFile(fileName))
        QMessageBox::warning(this, windowTitle(), julyTr("CANT_SAVE_SCRIPT", "Can't save script to \"%1\"").arg(fileName));
}

void ScriptWidget::on_buttonSave_clicked()
{
    saveScriptToFile(fileName);
}

bool ScriptWidget::saveScriptToFile(QString fileSave)
{
    if (fileSave.isEmpty())
        fileSave = fileName;

    ui->saveFon->setVisible(false);

    if (QFile::exists(fileSave))
        QFile::remove(fileSave);

    QSettings saveScript(fileSave, QSettings::IniFormat);
    saveScript.beginGroup("JLScript");
    saveScript.setValue("Version", baseValues.jlScriptVersion);
    saveScript.setValue("Name", windowTitle());
    saveScript.setValue("Notes", ui->notes->toPlainText());
    saveScript.setValue("Encrypted", false);
    saveScript.setValue("LogRowsCount", ui->limitRowsValue->value());
    saveScript.setValue("Data", ui->sourceCode->toPlainText());
    saveScript.sync();
    return QSettings(fileSave, QSettings::IniFormat).value("JLScript/Name", "").toString() != "";
}

void ScriptWidget::on_consoleOutput_textChanged()
{
    bool haveLog = !ui->consoleOutput->toPlainText().isEmpty();
    ui->clearFon->setVisible(haveLog);
}

void ScriptWidget::on_ruleAddButton_clicked()
{
    AddRuleDialog ruleWindow(scriptName, this);
    ruleWindow.setWindowFlags(mainWindow.windowFlags());

    if (ruleWindow.exec() == QDialog::Rejected)
        return;

    RuleHolder holder = ruleWindow.getRuleHolder();

    if (!holder.isValid())
        return;

    mainWindow.addRuleByHolder(holder, ruleWindow.isRuleEnabled(), ruleWindow.getGroupName());
    writeConsole("Script replaced by Rule: \"" + holder.description + "\"");
}
