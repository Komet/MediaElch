#pragma once

#include <QString>
#include <QStringList>

namespace mediaelch {

/// \brief Filesystem interface for MediaElch
/// \details This interface can be used to have a mockable file system.
///          QDir, QFile and everything are very useful, but to test accesses
///          to the filesystem, we want to mock them.  This interface can be
///          used for that.  Note that this class is currently not used a lot.
class Filesystem
{
public:
    Filesystem() = default;
    virtual ~Filesystem() = default;

    virtual QStringList directories(const QString& path) = 0;
};


class ElchFilesystem : public Filesystem
{
public:
    ElchFilesystem() = default;
    virtual ~ElchFilesystem() = default;

    QStringList directories(const QString& path) override;
};

} // namespace mediaelch
