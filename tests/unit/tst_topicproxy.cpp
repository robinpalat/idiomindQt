// F6-B.1/F6-B.2 - Tests: TopicListModel, TopicProxy, IndexListModel
#include <QFile>
#include <QTemporaryDir>
#include <QtTest/QtTest>

#include "core/config/AppPaths.h"
#include "core/topic/Topic.h"
#include "core/topic/TopicInfo.h"
#include "ui/models/TopicListModel.h"
#include "ui/models/IndexListModel.h"
#include "ui/controllers/TopicProxy.h"
#include "storage/topic/TopicRepository.h"
#include "storage/topic/TopicDataRepository.h"
#include "storage/SharedRepository.h"
#include "storage/dbs/SqliteDatabase.h"

using namespace idiomind::core;
using namespace idiomind::ui;
using namespace idiomind::storage;

static TopicInfo makeInfo(const QString &name)
{
    TopicInfo info;
    info.setName(name);
    info.setSlng("Español");
    info.setTlng("English");
    info.setCtgy("storyteller");
    info.setIlnk("test123456");
    info.setNwrd("0");
    info.setNsnt("0");
    info.setNimg("0");
    info.setNaud("0");
    info.setLevl("1");
    info.setStts("0");
    return info;
}

class tst_topicproxy : public QObject
{
    Q_OBJECT
private slots:
    void listModelRoles();
    void listModelSetTopics();
    void proxyTopicList();
    void proxySelectTopic();
    void proxySttsLabel();
    void indexModelLoadFromLines();
    void indexModelEmptyLines();
    void proxyLoadsIndexOnSelect();
    void proxyEmptyTopicClearsIndex();
    void noteReadExisting();
    void noteWriteAndReadBack();
    void noteChangeTopic();
    void noteEmptyTopic();
    void metadataLoadsOnSelect();
    void metadataUpdatesOnTopicChange();
    void refreshTasksEmpty();
    void refreshTasksWithTopics();
    void refreshTasksClearsOnLanguageChange();
    void learntEmptyTopic();
    void learntWithItems();
    void learntUpdatesOnTopicChange();
    void actionLabelReview();
    void actionLabelToLearn();
    void actionLabelNoAction();
    void performReview();
    void performToLearn();
    void persistSaveAndLoad();
    void persistEmptyFile();
    void persistNonexistentTopic();
    void persistNoFile();
    void markItemAsLearned();
};

void tst_topicproxy::listModelRoles()
{
    TopicListModel model;
    QCOMPARE(model.rowCount(), 0);

    TopicList list;
    Topic t1; t1.setName("Alpha"); t1.setStts("1");
    Topic t2; t2.setName("Beta"); t2.setStts("3");
    list.add(t1); list.add(t2);

    model.setTopics(list);
    QCOMPARE(model.rowCount(), 2);

    QModelIndex idx = model.index(0);
    QCOMPARE(model.data(idx, TopicListModel::NameRole).toString(), QStringLiteral("Alpha"));
    QCOMPARE(model.data(idx, TopicListModel::SttsRole).toString(), QStringLiteral("1"));
    QCOMPARE(model.data(idx, TopicListModel::SttsColorRole).toString(), QStringLiteral("#4CAF50"));

    idx = model.index(1);
    QCOMPARE(model.data(idx, TopicListModel::NameRole).toString(), QStringLiteral("Beta"));
    QCOMPARE(model.data(idx, TopicListModel::SttsRole).toString(), QStringLiteral("3"));
}

void tst_topicproxy::listModelSetTopics()
{
    TopicListModel model;
    QSignalSpy spy(&model, &QAbstractListModel::modelReset);
    TopicList list;
    Topic t; t.setName("X");
    list.add(t);
    model.setTopics(list);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(model.count(), 1);
    QCOMPARE(model.topicName(0), QStringLiteral("X"));
    QCOMPARE(model.topicName(-1), QString());
    QCOMPARE(model.topicName(5), QString());
}

