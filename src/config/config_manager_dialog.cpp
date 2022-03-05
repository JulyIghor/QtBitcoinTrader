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

#include "config_manager_dialog.h"
#include "config_manager.h"
#include "ui_config_manager_dialog.h"

ConfigManagerDialog::ConfigManagerDialog(QWidget* parent) : QDialog(parent), ui(new Ui::ConfigManagerDialog), nameChanging(false)
{
    ui->setupUi(this);

    ui->editName->setFocus();

    connect(ui->btnSave, &QPushButton::clicked, this, &ConfigManagerDialog::onBtnConfigSave);
    connect(ui->btnLoad, &QPushButton::clicked, this, &ConfigManagerDialog::onBtnConfigLoad);
    connect(ui->btnRemove, &QPushButton::clicked, this, &ConfigManagerDialog::onBtnConfigRemove);
    connect(ui->btnClose, &QPushButton::clicked, this, &ConfigManagerDialog::close);
    connect(ui->editName, &QLineEdit::textChanged, this, &ConfigManagerDialog::onNameTextChanged);
    connect(ui->listNames, &QListWidget::currentTextChanged, this, &ConfigManagerDialog::onNameListCurrentTextChanged);
    connect(ui->listNames, &QListWidget::itemClicked, this, &ConfigManagerDialog::onNameListItemClicked);
    connect(ui->listNames, &QListWidget::itemDoubleClicked, this, &ConfigManagerDialog::onNameListItemDoubleClicked);
    connect(::config, &ConfigManager::onChanged, this, &ConfigManagerDialog::onConfigChanged);

    ui->listNames->clear();
    ui->listNames->addItems(::config->getConfigNames());
    emit ui->listNames->currentTextChanged("");
}

ConfigManagerDialog::~ConfigManagerDialog()
{
    delete ui;
}

void ConfigManagerDialog::onBtnConfigSave()
{
    QString name = ui->editName->text();

    if (!::config->defaultNamesTr.contains(name))
    {
        ::config->save(name);
        close();
    }
}

void ConfigManagerDialog::onBtnConfigLoad()
{
    QString name = ui->editName->text();
    ::config->load(name);
    close();
}

void ConfigManagerDialog::onBtnConfigRemove()
{
    QString name = ui->editName->text();
    ::config->remove(name);
}

void ConfigManagerDialog::onConfigChanged()
{
    QString name = ui->editName->text();
    ui->listNames->clear();
    ui->listNames->addItems(::config->getConfigNames());
    ui->editName->setText(name);
    selectNameInList(name);
}

void ConfigManagerDialog::onNameTextChanged(const QString& text)
{
    ui->btnSave->setEnabled(!text.trimmed().isEmpty());
    nameChanging = true;
    selectNameInList(text);

    if (config->defaultNamesTr.contains(text))
    {
        ui->btnSave->setEnabled(false);
    }

    nameChanging = false;
}

void ConfigManagerDialog::onNameListCurrentTextChanged(const QString& text)
{
    if (!nameChanging)
    {
        ui->editName->setText(text);
    }

    if (config->defaultNamesTr.contains(text))
    {
        ui->btnLoad->setEnabled(true);
        ui->btnSave->setEnabled(false);
        ui->btnRemove->setEnabled(false);
    }
    else
    {
        bool nameExists = ::config->getConfigNames().contains(text);
        ui->btnLoad->setEnabled(nameExists);
        ui->btnSave->setEnabled(true);
        ui->btnRemove->setEnabled(nameExists);
    }
}

void ConfigManagerDialog::onNameListItemClicked(QListWidgetItem* item)
{
    // when clicked on single item - currentTextChanged not emited, so do it here
    ui->editName->setText(item->text());
}

void ConfigManagerDialog::onNameListItemDoubleClicked(QListWidgetItem* item)
{
    QString name = item->text();
    ::config->load(name);
    close();
}

void ConfigManagerDialog::selectNameInList(const QString& name)
{
    QList<QListWidgetItem*> items = ui->listNames->findItems(name, Qt::MatchExactly);
    QListWidgetItem* item = items.isEmpty() ? NULL : items.first();
    ui->listNames->setCurrentItem(item);
}
