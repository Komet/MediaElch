#include "Renamer.h"

#include "log/Log.h"
#include "settings/Settings.h"
#include "utils/Meta.h"

#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QVector>


QString renamerTypeToString(RenameType type)
{
    switch (type) {
    case RenameType::All: return "All";
    case RenameType::Movies: return "Movies";
    case RenameType::Concerts: return "Concerts";
    case RenameType::TvShows: return "TvShows";
    }
    qCWarning(generic) << "Unknown RenamerType";
    MediaElch_Debug_Unreachable();
    return "unknown";
}

/**
 * \brief Renamer base class for renaming files according to given patterns.
 *        Note: Currently RenamerDialog is required as a parameter. This may
 *        change in the future.
 * \param renamerConfig Configuration on pattern, etc. used by this renamer
 * \param dialog RenamerDialog that is notified on failure.
 */
Renamer::Renamer(RenamerConfig renamerConfig, RenamerDialog* dialog) :
    m_config(std::move(renamerConfig)),
    m_dialog{dialog},
    m_extraFiles(Settings::instance()->advanced()->subtitleFilters())
{
}
QString Renamer::replace(QString& text, const QString& search, QString replacement)
{
    text.replace("<" + search + ">", replacement.trimmed());
    return text;
}

void Renamer::replaceDelimiter(QString& text) const
{
    if (m_config.replaceDelimiter) {
        text.replace(" ", m_config.delimiter);
    }
}

bool Renamer::rename(const QString& file, const QString& newName)
{
    QFile f(file);
    if (!f.exists()) {
        return false;
    }

    QFile newFile(newName);
    if (newFile.exists() && QString::compare(file, newName, Qt::CaseInsensitive) != 0) {
        return false;
    }

    if (newFile.exists()) {
        if (!f.rename(newName + ".tmp")) {
            return false;
        }
        return f.rename(newName);
    }
    return f.rename(newName);
}

bool Renamer::rename(QDir& dir, QString newName)
{
    if (QString::compare(dir.path(), newName, Qt::CaseInsensitive) == 0) {
        QDir tmpDir;
        if (!tmpDir.rename(dir.path(), dir.path() + "tmp")) {
            return false;
        }
        return tmpDir.rename(dir.path() + "tmp", newName);
    }
    QDir tmpDir;
    return tmpDir.rename(dir.path(), newName);
}
