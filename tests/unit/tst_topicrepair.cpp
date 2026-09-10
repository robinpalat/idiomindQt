// F3.1 - tst_topicrepair: TopicRepairService replicando tls.sh check_index.
// Autoridad: docs/research/f3.1-bash-behavior.md §2 (verbatim), tls.sh 73-294.
//
// Nunca toca ~/.idiomind ni ~/.config/idiomind (AppPaths(homeDir=temp)).
#include <QDate>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSqlQuery>
#include <QTest>
#include <QTemporaryDir>

#include "core/config/AppPaths.h"
#include "core/learning/Review.h"
#include "core/topic/FlatItemCodec.h"
#include "core/topic/Item.h"
#include "services/topic/TopicRepairService.h"
#include "storage/dbs/SqliteDatabase.h"
#include "storage/topic/ReviewRepository.h"
#include "storage/topic/TopicDataRepository.h"
#include "storage/topic/TopicRepository.h"

using namespace idiomind::core;
using namespace idiomind::core::FlatItemCodec;

using idiomind::core::AppPaths;
using idiomind::services::TopicRepairService;
using idiomind::storage::ReviewRepository;
using idiomind::storage::SqliteDatabase;
using idiomind::storage::TopicDataRepository;
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

Item makeItem(const QString &trgt, const QString &srce, const QString &type)
{
    Item it;
    it.trgt = trgt;
    it.srce = srce;
    it.exmp = QStringLiteral("e");
    it.defn = QStringLiteral("d");
    it.note = QStringLiteral("n");
    it.wrds = QStringLiteral("w");
    it.grmr = QStringLiteral("g");
    it.tags = QStringLiteral("t");
    it.mark = QStringLiteral("FALSE");
    it.refr = QStringLiteral("r");
    it.imag = QStringLiteral("0");
    it.link = QStringLiteral("l");
    it.cdid = QStringLiteral("cd");
    it.type = type;
    return it;
}

QString confDir(const AppPaths &paths, const QString &topic)
{
    return paths.topicConfDir(QStringLiteral("English"), topic);
}

QString dataPath(const AppPaths &paths, const QString &topic)
{
    return confDir(paths, topic) + QStringLiteral("/data");
}

QString readAll(const QString &path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return QString();
    const QString out = QString::fromUtf8(f.readAll());
    f.close();
    return out;
}

QStringList readDataLines(const AppPaths &paths, const QString &topic)
{
    QStringList out;
    for (const QString &l : readAll(dataPath(paths, topic)).split(QLatin1Char('\n')))
        if (!l.isEmpty())
            out.append(l);
    return out;
}

void writeFileLines(const QString &path, const QStringList &lines)
{
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
        return;
    for (const QString &l : lines) {
        f.write(l.toUtf8());
        f.write("\n");
    }
    f.close();
}

void writeDb(const AppPaths &paths, const QString &table, const QString &sql)
{
    SqliteDatabase db(paths.topicDb(QStringLiteral("English"), QStringLiteral("T")));
    QVERIFY2(db.open(), qPrintable(table));
    QVERIFY2(db.execute(sql), qPrintable(sql));
}

} // namespace

class TestTopicRepair : public QObject
{
    Q_OBJECT

private slots:
    void pristineTopicNoChanges();
    void missingSttsAndDataTriggerRepair();
    void stts13RestoresFromBackup();
    void stts13WithoutBackupWritesOne();
    void nonNumericSttsBecomesOne();
    void date1BackfilledBeforeReviewStateReset();
    void newformFlagIsNoOp();
    void countMismatchRebuildsIndex();
    void emptyTemplateMarkerLinesDropped();
    void missingDataRestoresFromBackup();
    void missingDataWithoutBackupErrors();
    void mixedFormatsRebuildHonoursLearnState();
    void beyond200LinesAreTruncated();
    void blankLineCausesPartialRewrite();
    void nonSqliteDatabaseIsRecreated();
    void dbConfigReseeded();
    void dbReviewsTwoRowsPreserveFirstRowDates1To8();
    void dbIdRepairKeepsNameAndLosesStts();
};

