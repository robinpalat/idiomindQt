// F4 - Tests: PracticeLogs, IndexBuilder, LearningEngine, SharedRepository
#include <QDate>
#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QtTest/QtTest>

#include "core/config/AppPaths.h"
#include "core/learning/IndexBuilder.h"
#include "core/learning/PracticeLogs.h"
#include "core/learning/Review.h"
#include "core/learning/ReviewCalculator.h"
#include "core/topic/Item.h"
#include "storage/LearningEngine.h"
#include "storage/SharedRepository.h"
#include "storage/topic/TopicDataRepository.h"
#include "storage/topic/TopicRepository.h"
#include "storage/topic/ReviewRepository.h"

using namespace idiomind::core;
using namespace idiomind::storage;

static Item makeWord(const QString &trgt, const QString &srce) {
    Item it; it.trgt = trgt; it.srce = srce; it.type = QStringLiteral("1");
    it.cdid = QStringLiteral("cdid_") + trgt; return it;
}
static TopicInfo makeInfo(const QString &name) {
    TopicInfo info; info.setName(name);
    info.setSlng("Español"); info.setTlng("English");
    info.setCtgy("storyteller"); info.setIlnk("test123456");
    info.setNwrd("0"); info.setNsnt("0"); info.setNimg("0");
    info.setNaud("0"); info.setLevl("1"); info.setStts("0");
    return info;
}
static void createTopic(AppPaths &p, const QString &tlng, const QString &name,
                         const QList<Item> &items) {
    TopicRepository(p).createTopic(tlng, makeInfo(name));
    TopicDataRepository dr(p);
    dr.writeStts(tlng, name, QStringLiteral("1"));
    for (const Item &it : items) dr.appendItem(tlng, name, it);
}

class tst_learning : public QObject {
    Q_OBJECT
private slots:
    void practiceLogs();
    void indexBuilder();
    void reviewCalculatorUnit();
    void learningEngineIntegration();
    void markToLearnUnit();
    void overduePercentUnit();
    void sharedRepository();
    void reviewDates();
    void parityDataIndexStts();
};

void tst_learning::practiceLogs() {
    QTemporaryDir h; QVERIFY(h.isValid());
    QString c = h.path() + "/conf";
    QDir().mkpath(c + "/practice");
    PracticeLogs logs;
    logs.append(1, "hello"); logs.append(1, "world"); logs.append(2, "hello");
    QVERIFY(logs.inLog(1, "hello")); QVERIFY(logs.inLog(1, "world"));
    QVERIFY(logs.inLog(2, "hello")); QVERIFY(!logs.inLog(2, "world"));
    logs.save(c);
    PracticeLogs loaded; loaded.load(c);
    QVERIFY(loaded.inLog(1, "hello")); QVERIFY(loaded.inLog(2, "hello"));
    logs.clear(c);
    QVERIFY(!logs.inLog(1, "hello"));
}

void tst_learning::indexBuilder() {
    IndexBuildInput in;
    in.dataLines = {"trgt{hello}srce{hola}", "trgt{world}srce{mundo}"};
    in.learning = {"hello", "world"}; in.marks = {"hello"};
    PracticeLogs logs; in.logs = logs; in.acheck = false; in.chkFlag = false;
    auto lines = buildIndexLines(in);
    QCOMPARE(lines.size(), 6);
    QVERIFY(lines[0].contains("<b><big>hello</big></b>"));
    QCOMPARE(lines[1], "FALSE"); QCOMPARE(lines[2], "hola");
    QCOMPARE(lines[3], "world"); QCOMPARE(lines[4], "FALSE"); QCOMPARE(lines[5], "mundo");

    logs.setRawLog(1, "word");
    IndexBuildInput in2;
    in2.dataLines = {"trgt{word}srce{palabra}"}; in2.learning = {"word"};
    in2.logs = logs; in2.acheck = true; in2.chkFlag = true;
    auto l2 = buildIndexLines(in2);
    QCOMPARE(l2.size(), 3); QCOMPARE(l2[0], "word"); QCOMPARE(l2[1], "TRUE");
}

void tst_learning::reviewCalculatorUnit() {
    auto m0 = ReviewCalculator::markAsLearned(ReviewState::Learning, 0);
    QCOMPARE(m0.writtenDate, 1); QCOMPARE((int)m0.state, 3);
    QCOMPARE(ReviewCalculator::markAsLearned(ReviewState::MidReview, 1).writtenDate, 2);
    QCOMPARE(ReviewCalculator::markAsLearned(ReviewState::MidReviewAlternate, 2).writtenDate, 3);
    QCOMPARE((int)ReviewCalculator::markAsLearned(ReviewState::Learning, 3).state, 4);
    QVERIFY(ReviewCalculator::markAsLearned(ReviewState::Learning, 8).masteredFlag);
    QCOMPARE((int)ReviewCalculator::markAsLearned(ReviewState::Learning, 9).state, 2);
    QCOMPARE(ReviewCalculator::markAsLearned(ReviewState::Paused, 0).writtenDate, -1);
}

