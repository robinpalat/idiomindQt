#include "storage/topic/TopicDataRepository.h"

#include "storage/dbs/SqliteDatabase.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileDevice>
#include <QFileInfo>
#include <QSet>
#include <QSqlQuery>
#include <QStringList>

using namespace ::idiomind::core;
using namespace ::idiomind::core::FlatItemCodec;

namespace idiomind {
namespace storage {

using core::Item;

namespace {

QString quote(const QString &v)
{
    return QString(v).replace(QLatin1Char('\''), QStringLiteral("''"));
}

// <fichero> -> líneas (terminador de línea apartado, contenido intacto).
QStringList readLinesOf(const QString &filePath)
{
    QStringList out;
    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return out;
    while (!f.atEnd()) {
        QByteArray raw = f.readLine();
        while (raw.endsWith('\n') || raw.endsWith('\r'))
            raw.chop(1);
        out.append(QString::fromUtf8(raw));
    }
    return out;
}

// Contenido completo de un fichero (equivale a `$(< file)` del original).
QString readAllOf(const QString &filePath)
{
    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return QString();
    const QString out = QString::fromUtf8(f.readAll());
    f.close();
    return out;
}

} // namespace

TopicDataRepository::TopicDataRepository(core::AppPaths appPaths)
    : m_appPaths(std::move(appPaths))
{
}

QList<TopicDataRepository::DecodedItem> TopicDataRepository::loadItems(
    const QString &tlng, const QString &topic) const
{
    QList<DecodedItem> items;
    const QString conf = m_appPaths.topicConfDir(tlng, topic);
    for (const QString &line : readLinesOf(conf + QStringLiteral("/data"))) {
        if (line.isEmpty())
            continue;
        items.append(FlatItemCodec::decodeWithSegments(line));
    }
    return items;
}

bool TopicDataRepository::writeItems(const QString &tlng, const QString &topic,
                                     const QList<DecodedItem> &items) const
{
    const QString path = m_appPaths.topicConfDir(tlng, topic) + QStringLiteral("/data");
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;
    for (const DecodedItem &d : items) {
        f.write(FlatItemCodec::reencodeItem(d, d.item).toUtf8());
        f.write("\n");
    }
    f.close();
    return true;
}

bool TopicDataRepository::appendItem(const QString &tlng, const QString &topic,
                                     const Item &item)
{
    if (item.trgt.isEmpty())
        return false;
    const QString conf = m_appPaths.topicConfDir(tlng, topic);
    if (!QFileInfo::exists(conf) || !QFileInfo::exists(conf + QStringLiteral("/data")))
        return false;

    // Guard del original: `grep -Fo "trgt{${trgt}}"` => coincidencia literal
    // `trgt{<trgt>` (subcadena; puede falsar "trgt{ab" sobre "trgt{abc}").
    const QString guard = QStringLiteral("trgt{") + item.trgt;
    for (const QString &line : readLinesOf(conf + QStringLiteral("/data"))) {
        if (line.contains(guard))
            return false;
    }

    Item stored = item;
    const bool isWord = (item.type == QLatin1String("1"));
    if (isWord) { // type 1: unset wrds grmr link defn
        stored.wrds.clear();
        stored.grmr.clear();
        stored.link.clear();
        stored.defn.clear();
    } else {      // type 2: unset defn
        stored.defn.clear();
    }

    const QString dbPath = m_appPaths.topicDb(tlng, topic);
    insertRow(dbPath, QStringLiteral("learning"), stored.trgt);
    insertRow(dbPath, isWord ? QStringLiteral("words") : QStringLiteral("sentences"),
              stored.trgt);

    // índice: `echo -e "${trgt}\nFALSE\n${srce}" >> index`
    {
        QFile idx(conf + QStringLiteral("/index"));
        if (idx.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
            idx.write(stored.trgt.toUtf8());
            idx.write("\nFALSE\n");
            idx.write(stored.srce.toUtf8());
            idx.write("\n");
            idx.close();
        }
    }

    // data: `eval newline=$(sed -n 2p vars); echo "${newline}" >> data`
    {
        QFile data(conf + QStringLiteral("/data"));
        if (data.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
            data.write(FlatItemCodec::encode(stored).toUtf8());
            data.write("\n");
            data.close();
        }
    }
    bumpTopicMtime(m_appPaths.topicDataDir(tlng, topic));
    return true;
}

bool TopicDataRepository::updateItem(const QString &tlng, const QString &topic,
                                     const QString &targetTrgt, const Item &changes)
{
    if (targetTrgt.isEmpty())
        return false;
    QList<DecodedItem> items = loadItems(tlng, topic);
    int index = -1;
    for (int i = 0; i < items.size(); ++i) {
        if (items.at(i).item.trgt == targetTrgt) { index = i; break; }
    }
    if (index < 0) {
        // subcadena `trgt{<target>` (hazard del grep original)
        const QString guard = QStringLiteral("trgt{") + targetTrgt;
        for (int i = 0; i < items.size(); ++i) {
            const QString line = FlatItemCodec::reencodeItem(items.at(i), items.at(i).item);
            if (line.contains(guard)) { index = i; break; }
        }
    }
    if (index < 0)
        return false;

    Item cur = items.at(index).item;
    // Referencia para la regla `sed old==new` (no-op): el original compara
    // cada campo con el valor LEÍDO, no con el resultado de los renames.
    const Item orig = cur;
    const bool typeChanged = !changes.type.isEmpty() && changes.type != cur.type;
    const bool markChanged = !changes.mark.isEmpty() && changes.mark != cur.mark;
    const bool renamed =
        !changes.trgt.isEmpty() && changes.trgt != cur.trgt;

    if (renamed) {
        // index edit: rename en las 5 listas + practice logs (línea completa).
        const QString dbPath = m_appPaths.topicDb(tlng, topic);
        for (const QString &ta : {QStringLiteral("learning"), QStringLiteral("learnt"),
                                  QStringLiteral("words"), QStringLiteral("sentences"),
                                  QStringLiteral("marks")}) {
            updateListRows(dbPath, ta, changes.trgt, cur.trgt);
        }
        const QString conf = m_appPaths.topicConfDir(tlng, topic);
        QDir pd(conf + QStringLiteral("/practice"));
        const QStringList logs = pd.entryList(QDir::Files | QDir::NoDotAndDotDot);
        for (const QString &name : logs) {
            const QString path = conf + QStringLiteral("/practice/") + name;
            QStringList lines = readLinesOf(path);
            bool touched = false;
            for (QString &l : lines) {
                if (l == cur.trgt) { l = changes.trgt; touched = true; }
            }
            if (touched) {
                QFile f(path);
                if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
                    for (const QString &l : lines) {
                        f.write(l.toUtf8());
                        if (!l.isEmpty())
                            f.write("\n");
                    }
                    f.close();
                }
            }
        }
        cur.trgt = changes.trgt;
        // sed original: grmr{old}->grmr{nuevo trgt} (el bloque final deja
        // grmr="" en words y grmr=nuevo trgt en sentences).
        const bool finalSentence = typeChanged
            ? (changes.type != QLatin1String("1"))
            : (cur.type != QLatin1String("1"));
        cur.grmr = finalSentence ? changes.trgt : QString();
    }

