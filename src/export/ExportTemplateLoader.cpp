#include "ExportTemplateLoader.h"

#include <QBuffer>
#include <QDebug>
#include <QDir>
#include <QNetworkReply>
#include <QXmlStreamReader>

#ifndef EXTERN_QUAZIP
#include "quazip/quazip/quazip.h"
#include "quazip/quazip/quazipfile.h"
#else
#include "quazip5/quazip.h"
#include "quazip5/quazipfile.h"
#endif

#include "data/Storage.h"
#include "globals/VersionInfo.h"
#include "settings/Settings.h"

static constexpr const char* s_themeListUrl = "http://data.mediaelch.de/export_themes.xml";

ExportTemplateLoader::ExportTemplateLoader(QObject* parent) : QObject(parent)
{
    loadLocalTemplates();
}

ExportTemplateLoader* ExportTemplateLoader::instance(QObject* parent)
{
    static auto* s_instance = new ExportTemplateLoader(parent);
    return s_instance;
}

void ExportTemplateLoader::getRemoteTemplates()
{
    QNetworkReply* reply = m_qnam.get(QNetworkRequest(QUrl(s_themeListUrl)));
    connect(reply, &QNetworkReply::finished, this, &ExportTemplateLoader::onLoadRemoteTemplatesFinished);
}

void ExportTemplateLoader::onLoadRemoteTemplatesFinished()
{
    auto* reply = dynamic_cast<QNetworkReply*>(sender());
    if (reply == nullptr) {
        return;
    }
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Network Error" << reply->errorString();
        emit sigTemplatesLoaded(mergeTemplates(m_localTemplates, m_remoteTemplates));
        return;
    }

    QString msg = QString::fromUtf8(reply->readAll());
    QXmlStreamReader xml(msg);

    if (!xml.readNextStartElement() || xml.name() != "themes") {
        qWarning() << "[ExportTemplateLoader] export_themes.xml does not have a root <themes> element";
        emit sigTemplatesLoaded(mergeTemplates(m_localTemplates, m_remoteTemplates));
        return;
    }

    QVector<ExportTemplate*> templates;
    while (xml.readNextStartElement()) {
        if (xml.name() == "theme") {
            templates << parseTemplate(xml);
        } else {
            qWarning() << "[ExportTemplateLoader] Found unknown XML tag in theme list:" << xml.name();
            xml.skipCurrentElement();
        }
    }

    m_remoteTemplates = std::move(templates);

    emit sigTemplatesLoaded(mergeTemplates(m_localTemplates, m_remoteTemplates));
}

void ExportTemplateLoader::loadLocalTemplates()
{
    QString location = Settings::instance()->exportTemplatesDir();
    QDir storageDir(location);
    if (!storageDir.exists() && !storageDir.mkpath(location)) {
        qCritical() << "[ExportTemplateLoader] Could not create storage location";
        return;
    }

    m_localTemplates.clear();
    for (QFileInfo& info : storageDir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllDirs)) {
        QList<QFileInfo> infos = QDir(info.absoluteFilePath()).entryInfoList({"metadata.xml"});
        if (infos.isEmpty() || infos.count() > 1) {
            continue;
        }

        QFile file(infos.first().absoluteFilePath());
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "[ExportTemplateLoader] File" << infos.first().absoluteFilePath()
                       << "could not be opened for reading";
            continue;
        }

        QString content = QString::fromUtf8(file.readAll());
        file.close();

        QXmlStreamReader xml(content);

        if (!xml.readNextStartElement()) {
            qWarning() << "[ExportTemplateLoader] Couldn't read XML root element of local template";
            continue;
        }

        ExportTemplate* exportTemplate = parseTemplate(xml);
        exportTemplate->setInstalled(true);
        m_localTemplates << exportTemplate;
    }
}

