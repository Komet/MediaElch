#pragma once

#include <QRegularExpression>
#include <QString>
#include <QStringList>
#include <QVector>

namespace mediaelch {

/// \brief Filter which files should be used and which ignored.
/// \details Qt's QDir::entryList supports a positive filter list such as *.mp3, etc.
///          This class does exactly that.  Filter what's allowed and what not.
class FileFilter
{
public:
    FileFilter() = default;
    explicit FileFilter(QStringList filenameGlob) : fileGlob(std::move(filenameGlob)) {}

    bool hasValidFilters() const { return !fileGlob.isEmpty(); }

    bool isFileExcluded(const QString& filename) const;
    bool isFolderExcluded(const QString& folder) const;

public:
    QStringList fileGlob = {{"*"}};
    QVector<QRegularExpression> fileExcludes;
    QVector<QRegularExpression> folderExcludes;
};

} // namespace mediaelch
