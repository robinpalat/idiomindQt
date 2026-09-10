#include "core/topic/Topic.h"

namespace idiomind {
namespace core {

Topic::Topic(const TopicInfo &info)
    : m_info(info)
{
}

int Topic::statusValue() const
{
    // Espejo EXACTO de `numer='^[0-9]+$'` (ifs/cmns.sh) + tls.sh:
    //   `! [[ ${stts} =~ $numer ]] && stts=13`
    // Solo dígitos ASCII; el whitespace NUNCA es válido (bash [[ =~ ]] no lo
    // tolera; QString::toInt sí, por eso no se usa toInt como validador).
    QString s = m_info.stts();
    if (s.isEmpty())
        return 13;
    for (const QChar c : s) {
        if (c < QLatin1Char('0') || c > QLatin1Char('9'))
            return 13;
    }
    return s.toInt();
}

bool Topic::operator==(const Topic &o) const
{
    return m_info == o.m_info && m_items == o.m_items && m_note == o.m_note;
}

} // namespace core
} // namespace idiomind