ExportTemplate* ExportTemplateLoader::parseTemplate(QXmlStreamReader& xml)
{
    auto exportTemplate = new ExportTemplate(this);
    exportTemplate->setRemote(true);

    while (xml.readNextStartElement()) {
        if (xml.name() == "name") {
            exportTemplate->setName(xml.readElementText());
        } else if (xml.name() == "identifier") {
            exportTemplate->setIdentifier(xml.readElementText());
        } else if (xml.name() == "description") {
            exportTemplate->addDescription(xml.attributes().value("lang").toString(), xml.readElementText());
        } else if (xml.name() == "author") {
            exportTemplate->setAuthor(xml.readElementText());

        } else if (xml.name() == "engine") {
            // @since v2.6.3
            QString engine = xml.readElementText();
            Q_UNUSED(engine)
            // if (engine == "simple") {
            //    exportTemplate->setTemplateEngine(ExportEngine::Simple);
            // } else {
            // default for backwards compatibility because older templates don't have a <engine> tag
            exportTemplate->setTemplateEngine(ExportEngine::Simple);
            // }

        } else if (xml.name() == "mediaelch-min") {
            // @since v2.6.3
            exportTemplate->setMediaElchVersionMin(mediaelch::VersionInfo(xml.readElementText()));

        } else if (xml.name() == "mediaelch-max") {
            // @since v2.6.3
            exportTemplate->setMediaElchVersionMax(mediaelch::VersionInfo(xml.readElementText()));

        } else if (xml.name() == "file") {
            exportTemplate->setRemoteFile(xml.readElementText());
        } else if (xml.name() == "version") {
            exportTemplate->setVersion(xml.readElementText());
        } else if (xml.name() == "supports") {
            QVector<ExportTemplate::ExportSection> sections;
            while (xml.readNextStartElement()) {
                if (xml.name() == "section") {
                    QString section = xml.readElementText();
                    if (section == "movies") {
                        sections << ExportTemplate::ExportSection::Movies;
                    } else if (section == "tvshows") {
                        sections << ExportTemplate::ExportSection::TvShows;
                    } else if (section == "concerts") {
                        sections << ExportTemplate::ExportSection::Concerts;
                    }
                } else {
                    xml.skipCurrentElement();
                }
            }
            exportTemplate->setExportSections(sections);
        } else {
            xml.skipCurrentElement();
        }
    }

    return exportTemplate;
}

void ExportTemplateLoader::installTemplate(ExportTemplate* exportTemplate)
{
    QNetworkReply* reply = m_qnam.get(QNetworkRequest(QUrl(exportTemplate->remoteFile())));
    reply->setProperty("storage", Storage::toVariant(reply, exportTemplate));
    connect(reply, &QNetworkReply::finished, this, &ExportTemplateLoader::onDownloadTemplateFinished);
}

void ExportTemplateLoader::onDownloadTemplateFinished()
{
    auto* reply = dynamic_cast<QNetworkReply*>(sender());
    if (reply == nullptr) {
        return;
    }
    ExportTemplate* exportTemplate = reply->property("storage").value<Storage*>()->exportTemplate();
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Network Error" << reply->errorString();
        emit sigTemplateInstalled(exportTemplate, false);
        return;
    }

    QByteArray ba = reply->readAll();
    QBuffer buffer(&ba);
    if (!unpackTemplate(buffer, exportTemplate)) {
        qDebug() << "Could not unpack template";
        emit sigTemplateInstalled(exportTemplate, false);
        return;
    }

    emit sigTemplateInstalled(exportTemplate, true);

    loadLocalTemplates();
    emit sigTemplatesLoaded(mergeTemplates(m_localTemplates, m_remoteTemplates));
}

bool ExportTemplateLoader::uninstallTemplate(ExportTemplate* exportTemplate)
{
    QString location = Settings::instance()->exportTemplatesDir() + "/" + exportTemplate->identifier();
    QDir storageDir(location);
    if (storageDir.exists() && !removeDir(storageDir.absolutePath())) {
        emit sigTemplateUninstalled(exportTemplate, false);
        return false;
    }

    loadLocalTemplates();
    emit sigTemplateUninstalled(exportTemplate, true);
    emit sigTemplatesLoaded(mergeTemplates(m_localTemplates, m_remoteTemplates));
    return true;
}

