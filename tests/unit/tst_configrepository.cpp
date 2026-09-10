// F2.2 - tst_configrepository: persistencia básica del perfil (cfgdb + $DC_s/tpc).
// Autoridad: docs/04-databases.md, mkdb.sh create_cfgdb, c.conf, cnfg.sh.
//
// Todo sobre QTemporaryDir: se inyecta un homeDir alternativo en AppPaths,
// por lo que ningún test toca ~/.idiomind ni ~/.config/idiomind del usuario.
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTest>
#include <QTemporaryDir>

#include "core/config/AppPaths.h"
#include "storage/config/ConfigRepository.h"

using idiomind::core::AppPaths;
using idiomind::storage::ConfigRepository;

class TestConfigRepository : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void ensureProfileCreatesSqliteProfile();
    void ensureProfileIsIdempotent();
    void ensureProfileRecreatesNonSqliteFile();
    void languagesRoundTrip();
    void languageValuesAreEscaped();
    void optionsDefaultsAndRoundTrip();
    void setOptionsInBulk();
    void activeTopicFileFirstLine();
    void activeTopicClears();

private:
    QTemporaryDir m_tmp;
    AppPaths m_paths;
};

void TestConfigRepository::initTestCase()
{
    QVERIFY(m_tmp.isValid());
    // APPATHS_ALTERNATIVA: todo el test corre sobre un $HOME de prueba.
    m_paths = AppPaths(m_tmp.path());
    QVERIFY(m_paths.configRoot().startsWith(m_tmp.path()));
    QVERIFY(m_paths.dataRoot().startsWith(m_tmp.path()));
}

void TestConfigRepository::cleanupTestCase()
{
    // Cierra cualquier conexión SQLite abierta por el QSqlDatabase wrapper.
    QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);
}

static QString sqliteMagicHdr(const QString &path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly))
        return QString();
    return QString::fromLatin1(f.read(16));
}

void TestConfigRepository::ensureProfileCreatesSqliteProfile()
{
    ConfigRepository repo(m_paths);
    QVERIFY(repo.ensureProfile());

    // cfgdb existe y es SQLite real: $DC_s/config
    const QString cfgdb = m_paths.configDb();
    QVERIFY(QFileInfo::exists(cfgdb));
    QVERIFY(sqliteMagicHdr(cfgdb).startsWith(QStringLiteral("SQLite format 3")));

// Esquema EXACTO de create_cfgdb
    {
        {
            QSqlDatabase db = QSqlDatabase::addDatabase(
                QStringLiteral("QSQLITE"), QStringLiteral("cfg_schema_check"));
            db.setDatabaseName(cfgdb);
            QVERIFY(db.open());
            const QStringList tables = db.tables();
            QVERIFY(tables.contains(QStringLiteral("opts")));
            QVERIFY(tables.contains(QStringLiteral("lang")));
            QVERIFY(tables.contains(QStringLiteral("geom")));
            QVERIFY(tables.contains(QStringLiteral("sess")));
            QVERIFY(tables.contains(QStringLiteral("updt")));
            db.close();
        }
        QSqlDatabase::removeDatabase(QStringLiteral("cfg_schema_check"));
    }
}

void TestConfigRepository::ensureProfileIsIdempotent()
{
    ConfigRepository repo(m_paths);
    QVERIFY(repo.ensureProfile());
    QVERIFY(repo.ensureProfile()); // segunda llamada no re-crea ni rompe

    // Las filas iniciales se insertaron una única vez (1 fila por tabla).
    {
        {
            QSqlDatabase db = QSqlDatabase::addDatabase(
                QStringLiteral("QSQLITE"), QStringLiteral("cfg_rows_check"));
            db.setDatabaseName(m_paths.configDb());
            QVERIFY(db.open());
            QSqlQuery q(QStringLiteral("select count(*) from opts;"), db);
            QVERIFY(q.next());
            QCOMPARE(q.value(0).toInt(), 1);
            db.close();
        } // db y q destruidos -> removeDatabase sin conexiones activas
        QSqlDatabase::removeDatabase(QStringLiteral("cfg_rows_check"));
    }
}

void TestConfigRepository::ensureProfileRecreatesNonSqliteFile()
{
    // cnfg.sh líneas 13-14: si cfgdb no es SQLite, `mkdb.sh config` lo recrea.
    const QString cfgdb = m_paths.configDb();
    {
        QFile f(cfgdb);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write("this is not a sqlite database");
        f.close();
    }
    ConfigRepository repo(m_paths);
    QVERIFY(repo.ensureProfile());
    QVERIFY(sqliteMagicHdr(cfgdb).startsWith(QStringLiteral("SQLite format 3")));
}

