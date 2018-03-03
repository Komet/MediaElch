#include "PluginManager.h"

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QPluginLoader>
#include <QUrlQuery>

#include "data/Storage.h"
#include "globals/Helper.h"
#include "globals/Manager.h"
#include "notifications/NotificationBox.h"
#include "notifications/Notificator.h"
#include "settings/Settings.h"

PluginManager::PluginManager(QObject *parent) : QObject(parent)
{
#if defined(Q_OS_MAC)
    m_os = "osx";
#elif defined(Q_OS_WIN)
    m_os = "win";
#endif
}

PluginManager *PluginManager::instance(QObject *parent)
{
    static PluginManager *m_instance = nullptr;
    if (!m_instance)
        m_instance = new PluginManager(parent);
    return m_instance;
}

QList<PluginManager::Plugin> PluginManager::plugins()
{
    return m_plugins;
}

void PluginManager::loadSettings()
{
    foreach (PluginManager::Plugin plugin, m_plugins) {
        if (plugin.installed && plugin.plugin->hasSettings())
            plugin.plugin->loadSettings(Settings::instance()->settings());
    }
}

void PluginManager::saveSettings()
{
    foreach (PluginManager::Plugin plugin, m_plugins) {
        if (plugin.installed && plugin.plugin->hasSettings())
            plugin.plugin->saveSettings(Settings::instance()->settings());
    }
}

void PluginManager::loadPlugins()
{
    QStringList dirs = Settings::instance()->pluginDirs();
    if (dirs.isEmpty())
        return;

    foreach (const QString &dir, dirs) {
        QDir pluginsDir = QDir(dir);
        foreach (QString fileName, pluginsDir.entryList(QDir::Files))
            loadPlugin(pluginsDir.absoluteFilePath(fileName));
        sigPluginListUpdated(m_plugins);
    }
    downloadPluginList();
}

void PluginManager::downloadPluginList()
{
    QNetworkReply *reply = m_qnam.get(QNetworkRequest(QUrl("http://community.kvibes.de/api/plugins")));
    connect(reply, SIGNAL(finished()), this, SLOT(onPluginListDownloaded()));
}

void PluginManager::onPluginListDownloaded()
{
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    if (!reply)
        return;
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Network Error" << reply->errorString();
        return;
    }

    QString msg = QString::fromUtf8(reply->readAll());
    QXmlStreamReader xml(msg);
    xml.readNextStartElement();
    while (xml.readNextStartElement()) {
        if (xml.name() == "plugin")
            parsePluginData(xml);
        else
            xml.skipCurrentElement();
    }

    foreach (PluginManager::Plugin plugin, m_plugins) {
        if (plugin.updateAvailable) {
            NotificationBox::instance()->showMessage(
                tr("Plugin updates available"), NotificationBox::NotificationInfo, 5000);
            break;
        }
    }

    emit sigPluginListUpdated(m_plugins);
}

void PluginManager::parsePluginData(QXmlStreamReader &xml)
{
    QString name;
    QString identifier;
    QString version;
    QString minimumVersion;
    QList<PluginManager::PluginFile> files;

    while (xml.readNextStartElement()) {
        if (xml.name() == "name") {
            name = xml.readElementText();
        } else if (xml.name() == "identifier") {
            identifier = xml.readElementText();
        } else if (xml.name() == "version") {
            version = xml.readElementText();
        } else if (xml.name() == "minimumVersion") {
            minimumVersion = xml.readElementText();
        } else if (xml.name() == "files" && xml.attributes().value("os").toString() == m_os) {
            while (xml.readNextStartElement()) {
                if (xml.name() == "file") {
                    PluginManager::PluginFile f;
                    f.downloaded = false;
                    f.sha1 = xml.attributes().value("sha1").toString();
                    f.fileName = xml.readElementText();
                    f.downloaded = false;
                    files.append(f);
                } else {
                    xml.skipCurrentElement();
                }
            }
        } else {
            xml.skipCurrentElement();
        }
    }

    for (int i = 0, n = m_plugins.count(); i < n; ++i) {
        if (m_plugins[i].identifier == identifier) {
            m_plugins[i].version = version;
            m_plugins[i].files = files;
            m_plugins[i].updateAvailable =
                (Helper::instance()->compareVersionNumbers(m_plugins[i].installedVersion, version) == 1);
            return;
        }
    }

    PluginManager::Plugin plugin;
    plugin.identifier = identifier;
    plugin.installed = false;
    plugin.installedVersion = "";
    plugin.minimumVersion = minimumVersion;
    plugin.name = name;
    plugin.files = files;
    plugin.updateAvailable = false;
    plugin.version = version;

    m_plugins.append(plugin);
}

