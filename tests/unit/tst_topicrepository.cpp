// F2.2 - tst_topicrepository: TopicRepository sobre un $HOME de prueba.
// Autoridad: docs/05-topic-format.md, docs/04-databases.md, mkdb.sh
// create_tpcdb, add.sh new_topic, mngr.sh (mkmn / delete_topic),
// ifs/tpc.sh chek_topic, ifs/tls.sh check_index.
//
// Cada test usa su PROPIO QTemporaryDir como $HOME (AppPaths(homeDir=tmp) y
// reubica DM y DC); de esta forma ningún test toca ~/.idiomind ni
// ~/.config/idiomind (no-contaminación) ni se contamina con otro test.
#include <QDate>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTest>
#include <QTemporaryDir>

#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>

#include "core/config/AppPaths.h"
#include "core/topic/FlatItemCodec.h"
#include "core/topic/Topic.h"
#include "core/topic/TopicList.h"
#include "storage/config/ConfigRepository.h"
#include "storage/topic/TopicRepository.h"

using namespace idiomind::core;
using namespace idiomind::core::FlatItemCodec;

using idiomind::core::AppPaths;
using idiomind::core::Topic;
using idiomind::core::TopicList;
using idiomind::storage::ConfigRepository;
using idiomind::storage::TopicRepository;

namespace {

TopicInfo makeInfo(const QString &name)
{
    TopicInfo info;
    info.setName(name);
    info.setSlng(QStringLiteral("es"));
    info.setTlng(QStringLiteral("English"));
    return info;
}

void setMtime(const QString &path, const QDateTime &when)
{
    // QFile::setFileTime abre un handle -> no funciona sobre directorios.
    // utimensat opera directamente sobre la ruta (directorios incluidos) y
    // equivale exactamente a `touch -d` del mkmn original.
    const QByteArray p = QFile::encodeName(path);
    struct timespec ts[2];
    ts[0].tv_sec = 0;
    ts[0].tv_nsec = UTIME_NOW; // atime al instante (irrelevante para mkmn)
    ts[1].tv_sec = when.toSecsSinceEpoch();
    ts[1].tv_nsec = 0;
    QVERIFY2(::utimensat(AT_FDCWD, p.constData(), ts, 0) == 0,
             qPrintable(path));
}

} // namespace

class TestTopicRepository : public QObject
{
    Q_OBJECT

private slots:
    void listTopicsOnMissingLanguageIsEmpty();
    void createTopicBuildsOriginalLayout();
    void createTopicDatabaseMatchesCreateTpcdbDdl();
    void createTopicHonoursExplicitStts();
    void createTopicActivateWritesTpcFile();
    void createTopicDeduplicatesOnExistingName();
    void createTopicRejectsInvalidNamesAndCapacity();
    void topicExistsDetection();
    void loadTopicReadsBackState();
    void loadTopicDefaultsWhenFilesMissing();
    void loadTopicLoadsDbMetadataAndIgnoresIdStts();
    void listTopicsOrdersByMtimeThenName();
    void listTopicsExcludesHiddenShareDir();
    void removeTopicRemovesDirAndBackup();
    void removeTopicClearsActiveTopic();
    void removeTopicMissingReturnsFalse();
};

void TestTopicRepository::listTopicsOnMissingLanguageIsEmpty()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const AppPaths paths(tmp.path());
    TopicRepository repo(paths);
    QVERIFY(repo.listTopics(QStringLiteral("Français")).isEmpty());
}

