#include "storage/topic/TopicRepository.h"

#include "storage/config/ConfigRepository.h"
#include "storage/dbs/SqliteDatabase.h"

#include "core/topic/FlatItemCodec.h"

#include <QDate>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

using namespace ::idiomind::core;
using namespace ::idiomind::core::FlatItemCodec;

namespace idiomind {
namespace storage {

using core::Item;
using core::TopicInfo;
using core::Topic;
using core::TopicList;

namespace {
// DDL EXACTO de mkdb.sh create_tpcdb (una sentencia por entrada; el driver
// QSQLITE no acepta multi-statement en un único exec).
const char *kTpcDdl[] = {
    "create table if not exists id "
    "(name TEXT, slng TEXT, tlng TEXT, autr TEXT, cntt TEXT, ctgy TEXT, ilnk TEXT, "
    "orig TEXT, dtec TEXT, dteu TEXT, dtei TEXT, nwrd TEXT, nsnt TEXT, nimg TEXT, "
    "naud TEXT, nsze TEXT, levl TEXT, stts TEXT);",
    "create table if not exists config "
    "(words TEXT, sntcs TEXT, marks TEXT, learn TEXT, diffi TEXT, rplay TEXT, audio TEXT, "
    "ntosd TEXT, loop TEXT, rword TEXT, acheck TEXT, repass TEXT);",
    "create table if not exists reviews "
    "(date1 TEXT,date2 TEXT,date3 TEXT,date4 TEXT,date5 TEXT,"
    "date6 TEXT,date7 TEXT,date8 TEXT,date9 TEXT,date10 TEXT);",
    "create table if not exists learning (list TEXT);",
    "create table if not exists learnt (list TEXT);",
    "create table if not exists words (list TEXT);",
    "create table if not exists sentences (list TEXT);",
    "create table if not exists marks (list TEXT);",
    "create table if not exists Data "
    "(trgt TEXT, srce TEXT, exmp TEXT, defn TEXT, note TEXT, wrds TEXT, grmr TEXT, "
    "tags TEXT, mark TEXT, refr TEXT, imag TEXT, link TEXT, cdid TEXT, type TEXT);",
    "create table if not exists Pract1 (items_0 TEXT);",
    "create table if not exists Pract2 (items_0 TEXT);",
    "create table if not exists Pract3 (items_0 TEXT);",
    "create table if not exists Pract4 (items_0 TEXT);",
    "create table if not exists Pract5 (items_0 TEXT);",
    "create table if not exists Practice_stats (Pract1_icon INTEGER, "
    "Pract2_icon INTEGER, Pract3_icon INTEGER, Pract4_icon INTEGER, "
    "Pract5_icon INTEGER, Pract1_sess INTEGER, Pract2_sess INTEGER, "
    "Pract3_sess INTEGER, Pract4_sess INTEGER, Pract5_sess INTEGER);",
    "create table if not exists Translates "
    "(trgt TEXT, ch_srce TEXT, de_srce TEXT, en_srce TEXT, es_srce TEXT, "
    "fr_srce TEXT, it_srce TEXT, ja_srce TEXT, pt_srce TEXT, "
    "ru_srce TEXT, vi_srce TEXT);",
};

QString quote(const QString &v)
{
    return QString(v).replace(QLatin1Char('\''), QStringLiteral("''"));
}

// Ajusta el campo tsets correspondiente de TopicInfo (orden de sets.cfg).
bool setInfoField(TopicInfo &info, const QString &field, const QString &value)
{
    if (field == QLatin1String("name"))      info.setName(value);
    else if (field == QLatin1String("slng")) info.setSlng(value);
    else if (field == QLatin1String("tlng")) info.setTlng(value);
    else if (field == QLatin1String("autr")) info.setAutr(value);
    else if (field == QLatin1String("cntt")) info.setCntt(value);
    else if (field == QLatin1String("ctgy")) info.setCtgy(value);
    else if (field == QLatin1String("ilnk")) info.setIlnk(value);
    else if (field == QLatin1String("orig")) info.setOrig(value);
    else if (field == QLatin1String("dtec")) info.setDtec(value);
    else if (field == QLatin1String("dteu")) info.setDteu(value);
    else if (field == QLatin1String("dtei")) info.setDtei(value);
    else if (field == QLatin1String("nwrd")) info.setNwrd(value);
    else if (field == QLatin1String("nsnt")) info.setNsnt(value);
    else if (field == QLatin1String("nimg")) info.setNimg(value);
    else if (field == QLatin1String("naud")) info.setNaud(value);
    else if (field == QLatin1String("nsze")) info.setNsze(value);
    else if (field == QLatin1String("levl")) info.setLevl(value);
    else if (field == QLatin1String("info")) info.setInfo(value);
    else if (field == QLatin1String("stts")) info.setStts(value);
    else return false;
    return true;
}
} // namespace

TopicRepository::TopicRepository(core::AppPaths appPaths)
    : m_appPaths(std::move(appPaths))
{
}

core::TopicList TopicRepository::listTopics(const QString &tlng) const
{
    TopicList list;
    QDir dir(m_appPaths.topicsDirForLanguage(tlng));
    if (!dir.exists())
        return list;

    struct Entry {
        QString name;
        QDateTime mtime; // local, como QFileInfo
    };

    QList<Entry> entries;
    const QStringList names =
        dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden, QDir::NoSort);
    for (const QString &name : names) {
        if (name.startsWith(QLatin1Char('.')))
            continue; // find -not -path '*/\.*'
        const QFileInfo fi(dir.absoluteFilePath(name));
        if (!fi.isDir() || fi.isSymLink())
            continue; // find -type d (no dereferencea symlinks)
        entries.append({name, fi.lastModified()});
    }

