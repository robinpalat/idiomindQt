// F2.1 - FlatItemCodec + Item: línea plana original <-> Item, y Item JSON .idmnd.
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QString>
#include <QtTest/QtTest>

#include "core/topic/Item.h"
#include "core/topic/FlatItemCodec.h"

using namespace idiomind::core;
using namespace idiomind::core::FlatItemCodec;

namespace {

Item sampleWord()
{
    Item it;
    it.trgt = QStringLiteral("book");
    it.srce = QStringLiteral("libro");
    it.exmp = QStringLiteral("I read a book.");
    it.defn = QStringLiteral("a set of printed pages");
    it.note = QStringLiteral("n.");
    it.wrds = QStringLiteral("book_libro_");
    it.grmr = QStringLiteral("<span color='#3E539A'>book</span>");
    it.tags = QStringLiteral("school");
    it.mark = QStringLiteral("TRUE");
    it.refr = QStringLiteral("https://example.com/book");
    it.imag = QStringLiteral("2");
    it.imgr = QStringLiteral("book");
    it.link = QStringLiteral("");
    it.cdid = QStringLiteral("aabbccddeeff00112233445566778899");
    it.type = QStringLiteral("1");
    return it;
}

Item sampleSentence()
{
    Item it;
    it.trgt = QStringLiteral("\"When will you teach me how to swim?\" she asked.");
    it.srce = QStringLiteral("\u00bfCu\u00e1ndo me ense\u00f1ar\u00e1 a nadar? ella pregunt\u00f3.");
    it.exmp = QString();
    it.defn = QString();
    it.note = QString();
    it.wrds = QStringLiteral("When_Cuando_Asked_Pregunt\u00f3_You_T\u00fa_");
    it.grmr = QStringLiteral("<span color='#9C68BD'>\"When</span>");
    it.tags = QString();
    it.mark = QString();
    it.refr = QString();
    it.imag = QStringLiteral("0");
    it.imgr = QString();
    it.link = QString();
    it.cdid = QStringLiteral("d63633e9df7635e0b7271d0bec358674");
    it.type = QStringLiteral("2");
    return it;
}

// Lee un fixture .idmnd de 3 líneas (permitiendo `\n` final) y devuelve el
// documento JSON completo equivalente.
QJsonDocument loadIdmnd(const char *name, int *itemCount = nullptr)
{
    const QString path =
        QString::fromLatin1(IDIOMIND_TEST_FIXTURE_DIR) + QString::fromLatin1(name);
    if (!QFile::exists(path))
        return {};
    QFile f(path);
    f.open(QIODevice::ReadOnly | QIODevice::Text);
    QStringList lines = QString::fromUtf8(f.readAll()).split(QLatin1Char('\n'));
    while (!lines.isEmpty() && lines.constLast().trimmed().isEmpty())
        lines.removeLast();
    if (lines.size() != 3)
        return {};
    const QJsonDocument doc =
        QJsonDocument::fromJson((lines.at(0) + lines.at(1) + lines.at(2)).toUtf8());
    if (doc.isObject() && itemCount) {
        const QJsonObject items = doc.object().value(QStringLiteral("items")).toObject();
        if (itemCount)
            *itemCount = items.size();
    }
    return doc;
}

}  // namespace

class TstFlatItemCodec : public QObject {
    Q_OBJECT
private slots:
    // -- línea plana ----------------------------------------------------------
    void ascii();
    void unicode();
    void emptyFields();
    void apostrophes();
    void quotes();
    void wordSentence();
    void roundTrip();
    void legacyExtraFields();
    void embeddedCloseBraceIsLossyLikeOriginal();
    // -- JSON .idmnd ----------------------------------------------------------
    void itemJsonFieldOrder();
    void itemJsonRoundTrip();
    void multipleTranslationsJson();
    // -- fixtures reales ------------------------------------------------------
    void realFixturesDecodeToFlatConsistent();
};

