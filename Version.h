#ifndef VERSION_H
#define VERSION_H

#include <QString>

namespace MediaElch {
namespace Constants {

constexpr char AppName[] = "MediaElch";
constexpr char AppVersionStr[] = "2.4.3-dev";
constexpr char VersionName[] = "Talax";
constexpr char OrganizationName[] = "kvibes";
constexpr char OrganizationDomain[] = "http://mediaelch.de";

#ifdef QT_NO_DEBUG
constexpr char CompilationType[] = "Release";
#else
constexpr char CompilationType[] = "Debug";
#endif

const QString CompilerString = []() {
// Taken from QtCreator (qt-creator/src/plugins/coreplugin/icore.cpp) - Modified
#if defined(Q_CC_CLANG) // must be before GNU, because clang claims to be GNU too
    QString isAppleString;
#if defined(__apple_build_version__) // Apple clang has other version numbers
    isAppleString = QStringLiteral(" (Apple)");
#endif
    return QStringLiteral("Clang ") + QString::number(__clang_major__) + '.'
           + QString::number(__clang_minor__) + isAppleString;

#elif defined(Q_CC_GNU)
    return QStringLiteral("GCC ") + QStringLiteral(__VERSION__);

#elif defined(Q_CC_MSVC)
    if (_MSC_VER > 1999) {
        return QStringLiteral("MSVC <unknown>");
    }
    if (_MSC_VER >= 1910) {
        return QStringLiteral("MSVC 2017");
    }
    if (_MSC_VER >= 1900) {
        return QStringLiteral("MSVC 2015");
    }
#endif
    return QStringLiteral("<unknown compiler>");

}();

} // namespace Constants
} // namespace MediaElch

#endif // VERSION_H
