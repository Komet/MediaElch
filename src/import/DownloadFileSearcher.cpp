#include "import/DownloadFileSearcher.h"

#include "settings/Settings.h"

#include <QRegularExpression>

// Still required for wildcards at the moment.
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
#    include <QRegExp>
#endif

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
                    m_packages[base].size += static_cast<double>(it.fileInfo().size());
                } else {
                    Package p;
                    p.baseName = base;
                    p.size = static_cast<double>(it.fileInfo().size());
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
                    m_imports[base].size += static_cast<double>(it.fileInfo().size());
                } else {
                    Import i;
                    i.baseName = base;
                    if (isSubtitle(it.fileInfo())) {
                        i.extraFiles << it.filePath();
                    } else {
                        i.files << it.filePath();
                    }
                    i.size = static_cast<double>(it.fileInfo().size());
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
    QString fileName = fileInfo.fileName();

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

    static QRegularExpression rx("r[0-9]*");
    return rx.match(file.suffix()).hasMatch();
}

bool DownloadFileSearcher::isImportable(const QFileInfo& file) const
{
    QStringList filters;
    filters << Settings::instance()->advanced()->movieFilters().filters();
    filters << Settings::instance()->advanced()->tvShowFilters().filters();
    filters << Settings::instance()->advanced()->concertFilters().filters();
    filters.removeDuplicates();

    for (const QString& filter : asConst(filters)) {
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
        QRegExp rx(filter);
        rx.setPatternSyntax(QRegExp::Wildcard);
        if (rx.exactMatch(file.fileName())) {
            return true;
        }
#else
        QRegularExpression rx(QRegularExpression::wildcardToRegularExpression(filter));
        if (rx.match(file.fileName()).hasMatch()) {
            return true;
        }
#endif
    }
    return false;
}

bool DownloadFileSearcher::isSubtitle(const QFileInfo& file) const
{
    const QStringList filters = Settings::instance()->advanced()->subtitleFilters().filters();
    for (const QString& filter : filters) {
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
        QRegExp rx(filter);
        rx.setPatternSyntax(QRegExp::Wildcard);
        if (rx.exactMatch(file.fileName())) {
            return true;
        }
#else
        QRegularExpression rx(QRegularExpression::wildcardToRegularExpression(filter));
        if (rx.match(file.fileName()).hasMatch()) {
            return true;
        }
#endif
    }

    return false;
}

} // namespace mediaelch
