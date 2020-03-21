#include "imports/DownloadFileSearcher.h"

namespace mediaelch {

void DownloadFileSearcher::scan()
{
    for (const SettingsDir& settingsDir : Settings::instance()->directorySettings().downloadDirectories()) {
        QString dir = settingsDir.path.path();
        QDirIterator it(dir,
            QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files,
            QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);
        while (it.hasNext()) {
            it.next();
            if (isPackage(it.fileInfo())) {
                QString base = baseName(it.fileInfo());
                if (m_packages.contains(base)) {
                    m_packages[base].files.append(it.filePath());
                    m_packages[base].size += it.fileInfo().size();
                } else {
                    Package p;
                    p.baseName = base;
                    p.size = it.fileInfo().size();
                    p.files << it.filePath();
                    m_packages.insert(base, p);
                }
            } else if (isImportable(it.fileInfo()) || isSubtitle(it.fileInfo())) {
                QString base = it.fileInfo().completeBaseName();
                if (m_imports.contains(base)) {
                    if (isSubtitle(it.fileInfo())) {
                        m_imports[base].extraFiles.append(it.filePath());
                    } else {
                        m_imports[base].files.append(it.filePath());
                    }
                    m_imports[base].size += it.fileInfo().size();
                } else {
                    Import i;
                    i.baseName = base;
                    if (isSubtitle(it.fileInfo())) {
                        i.extraFiles << it.filePath();
                    } else {
                        i.files << it.filePath();
                    }
                    i.size = it.fileInfo().size();
                    m_imports.insert(base, i);
                }
            }
        }
    }

    QMapIterator<QString, Import> it(m_imports);
    QStringList onlyExtraFiles;
    while (it.hasNext()) {
        it.next();
        if (it.value().files.isEmpty()) {
            onlyExtraFiles.append(it.key());
        }
    }

    for (const QString& base : onlyExtraFiles) {
        m_imports.remove(base);
    }
}

QString DownloadFileSearcher::baseName(const QFileInfo& fileInfo) const
{
    QString fileName = fileInfo.fileName();
    QRegExp rx("(.*)(part[0-9]*)\\.rar");
    if (rx.exactMatch(fileName)) {
        return rx.cap(1).endsWith(".") ? rx.cap(1).mid(0, rx.cap(1).length() - 1) : rx.cap(1);
    }

    rx.setPattern("(.*)\\.r(ar|[0-9]*)");
    if (rx.exactMatch(fileName)) {
        return rx.cap(1);
    }

    return fileName;
}

bool DownloadFileSearcher::isPackage(const QFileInfo& file) const
{
    if (file.suffix() == "rar") {
        return true;
    }

    QRegExp rx("r[0-9]*");
    return rx.exactMatch(file.suffix());
}

bool DownloadFileSearcher::isImportable(const QFileInfo& file) const
{
    QStringList filters;
    filters << Settings::instance()->advanced()->movieFilters().filters();
    filters << Settings::instance()->advanced()->tvShowFilters().filters();
    filters << Settings::instance()->advanced()->concertFilters().filters();
    filters.removeDuplicates();

    for (const QString& filter : filters) {
        QRegExp rx(filter);
        rx.setPatternSyntax(QRegExp::Wildcard);
        if (rx.exactMatch(file.fileName())) {
            return true;
        }
    }
    return false;
}

bool DownloadFileSearcher::isSubtitle(const QFileInfo& file) const
{
    for (const QString& filter : Settings::instance()->advanced()->subtitleFilters().filters()) {
        QRegExp rx(filter);
        rx.setPatternSyntax(QRegExp::Wildcard);
        if (rx.exactMatch(file.fileName())) {
            return true;
        }
    }
    return false;
}

} // namespace mediaelch
