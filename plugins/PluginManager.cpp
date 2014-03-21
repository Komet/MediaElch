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
#include "settings/Settings.h"

PluginManager::PluginManager(QObject *parent) :
    QObject(parent)
{
#if defined(Q_OS_MAC)
    m_os = "osx";
#elif defined(Q_OS_WIN)
    m_os = "win";
#endif
}

PluginManager *PluginManager::instance(QObject *parent)
{
    static PluginManager *m_instance = 0;
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
    QStringList dirs = Settings::pluginDirs();
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
    // @todo: change url
    QNetworkReply *reply = m_qnam.get(QNetworkRequest(QUrl("http://community.local/api/plugins")));
    connect(reply, SIGNAL(finished()), this, SLOT(onPluginListDownloaded()));
}

void PluginManager::onPluginListDownloaded()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(sender());
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
            NotificationBox::instance()->showMessage(tr("Plugin updates available"), 5000);
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
    QString sha1Osx;
    QString sha1Win;

    while (xml.readNextStartElement()) {
        if (xml.name() == "name") {
            name = xml.readElementText();
        } else if (xml.name() == "identifier") {
            identifier = xml.readElementText();
        } else if (xml.name() == "version") {
            version = xml.readElementText();
        } else if (xml.name() == "minimumVersion") {
            minimumVersion = xml.readElementText();
        } else if (xml.name() == "sha1") {
            if (xml.attributes().value("os").toString() == "osx")
                sha1Osx = xml.readElementText();
            else if (xml.attributes().value("os").toString() == "win")
                sha1Win = xml.readElementText();
            else
                xml.readElementText();
        } else {
            xml.skipCurrentElement();
        }
    }

    for (int i=0, n=m_plugins.count() ; i<n ; ++i) {
        if (m_plugins[i].identifier == identifier) {
            m_plugins[i].version = version;
            m_plugins[i].sha1Osx = sha1Osx;
            m_plugins[i].sha1Win = sha1Win;
            m_plugins[i].updateAvailable = (Helper::instance()->compareVersionNumbers(m_plugins[i].installedVersion, version) == 1);
            return;
        }
    }

    PluginManager::Plugin plugin;
    plugin.identifier = identifier;
    plugin.installed = false;
    plugin.installedVersion = "";
    plugin.minimumVersion = minimumVersion;
    plugin.name = name;
    plugin.sha1Osx = sha1Osx;
    plugin.sha1Win = sha1Win;
    plugin.updateAvailable = false;
    plugin.version = version;

    m_plugins.append(plugin);
}

bool PluginManager::loadPlugin(const QString &fileName)
{
    QPluginLoader loader(fileName);
    QObject *plugin = loader.instance();
    if (plugin) {
        PluginInterface *iPlugin = qobject_cast<PluginInterface *>(plugin);
        if (iPlugin) {
            qDebug() << "Loading plugin" << iPlugin->name();

            if (Helper::instance()->compareVersionNumbers(QApplication::applicationVersion(), iPlugin->minimumVersion()) == 1) {
                NotificationBox::instance()->showMessage(tr("Plugin %1 requires at least MediaElch version %2").arg(iPlugin->name()).arg(iPlugin->minimumVersion()), 7000);
                return false;
            }

            iPlugin->init(Manager::instance()->movieModel(), Manager::instance()->tvShowModel(),
                          Manager::instance()->concertModel(), NotificationBox::instance(), Helper::instance());
            iPlugin->loadSettings(Settings::instance()->settings());
            Helper::instance()->applyStyle(iPlugin->widget());

            bool pluginFound = false;
            for (int i=0, n=m_plugins.count() ; i<n ; ++i) {
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

void PluginManager::installPlugin(PluginManager::Plugin plugin, const QString &licenseKey)
{

    QUrlQuery queryData;
    queryData.addQueryItem("keyNumber", licenseKey);
    // @todo: change url
    QUrl url(QString("http://community.local/api/plugin/%1/%2").arg(plugin.identifier).arg(m_os));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = m_qnam.post(request, queryData.toString().toUtf8());
    reply->setProperty("storage", Storage::toVariant(reply, plugin));
    connect(reply, SIGNAL(finished()), this, SLOT(onPluginDownloaded()));
}

void PluginManager::onPluginDownloaded()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(sender());
    if (!reply)
        return;
    reply->deleteLater();
    PluginManager::Plugin plugin = reply->property("storage").value<Storage*>()->plugin();
    bool isUpdate = plugin.updateAvailable;

    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Network Error" << reply->errorString();
        if (reply->error() == QNetworkReply::ContentOperationNotPermittedError)
            emit sigLicenseInvalid(plugin);
        else
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
    if ((m_os == "osx" && sha1 != plugin.sha1Osx) || (m_os == "win" && sha1 != plugin.sha1Win)) {
        emit sigPluginInstallFailure(plugin);
        return;
    }

    if (Settings::pluginDirs().isEmpty() || fileName.isEmpty()) {
        emit sigPluginInstallFailure(plugin);
        return;
    }

    QString pluginFileName = Settings::pluginDirs().first() + "/" + fileName;
    QFile f(pluginFileName);
    f.open(QFile::WriteOnly);
    f.write(ba);
    f.close();

    if (!loadPlugin(pluginFileName)) {
        emit sigPluginInstallFailure(plugin);
        return;
    }

    if (isUpdate)
        emit sigPluginUpdated(plugin);
    else
        emit sigPluginInstalled(plugin);
    emit sigPluginListUpdated(m_plugins);
}

void PluginManager::uninstallPlugin(PluginManager::Plugin plugin)
{
    QFileInfo fi(plugin.localFileName);
    if (!fi.isFile() || !fi.exists())
        return;

    for (int i=0, n=m_plugins.count() ; i<n ; ++i) {
        if (m_plugins[i].localFileName == plugin.localFileName) {
            m_plugins[i].installed = false;
            m_plugins[i].updateAvailable = false;
            m_plugins[i].localFileName = "";
            break;
        }
    }

    emit sigRemovePlugin(plugin.plugin);
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    QFile f(plugin.localFileName);
    f.remove();
    emit sigPluginListUpdated(m_plugins);
}

void PluginManager::updatePlugin(PluginManager::Plugin plugin, const QString &licenseKey)
{
    uninstallPlugin(plugin);
    installPlugin(plugin, licenseKey);
}