void TestTopicRepository::createTopicBuildsOriginalLayout()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const AppPaths paths(tmp.path());
    TopicRepository repo(paths);
    QVERIFY(repo.createTopic(QStringLiteral("English"), makeInfo(QStringLiteral("My Topic"))));

    const QString dataDir = paths.topicDataDir(QStringLiteral("English"), QStringLiteral("My Topic"));
    const QString conf = paths.topicConfDir(QStringLiteral("English"), QStringLiteral("My Topic"));

    // chek_topic: dirs images/ y .conf/
    QVERIFY(QDir(dataDir).exists());
    QVERIFY(QDir(dataDir + QStringLiteral("/images")).exists());
    QVERIFY(QDir(conf).exists());

    // note: `echo " " > note` -> exactamente espacio + \n
    {
        QFile note(conf + QStringLiteral("/note"));
        QVERIFY(note.open(QIODevice::ReadOnly));
        QCOMPARE(QString::fromUtf8(note.readAll()), QStringLiteral(" \n"));
    }

    // stts: `echo ${stts} > stts` con default 1
    {
        QFile stts(conf + QStringLiteral("/stts"));
        QVERIFY(stts.open(QIODevice::ReadOnly));
        const QString v = QString::fromUtf8(stts.readAll()).trimmed();
        QCOMPARE(v, QStringLiteral("1"));
        stts.close();
    }

    // data existe (touch)
    QVERIFY(QFileInfo::exists(conf + QStringLiteral("/data")));

    // check_index _check: practice/log{1,2,3}
    for (int i = 1; i <= 3; ++i)
        QVERIFY(QFileInfo::exists(conf + QStringLiteral("/practice/log%1").arg(i)));

    // DB tpc = $DC_tlt/tpc
    QVERIFY(QFileInfo::exists(paths.topicDb(QStringLiteral("English"), QStringLiteral("My Topic"))));

    // el topic NO aparece en el listado de otro idioma
    QVERIFY(repo.listTopics(QStringLiteral("Español")).isEmpty());
    QVERIFY(repo.topicExists(QStringLiteral("English"), QStringLiteral("My Topic")));
}

void TestTopicRepository::createTopicDatabaseMatchesCreateTpcdbDdl()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const AppPaths paths(tmp.path());
    const QString lang = QStringLiteral("English");
    const QString name = QStringLiteral("Ddl");
    TopicRepository repo(paths);
    QVERIFY(repo.createTopic(lang, makeInfo(name)));

    const QString dbPath = paths.topicDb(lang, name);
    QSqlDatabase db = QSqlDatabase::addDatabase(
        QStringLiteral("QSQLITE"), QStringLiteral("tpcdb_check"));
    db.setDatabaseName(dbPath);
    QVERIFY(db.open());

    const QStringList wantTables = {
        "id", "config", "reviews", "learning", "learnt", "words", "sentences",
        "marks", "Data", "Pract1", "Pract2", "Pract3", "Pract4", "Pract5",
        "Practice_stats", "Translates"};
    for (const QString &t : wantTables)
        QVERIFY2(db.tables().contains(t), qPrintable(QStringLiteral("missing %1").arg(t)));

    // fila id: name/slng/tlng/dtec=today, resto vacío
    {
        QSqlQuery q(QStringLiteral("select name,slng,tlng,dtec,orig,stts from id;"), db);
        QVERIFY(q.next());
        QCOMPARE(q.value(0).toString(), name);
        QCOMPARE(q.value(1).toString(), QStringLiteral("es"));
        QCOMPARE(q.value(2).toString(), lang);
        QCOMPARE(q.value(3).toString(), QDate::currentDate().toString(QStringLiteral("yyyy-MM-dd")));
        QCOMPARE(q.value(4).toString(), QString()); // orig '' en create_tpcdb
        QCOMPARE(q.value(5).toString(), QString()); // id.stts '' siempre al crear
    }
    // fila config: repass 0, acheck TRUE
    {
        QSqlQuery q(QStringLiteral("select acheck,repass from config;"), db);
        QVERIFY(q.next());
        QCOMPARE(q.value(0).toString(), QStringLiteral("TRUE"));
        QCOMPARE(q.value(1).toString(), QStringLiteral("0"));
    }
    // filas únicas por tabla: id/config/reviews/Practice_stats tienen 1 fila
    for (const QString &t : {QStringLiteral("id"), QStringLiteral("config"),
                             QStringLiteral("reviews"), QStringLiteral("Practice_stats")}) {
        QSqlQuery q(QStringLiteral("select count(*) from %1;").arg(t), db);
        QVERIFY(q.next());
        QCOMPARE(q.value(0).toInt(), 1);
    }
    db.close();
    QSqlDatabase::removeDatabase(QStringLiteral("tpcdb_check"));
}

