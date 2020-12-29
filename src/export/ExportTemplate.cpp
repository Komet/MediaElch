#include "ExportTemplate.h"

#include "Version.h"
#include "settings/Settings.h"

#include <QDir>
#include <QStringList>

ExportTemplate::ExportTemplate(QObject* parent) : QObject(parent)
{
}

void ExportTemplate::setAuthor(QString author)
{
    m_author = author;
}

const QString& ExportTemplate::author() const
{
    return m_author;
}

void ExportTemplate::addDescription(QString language, QString description)
{
    m_description.insert(language, description);
}

void ExportTemplate::setTemplateEngine(ExportEngine engine)
{
    m_templateEngine = engine;
}

QString ExportTemplate::description() const
{
    QString locale = QLocale::system().name();
    QString shortLocale = locale;
    if (locale.split("_").count() > 1) {
        shortLocale = locale.split("_").at(0);
    }
    QString description;
    QMapIterator<QString, QString> it(m_description);
    bool localeLoaded = false;
    while (it.hasNext()) {
        it.next();
        if (it.key() == locale) {
            description = it.value();
        } else if (!localeLoaded && it.key() == shortLocale) {
            description = it.value();
            localeLoaded = true;
        } else if (description.isEmpty()) {
            description = it.value();
        }
    }
    return description;
}

ExportEngine ExportTemplate::templateEngine() const
{
    return m_templateEngine;
}

void ExportTemplate::setExportSections(QVector<ExportSection> exportSections)
{
    m_exportSections = std::move(exportSections);
}

void ExportTemplate::setMediaElchVersionMin(mediaelch::VersionInfo minVersion)
{
    m_mediaelchMinVersion = minVersion.isValid() ? minVersion : mediaelch::currentVersion();
}

void ExportTemplate::setMediaElchVersionMax(mediaelch::VersionInfo maxVersion)
{
    m_mediaelchMaxVersion = maxVersion.isValid() ? maxVersion : mediaelch::currentVersion();
}

void ExportTemplate::setDirectory(mediaelch::DirectoryPath templateDirectory)
{
    m_directory = templateDirectory;
}

const QVector<ExportTemplate::ExportSection>& ExportTemplate::exportSections()
{
    return m_exportSections;
}

void ExportTemplate::setIdentifier(QString identifier)
{
    m_identifier = std::move(identifier);
}

const QString& ExportTemplate::identifier() const
{
    return m_identifier;
}

void ExportTemplate::setInstalled(bool installed)
{
    m_isInstalled = installed;
}

bool ExportTemplate::isInstalled() const
{
    return m_isInstalled;
}

void ExportTemplate::setName(QString name)
{
    m_name = std::move(name);
}

const QString& ExportTemplate::name() const
{
    return m_name;
}

void ExportTemplate::setWebsite(QString website)
{
    m_website = std::move(website);
}

const QString& ExportTemplate::website() const
{
    return m_website;
}

void ExportTemplate::setRemote(bool remote)
{
    m_isRemote = remote;
}

bool ExportTemplate::isRemote() const
{
    return m_isRemote;
}

void ExportTemplate::setRemoteFile(QString remoteFile)
{
    m_remoteFile = std::move(remoteFile);
}

const QString& ExportTemplate::remoteFileChecksum() const
{
    return m_remoteFileChecksum;
}

void ExportTemplate::setRemoteFileChecksum(QString remoteFileChecksum)
{
    m_remoteFileChecksum = std::move(remoteFileChecksum);
}

const QString& ExportTemplate::remoteFile() const
{
    return m_remoteFile;
}

void ExportTemplate::setVersion(QString version)
{
    m_version = std::move(version);
}

const QString& ExportTemplate::version() const
{
    return m_version;
}

void ExportTemplate::setRemoteVersion(QString remoteVersion)
{
    m_remoteVersion = std::move(remoteVersion);
}

const QString& ExportTemplate::remoteVersion() const
{
    return m_remoteVersion;
}

bool ExportTemplate::updateAvailable() const
{
    if (remoteVersion().isEmpty() || !isInstalled()) {
        return false;
    }

    return remoteVersion().toFloat() > version().toFloat();
}

const QMap<QString, QString>& ExportTemplate::descriptions() const
{
    return m_description;
}

bool ExportTemplate::lessThan(ExportTemplate* a, ExportTemplate* b)
{
    return (QString::localeAwareCompare(a->name(), b->name()) < 0);
}

