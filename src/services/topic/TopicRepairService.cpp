#include "services/topic/TopicRepairService.h"

#include "storage/dbs/SqliteDatabase.h"
#include "storage/topic/TopicDataRepository.h"
#include "storage/topic/TopicRepository.h"

#include "core/topic/FlatItemCodec.h"

#include <QDate>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QStringList>
#include <QVariant>

using namespace ::idiomind::core;
using namespace ::idiomind::core::FlatItemCodec;

namespace idiomind {
namespace services {

using storage::SqliteDatabase;
using storage::TopicDataRepository;
using storage::TopicRepository;

namespace {

// Elección del valor de `r` tales como `file` (mágica SQLite) -> datos.
bool isSqliteFile(const QString &path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly))
        return false;
    const QByteArray magic = f.read(16);
    f.close();
    return magic == QByteArrayLiteral("SQLite format 3\0");
}

QString firstLineOf(const QString &path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return QString();
    const QString line = QString::fromUtf8(f.readLine()).trimmed();
    f.close();
    return line;
}

QString readAllOf(const QString &path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return QString();
    const QString out = QString::fromUtf8(f.readAll());
    f.close();
    return out;
}

QStringList readLinesOf(const QString &path)
{
    QStringList out;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return out;
    while (!f.atEnd()) {
        QByteArray raw = f.readLine();
        while (raw.endsWith('\n') || raw.endsWith('\r'))
            raw.chop(1);
        out.append(QString::fromUtf8(raw));
    }
    return out;
}

void ensureFile(const QString &path)
{
    if (!QFileInfo::exists(path)) {
        QFile f(path);
        if (f.open(QIODevice::WriteOnly))
            f.close();
    }
}

bool isNumeric(const QString &s)
{
    static const QRegularExpression re(QStringLiteral("^[0-9]+$"));
    return re.match(s).hasMatch();
}

// _check / _restore: `echo "$stts" |grep -E '3|4|7|8|9|10'` -> SUBSTRING, no
// conjunto: stts que contengan cualquiera de esos dígitos o "10".
bool reviewStateMatch(const QString &stts)
{
    const QStringList needles{
        QStringLiteral("3"), QStringLiteral("4"), QStringLiteral("7"),
        QStringLiteral("8"), QStringLiteral("9"), QStringLiteral("10")};
    for (const QString &n : needles)
        if (stts.contains(n))
            return true;
    return false;
}

// tpc_db 5: `select *` -> tr '|' '\n' -> filas NO VACÍAS de una tabla (los
// listados del original usan tablas de una sola columna).
QStringList tableValues(const QString &dbPath, const QString &table)
{
    QStringList out;
    SqliteDatabase db(dbPath);
    if (!db.open())
        return out;
    QSqlQuery q = db.query(QStringLiteral("select * from '%1';").arg(table));
    while (q.isActive() && q.next()) {
        const QString v = q.value(0).toString();
        if (!v.isEmpty())
            out.append(v);
    }
    return out;
}

int countTableRows(const QString &dbPath, const QString &table)
{
    SqliteDatabase db(dbPath);
    if (!db.open())
        return -1;
    QSqlQuery q = db.query(QStringLiteral("select Count(*) from '%1';").arg(table));
    if (!(q.isActive() && q.next()))
        return -1;
    return q.value(0).toInt();
}

// `echo ${stts} > stts` / `echo 1 > stts` (valor + \n).
bool writeSttsFile(const QString &sttsPath, const QString &value)
{
    QFile f(sttsPath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;
    f.write(value.toUtf8());
    f.write("\n");
    f.close();
    return true;
}

} // namespace

TopicRepairService::TopicRepairService(core::AppPaths appPaths)
    : m_appPaths(std::move(appPaths))
{
}

namespace {

// Análogo del python de _restore `(fields[n].split(prefix))[1].split('}')[0]`.
// Devuelve false si la extracción es imposible (IndexError del original).
// Nota: cada fields[i] es `name{value` (el '}' es el separador entre
// segmentos), por lo que `rest.index('}')` suele ser -1 y el valor es "rest".
bool segmentValue(const QStringList &fields, int idx, const QString &prefix,
                  QString *out)
{
    if (idx >= fields.size())
        return false;
    const QString &f = fields.at(idx);
    const int p = f.indexOf(prefix);
    if (p < 0)
        return false;
    const QString rest = f.mid(p + prefix.size());
    const int close = rest.indexOf(QLatin1Char('}'));
    *out = close < 0 ? rest : rest.left(close);
    return true;
}

} // namespace

// ---------------------------------------------------------------------------
// _restore completa: data desde backup si falta, rebuild de listas+data con la
// semántica python del original (13/24, >200 truncado, crash => parcial).
// ---------------------------------------------------------------------------
bool TopicRepairService::restoreTopic(const QString &tlng, const QString &topic,
                                      const QString &stts, CheckReport &rep)
{
    const QString conf = m_appPaths.topicConfDir(tlng, topic);
    const QString dataPath = conf + QStringLiteral("/data");
    const QString dbPath = m_appPaths.topicDb(tlng, topic);

    // 1) data ausente -> de $DM/backup/<topic>.bk (sección ---- newest/oldest).
    if (!QFileInfo::exists(dataPath)) {
        const QString bk =
            m_appPaths.backupDir() + QLatin1Char('/') + topic + QStringLiteral(".bk");
        if (!QFileInfo::exists(bk))
            return false; // msg dialog-error + exit 1 en el original
        const QStringList lines = readLinesOf(bk);
        // sed -n '/----- newest/,/----- oldest/p' -> mantiene las marcas
        int start = -1, end = -1;
        for (int i = 0; i < lines.size(); ++i) {
            if (lines.at(i).contains(QStringLiteral("----- newest")) && start < 0)
                start = i;
            if (lines.at(i).contains(QStringLiteral("----- oldest")) && start >= 0) {
                end = i;
                break;
            }
        }
        QStringList section;
        if (start >= 0) {
            section = (end >= 0) ? lines.mid(start, end - start + 1)
                                 : lines.mid(start);
        }
        QStringList kept;
        for (const QString &l : section) {
            if (l.contains(QStringLiteral("----- newest"))
                || l.contains(QStringLiteral("----- oldest")))
                continue; // grep -Pv '\----- newest|oldest'
            kept.append(l);
            if (kept.size() >= 200)
                break; // head -n200
        }
        QFile data(dataPath);
        if (!data.open(QIODevice::WriteOnly | QIODevice::Text))
            return false;
        for (const QString &l : kept) {
            data.write(l.toUtf8());
            data.write("\n");
        }
        data.close();
        rep.dataRestored = true;
    }

    // 2) `sed -i "/trgt{}srce{}/d" data`
    {
        QStringList kept;
        for (const QString &l : readLinesOf(dataPath))
            if (!l.contains(QStringLiteral("trgt{}srce{}")))
                kept.append(l);
        QFile data(dataPath);
        if (!data.open(QIODevice::WriteOnly | QIODevice::Text))
            return false;
        for (const QString &l : kept) {
            data.write(l.toUtf8());
            data.write("\n");
        }
        data.close();
    }

    // 3) tpc_db 6 delete-all (sentences words learning learnt marks)
    for (const QString &ta : {QStringLiteral("sentences"), QStringLiteral("words"),
                              QStringLiteral("learning"), QStringLiteral("learnt"),
                              QStringLiteral("marks")}) {
        SqliteDatabase db(dbPath);
        if (db.open())
            db.execute(QStringLiteral("delete from '%1';").arg(ta));
    }
    // `echo -n "pragma foreign_keys=ON" |sqlite3` (sin efecto en Qt: sin FKs)
    {
        SqliteDatabase db(dbPath);
        if (db.open())
            db.execute(QStringLiteral("pragma foreign_keys=ON;"));
    }

    // 4) python-equivalente: reconstruye listas + datatmp con 13/24 y crash.
    const bool learned = reviewStateMatch(stts);
    QDir().mkpath(m_appPaths.tmpDir());
    const QString datatmpPath = m_appPaths.tmpDir() + QStringLiteral("/data");
    QStringList datatmp;
    bool crash = false;
    struct Pending {
        QString table;
        QString value;
    };
    QList<Pending> pending;

    int processed = 0;
    for (const QString &raw : readLinesOf(dataPath)) {
        if (processed >= 200)
            break; // if count > 200: break  (más de 200 se descartan)
        const QString line = raw.trimmed();
        if (!line.contains(QStringLiteral("trgt{"))) {
            crash = true; // fields[0].split('trgt{')[1] -> IndexError
            break;
        }
        const FlatItemCodec::DecodedItem d = FlatItemCodec::decodeWithSegments(line);
        QStringList fields;
        fields.reserve(d.segments.size());
        for (const auto &s : d.segments)
            fields.append(s.first + QLatin1Char('{') + s.second);

        QString typee, mark;
        if (!segmentValue(fields, 13, QStringLiteral("type{"), &typee)
            || !segmentValue(fields, 8, QStringLiteral("mark{"), &mark)) {
            if (!segmentValue(fields, 24, QStringLiteral("type{"), &typee)
                || !segmentValue(fields, 18, QStringLiteral("mark{"), &mark)) {
                crash = true; // IndexError en ambos branches
                break;
            }
        }
        QString trgt;
        if (!segmentValue(fields, 0, QStringLiteral("trgt{"), &trgt)) {
            crash = true;
            break;
        }

        if (typee == QLatin1String("1"))
            pending.append({QStringLiteral("words"), trgt});
        else
            pending.append({QStringLiteral("sentences"), trgt});
        if (mark == QLatin1String("TRUE"))
            pending.append({QStringLiteral("marks"), trgt});
        pending.append({learned ? QStringLiteral("learnt")
                                : QStringLiteral("learning"),
                        trgt});
        datatmp.append(line);
        ++processed;
    }

    // 5) las inserciones sólo si el python llega a `db.commit()`.
    if (!crash) {
        for (const Pending &p : pending) {
            SqliteDatabase db(dbPath);
            if (!db.open())
                continue;
            const QString safe =
                QString(p.value).replace(QLatin1Char('\''), QStringLiteral("''"));
            db.execute(QStringLiteral("insert into '%1' (list) values ('%2');")
                           .arg(p.table, safe));
        }
    }

    // 6) `mv -f $DT/data data` (SIEMPRE, incluso con crash parcial) + /^$/d.
    {
        QFile tmp(datatmpPath);
        if (tmp.open(QIODevice::WriteOnly | QIODevice::Text)) {
            for (const QString &l : datatmp) {
                tmp.write(l.toUtf8());
                tmp.write("\n");
            }
            tmp.close();
        }
        QFile::remove(dataPath);
        QFile::rename(datatmpPath, dataPath);
    }
    {
        QStringList kept;
        for (const QString &l : readLinesOf(dataPath))
            if (!l.isEmpty() && !l.trimmed().isEmpty())
                kept.append(l);
        QFile data(dataPath);
        if (data.open(QIODevice::WriteOnly | QIODevice::Text)) {
            for (const QString &l : kept) {
                data.write(l.toUtf8());
                data.write("\n");
            }
            data.close();
        }
    }
    rep.dataRestored = true;
    return true;
}

TopicRepairService::CheckReport TopicRepairService::checkIndex(
    const QString &tlng, const QString &topic, bool quiet)
{
    Q_UNUSED(quiet);
    CheckReport rep;
    const QString conf = m_appPaths.topicConfDir(tlng, topic);
    const QString dataDir = m_appPaths.topicDataDir(tlng, topic);
    const QString dbPath = m_appPaths.topicDb(tlng, topic);
    const QString sttsPath = conf + QStringLiteral("/stts");
    const QString dataPath = conf + QStringLiteral("/data");

    // check_dir / check_file: dirs + log1..3 y note vacíos si faltan.
    QDir().mkpath(dataDir);
    QDir().mkpath(dataDir + QStringLiteral("/images"));
    QDir().mkpath(conf);
    QDir().mkpath(conf + QStringLiteral("/practice"));
    ensureFile(conf + QStringLiteral("/practice/log1"));
    ensureFile(conf + QStringLiteral("/practice/log2"));
    ensureFile(conf + QStringLiteral("/practice/log3"));
    ensureFile(conf + QStringLiteral("/note"));

    // mp3 de tamaño 0 en el directorio del topic (sólo nivel superior).
    const QFileInfoList mp3s =
        QDir(dataDir).entryInfoList({QStringLiteral("*.mp3")}, QDir::Files);
    for (const QFileInfo &fi : mp3s)
        if (fi.size() == 0)
            QFile::remove(fi.absoluteFilePath());

    bool mkmn = false, fix = false;
    bool dbConfig = false, dbReviews = false, dbId = false;

    // stts + data.
    if (!QFileInfo::exists(sttsPath)) {
        writeSttsFile(sttsPath, QStringLiteral("1"));
        fix = true;
    }
    if (!QFileInfo::exists(dataPath))
        fix = true;
    QString stts = firstLineOf(sttsPath);
    if (!isNumeric(stts))
        stts = QStringLiteral("13");

    if (stts == QLatin1String("13")) {
        const QString bk = conf + QStringLiteral("/stts.bk");
        if (QFileInfo::exists(bk)) {
            stts = readAllOf(bk).trimmed();
            QFile::remove(bk);
        } else {
            stts = QStringLiteral("1");
        }
        if (!isNumeric(stts))
            stts = QStringLiteral("1");
        writeSttsFile(sttsPath, stts);
        mkmn = true;
        fix = true;
    }

    // DB check: `file` detecta SQLite; si no -> mkdb.sh tpc (recrear).
    if (!isSqliteFile(dbPath)) {
        TopicRepository(m_appPaths).recreateTopicDatabase(tlng, topic);
        fix = true;
    } else {
        if (countTableRows(dbPath, QStringLiteral("reviews")) != 1)
            dbReviews = true;
        if (countTableRows(dbPath, QStringLiteral("id")) != 1)
            dbId = true;
        if (countTableRows(dbPath, QStringLiteral("config")) != 1)
            dbConfig = true;
    }

    // stts > 1 y date1 vacía -> date1 = hoy (%.m/%d/%Y).
    if (isNumeric(stts) && stts.toInt() > 1) {
        SqliteDatabase db(dbPath);
        if (db.open()) {
            QSqlQuery q = db.query(QStringLiteral("select date1 from reviews;"));
            bool anyFilled = false;
            while (q.isActive() && q.next()) {
                const QVariant v = q.value(0);
                if (!v.isNull() && !v.toString().isEmpty()) {
                    anyFilled = true;
                    break;
                }
            }
            if (!anyFilled) {
                db.execute(
                    QStringLiteral("update reviews set date1='%1';")
                        .arg(QDate::currentDate().toString(QStringLiteral("MM/dd/yyyy"))));
            }
        }
    }

    // 0.cfg / id.cfg -> newform=1 (sólo marcador, sin acción).
    // (no operación: basta con detectarlo; se deja documentado)

    // `grep -o 'trgt{}srce{}' data` -> fix.
    if (QFileInfo::exists(dataPath)) {
        for (const QString &l : readLinesOf(dataPath)) {
            if (l.contains(QStringLiteral("trgt{}srce{}"))) {
                fix = true;
                break;
            }
        }
    }

    // counts: cnt0, index0, cnt1..cnt4.
    const QStringList dataLines =
        QFileInfo::exists(dataPath) ? readLinesOf(dataPath) : QStringList();
    int cnt0 = 0;
    for (const QString &l : dataLines)
        if (!l.trimmed().isEmpty())
            ++cnt0;
    int index0 = 0;
    if (QFileInfo::exists(conf + QStringLiteral("/index"))) {
        int nonEmpty = 0;
        for (const QString &l : readLinesOf(conf + QStringLiteral("/index")))
            if (!l.trimmed().isEmpty())
                ++nonEmpty;
        index0 = nonEmpty / 3;
    }
    const int cnt1 = tableValues(dbPath, QStringLiteral("learning")).size();
    const int cnt2 = tableValues(dbPath, QStringLiteral("learnt")).size();
    const int cnt3 = tableValues(dbPath, QStringLiteral("words")).size();
    const int cnt4 = tableValues(dbPath, QStringLiteral("sentences")).size();

    if (cnt3 + cnt4 != cnt0)
        fix = true;
    if (cnt1 + cnt2 != cnt0)
        fix = true;
    // tls.sh:165 `if [ $? != 0 ]` es MUERTO (no-op): no se reproduce.
    if (index0 != cnt1)
        fix = true;
    if (reviewStateMatch(stts)) {
        if (cnt0 == 0)
            writeSttsFile(sttsPath, QStringLiteral("1"));
        mkmn = true;
    }

    rep.fixed = fix;
    rep.dataCount = cnt0;
    rep.indexCount = index0;

    if (fix) {
        if (!restoreTopic(tlng, topic, stts, rep)) {
            rep.errored = true;
            return rep; // original: exit 1
        }
        mkmn = true;
    }

    // db fixes (independientes de fix; reproducen los flags de _check).
    if (dbConfig) {
        SqliteDatabase db(dbPath);
        if (db.open() && db.execute(QStringLiteral("delete from config;"))
            && db.execute(QStringLiteral(
                "insert into config (words,sntcs,marks,learn,diffi,rplay,"
                "audio,ntosd,loop,rword,acheck,repass) "
                "values ('TRUE','TRUE','FALSE','FALSE','FALSE','FALSE','FALSE',"
                "'FALSE','FALSE','FALSE','TRUE','0');"))) {
            rep.dbConfigFixed = true;
        }
    }

    if (dbReviews) {
        // firstrec = select * | tr '|' '\n' -> valores de la PRIMERA fila.
        QStringList rec;
        {
            SqliteDatabase db(dbPath);
            if (db.open()) {
                QSqlQuery q = db.query(QStringLiteral("select * from reviews;"));
                if (q.isActive() && q.next()) {
                    for (int i = 0; i < q.record().count(); ++i) {
                        const QVariant v = q.value(i);
                        rec.append(v.isNull() ? QString() : v.toString());
                    }
                }
            }
        }
        SqliteDatabase db(dbPath);
        db.open();
        db.execute(QStringLiteral("delete from reviews;"));
        for (int i = 1; i <= 8; ++i) {
            const QString val = rec.value(i - 1);
            if (i == 1) {
                // insert (date1) values ('<val>') — raw, sin escape (original)
                db.execute(QStringLiteral("insert into reviews (date1) values ('%1');")
                               .arg(val));
            } else {
                db.execute(QStringLiteral("update reviews set date%1='%2';")
                               .arg(i).arg(val));
            }
        }
        rep.dbReviewsFixed = true;
    }

    if (dbId) {
        // tsets del repair (las 18 columnas que BARREN los i=1..18 de tls.sh:
        // la última 'info' no existe en la tabla id => error ignorado, como en
        // la CLI sqlite3; la columna stts NO se restaura — quirk del original).
        const QStringList tsets{QStringLiteral("name"), QStringLiteral("slng"),
            QStringLiteral("tlng"), QStringLiteral("autr"), QStringLiteral("cntt"),
            QStringLiteral("ctgy"), QStringLiteral("ilnk"), QStringLiteral("orig"),
            QStringLiteral("dtec"), QStringLiteral("dteu"), QStringLiteral("dtei"),
            QStringLiteral("nwrd"), QStringLiteral("nsnt"), QStringLiteral("nimg"),
            QStringLiteral("naud"), QStringLiteral("nsze"), QStringLiteral("levl"),
            QStringLiteral("info")};
        QStringList rec;
        {
            SqliteDatabase db(dbPath);
            if (db.open()) {
                QSqlQuery q = db.query(QStringLiteral("select * from id;"));
                if (q.isActive() && q.next()) {
                    for (int i = 0; i < q.record().count(); ++i) {
                        const QVariant v = q.value(i);
                        rec.append(v.isNull() ? QString() : v.toString());
                    }
                }
            }
        }
        SqliteDatabase db(dbPath);
        db.open();
        db.execute(QStringLiteral("delete from id;"));
        for (int i = 1; i <= 18; ++i) {
            const QString col = tsets.value(i - 1);
            const QString val = rec.value(i - 1);
            if (col == QLatin1String("name")) {
                db.execute(QStringLiteral("insert into id (name) values ('%1');")
                               .arg(val));
            } else {
                db.execute(QStringLiteral("update id set %1='%2';").arg(col, val));
            }
        }
        rep.dbIdFixed = true;
    }

    // `if mkmn=1: colorize 1` — con $2 vacío: rebuildIndex(chkFlag=false).
    if (mkmn) {
        TopicDataRepository repo(m_appPaths);
        if (repo.rebuildIndex(tlng, topic, false))
            rep.indexRebuilt = true;
    }

    return rep;
}

} // namespace services
} // namespace idiomind