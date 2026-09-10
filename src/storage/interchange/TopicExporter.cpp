#include "storage/interchange/TopicExporter.h"

#include "core/topic/FlatItemCodec.h"
#include "core/topic/Item.h"
#include "storage/dbs/SqliteDatabase.h"
#include "storage/interchange/IdmndCodec.h"

#include <QCryptographicHash>
#include <QDate>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSqlQuery>

using namespace ::idiomind::core;
using namespace ::idiomind::core::FlatItemCodec;

namespace idiomind {
namespace storage {

using core::Item;

namespace {

// Escaping JSON estándar (idéntico al del resto del proyecto). Se usa para
// todos los valores del head excepto `info` (que ya viene transcodificado con
// \" y \/ como el Bash).
QString jsonEscape(const QString &in)
{
    QString out;
    out.reserve(in.size());
    for (const QChar c : in) {
        switch (c.unicode()) {
        case u'"': out += QStringLiteral("\\\""); break;
        case u'\\': out += QStringLiteral("\\\\"); break;
        case u'\b': out += QStringLiteral("\\b"); break;
        case u'\f': out += QStringLiteral("\\f"); break;
        case u'\n': out += QStringLiteral("\\n"); break;
        case u'\r': out += QStringLiteral("\\r"); break;
        case u'\t': out += QStringLiteral("\\t"); break;
        default:
            if (c.unicode() < 0x20)
                out += QStringLiteral("\\u%1").arg(static_cast<int>(c.unicode()), 4, 16,
                                                   QLatin1Char('0'));
            else
                out += c;
        }
    }
    return out;
}

// `echo -n ${1} | md5sum` en minúsculas (nmfile/get_item). Devuelve el hex.
QByteArray md5Hex(const QByteArray &input)
{
    return QCryptographicHash::hash(input, QCryptographicHash::Md5).toHex();
}

// `pre=$(sed "s/ /_/g;s/'//g" <<< "${tpc:0:15}" | iconv -c -f utf8 -t ascii);
// ilnk="${pre,,}${rand:0:20}"` con rand = md5 de .conf/data.
// iconv -c descarta los caracteres no ASCII (no translitera).
QString slug15(const QString &tpc)
{
    QString pre = tpc.left(15);
    pre.replace(QLatin1Char(' '), QLatin1Char('_'));
    pre.remove(QLatin1Char('\''));
    QString ascii;
    for (const QChar c : pre) {
        if (c.unicode() < 0x80)
            ascii += c;
    }
    return ascii.toLower();
}

// template de default/vars línea 1, con la lista slXX SIN slvi (el template
// ACTUAL del Bash) e imgr/imag sobrescritos por la regla del export.
QString itemJson(const Item &item)
{
    QStringList fields;
    const auto add = [&fields](const QString &k, const QString &v) {
        fields.append(QStringLiteral("\"%1\":\"%2\"").arg(k, jsonEscape(v)));
    };
    add(QStringLiteral("srce"), item.srce);
    for (const char *s : ItemDefs::translationSuffixes) {
        if (QLatin1String(s) == QLatin1String("vi"))
            continue; // el template actual no lleva slvi
        add(QStringLiteral("sl%1").arg(QLatin1String(s)),
            item.translations.value(QLatin1String(s)));
    }
    add(QStringLiteral("exmp"), item.exmp);
    add(QStringLiteral("defn"), item.defn);
    add(QStringLiteral("note"), item.note);
    add(QStringLiteral("wrds"), item.wrds);
    add(QStringLiteral("grmr"), item.grmr);
    add(QStringLiteral("tags"), item.tags);
    add(QStringLiteral("mark"), item.mark);
    add(QStringLiteral("refr"), item.refr);
    add(QStringLiteral("imag"), item.imag);
    add(QStringLiteral("imgr"), item.imgr);
    add(QStringLiteral("link"), item.link);
    add(QStringLiteral("cdid"), item.cdid);
    add(QStringLiteral("type"), item.type);
    return fields.join(QLatin1Char(','));
}

QString capFirst(const QString &s)
{
    if (s.isEmpty())
        return s;
    return s.at(0).toUpper() + s.mid(1);
}

} // namespace

TopicExporter::TopicExporter(core::AppPaths appPaths)
    : m_appPaths(std::move(appPaths))
{
}

QString TopicExporter::transcodeNote(const QString &note)
{
    // sed '/^$/d' | sed ':a;N;$!ba;s/\n/<br><br>/g;s/\&/&amp;/g'
    //        | sed 's|"|\\"|g;s|/|\\/|g'   (orden exacto del Bash)
    QStringList kept;
    for (const QString &line : note.split(QLatin1Char('\n'))) {
        if (!line.trimmed().isEmpty())
            kept.append(line);
    }
    QString out = kept.join(QStringLiteral("<br><br>"));
    out.replace(QLatin1Char('&'), QStringLiteral("&amp;"));
    out.replace(QLatin1Char('"'), QStringLiteral("\\\""));
    out.replace(QLatin1Char('/'), QStringLiteral("\\/"));
    return out;
}

QString TopicExporter::ilnkFallback(const QString &topic, const QByteArray &dataMd5)
{
    return slug15(topic) + QString::fromLatin1(dataMd5.left(20));
}

QByteArray TopicExporter::exportTopic(const QString &tlng, const QString &topic,
                                      bool *ok, QString *error) const
{
    const auto fail = [&](const QString &reason) {
        if (ok) *ok = false;
        if (error) *error = reason;
        return QByteArray{};
    };
    if (ok) *ok = false;

    const QString conf = m_appPaths.topicConfDir(tlng, topic);
    const QString dataDir = m_appPaths.topicDataDir(tlng, topic);
    const QString dataPath = conf + QStringLiteral("/data");
    QFile dataFile(dataPath);
    if (!dataFile.open(QIODevice::ReadOnly))
        return fail(QStringLiteral("data"));
    const QByteArray dataBytes = dataFile.readAll();
    dataFile.close();

    // get_item de cada línea + reglas imag/imgr.
    struct ExportedItem {
        QString trgt;
        QString body; // objeto JSON 23 claves (sin trgt)
    };
    QList<ExportedItem> items;

    // `while read -r _item` + `[ -n "${trgt}" ]`: se omiten líneas vacías y
    // trgt vacío.
    const QStringList lines = QString::fromUtf8(dataBytes).split(QLatin1Char('\n'));
    for (const QString &raw : lines) {
        const QString line = raw.trimmed();
        if (line.isEmpty())
            continue;
        Item item = FlatItemCodec::decode(line);
        if (item.trgt.isEmpty())
            continue;
        if (item.type == QLatin1String("1")) {
            item.imgr = item.trgt.left(1).toLower() + item.trgt.mid(1); // ${trgt,}
            if (QFileInfo::exists(dataDir + QStringLiteral("/images/")
                                  + item.trgt.toLower() + QStringLiteral(".jpg"))) {
                item.imag = QStringLiteral("2"); // imagen propia del topic
            } else if (QFileInfo::exists(m_appPaths.sharedDirForLanguage(tlng)
                                         + QStringLiteral("/images/")
                                         + item.trgt.toLower()
                                         + QStringLiteral("-1.jpg"))) {
                item.imag = QStringLiteral("1"); // imagen compartida
            } else {
                item.imag = QStringLiteral("0");
            }
        } else {
            item.imag = QStringLiteral("0");
            item.imgr.clear();
        }
        items.append({item.trgt, itemJson(item)});
    }

    // head: columnas de id (mods/export líneas 38-59) + defaults.
    struct HeadVals {
        QString autr, ctgy, levl, dtec, dtei, dteu, nwrd, nsnt, slng, tlng,
            naud, nimg, nsze, cntt, stts, ilnk;
    } h;
    const QString dbPath = m_appPaths.topicDb(tlng, topic);
    if (QFileInfo::exists(dbPath)) {
        SqliteDatabase db(dbPath);
        if (db.open()) {
            h.autr = db.value(QStringLiteral("select autr from id limit 1;"));
            h.ctgy = db.value(QStringLiteral("select ctgy from id limit 1;"));
            h.levl = db.value(QStringLiteral("select levl from id limit 1;"));
            h.dtec = db.value(QStringLiteral("select dtec from id limit 1;"));
            h.dtei = db.value(QStringLiteral("select dtei from id limit 1;"));
            h.nwrd = db.value(QStringLiteral("select nwrd from id limit 1;"));
            h.nsnt = db.value(QStringLiteral("select nsnt from id limit 1;"));
            h.slng = db.value(QStringLiteral("select slng from id limit 1;"));
            h.tlng = db.value(QStringLiteral("select tlng from id limit 1;"));
            h.naud = db.value(QStringLiteral("select naud from id limit 1;"));
            h.nimg = db.value(QStringLiteral("select nimg from id limit 1;"));
            h.nsze = db.value(QStringLiteral("select nsze from id limit 1;"));
            h.cntt = db.value(QStringLiteral("select cntt from id limit 1;"));
            h.stts = db.value(QStringLiteral("select stts from id limit 1;"));
            h.ilnk = db.value(QStringLiteral("select ilnk from id limit 1;"));
            db.close();
        }
    } else {
        return fail(QStringLiteral("db"));
    }
    h.dteu = QDate::currentDate().toString(QStringLiteral("yyyy-MM-dd"));
    if (h.autr.isEmpty()) h.autr.clear();
    if (h.nwrd.isEmpty()) h.nwrd = QStringLiteral("0");
    if (h.nsnt.isEmpty()) h.nsnt = QStringLiteral("0");
    if (h.naud.isEmpty()) h.naud = QStringLiteral("0");
    if (h.nimg.isEmpty()) h.nimg = QStringLiteral("0");
    if (h.nsze.isEmpty()) h.nsze.clear();
    if (h.stts.isEmpty()) h.stts = QStringLiteral("0");
    if (h.ilnk.isEmpty())
        h.ilnk = ilnkFallback(topic, md5Hex(dataBytes));

    // note -> info (mods/export línea 61-64).
    QFile noteFile(conf + QStringLiteral("/note"));
    QString note;
    if (noteFile.open(QIODevice::ReadOnly))
        note = QString::fromUtf8(noteFile.readAll());
    const QString info = transcodeNote(note);

    // --- ensamblado de las 3 líneas ---
    // línea 1: {"items":{
    // línea 2: "key":{...},"key":{...}} ,  (items + cierre + coma)
    // línea 3: head + }
    QStringList bodies;
    bodies.reserve(items.size());
    for (const ExportedItem &it : items)
        bodies.append(QStringLiteral("\"%1\":{%2}").arg(jsonEscape(it.trgt),
                                                        it.body));
    QString head = QStringLiteral("\"name\":\"%1\",\"slng\":\"%2\",\"tlng\":\"%3\","
                                  "\"autr\":\"%4\",\"cntt\":\"%5\",\"ctgy\":\"%6\","
                                  "\"ilnk\":\"%7\",\"orig\":\"%8\",\"dtec\":\"%9\","
                                  "\"dteu\":\"%10\",\"dtei\":\"%11\",\"nwrd\":\"%12\","
                                  "\"nsnt\":\"%13\",\"nimg\":\"%14\",\"naud\":\"%15\","
                                  "\"nsze\":\"%16\",\"levl\":\"%17\",\"info\":\"%18\","
                                  "\"stts\":\"%19\"")
                       .arg(jsonEscape(topic), jsonEscape(capFirst(h.slng)),
                            jsonEscape(capFirst(h.tlng)), jsonEscape(h.autr),
                            jsonEscape(h.cntt), jsonEscape(h.ctgy),
                            jsonEscape(h.ilnk), jsonEscape(topic),
                            jsonEscape(h.dtec), jsonEscape(h.dteu),
                            jsonEscape(h.dtei), jsonEscape(h.nwrd),
                            jsonEscape(h.nsnt), jsonEscape(h.nimg),
                            jsonEscape(h.naud), jsonEscape(h.nsze),
                            jsonEscape(h.levl), info, jsonEscape(h.stts));

    QString out;
    out += QLatin1String("{\"items\":{\n");
    out += bodies.join(QLatin1Char(','));
    out += QLatin1String("},\n");
    out += head;
    out += QLatin1String("}\n");

    if (ok) *ok = true;
    return out.toUtf8();
}

} // namespace storage
} // namespace idiomind
