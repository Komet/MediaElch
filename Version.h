#pragma once

#include "globals/VersionInfo.h"

#include <QString>

namespace mediaelch {

namespace constants {

// Update versions via ./scripts/release/bump_version.py
constexpr char AppName[] = "MediaElch";
constexpr char AppVersionStr[] = "2.10.7"; // major.minor.patch
constexpr char AppVersionFullStr[] = "2.10.7-dev"; // major.minor.patch[-identifier]
constexpr char VersionName[] = "Benzar";
constexpr char OrganizationName[] = "kvibes";

#ifdef QT_NO_DEBUG
constexpr char CompilationType[] = "Release";
#else
constexpr char CompilationType[] = "Debug";
#endif

const QString CompilerString = []() -> QString {
// Taken from QtCreator (qt-creator/src/plugins/coreplugin/icore.cpp) - Modified
#if defined(Q_CC_CLANG) // must be before GNU, because clang claims to be GNU too
    QString isAppleString;

#    if defined(__apple_build_version__) // Apple clang has other version numbers
    isAppleString = QLatin1String(" (Apple)");
#    endif
    return QLatin1String("Clang ") + QString::number(__clang_major__) + '.' + QString::number(__clang_minor__)
           + isAppleString;

#elif defined(Q_CC_GNU)
    return QLatin1String("GCC ") + QLatin1String(__VERSION__);

#elif defined(Q_CC_MSVC)
    if (_MSC_VER > 1999) {
        return QLatin1String("MSVC <unknown>");
    }
    if (_MSC_VER >= 1920) {
        return QLatin1String("MSVC 2019");
    }
    if (_MSC_VER >= 1910) {
        return QLatin1String("MSVC 2017");
    }
    if (_MSC_VER >= 1900) {
        return QLatin1String("MSVC 2015");
    }
    return QLatin1String("MSVC <unknown>");

#else
    return QLatin1String("<unknown compiler>");
#endif
}();

} // namespace constants

inline VersionInfo currentVersion()
{
    return VersionInfo(constants::AppVersionStr);
}

/// \brief   Returns a string that identifies this app and version.
/// \details The identifier can be used for User-Agent headers of network requests.
///          It is short, simple and follows RFC 1945 <https://datatracker.ietf.org/doc/html/rfc1945#section-3.7>
inline QString currentVersionIdentifier()
{
    return QStringLiteral("%1/%2").arg(constants::AppName, constants::AppVersionFullStr);
}

} // namespace mediaelch
