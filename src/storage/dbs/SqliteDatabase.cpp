#include "storage/dbs/SqliteDatabase.h"

#include <QAtomicInt>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QVariant>

namespace idiomind {
namespace storage {

namespace {
QString nextConnectionName()
{
    static QAtomicInt counter{0};
    const int id = counter.fetchAndAddRelaxed(1);
    return QStringLiteral("idiomind_sqlite_%1").arg(id);
}
} // namespace

SqliteDatabase::SqliteDatabase(QString path)
    : m_path(std::move(path))
    , m_connectionName(nextConnectionName())
{
}

SqliteDatabase::~SqliteDatabase()
{
    close();
}

bool SqliteDatabase::open()
{
    if (m_isOpen)
        return true;

    QString driver = QSqlDatabase::drivers().contains(QStringLiteral("QSQLITE"))
        ? QStringLiteral("QSQLITE")
        : QString();
    if (driver.isEmpty()) {
        qWarning("SqliteDatabase: no QSQLITE driver available");
        return false;
    }

    QSqlDatabase db = QSqlDatabase::addDatabase(driver, m_connectionName);
    db.setDatabaseName(m_path);
    if (!db.open()) {
        qWarning("SqliteDatabase: cannot open %s: %s", qPrintable(m_path),
                 qPrintable(db.lastError().text()));
        QSqlDatabase::removeDatabase(m_connectionName);
        return false;
    }

    // Compat: journal por defecto (el original no activó WAL nunca).
    QSqlQuery pragma(db);
    pragma.exec(QStringLiteral("pragma busy_timeout=2000"));
    m_isOpen = true;
    return true;
}

void SqliteDatabase::close()
{
    if (!m_isOpen)
        return;
    {
        QSqlDatabase db = QSqlDatabase::database(m_connectionName, false);
        if (db.isValid())
            db.close();
    }
    QSqlDatabase::removeDatabase(m_connectionName);
    m_isOpen = false;
}

bool SqliteDatabase::execute(const QString &sql)
{
    if (!open())
        return false;
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    if (!query.exec(sql)) {
        qWarning("SqliteDatabase: exec failed: %s\n  SQL: %s",
                 qPrintable(query.lastError().text()), qPrintable(sql));
        return false;
    }
    return true;
}

QSqlQuery SqliteDatabase::query(const QString &sql)
{
    if (!open())
        return QSqlQuery();
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.exec(sql);
    return query;
}

QString SqliteDatabase::value(const QString &sql)
{
    QSqlQuery query = this->query(sql);
    if (query.isActive() && query.next() && !query.value(0).isNull()) {
        const QVariant v = query.value(0);
        return (v.typeId() == QMetaType::Double)
            ? QString::number(v.toDouble())
            : v.toString();
    }
    return QString();
}

} // namespace storage
} // namespace idiomind
