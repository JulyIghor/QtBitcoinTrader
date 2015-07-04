#include "config_manager_dialog.h"
#include "ui_config_manager_dialog.h"
#include "config_manager.h"


ConfigManagerDialog::ConfigManagerDialog(QWidget* parent) :
    QDialog     (parent),
    ui          (new Ui::ConfigManagerDialog),
    nameChanging    (false)
{
    ui->setupUi(this);

    ui->editName->setFocus();

    connect(ui->btnSave,   &QPushButton::clicked, this, &ConfigManagerDialog::onBtnConfigSave);
    connect(ui->btnLoad,   &QPushButton::clicked, this, &ConfigManagerDialog::onBtnConfigLoad);
    connect(ui->btnRemove, &QPushButton::clicked, this, &ConfigManagerDialog::onBtnConfigRemove);
    connect(ui->btnClose,  &QPushButton::clicked, this, &ConfigManagerDialog::close);
    connect(ui->editName,  &QLineEdit::textChanged,          this, &ConfigManagerDialog::onNameTextChanged);
    connect(ui->listNames, &QListWidget::currentTextChanged, this, &ConfigManagerDialog::onNameListCurrentTextChanged);
    connect(ui->listNames, &QListWidget::itemClicked,        this, &ConfigManagerDialog::onNameListItemClicked);
    connect(ui->listNames, &QListWidget::itemDoubleClicked,  this, &ConfigManagerDialog::onNameListItemDoubleClicked);
    connect(::config,      &ConfigManager::onChanged,        this, &ConfigManagerDialog::onConfigChanged);

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
    if(!::config->defaultNamesTr.contains(name)){
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
    if(config->defaultNamesTr.contains(text)){
        ui->btnSave->setEnabled(false);
    }
    nameChanging = false;
}

void ConfigManagerDialog::onNameListCurrentTextChanged(const QString& text)
{
    if (!nameChanging) {
        ui->editName->setText(text);
    }

    if(config->defaultNamesTr.contains(text)){
        ui->btnLoad->setEnabled(true);
        ui->btnSave->setEnabled(false);
        ui->btnRemove->setEnabled(false);
    }
    else {
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
