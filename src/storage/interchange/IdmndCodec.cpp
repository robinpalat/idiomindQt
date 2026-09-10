#include "storage/interchange/IdmndCodec.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStringList>

namespace idiomind {
namespace storage {

using core::Item;
using core::TopicInfo;

namespace {
const char kLine1[] = "{\"items\":{";
} // namespace

void IdmndCodec::skipWs(const QString &s, int &i)
{
    while (i < s.size() && s.at(i).isSpace())
        ++i;
}

bool IdmndCodec::readJsonString(const QString &s, int &i, QString &out)
{
    if (i >= s.size() || s.at(i) != QLatin1Char('"'))
        return false;
    ++i;
    out.clear();
    while (i < s.size()) {
        const QChar c = s.at(i);
        if (c == QLatin1Char('"')) {
            ++i;
            return true;
        }
        if (c == QLatin1Char('\\')) {
            if (i + 1 >= s.size())
                return false;
            const QChar e = s.at(i + 1);
            switch (e.unicode()) {
            case u'"':  out += QLatin1Char('"');  i += 2; break;
            case u'\\': out += QLatin1Char('\\'); i += 2; break;
            case u'/':  out += QLatin1Char('/');  i += 2; break;
            case u'b':  out += QLatin1Char('\b'); i += 2; break;
            case u'f':  out += QLatin1Char('\f'); i += 2; break;
            case u'n':  out += QLatin1Char('\n'); i += 2; break;
            case u'r':  out += QLatin1Char('\r'); i += 2; break;
            case u't':  out += QLatin1Char('\t'); i += 2; break;
            case u'u': {
                bool hexOk = false;
                const int code = s.mid(i + 2, 4).toInt(&hexOk, 16);
                if (!hexOk)
                    return false;
                out += QChar(code);
                i += 6;
                break;
            }
            default:
                return false;
            }
        } else {
            out += c;
            ++i;
        }
    }
    return false;
}

bool IdmndCodec::parseItemsLine(const QString &line, QList<Item> &out,
                                QString *error)
{
    int i = 0;
    skipWs(line, i);

    // Objeto items vacío: líneas `}` u `},`.
    if (i < line.size() && line.at(i) == QLatin1Char('}')) {
        ++i;
        skipWs(line, i);
        if (i < line.size() && line.at(i) == QLatin1Char(','))
            ++i;
        skipWs(line, i);
        if (i < line.size()) {
            if (error) *error = QStringLiteral("Format");
            return false;
        }
        return true;
    }

    for (;;) {
        skipWs(line, i);
        QString key;
        if (!readJsonString(line, i, key)) {
            if (error) *error = QStringLiteral("Format");
            return false;
        }
        skipWs(line, i);
        if (i >= line.size() || line.at(i) != QLatin1Char(':')) {
            if (error) *error = QStringLiteral("Format");
            return false;
        }
        ++i;
        skipWs(line, i);
        if (i >= line.size() || line.at(i) != QLatin1Char('{')) {
            if (error) *error = QStringLiteral("Format");
            return false;
        }

        // Cuerpo del objeto ítem hasta el cierre equilibrado (respeta los
        // `\"` y `\\` de los strings).
        const int bodyStart = i;
        int depth = 0;
        bool inStr = false;
        bool escaped = false;
        for (; i < line.size(); ++i) {
            const QChar c = line.at(i);
            if (inStr) {
                if (escaped) {
                    escaped = false;
                } else if (c == QLatin1Char('\\')) {
                    escaped = true;
                } else if (c == QLatin1Char('"')) {
                    inStr = false;
                }
                continue;
            }
            switch (c.unicode()) {
            case u'"': inStr = true; break;
            case u'{': ++depth; break;
            case u'}':
                --depth;
                if (depth == 0) {
                    ++i; // tras la llave de cierre
                    goto itemDone;
                }
                break;
            default: break;
            }
        }
    itemDone:
        if (depth != 0) {
            if (error) *error = QStringLiteral("Format");
            return false;
        }

        const QByteArray body = line.mid(bodyStart, i - bodyStart).toUtf8();
        const QJsonDocument doc = QJsonDocument::fromJson(body);
        if (!doc.isObject()) {
            if (error) *error = QStringLiteral("Format");
            return false;
        }
        out.append(Item::fromJson(QJsonValue(doc.object()), key));

        skipWs(line, i);
        if (i >= line.size()) {
            if (error) *error = QStringLiteral("Format");
            return false;
        }
        const QChar c = line.at(i);
        if (c == QLatin1Char(',')) {
            ++i;
            continue;
        }
        if (c == QLatin1Char('}')) {
            ++i;
            skipWs(line, i);
            if (i < line.size() && line.at(i) == QLatin1Char(','))
                ++i;
            skipWs(line, i);
            if (i < line.size()) {
                if (error) *error = QStringLiteral("Format");
                return false;
            }
            return true;
        }
        if (error) *error = QStringLiteral("Format");
        return false;
    }
}

IdmndDocument IdmndCodec::parse(const QByteArray &text, bool *ok, QString *error)
{
    IdmndDocument doc;
    const auto fail = [&](const QString &reason) {
        if (ok) *ok = false;
        if (error) *error = reason;
        return doc;
    };
    if (ok) *ok = false;

    // check_format_1: `wc -l == 3` (3 saltos de línea) y JSON global válido.
    const int newlines = text.count('\n');
    if (newlines != 3)
        return fail(QStringLiteral("%1 Lines!").arg(newlines));
    const QJsonDocument global = QJsonDocument::fromJson(text);
    if (!global.isObject())
        return fail(QStringLiteral("Format"));

    const QStringList lines = QString::fromUtf8(text).split(QLatin1Char('\n'));
    // lines[0..2] contienen las 3 líneas; lines[3] es vacía (cola del '\n').
    if (lines[0].trimmed() != QLatin1String(kLine1))
        return fail(QStringLiteral("Format"));

    if (!parseItemsLine(lines[1], doc.items, error)) {
        if (error && error->isEmpty())
            *error = QStringLiteral("Format");
        return doc;
    }

    doc.info = TopicInfo::fromJsonText(lines[2]);
    if (doc.info.name().isEmpty())
        return fail(QStringLiteral("Format"));

    if (ok) *ok = true;
    return doc;
}

IdmndDocument IdmndCodec::parseFile(const QString &path, bool *ok, QString *error)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        if (ok) *ok = false;
        if (error) *error = QStringLiteral("Format");
        return {};
    }
    return parse(f.readAll(), ok, error);
}

namespace {

// Escaping JSON estándar de un valor string (idéntico al del resto del
// proyecto: Item::toJsonText / TopicInfo::toJsonText).
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

} // namespace

QByteArray IdmndCodec::write(const IdmndDocument &doc)
{
    QString out;
    out += QLatin1String("{\"items\":{\n");
    QStringList bodies;
    bodies.reserve(doc.items.size());
    for (const Item &item : doc.items)
        bodies.append(QStringLiteral("\"%1\":%2").arg(jsonEscape(item.trgt),
                                                      item.toJsonText()));
    out += bodies.join(QLatin1Char(','));
    out += QLatin1String("},\n");
    out += doc.info.toJsonText();
    out += QLatin1String("}\n");
    return out.toUtf8();
}

} // namespace storage
} // namespace idiomind
