// F3.2 - tests del codec .idmnd (parse / write / round-trip byte-a-byte).
//
// Se usan los 4 ficheros reales de fixtures/idmnd (vía la macro
// IDIOMIND_TEST_FIXTURE_DIR definida en tests/unit/CMakeLists.txt):
//   * I Want to Swim.idmnd         -> 25 type=2, clave con \"
//   * Most common prepositions...  -> 17 type=1, imgr no vacío
//   * Nazca Lines.idmnd            -> 35 (13 type=1 + 22 type=2)
//   * Algunas rarezas...           -> 20 type=2, info largo con comas
#include <QFile>
#include <QtTest/QtTest>

#include "core/topic/Item.h"
#include "core/topic/TopicInfo.h"
#include "storage/interchange/IdmndCodec.h"

using namespace idiomind::storage;
using namespace idiomind::core;

static QByteArray readFixture(const QString &name)
{
    QFile f(QLatin1String(IDIOMIND_TEST_FIXTURE_DIR) + '/' + name);
    if (!f.open(QIODevice::ReadOnly)) return {};
    return f.readAll();
}

class tst_idmndcodec : public QObject
{
    Q_OBJECT
private slots:
    void parseFixtures();
    void writeRoundtripBytes();
    void itemOrderPreserved();
    void escaping();
    void emptyItems();
    void parseErrors();
};

void tst_idmndcodec::parseFixtures()
{
    struct Expect {
        const char *file;
        int items;
        const char *name, *slng, *tlng;
        const char *nwrd, *nsnt, *nimg, *levl, *stts;
    };
    const Expect ex[] = {
        {"I Want to Swim.idmnd", 25, "I Want to Swim", "Español", "English",
         "0", "25", "0", "1", "0"},
        {"Most common prepositions in English.idmnd", 17,
         "Most common prepositions in English", "Español", "English",
         "17", "0", "0", "0", "0"},
        {"Nazca Lines.idmnd", 35, "Nazca Lines", "Español", "English",
         "13", "22", "12", "1", "0"},
        {"Algunas rarezas del idioma ingles.idmnd", 20,
         "Algunas rarezas del idioma ingles", "Español", "English",
         "0", "60", "0", "2", "0"},
    };
    for (const Expect &e : ex) {
        bool ok = false;
        QString why;
        const IdmndDocument doc = IdmndCodec::parseFile(
            QLatin1String(IDIOMIND_TEST_FIXTURE_DIR) + '/' + e.file,
            &ok, &why);
        QVERIFY2(ok, qPrintable(QString("parse %1: %2").arg(e.file, why)));
        QCOMPARE(doc.items.size(), static_cast<int>(e.items));
        QCOMPARE(doc.info.name(), QLatin1String(e.name));
        QCOMPARE(doc.info.slng(), QString::fromUtf8(e.slng));
        QCOMPARE(doc.info.tlng(), QLatin1String(e.tlng));
        QCOMPARE(doc.info.nwrd(), QLatin1String(e.nwrd));
        QCOMPARE(doc.info.nsnt(), QLatin1String(e.nsnt));
        QCOMPARE(doc.info.nimg(), QLatin1String(e.nimg));
        QCOMPARE(doc.info.levl(), QLatin1String(e.levl));
        QCOMPARE(doc.info.stts(), QLatin1String(e.stts));
        QVERIFY(!doc.info.info().isEmpty());
    }
}

void tst_idmndcodec::writeRoundtripBytes()
{
    const QStringList files = {
        "I Want to Swim.idmnd",
        "Most common prepositions in English.idmnd",
        "Nazca Lines.idmnd",
        "Algunas rarezas del idioma ingles.idmnd"};
    for (const QString &f : files) {
        const QByteArray original = readFixture(f);
        bool ok = false;
        const IdmndDocument doc = IdmndCodec::parse(original, &ok);
        QVERIFY(ok);
        const QByteArray emitted = IdmndCodec::write(doc);
        QCOMPARE(emitted, original);
    }
}

