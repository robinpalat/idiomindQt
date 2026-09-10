// F3.1 - tst_topicdatarepository: TopicDataRepository sobre un $HOME de prueba.
// Autoridad: docs/research/f3.1-bash-behavior.md, ifs/mods/add/add.sh index(),
// mngr.sh edit_item/delete_item_ok, ifs/tls.sh colorize.
//
// Nunca toca ~/.idiomind ni ~/.config/idiomind (AppPaths(homeDir=temp)).
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTest>
#include <QTemporaryDir>

#include "core/config/AppPaths.h"
#include "core/topic/FlatItemCodec.h"
#include "core/topic/Item.h"
#include "storage/dbs/SqliteDatabase.h"
#include "storage/topic/TopicDataRepository.h"
#include "storage/topic/TopicRepository.h"

using namespace idiomind::core;
using namespace idiomind::core::FlatItemCodec;

using idiomind::core::AppPaths;
using idiomind::storage::SqliteDatabase;
using idiomind::storage::TopicDataRepository;
using idiomind::storage::TopicRepository;

namespace {

Item makeItem(const QString &trgt, const QString &srce, const QString &type)
{
    Item it;
    it.trgt = trgt;
    it.srce = srce;
    it.exmp = QStringLiteral("e");
    it.defn = QStringLiteral("d");
    it.note = QStringLiteral("n");
    it.wrds = QStringLiteral("w");
    it.grmr = QStringLiteral("g");
    it.tags = QStringLiteral("t");
    it.mark = QStringLiteral("FALSE");
    it.refr = QStringLiteral("r");
    it.imag = QStringLiteral("0");
    it.link = QStringLiteral("l");
    it.cdid = QStringLiteral("u1");
    it.type = type;
    return it;
}

TopicInfo makeInfo(const QString &name)
{
    TopicInfo info;
    info.setName(name);
    info.setSlng(QStringLiteral("es"));
    info.setTlng(QStringLiteral("English"));
    return info;
}

QStringList dbList(const AppPaths &paths, const QString &table)
{
    QStringList out;
    SqliteDatabase db(paths.topicDb(QStringLiteral("English"), QStringLiteral("T")));
    if (!db.open())
        return out;
    QSqlQuery q = db.query(QStringLiteral("select list from '%1';").arg(table));
    while (q.isActive() && q.next())
        if (!q.value(0).toString().isEmpty())
            out.append(q.value(0).toString());
    return out;
}

QString readAll(const QString &path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return QString();
    const QString out = QString::fromUtf8(f.readAll());
    f.close();
    return out;
}

} // namespace

class TestTopicDataRepository : public QObject
{
    Q_OBJECT

private slots:
    void loadItemsMixedFormatsRoundTripByteId();
    void decode25FieldPreservesSlExtraSegments();
    void appendItemWord();
    void appendItemSentence();
    void appendItemValidations();
    void updateItemFieldEditsPreserveUnknownSegments();
    void updateItemRenameMovesTablesAndPracticeLogs();
    void updateItemRenameGrmrQuirk();
    void updateItemTypeSwapMovesTable();
    void updateItemMarkFalseIsNoOp();
    void removeItemRemovesEveryThing();
    void validateIndexAndRebuild();
    void sttsAndNoteFileOps();
};

