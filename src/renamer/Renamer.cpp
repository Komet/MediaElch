#include "Renamer.h"

#include "globals/Helper.h"
#include "movies/Movie.h"
#include "settings/Settings.h"

#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QVector>

/**
 * @brief Renamer base class for renaming files according to given patterns.
 *        Note: Currently RenamerDialog is required as a parameter. This may
 *        change in the future.
 * @param renamerConfig Configuration on pattern, etc. used by this renamer
 * @param dialog RenamerDialog that is notified on failure.
 */
Renamer::Renamer(RenamerConfig renamerConfig, RenamerDialog* dialog) :
    m_config(std::move(renamerConfig)),
    m_dialog{dialog},
    m_extraFiles(Settings::instance()->advanced()->subtitleFilters())
{
}

QString Renamer::typeToString(Renamer::RenameType type)
{
    switch (type) {
    case Renamer::RenameType::All: return "All";
    case Renamer::RenameType::Movies: return "Movies";
    case Renamer::RenameType::Concerts: return "Concerts";
    case Renamer::RenameType::TvShows: return "TvShows";
    }
    qWarning() << "Unknown RenamerType";
    return "unknown";
}

QString Renamer::replace(QString& text, const QString& search, const QString& replace)
{
    text.replace("<" + search + ">", replace);
    return text;
}

QString Renamer::replaceCondition(QString& text, const QString& condition, const QString& replace)
{
    QRegExp rx("\\{" + condition + "\\}(.*)\\{/" + condition + "\\}");
    rx.setMinimal(true);
    if (rx.indexIn(text) == -1) {
        return Renamer::replace(text, condition, replace);
    }

    QString search = QString("{%1}%2{/%1}").arg(condition).arg(rx.cap(1));
    text.replace(search, !replace.isEmpty() ? rx.cap(1) : "");
    return Renamer::replace(text, condition, replace);
}

QString Renamer::replaceCondition(QString& text, const QString& condition, bool hasCondition)
{
    QRegExp rx("\\{" + condition + "\\}(.*)\\{/" + condition + "\\}");
    rx.setMinimal(true);
    if (rx.indexIn(text) == -1) {
        return text;
    }

    QString search = QString("{%1}%2{/%1}").arg(condition).arg(rx.cap(1));
    text.replace(search, hasCondition ? rx.cap(1) : "");
    return text;
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