    // Campos editables (sed por nombre de campo, bloque final de edit_item).
    // La comparación usa el valor ORIGINAL leído (orig): si el llamador pasa
    // el valor actual sin cambios, no se reescribe (`sed old==new` no-op).
    if (typeChanged)        cur.type = changes.type;
    if (changes.srce != orig.srce) cur.srce = changes.srce;
    if (changes.exmp != orig.exmp) cur.exmp = changes.exmp;
    if (changes.defn != orig.defn) cur.defn = changes.defn;
    if (changes.note != orig.note) cur.note = changes.note;
    if (changes.wrds != orig.wrds) cur.wrds = changes.wrds;
    if (changes.grmr != orig.grmr) cur.grmr = changes.grmr;
    if (markChanged)        cur.mark = changes.mark;
    if (changes.cdid != orig.cdid) cur.cdid = changes.cdid;

    const QString finalTrgt = cur.trgt;
    const QString dbPath = m_appPaths.topicDb(tlng, topic);

    if (typeChanged) {
        // mover entre words/sentences (delete tabla antigua + insert nueva).
        const QString oldTable = (changes.type == QLatin1String("1"))
            ? QStringLiteral("sentences") : QStringLiteral("words");
        const QString newTable = (changes.type == QLatin1String("1"))
            ? QStringLiteral("words") : QStringLiteral("sentences");
        deleteRow(dbPath, oldTable, finalTrgt);
        insertRow(dbPath, newTable, finalTrgt);
    }
    if (markChanged && changes.mark == QLatin1String("TRUE")) {
        insertRow(dbPath, QStringLiteral("marks"), finalTrgt);
        // mark=FALSE: NO-OP (reproduce `tpc_db 4 marks "$trgt"` del original,
        // que borra con la columna equivocada y no toca la tabla).
    }