void TestTopicDataRepository::loadItemsMixedFormatsRoundTripByteId()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const AppPaths paths(tmp.path());
    TopicRepository(paths).createTopic(QStringLiteral("English"),
                                       makeInfo(QStringLiteral("T")));
    TopicDataRepository repo(paths);

    const QString conf = paths.topicConfDir(QStringLiteral("English"), QStringLiteral("T"));
    const QString dataPath = conf + QStringLiteral("/data");
    // formato 14 (writer) con '}' y espacios y unicode; formato 25 (import).
    const QString l14 =
        QStringLiteral("trgt{Pummeled}srce{orange}exmp{He pummeled.}defn{}"
                       "note{hi'lo}wrds{}grmr{}tags{}mark{TRUE}refr{}imag{0}"
                       "link{}cdid{u1}type{1}");
    const QString l25 =
        QStringLiteral("trgt{The quick fox}srce{aux}slch{}slde{}slen{}sles{}"
                       "slfr{}slit{}slja{}slpt{}slru{}slvi{}exmp{ }defn{}note{}"
                       "wrds{}grmr{}tags{}mark{FALSE}refr{}imag{0}imgr{im_1}"
                       "link{}cdid{u2}type{2}");
    const QString lUni =
        QStringLiteral("trgt{ünïcödé 政}srce{n}exmp{}defn{}note{}wrds{}grmr{}"
                       "tags{}mark{FALSE}refr{}imag{0}link{}cdid{u3}type{2}");
    const QString original = l14 + QLatin1Char('\n') + l25 + QLatin1Char('\n')
        + lUni + QLatin1Char('\n');
    {
        QFile data(dataPath);
        QVERIFY(data.open(QIODevice::WriteOnly | QIODevice::Text));
        data.write(original.toUtf8());
        data.close();
    }

    const auto items = repo.loadItems(QStringLiteral("English"), QStringLiteral("T"));
    QCOMPARE(items.size(), 3);
    QCOMPARE(items[0].item.trgt, QStringLiteral("Pummeled"));
    QCOMPARE(items[0].item.type, QStringLiteral("1"));
    QCOMPARE(items[0].item.note, QStringLiteral("hi'lo"));
    QCOMPARE(items[0].item.mark, QStringLiteral("TRUE"));
    QCOMPARE(items[1].item.trgt, QStringLiteral("The quick fox"));
    QCOMPARE(items[1].item.exmp, QStringLiteral(" "));
    QCOMPARE(items[2].item.trgt, QStringLiteral("ünïcödé 政"));

    // writeItems sin cambios -> round-trip byte a byte.
    QVERIFY(repo.writeItems(QStringLiteral("English"), QStringLiteral("T"), items));
    QCOMPARE(readAll(dataPath), original);
}

void TestTopicDataRepository::decode25FieldPreservesSlExtraSegments()
{
    const FlatItemCodec::DecodedItem d = FlatItemCodec::decodeWithSegments(
        QStringLiteral("trgt{FOO}srce{bar}slch{zh}slvi{vn}exmp{}defn{}note{}"
                       "wrds{}grmr{}tags{}mark{FALSE}refr{}imag{0}imgr{kk}"
                       "link{}cdid{c}type{2}"));
    QCOMPARE(d.item.trgt, QStringLiteral("FOO"));
    QCOMPARE(d.item.srce, QStringLiteral("bar"));
    QStringList segNames;
    for (const auto &s : d.segments)
        segNames.append(s.first);
    QCOMPARE(segNames.count(QStringLiteral("slch")), 1);
    QCOMPARE(segNames.count(QStringLiteral("slvi")), 1);
    QCOMPARE(segNames.count(QStringLiteral("imgr")), 1);
}

void TestTopicDataRepository::appendItemWord()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const AppPaths paths(tmp.path());
    QVERIFY(TopicRepository(paths).createTopic(QStringLiteral("English"),
                                               makeInfo(QStringLiteral("T"))));
    TopicDataRepository repo(paths);

    Item it = makeItem(QStringLiteral("wordapp"), QStringLiteral("E!"), QStringLiteral("1"));
    QVERIFY(repo.appendItem(QStringLiteral("English"), QStringLiteral("T"), it));

    const QString conf = paths.topicConfDir(QStringLiteral("English"), QStringLiteral("T"));
    const QString line = readAll(conf + QStringLiteral("/data")).trimmed();

    // type 1: unset wrds grmr link defn (exmp/note/tags/mark/refr/imag/cdid se guardan).
    Item expected = it;
    expected.wrds.clear();
    expected.grmr.clear();
    expected.link.clear();
    expected.defn.clear();
    QCOMPARE(line, FlatItemCodec::encode(expected));

    QCOMPARE(dbList(paths, QStringLiteral("learning")),
             QStringList{QStringLiteral("wordapp")});
    QCOMPARE(dbList(paths, QStringLiteral("words")),
             QStringList{QStringLiteral("wordapp")});
    QVERIFY(dbList(paths, QStringLiteral("sentences")).isEmpty());

    // índice: 3 líneas finales trgt / FALSE / srce.
    const QString index = readAll(conf + QStringLiteral("/index"));
    QVERIFY(index.endsWith(QStringLiteral("wordapp\nFALSE\nE!\n")));
}

