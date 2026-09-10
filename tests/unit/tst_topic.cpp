// F2.2 - tst_topic: agregado de dominio Topic (TopicInfo + ítems + nota).
// Autoridad: docs/09-qt-architecture.md §3.9, docs/05-topic-format.md,
// ifs/tpc.sh chek_topic, tls.sh check_index. Dominio puro, sin I/O.
#include <QTest>
#include <QtGlobal>

#include "core/topic/Item.h"
#include "core/topic/Topic.h"
#include "core/topic/TopicInfo.h"

using namespace idiomind::core;
using idiomind::core::Topic;

class TestTopic : public QObject
{
    Q_OBJECT

private slots:
    void noteDefaultIsASingleSpace();
    void statusValueParsesOnlyValidLearningStates();
    void infoNameIsTopicIdentity();
    void itemsRoundTrip();
    void equalityComparisons();
};

void TestTopic::noteDefaultIsASingleSpace()
{
    // Original: `echo " " > "${DC_tlt}/note"` (chek_topic). El valor por
    // defecto del Topic NO es cadena vacía: es un único espacio.
    Topic t;
    QCOMPARE(t.note(), QStringLiteral(" "));

    t.setNote(QStringLiteral("my note"));
    QCOMPARE(t.note(), QStringLiteral("my note"));
}

void TestTopic::statusValueParsesOnlyValidLearningStates()
{
    Topic t;
    // Inválidos/vacíos -> 13 (Corrupted), igual que el Bash:
    //   `! [[ $stts =~ $numer ]] && stts=13`
    t.setStts(QString());
    QCOMPARE(t.statusValue(), 13);
    t.setStts(QStringLiteral("abc"));
    QCOMPARE(t.statusValue(), 13);
    t.setStts(QStringLiteral(" 1 "));
    QCOMPARE(t.statusValue(), 13); // texto con espacios: no numérico puro

    // Estados válidos del original (1..10, y 0 para desactivado... los
    // valores de aprendizaje válidos son 1..10; 0 también es numérico).
    t.setStts(QStringLiteral("0"));
    QCOMPARE(t.statusValue(), 0);
    t.setStts(QStringLiteral("1"));
    QCOMPARE(t.statusValue(), 1);
    t.setStts(QStringLiteral("2"));
    QCOMPARE(t.statusValue(), 2);
    t.setStts(QStringLiteral("10"));
    QCOMPARE(t.statusValue(), 10);
    t.setStts(QStringLiteral("13"));
    QCOMPARE(t.statusValue(), 13);
    t.setStts(QStringLiteral("99"));
    QCOMPARE(t.statusValue(), 99); // numérico fuera de rango: se conserva
}

void TestTopic::infoNameIsTopicIdentity()
{
    Topic t;
    t.setName(QStringLiteral("My Topic"));
    QCOMPARE(t.name(), QStringLiteral("My Topic"));

    TopicInfo info;
    info.setName(QStringLiteral("Phrasal verbs"));
    info.setSlng(QStringLiteral("es"));
    info.setTlng(QStringLiteral("English"));
    info.setStts(QStringLiteral("4"));
    t.setInfo(info);

    QCOMPARE(t.name(), QStringLiteral("Phrasal verbs"));
    QCOMPARE(t.stts(), QStringLiteral("4"));
    QCOMPARE(t.statusValue(), 4);
    QCOMPARE(t.info().name(), QStringLiteral("Phrasal verbs"));
    QCOMPARE(t.info().slng(), QStringLiteral("es"));
    QCOMPARE(t.info().tlng(), QStringLiteral("English"));

    t.setStts(QStringLiteral("7"));
    QCOMPARE(t.info().stts(), QStringLiteral("7")); // delegada en TopicInfo
}

void TestTopic::itemsRoundTrip()
{
    Topic t;
    QVERIFY(t.isEmpty());
    QCOMPARE(t.items().size(), 0);

    Item a;
    a.trgt = QStringLiteral("hello");
    a.srce = QStringLiteral("hola");
    a.type = QStringLiteral("1");
    t.addItem(a);

    QCOMPARE(t.items().size(), 1);
    QVERIFY(!t.isEmpty());
    QCOMPARE(t.items().at(0).trgt, QStringLiteral("hello"));

    t.setItems({});
    QVERIFY(t.isEmpty());
}

void TestTopic::equalityComparisons()
{
    const Topic a = [] {
        Topic t;
        t.setName(QStringLiteral("A"));
        t.setStts(QStringLiteral("3"));
        return t;
    }();
    const Topic b = [] {
        Topic t;
        t.setName(QStringLiteral("A"));
        t.setStts(QStringLiteral("3"));
        return t;
    }();
    const Topic c = [] {
        Topic t;
        t.setName(QStringLiteral("B"));
        t.setStts(QStringLiteral("3"));
        return t;
    }();

    QVERIFY(a == b);
    QVERIFY(a != c);
    QVERIFY(!(a != b));

    Topic d = a;
    d.setNote(QStringLiteral("different"));
    QVERIFY(a != d);

    Topic e = a;
    e.addItem(Item());
    QVERIFY(a != e); // los ítems forman parte de la igualdad
}

QTEST_APPLESS_MAIN(TestTopic)
#include "tst_topic.moc"