void TstFlatItemCodec::ascii()
{
    // imgr NO vive en la línea plana (sólo en el JSON .idmnd): lo quitamos
    // para el round-trip flat como haría el propio get_item.
    Item it = sampleWord();
    it.imgr.clear();
    const QString line = FlatItemCodec::encode(it);
    QCOMPARE(line,
             QStringLiteral("trgt{book}srce{libro}exmp{I read a book.}"
                            "defn{a set of printed pages}note{n.}"
                            "wrds{book_libro_}grmr{<span color='#3E539A'>book</span>}"
                            "tags{school}mark{TRUE}refr{https://example.com/book}"
                            "imag{2}link{}cdid{aabbccddeeff00112233445566778899}"
                            "type{1}"));

    const Item back = FlatItemCodec::decode(line);
    QCOMPARE(back, it);
    QVERIFY(back.isWord());

    // exactamente 14 campos, orden canónico de default/vars línea 2
    QStringList names;
    for (int i = 0; i < ItemDefs::flatFieldCount(); ++i)
        names << QLatin1String(ItemDefs::flatFieldName(i));
    QCOMPARE(names,
             QStringList({"trgt", "srce", "exmp", "defn", "note", "wrds",
                          "grmr", "tags", "mark", "refr", "imag", "link",
                          "cdid", "type"}));
}

void TstFlatItemCodec::unicode()
{
    Item it;
    it.trgt = QStringLiteral("se\u00f1or");
    it.srce = QStringLiteral("\u00bfCu\u00e1ndo? \u2014 \u00e1\u00e9\u00ed\u00f3\u00fa\u00f1\u00fc");
    it.exmp = QStringLiteral("\u201cL\u00e9eme esto\u201d");
    it.note = QStringLiteral("\u4e2d\u6587 caf\u00e9");
    it.wrds = QStringLiteral("Se\u00f1or_Sr._");
    it.type = QStringLiteral("2");

    const QString line = FlatItemCodec::encode(it);
    QCOMPARE(FlatItemCodec::decode(line), it);
    QVERIFY(line.contains(QStringLiteral("\u00e1\u00e9\u00ed\u00f3\u00fa\u00f1\u00fc")));
    QVERIFY(line.contains(QStringLiteral("\u00bfCu\u00e1ndo?")));
    QVERIFY(line.contains(QStringLiteral("\u201cL\u00e9eme esto\u201d")));
    QVERIFY(line.contains(QStringLiteral("\u4e2d\u6587")));
}

void TstFlatItemCodec::emptyFields()
{
    Item it;
    it.trgt = QStringLiteral("a");
    it.type = QStringLiteral("1");
    const QString line = FlatItemCodec::encode(it);
    // campos vacíos emiten `campo{}` — el original los conserva
    QCOMPARE(line,
             QStringLiteral("trgt{a}srce{}exmp{}defn{}note{}wrds{}grmr{}"
                            "tags{}mark{}refr{}imag{}link{}cdid{}type{1}"));
    const Item back = FlatItemCodec::decode(line);
    QCOMPARE(back.trgt, QStringLiteral("a"));
    QVERIFY(back.srce.isEmpty());
    QVERIFY(back.note.isEmpty());
    QCOMPARE(back.cdid, QString());
    QCOMPARE(back.type, QStringLiteral("1"));
    QCOMPARE(back, it);
}

void TstFlatItemCodec::apostrophes()
{
    Item it;
    it.trgt = QStringLiteral("Don't Be Late");       // apóstrofe ASCII
    it.srce = QStringLiteral("No llegues tarde. No\u2019s\u2026 \u2018yo\u2019");
    it.exmp = QStringLiteral("don\u2019t / don't / \u2018quoted\u2019");
    it.type = QStringLiteral("2");

    const QString line = FlatItemCodec::encode(it);
    QCOMPARE(FlatItemCodec::decode(line), it);
    QVERIFY(line.contains(QStringLiteral("Don't Be Late")));
}

void TstFlatItemCodec::quotes()
{
    // el trgt lleva comillas dobles literales (como los fixtures reales)
    const Item it = sampleSentence();
    const QString line = FlatItemCodec::encode(it);
    const Item back = FlatItemCodec::decode(line);
    QCOMPARE(back, it);
    QVERIFY(back.trgt.startsWith(QLatin1Char('"')));
    QVERIFY(back.trgt.contains(QStringLiteral("\" she asked.")));
    QCOMPARE(back.srce, it.srce);
}

void TstFlatItemCodec::wordSentence()
{
    QVERIFY(sampleWord().isWord());
    QVERIFY(!sampleWord().isSentence());
    QVERIFY(!sampleSentence().isWord());
    QVERIFY(sampleSentence().isSentence());

    Item other;
    other.trgt = QStringLiteral("x");
    other.type = QStringLiteral("");
    QVERIFY(!other.isWord());  // todo lo que no sea "1" es frase
    QVERIFY(other.isSentence());
}

