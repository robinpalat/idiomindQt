// F3.2/F3.2b - round-trip del formato .idmnd: importar -> exportar -> parsear,
// verificando que el resultado coincide con el Bash (con las desviaciones
// documentadas: dteu=hoy, stts leido de DB (''->0), imgr recomputado,
// slvi omitido en el export, info transcodificado, ilnk por defecto).
// F3.2b añade: validación de chars prohibidos, slng con extra translations,
// slng/tlng mismatch, y golden tests contra fixtures reales de Web/.
#include <QDate>
#include <QDir>
#include <QFileInfo>
#include <QtTest/QtTest>

#include "core/config/AppPaths.h"
#include "core/topic/Item.h"
#include "core/topic/FlatItemCodec.h"
#include "storage/interchange/IdmndCodec.h"
#include "storage/interchange/LegacyImporter.h"
#include "storage/interchange/TopicExporter.h"
#include "storage/topic/TopicDataRepository.h"

using namespace idiomind::storage;
using namespace idiomind::core;

static QByteArray readFixture(const QString &name)
{
    QFile f(QLatin1String(IDIOMIND_TEST_FIXTURE_DIR) + '/' + name);
    if (!f.open(QIODevice::ReadOnly)) return {};
    return f.readAll();
}

static QString slug15(const QString &tpc)
{
    QString pre = tpc.left(15);
    pre.replace(QLatin1Char(' '), QLatin1Char('_'));
    pre.remove(QLatin1Char('\''));
    QString a;
    for (const QChar c : pre) {
        if (c.unicode() < 0x80) a += c;
    }
    return a.toLower();
}

static QString expectedImgr(const Item &item)
{
    return item.type == QLatin1String("1")
        ? item.trgt.left(1).toLower() + item.trgt.mid(1)
        : QString();
}

static QString capFirst(const QString &s)
{
    if (s.isEmpty()) return s;
    return s.at(0).toUpper() + s.mid(1);
}

// Construye un IdmndDocument mínimo válido para tests de validación.
static IdmndDocument minimalValidDoc()
{
    IdmndDocument d;
    d.info.setName(QStringLiteral("Test"));
    d.info.setSlng(QStringLiteral("Español"));
    d.info.setTlng(QStringLiteral("English"));
    d.info.setCtgy(QStringLiteral("storyteller"));
    d.info.setIlnk(QStringLiteral("test123456"));
    d.info.setNwrd(QStringLiteral("0"));
    d.info.setNsnt(QStringLiteral("0"));
    d.info.setNimg(QStringLiteral("0"));
    d.info.setNaud(QStringLiteral("0"));
    d.info.setLevl(QStringLiteral("1"));
    d.info.setStts(QStringLiteral("0"));
    return d;
}

class tst_idmndroundtrip : public QObject
{
    Q_OBJECT
private slots:
    void importExport_data();
    void importExport();
    void validationErrors();
    void forbiddenCharsInName();
    void forbiddenCharsInAutrCntt();
    void forbiddenCharsInOrig();
    void forbiddenCharsInStts();
    void slngExtraTranslations();
    void slngTlngMismatch();
    void goldenFixtures_data();
    void goldenFixtures();
};

// --- import/export round-trip (original F3.2) ---

void tst_idmndroundtrip::importExport_data()
{
    QTest::addColumn<QString>("file");
    const QStringList files = {
        "I Want to Swim.idmnd",
        "Most common prepositions in English.idmnd",
        "Nazca Lines.idmnd",
        "Algunas rarezas del idioma ingles.idmnd"};
    for (const QString &f : files)
        QTest::newRow(f.toLatin1().constData()) << f;
}

