#include "ExportTemplate.h"

#include <QStringList>

ExportTemplate::ExportTemplate(QObject *parent) :
    QObject(parent)
{
    m_isInstalled = false;
    m_isRemote = false;
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

QString ExportTemplate::description() const
{
    QString locale = QLocale::system().name();
    QString shortLocale = locale;
    if (locale.split("_").count() > 1)
        shortLocale = locale.split("_").at(0);
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

void ExportTemplate::setExportSections(QList<ExportSection> exportSections)
{
    m_exportSections = exportSections;
}

QList<ExportTemplate::ExportSection> ExportTemplate::exportSections()
{
    return m_exportSections;
}

void ExportTemplate::setIdentifier(QString identifier)
{
    m_identifier = identifier;
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
    m_name = name;
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
    m_remoteFile = remoteFile;
}

QString ExportTemplate::remoteFile() const
{
    return m_remoteFile;
}

void ExportTemplate::setVersion(QString version)
{
    m_version = version;
}

QString ExportTemplate::version() const
{
    return m_version;
}

void ExportTemplate::setRemoteVersion(QString remoteVersion)
{
    m_remoteVersion = remoteVersion;
}

QString ExportTemplate::remoteVersion() const
{
    return m_remoteVersion;
}

bool ExportTemplate::updateAvailable() const
{
    if (remoteVersion().isEmpty() || !isInstalled())
        return false;

    return remoteVersion().toFloat() > version().toFloat();
}

QMap<QString, QString> ExportTemplate::descriptions() const
{
    return m_description;
}

bool ExportTemplate::lessThan(ExportTemplate *a, ExportTemplate *b)
{
    return QString::localeAwareCompare(a->name(), b->name());
}

QDebug operator<<(QDebug dbg, const ExportTemplate &exportTemplate)
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

QDebug operator<<(QDebug dbg, const ExportTemplate *exportTemplate)
{
    dbg.nospace() << *exportTemplate;
    return dbg.space();
}