void TstFlatItemCodec::roundTrip()
{
    // ítems con todo tipo de densidad de campos (flat: sin imgr, como el
    // original al leer `data`)
    QList<Item> items = {sampleWord(), sampleSentence(), Item{},
                         [&] { Item i; i.trgt = QStringLiteral("t");
                               i.srce = QStringLiteral("s");
                               i.type = QStringLiteral(""); return i; }()};
    for (const Item &orig : items) {
        Item it = orig;
        it.imgr.clear();
        const Item back = FlatItemCodec::decode(FlatItemCodec::encode(it));
        QCOMPARE(back, it);
    }
}

void TstFlatItemCodec::legacyExtraFields()
{
    // Línea con campos extra antes de `type` (caso de "older files carry extra
    // fields" — docs/03). get_item (por nombre) lo resuelve; aquí la búsqueda
    // por nombre es la autoridad (ver Informe, DECISIONES DE DISEÑO).
    const QString line =
        QStringLiteral("trgt{legacy}srce{viejo}exmp{e}defn{d}note{n}wrds{w}"
                       "grmr{g}tags{t}mark{FALSE}refr{l}imag{0}link{k}cdid{c}"
                       "legacyField1{x}legacyField2{y}type{1}");
    const Item it = FlatItemCodec::decode(line);
    QCOMPARE(it.trgt, QStringLiteral("legacy"));
    QCOMPARE(it.type, QStringLiteral("1"));
    QVERIFY(it.isWord());
    QCOMPARE(it.mark, QStringLiteral("FALSE"));
}

void TstFlatItemCodec::embeddedCloseBraceIsLossyLikeOriginal()
{
    // Un '}' dentro de un valor: el original (`sed s/}/}\n/g` + grep por línea)
    // trunca el valor en el primer '}'. Comportamiento preservado, no mejorado.
    Item it;
    it.trgt = QStringLiteral("x");
    it.note = QStringLiteral("a}b");
    const QString line = FlatItemCodec::encode(it);
    const Item back = FlatItemCodec::decode(line);
    QCOMPARE(back.note, QStringLiteral("a"));  // pérdida idéntica al original
}

void TstFlatItemCodec::itemJsonFieldOrder()
{
    // Orden de los 24 campos JSON (real-files / default/vars línea 1 + slvi)
    const QList<QString> order = Item::jsonFieldOrder();
    QCOMPARE(order.size(), 24);
    QCOMPARE(order,
             QStringList({"srce", "slch", "slde", "slen", "sles", "slfr",
                          "slit", "slja", "slpt", "slru", "slvi", "exmp",
                          "defn", "note", "wrds", "grmr", "tags", "mark",
                          "refr", "imag", "imgr", "link", "cdid", "type"}));

    // ToJsonText emite el objeto en el orden exacto (QJsonDocument de Qt
    // reordena las claves; el .idmnd no puede permitírselo).
    const QString text = sampleSentence().toJsonText();
    QVERIFY(text.startsWith(QStringLiteral("{\"srce\":")));
    QVERIFY(text.indexOf(QStringLiteral("\"slch\"")) <
            text.indexOf(QStringLiteral("\"slvi\"")));
    QVERIFY(text.indexOf(QStringLiteral("\"slvi\"")) <
            text.indexOf(QStringLiteral("\"exmp\"")));
    QVERIFY(text.indexOf(QStringLiteral("\"imag\"")) <
            text.indexOf(QStringLiteral("\"imgr\"")));
    QVERIFY(text.indexOf(QStringLiteral("\"imgr\"")) <
            text.indexOf(QStringLiteral("\"link\"")));
    QVERIFY(text.indexOf(QStringLiteral("\"cdid\"")) <
            text.indexOf(QStringLiteral("\"type\"")));
    QVERIFY(text.endsWith(QStringLiteral("\"type\":\"2\"}")));

    // el texto es JSON válido y con escaping estándar
    const QJsonDocument doc =
        QJsonDocument::fromJson(text.toUtf8());
    QVERIFY2(doc.isObject(), qPrintable(text));
    QCOMPARE(doc.object().size(), ItemDefs::jsonFieldCount());
}