void tst_idmndcodec::itemOrderPreserved()
{
    const QByteArray nazca = readFixture("Nazca Lines.idmnd");
    bool ok = false;
    const IdmndDocument doc = IdmndCodec::parse(nazca, &ok);
    QVERIFY(ok);
    QCOMPARE(doc.items.size(), 35);
    QVERIFY(doc.items.first().trgt.startsWith(
                QLatin1String("The Nazca Lines are a ")));
    QVERIFY(doc.items.last().trgt == QLatin1String("Squatters"));
    const IdmndDocument doc2 = IdmndCodec::parse(IdmndCodec::write(doc), &ok);
    QVERIFY(ok);
    QCOMPARE(doc2.items.size(), 35);
    for (int i = 0; i < doc.items.size(); ++i)
        QCOMPARE(doc2.items[i].trgt, doc.items[i].trgt);
}

void tst_idmndcodec::escaping()
{
    IdmndDocument doc;
    doc.info.setName(QChar(0x201C) + QLatin1String("quote") + QChar(0x201D));
    doc.info.setInfo(QStringLiteral("a\"b\\c/d\ne"));
    Item item;
    item.trgt = QStringLiteral("say \"hi\" \\ path");
    item.srce = QStringLiteral("dijo \"hola\"");
    item.note = QStringLiteral("line1\nline2\t\b\f\r");
    item.type = QStringLiteral("1");
    item.translations.insert(QStringLiteral("ch"), QStringLiteral("'"));
    doc.items.append(item);

    const QByteArray out = IdmndCodec::write(doc);
    QVERIFY(out.contains("\\\""));
    QVERIFY(out.contains("\\\\"));
    QVERIFY(out.contains(QByteArray("\xe2\x80\x9c", 3)));

    bool ok = false;
    const IdmndDocument round = IdmndCodec::parse(out, &ok);
    QVERIFY(ok);
    QCOMPARE(round.info.name(), doc.info.name());
    QCOMPARE(round.info.info(), doc.info.info());
    QCOMPARE(round.items.first().trgt, doc.items.first().trgt);
    QCOMPARE(round.items.first().srce, doc.items.first().srce);
    QCOMPARE(round.items.first().note, doc.items.first().note);
    QCOMPARE(round.items.first().translations.value(QStringLiteral("ch")),
             QStringLiteral("'"));
    QCOMPARE(round.items.first().type, QStringLiteral("1"));
}

void tst_idmndcodec::emptyItems()
{
    IdmndDocument doc;
    doc.info.setName(QLatin1String("empty"));
    bool ok = false;
    const QByteArray out = IdmndCodec::write(doc);
    const IdmndDocument doc2 = IdmndCodec::parse(out, &ok);
    QVERIFY(ok);
    QVERIFY(doc2.items.isEmpty());
    QCOMPARE(doc2.info.name(), QLatin1String("empty"));
}

void tst_idmndcodec::parseErrors()
{
    { bool ok = false; QString why; IdmndCodec::parse("basura", &ok, &why);
      QVERIFY(!ok); QVERIFY(!why.isEmpty()); }
    { bool ok = false; QString why;
      IdmndCodec::parse("{\"items\":{\n\"k\":{}}\n", &ok, &why);
      QVERIFY(!ok); QVERIFY(why.contains(QLatin1String("Lines"))); }
    { bool ok = false; QString why;
      IdmndCodec::parse("{\"items\":[\n\"k\":{}},\n\"name\":\"x\",\"stts\":\"0\"}\n",
                        &ok, &why);
      QVERIFY(!ok); }
    { bool ok = false; QString why;
      IdmndCodec::parse("{\"items\":{\n\"k\":{}},\nnot json\n", &ok, &why);
      QVERIFY(!ok); }
}

QTEST_MAIN(tst_idmndcodec)
#include "tst_idmndcodec.moc"
