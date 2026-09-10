// F4 - IndexBuilder: generación del índice (colorize portado a C++).
// Autoridad: ifs/tls.sh colorize (1005-1058), docs/06-learning-system.md.
//
// Genera las 3 líneas por ítem del índice: texto (con spans Pango si aplica),
// flag TRUE/FALSE, y srce. El orden es el del data file; solo aparecen ítems
// que están en learning.
#pragma once

#include "core/learning/PracticeLogs.h"

#include <QSet>
#include <QString>
#include <QStringList>

namespace idiomind {
namespace core {

struct IndexBuildInput {
    QStringList dataLines;       // líneas de .conf/data (cada una = 1 ítem)
    QSet<QString> learning;      // tabla learning
    QSet<QString> marks;         // tabla marks
    PracticeLogs logs;           // log1..log3
    bool acheck = false;         // config.acheck == TRUE
    bool chkFlag = false;        // argumento colorize (1 = TRUE si acheck)
};

// Genera el contenido del fichero .conf/index.
// Devuelve las líneas exactas (sin trailing newline adicional).
QStringList buildIndexLines(const IndexBuildInput &input);

} // namespace core
} // namespace idiomind