void TstFlatItemCodec::itemJsonRoundTrip()
{
    const Item it = sampleWord();
    const QJsonDocument doc =
        QJsonDocument::fromJson(it.toJsonText().toUtf8());
    QVERIFY(doc.isObject());
    const Item restored = Item::fromJson(QJsonValue(doc.object()), it.trgt);
    QCOMPARE(restored, it);
    QCOMPARE(restored.imgr, QStringLiteral("book"));
    QCOMPARE(restored.cdid, it.cdid);
    QVERIFY(restored.isMarked());
}

void TstFlatItemCodec::multipleTranslationsJson()
{
    Item it;
    it.trgt = QStringLiteral("book");
    it.srce = QStringLiteral("libro");
    it.type = QStringLiteral("1");
    it.translations.insert(QStringLiteral("it"), QStringLiteral("libro (italiano)"));
    it.translations.insert(QStringLiteral("de"), QStringLiteral("Buch"));
    it.translations.insert(QStringLiteral("en"), QStringLiteral("book"));

    const QJsonObject obj = it.toJson().toObject();
    // todas las claves slXX presentes (el template original las emite siempre)
    QCOMPARE(obj.size(), ItemDefs::jsonFieldCount());
    QCOMPARE(obj.value(QStringLiteral("slit")).toString(), QStringLiteral("libro (italiano)"));
    QCOMPARE(obj.value(QStringLiteral("slde")).toString(), QStringLiteral("Buch"));
    QCOMPARE(obj.value(QStringLiteral("slen")).toString(), QStringLiteral("book"));
    QCOMPARE(obj.value(QStringLiteral("slch")).toString(), QString());

    const Item back = Item::fromJson(QJsonValue(obj), it.trgt);
    QCOMPARE(back, it);

    // y por toJsonText el JSON es consistente
    const QJsonDocument doc = QJsonDocument::fromJson(it.toJsonText().toUtf8());
    QVERIFY(doc.isObject());
    QCOMPARE(Item::fromJson(QJsonValue(doc.object()), it.trgt), it);
}

void TstFlatItemCodec::realFixturesDecodeToFlatConsistent()
{
    struct Fixture {
        const char *name;
        int words;
        int sentences;
    };
    const Fixture fixtures[] = {
        {"Most common prepositions in English.idmnd", 17, 0},
        {"I Want to Swim.idmnd", 0, 25},
        {"Nazca Lines.idmnd", 13, 22},
        {"Algunas rarezas del idioma ingles.idmnd", 0, 20},
    };

    for (const Fixture &fx : fixtures) {
        int itemCount = 0;
        const QJsonDocument doc = loadIdmnd(fx.name, &itemCount);
        QVERIFY2(!doc.isNull(), fx.name);
        QCOMPARE(itemCount, fx.words + fx.sentences);

        const QJsonObject items = doc.object().value(QStringLiteral("items")).toObject();
        int words = 0, sentences = 0;
        for (auto it = items.constBegin(); it != items.constEnd(); ++it) {
            const Item item = Item::fromJson(it.value(), it.key());
            if (item.isWord())
                ++words;
            else
                ++sentences;

            // .idmnd real -> Item -> JSON ordenado (toJsonText) -> Item
            const QJsonDocument jd =
                QJsonDocument::fromJson(item.toJsonText().toUtf8());
            QVERIFY2(jd.isObject(), qPrintable(item.trgt));
            QCOMPARE(Item::fromJson(QJsonValue(jd.object()), item.trgt), item);
            QCOMPARE(jd.object().size(), ItemDefs::jsonFieldCount());

            // .idmnd real -> JSON -> Item -> línea plana -> Item (consistencia).
            // imgr vive sólo en JSON; la línea plana (get_item) no lo lleva.
            Item flatItem = item;
            flatItem.imgr.clear();
            const Item flat = FlatItemCodec::decode(FlatItemCodec::encode(flatItem));
            QCOMPARE(flat, flatItem);
            QVERIFY(!item.trgt.isEmpty());

            // el ítem real no debe llevarse trgt dentro del objeto (JSON OK)
            QVERIFY(!it.value().toObject().contains(QStringLiteral("trgt")));
        }
        QCOMPARE(words, fx.words);
        QCOMPARE(sentences, fx.sentences);
        QVERIFY(!doc.object().value(QStringLiteral("name")).toString().isEmpty());
    }
}

QTEST_GUILESS_MAIN(TstFlatItemCodec)
#include "tst_flatitemcodec.moc"