    items[index].item = cur;
    writeItems(tlng, topic, items);

    // El original (edit_item) regenera el índice SOLO cuando: hubo rename,
    // cambió mark, o el tipo cambió A word (`type != type_mod && type_mod=1`).
    // Cambios de srce/exmp/defn/note y type->sentence NO lo refrescan (quirk).
    if (renamed || markChanged || (typeChanged && changes.type == QLatin1String("1")))
        rebuildIndex(tlng, topic, false);
    bumpTopicMtime(m_appPaths.topicDataDir(tlng, topic));
    return true;
}

bool TopicDataRepository::removeItem(const QString &tlng, const QString &topic,
                                     const QString &trgt)
{
    if (trgt.isEmpty())
        return false;
    const QString conf = m_appPaths.topicConfDir(tlng, topic);
    const QString dataPath = conf + QStringLiteral("/data");
    if (!QFileInfo::exists(dataPath))
        return false;

    QList<DecodedItem> items = loadItems(tlng, topic);
    const QString guard = QStringLiteral("trgt{") + trgt;
    QString cdid;
    int index = -1;
    for (int i = 0; i < items.size(); ++i) {
        if (items.at(i).item.trgt == trgt) { index = i; break; }
    }
    if (index < 0) {
        for (int i = 0; i < items.size(); ++i) {
            const QString line = FlatItemCodec::reencodeItem(items.at(i), items.at(i).item);
            if (line.contains(guard)) { index = i; break; }
        }
    }
    if (index >= 0) {
        cdid = items.at(index).item.cdid;
        items.removeAt(index);
        writeItems(tlng, topic, items);
    }

    // practice logs: quitar coincidencias de línea entera (grep -vxF), como
    // hace delete_item_ok; las líneas vacías se descartan en la reescritura.
    QDir pd(conf + QStringLiteral("/practice"));
    const QStringList logs = pd.entryList(QDir::Files | QDir::NoDotAndDotDot);
    for (const QString &name : logs) {
        const QString path = conf + QStringLiteral("/practice/") + name;
        QStringList kept;
        for (const QString &l : readLinesOf(path))
            if (l != trgt && !l.isEmpty())
                kept.append(l);
        QFile f(path);
        if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
            for (const QString &l : kept)
                f.write(l.toUtf8() + "\n");
            f.close();
        }
    }

    const QString dbPath = m_appPaths.topicDb(tlng, topic);
    for (const QString &ta : {QStringLiteral("learning"), QStringLiteral("learnt"),
                              QStringLiteral("words"), QStringLiteral("sentences"),
                              QStringLiteral("marks")}) {
        deleteRow(dbPath, ta, trgt);
    }

    // media: mp3 por cdid, imagen por trgt en minúsculas (cleanups).
    const QString dataDir = m_appPaths.topicDataDir(tlng, topic);
    if (!cdid.isEmpty())
        QFile::remove(dataDir + QLatin1Char('/') + cdid + QStringLiteral(".mp3"));
    QFile::remove(dataDir + QStringLiteral("/images/") + trgt.toLower() + QStringLiteral(".jpg"));

    if (readLinesOf(dataPath).size() < 200) {
        const QString lk = conf + QStringLiteral("/lk");
        if (QFileInfo::exists(lk))
            QFile::remove(lk);
    }
    const QString feeds = conf + QStringLiteral("/feeds");
    if (QFileInfo::exists(feeds)) {
        QFile excl(conf + QStringLiteral("/exclude"));
        if (excl.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
            excl.write(trgt.toUtf8());
            excl.write("\n");
            excl.close();
        }
    }
    rebuildIndex(tlng, topic, false);
    bumpTopicMtime(m_appPaths.topicDataDir(tlng, topic));
    return true;
}

// ---------------------------------------------------------------------------
// Índice
// ---------------------------------------------------------------------------

QStringList TopicDataRepository::loadIndex(const QString &tlng,
                                           const QString &topic) const
{
    return readLinesOf(m_appPaths.topicConfDir(tlng, topic) + QStringLiteral("/index"));
}

TopicDataRepository::IndexReport TopicDataRepository::validateIndex(
    const QString &tlng, const QString &topic) const
{
    IndexReport rep;
    const QString conf = m_appPaths.topicConfDir(tlng, topic);
    int nonEmpty = 0;
    for (const QString &l : readLinesOf(conf + QStringLiteral("/index")))
        if (!l.trimmed().isEmpty())
            ++nonEmpty;
    rep.indexCount = nonEmpty / 3;
    rep.learningCount = countRows(m_appPaths.topicDb(tlng, topic),
                                  QStringLiteral("learning"));
    rep.valid = (rep.indexCount == rep.learningCount);
    return rep;
}