void TestConfigRepository::languagesRoundTrip()
{
    ConfigRepository repo(m_paths);
    QVERIFY(repo.ensureProfile());

    // create_cfgdb inserta lang ('',''); por defecto vacío.
    QCOMPARE(repo.targetLanguage(), QString());
    QCOMPARE(repo.sourceLanguage(), QString());

    QVERIFY(repo.setTargetLanguage(QStringLiteral("English")));
    QVERIFY(repo.setSourceLanguage(QStringLiteral("Español")));
    QCOMPARE(repo.targetLanguage(), QStringLiteral("English"));
    QCOMPARE(repo.sourceLanguage(), QStringLiteral("Español"));

    QVERIFY(repo.setLanguages(QStringLiteral("日本語"), QStringLiteral("English")));
    QCOMPARE(repo.targetLanguage(), QStringLiteral("日本語"));
    QCOMPARE(repo.sourceLanguage(), QStringLiteral("English"));
}

void TestConfigRepository::languageValuesAreEscaped()
{
    ConfigRepository repo(m_paths);
    QVERIFY(repo.ensureProfile());

    // Comillas simples escapadas al estilo SQLite (como el mundo real).
    QVERIFY(repo.setTargetLanguage(QStringLiteral("O'Brien")));
    QCOMPARE(repo.targetLanguage(), QStringLiteral("O'Brien"));
}

void TestConfigRepository::optionsDefaultsAndRoundTrip()
{
    ConfigRepository repo(m_paths);
    QVERIFY(repo.ensureProfile());

    // create_cfgdb: 1 fila opts con los valores por defecto del original.
    QCOMPARE(repo.option(QStringLiteral("gramr")), QStringLiteral("TRUE"));
    // columna inexistente -> cadena vacía (SELECT falla, value() devuelve "")
    QCOMPARE(repo.option(QStringLiteral("nonexistent_column")), QString());
    QCOMPARE(repo.option(QStringLiteral("slang")), QString());

    const QVariantMap all = repo.options();
    QCOMPARE(all.size(), 13); // gramr..slang
    QCOMPARE(all.value(QStringLiteral("gramr")).toString(), QStringLiteral("TRUE"));
    QCOMPARE(all.value(QStringLiteral("intrf")).toString(), QStringLiteral("default"));

    QVERIFY(repo.setOption(QStringLiteral("gramr"), QStringLiteral("FALSE")));
    QCOMPARE(repo.option(QStringLiteral("gramr")), QStringLiteral("FALSE"));

    // escaping en valores
    QVERIFY(repo.setOption(QStringLiteral("tlang"), QStringLiteral("don't")));
    QCOMPARE(repo.option(QStringLiteral("tlang")), QStringLiteral("don't"));
    // restore para no romper el resto de asserts del slot
    QVERIFY(repo.setOption(QStringLiteral("gramr"), QStringLiteral("TRUE")));
}

void TestConfigRepository::setOptionsInBulk()
{
    ConfigRepository repo(m_paths);
    QVERIFY(repo.ensureProfile());

    // `audio`/`loop` son columnas de la tabla config del topic, NO de opts
    // del perfil: se usan claves reales de opts (ttrgt/synth/trans).
    QVariantMap values;
    values.insert(QStringLiteral("gramr"), QStringLiteral("FALSE"));
    values.insert(QStringLiteral("ttrgt"), QStringLiteral("TRUE"));
    values.insert(QStringLiteral("synth"), QStringLiteral("espeak"));
    QVERIFY(repo.setOptions(values));

    QCOMPARE(repo.option(QStringLiteral("gramr")), QStringLiteral("FALSE"));
    QCOMPARE(repo.option(QStringLiteral("ttrgt")), QStringLiteral("TRUE"));
    QCOMPARE(repo.option(QStringLiteral("synth")), QStringLiteral("espeak"));
    QCOMPARE(repo.option(QStringLiteral("intrf")), QStringLiteral("default")); // intacto

    // mapa vacío = no-op (no rompe la fila)
    QVERIFY(repo.setOptions({}));
}

void TestConfigRepository::activeTopicFileFirstLine()
{
    // $DC_s/tpc: 1ª línea = topic activo (igual que `sed -n 1p` en c.conf).
    ConfigRepository repo(m_paths);
    QVERIFY(repo.setActiveTopic(QStringLiteral("My Topic")));

    const QString tpcPath = m_paths.configRoot() + QStringLiteral("/tpc");
    QVERIFY(QFileInfo::exists(tpcPath));
    QCOMPARE(repo.activeTopic(), QStringLiteral("My Topic"));
}

void TestConfigRepository::activeTopicClears()
{
    ConfigRepository repo(m_paths);
    QVERIFY(repo.setActiveTopic(QStringLiteral("A")));
    QVERIFY(repo.setActiveTopic(QStringLiteral("B")));
    QCOMPARE(repo.activeTopic(), QStringLiteral("B"));

    QVERIFY(repo.setActiveTopic(QString()));
    QCOMPARE(repo.activeTopic(), QString());
}

QTEST_GUILESS_MAIN(TestConfigRepository)
#include "tst_configrepository.moc"