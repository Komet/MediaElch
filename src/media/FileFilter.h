#pragma once

#include <QDir>
#include <QString>
#include <QStringList>

namespace mediaelch {

class FileFilter
{
public:
    FileFilter() = default;
    explicit FileFilter(QStringList filters) : m_filters(std::move(filters)) {}

    QStringList files(QDir directory) const;
    bool hasFilter() const;
    QStringList filters() const;

private:
    QStringList m_filters;
};

} // namespace mediaelch
