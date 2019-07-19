#pragma once

#include <QDebug>
#include <QString>
#include <ostream>

class Certification
{
public:
    Certification() = default;
    explicit Certification(QString certification);
    explicit Certification(int certification);

    bool operator==(const Certification& other) const;
    bool operator!=(const Certification& other) const;

    QString toString() const;
    bool isValid() const;

    static const Certification NoCertification;
    static Certification FSK(QString age);

private:
    QString m_certification;
};

std::ostream& operator<<(std::ostream& os, const Certification& id);
QDebug operator<<(QDebug debug, const Certification& id);
