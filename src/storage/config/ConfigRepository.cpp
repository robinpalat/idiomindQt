#include "storage/config/ConfigRepository.h"

#include "storage/dbs/SqliteDatabase.h"

#include <QDir>
#include <QFile>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QVariant>

namespace idiomind {
namespace storage {

using core::AppPaths;

namespace {
// Escapa comillas simples al estilo SQLite (sqlite3 CLI, como el original).
QString escape(const QString &v)
{
    return QString(v).replace(QLatin1Char('\''), QStringLiteral("''"));
}
// Esquema y filas iniciales EXACTOS de mkdb.sh create_cfgdb. No se altera el
// DDL original (docs: "no schema migration"; tolerar tablas legadas). El
// driver SQLITE de Qt no admite multi-statement en un único exec: se ejecuta
// una sentencia por entrada.
const char *kCreateCfgdbSql[] = {
    "create table if not exists opts (gramr TEXT, trans TEXT, dlaud TEXT, "
    "ttrgt TEXT, itray TEXT, swind TEXT, stsks TEXT, intrf TEXT, synth TEXT, "
    "txaud TEXT, tlang TEXT, level TEXT, slang TEXT);",
    "create table if not exists lang (tlng TEXT, slng TEXT);",
    "create table if not exists geom (vals TEXT);",
    "create table if not exists sess (date TEXT);",
    "create table if not exists updt (date TEXT, ignr TEXT);",
};

const char *kOptsKeys[] = {
    "gramr", "trans", "dlaud", "ttrgt", "itray", "swind", "stsks",
    "intrf", "synth", "txaud", "tlang", "level", "slang",
};
} // namespace

ConfigRepository::ConfigRepository(AppPaths appPaths)
    : m_appPaths(std::move(appPaths))
{
}

SqliteDatabase *ConfigRepository::db() const
{
    if (m_db == nullptr) {
        m_db = new SqliteDatabase(m_appPaths.configDb());
        m_db->open();
    }
    return m_db;
}

bool ConfigRepository::ensureProfile()
{
    const QString cfgdb = m_appPaths.configDb();

    const bool exists = QFileInfo::exists(cfgdb);
    bool validSqlite = false;
    if (exists) {
        QFile f(cfgdb);
        if (f.open(QIODevice::ReadOnly)) {
            validSqlite = f.read(16).contains("SQLite");
            f.close();
        }
    }
    if (exists && validSqlite) {
        return db()->isOpen();
    }

    // cnfg.sh: [ ! -f ] o no-SQLite -> `mkdb.sh config`. El fichero corrupto
    // se ELIMINA primero (sqlite reportaría "file is not a database" sobre
    // contenido basura; recrear requiere DM limpio, igual que mkdb.sh).
    if (exists)
        QFile::remove(cfgdb);
    QDir().mkpath(m_appPaths.configRoot());
    SqliteDatabase fresh(cfgdb);
    if (!fresh.open())
        return false;

    for (const char *stmnt : kCreateCfgdbSql) {
        if (!fresh.execute(QLatin1String(stmnt)))
            return false;
    }

    // create_cfgdb inserta 1 fila en opts/lang/geom/sess/updt con los valores
    // por defecto del original (`mkdb.sh config "<tlng>"`; $2='' en c.conf,
    // el idioma real se asigna en el diálogo de preferencias / 1u).
    QSqlQuery q(fresh.query(QStringLiteral(
        "insert into opts (gramr,trans,dlaud,ttrgt,itray,swind,stsks,"
        "intrf,synth,txaud,tlang,level,slang) "
        "values ('TRUE','TRUE','TRUE','FALSE','FALSE','FALSE','TRUE',"
        "'default','','','','','');")));
    if (q.lastError().isValid())
        return false;
    if (!fresh.execute(
            QStringLiteral("insert into lang (tlng,slng) values ('','');")))
        return false;
    if (!fresh.execute(QStringLiteral("insert into geom (vals) values ('');")))
        return false;
    if (!fresh.execute(QStringLiteral("insert into sess (date) values ('');")))
        return false;
    if (!fresh.execute(QStringLiteral("insert into updt (date) values ('');")))
        return false;

    fresh.close();
    return db()->open();
}

QString ConfigRepository::targetLanguage() const
{
    return db()->value(QStringLiteral("select tlng from lang;"));
}

QString ConfigRepository::sourceLanguage() const
{
    return db()->value(QStringLiteral("select slng from lang;"));
}

bool ConfigRepository::setTargetLanguage(const QString &tlng)
{
    return db()->execute(QStringLiteral("update lang set tlng='%1';")
                             .arg(escape(tlng)));
}

bool ConfigRepository::setSourceLanguage(const QString &slng)
{
    return db()->execute(QStringLiteral("update lang set slng='%1';")
                             .arg(escape(slng)));
}

bool ConfigRepository::setLanguages(const QString &tlng, const QString &slng)
{
    return setTargetLanguage(tlng) && setSourceLanguage(slng);
}

QVariantMap ConfigRepository::options() const
{
    QVariantMap map;
    if (!db()->isOpen())
        return map;
    QSqlQuery query = db()->query(QStringLiteral("select * from opts;"));
    if (!query.isActive() || !query.next())
        return map;

    const QSqlRecord record = query.record();
    for (int i = 0; i < record.count(); ++i) {
        const QVariant v = query.value(i);
        map.insert(record.fieldName(i), v.isNull() ? QString() : v.toString());
    }
    return map;
}

QString ConfigRepository::option(const QString &key) const
{
    return db()->value(QStringLiteral("select %1 from opts;").arg(key));
}

bool ConfigRepository::setOption(const QString &key, const QString &value)
{
    return db()->execute(QStringLiteral("update opts set %1='%2';")
                             .arg(key, escape(value)));
}

bool ConfigRepository::setOptions(const QVariantMap &values)
{
    if (values.isEmpty())
        return true;
    QStringList assignments;
    for (auto it = values.constBegin(); it != values.constEnd(); ++it)
        assignments << (it.key() + QLatin1String("='") + escape(it.value().toString()) +
                        QLatin1Char('\''));
    return db()->execute(QStringLiteral("update opts set %1;")
                             .arg(assignments.join(QLatin1String(", "))));
}

QString ConfigRepository::activeTopic() const
{
    // Original: `tpc="$(sed -n 1p "$HOME/.config/idiomind/tpc")"` (c.conf).
    QFile f(m_appPaths.configRoot() + QStringLiteral("/tpc"));
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return QString();
    const QString first = f.readLine().trimmed();
    return first;
}

bool ConfigRepository::setActiveTopic(const QString &topic)
{
    QDir().mkpath(m_appPaths.configRoot());
    QFile f(m_appPaths.configRoot() + QStringLiteral("/tpc"));
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;
    f.write(topic.toUtf8());
    if (!topic.endsWith(QLatin1Char('\n')))
        f.write("\n");
    f.close();
    return f.error() == QFileDevice::NoError;
}

} // namespace storage
} // namespace idiomind