void TestTopicDataRepository::appendItemSentence()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const AppPaths paths(tmp.path());
    QVERIFY(TopicRepository(paths).createTopic(QStringLiteral("English"),
                                               makeInfo(QStringLiteral("T"))));
    TopicDataRepository repo(paths);

    Item it = makeItem(QStringLiteral("s1"), QStringLiteral("S"), QStringLiteral("2"));
    QVERIFY(repo.appendItem(QStringLiteral("English"), QStringLiteral("T"), it));

    // type != 1: unset defn (wrds/grmr/link se conservan).
    Item expected = it;
    expected.defn.clear();
    const QString line =
        readAll(paths.topicConfDir(QStringLiteral("English"), QStringLiteral("T"))
                + QStringLiteral("/data")).trimmed();
    QCOMPARE(line, FlatItemCodec::encode(expected));
    QCOMPARE(dbList(paths, QStringLiteral("learning")), QStringList{QStringLiteral("s1")});
    QVERIFY(dbList(paths, QStringLiteral("words")).isEmpty());
    QCOMPARE(dbList(paths, QStringLiteral("sentences")), QStringList{QStringLiteral("s1")});
}

void TestTopicDataRepository::appendItemValidations()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const AppPaths paths(tmp.path());
    QVERIFY(TopicRepository(paths).createTopic(QStringLiteral("English"),
                                               makeInfo(QStringLiteral("T"))));
    TopicDataRepository repo(paths);

    // trgt vacío -> false.
    Item empty = makeItem(QString(), QStringLiteral("x"), QStringLiteral("1"));
    QVERIFY(!repo.appendItem(QStringLiteral("English"), QStringLiteral("T"), empty));

    // duplicado exacto -> false (silencioso, guard trgt{<trgt>).
    Item a = makeItem(QStringLiteral("wordapp"), QStringLiteral("x"), QStringLiteral("1"));
    QVERIFY(repo.appendItem(QStringLiteral("English"), QStringLiteral("T"), a));
    QVERIFY(!repo.appendItem(QStringLiteral("English"), QStringLiteral("T"), a));

    // prefijo -> guard `grep -Fo "trgt{${trgt}}"` también lo salta.
    Item b = makeItem(QStringLiteral("wordap"), QStringLiteral("y"), QStringLiteral("1"));
    QVERIFY(!repo.appendItem(QStringLiteral("English"), QStringLiteral("T"), b));

    const auto items = repo.loadItems(QStringLiteral("English"), QStringLiteral("T"));
    QCOMPARE(items.size(), 1);
    QCOMPARE(dbList(paths, QStringLiteral("learning")).size(), 1);
}