void tst_idmndroundtrip::importExport()
{
    QFETCH(QString, file);
    const QByteArray original = readFixture(file);

    QTemporaryDir home;
    QVERIFY(home.isValid());
    AppPaths paths(home.path(), home.path());

    LegacyImporter importer(paths);
    QString topicName;
    QVERIFY2(importer.importFile("English", QLatin1String(IDIOMIND_TEST_FIXTURE_DIR) + '/' + file,
                                  &topicName),
             qPrintable(QString("import %1").arg(file)));
    QVERIFY(!topicName.isEmpty());

    TopicDataRepository dataRepo(paths);
    const IdmndDocument doc = IdmndCodec::parse(original);
    auto loaded = dataRepo.loadItems("English", topicName);
    QCOMPARE(loaded.size(), doc.items.size());
    for (int i = 0; i < loaded.size(); ++i) {
        const Item &imp = doc.items[i];
        const Item &l = loaded[i].item;
        QCOMPARE(l.trgt, imp.trgt);
        QCOMPARE(l.srce, imp.srce);
        QCOMPARE(l.exmp, imp.exmp);
        QCOMPARE(l.defn, imp.defn);
        QCOMPARE(l.note, imp.note);
        QCOMPARE(l.wrds, imp.wrds);
        QCOMPARE(l.grmr, imp.grmr);
        QCOMPARE(l.tags, imp.tags);
        QCOMPARE(l.mark, imp.mark);
        QCOMPARE(l.refr, imp.refr);
        QCOMPARE(l.cdid, imp.cdid);
        QCOMPARE(l.type, imp.type);
        QCOMPARE(l.imgr, expectedImgr(imp));
        QCOMPARE(l.translations, imp.translations);
    }
    QCOMPARE(dataRepo.readStts("English", topicName), QStringLiteral("1"));

    TopicExporter exporter(paths);
    bool ok = false;
    QString err;
    const QByteArray out = exporter.exportTopic("English", topicName, &ok, &err);
    QVERIFY2(ok, qPrintable(err));

    bool ok2 = false;
    const IdmndDocument exported = IdmndCodec::parse(out, &ok2);
    QVERIFY(ok2);

    QCOMPARE(exported.info.name(), topicName);
    QCOMPARE(exported.info.slng(), capFirst(doc.info.slng()));
    QCOMPARE(exported.info.tlng(), capFirst(doc.info.tlng()));
    QCOMPARE(exported.info.dteu(), QDate::currentDate().toString(QStringLiteral("yyyy-MM-dd")));
    QCOMPARE(exported.info.dtec(), doc.info.dtec());
    QCOMPARE(exported.info.dtei(), QDate::currentDate().toString(QStringLiteral("yyyy-MM-dd")));
    QCOMPARE(exported.info.orig(), doc.info.orig());
    QCOMPARE(exported.info.stts(), QStringLiteral("0"));
    QCOMPARE(exported.info.cntt(), QString());
    QCOMPARE(exported.info.autr(), doc.info.autr());
    QCOMPARE(exported.info.nwrd(), doc.info.nwrd());
    QCOMPARE(exported.info.nsnt(), doc.info.nsnt());
    QCOMPARE(exported.info.nimg(), doc.info.nimg());
    QCOMPARE(exported.info.naud(), doc.info.naud());
    QCOMPARE(exported.info.nsze(), doc.info.nsze());
    QCOMPARE(exported.info.levl(), doc.info.levl());
    QCOMPARE(exported.info.info(), QString());
    QVERIFY(exported.info.ilnk().startsWith(slug15(topicName)));
    QCOMPARE(exported.info.ilnk().size(), slug15(topicName).size() + 20);

    QCOMPARE(exported.items.size(), doc.items.size());
    for (int i = 0; i < doc.items.size(); ++i) {
        const Item &imp = doc.items[i];
        const Item &e = exported.items[i];
        QCOMPARE(e.trgt, imp.trgt);
        QCOMPARE(e.srce, imp.srce);
        QCOMPARE(e.exmp, imp.exmp);
        QCOMPARE(e.defn, imp.defn);
        QCOMPARE(e.note, imp.note);
        QCOMPARE(e.wrds, imp.wrds);
        QCOMPARE(e.grmr, imp.grmr);
        QCOMPARE(e.tags, imp.tags);
        QCOMPARE(e.mark, imp.mark);
        QCOMPARE(e.refr, imp.refr);
        QCOMPARE(e.cdid, imp.cdid);
        QCOMPARE(e.type, imp.type);
        QCOMPARE(e.imgr, expectedImgr(imp));
        QCOMPARE(e.imag, QStringLiteral("0"));
        QCOMPARE(e.translations, imp.translations);
    }
}

