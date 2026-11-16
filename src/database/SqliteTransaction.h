#pragma once

#include "log/Log.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

namespace mediaelch {

/// An atomic SQLite update that also works inside a caller's transaction.
class SqliteTransaction
{
public:
    explicit SqliteTransaction(QSqlDatabase db) : m_db{db}
    {
        m_active = execute("SAVEPOINT mediaelch_persistence_update");
    }

    ~SqliteTransaction()
    {
        if (m_active && execute("ROLLBACK TO SAVEPOINT mediaelch_persistence_update")) {
            execute("RELEASE SAVEPOINT mediaelch_persistence_update");
        }
    }

    SqliteTransaction(const SqliteTransaction&) = delete;
    SqliteTransaction& operator=(const SqliteTransaction&) = delete;

    bool isActive() const { return m_active; }

    bool exec(QSqlQuery& query)
    {
        if (!query.exec()) {
            qCWarning(generic) << "[Database] Atomic update failed:" << query.lastError().text();
            return false;
        }
        return true;
    }

    bool commit()
    {
        if (!m_active || !execute("RELEASE SAVEPOINT mediaelch_persistence_update")) {
            return false;
        }
        m_active = false;
        return true;
    }

private:
    bool execute(const QString& sql)
    {
        QSqlQuery query(m_db);
        if (!query.exec(sql)) {
            qCWarning(generic) << "[Database] Transaction failed:" << query.lastError().text();
            return false;
        }
        return true;
    }

    QSqlDatabase m_db;
    bool m_active = false;
};

} // namespace mediaelch
