#include "file/FileFilter.h"

namespace mediaelch {

QStringList FileFilter::files(QDir directory) const
{
    if (m_filters.isEmpty() || !directory.exists()) {
        return {};
    }
    return directory.entryList(m_filters, QDir::Files | QDir::System);
}

bool FileFilter::hasFilter() const
{
    return !m_filters.isEmpty();
}

QStringList FileFilter::filters() const
{
    return m_filters;
}

} // namespace mediaelch