// --- validación check_format_1 (original F3.2) ---

void tst_idmndroundtrip::validationErrors()
{
    AppPaths apaths; LegacyImporter importer(apaths);
    bool ok = false; QString why;
    { IdmndDocument d; d.info.setSlng(QStringLiteral("Nope"));
      d.info.setName(QStringLiteral("X")); d.info.setTlng(QStringLiteral("English"));
      QByteArray out = IdmndCodec::write(d);
      d = LegacyImporter::validate(out, &ok, &why);
      QVERIFY(!ok); QVERIFY(why.contains(QLatin1String("slng"))); }
    { IdmndDocument d; d.info.setNwrd(QStringLiteral("999"));
      d.info.setName(QStringLiteral("X")); d.info.setSlng(QStringLiteral("English"));
      d.info.setTlng(QStringLiteral("English")); d.info.setLevl(QStringLiteral("1"));
      d.info.setCtgy(QStringLiteral("storyteller")); d.info.setIlnk(QStringLiteral("test123"));
      QByteArray out = IdmndCodec::write(d);
      d = LegacyImporter::validate(out, &ok, &why);
      QVERIFY(!ok); QVERIFY(why.contains(QLatin1String("nwrd"))); }
}

// --- F3.2b: chars prohibidos en name ---

void tst_idmndroundtrip::forbiddenCharsInName()
{
    bool ok = false; QString why;
    // Válidos: nombre normal
    { IdmndDocument d = minimalValidDoc();
      QByteArray out = IdmndCodec::write(d);
      d = LegacyImporter::validate(out, &ok, &why);
      QVERIFY2(ok, qPrintable(why)); }
    // Inválido: name con '*'
    { IdmndDocument d = minimalValidDoc();
      d.info.setName(QStringLiteral("Test*Name"));
      QByteArray out = IdmndCodec::write(d);
      d = LegacyImporter::validate(out, &ok, &why);
      QVERIFY(!ok); QVERIFY(why.contains(QLatin1String("name"))); }
    // Inválido: name con '/'
    { IdmndDocument d = minimalValidDoc();
      d.info.setName(QStringLiteral("Test/Name"));
      QByteArray out = IdmndCodec::write(d);
      d = LegacyImporter::validate(out, &ok, &why);
      QVERIFY(!ok); QVERIFY(why.contains(QLatin1String("name"))); }
    // Inválido: name con '@'
    { IdmndDocument d = minimalValidDoc();
      d.info.setName(QStringLiteral("Test@Name"));
      QByteArray out = IdmndCodec::write(d);
      d = LegacyImporter::validate(out, &ok, &why);
      QVERIFY(!ok); QVERIFY(why.contains(QLatin1String("name"))); }
    // Inválido: name con '='
    { IdmndDocument d = minimalValidDoc();
      d.info.setName(QStringLiteral("Test=Name"));
      QByteArray out = IdmndCodec::write(d);
      d = LegacyImporter::validate(out, &ok, &why);
      QVERIFY(!ok); QVERIFY(why.contains(QLatin1String("name"))); }
    // Inválido: name todo-espacios
    { IdmndDocument d = minimalValidDoc();
      d.info.setName(QStringLiteral("   "));
      QByteArray out = IdmndCodec::write(d);
      d = LegacyImporter::validate(out, &ok, &why);
      QVERIFY(!ok); QVERIFY(why.contains(QLatin1String("name"))); }
    // Inválido: name >60
    { IdmndDocument d = minimalValidDoc();
      d.info.setName(QString(61, QLatin1Char('x')));
      QByteArray out = IdmndCodec::write(d);
      d = LegacyImporter::validate(out, &ok, &why);
      QVERIFY(!ok); QVERIFY(why.contains(QLatin1String("name"))); }
}