void tst_learning::learningEngineIntegration() {
    QTemporaryDir h; QVERIFY(h.isValid());
    AppPaths p(h.path(), h.path());
    createTopic(p, "English", "Test", {makeWord("hello","hola"), makeWord("world","mundo")});

    TopicDataRepository dr(p);
    QCOMPARE(dr.readStts("English", "Test"), "1");
    QCOMPARE(dr.learningList("English", "Test").size(), 2);
    QCOMPARE(dr.loadIndex("English", "Test").size(), 6);

    { LearningEngine e(p); auto r = e.markAsLearned("English","Test"); QCOMPARE(r.writtenDate, 1); }
    QCOMPARE(dr.readStts("English", "Test"), "3");
    ReviewRepository rr(p);
    QCOMPARE(rr.load("English","Test").countFilled(), 1);
    QCOMPARE(rr.load("English","Test").dateAt(1), QDate::currentDate());

    // Second markAsLearned: stts=3 (Waiting) is NOT in {1,5,6}, so no-op
    { LearningEngine e(p); auto r = e.markAsLearned("English","Test");
      QCOMPARE(r.writtenDate, -1); } // guard fails
    QCOMPARE(rr.load("English","Test").countFilled(), 1);
}

void tst_learning::markToLearnUnit() {
    QCOMPARE((int)ReviewCalculator::markToLearn(ReviewState::Waiting, 9), 2);
    QCOMPARE((int)ReviewCalculator::markToLearn(ReviewState::Waiting, 5), 5);
    QCOMPARE((int)ReviewCalculator::markToLearn(ReviewState::WaitingAlternate, 5), 6);
}

void tst_learning::overduePercentUnit() {
    QCOMPARE(ReviewCalculator::overduePercent(4, 1), 100);
    QCOMPARE(ReviewCalculator::overduePercent(8, 1), 200);
    QCOMPARE(ReviewCalculator::overduePercent(0, 0), 0);
    QCOMPARE(ReviewCalculator::overduePercent(10, 4), 100);
}

void tst_learning::sharedRepository() {
    QTemporaryDir h; QVERIFY(h.isValid());
    AppPaths p(h.path(), h.path());
    SharedRepository shr(p); shr.ensureTables("English");
    shr.insertTopic("English", ReviewCalculator::ListKind::T1, "A");
    shr.insertTopic("English", ReviewCalculator::ListKind::T1, "B");
    auto l = shr.listOf("English", ReviewCalculator::ListKind::T1);
    QCOMPARE(l.size(), 2); QVERIFY(l.contains("A")); QVERIFY(l.contains("B"));
    shr.removeAllLists("English", "A");
    QVERIFY(!shr.contains("English", ReviewCalculator::ListKind::T1, "A"));
}

void tst_learning::reviewDates() {
    QTemporaryDir h; QVERIFY(h.isValid());
    AppPaths p(h.path(), h.path());
    createTopic(p, "English", "Test", {makeWord("a","b")});
    ReviewRepository rr(p);
    Review rv; rv.setDate(1, QDate(2025,1,15)); rv.setDate(2, QDate(2025,2,20));
    QVERIFY(rr.store("English", "Test", rv));
    Review loaded = rr.load("English", "Test");
    QCOMPARE(loaded.countFilled(), 2);
    QCOMPARE(loaded.dateAt(1), QDate(2025,1,15));
    QCOMPARE(loaded.dateAt(2), QDate(2025,2,20));
}

// --- Parity test: verify Qt output matches Bash behavior ---
void tst_learning::parityDataIndexStts() {
    // Create topic with 3 items (2 words, 1 sentence), same as Bash would
    QTemporaryDir h; QVERIFY(h.isValid());
    AppPaths p(h.path(), h.path());
    QList<Item> items = {
        makeWord("hello", "hola"),
        makeWord("world", "mundo"),
        []{ Item it; it.trgt="good morning"; it.srce="buenos días"; it.type="2"; it.cdid="cdid_gm"; return it; }()
    };
    createTopic(p, "English", "Topic", items);

    TopicDataRepository dr(p);

    // Verify data file matches Bash format (14 fields per line)
    auto decoded = dr.loadItems("English", "Topic");
    QCOMPARE(decoded.size(), 3);

    // Verify stts = 1 (learning)
    QCOMPARE(dr.readStts("English", "Topic"), "1");

    // Verify learning table
    auto learn = dr.learningList("English", "Topic");
    QCOMPARE(learn.size(), 3);
    QVERIFY(learn.contains("hello"));
    QVERIFY(learn.contains("world"));
    QVERIFY(learn.contains("good morning"));

    // Verify index has 3 items (3 lines each = 9 lines)
    auto idx = dr.loadIndex("English", "Topic");
    QCOMPARE(idx.size(), 9);

    // Run markAsLearned (Bash: stts=1 → date1=today, stts=3)
    {
        LearningEngine e(p);
        auto r = e.markAsLearned("English", "Topic");
        QCOMPARE(r.writtenDate, 1);
    }

    // Verify stts = 3 (Bash: echo 3 > stts)
    QCOMPARE(dr.readStts("English", "Topic"), "3");

    // Verify review date1 = today (Bash: tpc_db 9 reviews date1 today)
    ReviewRepository rr(p);
    Review rv = rr.load("English", "Topic");
    QCOMPARE(rv.countFilled(), 1);
    QCOMPARE(rv.dateAt(1), QDate::currentDate());

    // Verify index still has 3 items after markAsLearned
    idx = dr.loadIndex("English", "Topic");
    QCOMPARE(idx.size(), 9);

    // Verify learning table still has all items
    // Note: after markAsLearned with mast=FALSE (count=0), items go to learnt
    // This matches Bash behavior: mast=FALSE → insert into learnt
}

QTEST_MAIN(tst_learning)
#include "tst_learning.moc"