void TestTopicRepair::pristineTopicNoChanges()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const AppPaths paths(tmp.path());
    QVERIFY(TopicRepository(paths).createTopic(QStringLiteral("English"),
                                               makeInfo(QStringLiteral("T"))));

    const auto rep = TopicRepairService(paths).checkIndex(
        QStringLiteral("English"), QStringLiteral("T"), true);
    QVERIFY(!rep.fixed);
    QVERIFY(!rep.dataRestored);
    QVERIFY(!rep.indexRebuilt);
    QVERIFY(!rep.errored);
    QCOMPARE(rep.dataCount, 0);
    QCOMPARE(rep.indexCount, 0);
    QCOMPARE(readAll(confDir(paths, QStringLiteral("T")) + QStringLiteral("/stts")),
             QStringLiteral("1\n"));
}

void TestTopicRepair::missingSttsAndDataTriggerRepair()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const AppPaths paths(tmp.path());
    QVERIFY(TopicRepository(paths).createTopic(QStringLiteral("English"),
                                               makeInfo(QStringLiteral("T"))));
    const QString conf = confDir(paths, QStringLiteral("T"));
    QVERIFY(QFile::remove(conf + QStringLiteral("/stts")));
    QVERIFY(QFile::remove(conf + QStringLiteral("/data")));

    // sin stts -> "1" creado (fix); sin data -> fix; sin backup -> errored.
    const auto rep = TopicRepairService(paths).checkIndex(
        QStringLiteral("English"), QStringLiteral("T"), true);
    QVERIFY(rep.fixed);
    QVERIFY(rep.errored);
    QCOMPARE(readAll(conf + QStringLiteral("/stts")), QStringLiteral("1\n"));
}

void TestTopicRepair::stts13RestoresFromBackup()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const AppPaths paths(tmp.path());
    QVERIFY(TopicRepository(paths).createTopic(QStringLiteral("English"),
                                               makeInfo(QStringLiteral("T"))));
    const QString conf = confDir(paths, QStringLiteral("T"));
    {
        QFile f(conf + QStringLiteral("/stts"));
        QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Text));
        f.write("13\n");
        f.close();
    }
    {
        QFile f(conf + QStringLiteral("/stts.bk"));
        QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Text));
        f.write("3\n");
        f.close();
    }

    const auto rep = TopicRepairService(paths).checkIndex(
        QStringLiteral("English"), QStringLiteral("T"), true);
    QVERIFY(rep.fixed);
    QVERIFY(!rep.errored);
    // El backup "3" restaura stts, pero cnt0==0 (tema vacío) hace que el
    // quirk final `echo 1 > stts` lo pise (stts "3" casa con el patrón).
    QCOMPARE(readAll(conf + QStringLiteral("/stts")), QStringLiteral("1\n"));
    QVERIFY(!QFileInfo::exists(conf + QStringLiteral("/stts.bk")));
    QVERIFY(rep.indexRebuilt); // mkmn=1 de la rama 13
}

void TestTopicRepair::stts13WithoutBackupWritesOne()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const AppPaths paths(tmp.path());
    QVERIFY(TopicRepository(paths).createTopic(QStringLiteral("English"),
                                               makeInfo(QStringLiteral("T"))));
    const QString conf = confDir(paths, QStringLiteral("T"));
    {
        QFile f(conf + QStringLiteral("/stts"));
        QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Text));
        f.write("13\n");
        f.close();
    }

    const auto rep = TopicRepairService(paths).checkIndex(
        QStringLiteral("English"), QStringLiteral("T"), true);
    QVERIFY(rep.fixed);
    QCOMPARE(readAll(conf + QStringLiteral("/stts")), QStringLiteral("1\n"));
    QVERIFY(rep.indexRebuilt);
}

