#include "storage/SharedRepository.h"

#include "storage/dbs/SqliteDatabase.h"

#include <QDir>
#include <QFileInfo>
#include <QSqlQuery>

namespace idiomind {
namespace storage {

SharedRepository::SharedRepository(core::AppPaths appPaths)
    : m_appPaths(std::move(appPaths))
{
}

QString SharedRepository::tableName(ReviewCalculatorKind kind)
{
    return QStringLiteral("T%1").arg(static_cast<int>(kind));
}

bool SharedRepository::ensureTables(const QString &tlng) const
{
    const QString shrdb = m_appPaths.sharedDb(tlng);
    QDir().mkpath(QFileInfo(shrdb).absolutePath());
    SqliteDatabase db(shrdb);
    if (!db.open())
        return false;
    for (int n = 1; n <= 10; ++n) {
        db.execute(QStringLiteral("create table if not exists T%1 (list TEXT);").arg(n));
    }
    return true;
}

bool SharedRepository::insertTopic(const QString &tlng,
                                   ReviewCalculatorKind kind,
                                   const QString &topic) const
{
    if (kind == ReviewCalculatorKind::None)
        return false;
    const QString shrdb = m_appPaths.sharedDb(tlng);
    SqliteDatabase db(shrdb);
    if (!db.open())
        return false;
    const QString table = tableName(kind);
    return db.execute(QStringLiteral("insert into %1 (list) values ('%2');")
                          .arg(table, topic));
}

bool SharedRepository::removeTopic(const QString &tlng,
                                   ReviewCalculatorKind kind,
                                   const QString &topic) const
{
    if (kind == ReviewCalculatorKind::None)
        return false;
    const QString shrdb = m_appPaths.sharedDb(tlng);
    SqliteDatabase db(shrdb);
    if (!db.open())
        return false;
    const QString table = tableName(kind);
    return db.execute(QStringLiteral("delete from %1 where list='%2';")
                          .arg(table, topic));
}

bool SharedRepository::removeAllLists(const QString &tlng, const QString &topic) const
{
    const QString shrdb = m_appPaths.sharedDb(tlng);
    SqliteDatabase db(shrdb);
    if (!db.open())
        return false;
    for (int n = 1; n <= 10; ++n) {
        db.execute(QStringLiteral("delete from T%1 where list='%2';")
                       .arg(QString::number(n), topic));
    }
    return true;
}

QStringList SharedRepository::listOf(const QString &tlng,
                                     ReviewCalculatorKind kind) const
{
    if (kind == ReviewCalculatorKind::None)
        return {};
    const QString shrdb = m_appPaths.sharedDb(tlng);
    SqliteDatabase db(shrdb);
    if (!db.open())
        return {};
    const QString table = tableName(kind);
    QSqlQuery q = db.query(QStringLiteral("select * from %1;").arg(table));
    QStringList result;
    while (q.next())
        result.append(q.value(0).toString());
    return result;
}

QStringList SharedRepository::listOfNumber(const QString &tlng, int number) const
{
    if (number < 1 || number > 10)
        return {};
    const QString shrdb = m_appPaths.sharedDb(tlng);
    SqliteDatabase db(shrdb);
    if (!db.open())
        return {};
    QSqlQuery q = db.query(QStringLiteral("select list from T%1;").arg(number));
    QStringList result;
    while (q.isActive() && q.next())
        result.append(q.value(0).toString());
    return result;
}

bool SharedRepository::contains(const QString &tlng,
                                ReviewCalculatorKind kind,
                                const QString &topic) const
{
    if (kind == ReviewCalculatorKind::None)
        return false;
    const QString shrdb = m_appPaths.sharedDb(tlng);
    SqliteDatabase db(shrdb);
    if (!db.open())
        return false;
    const QString table = tableName(kind);
    QSqlQuery q = db.query(QStringLiteral("select list from %1 where list='%2';")
                               .arg(table, topic));
    return q.next();
}

} // namespace storage
} // namespace idiomind