void TestTopicDataRepository::updateItemFieldEditsPreserveUnknownSegments()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const AppPaths paths(tmp.path());
    QVERIFY(TopicRepository(paths).createTopic(QStringLiteral("English"),
                                               makeInfo(QStringLiteral("T"))));
    TopicDataRepository repo(paths);

    const QString conf = paths.topicConfDir(QStringLiteral("English"), QStringLiteral("T"));
    Item it = makeItem(QStringLiteral("FOO"), QStringLiteral("bar"), QStringLiteral("2"));
    QVERIFY(repo.appendItem(QStringLiteral("English"), QStringLiteral("T"), it));
    const QString indexAfterAppend = readAll(conf + QStringLiteral("/index"));

    // Reescritura a línea 25 campos con slch/slvi/imgr poblados.
    const QString dataPath = conf + QStringLiteral("/data");
    const QString line25 =
        QStringLiteral("trgt{FOO}srce{bar}slch{zh}slvi{vn}exmp{ex}defn{df}note{nt}"
                       "wrds{wr}grmr{gr}tags{tg}mark{FALSE}refr{rf}imag{0}imgr{kk}"
                       "link{li}cdid{cu}type{2}");
    {
        QFile f(dataPath);
        QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Text));
        f.write(line25.toUtf8());
        f.write("\n");
        f.close();
    }

    // Edición srce+exmp+note (sin rename/mark/type): índices quedan obsoletos.
    Item changes = it;
    changes.srce = QStringLiteral("barn");
    changes.exmp = QStringLiteral("exn");
    changes.note = QStringLiteral("ntn");
    QVERIFY(repo.updateItem(QStringLiteral("English"), QStringLiteral("T"),
                            QStringLiteral("FOO"), changes));

    const QString edited = readAll(dataPath);
    QVERIFY(edited.contains(QStringLiteral("trgt{FOO}")));
    QVERIFY(edited.contains(QStringLiteral("srce{barn}")));
    QVERIFY(edited.contains(QStringLiteral("exmp{exn}")));
    QVERIFY(edited.contains(QStringLiteral("note{ntn}")));
    // los segmentos desconocidos se conservan byte a byte y en su orden.
    QVERIFY(edited.contains(QStringLiteral("slch{zh}slvi{vn}")));
    QVERIFY(edited.contains(QStringLiteral("imgr{kk}")));

    // srce/exmp/note -> el índice NO se regenera (quirk del original).
    QCOMPARE(readAll(conf + QStringLiteral("/index")), indexAfterAppend);

    // Cambio de mark a TRUE: SÍ regenera el índice (colorize_run=1) y marca.
    Item marked = it;
    marked.srce = QStringLiteral("barn");
    marked.mark = QStringLiteral("TRUE");
    QVERIFY(repo.updateItem(QStringLiteral("English"), QStringLiteral("T"),
                            QStringLiteral("FOO"), marked));
    QCOMPARE(dbList(paths, QStringLiteral("marks")), QStringList{QStringLiteral("FOO")});
    const QString indexAfterMark = readAll(conf + QStringLiteral("/index"));
    QVERIFY(indexAfterMark.startsWith(QStringLiteral("<b><big>FOO</big></b>\n")));
    QVERIFY(indexAfterMark.endsWith(QStringLiteral("barn\n")));

    // Edición de srce únicamente (sin tocar mark): el índice no se regenera,
    // queda obsoleto (quirk edit_item: srce no dispara colorize_run).
    Item srceOnly = marked; // mark TRUE ya en el estado actual
    srceOnly.srce = QStringLiteral("barbar");
    QVERIFY(repo.updateItem(QStringLiteral("English"), QStringLiteral("T"),
                            QStringLiteral("FOO"), srceOnly));
    QCOMPARE(readAll(conf + QStringLiteral("/index")), indexAfterMark);
}

void TestTopicDataRepository::updateItemRenameMovesTablesAndPracticeLogs()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const AppPaths paths(tmp.path());
    QVERIFY(TopicRepository(paths).createTopic(QStringLiteral("English"),
                                               makeInfo(QStringLiteral("T"))));
    TopicDataRepository repo(paths);

    const QString conf = paths.topicConfDir(QStringLiteral("English"), QStringLiteral("T"));
    QDir().mkpath(conf + QStringLiteral("/practice"));
    {
        QFile f(conf + QStringLiteral("/practice/log1"));
        QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Text));
        f.write("old\nother\n");
        f.close();
    }

    Item it = makeItem(QStringLiteral("old"), QStringLiteral("sr"), QStringLiteral("2"));
    QVERIFY(repo.appendItem(QStringLiteral("English"), QStringLiteral("T"), it));
    // item en marks y learnt (listas no-learning) para probar el rename completo.
    {
        SqliteDatabase db(paths.topicDb(QStringLiteral("English"), QStringLiteral("T")));
        QVERIFY(db.open());
        QVERIFY(db.execute("insert into learnt (list) values ('old');"));
        QVERIFY(db.execute("insert into marks (list) values ('old');"));
    }

    Item changes = it;
    changes.trgt = QStringLiteral("new");
    QVERIFY(repo.updateItem(QStringLiteral("English"), QStringLiteral("T"),
                            QStringLiteral("old"), changes));

    const QString dataPath = conf + QStringLiteral("/data");
    const QString edited = readAll(dataPath);
    QVERIFY(edited.contains(QStringLiteral("trgt{new}")));
    QVERIFY(!edited.contains(QStringLiteral("trgt{old}")));

    // rename en todas las listas.
    QCOMPARE(dbList(paths, QStringLiteral("learning")), QStringList{QStringLiteral("new")});
    QCOMPARE(dbList(paths, QStringLiteral("learnt")), QStringList{QStringLiteral("new")});
    QCOMPARE(dbList(paths, QStringLiteral("sentences")), QStringList{QStringLiteral("new")});
    QCOMPARE(dbList(paths, QStringLiteral("marks")), QStringList{QStringLiteral("new")});

    // logs de práctica: línea completa renombrada, el resto intacto.
    QCOMPARE(readAll(conf + QStringLiteral("/practice/log1")),
             QStringLiteral("new\nother\n"));

    // índice regenerado por rename: 'new' marcado (bold); sin rastro de 'old'.
    const QString index = readAll(conf + QStringLiteral("/index"));
    QVERIFY(index.contains(QStringLiteral("<b><big>new</big></b>\nFALSE")));
    QVERIFY(!index.contains(QStringLiteral("old")));
}