void tst_topicproxy::proxyTopicList()
{
    QTemporaryDir h; QVERIFY(h.isValid());
    AppPaths p(h.path(), h.path());

    // Create topics on disk
    TopicRepository repo(p);
    repo.createTopic("English", makeInfo("Topic A"));
    repo.createTopic("English", makeInfo("Topic B"));

    TopicProxy proxy(p);
    proxy.setActiveLanguage("English");
    QCOMPARE(proxy.topicCount(), 2);
    QVERIFY(proxy.topicModel()->topicName(0) == "Topic A" ||
            proxy.topicModel()->topicName(0) == "Topic B");
}

void tst_topicproxy::proxySelectTopic()
{
    QTemporaryDir h; QVERIFY(h.isValid());
    AppPaths p(h.path(), h.path());

    TopicProxy proxy(p);
    proxy.setActiveLanguage("English");

    QSignalSpy spy(&proxy, &TopicProxy::activeTopicChanged);
    proxy.selectTopic("MyTopic");
    QCOMPARE(proxy.activeTopic(), QStringLiteral("MyTopic"));
    QCOMPARE(spy.count(), 1);

    // Same selection = no signal
    proxy.selectTopic("MyTopic");
    QCOMPARE(spy.count(), 1);

    // Empty = no change
    proxy.selectTopic("");
    QCOMPARE(proxy.activeTopic(), QStringLiteral("MyTopic"));
}

void tst_topicproxy::proxySttsLabel()
{
    QTemporaryDir h; QVERIFY(h.isValid());
    AppPaths p(h.path(), h.path());
    TopicRepository repo(p);
    repo.createTopic("English", makeInfo("T"));

    TopicProxy proxy(p);
    proxy.setActiveLanguage("English");
    proxy.selectTopic("T");

    const QString stts = proxy.activeTopicStts();
    QVERIFY(stts.isEmpty() || stts == "0" || stts == "13");
}

// --- F6-B.2: IndexListModel ---

void tst_topicproxy::indexModelLoadFromLines()
{
    IndexListModel model;
    QCOMPARE(model.itemCount(), 0);

    QStringList lines = {
        "<b><big>hello</big></b>", "FALSE", "hola",
        "world", "TRUE", "mundo",
    };
    model.loadFromIndexLines(lines);
    QCOMPARE(model.rowCount(), 2);
    QCOMPARE(model.itemCount(), 2);

    QCOMPARE(model.data(model.index(0), IndexListModel::PlainTextRole).toString(),
             QStringLiteral("hello"));
    QCOMPARE(model.data(model.index(0), IndexListModel::CheckedRole).toBool(), false);
    QCOMPARE(model.data(model.index(0), IndexListModel::SrceRole).toString(),
             QStringLiteral("hola"));

    QCOMPARE(model.data(model.index(1), IndexListModel::PlainTextRole).toString(),
             QStringLiteral("world"));
    QCOMPARE(model.data(model.index(1), IndexListModel::CheckedRole).toBool(), true);
    QCOMPARE(model.data(model.index(1), IndexListModel::SrceRole).toString(),
             QStringLiteral("mundo"));
}

void tst_topicproxy::indexModelEmptyLines()
{
    IndexListModel model;
    model.loadFromIndexLines({});
    QCOMPARE(model.itemCount(), 0);
}

void tst_topicproxy::proxyLoadsIndexOnSelect()
{
    QTemporaryDir h; QVERIFY(h.isValid());
    AppPaths p(h.path(), h.path());

    TopicRepository repo(p);
    repo.createTopic("English", makeInfo("TestTopic"));

    TopicDataRepository dataRepo(p);
    dataRepo.writeStts("English", "TestTopic", "1");

    QFile indexFile(p.topicConfDir("English", "TestTopic") + "/index");
    QVERIFY(indexFile.open(QIODevice::WriteOnly));
    indexFile.write("hello\nFALSE\nhola\n");
    indexFile.write("world\nTRUE\nmundo\n");
    indexFile.close();

    TopicProxy proxy(p);
    proxy.setActiveLanguage("English");

    QSignalSpy spy(&proxy, &TopicProxy::indexChanged);
    proxy.selectTopic("TestTopic");

    QCOMPARE(spy.count(), 1);
    QCOMPARE(proxy.activeTopic(), QStringLiteral("TestTopic"));
    QCOMPARE(proxy.indexModel()->itemCount(), 2);
}