bool PluginManager::loadPlugin(const QString &fileName)
{
    qDebug() << "Trying to load" << fileName;
    QPluginLoader loader(fileName);
    QObject *plugin = loader.instance();
    if (plugin) {
        PluginInterface *iPlugin = qobject_cast<PluginInterface *>(plugin);
        if (iPlugin) {
            qDebug() << "Loading plugin" << iPlugin->name();

            if (Helper::instance()->compareVersionNumbers(QApplication::applicationVersion(), iPlugin->minimumVersion())
                == 1) {
                NotificationBox::instance()->showMessage(tr("Plugin %1 requires at least MediaElch version %2")
                                                             .arg(iPlugin->name())
                                                             .arg(iPlugin->minimumVersion()),
                    NotificationBox::NotificationError,
                    7000);
                return false;
            }

            qApp->installTranslator(iPlugin->pluginTranslator(QLocale::system().name()));
            iPlugin->init(Manager::instance()->movieModel(),
                Manager::instance()->tvShowModel(),
                Manager::instance()->concertModel(),
                NotificationBox::instance(),
                Notificator::instance(),
                Helper::instance());
            iPlugin->loadSettings(Settings::instance()->settings());
            Helper::instance()->applyStyle(iPlugin->widget());

            bool pluginFound = false;
            for (int i = 0, n = m_plugins.count(); i < n; ++i) {
                if (m_plugins[i].identifier == iPlugin->identifier()) {
                    m_plugins[i].installed = true;
                    m_plugins[i].installedVersion = iPlugin->version();
                    m_plugins[i].minimumVersion = iPlugin->minimumVersion();
                    m_plugins[i].name = iPlugin->name();
                    m_plugins[i].plugin = iPlugin;
                    m_plugins[i].updateAvailable = false;
                    m_plugins[i].version = iPlugin->version();
                    m_plugins[i].localFileName = fileName;
                    pluginFound = true;
                    break;
                }
            }
            if (!pluginFound) {
                PluginManager::Plugin plugin;
                plugin.identifier = iPlugin->identifier();
                plugin.installed = true;
                plugin.installedVersion = iPlugin->version();
                plugin.minimumVersion = iPlugin->minimumVersion();
                plugin.name = iPlugin->name();
                plugin.plugin = iPlugin;
                plugin.updateAvailable = false;
                plugin.version = iPlugin->version();
                plugin.localFileName = fileName;
                m_plugins.append(plugin);
            }
            emit sigAddPlugin(iPlugin);
            return true;
        }
    }
    return false;
}

void PluginManager::installPlugin(PluginManager::Plugin plugin)
{
    for (int i = 0, n = plugin.files.count(); i < n; ++i) {
        if (plugin.files[i].downloaded == false) {
            QUrl url(QString("http://community.kvibes.de/api/plugin/%1/%2/%3")
                         .arg(plugin.identifier)
                         .arg(m_os)
                         .arg(plugin.files[i].fileName));
            QNetworkRequest request(url);
            request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
            QNetworkReply *reply = m_qnam.get(request);
            reply->setProperty("storage", Storage::toVariant(reply, plugin));
            connect(reply, SIGNAL(finished()), this, SLOT(onPluginDownloaded()));
            return;
        }
    }

    bool loaded = false;
    for (int i = 0, n = plugin.files.count(); i < n; ++i) {
        if (!Settings::instance()->pluginDirs().isEmpty()
            && loadPlugin(Settings::instance()->pluginDirs().first() + "/" + plugin.files[i].fileName) && !loaded)
            loaded = true;
    }

    if (!loaded) {
        qDebug() << "no file loaded";
        emit sigPluginInstallFailure(plugin);
        return;
    }

    if (plugin.updateAvailable)
        emit sigPluginUpdated(plugin);
    else
        emit sigPluginInstalled(plugin);
    emit sigPluginListUpdated(m_plugins);
}

void PluginManager::onPluginDownloaded()
{
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    if (!reply)
        return;
    reply->deleteLater();
    PluginManager::Plugin plugin = reply->property("storage").value<Storage *>()->plugin();

    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Network Error" << reply->errorString();
        emit sigPluginInstallFailure(plugin);
        return;
    }

    QString disposition = reply->rawHeader("Content-disposition");
    QRegExp rx("attachment; filename=\"(.*)\"");
    rx.setMinimal(true);
    if (rx.indexIn(disposition) == -1) {
        emit sigPluginInstallFailure(plugin);
        return;
    }
    QString fileName = rx.cap(1);

    QByteArray ba = reply->readAll();
    QString sha1 = QCryptographicHash::hash(ba, QCryptographicHash::Sha1).toHex();
    for (int i = 0, n = plugin.files.count(); i < n; ++i) {
        if (plugin.files[i].fileName == fileName) {
            plugin.files[i].downloaded = true;
            if (sha1 != plugin.files[i].sha1) {
                qDebug() << plugin.files[i].fileName << "sha1 mismatch" << sha1 << plugin.files[i].sha1;
                emit sigPluginInstallFailure(plugin);
                return;
            }
        }
    }

    if (Settings::instance()->pluginDirs().isEmpty() || fileName.isEmpty()) {
        emit sigPluginInstallFailure(plugin);
        return;
    }

    QString pluginFileName = Settings::instance()->pluginDirs().first() + "/" + fileName;
    QFile f(pluginFileName);
    f.open(QFile::WriteOnly);
    f.write(ba);
    f.close();

    installPlugin(plugin);
}

void PluginManager::uninstallPlugin(PluginManager::Plugin plugin)
{
    QFileInfo fi(plugin.localFileName);
    if (!fi.isFile() || !fi.exists())
        return;

    for (int i = 0, n = m_plugins.count(); i < n; ++i) {
        if (m_plugins[i].localFileName == plugin.localFileName) {
            m_plugins[i].installed = false;
            m_plugins[i].updateAvailable = false;
            m_plugins[i].localFileName = "";
            for (int x = 0, y = m_plugins[i].files.count(); x < y; ++x)
                m_plugins[i].files[x].downloaded = false;
            break;
        }
    }

    emit sigRemovePlugin(plugin.plugin);
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    QFile f(plugin.localFileName);
    f.remove();
    emit sigPluginListUpdated(m_plugins);
}

void PluginManager::updatePlugin(PluginManager::Plugin plugin)
{
    uninstallPlugin(plugin);
    installPlugin(plugin);
}
