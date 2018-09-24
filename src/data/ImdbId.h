#ifndef IMDBID_H
#define IMDBID_H

#include <QString>

class ImdbId
{
public:
    ImdbId() = default;
    explicit ImdbId(QString imdbId);

    bool operator==(const ImdbId &other);
    bool operator!=(const ImdbId &other);

    QString toString() const;
    bool isValid() const;
    static bool isValidFormat(const QString &imdbId);

    static const ImdbId NoId;

private:
    // can be replaced by std::optional<T> in C++17
    QString m_imdbId;
    bool m_isValid = false;
};

#endif // IMDBID_H
