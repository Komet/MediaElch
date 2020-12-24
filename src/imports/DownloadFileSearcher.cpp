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
            if (m_scanDownloads && isPackage(it.fileInfo())) {
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

            } else if (m_scanImports && (isImportable(it.fileInfo()) || isSubtitle(it.fileInfo()))) {
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

    emit sigScanFinished(this);
}

QString DownloadFileSearcher::baseName(const QFileInfo& fileInfo) const
{
    const QString fileName = fileInfo.fileName();

    QRegularExpression rx("^(.*)(part[0-9]*)\\.rar$");
    QRegularExpressionMatch match = rx.match(fileName);
    if (match.hasMatch()) {
        return match.captured(1).endsWith(".") ? match.captured(1).mid(0, match.captured(1).length() - 1)
                                               : match.captured(1);
    }

    rx.setPattern("^(.*)\\.r(?:ar|[0-9]*)$");
    match = rx.match(fileName);
    if (match.hasMatch()) {
        return match.captured(1);
    }

    return fileName;
}

bool DownloadFileSearcher::isPackage(const QFileInfo& file) const
{
    if (file.suffix() == "rar") {
        return true;
    }

    QRegularExpression rx("r[0-9]*");
    return rx.match(file.suffix()).hasMatch();
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
        // TODO: Remove when we require Qt 5.15 or higher. Then use QRegularExpression::wildcardToRegularExpression.
        rx.setPatternSyntax(QRegExp::Wildcard);
        if (rx.exactMatch(file.fileName())) {
            return true;
        }
    }
    return false;
}

bool DownloadFileSearcher::isSubtitle(const QFileInfo& file) const
{
    const QStringList filters = Settings::instance()->advanced()->subtitleFilters().filters();
    for (const QString& filter : filters) {
        // TODO: Remove when we require Qt 5.15 or higher. Then use QRegularExpression::wildcardToRegularExpression.
        QRegExp rx(filter);
        rx.setPatternSyntax(QRegExp::Wildcard);
        if (rx.exactMatch(file.fileName())) {
            return true;
        }
    }
    return false;
}

} // namespace mediaelch
