#include "core/topic/TopicInfo.h"

#include <QJsonDocument>
#include <QJsonValue>

namespace idiomind {
namespace core {

const char *TopicInfo::fieldName(int i)
{
    static constexpr const char *names[] = {
        "name", "slng", "tlng", "autr", "cntt", "ctgy", "ilnk",
        "orig", "dtec", "dteu", "dtei", "nwrd", "nsnt", "nimg",
        "naud", "nsze", "levl", "info", "stts"};
    return (i >= 0 && i < int(sizeof(names) / sizeof(names[0]))) ? names[i]
                                                                 : nullptr;
}

QJsonObject TopicInfo::toJson() const
{
    QJsonObject o;
    const QString *fields[] = {
        &m_name, &m_slng, &m_tlng, &m_autr, &m_cntt, &m_ctgy, &m_ilnk,
        &m_orig, &m_dtec, &m_dteu, &m_dtei, &m_nwrd, &m_nsnt, &m_nimg,
        &m_naud, &m_nsze, &m_levl, &m_info, &m_stts};
    for (int i = 0; i < fieldCount(); ++i)
        o.insert(QLatin1String(fieldName(i)), *fields[i]);
    return o;
}

TopicInfo TopicInfo::fromJson(const QJsonObject &obj)
{
    TopicInfo t;
    t.m_name = obj.value(QLatin1String(fieldName(0))).toString();
    t.m_slng = obj.value(QLatin1String(fieldName(1))).toString();
    t.m_tlng = obj.value(QLatin1String(fieldName(2))).toString();
    t.m_autr = obj.value(QLatin1String(fieldName(3))).toString();
    t.m_cntt = obj.value(QLatin1String(fieldName(4))).toString();
    t.m_ctgy = obj.value(QLatin1String(fieldName(5))).toString();
    t.m_ilnk = obj.value(QLatin1String(fieldName(6))).toString();
    t.m_orig = obj.value(QLatin1String(fieldName(7))).toString();
    t.m_dtec = obj.value(QLatin1String(fieldName(8))).toString();
    t.m_dteu = obj.value(QLatin1String(fieldName(9))).toString();
    t.m_dtei = obj.value(QLatin1String(fieldName(10))).toString();
    t.m_nwrd = obj.value(QLatin1String(fieldName(11))).toString();
    t.m_nsnt = obj.value(QLatin1String(fieldName(12))).toString();
    t.m_nimg = obj.value(QLatin1String(fieldName(13))).toString();
    t.m_naud = obj.value(QLatin1String(fieldName(14))).toString();
    t.m_nsze = obj.value(QLatin1String(fieldName(15))).toString();
    t.m_levl = obj.value(QLatin1String(fieldName(16))).toString();
    t.m_info = obj.value(QLatin1String(fieldName(17))).toString();
    t.m_stts = obj.value(QLatin1String(fieldName(18))).toString();
    return t;
}

namespace {
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

QString TopicInfo::toJsonText() const
{
    QString out;
    const QString *fields[] = {
        &m_name, &m_slng, &m_tlng, &m_autr, &m_cntt, &m_ctgy, &m_ilnk,
        &m_orig, &m_dtec, &m_dteu, &m_dtei, &m_nwrd, &m_nsnt, &m_nimg,
        &m_naud, &m_nsze, &m_levl, &m_info, &m_stts};
    for (int i = 0; i < fieldCount(); ++i) {
        if (i)
            out += QChar::fromLatin1(',');
        out += QStringLiteral("\"%1\":\"%2\"").arg(QLatin1String(fieldName(i)),
                                                   jsonEscape(*fields[i]));
    }
    return out;
}

TopicInfo TopicInfo::fromJsonText(const QString &text)
{
    QString t = text.trimmed();
    if (!t.startsWith(QChar::fromLatin1('{')))
        t = QStringLiteral("{") + t;
    if (!t.endsWith(QChar::fromLatin1('}')))
        t += QChar::fromLatin1('}');
    const QJsonDocument doc = QJsonDocument::fromJson(t.toUtf8());
    if (!doc.isObject())
        return {};
    return fromJson(doc.object());
}

bool TopicInfo::operator==(const TopicInfo &o) const
{
    return m_name == o.m_name && m_slng == o.m_slng && m_tlng == o.m_tlng &&
           m_autr == o.m_autr && m_cntt == o.m_cntt && m_ctgy == o.m_ctgy &&
           m_ilnk == o.m_ilnk && m_orig == o.m_orig && m_dtec == o.m_dtec &&
           m_dteu == o.m_dteu && m_dtei == o.m_dtei && m_nwrd == o.m_nwrd &&
           m_nsnt == o.m_nsnt && m_nimg == o.m_nimg && m_naud == o.m_naud &&
           m_nsze == o.m_nsze && m_levl == o.m_levl && m_info == o.m_info &&
           m_stts == o.m_stts;
}

} // namespace core
} // namespace idiomind