void TestTopicDataRepository::updateItemRenameGrmrQuirk()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const AppPaths paths(tmp.path());
    QVERIFY(TopicRepository(paths).createTopic(QStringLiteral("English"),
                                               makeInfo(QStringLiteral("T"))));
    TopicDataRepository repo(paths);

    // word (type 1): el append deja grmr vacío; el llamador pasa el valor
    // actual (grmr=""), así que el sed old==new NO reescribe y el quirk del
    // rename deja grmr VACÍO (nada que ver con el "g" del template).
    Item w = makeItem(QStringLiteral("wold"), QStringLiteral("s"), QStringLiteral("1"));
    w.grmr = QString();
    QVERIFY(repo.appendItem(QStringLiteral("English"), QStringLiteral("T"), w));
    Item wc = w;
    wc.trgt = QStringLiteral("wnew");
    QVERIFY(repo.updateItem(QStringLiteral("English"), QStringLiteral("T"),
                            QStringLiteral("wold"), wc));

    // sentence (type 2): grmr = nuevo trgt (quirk sed original).
    Item s2 = makeItem(QStringLiteral("sold"), QStringLiteral("s"), QStringLiteral("2"));
    QVERIFY(repo.appendItem(QStringLiteral("English"), QStringLiteral("T"), s2));
    Item sc = s2;
    sc.trgt = QStringLiteral("snew");
    QVERIFY(repo.updateItem(QStringLiteral("English"), QStringLiteral("T"),
                            QStringLiteral("sold"), sc));

    const auto items = repo.loadItems(QStringLiteral("English"), QStringLiteral("T"));
    QCOMPARE(items.size(), 2);
    if (items[0].item.trgt == QStringLiteral("wnew")) {
        QVERIFY(items[0].item.grmr.isEmpty());
        QCOMPARE(items[1].item.grmr, QStringLiteral("snew"));
    } else {
        QVERIFY(items[1].item.grmr.isEmpty());
        QCOMPARE(items[0].item.grmr, QStringLiteral("snew"));
    }
}

void TestTopicDataRepository::updateItemTypeSwapMovesTable()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const AppPaths paths(tmp.path());
    QVERIFY(TopicRepository(paths).createTopic(QStringLiteral("English"),
                                               makeInfo(QStringLiteral("T"))));
    TopicDataRepository repo(paths);

    Item it = makeItem(QStringLiteral("swap"), QStringLiteral("s"), QStringLiteral("2"));
    QVERIFY(repo.appendItem(QStringLiteral("English"), QStringLiteral("T"), it));
    QCOMPARE(dbList(paths, QStringLiteral("sentences")), QStringList{QStringLiteral("swap")});

    Item changes = it;
    changes.type = QStringLiteral("1");
    QVERIFY(repo.updateItem(QStringLiteral("English"), QStringLiteral("T"),
                            QStringLiteral("swap"), changes));

    QVERIFY(dbList(paths, QStringLiteral("sentences")).isEmpty());
    QCOMPARE(dbList(paths, QStringLiteral("words")), QStringList{QStringLiteral("swap")});
    const auto items = repo.loadItems(QStringLiteral("English"), QStringLiteral("T"));
    QCOMPARE(items[0].item.type, QStringLiteral("1"));
}

