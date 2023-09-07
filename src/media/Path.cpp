#include "media/Path.h"

#include <QCryptographicHash>

namespace mediaelch {

QString DirectoryPath::toNativePathString() const
{
    return QDir::toNativeSeparators(toString());
}

bool DirectoryPath::isParentFolderOf(const DirectoryPath& child) const
{
    return child.toString().startsWith(toString());
}

QString DirectoryPath::filePath(const QString& fileName) const
{
    return m_dir.absoluteFilePath(fileName);
}

DirectoryPath DirectoryPath::subDir(const QString& dirName) const
{
    return DirectoryPath{toString() + '/' + dirName};
}


bool operator==(const DirectoryPath& lhs, const DirectoryPath& rhs)
{
    return lhs.toString() == rhs.toString();
}

bool operator!=(const DirectoryPath& lhs, const DirectoryPath& rhs)
{
    return !(lhs == rhs);
}

QDebug operator<<(QDebug debug, const DirectoryPath& dir)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "DirectoryPath(" << dir.toString() << ')';
    return debug;
}


bool operator==(const FilePath& lhs, const FilePath& rhs)
{
    return lhs.toString() == rhs.toString();
}

bool operator!=(const FilePath& lhs, const FilePath& rhs)
{
    return !(lhs == rhs);
}

QDebug operator<<(QDebug debug, const FilePath& dir)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "FilePath(" << dir.toString() << ')';
    return debug;
}

QString FilePath::toNativePathString() const
{
    return QDir::toNativeSeparators(toString());
}

DirectoryPath FilePath::dir() const
{
    return DirectoryPath(m_fileInfo.dir());
}


bool operator==(const FileList& lhs, const FileList& rhs)
{
    return lhs.toStringList() == rhs.toStringList();
}

bool operator!=(const FileList& lhs, const FileList& rhs)
{
    return !(lhs == rhs);
}

void operator<<(FileList& list, const FilePath& file)
{
    list.push_back(file);
}

QString pathHash(const mediaelch::FilePath& path)
{
    return QCryptographicHash::hash(path.toString().toUtf8(), QCryptographicHash::Md5).toHex();
}

} // namespace mediaelch