QString ExportTemplate::getTemplate(ExportTemplate::ExportSection section)
{
    QString baseName;
    if (section == ExportSection::Movies) {
        baseName = "movies";
    } else if (section == ExportSection::Movie) {
        baseName = "movies/movie";
    } else if (section == ExportSection::Concerts) {
        baseName = "concerts";
    } else if (section == ExportSection::Concert) {
        baseName = "concerts/concert";
    } else if (section == ExportSection::TvShows) {
        baseName = "tvshows";
    } else if (section == ExportSection::TvShow) {
        baseName = "tvshows/tvshow";
    } else if (section == ExportSection::Episode) {
        baseName = "episodes/episode";
    } else {
        return "";
    }
    QString locale = QLocale::system().name();
    QString shortLocale = locale;
    if (locale.split("_").count() > 1) {
        shortLocale = locale.split("_").at(0);
    }

    QString templateFile;
    if (QFileInfo(QString{getTemplateLocation().toString() + "/%1_%2.html"}.arg(baseName).arg(locale)).exists()) {
        templateFile = QString{getTemplateLocation().toString() + "/%1_%2.html"}.arg(baseName).arg(locale);
    } else if (QFileInfo(QString{getTemplateLocation().toString() + "/%1_%2.html"}.arg(baseName).arg(shortLocale))
                   .exists()) {
        templateFile = QString{getTemplateLocation().toString() + "/%1_%2.html"}.arg(baseName).arg(shortLocale);
    } else if (QFileInfo(getTemplateLocation().toString() + QStringLiteral("/%1.html").arg(baseName)).exists()) {
        templateFile = getTemplateLocation().toString() + QStringLiteral("/%1.html").arg(baseName);
    }

    if (templateFile.isEmpty()) {
        return "";
    }
    QString content;
    QFile file(templateFile);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        return "";
    }
    content = QString::fromUtf8(file.readAll());
    file.close();

    return content;
}

mediaelch::DirectoryPath ExportTemplate::getTemplateLocation()
{
    if (m_directory.isValid()) {
        return m_directory;
    }
    return Settings::instance()->exportTemplatesDir().subDir(identifier());
}

void ExportTemplate::copyTo(mediaelch::DirectoryPath path)
{
    QStringList excludes{"metadata.xml", //
        "movies.html",                   //
        "movies",                        //
        "concerts.html",                 //
        "concerts",                      //
        "tvshows.html",                  //
        "tvshows",                       //
        "episode"};

    QDir templateDir(getTemplateLocation().dir());
    const auto entries = templateDir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs);
    for (const QFileInfo& fi : entries) {
        if (excludes.contains(fi.fileName())) {
            continue;
        }

        if (fi.isDir()) {
            copyDir(fi.absoluteFilePath(), path.filePath(fi.fileName()));
        } else {
            QFile::copy(fi.absoluteFilePath(), path.filePath(fi.fileName()));
        }
    }
}

mediaelch::VersionInfo ExportTemplate::mediaElchVersionMin()
{
    return m_mediaelchMinVersion;
}

mediaelch::VersionInfo ExportTemplate::mediaElchVersionMax()
{
    return m_mediaelchMaxVersion;
}

mediaelch::DirectoryPath ExportTemplate::directory() const
{
    return m_directory;
}

bool ExportTemplate::copyDir(const QString& srcPath, const QString& dstPath)
{
    QDir parentDstDir(QFileInfo(dstPath).path());
    if (!parentDstDir.mkdir(QFileInfo(dstPath).fileName())) {
        return false;
    }

    QDir srcDir(srcPath);
    const auto entries = srcDir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
    for (const QFileInfo& info : entries) {
        QString srcItemPath = srcPath + "/" + info.fileName();
        QString dstItemPath = dstPath + "/" + info.fileName();
        if (info.isDir()) {
            if (!copyDir(srcItemPath, dstItemPath)) {
                return false;
            }
        } else if (info.isFile()) {
            if (!QFile::copy(srcItemPath, dstItemPath)) {
                return false;
            }
        } else {
            qDebug() << "Unhandled item" << info.filePath() << "in cpDir";
        }
    }
    return true;
}

QDebug operator<<(QDebug dbg, const ExportTemplate& exportTemplate)
{
    QString nl = "\n";
    QString out;
    out.append("Export Template").append(nl);
    out.append(QString("  Name:             ").append(exportTemplate.name()).append(nl));
    out.append(QString("  Identifier:       ").append(exportTemplate.identifier()).append(nl));
    out.append(QString("  Author:           ").append(exportTemplate.author()).append(nl));
    out.append(QString("  Version:          ").append(exportTemplate.version()).append(nl));
    out.append(QString("  Remote Version:   ").append(exportTemplate.remoteVersion()).append(nl));
    out.append(QString("  Remote File:      ").append(exportTemplate.remoteFile()).append(nl));
    out.append(QString("  Installed:        ").append((exportTemplate.isInstalled()) ? "yes" : "no").append(nl));
    out.append(QString("  Remote:           ").append((exportTemplate.isRemote() ? "yes" : "no")).append(nl));
    out.append(QString("  Update available: ").append((exportTemplate.updateAvailable() ? "yes" : "no")).append(nl));
    out.append(QString("  Descriptions:     ").append(nl));
    QMapIterator<QString, QString> it(exportTemplate.descriptions());
    while (it.hasNext()) {
        it.next();
        out.append(QString("    %1: %2").arg(it.key(), it.value()).append(nl));
    }
    dbg.nospace() << out;
    return dbg.maybeSpace();
}

QDebug operator<<(QDebug dbg, const ExportTemplate* exportTemplate)
{
    dbg.nospace() << *exportTemplate;
    return dbg.space();
}
