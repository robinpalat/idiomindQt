#include "Item.h"

#include <QJsonObject>

namespace idiomind {
namespace core {

namespace {
const QString trueMark = QStringLiteral("TRUE");

// Escaping JSON estándar para un valor string (igual que QJsonDocument).
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
}

QString Item::toJsonText() const
{
    QString out = QStringLiteral("{");
    const auto member = [&out](const QString &k, const QString &v) {
        out += QChar::fromLatin1('"') + k +
               QStringLiteral("\":\"") + jsonEscape(v) +
               QStringLiteral("\",");
    };
    member(QStringLiteral("srce"), srce);
    for (const char *s : ItemDefs::translationSuffixes)
        member(QStringLiteral("sl%1").arg(QLatin1String(s)),
               translations.value(QLatin1String(s)));
    member(QStringLiteral("exmp"), exmp);
    member(QStringLiteral("defn"), defn);
    member(QStringLiteral("note"), note);
    member(QStringLiteral("wrds"), wrds);
    member(QStringLiteral("grmr"), grmr);
    member(QStringLiteral("tags"), tags);
    member(QStringLiteral("mark"), mark);
    member(QStringLiteral("refr"), refr);
    member(QStringLiteral("imag"), imag);
    member(QStringLiteral("imgr"), imgr);
    member(QStringLiteral("link"), link);
    member(QStringLiteral("cdid"), cdid);
    member(QStringLiteral("type"), type);
    out.chop(1);  // última coma
    out += QChar::fromLatin1('}');
    return out;
}

bool Item::isWord() const { return type == QStringLiteral("1"); }
bool Item::isSentence() const { return !isWord(); }
bool Item::isMarked() const { return mark == trueMark; }

QJsonValue Item::toJson() const
{
    // Orden exacto del formato .idmnd (default/vars línea 1 + slvi).
    QJsonObject o;
    o.insert(QStringLiteral("srce"), srce);
    for (const char *s : ItemDefs::translationSuffixes)
        o.insert(QStringLiteral("sl%1").arg(QLatin1String(s)),
                 translations.value(QLatin1String(s)));
    o.insert(QStringLiteral("exmp"), exmp);
    o.insert(QStringLiteral("defn"), defn);
    o.insert(QStringLiteral("note"), note);
    o.insert(QStringLiteral("wrds"), wrds);
    o.insert(QStringLiteral("grmr"), grmr);
    o.insert(QStringLiteral("tags"), tags);
    o.insert(QStringLiteral("mark"), mark);
    o.insert(QStringLiteral("refr"), refr);
    o.insert(QStringLiteral("imag"), imag);
    o.insert(QStringLiteral("imgr"), imgr);
    o.insert(QStringLiteral("link"), link);
    o.insert(QStringLiteral("cdid"), cdid);
    o.insert(QStringLiteral("type"), type);
    return QJsonValue(o);
}

Item Item::fromJson(const QJsonValue &value, const QString &trgt)
{
    Item it;
    it.trgt = trgt;
    const QJsonObject o = value.toObject();
    it.srce = o.value(QStringLiteral("srce")).toString();
    for (const char *s : ItemDefs::translationSuffixes) {
        const QString key = QStringLiteral("sl%1").arg(QLatin1String(s));
        // slXX vacío = "sin traducción alternativa": no puebla el mapa (así
        // un ítem con todos los slXX vacíos tiene mapa vacío, como el original).
        if (o.contains(key) && !o.value(key).toString().isEmpty())
            it.translations.insert(QLatin1String(s), o.value(key).toString());
    }
    it.exmp = o.value(QStringLiteral("exmp")).toString();
    it.defn = o.value(QStringLiteral("defn")).toString();
    it.note = o.value(QStringLiteral("note")).toString();
    it.wrds = o.value(QStringLiteral("wrds")).toString();
    it.grmr = o.value(QStringLiteral("grmr")).toString();
    it.tags = o.value(QStringLiteral("tags")).toString();
    it.mark = o.value(QStringLiteral("mark")).toString();
    it.refr = o.value(QStringLiteral("refr")).toString();
    it.imag = o.value(QStringLiteral("imag")).toString();
    it.imgr = o.value(QStringLiteral("imgr")).toString();  // ausente => vacío
    it.link = o.value(QStringLiteral("link")).toString();
    it.cdid = o.value(QStringLiteral("cdid")).toString();
    it.type = o.value(QStringLiteral("type")).toString();
    return it;
}

QList<QString> Item::jsonFieldOrder()
{
    QList<QString> order;
    for (int i = 0; i < ItemDefs::jsonFieldCount(); ++i)
        order << QLatin1String(ItemDefs::jsonFieldName(i));
    return order;
}

bool Item::operator==(const Item &o) const
{
    return trgt == o.trgt && srce == o.srce && exmp == o.exmp &&
           defn == o.defn && note == o.note && wrds == o.wrds &&
           grmr == o.grmr && tags == o.tags && mark == o.mark &&
           refr == o.refr && imag == o.imag && imgr == o.imgr &&
           link == o.link && cdid == o.cdid && type == o.type &&
           translations == o.translations;
}

} // namespace core
} // namespace idiomind