#include <QtTest>

#include <QDir>
#include <QTemporaryDir>

#include "core/config/AppPaths.h"

using idiomind::core::AppPaths;

class TstAppPaths : public QObject
{
    Q_OBJECT

private slots:
    void legacyRoots();
    void derivedDirectories();
    void tmpDirKeepsOriginalNaming();
    void noFilesystemSideEffects();
};

// ---- $HOME/.idiomind y $HOME/.config/idiomind (doc 02) ---------------------
void TstAppPaths::legacyRoots()
{
    const QString home = QStringLiteral("/tmp/idiomind-test-home");
    const AppPaths p(home);

    QCOMPARE(p.dataRoot(), QStringLiteral("/tmp/idiomind-test-home/.idiomind"));
    QCOMPARE(p.configRoot(),
             QStringLiteral("/tmp/idiomind-test-home/.config/idiomind"));
    QCOMPARE(p.homeDir(), home);
}

// ---- derivación de directorios (doc 02) ------------------------------------
void TstAppPaths::derivedDirectories()
{
    const QString home = QStringLiteral("/tmp/idiomind-test-home");
    const AppPaths p(home);

    const QString topics = QStringLiteral("/tmp/idiomind-test-home/.idiomind/topics");
    QCOMPARE(p.topicsDir(), topics);
    QCOMPARE(p.backupDir(),
             QStringLiteral("/tmp/idiomind-test-home/.idiomind/backup"));

    // por idioma de aprendizaje
    QCOMPARE(p.topicsDirForLanguage(QStringLiteral("English")),
             topics + QStringLiteral("/English"));
    QCOMPARE(p.sharedDirForLanguage(QStringLiteral("English")),
             topics + QStringLiteral("/English/.share"));

    // por topic (los nombres pueden contener espacios, igual que el original)
    const QString topicName = QStringLiteral("April collet 2");
    QCOMPARE(p.topicDataDir(QStringLiteral("English"), topicName),
             topics + QStringLiteral("/English/April collet 2"));
    // F2.2: DC_tlt vive DENTRO del topic (c.conf línea 25), bajo DM_tl, y es
    // específico del idioma.
    QCOMPARE(p.topicConfDir(QStringLiteral("English"), topicName),
             topics + QStringLiteral("/English/April collet 2/.conf"));

    // bases de datos (doc 04)
    QCOMPARE(p.configDb(),
             QStringLiteral("/tmp/idiomind-test-home/.config/idiomind/config"));
    QCOMPARE(p.sharedDataDir(QStringLiteral("English")),
             topics + QStringLiteral("/English/.share/data"));
    QCOMPARE(p.sharedDb(QStringLiteral("English")),
             topics + QStringLiteral("/English/.share/data/config"));
    QCOMPARE(p.tlngDb(QStringLiteral("English")),
             topics + QStringLiteral("/English/.share/data/English.db"));
    QCOMPARE(p.topicDb(QStringLiteral("English"), topicName),
             topics + QStringLiteral("/English/April collet 2/.conf/tpc"));
}

// ---- DT = <tmpRoot>/.idiomind-<user> (doc 02) ------------------------------
void TstAppPaths::tmpDirKeepsOriginalNaming()
{
    const QString home = QStringLiteral("/tmp/idiomind-test-home");
    const QString tmpRoot = QDir::tempPath();
    const AppPaths p(home, tmpRoot);

    const QString expectedUser = qEnvironmentVariable("USER", QStringLiteral("user"));
    QCOMPARE(p.tmpDir(),
             QDir::cleanPath(tmpRoot + QStringLiteral("/.idiomind-") + expectedUser));
}

// ---- AppPaths no crea NADA sobre el filesystem real ------------------------
void TstAppPaths::noFilesystemSideEffects()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString home = dir.path();
    const AppPaths p(home, home);

    // Calcular todas las rutas no debe crear directorios.
    QVERIFY(!QDir(p.dataRoot()).exists());
    QVERIFY(!QDir(p.configRoot()).exists());
    QVERIFY(!QDir(p.topicsDir()).exists());
    QVERIFY(!QDir(p.backupDir()).exists());
    QVERIFY(!QDir(p.topicsDirForLanguage(QStringLiteral("English"))).exists());
    QVERIFY(!QDir(p.sharedDirForLanguage(QStringLiteral("English"))).exists());
    QVERIFY(!QDir(p.topicDataDir(QStringLiteral("English"), QStringLiteral("T"))).exists());
    QVERIFY(!QDir(p.topicConfDir(QStringLiteral("English"), QStringLiteral("T"))).exists());
    QVERIFY(!QDir(p.tmpDir()).exists());
    QVERIFY(!QFileInfo::exists(p.configDb()));
    QVERIFY(!QFileInfo::exists(p.topicDb(QStringLiteral("English"), QStringLiteral("T"))));
}

QTEST_GUILESS_MAIN(TstAppPaths)
#include "tst_apppaths.moc"