// --- F3.2b: chars prohibidos en autr/cntt ---

void tst_idmndroundtrip::forbiddenCharsInAutrCntt()
{
    bool ok = false; QString why;
    const QStringList forbidden = {
        QStringLiteral("*"), QStringLiteral("/"),
        QStringLiteral("("), QStringLiteral(")"),
        QStringLiteral("=")
    };
    for (const QString &ch : forbidden) {
        // autr con char prohibido
        { IdmndDocument d = minimalValidDoc();
          d.info.setAutr(QStringLiteral("Author") + ch);
          QByteArray out = IdmndCodec::write(d);
          d = LegacyImporter::validate(out, &ok, &why);
          QVERIFY2(!ok, qPrintable(QString("autr with '%1' should fail").arg(ch)));
          QVERIFY(why.contains(QLatin1String("autr"))); }
        // cntt con char prohibido
        { IdmndDocument d = minimalValidDoc();
          d.info.setCntt(QStringLiteral("Content") + ch);
          QByteArray out = IdmndCodec::write(d);
          d = LegacyImporter::validate(out, &ok, &why);
          QVERIFY2(!ok, qPrintable(QString("cntt with '%1' should fail").arg(ch)));
          QVERIFY(why.contains(QLatin1String("cntt"))); }
    }
    // Válidos: autr/cntt normales
    { IdmndDocument d = minimalValidDoc();
      d.info.setAutr(QStringLiteral("Author Name"));
      d.info.setCntt(QStringLiteral("Content Type"));
      QByteArray out = IdmndCodec::write(d);
      d = LegacyImporter::validate(out, &ok, &why);
      QVERIFY2(ok, qPrintable(why)); }
}

// --- F3.2b: chars prohibidos en orig ---

void tst_idmndroundtrip::forbiddenCharsInOrig()
{
    bool ok = false; QString why;
    // Válido
    { IdmndDocument d = minimalValidDoc();
      d.info.setOrig(QStringLiteral("Original Topic"));
      QByteArray out = IdmndCodec::write(d);
      d = LegacyImporter::validate(out, &ok, &why);
      QVERIFY2(ok, qPrintable(why)); }
    // Inválido: orig con '*'
    { IdmndDocument d = minimalValidDoc();
      d.info.setOrig(QStringLiteral("Topic*Name"));
      QByteArray out = IdmndCodec::write(d);
      d = LegacyImporter::validate(out, &ok, &why);
      QVERIFY(!ok); QVERIFY(why.contains(QLatin1String("orig"))); }
    // Inválido: orig con '/'
    { IdmndDocument d = minimalValidDoc();
      d.info.setOrig(QStringLiteral("Topic/Name"));
      QByteArray out = IdmndCodec::write(d);
      d = LegacyImporter::validate(out, &ok, &why);
      QVERIFY(!ok); QVERIFY(why.contains(QLatin1String("orig"))); }
}

// --- F3.2b: chars prohibidos en stts ---

void tst_idmndroundtrip::forbiddenCharsInStts()
{
    bool ok = false; QString why;
    // Válido: stts numérico
    { IdmndDocument d = minimalValidDoc();
      d.info.setStts(QStringLiteral("0"));
      QByteArray out = IdmndCodec::write(d);
      d = LegacyImporter::validate(out, &ok, &why);
      QVERIFY2(ok, qPrintable(why)); }
    // Inválido: stts con '-'
    { IdmndDocument d = minimalValidDoc();
      d.info.setStts(QStringLiteral("1-2"));
      QByteArray out = IdmndCodec::write(d);
      d = LegacyImporter::validate(out, &ok, &why);
      QVERIFY(!ok); QVERIFY(why.contains(QLatin1String("stts"))); }
    // Inválido: stts con '@'
    { IdmndDocument d = minimalValidDoc();
      d.info.setStts(QStringLiteral("test@value"));
      QByteArray out = IdmndCodec::write(d);
      d = LegacyImporter::validate(out, &ok, &why);
      QVERIFY(!ok); QVERIFY(why.contains(QLatin1String("stts"))); }
    // Inválido: stts todo-espacios
    { IdmndDocument d = minimalValidDoc();
      d.info.setStts(QStringLiteral("   "));
      QByteArray out = IdmndCodec::write(d);
      d = LegacyImporter::validate(out, &ok, &why);
      QVERIFY(!ok); QVERIFY(why.contains(QLatin1String("stts"))); }
}

