#pragma once

#include <QDebug>

namespace mediaelch {

/// \brief MediaElch Database ID
/// \todo Make constructor explicit.
class DatabaseId
{
public:
    DatabaseId() : id{-1} {}
    /*implicit*/ DatabaseId(int dbId) : id{dbId} {}

    int toInt() const { return id; }
    bool isValid() const { return id > -1; }

public:
    int id{-1};
};

} // namespace mediaelch

QDebug operator<<(QDebug dbg, const mediaelch::DatabaseId& db);