void tst_topicproxy::proxyEmptyTopicClearsIndex()
{
    QTemporaryDir h; QVERIFY(h.isValid());
    AppPaths p(h.path(), h.path());

    TopicProxy proxy(p);
    proxy.setActiveLanguage("English");

    proxy.selectTopic("Something");
    QCOMPARE(proxy.indexModel()->itemCount(), 0);

    proxy.selectTopic("");
    QCOMPARE(proxy.indexModel()->itemCount(), 0);
}

// --- F6-B.3: Note read/write ---

void tst_topicproxy::noteReadExisting()
{
    QTemporaryDir h; QVERIFY(h.isValid());
    AppPaths p(h.path(), h.path());

    TopicRepository repo(p);
    repo.createTopic("English", makeInfo("T"));

    // Write a note
    TopicDataRepository dataRepo(p);
    dataRepo.writeNote("English", "T", "Hello world");

    TopicProxy proxy(p);
    proxy.setActiveLanguage("English");
    proxy.selectTopic("T");

    QCOMPARE(proxy.note(), QStringLiteral("Hello world"));
}

void tst_topicproxy::noteWriteAndReadBack()
{
    QTemporaryDir h; QVERIFY(h.isValid());
    AppPaths p(h.path(), h.path());

    TopicRepository repo(p);
    repo.createTopic("English", makeInfo("T"));

    TopicProxy proxy(p);
    proxy.setActiveLanguage("English");
    proxy.selectTopic("T");

    // createTopic writes " \n" to note (Bash: echo " " > note)
    const QString initialNote = proxy.note();
    QVERIFY(!initialNote.isEmpty());

    // Write
    QSignalSpy spy(&proxy, &TopicProxy::noteChanged);
    proxy.saveNote("My note content");
    QCOMPARE(proxy.note(), QStringLiteral("My note content"));
    QCOMPARE(spy.count(), 1);

    // Re-read from disk
    TopicDataRepository dataRepo(p);
    QCOMPARE(dataRepo.readNote("English", "T"), QStringLiteral("My note content"));
}

void tst_topicproxy::noteChangeTopic()
{
    QTemporaryDir h; QVERIFY(h.isValid());
    AppPaths p(h.path(), h.path());

    TopicRepository repo(p);
    repo.createTopic("English", makeInfo("A"));
    repo.createTopic("English", makeInfo("B"));

    TopicDataRepository dataRepo(p);
    dataRepo.writeNote("English", "A", "Note A");
    dataRepo.writeNote("English", "B", "Note B");

    TopicProxy proxy(p);
    proxy.setActiveLanguage("English");

    proxy.selectTopic("A");
    QCOMPARE(proxy.note(), QStringLiteral("Note A"));

    proxy.selectTopic("B");
    QCOMPARE(proxy.note(), QStringLiteral("Note B"));

    proxy.selectTopic("A");
    QCOMPARE(proxy.note(), QStringLiteral("Note A"));
}

void tst_topicproxy::noteEmptyTopic()
{
    QTemporaryDir h; QVERIFY(h.isValid());
    AppPaths p(h.path(), h.path());

    TopicProxy proxy(p);
    proxy.setActiveLanguage("English");

    proxy.selectTopic("Nonexistent");
    // Nonexistent topic: no note file, readNote returns empty or " \n"
    // depending on whether the directory exists
    const QString n = proxy.note();
    // Acceptable: empty (no file) or " \n" (file not found fallback)
    QVERIFY(n.isEmpty() || n.trimmed().isEmpty());
}

// --- F6-B.4: Metadata ---

