#pragma once

#include <QDialog>


class QListWidgetItem;


namespace Ui {
class ConfigManagerDialog;
}


class ConfigManagerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigManagerDialog(QWidget* parent = NULL);
    ~ConfigManagerDialog();

private:
    Ui::ConfigManagerDialog *ui;

private Q_SLOTS:
    void onBtnConfigSave();
    void onBtnConfigLoad();
    void onBtnConfigRemove();
    void onConfigChanged();
    void onNameTextChanged(const QString& text);
    void onNameListCurrentTextChanged(const QString& text);
    void onNameListItemClicked(QListWidgetItem* item);
    void onNameListItemDoubleClicked(QListWidgetItem* item);

private:
    void selectNameInList(const QString& name);

private:
    bool            nameChanging;
};

