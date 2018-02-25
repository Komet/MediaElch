#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <QNetworkAccessManager>
#include <QObject>
#include <QXmlStreamReader>

#include "plugins/PluginInterface.h"

class PluginManager : public QObject
{
    Q_OBJECT
public:
    struct PluginFile {
        QString fileName;
        QString sha1;
        bool downloaded;
    };

    struct Plugin {
        QString name;
        QString identifier;
        QString version;
        QString installedVersion;
        QString minimumVersion;
        QList<PluginManager::PluginFile> files;
        bool installed;
        bool updateAvailable;
        PluginInterface *plugin;
        QString localFileName;
    };

    explicit PluginManager(QObject *parent = nullptr);
    static PluginManager *instance(QObject *parent = nullptr);
    QList<PluginManager::Plugin> plugins();

public slots:
    void loadPlugins();
    void loadSettings();
    void saveSettings();
    void installPlugin(PluginManager::Plugin plugin);
    void uninstallPlugin(PluginManager::Plugin plugin);
    void updatePlugin(PluginManager::Plugin plugin);

signals:
    void sigAddPlugin(PluginInterface*);
    void sigRemovePlugin(PluginInterface*);
    void sigPluginListUpdated(QList<PluginManager::Plugin>);
    void sigLicenseInvalid(PluginManager::Plugin);
    void sigPluginInstalled(PluginManager::Plugin);
    void sigPluginInstallFailure(PluginManager::Plugin);
    void sigPluginUpdated(PluginManager::Plugin);

private slots:
    void onPluginListDownloaded();
    void onPluginDownloaded();

private:
    QList<PluginManager::Plugin> m_plugins;
    QNetworkAccessManager m_qnam;
    QString m_os;

    void downloadPluginList();
    void parsePluginData(QXmlStreamReader &xml);
    bool loadPlugin(const QString &fileName);
};

#endif // PLUGINMANAGER_H
