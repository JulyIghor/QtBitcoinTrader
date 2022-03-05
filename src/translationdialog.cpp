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

#include "translationdialog.h"
#include "main.h"
#include <QDesktopServices>
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>

TranslationDialog::TranslationDialog(QWidget* parent) : QDialog(parent)
{
    ui.setupUi(this);
    ui.buttonSaveAs->setEnabled(false);
    setWindowFlags(Qt::Window);
    setAttribute(Qt::WA_DeleteOnClose, true);
    // setFixedSize(size());

    julyTranslator.translateUi(this);

    ui.deleteTranslationButton->setEnabled(!julyTranslator.lastFile().startsWith(":/Resource"));

    ui.languageName->setText(julyTr("LANGUAGE_NAME", "Invalid"));
    authorAbout = new TranslationLine;
    ui.authorLayout->addWidget(authorAbout);
    authorAbout->setItemText(julyTr("LANGUAGE_AUTHOR", "Invalid"));

    gridLayout = new QGridLayout;
    fonWidget.setLayout(gridLayout);
    ui.scrollArea->setWidget(&fonWidget);

    JulyTranslator defaultTranslation;
    defaultTranslation.loadFromFile(":/Resources/Language/English.lng");

    fillLayoutByMap(&(julyTranslator.labelMap), "Label_", &(defaultTranslation.labelMap));
    fillLayoutByMap(&(julyTranslator.groupBoxMap), "GroupBox_", &(defaultTranslation.groupBoxMap));
    fillLayoutByMap(&(julyTranslator.checkBoxMap), "CheckBox_", &(defaultTranslation.checkBoxMap));
    fillLayoutByMap(&(julyTranslator.buttonMap), "Button_", &(defaultTranslation.buttonMap));
    fillLayoutByMap(&(julyTranslator.spinBoxMap), "SpinBox_", &(defaultTranslation.spinBoxMap));
    fillLayoutByMap(&(julyTranslator.stringMap), "String_", &(defaultTranslation.stringMap));

    setTabOrder(ui.languageName, authorAbout);

    int currentRow = 0;
    QWidget* lastWidget = authorAbout;

    for (int n = 0; n < lineEdits.size(); n++)
        if (!lineEdits[n]->isChanged())
        {
            TranslationLine* nextWidget = lineEdits[n];
            gridLayout->addWidget(nextWidget, currentRow++, 0);
            setTabOrder(lastWidget, nextWidget);
            lastWidget = nextWidget;
        }

    for (int n = 0; n < lineEdits.size(); n++)
        if (lineEdits[n]->isChanged())
        {
            TranslationLine* nextWidget = lineEdits[n];
            gridLayout->addWidget(nextWidget, currentRow++, 0);
            setTabOrder(lastWidget, nextWidget);
            lastWidget = nextWidget;
        }

    setTabOrder(lastWidget, ui.searchLineEdit);

    resize(800, 640);
    fixLayout();

    if (baseValues.mainWindow_)
        mainWindow.addPopupDialog(1);

    QTimer::singleShot(100, this, SLOT(fixLayout()));
}

TranslationDialog::~TranslationDialog()
{

    delete gridLayout;

    if (baseValues.mainWindow_)
        mainWindow.addPopupDialog(-1);
}

void TranslationDialog::fixLayout()
{
    QSize minSizeHint = fonWidget.minimumSizeHint();

    if (mainWindow.isValidSize(&minSizeHint))
        fonWidget.setFixedHeight(fonWidget.minimumSizeHint().height());
}

void TranslationDialog::resizeEvent(QResizeEvent* event)
{
    event->accept();
    fixLayout();
}

void TranslationDialog::deleteTranslationButton()
{
    QMessageBox msgBox(this);
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setWindowTitle(julyTr("MESSAGE_CONFIRM_DELETE_TRANSLATION", "Please confirm removing file"));
    msgBox.setText(julyTr("MESSAGE_CONFIRM_DELETE_TRANSLATION_TEXT", "Are you sure to delete translation file?"));

    auto buttonYes = msgBox.addButton(julyTr("YES", "Yes"), QMessageBox::YesRole);
    msgBox.addButton(julyTr("NO", "No"), QMessageBox::NoRole);
    msgBox.exec();
    if (msgBox.clickedButton() != buttonYes)
        return;

    if (QFile::exists(julyTranslator.lastFile()))
        QFile::remove(julyTranslator.lastFile());

    ui.deleteTranslationButton->setEnabled(QFile::exists(julyTranslator.lastFile()));
    mainWindow.reloadLanguage();
    close();
}

