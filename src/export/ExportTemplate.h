#pragma once

#include "globals/Globals.h"
#include "globals/VersionInfo.h"

#include <QObject>
#include <QString>
#include <QVector>

/// Represents different template engines. Future releases may introduce
/// mustache or excel engines.
enum class ExportEngine
{
    Simple // Default
};

class ExportTemplate : public QObject
{
    Q_OBJECT
public:
    enum class ExportSection
    {
        Movies,
        TvShows,
        Concerts,
        Movie,
        Concert,
        TvShow,
        Episode
    };

    explicit ExportTemplate(QObject* parent = nullptr);
    bool isRemote() const;
    bool isInstalled() const;
    bool updateAvailable() const;
    QString identifier() const;
    QString name() const;
    QString author() const;
    QString description() const;
    ExportEngine templateEngine() const;
    QString version() const;
    QString remoteVersion() const;
    QString remoteFile() const;
    QVector<ExportTemplate::ExportSection> exportSections();
    QMap<QString, QString> descriptions() const;
    QString getTemplate(ExportTemplate::ExportSection section);
    QString getTemplateLocation();
    void copyTo(QString path);
    mediaelch::VersionInfo mediaElchVersionMin();
    mediaelch::VersionInfo mediaElchVersionMax();
    QDir directory() const;

    void setRemote(bool remote);
    void setInstalled(bool installed);
    void setIdentifier(QString identifier);
    void setName(QString name);
    void setAuthor(QString author);
    void setRemoteFile(QString remoteFile);
    void addDescription(QString language, QString description);
    void setTemplateEngine(ExportEngine engine);
    void setVersion(QString version);
    void setRemoteVersion(QString remoteVersion);
    void setExportSections(QVector<ExportTemplate::ExportSection> exportSections);
    void setMediaElchVersionMin(mediaelch::VersionInfo minVersion);
    void setMediaElchVersionMax(mediaelch::VersionInfo maxVersion);
    /// Set the directory this template exists in.
    /// Default is MediaElch's data directory + "export_themes" + identifier.
    void setDirectory(QDir templateDirectory);

    static bool lessThan(ExportTemplate* a, ExportTemplate* b);

private:
    bool m_isRemote = false;
    bool m_isInstalled = false;
    QString m_identifier;
    QString m_name;
    QString m_author;
    QString m_remoteFile;
    QMap<QString, QString> m_description;
    ExportEngine m_templateEngine = ExportEngine::Simple;
    QString m_version;
    QString m_remoteVersion;
    QVector<ExportTemplate::ExportSection> m_exportSections;
    mediaelch::VersionInfo m_mediaelchMinVersion;
    mediaelch::VersionInfo m_mediaelchMaxVersion;
    QString m_directory;

    bool copyDir(const QString& srcPath, const QString& dstPath);
};

QDebug operator<<(QDebug dbg, const ExportTemplate& exportTemplate);
QDebug operator<<(QDebug dbg, const ExportTemplate* exportTemplate);
