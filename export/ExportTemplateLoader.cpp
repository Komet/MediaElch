#include "ExportTemplateLoader.h"

#include <QBuffer>
#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QNetworkReply>
#include <QXmlStreamReader>
#include <quazip/quazip.h>
#include <quazip/quazipfile.h>

#include "data/Storage.h"

ExportTemplateLoader::ExportTemplateLoader(QObject *parent) :
    QObject(parent)
{
    loadLocalTemplates();
}

ExportTemplateLoader *ExportTemplateLoader::instance(QObject *parent)
{
    static ExportTemplateLoader *instance = 0;
    if (!instance)
        instance = new ExportTemplateLoader(parent);
    return instance;
}

QNetworkAccessManager *ExportTemplateLoader::qnam()
{
    return &m_qnam;
}

void ExportTemplateLoader::getRemoteTemplates()
{
    QNetworkReply *reply = qnam()->get(QNetworkRequest(QUrl("http://data.mediaelch.de/export_themes.xml")));
    connect(reply, SIGNAL(finished()), this, SLOT(onLoadRemoteTemplatesFinished()));
}

void ExportTemplateLoader::onLoadRemoteTemplatesFinished()
{
    QList<ExportTemplate*> templates;
    QNetworkReply *reply = static_cast<QNetworkReply*>(sender());
    if (!reply)
        return;
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Network Error" << reply->errorString();
        emit sigTemplatesLoaded(mergeTemplates(m_localTemplates, m_remoteTemplates));
        return;
    }

    QString msg = QString::fromUtf8(reply->readAll());
    QXmlStreamReader xml(msg);
    xml.readNextStartElement();
    while (xml.readNextStartElement()) {
        if (xml.name() == "theme") {
            ExportTemplate *exportTemplate = parseTemplate(xml);
            templates << exportTemplate;
        } else {
            xml.skipCurrentElement();
        }
    }

    m_remoteTemplates = templates;

    emit sigTemplatesLoaded(mergeTemplates(m_localTemplates, m_remoteTemplates));
}

void ExportTemplateLoader::loadLocalTemplates()
{
    QString location = QDesktopServices::storageLocation(QDesktopServices::DataLocation) + QDir::separator() + "export_themes";
    QDir storageDir(location);
    if (!storageDir.exists() && !storageDir.mkpath(location)) {
        qWarning() << "Could not create storage location";
        return;
    }

    m_localTemplates.clear();
    foreach (QFileInfo info, storageDir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllDirs)) {
        QList<QFileInfo> infos = QDir(info.absoluteFilePath()).entryInfoList(QStringList() << "metadata.xml");
        if (infos.isEmpty() || infos.count() > 1)
            continue;

        QFile file(infos.first().absoluteFilePath());
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "File" << infos.first().absoluteFilePath() << "could not be opened for reading";
            continue;
        }
        QString content = QString::fromUtf8(file.readAll());
        file.close();

        QXmlStreamReader xml(content);
        xml.readNextStartElement();
        ExportTemplate *exportTemplate = parseTemplate(xml);
        exportTemplate->setInstalled(true);
        m_localTemplates << exportTemplate;
    }
}

ExportTemplate *ExportTemplateLoader::parseTemplate(QXmlStreamReader &xml)
{
    ExportTemplate *exportTemplate = new ExportTemplate(this);
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
        } else if (xml.name() == "file") {
            exportTemplate->setRemoteFile(xml.readElementText());
        } else if (xml.name() == "version") {
            exportTemplate->setVersion(xml.readElementText());
        } else if (xml.name() == "supports") {
            QList<ExportTemplate::ExportSection> sections;
            while (xml.readNextStartElement()) {
                if (xml.name() == "section") {
                    QString section = xml.readElementText();
                    if (section == "movies")
                        sections << ExportTemplate::SectionMovies;
                    else if (section == "tvshows")
                        sections << ExportTemplate::SectionTvShows;
                    else if (section == "concerts")
                        sections << ExportTemplate::SectionConcerts;
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

void ExportTemplateLoader::installTemplate(ExportTemplate *exportTemplate)
{
    QNetworkReply *reply = qnam()->get(QNetworkRequest(QUrl(exportTemplate->remoteFile())));
    reply->setProperty("storage", Storage::toVariant(reply, exportTemplate));
    connect(reply, SIGNAL(finished()), this, SLOT(onDownloadTemplateFinished()));
}

void ExportTemplateLoader::onDownloadTemplateFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(sender());
    if (!reply)
        return;
    ExportTemplate *exportTemplate = reply->property("storage").value<Storage*>()->exportTemplate();
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Network Error" << reply->errorString();
        emit sigTemplateInstalled(exportTemplate, false);
        return;
    }

    QByteArray ba = reply->readAll();
    QBuffer buffer(&ba);
    if (!unpackTemplate(buffer, exportTemplate)) {
        emit sigTemplateInstalled(exportTemplate, false);
        return;
    }

    emit sigTemplateInstalled(exportTemplate, true);

    loadLocalTemplates();
    emit sigTemplatesLoaded(mergeTemplates(m_localTemplates, m_remoteTemplates));
}

bool ExportTemplateLoader::uninstallTemplate(ExportTemplate *exportTemplate)
{
    QString location = QDesktopServices::storageLocation(QDesktopServices::DataLocation) + "/export_themes/" + exportTemplate->identifier();
    QDir storageDir(location);
    if (storageDir.exists() && !removeDir(storageDir.absolutePath()))
        return false;

    loadLocalTemplates();
    emit sigTemplatesLoaded(mergeTemplates(m_localTemplates, m_remoteTemplates));
    return true;
}

bool ExportTemplateLoader::unpackTemplate(QBuffer &buffer, ExportTemplate *exportTemplate)
{
    QString location = QDesktopServices::storageLocation(QDesktopServices::DataLocation) + QDir::separator() + "export_themes";
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

    QuaZip zip(&buffer);
    if (!zip.open(QuaZip::mdUnzip)) {
        qWarning() << "Zip file could not be opened";
        return false;
    }
    QuaZipFile file(&zip);
    for (bool more=zip.goToFirstFile() ; more ; more=zip.goToNextFile()) {
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

bool ExportTemplateLoader::removeDir(const QString &dirName)
{
    bool result = true;
    QDir dir(dirName);
    if (dir.exists(dirName)) {
        foreach (QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
            if (info.isDir())
                result = removeDir(info.absoluteFilePath());
            else
                result = QFile::remove(info.absoluteFilePath());

            if (!result)
                return result;
        }
        result = dir.rmdir(dirName);
    }

    return result;
}

QList<ExportTemplate*> ExportTemplateLoader::mergeTemplates(QList<ExportTemplate *> local, QList<ExportTemplate *> remote)
{
    QList<ExportTemplate*> templates = local;
    foreach (ExportTemplate *remoteTemplate, remote) {
        bool found = false;
        foreach (ExportTemplate *localTemplate, templates) {
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
    return templates;
}

QList<ExportTemplate*> ExportTemplateLoader::installedTemplates()
{
    return m_localTemplates;
}
