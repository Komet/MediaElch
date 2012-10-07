#ifndef HELPER_H
#define HELPER_H

#include <QString>

/**
 * @brief Some convenience functions are bundled here
 */
class Helper
{
public:
    static QString toLatin1PercentEncoding(QString str);
    static QString formatTrailerUrl(QString url);
    static bool isDvd(QString path);
    static bool isBluRay(QString path);
};

#endif // HELPER_H
