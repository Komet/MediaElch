#pragma once

#include "utils/Meta.h"

#include <QDebug>
#include <QString>
#include <ostream>

/// Certification describes how old one has to be to be legally
/// allowed to watch a movie.
class Certification
{
public:
    Certification() = default;
    /// Create a certification from a string. Any input is
    /// allowed. Empty strings are considered invalid and isValid()
    /// will return false.
    ///
    /// \param certification Given certification a string; no trimming
    ///                      or whatsoever is performed.
    explicit Certification(QString certification);

    bool operator==(const Certification& other) const;
    bool operator!=(const Certification& other) const;

    QString toString() const;
    bool isValid() const;

    static const Certification NoCertification;
    static Certification FSK(QString age);

private:
    QString m_certification;
};

inline ELCH_QHASH_RETURN_TYPE qHash(const Certification& cert, uint seed)
{
    return qHash(cert.toString(), seed);
}

std::ostream& operator<<(std::ostream& os, const Certification& id);
QDebug operator<<(QDebug debug, const Certification& id);