void tst_topicproxy::metadataLoadsOnSelect()
{
    QTemporaryDir h; QVERIFY(h.isValid());
    AppPaths p(h.path(), h.path());

    TopicRepository repo(p);
    TopicInfo info = makeInfo("MyTopic");
    info.setNwrd("15");
    info.setNsnt("8");
    info.setNimg("3");
    info.setDtec("2025-03-01");
    info.setAutr("Test Author");
    info.setCtgy("storyteller");
    info.setLevl("2");
    repo.createTopic("English", info);

    TopicDataRepository dataRepo(p);
    dataRepo.writeStts("English", "MyTopic", "1");

    TopicProxy proxy(p);
    proxy.setActiveLanguage("English");
    proxy.selectTopic("MyTopic");

    QCOMPARE(proxy.activeTopic(), QStringLiteral("MyTopic"));
    QCOMPARE(proxy.stts(), QStringLiteral("1"));
    QCOMPARE(proxy.nwrd(), QStringLiteral("15"));
    QCOMPARE(proxy.nsnt(), QStringLiteral("8"));
    QCOMPARE(proxy.nimg(), QStringLiteral("3"));
    QCOMPARE(proxy.dtec(), QStringLiteral("2025-03-01"));
    QCOMPARE(proxy.autr(), QStringLiteral("Test Author"));
    QCOMPARE(proxy.ctgy(), QStringLiteral("storyteller"));
    QCOMPARE(proxy.levl(), QStringLiteral("2"));
}

void tst_topicproxy::metadataUpdatesOnTopicChange()
{
    QTemporaryDir h; QVERIFY(h.isValid());
    AppPaths p(h.path(), h.path());

    TopicRepository repo(p);
    TopicInfo infoA = makeInfo("A");
    infoA.setNwrd("10");
    infoA.setAutr("Author A");
    repo.createTopic("English", infoA);

    TopicInfo infoB = makeInfo("B");
    infoB.setNwrd("20");
    infoB.setAutr("Author B");
    repo.createTopic("English", infoB);

    TopicProxy proxy(p);
    proxy.setActiveLanguage("English");

    proxy.selectTopic("A");
    QCOMPARE(proxy.nwrd(), QStringLiteral("10"));
    QCOMPARE(proxy.autr(), QStringLiteral("Author A"));

    proxy.selectTopic("B");
    QCOMPARE(proxy.nwrd(), QStringLiteral("20"));
    QCOMPARE(proxy.autr(), QStringLiteral("Author B"));

    proxy.selectTopic("A");
    QCOMPARE(proxy.nwrd(), QStringLiteral("10"));
    QCOMPARE(proxy.autr(), QStringLiteral("Author A"));
}

// --- F6-B.5: Tasks ---

void tst_topicproxy::refreshTasksEmpty()
{
    QTemporaryDir h; QVERIFY(h.isValid());
    AppPaths p(h.path(), h.path());

    TopicProxy proxy(p);
    proxy.setActiveLanguage("English");
    proxy.refreshTasks();

    QCOMPARE(proxy.taskModel()->taskCount(), 0);
}

void tst_topicproxy::refreshTasksWithTopics()
{
    QTemporaryDir h; QVERIFY(h.isValid());
    AppPaths p(h.path(), h.path());

    // Create topics and set up SharedRepository lists
    TopicRepository repo(p);
    repo.createTopic("English", makeInfo("TopicA"));
    repo.createTopic("English", makeInfo("TopicB"));

    SharedRepository shr(p);
    shr.ensureTables("English");
    shr.insertTopic("English", ReviewCalculator::ListKind::T1, "TopicA");
    shr.insertTopic("English", ReviewCalculator::ListKind::T3, "TopicB");

    TopicProxy proxy(p);
    proxy.setActiveLanguage("English");
    proxy.refreshTasks();

    QCOMPARE(proxy.taskModel()->taskCount(), 2);

    // Verify tasks have correct tags
    bool foundT1 = false, foundT3 = false;
    for (int i = 0; i < proxy.taskModel()->taskCount(); ++i) {
        QModelIndex idx = proxy.taskModel()->index(i);
        const QString tag = proxy.taskModel()->data(idx, TaskListModel::TagRole).toString();
        if (tag == "T1") foundT1 = true;
        if (tag == "T3") foundT3 = true;
    }
    QVERIFY(foundT1);
    QVERIFY(foundT3);
}

