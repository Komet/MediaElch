#include "ExportTemplate.h"

#include "Version.h"
#include "settings/Settings.h"

#include <QDir>
#include <QStringList>

ExportTemplate::ExportTemplate(QObject* parent) : QObject(parent), m_isRemote{false}, m_isInstalled{false}
{
}

void ExportTemplate::setAuthor(QString author)
{
    m_author = author;
}

QString ExportTemplate::author() const
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

void ExportTemplate::setDirectory(QDir templateDirectory)
{
    m_directory = templateDirectory.path();
}

QVector<ExportTemplate::ExportSection> ExportTemplate::exportSections()
{
    return m_exportSections;
}

void ExportTemplate::setIdentifier(QString identifier)
{
    m_identifier = std::move(identifier);
}

QString ExportTemplate::identifier() const
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

QString ExportTemplate::name() const
{
    return m_name;
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

QString ExportTemplate::remoteFile() const
{
    return m_remoteFile;
}

void ExportTemplate::setVersion(QString version)
{
    m_version = std::move(version);
}

QString ExportTemplate::version() const
{
    return m_version;
}

void ExportTemplate::setRemoteVersion(QString remoteVersion)
{
    m_remoteVersion = std::move(remoteVersion);
}

QString ExportTemplate::remoteVersion() const
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

QMap<QString, QString> ExportTemplate::descriptions() const
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
    if (QFileInfo(QString{getTemplateLocation() + "/%1_%2.html"}.arg(baseName).arg(locale)).exists()) {
        templateFile = QString{getTemplateLocation() + "/%1_%2.html"}.arg(baseName).arg(locale);
    } else if (QFileInfo(QString{getTemplateLocation() + "/%1_%2.html"}.arg(baseName).arg(shortLocale)).exists()) {
        templateFile = QString{getTemplateLocation() + "/%1_%2.html"}.arg(baseName).arg(shortLocale);
    } else if (QFileInfo(getTemplateLocation() + QStringLiteral("/%1.html").arg(baseName)).exists()) {
        templateFile = getTemplateLocation() + QStringLiteral("/%1.html").arg(baseName);
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

QString ExportTemplate::getTemplateLocation()
{
    if (!m_directory.isEmpty()) {
        return m_directory;
    }
    qCritical() << "Testabc";
    return Settings::instance()->exportTemplatesDir() + "/" + identifier();
}

void ExportTemplate::copyTo(QString path)
{
    QStringList excludes{"metadata.xml", //
        "movies.html",                   //
        "movies",                        //
        "concerts.html",                 //
        "concerts",                      //
        "tvshows.html",                  //
        "tvshows",                       //
        "episode"};

    QDir templateDir(getTemplateLocation());
    for (const QFileInfo& fi : templateDir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs)) {
        if (excludes.contains(fi.fileName())) {
            continue;
        }

        if (fi.isDir()) {
            copyDir(fi.absoluteFilePath(), path + "/" + fi.fileName());
        } else {
            QFile::copy(fi.absoluteFilePath(), path + "/" + fi.fileName());
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

QDir ExportTemplate::directory() const
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
    for (const QFileInfo& info : srcDir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot)) {
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
        out.append(QString("    %1: %2").arg(it.key()).arg(it.value()).append(nl));
    }
    dbg.nospace() << out;
    return dbg.maybeSpace();
}

QDebug operator<<(QDebug dbg, const ExportTemplate* exportTemplate)
{
    dbg.nospace() << *exportTemplate;
    return dbg.space();
}