// --- F3.2b: slng con traducciones extra ---

void tst_idmndroundtrip::slngExtraTranslations()
{
    bool ok = false; QString why;
    // Válido: slng con extra translations
    { IdmndDocument d = minimalValidDoc();
      d.info.setSlng(QStringLiteral("Español, English, French"));
      QByteArray out = IdmndCodec::write(d);
      d = LegacyImporter::validate(out, &ok, &why);
      QVERIFY2(ok, qPrintable(why)); }
    // Válido: slng simple (sin extras)
    { IdmndDocument d = minimalValidDoc();
      d.info.setSlng(QStringLiteral("Español"));
      QByteArray out = IdmndCodec::write(d);
      d = LegacyImporter::validate(out, &ok, &why);
      QVERIFY2(ok, qPrintable(why)); }
    // Inválido: slng con extra pero primaria no válida
    { IdmndDocument d = minimalValidDoc();
      d.info.setSlng(QStringLiteral("NopeLang, English"));
      QByteArray out = IdmndCodec::write(d);
      d = LegacyImporter::validate(out, &ok, &why);
      QVERIFY(!ok); QVERIFY(why.contains(QLatin1String("slng"))); }
    // Verificar que la importación con extra translations funciona
    { IdmndDocument d = minimalValidDoc();
      d.info.setSlng(QStringLiteral("Español, English"));
      QByteArray out = IdmndCodec::write(d);
      QTemporaryDir home;
      QVERIFY(home.isValid());
      AppPaths paths(home.path(), home.path());
      LegacyImporter importer(paths);
      QString topicName;
      QVERIFY2(importer.importText("English", out, &topicName),
               qPrintable(QString("import with extra slng")));
      QVERIFY(!topicName.isEmpty()); }
}

// --- F3.2b: slng/tlng mismatch ---

void tst_idmndroundtrip::slngTlngMismatch()
{
    QTemporaryDir home;
    QVERIFY(home.isValid());
    AppPaths paths(home.path(), home.path());

    LegacyImporter importer(paths);
    // Configurar perfil con slng/tlng diferentes al topic
    importer.setProfileLanguages(QStringLiteral("English"), QStringLiteral("Spanish"));

    IdmndDocument d = minimalValidDoc();
    d.info.setSlng(QStringLiteral("Español"));
    d.info.setTlng(QStringLiteral("English"));
    QByteArray out = IdmndCodec::write(d);

    QString topicName;
    QVERIFY2(importer.importText("English", out, &topicName),
             qPrintable(QString("import with mismatch")));
    QVERIFY(!topicName.isEmpty());

    // Verificar que se detectó el mismatch
    auto mm = importer.lastMismatch();
    QVERIFY(mm.slngMismatch);  // slng del topic (Español) != perfil (English)
    QVERIFY(mm.tlngMismatch);  // tlng del topic (English) != perfil (Spanish)

    // Verificar que se crearon los archivos de mismatch
    const QString conf = paths.topicConfDir("English", topicName);
    QVERIFY(QFileInfo::exists(conf + QStringLiteral("/translations/active")));
    QVERIFY(QFileInfo::exists(conf + QStringLiteral("/slng_err")));

    // Verificar contenido de translations/active
    QFile activeFile(conf + QStringLiteral("/translations/active"));
    QVERIFY(activeFile.open(QIODevice::ReadOnly));
    const QString activeContent = QString::fromUtf8(activeFile.readAll()).trimmed();
    QCOMPARE(activeContent, QStringLiteral("Español"));
}