void TestTopicDataRepository::updateItemMarkFalseIsNoOp()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const AppPaths paths(tmp.path());
    QVERIFY(TopicRepository(paths).createTopic(QStringLiteral("English"),
                                               makeInfo(QStringLiteral("T"))));
    TopicDataRepository repo(paths);

    Item it = makeItem(QStringLiteral("m1"), QStringLiteral("s"), QStringLiteral("1"));
    QVERIFY(repo.appendItem(QStringLiteral("English"), QStringLiteral("T"), it));

    // TRUE -> inserta en marks.
    Item on = it;
    on.mark = QStringLiteral("TRUE");
    QVERIFY(repo.updateItem(QStringLiteral("English"), QStringLiteral("T"),
                            QStringLiteral("m1"), on));
    QCOMPARE(dbList(paths, QStringLiteral("marks")), QStringList{QStringLiteral("m1")});

    // FALSE -> NO-OP (bug `tpc_db 4 marks "$trgt"` del original): NO se borra.
    Item off = it;
    off.mark = QStringLiteral("FALSE");
    QVERIFY(repo.updateItem(QStringLiteral("English"), QStringLiteral("T"),
                            QStringLiteral("m1"), off));
    QCOMPARE(dbList(paths, QStringLiteral("marks")), QStringList{QStringLiteral("m1")});
}

void TestTopicDataRepository::removeItemRemovesEveryThing()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const AppPaths paths(tmp.path());
    QVERIFY(TopicRepository(paths).createTopic(QStringLiteral("English"),
                                               makeInfo(QStringLiteral("T"))));
    TopicDataRepository repo(paths);

    const QString conf = paths.topicConfDir(QStringLiteral("English"), QStringLiteral("T"));
    const QString dataDir = paths.topicDataDir(QStringLiteral("English"), QStringLiteral("T"));
    const QString dbPath = paths.topicDb(QStringLiteral("English"), QStringLiteral("T"));

    Item victim = makeItem(QStringLiteral("gone"), QStringLiteral("s"), QStringLiteral("2"));
    victim.cdid = QStringLiteral("abc123");
    QVERIFY(repo.appendItem(QStringLiteral("English"), QStringLiteral("T"), victim));
    Item keep = makeItem(QStringLiteral("stay"), QStringLiteral("s"), QStringLiteral("1"));
    QVERIFY(repo.appendItem(QStringLiteral("English"), QStringLiteral("T"), keep));

    // ficheros físicos y logs.
    QDir().mkpath(dataDir + QStringLiteral("/images"));
    QDir().mkpath(conf + QStringLiteral("/practice"));
    {
        QFile f(dataDir + QStringLiteral("/abc123.mp3"));
        f.open(QIODevice::WriteOnly); f.write("x"); f.close();
    }
    {
        QFile f(dataDir + QStringLiteral("/images/gone.jpg"));
        f.open(QIODevice::WriteOnly); f.write("x"); f.close();
    }
    {
        QFile f(conf + QStringLiteral("/practice/log1"));
        f.open(QIODevice::WriteOnly);
        f.write("gone\n\nstay\n");
        f.close();
    }
    QDir().mkpath(conf);
    {
        QFile f(conf + QStringLiteral("/lk"));
        f.open(QIODevice::WriteOnly); f.write("x"); f.close();
    }
    {
        QFile f(conf + QStringLiteral("/feeds"));
        f.open(QIODevice::WriteOnly); f.write("x"); f.close();
    }

    QVERIFY(repo.removeItem(QStringLiteral("English"), QStringLiteral("T"),
                            QStringLiteral("gone")));

    const auto items = repo.loadItems(QStringLiteral("English"), QStringLiteral("T"));
    QCOMPARE(items.size(), 1);
    QCOMPARE(items[0].item.trgt, QStringLiteral("stay"));

    // listas sin rastro (learning del que se borró).
    QVERIFY(dbList(paths, QStringLiteral("learning")) ==
            QStringList{QStringLiteral("stay")});
    for (const QString &ta : {QStringLiteral("learnt"), QStringLiteral("words"),
                              QStringLiteral("sentences"), QStringLiteral("marks")}) {
        for (const QString &v : dbList(paths, ta))
            QVERIFY2(v != QStringLiteral("gone"), qPrintable(ta));
    }

    // media borrada.
    QVERIFY(!QFileInfo::exists(dataDir + QStringLiteral("/abc123.mp3")));
    QVERIFY(!QFileInfo::exists(dataDir + QStringLiteral("/images/gone.jpg")));

    // data < 200 y lk existía -> lk fuera; feeds existía -> exclude con gone.
    QVERIFY(!QFileInfo::exists(conf + QStringLiteral("/lk")));
    QVERIFY(QFileInfo::exists(conf + QStringLiteral("/exclude")));
    QVERIFY(readAll(conf + QStringLiteral("/exclude")).trimmed() ==
            QStringLiteral("gone"));

    // log1 con entradas exactas + hueco: gone cae, stay queda, sin vacíos.
    QCOMPARE(readAll(conf + QStringLiteral("/practice/log1")), QStringLiteral("stay\n"));

    // índice regenerado.
    const QString index = readAll(conf + QStringLiteral("/index"));
    QVERIFY(index.contains(QStringLiteral("stay")));
    QVERIFY(!index.contains(QStringLiteral("gone")));
}