void TestTopicRepair::nonNumericSttsBecomesOne()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const AppPaths paths(tmp.path());
    QVERIFY(TopicRepository(paths).createTopic(QStringLiteral("English"),
                                               makeInfo(QStringLiteral("T"))));
    const QString conf = confDir(paths, QStringLiteral("T"));
    {
        QFile f(conf + QStringLiteral("/stts"));
        QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Text));
        f.write("hola\n");
        f.close();
    }

    const auto rep = TopicRepairService(paths).checkIndex(
        QStringLiteral("English"), QStringLiteral("T"), true);
    QVERIFY(rep.fixed);
    QCOMPARE(readAll(conf + QStringLiteral("/stts")), QStringLiteral("1\n"));
}

void TestTopicRepair::date1BackfilledBeforeReviewStateReset()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const AppPaths paths(tmp.path());
    QVERIFY(TopicRepository(paths).createTopic(QStringLiteral("English"),
                                               makeInfo(QStringLiteral("T"))));
    const QString conf = confDir(paths, QStringLiteral("T"));
    {
        QFile f(conf + QStringLiteral("/stts"));
        QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Text));
        f.write("3\n");
        f.close();
    }

    // stts>1: date1 vacío -> hoy (tpc_db 9, no marca fix); como cnt0==0 y stts
    // coincide con el patrón de estado, al final stts pasa a 1 (quirk
    // `echo 1 > stts`, tampoco es un fix).
    const auto rep = TopicRepairService(paths).checkIndex(
        QStringLiteral("English"), QStringLiteral("T"), true);
    QVERIFY(!rep.fixed);
    QCOMPARE(readAll(conf + QStringLiteral("/stts")), QStringLiteral("1\n"));
    SqliteDatabase db(paths.topicDb(QStringLiteral("English"), QStringLiteral("T")));
    QVERIFY(db.open());
    QCOMPARE(db.value(QStringLiteral("select date1 from reviews;")),
             QDate::currentDate().toString(QStringLiteral("MM/dd/yyyy")));
}

void TestTopicRepair::newformFlagIsNoOp()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const AppPaths paths(tmp.path());
    QVERIFY(TopicRepository(paths).createTopic(QStringLiteral("English"),
                                               makeInfo(QStringLiteral("T"))));
    const QString conf = confDir(paths, QStringLiteral("T"));
    {
        QFile f(conf + QStringLiteral("/0.cfg"));
        f.open(QIODevice::WriteOnly);
        f.close();
    }

    // 0.cfg/id.cfg sólo marcan newform: ninguna acción, no es un fix.
    const auto rep = TopicRepairService(paths).checkIndex(
        QStringLiteral("English"), QStringLiteral("T"), true);
    QVERIFY(!rep.fixed);
    QVERIFY(!rep.dataRestored);
    QVERIFY(QFileInfo::exists(conf + QStringLiteral("/0.cfg")));
}

void TestTopicRepair::countMismatchRebuildsIndex()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const AppPaths paths(tmp.path());
    QVERIFY(TopicRepository(paths).createTopic(QStringLiteral("English"),
                                               makeInfo(QStringLiteral("T"))));
    TopicDataRepository repo(paths);
    QVERIFY(repo.appendItem(QStringLiteral("English"), QStringLiteral("T"),
                            makeItem(QStringLiteral("v1"), QStringLiteral("sr"),
                                     QStringLiteral("1"))));

    // index borrado -> index0(0) != cnt1(1) -> fix; restore re-crea lists/data.
    const QString conf = confDir(paths, QStringLiteral("T"));
    QVERIFY(QFile::remove(conf + QStringLiteral("/index")));

    const auto rep = TopicRepairService(paths).checkIndex(
        QStringLiteral("English"), QStringLiteral("T"), true);
    QVERIFY(rep.fixed);
    QVERIFY(!rep.errored);
    QVERIFY(rep.indexRebuilt);
    QVERIFY(rep.dataRestored);
    QCOMPARE(rep.dataCount, 1);
    QCOMPARE(rep.indexCount, 0); // el índice roto era el que se reparó
    QCOMPARE(readAll(conf + QStringLiteral("/index")), QStringLiteral("v1\nFALSE\nsr\n"));
    QCOMPARE(TopicDataRepository(paths).learningList(
                 QStringLiteral("English"), QStringLiteral("T")),
             QStringList{QStringLiteral("v1")});
}