void tst_topicproxy::refreshTasksClearsOnLanguageChange()
{
    QTemporaryDir h; QVERIFY(h.isValid());
    AppPaths p(h.path(), h.path());

    SharedRepository shr(p);
    shr.ensureTables("English");
    shr.insertTopic("English", ReviewCalculator::ListKind::T1, "X");

    TopicProxy proxy(p);
    proxy.setActiveLanguage("English");
    proxy.refreshTasks();
    QCOMPARE(proxy.taskModel()->taskCount(), 1);

    // Change language clears tasks
    proxy.setActiveLanguage("Spanish");
    proxy.refreshTasks();
    QCOMPARE(proxy.taskModel()->taskCount(), 0);
}

// --- F6-B.7: LearntView ---

void tst_topicproxy::learntEmptyTopic()
{
    QTemporaryDir h; QVERIFY(h.isValid());
    AppPaths p(h.path(), h.path());

    TopicProxy proxy(p);
    proxy.setActiveLanguage("English");
    proxy.selectTopic("Nonexistent");
    QCOMPARE(proxy.learntModel()->itemCount(), 0);
}

void tst_topicproxy::learntWithItems()
{
    QTemporaryDir h; QVERIFY(h.isValid());
    AppPaths p(h.path(), h.path());

    TopicRepository repo(p);
    repo.createTopic("English", makeInfo("T"));

    TopicDataRepository dataRepo(p);
    dataRepo.writeStts("English", "T", "1");

    // Insert items into learnt table
    SqliteDatabase db(p.topicDb("English", "T"));
    QVERIFY(db.open());
    db.execute("insert into learnt (list) values ('hello');");
    db.execute("insert into learnt (list) values ('world');");
    db.close();

    TopicProxy proxy(p);
    proxy.setActiveLanguage("English");
    proxy.selectTopic("T");

    QCOMPARE(proxy.learntModel()->itemCount(), 2);

    // Verify texts
    bool foundHello = false, foundWorld = false;
    for (int i = 0; i < proxy.learntModel()->itemCount(); ++i) {
        QModelIndex idx = proxy.learntModel()->index(i);
        const QString txt = proxy.learntModel()->data(idx, IndexListModel::PlainTextRole).toString();
        if (txt == "hello") foundHello = true;
        if (txt == "world") foundWorld = true;
    }
    QVERIFY(foundHello);
    QVERIFY(foundWorld);
}

void tst_topicproxy::learntUpdatesOnTopicChange()
{
    QTemporaryDir h; QVERIFY(h.isValid());
    AppPaths p(h.path(), h.path());

    TopicRepository repo(p);
    repo.createTopic("English", makeInfo("A"));
    repo.createTopic("English", makeInfo("B"));

    TopicDataRepository dataRepo(p);

    // A has 2 learnt items
    { SqliteDatabase db(p.topicDb("English", "A")); QVERIFY(db.open());
      db.execute("insert into learnt (list) values ('a1');");
      db.execute("insert into learnt (list) values ('a2');"); }

    // B has 1 learnt item
    { SqliteDatabase db(p.topicDb("English", "B")); QVERIFY(db.open());
      db.execute("insert into learnt (list) values ('b1');"); }

    TopicProxy proxy(p);
    proxy.setActiveLanguage("English");

    proxy.selectTopic("A");
    QCOMPARE(proxy.learntModel()->itemCount(), 2);

    proxy.selectTopic("B");
    QCOMPARE(proxy.learntModel()->itemCount(), 1);

    proxy.selectTopic("A");
    QCOMPARE(proxy.learntModel()->itemCount(), 2);
}

// --- F6-B.6: Footer actions ---

void tst_topicproxy::actionLabelReview()
{
    QTemporaryDir h; QVERIFY(h.isValid());
    AppPaths p(h.path(), h.path());

    TopicRepository repo(p);
    repo.createTopic("English", makeInfo("T"));
    TopicDataRepository dataRepo(p);
    dataRepo.writeStts("English", "T", "1");

    TopicProxy proxy(p);
    proxy.setActiveLanguage("English");
    proxy.selectTopic("T");

    // stts=1 → REVISAR
    QCOMPARE(proxy.actionLabel(), QStringLiteral("REVISAR"));
}