void TestTopicRepository::createTopicHonoursExplicitStts()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const AppPaths paths(tmp.path());
    TopicRepository repo(paths);
    TopicInfo info = makeInfo(QStringLiteral("Hard"));
    info.setStts(QStringLiteral("6"));
    QVERIFY(repo.createTopic(QStringLiteral("English"), info));

    const QString conf = paths.topicConfDir(QStringLiteral("English"), QStringLiteral("Hard"));
    QFile stts(conf + QStringLiteral("/stts"));
    QVERIFY(stts.open(QIODevice::ReadOnly));
    QCOMPARE(QString::fromUtf8(stts.readAll()).trimmed(), QStringLiteral("6"));
    stts.close();
}

void TestTopicRepository::createTopicActivateWritesTpcFile()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const AppPaths paths(tmp.path());
    TopicRepository repo(paths);
    QVERIFY(repo.createTopic(QStringLiteral("English"), makeInfo(QStringLiteral("Active")),
                             /*activate=*/true));
    ConfigRepository cfg(paths);
    QCOMPARE(cfg.activeTopic(), QStringLiteral("Active"));

    // sin activate no toca $DC_s/tpc
    QVERIFY(repo.createTopic(QStringLiteral("English"), makeInfo(QStringLiteral("Passive")),
                             /*activate=*/false));
    QCOMPARE(cfg.activeTopic(), QStringLiteral("Active"));
}

void TestTopicRepository::createTopicDeduplicatesOnExistingName()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const AppPaths paths(tmp.path());
    TopicRepository repo(paths);
    QVERIFY(repo.createTopic(QStringLiteral("English"), makeInfo(QStringLiteral("My Topic"))));

    // new_topic: si ya existe, se crea "My Topic (1)"
    QVERIFY(repo.createTopic(QStringLiteral("English"), makeInfo(QStringLiteral("My Topic"))));
    QVERIFY(repo.topicExists(QStringLiteral("English"), QStringLiteral("My Topic")));
    QVERIFY(repo.topicExists(QStringLiteral("English"), QStringLiteral("My Topic (1)")));

    // tercera vez -> "My Topic (2)"
    QVERIFY(repo.createTopic(QStringLiteral("English"), makeInfo(QStringLiteral("My Topic"))));
    QVERIFY(repo.topicExists(QStringLiteral("English"), QStringLiteral("My Topic (2)")));

    // nombre con espacio final: trimmed -> "My Topic", colisiona, dedup "(3)"
    QVERIFY(repo.createTopic(QStringLiteral("English"), makeInfo(QStringLiteral("My Topic "))));
    QVERIFY(repo.topicExists(QStringLiteral("English"), QStringLiteral("My Topic")));
    QVERIFY(repo.topicExists(QStringLiteral("English"), QStringLiteral("My Topic (3)")));

    // un nombre distinto no colisiona
    QVERIFY(!repo.topicExists(QStringLiteral("English"), QStringLiteral("Japanese name")));
}