    std::sort(entries.begin(), entries.end(), [](const Entry &a, const Entry &b) {
        if (a.mtime == b.mtime)
            return a.name < b.name; // desempate determinista exigido por F2.2
        return a.mtime > b.mtime;   // ls -tNd: más reciente primero
    });

    // -mtime -80 -> edad < 80 días; -mtime +79 -> edad >= 80 días.
    const qint64 thresholdMs = qint64(80) * 24 * 3600 * 1000;
    const qint64 nowMs = QDateTime::currentDateTime().toMSecsSinceEpoch();
    for (const Entry &e : entries) {
        if (nowMs - e.mtime.toMSecsSinceEpoch() < thresholdMs) {
            Topic topic; topic.setName(e.name);
            // Lee stts del .conf/stts (replica mkmn: sed -n 1p stts)
            const QString sttsPath = m_appPaths.topicConfDir(tlng, e.name)
                                     + QStringLiteral("/stts");
            QFile sttsFile(sttsPath);
            if (sttsFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                const QString stts = QString::fromUtf8(sttsFile.readLine()).trimmed();
                topic.setStts(stts);
            }
            list.add(topic);
        }
    }
    for (const Entry &e : entries) {
        if (nowMs - e.mtime.toMSecsSinceEpoch() >= thresholdMs) {
            Topic topic; topic.setName(e.name);
            const QString sttsPath = m_appPaths.topicConfDir(tlng, e.name)
                                     + QStringLiteral("/stts");
            QFile sttsFile(sttsPath);
            if (sttsFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                const QString stts = QString::fromUtf8(sttsFile.readLine()).trimmed();
                topic.setStts(stts);
            }
            list.add(topic);
        }
    }
    return list;
}

bool TopicRepository::topicExists(const QString &tlng, const QString &name) const
{
    if (name.isEmpty())
        return false;
    const QFileInfo fi(m_appPaths.topicDataDir(tlng, name));
    return fi.isDir() && !fi.isSymLink()
        && !fi.fileName().startsWith(QLatin1Char('.'));
}

core::Topic TopicRepository::loadTopic(const QString &tlng, const QString &name) const
{
    Topic topic;
    const QString conf = m_appPaths.topicConfDir(tlng, name);

    // stts: 1ª línea de .conf/stts (autoridad runtime). Si el fichero falta,
    // info.stts queda vacío (statusValue() -> 13, como mkmn ante no-numérico).
    QFile stts(conf + QStringLiteral("/stts"));
    if (stts.open(QIODevice::ReadOnly | QIODevice::Text)) {
        const QString first = QString::fromUtf8(stts.readLine()).trimmed();
        stts.close();
        topic.setStts(first);
    }

    // note: contenido del fichero (por defecto el original escribe " ").
    QFile note(conf + QStringLiteral("/note"));
    if (note.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString text = QString::fromUtf8(note.readAll());
        note.close();
        while (text.endsWith(QLatin1Char('\n')) || text.endsWith(QLatin1Char('\r')))
            text.chop(1);
        topic.setNote(text);
    }

    // ítems: 1 línea plana por ítem en .conf/data (FlatItemCodec = get_item).
    QList<Item> items;
    QFile data(conf + QStringLiteral("/data"));
    if (data.open(QIODevice::ReadOnly | QIODevice::Text)) {
        while (!data.atEnd()) {
            const QString line = QString::fromUtf8(data.readLine()).trimmed();
            if (!line.isEmpty())
                items.append(FlatItemCodec::decode(line));
        }
        data.close();
    }
    topic.setItems(items);

    // info: metadata desde la tabla id de la DB (si existe y tiene fila).
    // name siempre del directorio; stts SIEMPRE del fichero .conf/stts.
    TopicInfo info;
    info.setName(name);
    const QString dbPath = m_appPaths.topicDb(tlng, name);
    if (QFileInfo::exists(dbPath)) {
        SqliteDatabase db(dbPath);
        if (db.open()) {
            QStringList cols;
            {
                QSqlQuery table = db.query(QStringLiteral("pragma table_info(id);"));
                while (table.isActive() && table.next())
                    cols.append(table.value(1).toString());
            }
            for (const QString &col : cols) {
                if (col == QLatin1String("name") || col == QLatin1String("stts"))
                    continue;
                const QString v = db.value(
                    QStringLiteral("select %1 from id limit 1;").arg(col));
                setInfoField(info, col, v);
            }
            db.close();
        }
    }
    info.setName(name);
    info.setStts(topic.stts()); // stts runtime del fichero, nunca de id.stts
    topic.setInfo(info);

    return topic;
}

bool TopicRepository::createTopic(const QString &tlng, const TopicInfo &info, bool activate)
{
    QString name = info.name().trimmed();
    if (name.isEmpty() || name.size() > 55)
        return false;
    if (!topicCountBelowLimit(tlng))
        return false;

    // dedup " (n)" de new_topic (1..50).
    QString finalName = name;
    if (topicExists(tlng, finalName)) {
        bool found = false;
        for (int i = 1; i <= 50; ++i) {
            const QString candidate = QStringLiteral("%1 (%2)").arg(name).arg(i);
            if (!topicExists(tlng, candidate)) {
                finalName = candidate;
                found = true;
                break;
            }
        }
        if (!found)
            return false;
    }

    const QString dataDir = m_appPaths.topicDataDir(tlng, finalName);
    const QString conf = m_appPaths.topicConfDir(tlng, finalName);

    // chek_topic: check_dir "$DM_tlt/images" "$DC_tlt"
    if (!QDir().mkpath(dataDir + QStringLiteral("/images")))
        return false;
    if (!QDir().mkpath(conf))
        return false;
    // check_index _check: .conf/practice/log{1..3}
    QDir().mkpath(conf + QStringLiteral("/practice"));
    for (int i = 1; i <= 3; ++i) {
        const QString log = conf + QStringLiteral("/practice/log%1").arg(i);
        if (!QFileInfo::exists(log)) {
            QFile f(log);
            if (!f.open(QIODevice::WriteOnly))
                return false;
            f.close();
        }
    }

    // note: `echo " " > note`
    {
        QFile note(conf + QStringLiteral("/note"));
        if (!note.open(QIODevice::WriteOnly | QIODevice::Text))
            return false;
        note.write(" \n");
        note.close();
    }

    // stts: `echo ${stts} > stts` (default 1 si no se indica).
    const QString sttsValue = info.stts().isEmpty() ? QStringLiteral("1") : info.stts();
    {
        QFile stts(conf + QStringLiteral("/stts"));
        if (!stts.open(QIODevice::WriteOnly | QIODevice::Text))
            return false;
        stts.write(sttsValue.toUtf8());
        stts.write("\n");
        stts.close();
    }

    // data: `touch data`
    {
        const QString dataFile = conf + QStringLiteral("/data");
        if (!QFileInfo::exists(dataFile)) {
            QFile data(dataFile);
            if (!data.open(QIODevice::WriteOnly))
                return false;
            data.close();
        }
    }

    // DB tpc (create_tpcdb: rm previo + DDL + inserts).
    TopicInfo dbInfo = info;
    dbInfo.setName(finalName);
    if (!ensureTopicDatabase(tlng, dbInfo))
        return false;

    // chek_topic: `repass > 8` -> stts=2 (en topic nuevo repass='0': no-op,
    // pero se evalúa igual que el original para reproducción fiel).
    {
        SqliteDatabase db(m_appPaths.topicDb(tlng, finalName));
        if (db.open()) {
            bool ok = false;
            const int repass = db.value(QStringLiteral("select repass from config;")).toInt(&ok);
            if (ok && repass > 8) {
                QFile stts(conf + QStringLiteral("/stts"));
                if (stts.open(QIODevice::WriteOnly | QIODevice::Text)) {
                    stts.write("2\n");
                    stts.close();
                }
            }
            db.close();
        }
    }

    if (activate)
        ConfigRepository(m_appPaths).setActiveTopic(finalName);

    return true;
}

bool TopicRepository::removeTopic(const QString &tlng, const QString &name)
{
    if (!topicExists(tlng, name))
        return false;

    // delete_topic: cleanups "$DM/backup/${tpc}.bk" y "$DM_tl/${tpc}".
    const QString backup =
        m_appPaths.backupDir() + QLatin1Char('/') + name + QStringLiteral(".bk");
    if (QFileInfo::exists(backup))
        QFile::remove(backup);

    const bool removed = QDir(m_appPaths.topicDataDir(tlng, name)).removeRecursively();

    // delete_topic: `> "$DC_s/tpc"` si apuntaba al topic borrado.
    ConfigRepository cfg(m_appPaths);
    if (cfg.activeTopic() == name)
        cfg.setActiveTopic(QString());

    return removed;
}

bool TopicRepository::recreateTopicDatabase(const QString &tlng, const QString &name)
{
    if (name.isEmpty())
        return false;
    // mkdb.sh tpc: `rm -f tpc` + create_tpcdb (esquema y filas exactos).
    // El repair corre con el perfil cargado; mkdb usa $slng del perfil actual
    // (sin crear el fichero de config si no existe).
    QString slng;
    const QString cfg = m_appPaths.configDb();
    if (QFileInfo::exists(cfg)) {
        SqliteDatabase db(cfg);
        if (db.open())
            slng = db.value(QStringLiteral("select slng from lang limit 1;"));
    }
    TopicInfo info;
    info.setName(name);
    info.setSlng(slng);
    info.setTlng(tlng);
    return ensureTopicDatabase(tlng, info);
}

bool TopicRepository::ensureTopicDatabase(const QString &tlng, const TopicInfo &info)
{
    const QString dbPath = m_appPaths.topicDb(tlng, info.name());
    QFile::remove(dbPath);

    SqliteDatabase db(dbPath);
    if (!db.open())
        return false;

    for (const char *stmnt : kTpcDdl) {
        if (!db.execute(QLatin1String(stmnt)))
            return false;
    }

    // create_tpcdb: inserts de id / config / reviews / Practice_stats.
    const QString dtec = info.dtec().isEmpty()
        ? QDate::currentDate().toString(QStringLiteral("yyyy-MM-dd"))
        : info.dtec();
    QSqlQuery qId = db.query(QStringLiteral(
        "insert into id (name,slng,tlng,autr,cntt,ctgy,ilnk,orig,"
        "dtec,dteu,dtei,nwrd,nsnt,nimg,naud,nsze,levl,stts) "
        "values ('%1','%2','%3','%4','%5','%6','%7','%8','%9','','',"
        "'%10','%11','%12','%13','%14','%15','');")
                                 .arg(quote(info.name()), quote(info.slng()),
                                      quote(info.tlng()), quote(info.autr()),
                                      quote(info.cntt()), quote(info.ctgy()),
                                      quote(info.ilnk()), quote(info.orig()),
                                      quote(dtec),
                                      quote(info.nwrd()), quote(info.nsnt()),
                                      quote(info.nimg()), quote(info.naud()),
                                      quote(info.nsze()), quote(info.levl())));
    if (qId.lastError().isValid())
        return false;
    if (!db.execute(QStringLiteral(
            "insert into config (words,sntcs,marks,learn,diffi,rplay,audio,"
            "ntosd,loop,rword,acheck,repass) "
            "values ('FALSE','FALSE','FALSE','FALSE','FALSE','FALSE','FALSE',"
            "'FALSE','FALSE','FALSE','TRUE','0');")))
        return false;
    if (!db.execute(QStringLiteral("insert into reviews (date1) values ('');")))
        return false;
    if (!db.execute(QStringLiteral(
            "insert into Practice_stats (Pract1_icon,Pract2_icon,Pract3_icon,"
            "Pract4_icon,Pract5_icon,Pract1_sess,Pract2_sess,Pract3_sess,"
            "Pract4_sess,Pract5_sess) values (0,0,0,0,0,0,0,0,0,0);")))
        return false;
    db.close();
    return true;
}

bool TopicRepository::topicCountBelowLimit(const QString &tlng) const
{
    // new_topic: tope de 120 topics por idioma.
    QDir dir(m_appPaths.topicsDirForLanguage(tlng));
    if (!dir.exists())
        return true;
    int count = 0;
    const QStringList names =
        dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden, QDir::NoSort);
    for (const QString &name : names) {
        if (!name.startsWith(QLatin1Char('.')))
            ++count;
    }
    return count < 120;
}

} // namespace storage
} // namespace idiomind