void TestTopicRepair::emptyTemplateMarkerLinesDropped()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const AppPaths paths(tmp.path());
    QVERIFY(TopicRepository(paths).createTopic(QStringLiteral("English"),
                                               makeInfo(QStringLiteral("T"))));
    TopicDataRepository repo(paths);
    QVERIFY(repo.appendItem(QStringLiteral("English"), QStringLiteral("T"),
                            makeItem(QStringLiteral("real"), QStringLiteral("s"),
                                     QStringLiteral("1"))));

    // Añadir una línea marcador `trgt{}srce{}` rota (template vacío).
    {
        QFile f(dataPath(paths, QStringLiteral("T")));
        QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text));
        f.write("trgt{}srce{}\n");
        f.close();
    }

    const auto rep = TopicRepairService(paths).checkIndex(
        QStringLiteral("English"), QStringLiteral("T"), true);
    QVERIFY(rep.fixed);
    const QStringList lines = readDataLines(paths, QStringLiteral("T"));
    QCOMPARE(lines.size(), 1);
    QVERIFY(!lines[0].contains(QStringLiteral("trgt{}srce{}")));
    QCOMPARE(TopicDataRepository(paths).learningList(
                 QStringLiteral("English"), QStringLiteral("T")),
             QStringList{QStringLiteral("real")});
}

void TestTopicRepair::missingDataRestoresFromBackup()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const AppPaths paths(tmp.path());
    QVERIFY(TopicRepository(paths).createTopic(QStringLiteral("English"),
                                               makeInfo(QStringLiteral("T"))));
    const QString conf = confDir(paths, QStringLiteral("T"));
    QVERIFY(QFile::remove(conf + QStringLiteral("/data")));

    const QString itemA =
        FlatItemCodec::encode(makeItem(QStringLiteral("alpha"), QStringLiteral("a"),
                                       QStringLiteral("1")));
    const QString itemB =
        FlatItemCodec::encode(makeItem(QStringLiteral("beta"), QStringLiteral("b"),
                                       QStringLiteral("2")));
    QDir().mkpath(paths.backupDir());
    {
        QFile bk(paths.backupDir() + QStringLiteral("/T.bk"));
        QVERIFY(bk.open(QIODevice::WriteOnly | QIODevice::Text));
        bk.write("----- newest\n");
        bk.write(itemA.toUtf8());
        bk.write("\n");
        bk.write(itemB.toUtf8());
        bk.write("\n");
        bk.write("----- oldest\n");
        bk.close();
    }

    const auto rep = TopicRepairService(paths).checkIndex(
        QStringLiteral("English"), QStringLiteral("T"), true);
    QVERIFY(rep.fixed);
    QVERIFY(rep.dataRestored);
    QVERIFY(!rep.errored);
    const QStringList lines = readDataLines(paths, QStringLiteral("T"));
    QCOMPARE(lines.size(), 2);
    QCOMPARE(TopicDataRepository(paths).learningList(
                 QStringLiteral("English"), QStringLiteral("T")),
             (QStringList{QStringLiteral("alpha"), QStringLiteral("beta")}));
    QCOMPARE(TopicDataRepository(paths).marksList(
                 QStringLiteral("English"), QStringLiteral("T")),
             QStringList{});
}

void TestTopicRepair::missingDataWithoutBackupErrors()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const AppPaths paths(tmp.path());
    QVERIFY(TopicRepository(paths).createTopic(QStringLiteral("English"),
                                               makeInfo(QStringLiteral("T"))));
    const QString conf = confDir(paths, QStringLiteral("T"));
    QVERIFY(QFile::remove(conf + QStringLiteral("/data")));

    const auto rep = TopicRepairService(paths).checkIndex(
        QStringLiteral("English"), QStringLiteral("T"), true);
    QVERIFY(rep.fixed);
    QVERIFY(rep.errored);
}