bool TopicDataRepository::rebuildIndex(const QString &tlng, const QString &topic,
                                       bool chkFlag)
{
    const QString conf = m_appPaths.topicConfDir(tlng, topic);
    const QString dbPath = m_appPaths.topicDb(tlng, topic);
    const QStringList learning = tableRows(dbPath, QStringLiteral("learning"));
    const QStringList marks = tableRows(dbPath, QStringLiteral("marks"));
    const QSet<QString> learnSet(learning.begin(), learning.end());
    const QSet<QString> marksSet(marks.begin(), marks.end());

    // colorize: `item in logN` es SUBSTRING sobre el contenido completo del
    // log (`$(< file)` + `in` de python). Se reproduce igual.
    const QString log1 = readAllOf(conf + QStringLiteral("/practice/log1"));
    const QString log2 = readAllOf(conf + QStringLiteral("/practice/log2"));
    const QString log3 = readAllOf(conf + QStringLiteral("/practice/log3"));

    const QString acheck = configAcheck(dbPath);
    const bool chk = (acheck == QLatin1String("TRUE")) && chkFlag;

    QFile idx(conf + QStringLiteral("/index"));
    if (!idx.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;
    for (const QString &line : readLinesOf(conf + QStringLiteral("/data"))) {
        const QString stripped = line.trimmed();
        if (stripped.isEmpty())
            continue;
        const DecodedItem d = FlatItemCodec::decodeWithSegments(stripped);
        if (!learnSet.contains(d.item.trgt))
            continue;
        QString srce;
        for (const auto &s : d.segments)
            if (s.first == QLatin1String("srce")) { srce = s.second; break; }
        QString text = marksSet.contains(d.item.trgt)
            ? QStringLiteral("<b><big>") + d.item.trgt + QStringLiteral("</big></b>")
            : d.item.trgt;
        QString flag = QStringLiteral("FALSE");
        if (log3.contains(d.item.trgt)) {
            text = QStringLiteral("<span color='#AE3259'>") + text + QStringLiteral("</span>");
        } else if (log2.contains(d.item.trgt)) {
            text = QStringLiteral("<span color='#C15F27'>") + text + QStringLiteral("</span>");
        } else if (log1.contains(d.item.trgt)) {
            flag = chk ? QStringLiteral("TRUE") : QStringLiteral("FALSE");
        }
        idx.write(text.toUtf8());
        idx.write("\n");
        idx.write(flag.toUtf8());
        idx.write("\n");
        idx.write(srce.toUtf8());
        idx.write("\n");
    }
    idx.close();
    bumpTopicMtime(m_appPaths.topicDataDir(tlng, topic));
    return true;
}

// ---------------------------------------------------------------------------
// Ficheros de estado
// ---------------------------------------------------------------------------

QString TopicDataRepository::readStts(const QString &tlng, const QString &topic) const
{
    const QStringList lines =
        readLinesOf(m_appPaths.topicConfDir(tlng, topic) + QStringLiteral("/stts"));
    return lines.isEmpty() ? QString() : lines.first();
}

bool TopicDataRepository::writeStts(const QString &tlng, const QString &topic,
                                    const QString &value) const
{
    QFile f(m_appPaths.topicConfDir(tlng, topic) + QStringLiteral("/stts"));
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;
    f.write(value.toUtf8());
    f.write("\n");
    f.close();
    return true;
}

bool TopicDataRepository::moveSttsToBackup(const QString &tlng, const QString &topic) const
{
    const QString conf = m_appPaths.topicConfDir(tlng, topic);
    if (!QFileInfo::exists(conf + QStringLiteral("/stts")))
        return false;
    if (!QFile::rename(conf + QStringLiteral("/stts"), conf + QStringLiteral("/stts.bk")))
        return false;
    return writeStts(tlng, topic, QStringLiteral("0"));
}

bool TopicDataRepository::restoreSttsFromBackup(const QString &tlng, const QString &topic) const
{
    const QString conf = m_appPaths.topicConfDir(tlng, topic);
    const QStringList lines = readLinesOf(conf + QStringLiteral("/stts.bk"));
    if (lines.isEmpty())
        return false;
    if (!writeStts(tlng, topic, lines.first()))
        return false;
    return QFile::remove(conf + QStringLiteral("/stts.bk"));
}

bool TopicDataRepository::removeSttsBackup(const QString &tlng, const QString &topic) const
{
    return QFile::remove(m_appPaths.topicConfDir(tlng, topic) + QStringLiteral("/stts.bk"));
}

QString TopicDataRepository::readNote(const QString &tlng, const QString &topic) const
{
    QFile f(m_appPaths.topicConfDir(tlng, topic) + QStringLiteral("/note"));
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return QStringLiteral(" ");
    const QString text = QString::fromUtf8(f.readAll());
    f.close();
    return text;
}

bool TopicDataRepository::writeNote(const QString &tlng, const QString &topic,
                                    const QString &text) const
{
    QFile f(m_appPaths.topicConfDir(tlng, topic) + QStringLiteral("/note"));
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;
    f.write(text.toUtf8());
    f.close();
    return true;
}

QStringList TopicDataRepository::learningList(const QString &tlng, const QString &topic) const
{
    return tableRows(m_appPaths.topicDb(tlng, topic), QStringLiteral("learning"));
}

QStringList TopicDataRepository::learntList(const QString &tlng, const QString &topic) const
{
    return tableRows(m_appPaths.topicDb(tlng, topic), QStringLiteral("learnt"));
}

QStringList TopicDataRepository::marksList(const QString &tlng, const QString &topic) const
{
    return tableRows(m_appPaths.topicDb(tlng, topic), QStringLiteral("marks"));
}

bool TopicDataRepository::removeLearningItem(const QString &tlng, const QString &topic,
                                              const QString &trgt)
{
    return deleteRow(m_appPaths.topicDb(tlng, topic),
                     QStringLiteral("learning"), trgt);
}

bool TopicDataRepository::insertLearntItem(const QString &tlng, const QString &topic,
                                            const QString &trgt)
{
    return insertRow(m_appPaths.topicDb(tlng, topic),
                     QStringLiteral("learnt"), trgt);
}

void TopicDataRepository::bumpTopicMtime(const QString &topicDataDirPath)
{
    QFile f(topicDataDirPath);
    f.setFileTime(QDateTime::currentDateTime(), QFileDevice::FileModificationTime);
}

// ---------------------------------------------------------------------------
// helpers estáticos
// ---------------------------------------------------------------------------

QStringList TopicDataRepository::readLines(const QString &filePath)
{
    return readLinesOf(filePath);
}

QStringList TopicDataRepository::practiceLogs(const QString &confDir)
{
    return readLinesOf(confDir + QStringLiteral("/practice/log1"))
        + readLinesOf(confDir + QStringLiteral("/practice/log2"))
        + readLinesOf(confDir + QStringLiteral("/practice/log3"));
}

QStringList TopicDataRepository::tableRows(const QString &dbPath, const QString &table)
{
    QStringList rows;
    SqliteDatabase db(dbPath);
    if (!db.open())
        return rows;
    QSqlQuery q = db.query(QStringLiteral("select * from '%1';").arg(table));
    while (q.isActive() && q.next()) {
        const QString v = q.value(0).toString();
        if (!v.isEmpty())
            rows.append(v);
    }
    return rows;
}

int TopicDataRepository::countRows(const QString &dbPath, const QString &table)
{
    return tableRows(dbPath, table).size();
}

bool TopicDataRepository::updateListRows(const QString &dbPath, const QString &table,
                                         const QString &newValue, const QString &oldValue)
{
    SqliteDatabase db(dbPath);
    if (!db.open())
        return false;
    return db.execute(QStringLiteral("update '%1' set list='%2' where list='%3';")
                          .arg(table, quote(newValue), quote(oldValue)));
}

bool TopicDataRepository::deleteRow(const QString &dbPath, const QString &table,
                                    const QString &value)
{
    SqliteDatabase db(dbPath);
    if (!db.open())
        return false;
    return db.execute(QStringLiteral("delete from '%1' where list='%2';")
                          .arg(table, quote(value)));
}

bool TopicDataRepository::insertRow(const QString &dbPath, const QString &table,
                                    const QString &value)
{
    SqliteDatabase db(dbPath);
    if (!db.open())
        return false;
    return db.execute(QStringLiteral("insert into '%1' (list) values ('%2');")
                          .arg(table, quote(value)));
}

QString TopicDataRepository::configAcheck(const QString &dbPath)
{
    SqliteDatabase db(dbPath);
    if (!db.open())
        return QString();
    return db.value(QStringLiteral("select acheck from config limit 1;"));
}

} // namespace storage
} // namespace idiomind