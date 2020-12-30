#pragma once

#include "file/FileFilter.h"

#include <QString>
#include <QStringList>
#include <QVector>

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
    enum class RenameType : int8_t
    {
        Movies,
        TvShows,
        Concerts,
        All
    };
    enum class RenameResult : int8_t
    {
        Failed,
        Success
    };
    enum class RenameOperation : int8_t
    {
        CreateDir,
        Move,
        Rename
    };
    enum class RenameError : int8_t
    {
        None, // Todo: Be more specific about what error occurred
        Error
    };

    Renamer(RenamerConfig config, RenamerDialog* dialog);

    static QString typeToString(Renamer::RenameType type);
    static QString replace(QString& text, const QString& search, QString replacement);
    static QString replaceCondition(QString& text, const QString& condition, const QString& replace);
    static QString replaceCondition(QString& text, const QString& condition, bool hasCondition);

    static bool rename(QDir& dir, QString newName);
    static bool rename(const QString& file, const QString& newName);

protected:
    RenamerConfig m_config;
    RenamerDialog* m_dialog;
    const mediaelch::FileFilter& m_extraFiles;
};