void TestTopicRepository::createTopicRejectsInvalidNamesAndCapacity()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const AppPaths paths(tmp.path());
    TopicRepository repo(paths);
    // empty -> false
    QVERIFY(!repo.createTopic(QStringLiteral("English"), makeInfo(QStringLiteral("  "))));
    // nombre > 55 (new_topic lo rechaza; la UI renombraría a "Untitled")
    {
        TopicInfo longName;
        longName.setName(QString(60, QLatin1Char('x')));
        QVERIFY(!repo.createTopic(QStringLiteral("English"), longName));
    }

    // tope real: new_topic "Maximum number of topics reached" (>=120 dirs).
    QDir lang(paths.topicsDirForLanguage(QStringLiteral("English")));
    QVERIFY(lang.mkpath(QStringLiteral(".")));
    for (int i = 1; i <= 120; ++i)
        QVERIFY(lang.mkdir(QStringLiteral("t%1").arg(i)));
    QVERIFY(!repo.createTopic(QStringLiteral("English"), makeInfo(QStringLiteral("OneTooMany"))));

    // <120 dirs deja crear (el tope es estricto)
    QDir lang2(paths.topicsDirForLanguage(QStringLiteral("German")));
    QVERIFY(lang2.mkpath(QStringLiteral(".")));
    for (int i = 1; i <= 119; ++i)
        QVERIFY(lang2.mkdir(QStringLiteral("t%1").arg(i)));
    QVERIFY(repo.createTopic(QStringLiteral("German"), makeInfo(QStringLiteral("Allowed"))));
    QVERIFY(repo.topicExists(QStringLiteral("German"), QStringLiteral("Allowed")));
}

void TestTopicRepository::topicExistsDetection()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const AppPaths paths(tmp.path());
    TopicRepository repo(paths);
    QVERIFY(!repo.topicExists(QStringLiteral("English"), QStringLiteral("nope")));
    QVERIFY(!repo.topicExists(QStringLiteral("English"), QString()));

    QVERIFY(repo.createTopic(QStringLiteral("English"), makeInfo(QStringLiteral("Exists"))));
    QVERIFY(repo.topicExists(QStringLiteral("English"), QStringLiteral("Exists")));

    // un fichero normal no es topic
    QFile f(paths.topicsDirForLanguage(QStringLiteral("English")) + QStringLiteral("/plain"));
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write("x");
    f.close();
    QVERIFY(!repo.topicExists(QStringLiteral("English"), QStringLiteral("plain")));

    // dir oculto (.share) NO es topic
    QDir().mkpath(paths.topicsDirForLanguage(QStringLiteral("English")) + QStringLiteral("/.share"));
    QVERIFY(!repo.topicExists(QStringLiteral("English"), QStringLiteral(".share")));
}

void TestTopicRepository::loadTopicReadsBackState()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const AppPaths paths(tmp.path());
    TopicRepository repo(paths);
    const QString lang = QStringLiteral("English");
    const QString name = QStringLiteral("Back");
    QVERIFY(repo.createTopic(lang, makeInfo(name)));

    // escribe 2 ítems en data (línea plana, como el writer original)
    Item a;
    a.trgt = QStringLiteral("hello");
    a.srce = QStringLiteral("hola");
    a.type = QStringLiteral("1");
    Item b;
    b.trgt = QStringLiteral("goodbye");
    b.srce = QStringLiteral("adiós");
    b.type = QStringLiteral("1");
    b.mark = QStringLiteral("TRUE");

    const QString dataFile = paths.topicConfDir(lang, name) + QStringLiteral("/data");
    {
        QFile data(dataFile);
        QVERIFY(data.open(QIODevice::WriteOnly | QIODevice::Text));
        data.write(FlatItemCodec::encode(a).toUtf8());
        data.write("\n");
        data.write(FlatItemCodec::encode(b).toUtf8());
        data.write("\n");
        data.close();
    }
    // nota distinta para comprobar la lectura fiel
    {
        QFile note(paths.topicConfDir(lang, name) + QStringLiteral("/note"));
        QVERIFY(note.open(QIODevice::WriteOnly | QIODevice::Text));
        note.write("nota del tema\n");
        note.close();
    }

    const Topic loaded = repo.loadTopic(lang, name);
    QCOMPARE(loaded.name(), name);
    QCOMPARE(loaded.stts(), QStringLiteral("1"));
    QCOMPARE(loaded.statusValue(), 1);
    QCOMPARE(loaded.note(), QStringLiteral("nota del tema"));
    QCOMPARE(loaded.items().size(), 2);
    QCOMPARE(loaded.items().at(0).trgt, QStringLiteral("hello"));
    QCOMPARE(loaded.items().at(0).srce, QStringLiteral("hola"));
    QCOMPARE(loaded.items().at(1).trgt, QStringLiteral("goodbye"));
    QCOMPARE(loaded.items().at(1).mark, QStringLiteral("TRUE"));
    QCOMPARE(loaded.items().at(1).srce, QStringLiteral("adiós"));
}