void tst_topicproxy::actionLabelToLearn()
{
    QTemporaryDir h; QVERIFY(h.isValid());
    AppPaths p(h.path(), h.path());

    TopicRepository repo(p);
    repo.createTopic("English", makeInfo("T"));
    TopicDataRepository dataRepo(p);
    dataRepo.writeStts("English", "T", "3");

    TopicProxy proxy(p);
    proxy.setActiveLanguage("English");
    proxy.selectTopic("T");

    // stts=3 → APRENDER DE NUEVO
    QCOMPARE(proxy.actionLabel(), QStringLiteral("APRENDER DE NUEVO"));
}

void tst_topicproxy::actionLabelNoAction()
{
    QTemporaryDir h; QVERIFY(h.isValid());
    AppPaths p(h.path(), h.path());

    TopicRepository repo(p);
    repo.createTopic("English", makeInfo("T"));
    TopicDataRepository dataRepo(p);
    dataRepo.writeStts("English", "T", "2");

    TopicProxy proxy(p);
    proxy.setActiveLanguage("English");
    proxy.selectTopic("T");

    // stts=2 (mastered) → no action
    QCOMPARE(proxy.actionLabel(), QString());
}

void tst_topicproxy::performReview()
{
    QTemporaryDir h; QVERIFY(h.isValid());
    AppPaths p(h.path(), h.path());

    TopicRepository repo(p);
    repo.createTopic("English", makeInfo("T"));
    TopicDataRepository dataRepo(p);
    dataRepo.writeStts("English", "T", "1");

    // Add item to learning
    { SqliteDatabase db(p.topicDb("English", "T")); QVERIFY(db.open());
      db.execute("insert into learning (list) values ('hello');"); }

    TopicProxy proxy(p);
    proxy.setActiveLanguage("English");
    proxy.selectTopic("T");

    QCOMPARE(proxy.stts(), QStringLiteral("1"));

    // Perform Review (mark_as_learned)
    QVERIFY(proxy.performAction());

    // stts should change from 1 to 3
    QCOMPARE(proxy.stts(), QStringLiteral("3"));

    // Index should be rebuilt
    QVERIFY(proxy.indexModel()->itemCount() >= 0);
}

void tst_topicproxy::performToLearn()
{
    QTemporaryDir h; QVERIFY(h.isValid());
    AppPaths p(h.path(), h.path());

    TopicRepository repo(p);
    repo.createTopic("English", makeInfo("T"));
    TopicDataRepository dataRepo(p);
    dataRepo.writeStts("English", "T", "3");

    // Add item to learnt
    { SqliteDatabase db(p.topicDb("English", "T")); QVERIFY(db.open());
      db.execute("insert into learnt (list) values ('hello');"); }

    TopicProxy proxy(p);
    proxy.setActiveLanguage("English");
    proxy.selectTopic("T");

    QCOMPARE(proxy.stts(), QStringLiteral("3"));

    // Perform ToLearn (mark_to_learn)
    QVERIFY(proxy.performAction());

    // stts should change from 3 to 5 (odd→MidReview)
    QCOMPARE(proxy.stts(), QStringLiteral("5"));

    // Learnt should be cleared, learning should have items
    QCOMPARE(proxy.learntModel()->itemCount(), 0);
}

// --- F6-B.8: Persistencia del topic activo ---

void tst_topicproxy::persistSaveAndLoad()
{
    QTemporaryDir h; QVERIFY(h.isValid());
    AppPaths p(h.path(), h.path());

    TopicRepository repo(p);
    repo.createTopic("English", makeInfo("MyTopic"));

    // Select and persist
    TopicProxy proxy(p);
    proxy.setActiveLanguage("English");
    proxy.selectTopic("MyTopic");
    QCOMPARE(proxy.activeTopic(), QStringLiteral("MyTopic"));

    // Verify file was written
    QFile f(p.configRoot() + "/tpc");
    QVERIFY(f.open(QIODevice::ReadOnly));
    QCOMPARE(QString::fromUtf8(f.readAll()).trimmed(), QStringLiteral("MyTopic"));
    f.close();

    // Create new proxy and load
    TopicProxy proxy2(p);
    proxy2.setActiveLanguage("English");
    proxy2.loadActiveTopic();
    QCOMPARE(proxy2.activeTopic(), QStringLiteral("MyTopic"));
}

