#pragma once

#include <QObject>
#include <QList>
#include <QSettings>


class QSettings;


class ConfigManager :
        public QObject
{
    Q_OBJECT

public:
    explicit ConfigManager(const QString& configFileName, QObject* parent = NULL);
    ~ConfigManager();

    void sync();

    QStringList getConfigNames();
    void save(const QString& name);
    void load(const QString& name);
    void remove(const QString& name);
    void translateDefaultNames();

    QStringList       defaultNames;
    QStringList       defaultNamesTr;
    QList<QByteArray> defaultGeometry;
    QList<QByteArray> defaultState;

Q_SIGNALS:
    void onError(const QString& error);
    void onChanged();

private:
    QSettings                settings;
    QHash<QString,QVariant> settingsCache;
};

extern ConfigManager* config;
