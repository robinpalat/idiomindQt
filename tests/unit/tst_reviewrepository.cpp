// F3.1 - tst_reviewrepository: ReviewRepository (fila `reviews` date1..date10).
// Autoridad: docs/04-databases.md, mkdb.sh create_tpcdb, mngr.sh
// mark_as_learned, ifs/cmns.sh calculate_review.
#include <QDate>
#include <QTest>
#include <QTemporaryDir>

#include "core/config/AppPaths.h"
#include "core/learning/Review.h"
#include "storage/dbs/SqliteDatabase.h"
#include "storage/topic/ReviewRepository.h"
#include "storage/topic/TopicRepository.h"

using namespace idiomind::core;

using idiomind::core::AppPaths;
using idiomind::storage::ReviewRepository;
using idiomind::storage::SqliteDatabase;
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

int reviewRowCount(const AppPaths &paths)
{
    SqliteDatabase db(paths.topicDb(QStringLiteral("English"), QStringLiteral("T")));
    if (!db.open())
        return -1;
    const QString v = db.value(QStringLiteral("select Count(*) from reviews;"));
    return v.toInt();
}

QString dateText(const AppPaths &paths, int column)
{
    SqliteDatabase db(paths.topicDb(QStringLiteral("English"), QStringLiteral("T")));
    if (!db.open())
        return QString();
    return db.value(QStringLiteral("select date%1 from reviews;").arg(column));
}

} // namespace

class TestReviewRepository : public QObject
{
    Q_OBJECT

private slots:
    void loadEmptyReturnsEmptyReview();
    void loadPartialAndFullRows();
    void storeRoundTripsAndKeepsSingleRow();
    void storeEnsuresRowWhenMissing();
    void clearResetsToSingleEmptyRow();
    void backfillDate1WhenEmpty();
    void backfillKeepsExistingDate1();
    void countFilledMirrorsNonEmptyColumns();
};

void TestReviewRepository::loadEmptyReturnsEmptyReview()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const AppPaths paths(tmp.path());
    QVERIFY(TopicRepository(paths).createTopic(QStringLiteral("English"),
                                               makeInfo(QStringLiteral("T"))));
    ReviewRepository repo(paths);

    // topic nuevo: fila (date1) values ('') -> Review vacío.
    const Review r = repo.load(QStringLiteral("English"), QStringLiteral("T"));
    QCOMPARE(r.countFilled(), 0);
    QVERIFY(!r.dateAt(1).isValid());
    QCOMPARE(r.sqliteDate(1), QString());
}

void TestReviewRepository::loadPartialAndFullRows()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const AppPaths paths(tmp.path());
    QVERIFY(TopicRepository(paths).createTopic(QStringLiteral("English"),
                                               makeInfo(QStringLiteral("T"))));
    ReviewRepository repo(paths);
    const QString db = paths.topicDb(QStringLiteral("English"), QStringLiteral("T"));

    {
        SqliteDatabase d(db);
        QVERIFY(d.open());
        QVERIFY(d.execute("update reviews set date1='01/02/2024', date4='03/04/2025';"));
    }

    const Review r = repo.load(QStringLiteral("English"), QStringLiteral("T"));
    QCOMPARE(r.countFilled(), 2);
    QCOMPARE(r.dateAt(1), QDate(2024, 1, 2));
    QCOMPARE(r.dateAt(4), QDate(2025, 3, 4));
    QVERIFY(!r.dateAt(2).isValid());

    // columna con valor inválido: setSqliteDate la rechaza (dominio F2.1).
    {
        SqliteDatabase d(db);
        QVERIFY(d.open());
        QVERIFY(d.execute("update reviews set date2='not-a-date';"));
    }
    const Review r2 = repo.load(QStringLiteral("English"), QStringLiteral("T"));
    QVERIFY(!r2.dateAt(2).isValid());
    QCOMPARE(r2.sqliteDate(2), QString());
}

void TestReviewRepository::storeRoundTripsAndKeepsSingleRow()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const AppPaths paths(tmp.path());
    QVERIFY(TopicRepository(paths).createTopic(QStringLiteral("English"),
                                               makeInfo(QStringLiteral("T"))));
    ReviewRepository repo(paths);

    Review in;
    in.setSqliteDate(1, QStringLiteral("01/01/2001"));
    in.setSqliteDate(5, QStringLiteral("05/05/2005"));
    in.setSqliteDate(10, QStringLiteral("10/10/2010"));
    QVERIFY(repo.store(QStringLiteral("English"), QStringLiteral("T"), in));

    QCOMPARE(reviewRowCount(paths), 1);
    const Review out = repo.load(QStringLiteral("English"), QStringLiteral("T"));
    QCOMPARE(out.countFilled(), 3);
    QCOMPARE(out.sqliteDate(1), QStringLiteral("01/01/2001"));
    QCOMPARE(out.sqliteDate(5), QStringLiteral("05/05/2005"));
    QCOMPARE(out.sqliteDate(10), QStringLiteral("10/10/2010"));

    // las columnas no establecidas quedan vacías (sin migas de filas previas).
    QCOMPARE(dateText(paths, 2), QStringLiteral(""));

    // re-store parcial sobrescribe la fila única (store escribe el vector
    // completo; la fecha 1 del store anterior se limpia al quedar vacía).
    Review part;
    part.setSqliteDate(2, QStringLiteral("02/02/2002"));
    QVERIFY(repo.store(QStringLiteral("English"), QStringLiteral("T"), part));
    QCOMPARE(reviewRowCount(paths), 1);
    QCOMPARE(dateText(paths, 2), QStringLiteral("02/02/2002"));
    QCOMPARE(dateText(paths, 1), QStringLiteral(""));
}