void TestTopicDataRepository::validateIndexAndRebuild()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const AppPaths paths(tmp.path());
    QVERIFY(TopicRepository(paths).createTopic(QStringLiteral("English"),
                                               makeInfo(QStringLiteral("T"))));
    TopicDataRepository repo(paths);

    Item it = makeItem(QStringLiteral("v1"), QStringLiteral("sr"), QStringLiteral("1"));
    QVERIFY(repo.appendItem(QStringLiteral("English"), QStringLiteral("T"), it));

    auto rep = repo.validateIndex(QStringLiteral("English"), QStringLiteral("T"));
    QCOMPARE(rep.indexCount, 1);
    QCOMPARE(rep.learningCount, 1);
    QVERIFY(rep.valid);

    // índice corrupto -> invalid y rebuild correcto.
    const QString conf = paths.topicConfDir(QStringLiteral("English"), QStringLiteral("T"));
    {
        QFile f(conf + QStringLiteral("/index"));
        QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Text));
        f.write("junk\n");
        f.close();
    }
    rep = repo.validateIndex(QStringLiteral("English"), QStringLiteral("T"));
    QCOMPARE(rep.indexCount, 0);
    QVERIFY(!rep.valid);

    // rebuildIndex regenera las 3 líneas desde data+learning.
    QVERIFY(repo.rebuildIndex(QStringLiteral("English"), QStringLiteral("T"), false));
    const QString index = readAll(conf + QStringLiteral("/index"));
    QVERIFY(index == QStringLiteral("v1\nFALSE\nsr\n"));
}

void TestTopicDataRepository::sttsAndNoteFileOps()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const AppPaths paths(tmp.path());
    QVERIFY(TopicRepository(paths).createTopic(QStringLiteral("English"),
                                               makeInfo(QStringLiteral("T"))));
    TopicDataRepository repo(paths);

    const QString conf = paths.topicConfDir(QStringLiteral("English"), QStringLiteral("T"));

    QCOMPARE(repo.readStts(QStringLiteral("English"), QStringLiteral("T")),
             QStringLiteral("1"));
    QVERIFY(repo.writeStts(QStringLiteral("English"), QStringLiteral("T"),
                           QStringLiteral("4")));
    QCOMPARE(repo.readStts(QStringLiteral("English"), QStringLiteral("T")),
             QStringLiteral("4"));

    QVERIFY(repo.moveSttsToBackup(QStringLiteral("English"), QStringLiteral("T")));
    QVERIFY(QFileInfo::exists(conf + QStringLiteral("/stts.bk")));
    QCOMPARE(repo.readStts(QStringLiteral("English"), QStringLiteral("T")),
             QStringLiteral("0"));
    QVERIFY(repo.restoreSttsFromBackup(QStringLiteral("English"), QStringLiteral("T")));
    QCOMPARE(repo.readStts(QStringLiteral("English"), QStringLiteral("T")),
             QStringLiteral("4"));
    QVERIFY(!QFileInfo::exists(conf + QStringLiteral("/stts.bk")));

    QVERIFY(repo.writeNote(QStringLiteral("English"), QStringLiteral("T"),
                           QStringLiteral("nota\" 'x'")));
    QCOMPARE(repo.readNote(QStringLiteral("English"), QStringLiteral("T")),
             QStringLiteral("nota\" 'x'"));
}

QTEST_GUILESS_MAIN(TestTopicDataRepository)
#include "tst_topicdatarepository.moc"