void TestTopicRepository::loadTopicDefaultsWhenFilesMissing()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const AppPaths paths(tmp.path());
    TopicRepository repo(paths);
    const QString lang = QStringLiteral("English");
    const QString name = QStringLiteral("Empty");
    QVERIFY(repo.createTopic(lang, makeInfo(name)));

    // quita stts -> el estado queda vacío (statusValue -> 13, como mkmn)
    QFile::remove(paths.topicConfDir(lang, name) + QStringLiteral("/stts"));
    const Topic loaded = repo.loadTopic(lang, name);
    QCOMPARE(loaded.stts(), QString());
    QCOMPARE(loaded.statusValue(), 13);
    QCOMPARE(loaded.note(), QStringLiteral(" ")); // note por defecto
    QVERIFY(loaded.items().isEmpty());
}

void TestTopicRepository::loadTopicLoadsDbMetadataAndIgnoresIdStts()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const AppPaths paths(tmp.path());
    TopicRepository repo(paths);
    const QString lang = QStringLiteral("English");
    const QString name = QStringLiteral("Meta");
    QVERIFY(repo.createTopic(lang, makeInfo(name)));

    // metadatos reales (p.ej. importado DW): autr/dteu/nwrd
    const QString dbPath = paths.topicDb(lang, name);
    QSqlDatabase db = QSqlDatabase::addDatabase(
        QStringLiteral("QSQLITE"), QStringLiteral("id_meta_check"));
    db.setDatabaseName(dbPath);
    QVERIFY(db.open());
    {
        QSqlQuery q(
            QStringLiteral("update id set autr='Jane', dteu='2024-06-01', nwrd='12' where 1;"), db);
        QVERIFY(q.isActive());
        // id.stts manipulada para verificar que NO es autoridad
        QSqlQuery q2(QStringLiteral("update id set stts='9' where 1;"), db);
        QVERIFY(q2.isActive());
    }
    db.close();
    QSqlDatabase::removeDatabase(QStringLiteral("id_meta_check"));

    const Topic loaded = repo.loadTopic(lang, name);
    QCOMPARE(loaded.info().autr(), QStringLiteral("Jane"));
    QCOMPARE(loaded.info().dteu(), QStringLiteral("2024-06-01"));
    QCOMPARE(loaded.info().nwrd(), QStringLiteral("12"));
    // stts runtime = fichero .conf/stts (1), NUNCA la columna id.stts (9)
    QCOMPARE(loaded.stts(), QStringLiteral("1"));
    QCOMPARE(loaded.info().stts(), QStringLiteral("1"));
}