bool ExportTemplateLoader::unpackTemplate(QBuffer& buffer, ExportTemplate* exportTemplate)
{
    QString location = Settings::instance()->exportTemplatesDir();
    QDir storageDir(location);
    if (!storageDir.exists() && !storageDir.mkpath(location)) {
        qWarning() << "Could not create storage location";
        return false;
    }

    storageDir.setPath(location + QDir::separator() + exportTemplate->identifier());
    if ((exportTemplate->isInstalled() || storageDir.exists()) && !uninstallTemplate(exportTemplate)) {
        qWarning() << "Could not uninstall template";
        return false;
    }

    if (!storageDir.mkpath(storageDir.absolutePath())) {
        qWarning() << "Could not create storage path";
        return false;
    }

    QuaZip zip(&buffer);
    if (!zip.open(QuaZip::mdUnzip)) {
        qWarning() << "Zip file could not be opened";
        return false;
    }
    QuaZipFile file(&zip);
    for (bool more = zip.goToFirstFile(); more; more = zip.goToNextFile()) {
        if (zip.getCurrentFileName().endsWith("/")) {
            if (!storageDir.mkdir(zip.getCurrentFileName())) {
                qWarning() << "Could not create subdirectory";
                return false;
            }
            continue;
        }
        file.open(QIODevice::ReadOnly);
        QByteArray ba = file.readAll();
        file.close();

        QFile f(QString("%1/%2").arg(storageDir.absolutePath()).arg(zip.getCurrentFileName()));
        if (f.open(QIODevice::WriteOnly)) {
            f.write(ba);
            f.close();
        }
    }
    if (zip.getZipError() != UNZ_OK) {
        qWarning() << "There was an error while uncompressing the file";
        return false;
    }

    return true;
}

bool ExportTemplateLoader::removeDir(const QString& dirName)
{
    bool result = true;
    QDir dir(dirName);
    if (dir.exists(dirName)) {
        for (const QFileInfo& info : dir.entryInfoList(
                 QDir::NoDotAndDotDot | QDir::System | QDir::Hidden | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
            if (info.isDir()) {
                result = removeDir(info.absoluteFilePath());
            } else {
                result = QFile::remove(info.absoluteFilePath());
            }

            if (!result) {
                return false;
            }
        }
        result = dir.rmdir(dirName);
    }

    return result;
}

QVector<ExportTemplate*> ExportTemplateLoader::mergeTemplates(QVector<ExportTemplate*> local,
    QVector<ExportTemplate*> remote)
{
    QVector<ExportTemplate*> templates = local;
    for (ExportTemplate* remoteTemplate : remote) {
        bool found = false;
        for (ExportTemplate* localTemplate : templates) {
            if (localTemplate->identifier() == remoteTemplate->identifier()) {
                found = true;
                localTemplate->setRemote(true);
                localTemplate->setRemoteVersion(remoteTemplate->version());
                localTemplate->setRemoteFile(remoteTemplate->remoteFile());
            }
        }
        if (!found) {
            templates << remoteTemplate;
        }
    }

    std::sort(templates.begin(), templates.end(), ExportTemplate::lessThan);

    return templates;
}

QVector<ExportTemplate*> ExportTemplateLoader::installedTemplates()
{
    std::sort(m_localTemplates.begin(), m_localTemplates.end(), ExportTemplate::lessThan);
    return m_localTemplates;
}

ExportTemplate* ExportTemplateLoader::getTemplateByIdentifier(QString identifier)
{
    if (identifier.isEmpty()) {
        return nullptr;
    }
    auto result = std::find_if(m_localTemplates.begin(), m_localTemplates.end(), [&identifier](auto& exportTemplate) {
        return (exportTemplate->identifier() == identifier);
    });
    return (result != m_localTemplates.cend()) ? *result : nullptr;
}
