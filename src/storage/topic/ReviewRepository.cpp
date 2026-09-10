#include "storage/topic/ReviewRepository.h"

#include "storage/dbs/SqliteDatabase.h"

#include <QSqlQuery>
#include <QSqlRecord>
#include <QStringList>

namespace idiomind {
namespace storage {

namespace {

QString quote(const QString &v)
{
    return QString(v).replace(QLatin1Char('\''), QStringLiteral("''"));
}

} // namespace

ReviewRepository::ReviewRepository(core::AppPaths appPaths)
    : m_appPaths(std::move(appPaths))
{
}

Review ReviewRepository::load(const QString &tlng, const QString &topic) const
{
    Review review;
    const QString dbPath = m_appPaths.topicDb(tlng, topic);
    SqliteDatabase db(dbPath);
    if (!db.open())
        return review;
    QSqlQuery q = db.query(QStringLiteral("select * from reviews;"));
    if (q.isActive() && q.next()) {
        for (int i = 0; i < Review::MaxDates; ++i) {
            QString s;
            const QSqlRecord rec = q.record();
            if (i < q.record().count()) {
                const QVariant v = q.value(i);
                if (!v.isNull())
                    s = v.toString();
            }
            if (!s.isEmpty())
                review.setSqliteDate(i + 1, s);
        }
    }
    return review;
}

bool ReviewRepository::store(const QString &tlng, const QString &topic,
                             const Review &review) const
{
    const QString dbPath = m_appPaths.topicDb(tlng, topic);
    SqliteDatabase db(dbPath);
    if (!db.open())
        return false;
    // Ensure row exists
    if (!rowExists(dbPath)) {
        if (!db.execute(QStringLiteral("insert into reviews (date1) values ('');")))
            return false;
    }
    // Build single UPDATE with all columns
    QStringList parts;
    for (int i = 1; i <= Review::MaxDates; ++i) {
        const QString v = review.sqliteDate(i);
        parts.append(QStringLiteral("date%1='%2'").arg(QString::number(i), quote(v)));
    }
    return db.execute(QStringLiteral("update reviews set %1;").arg(parts.join(QLatin1Char(','))));
}

bool ReviewRepository::clear(const QString &tlng, const QString &topic) const
{
    const QString dbPath = m_appPaths.topicDb(tlng, topic);
    SqliteDatabase db(dbPath);
    if (!db.open())
        return false;
    if (!db.execute(QStringLiteral("delete from reviews;")))
        return false;
    return db.execute(QStringLiteral("insert into reviews (date1) values ('');"));
}

bool ReviewRepository::backfillDate1IfEmpty(const QString &tlng, const QString &topic) const
{
    const QString dbPath = m_appPaths.topicDb(tlng, topic);
    SqliteDatabase db(dbPath);
    if (!db.open())
        return false;
    const QString d1 = db.value(QStringLiteral("select date1 from reviews;"));
    if (!d1.isEmpty())
        return true; // ya tiene fecha (nada que rellenar)
    if (!writeColumn(dbPath, 1, QDate::currentDate().toString(QStringLiteral("MM/dd/yyyy"))))
        return false;
    return true;
}

int ReviewRepository::countFilled(const QString &tlng, const QString &topic) const
{
    return filledColumns(m_appPaths.topicDb(tlng, topic));
}

int ReviewRepository::filledColumns(const QString &dbPath)
{
    SqliteDatabase db(dbPath);
    if (!db.open())
        return 0;
    QSqlQuery q = db.query(QStringLiteral("select * from reviews;"));
    int count = 0;
    if (q.isActive() && q.next()) {
        for (int i = 0; i < q.record().count(); ++i) {
            const QVariant v = q.value(i);
            if (!v.isNull() && !v.toString().trimmed().isEmpty())
                ++count;
        }
    }
    return count;
}

bool ReviewRepository::writeColumn(const QString &dbPath, int column, const QString &value)
{
    SqliteDatabase db(dbPath);
    if (!db.open())
        return false;
    return db.execute(QStringLiteral("update reviews set date%1='%2';")
                          .arg(column).arg(quote(value)));
}

bool ReviewRepository::rowExists(const QString &dbPath)
{
    SqliteDatabase db(dbPath);
    if (!db.open())
        return false;
    QSqlQuery q = db.query(QStringLiteral("select count(*) from reviews;"));
    if (!(q.isActive() && q.next()))
        return false;
    return q.value(0).toInt() == 1;
}

} // namespace storage
} // namespace idiomind