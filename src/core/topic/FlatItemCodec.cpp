#include "core/topic/FlatItemCodec.h"

namespace idiomind {
namespace core {
namespace FlatItemCodec {

QString encode(const Item &item)
{
    QString out;
    out += QStringLiteral("trgt{") + item.trgt + QChar::fromLatin1('}');
    out += QStringLiteral("srce{") + item.srce + QChar::fromLatin1('}');
    out += QStringLiteral("exmp{") + item.exmp + QChar::fromLatin1('}');
    out += QStringLiteral("defn{") + item.defn + QChar::fromLatin1('}');
    out += QStringLiteral("note{") + item.note + QChar::fromLatin1('}');
    out += QStringLiteral("wrds{") + item.wrds + QChar::fromLatin1('}');
    out += QStringLiteral("grmr{") + item.grmr + QChar::fromLatin1('}');
    out += QStringLiteral("tags{") + item.tags + QChar::fromLatin1('}');
    out += QStringLiteral("mark{") + item.mark + QChar::fromLatin1('}');
    out += QStringLiteral("refr{") + item.refr + QChar::fromLatin1('}');
    out += QStringLiteral("imag{") + item.imag + QChar::fromLatin1('}');
    out += QStringLiteral("link{") + item.link + QChar::fromLatin1('}');
    out += QStringLiteral("cdid{") + item.cdid + QChar::fromLatin1('}');
    out += QStringLiteral("type{") + item.type + QChar::fromLatin1('}');
    return out;
}

namespace {

// Equivalente a `grep -oP '(?<=<name>{).*(?=})'` sobre la línea partida por
// '}' (get_item): texto entre `name{` y el PRIMER '}' que le siga.
QString field(const QString &line, const QString &name)
{
    const QString prefix = name + QChar::fromLatin1('{');
    const int start = line.indexOf(prefix);
    if (start < 0)
        return {};
    const int valueStart = start + prefix.size();
    const int end = line.indexOf(QChar::fromLatin1('}'), valueStart);
    if (end < 0)
        return {};
    return line.mid(valueStart, end - valueStart);
}

}  // namespace

Item decode(const QString &line)
{
    Item item;
    item.trgt = field(line, QStringLiteral("trgt"));
    item.srce = field(line, QStringLiteral("srce"));
    item.exmp = field(line, QStringLiteral("exmp"));
    item.defn = field(line, QStringLiteral("defn"));
    item.note = field(line, QStringLiteral("note"));
    item.wrds = field(line, QStringLiteral("wrds"));
    item.grmr = field(line, QStringLiteral("grmr"));
    item.tags = field(line, QStringLiteral("tags"));
    item.mark = field(line, QStringLiteral("mark"));
    item.refr = field(line, QStringLiteral("refr"));
    item.imag = field(line, QStringLiteral("imag"));
    item.link = field(line, QStringLiteral("link"));
    item.cdid = field(line, QStringLiteral("cdid"));
    item.type = field(line, QStringLiteral("type"));
    return item;
}

DecodedItem decodeWithSegments(const QString &line)
{
    DecodedItem out;
    out.item = decode(line);
    int i = 0;
    while (i < line.size()) {
        if (line.at(i) == QLatin1Char('}')) {  // separador entre segmentos
            ++i;
            continue;
        }
        const int open = line.indexOf(QLatin1Char('{'), i);
        if (open < 0)
            break;
        const QString name = line.mid(i, open - i);
        const int close = line.indexOf(QLatin1Char('}'), open + 1);
        if (close < 0) {
            out.segments.append(Segment(name, line.mid(open + 1)));
            break;
        }
        const QString value = line.mid(open + 1, close - open - 1);
        out.segments.append(Segment(name, value));
        if (name == QLatin1String("imgr"))
            out.item.imgr = value;
        i = close + 1;
    }
    return out;
}

namespace {

// Valor canónico (0..13: trgt,srce,exmp,defn,note,wrds,grmr,tags,mark,refr,
// imag,link,cdid,type). -1 si el nombre no es un campo canónico.
int canonicalIndex(const QString &name)
{
    for (int i = 0; i < ItemDefs::flatFieldCount(); ++i) {
        if (name == QLatin1String(ItemDefs::flatFieldName(i)))
            return i;
    }
    return -1;
}

}  // namespace

QString reencodeItem(const DecodedItem &orig, const Item &current)
{
    const auto &segs = orig.segments;
    if (segs.isEmpty())
        return encode(current);

    QString out;
    QStringList appended;
    for (const Segment &seg : segs) {
        const int idx = canonicalIndex(seg.first);
        QString value = seg.second;  // desconocido (slXX/imgr...) -> byte a byte
        if (idx >= 0) {
            switch (idx) {
            case 0:  value = current.trgt; break;
            case 1:  value = current.srce; break;
            case 2:  value = current.exmp; break;
            case 3:  value = current.defn; break;
            case 4:  value = current.note; break;
            case 5:  value = current.wrds; break;
            case 6:  value = current.grmr; break;
            case 7:  value = current.tags; break;
            case 8:  value = current.mark; break;
            case 9:  value = current.refr; break;
            case 10: value = current.imag; break;
            case 11: value = current.link; break;
            case 12: value = current.cdid; break;
            case 13: value = current.type; break;
            }
            appended.append(seg.first);
        }
        out += seg.first + QLatin1Char('{') + value + QLatin1Char('}');
    }
    // Campos canónicos que la línea original no tenía -> se añaden al final en
    // el orden canónico (el original los escribía vacíos en el template).
    for (int i = 0; i < ItemDefs::flatFieldCount(); ++i) {
        if (!appended.contains(QLatin1String(ItemDefs::flatFieldName(i)))) {
            const QString v = [&]() {
                switch (i) {
                case 0:  return current.trgt;
                case 1:  return current.srce;
                case 2:  return current.exmp;
                case 3:  return current.defn;
                case 4:  return current.note;
                case 5:  return current.wrds;
                case 6:  return current.grmr;
                case 7:  return current.tags;
                case 8:  return current.mark;
                case 9:  return current.refr;
                case 10: return current.imag;
                case 11: return current.link;
                case 12: return current.cdid;
                default: return current.type;
                }
            }();
            out += QLatin1String(ItemDefs::flatFieldName(i)) + QLatin1Char('{')
                + v + QLatin1Char('}');
        }
    }
    return out;
}

}  // namespace FlatItemCodec
} // namespace core
} // namespace idiomind