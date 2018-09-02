#ifndef RENAMER_H
#define RENAMER_H

#include <QList>
#include <QString>
#include <QStringList>

class Movie;
class RenamerDialog;
class QDir;

struct RenamerConfig
{
    QString filePattern;
    QString filePatternMulti;
    QString directoryPattern;
    bool renameFiles = false;
    bool renameDirectories = false;
    bool dryRun = false;
};

class Renamer
{
public:
    enum class RenameType
    {
        Movies,
        TvShows,
        Concerts,
        All
    };
    enum class RenameResult
    {
        Failed,
        Success
    };
    enum class RenameOperation
    {
        CreateDir,
        Move,
        Rename
    };
    enum class RenameError
    {
        None, // Todo: Be more specific about what error occurred
        Error
    };

    Renamer(RenamerConfig config, RenamerDialog *dialog);

    static QString typeToString(Renamer::RenameType type);
    static QString replace(QString &text, const QString &search, const QString &replace);
    static QString replaceCondition(QString &text, const QString &condition, const QString &replace);
    static QString replaceCondition(QString &text, const QString &condition, bool hasCondition);

    static bool rename(QDir &dir, QString newName);
    static bool rename(const QString &file, const QString &newName);

protected:
    RenamerConfig m_config;
    RenamerDialog *m_dialog;
    QStringList m_extraFiles;
};

#endif // RENAMER_H
