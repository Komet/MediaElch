#include "Certification.h"

#include <QString>
#include <utility>

Certification::Certification(QString certification) : m_certification(std::move(certification))
{
}

const Certification Certification::NoCertification = Certification();

bool Certification::operator==(const Certification& other) const
{
    return m_certification == other.m_certification;
}

bool Certification::operator!=(const Certification& other) const
{
    return !(*this == other);
}

QString Certification::toString() const
{
    return m_certification;
}

bool Certification::isValid() const
{
    return !m_certification.isEmpty();
}

Certification Certification::FSK(QString age)
{
    return Certification(QStringLiteral("FSK %1").arg(age));
}

std::ostream& operator<<(std::ostream& os, const Certification& id)
{
    return os << id.toString().toStdString();
}

QDebug operator<<(QDebug debug, const Certification& id)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "Certification(" << id.toString() << ')';
    return debug;
}