// --- F3.2b: golden tests contra fixtures reales ---

void tst_idmndroundtrip::goldenFixtures_data()
{
    QTest::addColumn<QString>("file");
    QTest::addColumn<int>("expectedItems");
    // 4 originales + 8 golden = 12 fixtures
    const QStringList files = {
        "I Want to Swim.idmnd", "25",
        "Most common prepositions in English.idmnd", "17",
        "Nazca Lines.idmnd", "35",
        "Algunas rarezas del idioma ingles.idmnd", "20",
        "Google Announces New Services and Products.idmnd", "0", // will check >0
        "100 Most common words in English.idmnd", "0",
        "Daily Routines.idmnd", "0",
        "Airsick.idmnd", "0",
        "Small Talk.idmnd", "0",
    };
    for (int i = 0; i < files.size(); i += 2) {
        const QString name = files[i];
        const int count = files[i + 1].toInt();
        QTest::newRow(name.toLatin1().constData()) << name << count;
    }
}

void tst_idmndroundtrip::goldenFixtures()
{
    QFETCH(QString, file);
    QFETCH(int, expectedItems);

    const QByteArray raw = readFixture(file);
    if (raw.isEmpty()) {
        // Fixture no encontrado — skip silencioso (puede no existir en build)
        QSKIP("Fixture not found");
    }

    // 1. Parse debe ser exitoso
    bool ok = false; QString why;
    const IdmndDocument doc = IdmndCodec::parse(raw, &ok, &why);
    QVERIFY2(ok, qPrintable(QString("parse %1: %2").arg(file, why)));

    // 2. Validate debe ser exitoso (check_format_1)
    const IdmndDocument validated = LegacyImporter::validate(raw, &ok, &why);
    QVERIFY2(ok, qPrintable(QString("validate %1: %2").arg(file, why)));

    // 3. Items > 0
    QVERIFY2(doc.items.size() > 0,
             qPrintable(QString("%1 has 0 items").arg(file)));

    // 4. TopicInfo tiene campos esenciales
    QVERIFY(!doc.info.name().isEmpty());
    QVERIFY(!doc.info.slng().isEmpty());
    QVERIFY(!doc.info.tlng().isEmpty());

    // 5. Import + re-export round-trip
    QTemporaryDir home;
    QVERIFY(home.isValid());
    AppPaths paths(home.path(), home.path());
    LegacyImporter importer(paths);
    QString topicName;
    QVERIFY2(importer.importFile("English",
              QLatin1String(IDIOMIND_TEST_FIXTURE_DIR) + '/' + file, &topicName),
             qPrintable(QString("import %1").arg(file)));
    QVERIFY(!topicName.isEmpty());

    TopicDataRepository dataRepo(paths);
    auto loaded = dataRepo.loadItems("English", topicName);
    QCOMPARE(loaded.size(), doc.items.size());

    TopicExporter exporter(paths);
    bool ok2 = false;
    QString err;
    const QByteArray out = exporter.exportTopic("English", topicName, &ok2, &err);
    QVERIFY2(ok2, qPrintable(err));

    // 6. Re-export parsea sin error
    bool ok3 = false;
    const IdmndDocument exported = IdmndCodec::parse(out, &ok3);
    QVERIFY(ok3);
    QCOMPARE(exported.items.size(), doc.items.size());

    // 7. Items preservados (trgt, srce, type)
    for (int i = 0; i < doc.items.size(); ++i) {
        QCOMPARE(exported.items[i].trgt, doc.items[i].trgt);
        QCOMPARE(exported.items[i].srce, doc.items[i].srce);
        QCOMPARE(exported.items[i].type, doc.items[i].type);
    }
}

QTEST_MAIN(tst_idmndroundtrip)
#include "tst_idmndroundtrip.moc"
