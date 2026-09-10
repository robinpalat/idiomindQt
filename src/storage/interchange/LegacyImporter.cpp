#include "storage/interchange/LegacyImporter.h"

#include "storage/dbs/SqliteDatabase.h"
#include "storage/interchange/IdmndCodec.h"
#include "storage/topic/TopicDataRepository.h"
#include "storage/topic/TopicRepository.h"

#include <QDate>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QSqlQuery>
#include <QTemporaryDir>
#include <QUrl>
#include <QtEndian>

#include <cstring>
#include <zlib.h>

using namespace ::idiomind::core;
using namespace ::idiomind::core::FlatItemCodec;

namespace idiomind {
namespace storage {

using core::Item;

namespace {

// Minimal ZIP reader using zlib. Handles the Deflate-compressed ZIP format
// used by the Bash `zip` command (the same format the original export creates).
// Only handles local file headers + deflate; no encryption, no data descriptors.
struct ZipEntry {
    QString name;
    QByteArray data;
};

static bool inflateAll(QByteArray &out, const char *in, size_t inLen)
{
    z_stream strm{};
    if (inflateInit2(&strm, -15) != Z_OK)
        return false;
    strm.next_in = reinterpret_cast<Bytef *>(const_cast<char *>(in));
    strm.avail_in = static_cast<uInt>(inLen);
    char buf[32768];
    int ret;
    do {
        strm.next_out = reinterpret_cast<Bytef *>(buf);
        strm.avail_out = sizeof(buf);
        ret = inflate(&strm, Z_NO_FLUSH);
        if (ret != Z_OK && ret != Z_STREAM_END) {
            inflateEnd(&strm);
            return false;
        }
        out.append(buf, sizeof(buf) - strm.avail_out);
    } while (ret != Z_STREAM_END);
    inflateEnd(&strm);
    return true;
}

static QList<ZipEntry> readZipEntries(const QByteArray &zipData)
{
    QList<ZipEntry> entries;
    // Find End of Central Directory (EOCD) signature 0x06054b50.
    // It's at the end of the file, search backwards.
    const char *data = zipData.constData();
    const size_t len = static_cast<size_t>(zipData.size());
    if (len < 22)
        return entries;

    size_t eocdPos = 0;
    bool found = false;
    for (size_t i = len - 22; i >= 0 && i < len; --i) {
        if (memcmp(data + i, "PK\x05\x06", 4) == 0) {
            eocdPos = i;
            found = true;
            break;
        }
        if (i == 0)
            break;
    }
    if (!found)
        return entries;

    // Read central directory offset from EOCD.
    const quint32 cdOffset = qFromLittleEndian<quint32>(
        reinterpret_cast<const uchar *>(data + eocdPos + 16));

    // Scan central directory entries to find filenames and local header offsets.
    size_t pos = cdOffset;
    while (pos + 46 <= len && memcmp(data + pos, "PK\x01\x02", 4) == 0) {
        const quint16 method = qFromLittleEndian<quint16>(
            reinterpret_cast<const uchar *>(data + pos + 10));
        const quint32 compSize = qFromLittleEndian<quint32>(
            reinterpret_cast<const uchar *>(data + pos + 20));
        const quint32 localOffset = qFromLittleEndian<quint32>(
            reinterpret_cast<const uchar *>(data + pos + 42));
        const quint16 fnameLen = qFromLittleEndian<quint16>(
            reinterpret_cast<const uchar *>(data + pos + 28));
        const quint16 extraLen = qFromLittleEndian<quint16>(
            reinterpret_cast<const uchar *>(data + pos + 30));
        const quint16 commentLen = qFromLittleEndian<quint16>(
            reinterpret_cast<const uchar *>(data + pos + 32));
        const QString name = QString::fromUtf8(data + pos + 46, fnameLen);
        pos += 46 + fnameLen + extraLen + commentLen;
        // Now read the local file header to extract the data.
        const size_t lpos = localOffset;
        if (lpos + 30 > len)
            break;
        if (memcmp(data + lpos, "PK\x03\x04", 4) != 0)
            break;
        const quint16 lfNameLen = qFromLittleEndian<quint16>(
            reinterpret_cast<const uchar *>(data + lpos + 26));
        const quint16 lfExtraLen = qFromLittleEndian<quint16>(
            reinterpret_cast<const uchar *>(data + lpos + 28));
        const char *compData = data + lpos + 30 + lfNameLen + lfExtraLen;

        ZipEntry entry;
        entry.name = name;
        if (method == 0) {
            // Stored (no compression).
            entry.data = QByteArray(compData, static_cast<int>(compSize));
        } else if (method == 8) {
            // Deflate.
            QByteArray inflated;
            if (inflateAll(inflated, compData, static_cast<size_t>(compSize)))
                entry.data = inflated;
        }
        // Skip directory entries and empty names.
        if (!name.isEmpty() && !name.endsWith(QLatin1Char('/')))
            entries.append(entry);
        // Advance past central directory for next entry.
        // Already advanced past fnameLen above; next entry starts at current pos.
    }
    return entries;
}

QString kDateRe = QStringLiteral("^[0-9]{4}-[0-9]{2}-[0-9]{2}$");
const QString kNumerRe = QStringLiteral("^[0-9]+$");

// Nombres de "slangs" (default/sets.cfg líneas 31-84). Se usan EXACTOS
// (incluido el espacio final de 'Soomaali ', quirk del original).
const QStringList kSlangs = {
    QStringLiteral("Afrikaans"),       QStringLiteral("Azərbaycanca"),
    QStringLiteral("Català"),          QStringLiteral("Dansk"),
    QStringLiteral("Deutsch"),         QStringLiteral("English"),
    QStringLiteral("Español"),         QStringLiteral("Filipino"),
    QStringLiteral("Français"),        QStringLiteral("Italiano"),
    QStringLiteral("Kiswahili"),       QStringLiteral("Lietuvių"),
    QStringLiteral("Magyar"),          QStringLiteral("Malagasy"),
    QStringLiteral("Malti"),           QStringLiteral("Nederlands"),
    QStringLiteral("Norsk"),           QStringLiteral("Polski"),
    QStringLiteral("Português"),       QStringLiteral("Română"),
    QStringLiteral("Slovenčina"),      QStringLiteral("Slovenščina"),
    QStringLiteral("Soomaali "),       QStringLiteral("Suomi"),
    QStringLiteral("Svenska"),         QStringLiteral("Tiếng Việt"),
    QStringLiteral("Türkçe"),          QStringLiteral("Ελληνικά"),
    QStringLiteral("Беларуская"),      QStringLiteral("Беларуская"),
    QStringLiteral("Қазақ Тілі"),      QStringLiteral("Македонски"),
    QStringLiteral("Монгол"),          QStringLiteral("Русский"),
    QStringLiteral("Српски"),          QStringLiteral("Українська"),
    QStringLiteral("Ўзбек"),           QStringLiteral("ქართული"),
    QStringLiteral("Հայերեն"),        QStringLiteral("עברית"),
    QStringLiteral("اردو"),            QStringLiteral("العربية"),
    QStringLiteral("پښتو"),            QStringLiteral("فارسی"),
    QStringLiteral("മലയാളം"),          QStringLiteral("한국어"),
    QStringLiteral("中文 (香港)"),      QStringLiteral("中文（简体中文）"),
    QStringLiteral("中文（繁體中文）"), QStringLiteral("日本語"),
};

// Nombres de "tlangs" (default/sets.cfg líneas 84-92).
const QStringList kTlangs = {
    QStringLiteral("English"), QStringLiteral("Spanish"),
    QStringLiteral("Italian"), QStringLiteral("Portuguese"),
    QStringLiteral("German"),  QStringLiteral("Japanese"),
    QStringLiteral("French"),  QStringLiteral("Chinese"),
    QStringLiteral("Russian"),
};

// Categories (default/sets.cfg líneas 108-112), lowercased (comparación ctgy).
const QStringList kCategoriesLower = {
    QStringLiteral("article"),       QStringLiteral("in the city"),
    QStringLiteral("culture"),       QStringLiteral("beginners"),
    QStringLiteral("entertainment"), QStringLiteral("funny"),
    QStringLiteral("grammar"),       QStringLiteral("history"),
    QStringLiteral("at home"),       QStringLiteral("how-tos"),
    QStringLiteral("internet"),      QStringLiteral("interview"),
    QStringLiteral("movies"),        QStringLiteral("music"),
    QStringLiteral("nature"),        QStringLiteral("news"),
    QStringLiteral("storyteller"),   QStringLiteral("others"),
    QStringLiteral("places"),        QStringLiteral("quotes"),
    QStringLiteral("talks"),         QStringLiteral("science"),
    QStringLiteral("media"),         QStringLiteral("sport"),
    QStringLiteral("tech"),          QStringLiteral("vocabulary"),
};

bool allWhitespace(const QString &s)
{
    for (const QChar c : s) {
        if (!c.isSpace())
            return false;
    }
    return true;
}

bool isNumeric(const QString &s)
{
    return QRegularExpression(kNumerRe).match(s).hasMatch();
}

bool isDate(const QString &s)
{
    return QRegularExpression(kDateRe).match(s).hasMatch();
}

// Verifica si `s` contiene alguno de los caracteres en `forbidden`.
// Equivalente a la intención de `grep -o -E '<patron>' <<< "${val}"` del Bash
// (el Bash original tiene un bug: la regex siempre coincide; aquí reproducimos
// la intención, no el bug).
bool containsForbidden(const QString &s, const QString &forbidden)
{
    for (const QChar c : s) {
        if (forbidden.contains(c))
            return true;
    }
    return false;
}

// Línea plana de 25 segmentos (web/import): `trgt{...}` + el orden canónico
// de ItemDefs::jsonFieldName (srce, slXX, ..., type). Sin escaping, igual que
// el añadir del original. Es lo que escribe main.sh:326-329 tras el transform.
QString importLine(const Item &item)
{
    QString out = QStringLiteral("trgt{") + item.trgt + QLatin1Char('}');
    for (int i = 0; i < ItemDefs::jsonFieldCount(); ++i) {
        const QString name = QLatin1String(ItemDefs::jsonFieldName(i));
        QString value;
        if (name == QLatin1String("srce")) {
            value = item.srce;
        } else if (name == QLatin1String("exmp")) {
            value = item.exmp;
        } else if (name == QLatin1String("defn")) {
            value = item.defn;
        } else if (name == QLatin1String("note")) {
            value = item.note;
        } else if (name == QLatin1String("wrds")) {
            value = item.wrds;
        } else if (name == QLatin1String("grmr")) {
            value = item.grmr;
        } else if (name == QLatin1String("tags")) {
            value = item.tags;
        } else if (name == QLatin1String("mark")) {
            value = item.mark;
        } else if (name == QLatin1String("refr")) {
            value = item.refr;
        } else if (name == QLatin1String("imag")) {
            value = item.imag;
        } else if (name == QLatin1String("imgr")) {
            value = item.imgr;
        } else if (name == QLatin1String("link")) {
            value = item.link;
        } else if (name == QLatin1String("cdid")) {
            value = item.cdid;
        } else if (name == QLatin1String("type")) {
            value = item.type;
        } else if (name.startsWith(QLatin1String("sl"))) {
            value = item.translations.value(name.mid(2));
        }
        out += name + QLatin1Char('{') + value + QLatin1Char('}');
    }
    return out;
}

// Escapa una comilla simple como tpc_db del original (`s|'|''|g`).
QString sqlQuote(const QString &v)
{
    return QString(v).replace(QLatin1Char('\''), QStringLiteral("''"));
}

// Número de topics NO ocultos del idioma (tope de new_topic / main.sh:287).
int topicCount(const core::AppPaths &paths, const QString &tlng)
{
    QDir dir(paths.topicsDirForLanguage(tlng));
    if (!dir.exists())
        return 0;
    int n = 0;
    for (const QString &name :
         dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden,
                       QDir::NoSort)) {
        if (!name.startsWith(QLatin1Char('.')))
            ++n;
    }
    return n;
}

// `check_dir` del import (main.sh:302-304): directorios del topic y share.
void ensureImportDirs(const core::AppPaths &paths, const QString &tlng,
                      const QString &name)
{
    const QString dataDir = paths.topicDataDir(tlng, name);
    const QString conf = paths.topicConfDir(tlng, name);
    const QString shared = paths.sharedDirForLanguage(tlng);
    QDir().mkpath(dataDir + QStringLiteral("/images"));
    QDir().mkpath(conf + QStringLiteral("/practice"));
    QDir().mkpath(shared + QStringLiteral("/images"));
    QDir().mkpath(shared + QStringLiteral("/audio"));
    QDir().mkpath(shared + QStringLiteral("/data"));
}

// `check_file` (main.sh:324-325): crea el fichero VACÍO si no existe.
void ensureEmptyFile(const QString &path)
{
    if (QFileInfo::exists(path))
        return;
    QFile f(path);
    if (f.open(QIODevice::WriteOnly))
        f.close();
}

} // namespace

LegacyImporter::LegacyImporter(core::AppPaths appPaths)
    : m_appPaths(std::move(appPaths))
{
}

void LegacyImporter::setProfileLanguages(const QString &slng, const QString &tlng)
{
    m_profileSlng = slng;
    m_profileTlng = tlng;
}

IdmndDocument LegacyImporter::validate(const QByteArray &text, bool *ok,
                                       QString *error)
{
    bool parseOk = false;
    QString why;
    IdmndDocument doc = IdmndCodec::parse(text, &parseOk, &why);
    if (!parseOk) {
        if (ok) *ok = false;
        if (error) *error = why.isEmpty() ? QStringLiteral("Format") : why;
        return {};
    }

    const auto fail = [&](const QString &reason) {
        if (ok) *ok = false;
        if (error) *error = reason;
        return IdmndDocument{};
    };
    if (ok) *ok = false;

    // --- check_format_1: validación de VALORES (reglas efectivas) ----------
    // n=0  name: no todo-espacios, <=60, sin chars prohibidos `*/@=`.
    const QString &name = doc.info.name();
    if (allWhitespace(name) || name.size() > 60)
        return fail(QStringLiteral("name"));
    if (containsForbidden(name, QStringLiteral("*/@=")))
        return fail(QStringLiteral("name"));
    // n=1  slng: puede llevar traducciones extra tras coma ("Español, English").
    // Se valida la parte primaria contra kSlangs; las extras se ignoran.
    QString slngPrimary = doc.info.slng();
    if (slngPrimary.contains(QLatin1Char(','))) {
        slngPrimary = slngPrimary.section(QLatin1Char(','), 0, 0)
                          .trimmed();
    }
    if (!kSlangs.contains(slngPrimary))
        return fail(QStringLiteral("slng"));
    // n=2  tlng: nombre exacto de "tlangs".
    if (!kTlangs.contains(doc.info.tlng()))
        return fail(QStringLiteral("tlng"));
    // n=3/4  autr/cntt: <=30, sin chars prohibidos `*/()=`
    // (Bash: regex `\*|\/|$|\)|\(|=`).
    if (doc.info.autr().size() > 30 || doc.info.cntt().size() > 30)
        return fail(QStringLiteral("autr/cntt"));
    if (containsForbidden(doc.info.autr(), QStringLiteral("*/()=")) ||
        containsForbidden(doc.info.cntt(), QStringLiteral("*/()=")))
        return fail(QStringLiteral("autr/cntt"));
    // n=5  ctgy: con '_'->' ', minúsculas, en Categories.
    QString ctgy = doc.info.ctgy();
    ctgy.replace(QLatin1Char('_'), QLatin1Char(' '));
    if (!kCategoriesLower.contains(ctgy.toLower()))
        return fail(QStringLiteral("ctgy"));
    // n=6  ilnk: no todo-espacios, <=36.
    const QString &ilnk = doc.info.ilnk();
    if (allWhitespace(ilnk) || ilnk.size() > 36)
        return fail(QStringLiteral("ilnk"));
    // n=7  orig: <=60, sin chars prohibidos `*/@=`.
    if (doc.info.orig().size() > 60)
        return fail(QStringLiteral("orig"));
    if (containsForbidden(doc.info.orig(), QStringLiteral("*/@=")))
        return fail(QStringLiteral("orig"));
    // n=8/9/10  dtec/dteu/dtei: vacías OK; si no, formato AAAA-MM-DD y <=12.
    for (const QString *d : {&doc.info.dtec(), &doc.info.dteu(), &doc.info.dtei()}) {
        if (!d->isEmpty() && (!isDate(*d) || d->size() > 12))
            return fail(QStringLiteral("dtec/dteu/dtei"));
    }
    // n=11/12/13  nwrd/nsnt/nimg: numéricos y <=200.
    for (const QString *v : {&doc.info.nwrd(), &doc.info.nsnt(), &doc.info.nimg()}) {
        if (!isNumeric(*v) || v->toInt() > 200)
            return fail(QStringLiteral("nwrd/nsnt/nimg"));
    }
    // n=14  naud: numérico y <=1000.
    const QString &naud = doc.info.naud();
    if (!isNumeric(naud) || naud.toInt() > 1000)
        return fail(QStringLiteral("naud"));
    // n=15  nsze: <=6 caracteres.
    if (doc.info.nsze().size() > 6)
        return fail(QStringLiteral("nsze"));
    // n=16  levl: numérico y <=2 dígitos.
    const QString &levl = doc.info.levl();
    if (!isNumeric(levl) || levl.size() > 2)
        return fail(QStringLiteral("levl"));
    // n=17  info: <=10240 caracteres.
    if (doc.info.info().size() > 10240)
        return fail(QStringLiteral("info"));
    // n=18  stts: no todo-espacios, <=40, sin chars prohibidos `*/@=-`.
    const QString &stts = doc.info.stts();
    if (allWhitespace(stts) || stts.size() > 40)
        return fail(QStringLiteral("stts"));
    if (containsForbidden(stts, QStringLiteral("*/@=-")))
        return fail(QStringLiteral("stts"));

    if (ok) *ok = true;
    return doc;
}

bool LegacyImporter::importText(const QString &tlng, const QByteArray &idmnd,
                                QString *topicName, QString *error) const
{
    bool ok = false;
    QString why;
    const IdmndDocument doc = validate(idmnd, &ok, &why);
    if (!ok) {
        if (error) *error = why.isEmpty() ? QStringLiteral("Format") : why;
        return false;
    }
    return install(doc, tlng, topicName, error);
}

bool LegacyImporter::importFile(const QString &tlng, const QString &path,
                                QString *topicName, QString *error) const
{
    QString localPath = path;
    const QUrl url(path);
    if (url.isLocalFile())
        localPath = url.toLocalFile();
    QFile f(localPath);
    if (!f.open(QIODevice::ReadOnly)) {
        if (error) *error = QStringLiteral("Format");
        return false;
    }
    const QByteArray data = f.readAll();
    f.close();

    // ZIP: magic bytes PK\x03\x04 (Bash export creates a ZIP with .idmnd extension)
    if (data.size() >= 4 && data[0] == 'P' && data[1] == 'K'
        && data[2] == '\x03' && data[3] == '\x04') {
        return installZip(data, tlng, topicName, error);
    }

    // Plain text .idmnd (legacy 3-line format)
    return importText(tlng, data, topicName, error);
}

bool LegacyImporter::install(const IdmndDocument &doc, const QString &tlng,
                             QString *topicName, QString *error) const
{
    // main.sh:287: tope de 120 topics por idioma (antes de dedup/crear).
    if (topicCount(m_appPaths, tlng) >= 120) {
        if (error) *error = QStringLiteral("Maximum number of topics reached.");
        return false;
    }

    // dedup "name (i)" 1..50 (main.sh:293-300; caída de 50 -> " (51)").
    const QString base = doc.info.name().trimmed();
    QString finalName = base;
    const auto exists = [&](const QString &n) {
        return QFileInfo(m_appPaths.topicDataDir(tlng, n)).isDir();
    };
    if (exists(base)) {
        int i = 1;
        for (; i <= 50; ++i) {
            if (!exists(QStringLiteral("%1 (%2)").arg(base).arg(i)))
                break;
        }
        finalName = QStringLiteral("%1 (%2)").arg(base).arg(i);
    }

    const QString dataDir = m_appPaths.topicDataDir(tlng, finalName);
    const QString conf = m_appPaths.topicConfDir(tlng, finalName);
    ensureImportDirs(m_appPaths, tlng, finalName);

    // mkdb tpc (main.sh:308): esquema DB del topic con el nombre deduplicado.
    // (El Bash creaba la fila id con el slng del PERFIL y luego la
    // sobrescribía con el del fichero; el final es idéntico.)
    TopicRepository topics(m_appPaths);
    if (!topics.recreateTopicDatabase(tlng, finalName)) {
        if (error) *error = QStringLiteral("db");
        return false;
    }

    // `tpc_db 9 id <col> '<valor>'` (main.sh:309-323). cntt/dteu/info/stts NO
    // se escriben: quedan '' (como deja mkdb; el export lo replica).
    SqliteDatabase db(m_appPaths.topicDb(tlng, finalName));
    if (!db.open()) {
        if (error) *error = QStringLiteral("db");
        return false;
    }
    const auto setId = [&](const QString &col, const QString &value) {
        db.execute(QStringLiteral("update id set %1='%2';").arg(col, sqlQuote(value)));
    };
    setId(QStringLiteral("name"), finalName);
    setId(QStringLiteral("slng"), doc.info.slng());
    setId(QStringLiteral("tlng"), doc.info.tlng());
    setId(QStringLiteral("autr"), doc.info.autr());
    setId(QStringLiteral("ctgy"), doc.info.ctgy());
    setId(QStringLiteral("ilnk"), doc.info.ilnk());
    setId(QStringLiteral("orig"), doc.info.orig());
    setId(QStringLiteral("dtec"), doc.info.dtec());
    const QString today = QDate::currentDate().toString(QStringLiteral("yyyy-MM-dd"));
    setId(QStringLiteral("dtei"), today);
    setId(QStringLiteral("nwrd"), doc.info.nwrd());
    setId(QStringLiteral("nsnt"), doc.info.nsnt());
    setId(QStringLiteral("nimg"), doc.info.nimg());
    setId(QStringLiteral("naud"), doc.info.naud());
    setId(QStringLiteral("nsze"), doc.info.nsze());
    setId(QStringLiteral("levl"), doc.info.levl());

    // check_file: practice/log1..3, note y download VACÍOS (main.sh:324-325).
    for (int i = 1; i <= 3; ++i)
        ensureEmptyFile(conf + QStringLiteral("/practice/log%1").arg(i));
    ensureEmptyFile(conf + QStringLiteral("/note"));
    ensureEmptyFile(conf + QStringLiteral("/download"));

    // data: una línea plana de 25 segmentos por ítem (main.sh:326-329).
    TopicDataRepository dataRepo(m_appPaths);
    QList<FlatItemCodec::DecodedItem> decoded;
    decoded.reserve(doc.items.size());
    for (Item item : doc.items) {
        // Recompute imgr: words get first-char-lowered trgt, sentences empty.
        if (item.type == QLatin1String("1") && !item.trgt.isEmpty())
            item.imgr = item.trgt.left(1).toLower() + item.trgt.mid(1);
        else
            item.imgr.clear();
        decoded.append(FlatItemCodec::decodeWithSegments(importLine(item)));
    }
    if (!dataRepo.writeItems(tlng, finalName, decoded)) {
        if (error) *error = QStringLiteral("data");
        return false;
    }

    // tablas (main.sh:362-369): learning siempre, words/sentences según type,
    // marks si mark==TRUE, y Data con 14 columnas (el Python no inserta ni
    // slXX ni imgr).
    for (const Item &item : doc.items) {
        const QString t = sqlQuote(item.trgt);
        if (item.type == QLatin1String("1"))
            db.execute(QStringLiteral("insert into words (list) values ('%1');").arg(t));
        else if (item.type == QLatin1String("2"))
            db.execute(QStringLiteral("insert into sentences (list) values ('%1');").arg(t));
        if (item.mark == QLatin1String("TRUE"))
            db.execute(QStringLiteral("insert into marks (list) values ('%1');").arg(t));
        db.execute(QStringLiteral("insert into learning (list) values ('%1');").arg(t));
        const QStringList vals = {
            sqlQuote(item.trgt), sqlQuote(item.srce), sqlQuote(item.exmp),
            sqlQuote(item.defn), sqlQuote(item.note), sqlQuote(item.wrds),
            sqlQuote(item.grmr), sqlQuote(item.tags), sqlQuote(item.mark),
            sqlQuote(item.refr), sqlQuote(item.imag), sqlQuote(item.link),
            sqlQuote(item.cdid), sqlQuote(item.type)};
        db.execute(QStringLiteral(
            "insert into Data (trgt,srce,exmp,defn,note,wrds,grmr,tags,mark,"
            "refr,imag,link,cdid,type) values ('%1','%2','%3','%4','%5','%6','%7',"
            "'%8','%9','%10','%11','%12','%13','%14');")
                       .arg(vals.at(0), vals.at(1), vals.at(2), vals.at(3),
                            vals.at(4), vals.at(5), vals.at(6), vals.at(7),
                            vals.at(8), vals.at(9), vals.at(10), vals.at(11),
                            vals.at(12), vals.at(13)));
    }
    db.close();

    // colorize 1 (main.sh:375) y `echo 1 > stts` (main.sh:423).
    if (!dataRepo.rebuildIndex(tlng, finalName, true)) {
        if (error) *error = QStringLiteral("index");
        return false;
    }
    dataRepo.writeStts(tlng, finalName, QStringLiteral("1"));

    // slng/tlng mismatch (main.sh:407-416): si el slng del topic difiere del
    // slng del perfil, crea translations/active + slng_err.
    // El reinicio de tray y la restauración de config DB son runtime (pendiente).
    m_lastMismatch = MismatchResult{};
    if (!m_profileSlng.isEmpty()) {
        const QString topicSlng = doc.info.slng();
        if (topicSlng != m_profileSlng) {
            m_lastMismatch.slngMismatch = true;
            const QString transDir = conf + QStringLiteral("/translations");
            QDir().mkpath(transDir);
            QFile activeFile(transDir + QStringLiteral("/active"));
            if (activeFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                activeFile.write(topicSlng.toUtf8());
                activeFile.write("\n");
                activeFile.close();
            }
            // slng_err: fichero vacío que indica mismatch (main.sh:415).
            QFile slngErrFile(conf + QStringLiteral("/slng_err"));
            if (!slngErrFile.exists())
                slngErrFile.open(QIODevice::WriteOnly | QIODevice::Truncate);
        }
        if (doc.info.tlng() != m_profileTlng) {
            m_lastMismatch.tlngMismatch = true;
        }
    }

    if (topicName) *topicName = finalName;
    return true;
}

bool LegacyImporter::installZip(const QByteArray &zipData, const QString &tlng,
                                QString *topicName, QString *error) const
{
    // Extract ZIP using a minimal zlib-based reader.
    // The format is the same ZIP created by the Bash export (zip -qr).
    const QList<ZipEntry> entries = readZipEntries(zipData);
    if (entries.isEmpty()) {
        if (error) *error = QStringLiteral("Format");
        return false;
    }

    // Write extracted files to a temp directory.
    QTemporaryDir tmpDir;
    if (!tmpDir.isValid()) {
        if (error) *error = QStringLiteral("temp");
        return false;
    }
    const QString tmpPath = tmpDir.path();
    for (const ZipEntry &entry : entries) {
        const QString dest = tmpPath + QLatin1Char('/') + entry.name;
        QDir().mkpath(QFileInfo(dest).absolutePath());
        QFile f(dest);
        if (f.open(QIODevice::WriteOnly)) {
            f.write(entry.data);
            f.close();
        }
    }

    // Find topic.idmnd in the extracted tree.
    const QString idmndPath = tmpPath + QStringLiteral("/topic.idmnd");
    if (!QFileInfo::exists(idmndPath)) {
        QDir root(tmpPath);
        const QStringList found = root.entryList({QStringLiteral("*.idmnd")}, QDir::Files);
        if (found.isEmpty()) {
            if (error) *error = QStringLiteral("Format");
            return false;
        }
        QFile file(root.absoluteFilePath(found.first()));
        if (!file.open(QIODevice::ReadOnly)
            || !importText(tlng, file.readAll(), topicName, error))
            return false;
    } else {
        QFile file(idmndPath);
        if (!file.open(QIODevice::ReadOnly)
            || !importText(tlng, file.readAll(), topicName, error))
            return false;
    }

    if (!topicName || topicName->isEmpty())
        return true;

    const QString dataDir = m_appPaths.topicDataDir(tlng, *topicName);
    const QString sharedDir = m_appPaths.sharedDirForLanguage(tlng);

    // Restore images
    const QString imagesDir = tmpPath + QStringLiteral("/images");
    if (QDir(imagesDir).exists()) {
        for (const QString &img : QDir(imagesDir).entryList(
                 {QStringLiteral("*.jpg"), QStringLiteral("*.png")}, QDir::Files)) {
            if (img.contains(QStringLiteral("-1."))) {
                const QString dest = sharedDir + QStringLiteral("/images/") + img;
                QDir().mkpath(sharedDir + QStringLiteral("/images"));
                QFile::copy(imagesDir + QLatin1Char('/') + img, dest);
            } else if (img.contains(QStringLiteral("-2."))) {
                const QString base = img.left(img.size() - 4).replace(QStringLiteral("-2"), QString());
                const QString dest = dataDir + QStringLiteral("/images/") + base + QStringLiteral(".jpg");
                QDir().mkpath(dataDir + QStringLiteral("/images"));
                QFile::copy(imagesDir + QLatin1Char('/') + img, dest);
            }
        }
    }

    // Restore topic audio
    const QString topicAudio = tmpPath + QStringLiteral("/audio/topic");
    if (QDir(topicAudio).exists())
        for (const QString &mp3 : QDir(topicAudio).entryList({QStringLiteral("*.mp3")}, QDir::Files))
            QFile::copy(topicAudio + QLatin1Char('/') + mp3, dataDir + QLatin1Char('/') + mp3);

    // Restore shared audio
    const QString sharedAudio = tmpPath + QStringLiteral("/audio/shared");
    if (QDir(sharedAudio).exists()) {
        QDir().mkpath(sharedDir + QStringLiteral("/audio"));
        for (const QString &mp3 : QDir(sharedAudio).entryList({QStringLiteral("*.mp3")}, QDir::Files))
            QFile::copy(sharedAudio + QLatin1Char('/') + mp3,
                        sharedDir + QStringLiteral("/audio/") + mp3);
    }

    return true;
}

} // namespace storage
} // namespace idiomind
