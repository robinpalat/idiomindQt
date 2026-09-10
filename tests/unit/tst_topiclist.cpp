// F2.2 - tst_topiclist: colección ordenada de Topic (dominio puro, sin I/O).
// Autoridad: docs/09-qt-architecture.md §3.8, mngr.sh mkmn / add.sh new_topic.
// El ORDEN lo impone TopicRepository::listTopics (mkmn); aquí se preserva el
// de inserción y se prueban add/remove/findByName/findByPath/contains.
#include <QTest>

#include "core/topic/Topic.h"
#include "core/topic/TopicList.h"

using idiomind::core::Topic;
using idiomind::core::TopicList;

class TestTopicList : public QObject
{
    Q_OBJECT

private slots:
    void addAppendsPreservingInsertionOrder();
    void addRejectsDuplicateNames();
    void findByNameAndContains();
    void removeByName();
    void findByPathResolvesDataAndConfDirs();
    void findByPathValidatesAgainstRoot();
    void clearAndCount();
};

static Topic makeTopic(const QString &name)
{
    Topic t;
    t.setName(name);
    return t;
}

void TestTopicList::addAppendsPreservingInsertionOrder()
{
    TopicList list;
    QVERIFY(list.add(makeTopic(QStringLiteral("B"))));
    QVERIFY(list.add(makeTopic(QStringLiteral("A"))));
    QVERIFY(list.add(makeTopic(QStringLiteral("C"))));

    // La colección NO reordena: se preserva el orden de inserción; el orden
    // mkmn (find | ls -tNd) lo aplica el repositorio antes de poblar.
    QCOMPARE(list.count(), 3);
    QCOMPARE(list.at(0).name(), QStringLiteral("B"));
    QCOMPARE(list.at(1).name(), QStringLiteral("A"));
    QCOMPARE(list.at(2).name(), QStringLiteral("C"));
    QVERIFY(!list.isEmpty());
}

void TestTopicList::addRejectsDuplicateNames()
{
    TopicList list;
    QVERIFY(list.add(makeTopic(QStringLiteral("My Topic"))));
    QVERIFY(!list.add(makeTopic(QStringLiteral("My Topic"))));
    QCOMPARE(list.count(), 1);

    // Nombres con espacios/unicode son distintos si el texto difiere.
    QVERIFY(list.add(makeTopic(QStringLiteral("My Topic "))));
    QVERIFY(list.add(makeTopic(QStringLiteral("日本語"))));
    QCOMPARE(list.count(), 3);
}

void TestTopicList::findByNameAndContains()
{
    TopicList list;
    list.add(makeTopic(QStringLiteral("Phrasal verbs")));
    list.add(makeTopic(QStringLiteral("My Topic")));

    const Topic *found = list.findByName(QStringLiteral("My Topic"));
    QVERIFY(found != nullptr);
    QCOMPARE(found->name(), QStringLiteral("My Topic"));
    QVERIFY(list.contains(QStringLiteral("Phrasal verbs")));
    QVERIFY(!list.contains(QStringLiteral("nonexistent")));
    QCOMPARE(list.findByName(QStringLiteral("nonexistent")), nullptr);

    // búsqueda por coincidencia exacta (no substring)
    QCOMPARE(list.findByName(QStringLiteral("Topic")), nullptr);

    // el overload const devuelve puntero equivalente de solo lectura
    const TopicList &clist = list;
    QCOMPARE(clist.findByName(QStringLiteral("Phrasal verbs"))->name(),
             QStringLiteral("Phrasal verbs"));
    QCOMPARE(clist.findByName(QStringLiteral("My Topic"))->name(),
             QStringLiteral("My Topic"));
}

void TestTopicList::removeByName()
{
    TopicList list;
    list.add(makeTopic(QStringLiteral("A")));
    list.add(makeTopic(QStringLiteral("B")));

    QVERIFY(list.remove(QStringLiteral("A")));
    QCOMPARE(list.count(), 1);
    QCOMPARE(list.at(0).name(), QStringLiteral("B"));

    QVERIFY(!list.remove(QStringLiteral("A"))); // ya no estaba
    QCOMPARE(list.count(), 1);
}

void TestTopicList::findByPathResolvesDataAndConfDirs()
{
    TopicList list;
    list.add(makeTopic(QStringLiteral("My Topic")));

    // DM_tlt: .../topics/English/My Topic
    Topic *viaData = list.findByPath(QStringLiteral(
        "/home/user/.idiomind/topics/English/My Topic"));
    QVERIFY(viaData != nullptr);
    QCOMPARE(viaData->name(), QStringLiteral("My Topic"));

    // DC_tlt: misma ruta + .conf
    Topic *viaConf = list.findByPath(QStringLiteral(
        "/home/user/.idiomind/topics/English/My Topic/.conf"));
    QVERIFY(viaConf != nullptr);
    QCOMPARE(viaConf->name(), QStringLiteral("My Topic"));

    // ruta con .conf pero el topic no está en la lista
    QCOMPARE(list.findByPath(QStringLiteral("/x/y/Unlisted/.conf")), nullptr);
    QCOMPARE(list.findByPath(QString()), nullptr);
}

void TestTopicList::findByPathValidatesAgainstRoot()
{
    TopicList list;
    list.add(makeTopic(QStringLiteral("My Topic")));

    const QString root = QStringLiteral("/home/user/.idiomind/topics/English");
    QVERIFY(list.findByPath(root + QStringLiteral("/My Topic"), root) != nullptr);
    QVERIFY(list.findByPath(root + QStringLiteral("/My Topic/.conf"), root) != nullptr);

    // Ruta que no deriva de la raíz indicada -> nullptr (evita matches falsos
    // con un topic del mismo nombre en otro idioma).
    QCOMPARE(list.findByPath(QStringLiteral("/other/root/My Topic"), root),
             nullptr);
    QCOMPARE(list.findByPath(root + QStringLiteral("/Other Topic"), root),
             nullptr);
}

void TestTopicList::clearAndCount()
{
    TopicList list;
    QVERIFY(list.isEmpty());
    list.add(makeTopic(QStringLiteral("A")));
    list.add(makeTopic(QStringLiteral("B")));
    QCOMPARE(list.count(), 2);
    list.clear();
    QVERIFY(list.isEmpty());
    QCOMPARE(list.count(), 0);
}

QTEST_APPLESS_MAIN(TestTopicList)
#include "tst_topiclist.moc"