void TestTopicRepository::listTopicsOrdersByMtimeThenName()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const AppPaths paths(tmp.path());
    TopicRepository repo(paths);
    const QString lang = QStringLiteral("English");
    const QDateTime now = QDateTime::currentDateTime();

    QVERIFY(repo.createTopic(lang, makeInfo(QStringLiteral("Rec1"))));
    QVERIFY(repo.createTopic(lang, makeInfo(QStringLiteral("Rec2"))));
    QVERIFY(repo.createTopic(lang, makeInfo(QStringLiteral("Old2"))));
    QVERIFY(repo.createTopic(lang, makeInfo(QStringLiteral("Old1"))));

    const QString tl = paths.topicsDirForLanguage(lang);
    setMtime(tl + QStringLiteral("/Rec1"), now.addSecs(-60 * 5));
    setMtime(tl + QStringLiteral("/Rec2"), now);
    setMtime(tl + QStringLiteral("/Old2"), now.addDays(-100));
    setMtime(tl + QStringLiteral("/Old1"), now.addDays(-90));

    // mkmn: recientes (<80 días) primero, luego el resto; dentro de cada
    // grupo, más reciente primero (ls -tNd).
    const TopicList list = repo.listTopics(lang);
    QCOMPARE(list.count(), 4);
    QCOMPARE(list.at(0).name(), QStringLiteral("Rec2"));
    QCOMPARE(list.at(1).name(), QStringLiteral("Rec1"));
    QCOMPARE(list.at(2).name(), QStringLiteral("Old1"));
    QCOMPARE(list.at(3).name(), QStringLiteral("Old2"));

    // desempate determinista de F2.2: mtime idéntico -> orden alfabético
    setMtime(tl + QStringLiteral("/Rec1"), now);
    const TopicList tied = repo.listTopics(lang);
    QCOMPARE(tied.count(), 4);
    QCOMPARE(tied.at(0).name(), QStringLiteral("Rec1"));
    QCOMPARE(tied.at(1).name(), QStringLiteral("Rec2"));
    QCOMPARE(tied.at(2).name(), QStringLiteral("Old1"));
    QCOMPARE(tied.at(3).name(), QStringLiteral("Old2"));
}

void TestTopicRepository::listTopicsExcludesHiddenShareDir()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const AppPaths paths(tmp.path());
    TopicRepository repo(paths);
    const QString lang = QStringLiteral("English");
    QVERIFY(repo.createTopic(lang, makeInfo(QStringLiteral("Visible"))));

    // .share creado DESPUÉS (más reciente que Visible) y debe quedar oculto
    QDir().mkpath(paths.sharedDirForLanguage(lang));
    const QDateTime now = QDateTime::currentDateTime();
    setMtime(paths.sharedDirForLanguage(lang), now);

    const TopicList list = repo.listTopics(lang);
    QCOMPARE(list.count(), 1);
    QCOMPARE(list.at(0).name(), QStringLiteral("Visible"));
}

void TestTopicRepository::removeTopicRemovesDirAndBackup()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const AppPaths paths(tmp.path());
    TopicRepository repo(paths);
    const QString lang = QStringLiteral("English");
    const QString name = QStringLiteral("Gone");
    QVERIFY(repo.createTopic(lang, makeInfo(name)));

    // backup que delete_topic debe limpiar: $DM/backup/<name>.bk
    QDir().mkpath(paths.backupDir());
    QFile bk(paths.backupDir() + QStringLiteral("/") + name + QStringLiteral(".bk"));
    QVERIFY(bk.open(QIODevice::WriteOnly));
    bk.write("data");
    bk.close();

    QVERIFY(repo.removeTopic(lang, name));
    QVERIFY(!repo.topicExists(lang, name));
    QVERIFY(!QFileInfo::exists(
        paths.backupDir() + QStringLiteral("/") + name + QStringLiteral(".bk")));
    QVERIFY(!repo.listTopics(lang).contains(name));
}

void TestTopicRepository::removeTopicClearsActiveTopic()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const AppPaths paths(tmp.path());
    TopicRepository repo(paths);
    const QString lang = QStringLiteral("English");
    const QString name = QStringLiteral("Active");
    QVERIFY(repo.createTopic(lang, makeInfo(name), /*activate=*/true));
    QCOMPARE(ConfigRepository(paths).activeTopic(), name);

    QVERIFY(repo.removeTopic(lang, name));
    // delete_topic: `> "$DC_s/tpc"` si el borrado era el activo
    QCOMPARE(ConfigRepository(paths).activeTopic(), QString());
}

void TestTopicRepository::removeTopicMissingReturnsFalse()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const AppPaths paths(tmp.path());
    TopicRepository repo(paths);
    QVERIFY(!repo.removeTopic(QStringLiteral("English"), QStringLiteral("neverExisted")));
}

QTEST_GUILESS_MAIN(TestTopicRepository)
#include "tst_topicrepository.moc"