void TestTopicRepair::mixedFormatsRebuildHonoursLearnState()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const AppPaths paths(tmp.path());
    QVERIFY(TopicRepository(paths).createTopic(QStringLiteral("English"),
                                               makeInfo(QStringLiteral("T"))));
    const QString conf = confDir(paths, QStringLiteral("T"));

    Item word = makeItem(QStringLiteral("w1"), QStringLiteral("sr1"), QStringLiteral("1"));
    word.mark = QStringLiteral("TRUE");
    Item sent = makeItem(QStringLiteral("s1"), QStringLiteral("sr2"), QStringLiteral("2"));
    const QString l14 = FlatItemCodec::encode(word);
    const QString l25 =
        QStringLiteral("trgt{s1}srce{sr2}slch{}slde{}slen{}sles{}slfr{}slit{}"
                       "slja{}slpt{}slru{}slvi{}exmp{ }defn{}note{}wrds{}grmr{}"
                       "tags{}mark{FALSE}refr{}imag{0}imgr{im}link{}cdid{cd}type{2}");
    writeFileLines(dataPath(paths, QStringLiteral("T")), {l14, l25});

    // Estado aprendido (stts 8) + listas e índice rotos.
    {
        QFile f(conf + QStringLiteral("/stts"));
        QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Text));
        f.write("8\n");
        f.close();
    }
    for (const QString &ta : {QStringLiteral("learning"), QStringLiteral("learnt"),
                              QStringLiteral("words"), QStringLiteral("sentences"),
                              QStringLiteral("marks")}) {
        writeDb(paths, QStringLiteral("learning"),
                QStringLiteral("delete from '%1';").arg(ta));
    }

    const auto rep = TopicRepairService(paths).checkIndex(
        QStringLiteral("English"), QStringLiteral("T"), true);
    QVERIFY(rep.fixed);
    QVERIFY(!rep.errored);
    QVERIFY(rep.indexRebuilt);

    // s==1 (stts 8): los ítems van a learnt, no a learning.
    TopicDataRepository repo(paths);
    QVERIFY(repo.learningList(QStringLiteral("English"), QStringLiteral("T")).isEmpty());
    QCOMPARE(repo.marksList(QStringLiteral("English"), QStringLiteral("T")),
             QStringList{QStringLiteral("w1")});
    SqliteDatabase db(paths.topicDb(QStringLiteral("English"), QStringLiteral("T")));
    QVERIFY(db.open());
    QStringList learnt;
    QSqlQuery lq = db.query(QStringLiteral("select list from learnt;"));
    while (lq.isActive() && lq.next())
        learnt.append(lq.value(0).toString());
    QCOMPARE(learnt.size(), 2);
    QVERIFY(learnt.contains(QStringLiteral("w1")));
    QVERIFY(learnt.contains(QStringLiteral("s1")));
    QStringList wordsT;
    QSqlQuery wq = db.query(QStringLiteral("select list from words;"));
    while (wq.isActive() && wq.next())
        wordsT.append(wq.value(0).toString());
    QCOMPARE(wordsT, QStringList{QStringLiteral("w1")}); // type 1 -> words
}

