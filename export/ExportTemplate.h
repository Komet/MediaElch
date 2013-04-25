#ifndef EXPORTTEMPLATE_H
#define EXPORTTEMPLATE_H

#include <QList>
#include <QObject>
#include <QString>

#include "globals/Globals.h"

class ExportTemplate : public QObject
{
    Q_OBJECT
public:
    enum ExportSection {
        SectionMovies, SectionTvShows, SectionConcerts
    };

    explicit ExportTemplate(QObject *parent = 0);
    bool isRemote() const;
    bool isInstalled() const;
    bool updateAvailable() const;
    QString identifier() const;
    QString name() const;
    QString author() const;
    QString description() const;
    QString version() const;
    QString remoteVersion() const;
    QString remoteFile() const;
    QList<ExportTemplate::ExportSection> exportSections();
    QMap<QString, QString> descriptions() const;

    void setRemote(bool remote);
    void setInstalled(bool installed);
    void setIdentifier(QString identifier);
    void setName(QString name);
    void setAuthor(QString author);
    void setRemoteFile(QString remoteFile);
    void addDescription(QString language, QString description);
    void setVersion(QString version);
    void setRemoteVersion(QString remoteVersion);
    void setExportSections(QList<ExportTemplate::ExportSection> exportSections);

private:
    bool m_isRemote;
    bool m_isInstalled;
    QString m_identifier;
    QString m_name;
    QString m_author;
    QString m_remoteFile;
    QMap<QString, QString> m_description;
    QString m_version;
    QString m_remoteVersion;
    QList<ExportTemplate::ExportSection> m_exportSections;
};

QDebug operator<<(QDebug dbg, const ExportTemplate &exportTemplate);
QDebug operator<<(QDebug dbg, const ExportTemplate *exportTemplate);

#endif // EXPORTTEMPLATE_H