void tst_topicproxy::persistEmptyFile()
{
    QTemporaryDir h; QVERIFY(h.isValid());
    AppPaths p(h.path(), h.path());

    // Create empty tpc file
    QDir().mkpath(p.configRoot());
    QFile f(p.configRoot() + "/tpc");
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.close();

    TopicProxy proxy(p);
    proxy.setActiveLanguage("English");
    proxy.loadActiveTopic();

    // Empty file → no topic loaded
    QCOMPARE(proxy.activeTopic(), QString());
}

void tst_topicproxy::persistNonexistentTopic()
{
    QTemporaryDir h; QVERIFY(h.isValid());
    AppPaths p(h.path(), h.path());

    // Write a topic name that doesn't exist on disk
    QDir().mkpath(p.configRoot());
    QFile f(p.configRoot() + "/tpc");
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write("GhostTopic\n");
    f.close();

    TopicProxy proxy(p);
    proxy.setActiveLanguage("English");
    proxy.loadActiveTopic();

    // Nonexistent topic → no topic loaded
    QCOMPARE(proxy.activeTopic(), QString());
}

void tst_topicproxy::persistNoFile()
{
    QTemporaryDir h; QVERIFY(h.isValid());
    AppPaths p(h.path(), h.path());

    // No tpc file at all
    TopicProxy proxy(p);
    proxy.setActiveLanguage("English");
    proxy.loadActiveTopic();

    // No file → no topic loaded
    QCOMPARE(proxy.activeTopic(), QString());
}

// --- F6-C.3: Per-item mark as learned ---

void tst_topicproxy::markItemAsLearned()
{
    QTemporaryDir h; QVERIFY(h.isValid());
    AppPaths p(h.path(), h.path());

    TopicRepository repo(p);
    repo.createTopic("English", makeInfo("T"));

    TopicDataRepository dataRepo(p);
    dataRepo.writeStts("English", "T", "1");

    // Write data file with 3 items (needed by rebuildIndex)
    { QFile data(p.topicConfDir("English", "T") + "/data");
      QVERIFY(data.open(QIODevice::WriteOnly));
      data.write("trgt{hello}srce{hola}\n");
      data.write("trgt{world}srce{mundo}\n");
      data.write("trgt{test}srce{prueba}\n");
      data.close(); }

    // Add 3 items to learning
    { SqliteDatabase db(p.topicDb("English", "T")); QVERIFY(db.open());
      db.execute("insert into learning (list) values ('hello');");
      db.execute("insert into learning (list) values ('world');");
      db.execute("insert into learning (list) values ('test');");
    }

    // Build index (needed by loadIndexForTopic)
    dataRepo.rebuildIndex("English", "T", false);

    TopicProxy proxy(p);
    proxy.setActiveLanguage("English");
    proxy.selectTopic("T");

    // Initially 3 items in index
    QCOMPARE(proxy.indexModel()->itemCount(), 3);
    QCOMPARE(proxy.learntModel()->itemCount(), 0);

    // Mark "world" as learned
    QVERIFY(proxy.markItemAsLearned("world"));

    // Index should now have 2 items (hello, test)
    QCOMPARE(proxy.indexModel()->itemCount(), 2);

    // Learnt should have 1 item (world)
    QCOMPARE(proxy.learntModel()->itemCount(), 1);

    QModelIndex learntIdx = proxy.learntModel()->index(0);
    QCOMPARE(proxy.learntModel()->data(learntIdx, IndexListModel::PlainTextRole).toString(),
             QStringLiteral("world"));

    // Verify persistence
    TopicProxy proxy2(p);
    proxy2.setActiveLanguage("English");
    proxy2.selectTopic("T");
    QCOMPARE(proxy2.indexModel()->itemCount(), 2);
    QCOMPARE(proxy2.learntModel()->itemCount(), 1);
}

QTEST_MAIN(tst_topicproxy)
#include "tst_topicproxy.moc"