void TestTopicRepair::beyond200LinesAreTruncated()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const AppPaths paths(tmp.path());
    QVERIFY(TopicRepository(paths).createTopic(QStringLiteral("English"),
                                               makeInfo(QStringLiteral("T"))));
    TopicDataRepository repo(paths);
    for (int i = 0; i < 205; ++i) {
        const QString t = QStringLiteral("item%1").arg(i, 3, 10, QLatin1Char('0'));
        QVERIFY(repo.appendItem(QStringLiteral("English"), QStringLiteral("T"),
                                makeItem(t, QStringLiteral("s"), QStringLiteral("1"))));
    }
    // Corromper: learning vacío -> cnt1+cnt2(0) != cnt0(205) -> restore.
    writeDb(paths, QStringLiteral("learning"), QStringLiteral("delete from learning;"));
    QVERIFY(QFile::remove(confDir(paths, QStringLiteral("T")) + QStringLiteral("/index")));

    const auto rep = TopicRepairService(paths).checkIndex(
        QStringLiteral("English"), QStringLiteral("T"), true);
    QVERIFY(rep.fixed);
    QVERIFY(!rep.errored);
    // >200 se descartan en el rebuild (quirk count>200 break).
    QCOMPARE(rep.dataCount, 205);
    QCOMPARE(readDataLines(paths, QStringLiteral("T")).size(), 200);
    QCOMPARE(repo.learningList(QStringLiteral("English"), QStringLiteral("T")).size(), 200);
}

void TestTopicRepair::blankLineCausesPartialRewrite()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const AppPaths paths(tmp.path());
    QVERIFY(TopicRepository(paths).createTopic(QStringLiteral("English"),
                                               makeInfo(QStringLiteral("T"))));
    TopicDataRepository repo(paths);
    const QString lineA =
        FlatItemCodec::encode(makeItem(QStringLiteral("a"), QStringLiteral("x"),
                                       QStringLiteral("1")));
    const QString lineB =
        FlatItemCodec::encode(makeItem(QStringLiteral("b"), QStringLiteral("y"),
                                       QStringLiteral("1")));
    writeFileLines(dataPath(paths, QStringLiteral("T")), {lineA, QString(), lineB});
    writeDb(paths, QStringLiteral("learning"), QStringLiteral("delete from learning;"));

    const auto rep = TopicRepairService(paths).checkIndex(
        QStringLiteral("English"), QStringLiteral("T"), true);
    QVERIFY(rep.fixed);
    QVERIFY(!rep.errored);
    // La línea en blanco rompe el python del original: datatmp parcial [A] se
    // mueve (data=[A]) y las listas se quedan vacías (rollback sin commit).
    QCOMPARE(readDataLines(paths, QStringLiteral("T")), QStringList{lineA});
    QVERIFY(repo.learningList(QStringLiteral("English"), QStringLiteral("T")).isEmpty());
}

void TestTopicRepair::nonSqliteDatabaseIsRecreated()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const AppPaths paths(tmp.path());
    QVERIFY(TopicRepository(paths).createTopic(QStringLiteral("English"),
                                               makeInfo(QStringLiteral("T"))));
    const QString dbPath = paths.topicDb(QStringLiteral("English"), QStringLiteral("T"));
    {
        QFile f(dbPath);
        QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Text));
        f.write("esto no es una base sqlite\n");
        f.close();
    }

    const auto rep = TopicRepairService(paths).checkIndex(
        QStringLiteral("English"), QStringLiteral("T"), true);
    QVERIFY(rep.fixed);
    QVERIFY(!rep.errored);
    SqliteDatabase db(dbPath);
    QVERIFY(db.open());
    QCOMPARE(db.value(QStringLiteral("select Count(*) from reviews;")), QStringLiteral("1"));
    QCOMPARE(db.value(QStringLiteral("select Count(*) from config;")), QStringLiteral("1"));
}

void TestTopicRepair::dbConfigReseeded()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const AppPaths paths(tmp.path());
    QVERIFY(TopicRepository(paths).createTopic(QStringLiteral("English"),
                                               makeInfo(QStringLiteral("T"))));
    writeDb(paths, QStringLiteral("config"), QStringLiteral("delete from config;"));

    const auto rep = TopicRepairService(paths).checkIndex(
        QStringLiteral("English"), QStringLiteral("T"), true);
    QVERIFY(rep.dbConfigFixed);
    QVERIFY(!rep.fixed); // el problema de config no es un fix de _check
    SqliteDatabase db(paths.topicDb(QStringLiteral("English"), QStringLiteral("T")));
    QVERIFY(db.open());
    QCOMPARE(db.value(QStringLiteral("select words from config;")), QStringLiteral("TRUE"));
    QCOMPARE(db.value(QStringLiteral("select acheck from config;")), QStringLiteral("TRUE"));
    QCOMPARE(db.value(QStringLiteral("select repass from config;")), QStringLiteral("0"));
}

