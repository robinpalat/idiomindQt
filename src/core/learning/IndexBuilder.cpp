#include "core/learning/IndexBuilder.h"

#include "core/topic/FlatItemCodec.h"

namespace idiomind {
namespace core {

QStringList buildIndexLines(const IndexBuildInput &input)
{
    QStringList lines;
    const bool chk = input.acheck && input.chkFlag;

    for (const QString &raw : input.dataLines) {
        const QString stripped = raw.trimmed();
        if (stripped.isEmpty())
            continue;
        const FlatItemCodec::DecodedItem d = FlatItemCodec::decodeWithSegments(stripped);
        if (!input.learning.contains(d.item.trgt))
            continue;

        // srce: buscar en segmentos
        QString srce;
        for (const auto &s : d.segments)
            if (s.first == QLatin1String("srce")) { srce = s.second; break; }

        // marks: <b><big>trgt</big></b>
        QString text = input.marks.contains(d.item.trgt)
            ? QStringLiteral("<b><big>") + d.item.trgt + QStringLiteral("</big></b>")
            : d.item.trgt;

        // log3 (rojo) > log2 (naranja) > log1 (chk) > plain
        QString flag = QStringLiteral("FALSE");
        if (input.logs.inLog(3, d.item.trgt)) {
            text = QStringLiteral("<span color='#AE3259'>") + text + QStringLiteral("</span>");
        } else if (input.logs.inLog(2, d.item.trgt)) {
            text = QStringLiteral("<span color='#C15F27'>") + text + QStringLiteral("</span>");
        } else if (input.logs.inLog(1, d.item.trgt)) {
            flag = chk ? QStringLiteral("TRUE") : QStringLiteral("FALSE");
        }

        lines.append(text);
        lines.append(flag);
        lines.append(srce);
    }
    return lines;
}

} // namespace core
} // namespace idiomind