void TranslationDialog::fillLayoutByMap(QMap<QString, QString>* cMap, const QString& subName, QMap<QString, QString>* dMap)
{
    QStringList currentIdList = dMap->keys();

    for (int n = 0; n < currentIdList.size(); n++)
    {
        if (currentIdList.at(n).startsWith("LANGUAGE_"))
            continue;

        auto* newEdit = new TranslationLine;
        QString defText = dMap->value(currentIdList.at(n), "");
        newEdit->setDefaultText(defText);
        newEdit->setToolTip(defText.replace("<br>", "\n"));
        newEdit->setWindowTitle(subName + currentIdList.at(n));
        newEdit->setItemText(cMap->value(currentIdList.at(n), ""));
        connect(newEdit, &TranslationLine::lineTextChanged, this, &TranslationDialog::lineTextChanged);
        lineEdits << newEdit;
    }
}

void TranslationDialog::lineTextChanged()
{
    fixLayout();
    ui.buttonApply->setEnabled(true);
}

void TranslationDialog::applyButton()
{
    QStringList resultList;

    for (int n = 0; n < lineEdits.size(); n++)
    {
        QString curText = lineEdits.at(n)->getValidText();

        if (curText.isEmpty())
        {
            ui.buttonSaveAs->setEnabled(false);
            QMessageBox::warning(this, windowTitle(), julyTr("LANGUAGE_NOT_APPROVED", "Please fill empty fields"));
            return;
        }

        resultList << lineEdits.at(n)->windowTitle() + "=" + curText;
    }

    resultList << "String_LANGUAGE_NAME=" + ui.languageName->text();
    resultList << "String_LANGUAGE_AUTHOR=" + authorAbout->getValidText();
    QString localeName = locale().name(); // if(localeName.contains("_"))localeName.split("_").first();
    resultList << "String_LANGUAGE_LOCALE=" + localeName;
    QFile writeFile(appDataDir + "/Language/Custom.lng");
    writeFile.open(QIODevice::WriteOnly | QIODevice::Truncate);
    writeFile.write(resultList.join("\r\n").toUtf8());
    writeFile.close();

    if (baseValues.mainWindow_)
        mainWindow.reloadLanguage(appDataDir + "/Language/Custom.lng");

    ui.buttonSaveAs->setEnabled(true);
    ui.buttonApply->setEnabled(false);
    ui.deleteTranslationButton->setEnabled(QFile::exists(appDataDir + "/Language/Custom.lng"));
}

void TranslationDialog::saveAsButton()
{
    applyButton();

    if (!ui.buttonSaveAs->isEnabled())
        return;

    QString fileName = QFileDialog::getSaveFileName(
        this,
        julyTr("SAVE_TRANSLATION", "Save Translation"),
        baseValues.desktopLocation + "/" + ui.languageName->text().replace("/", "_").replace("\\", "").replace(":", "").replace("?", "") +
            ".lng",
        "(*.lng)");

    if (fileName.isEmpty())
        return;

    if (QFile::exists(fileName))
        QFile::remove(fileName);

    QFile::copy(julyTranslator.lastFile(), fileName);
}

void TranslationDialog::searchLang(const QString& filterText)
{
    if (filterText.isEmpty())
    {
        for (int n = 0; n < lineEdits.size(); n++)
            lineEdits[n]->setVisible(true);
    }
    else
    {
        QStringList langFilter = filterText.split(" ");

        for (int n = 0; n < lineEdits.size(); n++)
        {
            QString curText = lineEdits[n]->getValidText();
            bool containsText = true;

            for (int k = 0; k < langFilter.size(); k++)
                if (!curText.contains(langFilter.at(k), Qt::CaseInsensitive))
                    containsText = false;

            lineEdits[n]->setVisible(containsText);
        }
    }

    fonWidget.setFixedHeight(fonWidget.minimumSizeHint().height());
}