void TestTopicRepair::dbReviewsTwoRowsPreserveFirstRowDates1To8()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const AppPaths paths(tmp.path());
    QVERIFY(TopicRepository(paths).createTopic(QStringLiteral("English"),
                                               makeInfo(QStringLiteral("T"))));
    const QString dbPath = paths.topicDb(QStringLiteral("English"), QStringLiteral("T"));
    {
        SqliteDatabase db(dbPath);
        QVERIFY(db.open());
        // primera fila con fechas (date1..date8 y date10).
        QVERIFY(db.execute(
            "update reviews set date1='01/02/2024', date2='02/02/2024', "
            "date3='03/02/2024', date4='04/02/2024', date5='05/02/2024', "
            "date6='06/02/2024', date7='07/02/2024', date8='08/02/2024', "
            "date10='10/10/2024';"));
        // segunda fila -> reviews Count(*) != 1 -> db_reviews fix.
        QVERIFY(db.execute("insert into reviews (date1) values ('99/99/9999');"));
    }

    const auto rep = TopicRepairService(paths).checkIndex(
        QStringLiteral("English"), QStringLiteral("T"), true);
    QVERIFY(rep.dbReviewsFixed);
    QVERIFY(!rep.fixed);

    const Review r = ReviewRepository(paths).load(QStringLiteral("English"),
                                                  QStringLiteral("T"));
    QCOMPARE(r.sqliteDate(1), QStringLiteral("01/02/2024"));
    QCOMPARE(r.sqliteDate(8), QStringLiteral("08/02/2024"));
    QVERIFY(r.dateAt(9).isValid() == false);  // date9/10 se PIERDEN (quirk)
    QVERIFY(r.dateAt(10).isValid() == false);
    SqliteDatabase db(dbPath);
    QVERIFY(db.open());
    QCOMPARE(db.value(QStringLiteral("select Count(*) from reviews;")),
             QStringLiteral("1"));
}

void TestTopicRepair::dbIdRepairKeepsNameAndLosesStts()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const AppPaths paths(tmp.path());
    QVERIFY(TopicRepository(paths).createTopic(QStringLiteral("English"),
                                               makeInfo(QStringLiteral("T"))));
    const QString dbPath = paths.topicDb(QStringLiteral("English"), QStringLiteral("T"));
    {
        SqliteDatabase db(dbPath);
        QVERIFY(db.open());
        QVERIFY(db.execute(
            "update id set autr='autor', nwrd='5', stts='9';"));
        QVERIFY(db.execute(
            "insert into id (name) values ('basura');"));
    }

    const auto rep = TopicRepairService(paths).checkIndex(
        QStringLiteral("English"), QStringLiteral("T"), true);
    QVERIFY(rep.dbIdFixed);
    QVERIFY(!rep.fixed);

    SqliteDatabase db(dbPath);
    QVERIFY(db.open());
    QCOMPARE(db.value(QStringLiteral("select Count(*) from id;")), QStringLiteral("1"));
    QCOMPARE(db.value(QStringLiteral("select name from id;")), QStringLiteral("T"));
    QCOMPARE(db.value(QStringLiteral("select autr from id;")), QStringLiteral("autor"));
    QCOMPARE(db.value(QStringLiteral("select nwrd from id;")), QStringLiteral("5"));
    // El repair escribe sobre 'info' (no existe): stts NO se restaura (quirk).
    QCOMPARE(db.value(QStringLiteral("select stts from id;")), QString());
}

QTEST_GUILESS_MAIN(TestTopicRepair)
#include "tst_topicrepair.moc"