void TestReviewRepository::storeEnsuresRowWhenMissing()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const AppPaths paths(tmp.path());
    QVERIFY(TopicRepository(paths).createTopic(QStringLiteral("English"),
                                               makeInfo(QStringLiteral("T"))));
    ReviewRepository repo(paths);
    const QString db = paths.topicDb(QStringLiteral("English"), QStringLiteral("T"));

    {
        SqliteDatabase d(db);
        QVERIFY(d.open());
        QVERIFY(d.execute("delete from reviews;"));
    }
    QCOMPARE(reviewRowCount(paths), 0);

    Review in;
    in.setSqliteDate(3, QStringLiteral("03/03/2003"));
    QVERIFY(repo.store(QStringLiteral("English"), QStringLiteral("T"), in));
    QCOMPARE(reviewRowCount(paths), 1);
    QCOMPARE(dateText(paths, 3), QStringLiteral("03/03/2003"));
}

void TestReviewRepository::clearResetsToSingleEmptyRow()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const AppPaths paths(tmp.path());
    QVERIFY(TopicRepository(paths).createTopic(QStringLiteral("English"),
                                               makeInfo(QStringLiteral("T"))));
    ReviewRepository repo(paths);

    Review in;
    in.setSqliteDate(1, QStringLiteral("01/01/2001"));
    in.setSqliteDate(9, QStringLiteral("09/09/2009"));
    QVERIFY(repo.store(QStringLiteral("English"), QStringLiteral("T"), in));
    QCOMPARE(repo.countFilled(QStringLiteral("English"), QStringLiteral("T")), 2);

    QVERIFY(repo.clear(QStringLiteral("English"), QStringLiteral("T")));
    QCOMPARE(reviewRowCount(paths), 1);
    QCOMPARE(repo.countFilled(QStringLiteral("English"), QStringLiteral("T")), 0);
    const Review r = repo.load(QStringLiteral("English"), QStringLiteral("T"));
    QVERIFY(!r.dateAt(1).isValid());
}

void TestReviewRepository::backfillDate1WhenEmpty()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const AppPaths paths(tmp.path());
    QVERIFY(TopicRepository(paths).createTopic(QStringLiteral("English"),
                                               makeInfo(QStringLiteral("T"))));
    ReviewRepository repo(paths);

    QVERIFY(repo.backfillDate1IfEmpty(QStringLiteral("English"), QStringLiteral("T")));
    // stts > 1 del repair: date1 = hoy %m/%d/%Y.
    QCOMPARE(dateText(paths, 1),
             QDate::currentDate().toString(QStringLiteral("MM/dd/yyyy")));
}

void TestReviewRepository::backfillKeepsExistingDate1()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const AppPaths paths(tmp.path());
    QVERIFY(TopicRepository(paths).createTopic(QStringLiteral("English"),
                                               makeInfo(QStringLiteral("T"))));
    ReviewRepository repo(paths);

    Review in;
    in.setSqliteDate(1, QStringLiteral("12/31/2020"));
    QVERIFY(repo.store(QStringLiteral("English"), QStringLiteral("T"), in));
    QVERIFY(repo.backfillDate1IfEmpty(QStringLiteral("English"), QStringLiteral("T")));
    QCOMPARE(dateText(paths, 1), QStringLiteral("12/31/2020"));
}

void TestReviewRepository::countFilledMirrorsNonEmptyColumns()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const AppPaths paths(tmp.path());
    QVERIFY(TopicRepository(paths).createTopic(QStringLiteral("English"),
                                               makeInfo(QStringLiteral("T"))));
    ReviewRepository repo(paths);

    Review in;
    in.setSqliteDate(2, QStringLiteral("02/02/2002"));
    in.setSqliteDate(8, QStringLiteral("08/08/2008"));
    QVERIFY(repo.store(QStringLiteral("English"), QStringLiteral("T"), in));
    QCOMPARE(repo.countFilled(QStringLiteral("English"), QStringLiteral("T")), 2);
}

QTEST_GUILESS_MAIN(TestReviewRepository)
#include "tst_reviewrepository.moc"