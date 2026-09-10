// F3.1 - TopicRepairService: replicación de `tls.sh check_index` (verbatim en
// docs/research/f3.1-bash-behavior.md §2). Diagnostica y repara la integridad
// de un topic (data/index/stts/stts.bk/note + tablas SQLite).
//
// Reglas de fidelidad (todas documentadas):
//  - Secuencia EXACTA de _check -> _restore -> db fixes -> colorize.
//  - No-notify en Qt (el parámetro quiet se acepta y se ignora: no hay UI).
//  - `if [ $? != 0 ]` de tls.sh:165 es MUERTO (no-op) -> NO se reproduce.
//  - mkmn/.share/index (menú topics) queda FUERA (documentado en el diseño).
//  - El rehacer index final usa colorize con $2 VACÍO: chk SIEMPRE FALSE
//    (qué hace `colorize 1` real: `[[ ${2} = 1 ]]` es falso) => rebuildIndex
//    se llama con chkFlag=false en TODAS las llamadas internas.
#pragma once

#include "core/config/AppPaths.h"

#include <QString>

namespace idiomind {
namespace services {

class TopicRepairService
{
public:
    struct CheckReport
    {
        bool fixed = false;           // se detectó algo que reparar
        bool dataRestored = false;    // data recuperada desde .bk o reescrita
        bool indexRebuilt = false;    // colorize ejecutado (mkmn==1)
        bool dbConfigFixed = false;
        bool dbReviewsFixed = false;
        bool dbIdFixed = false;
        bool errored = false;         // sin backup para data faltante
        int dataCount = 0;            // cnt0 (líneas no vacías de data)
        int indexCount = 0;           // index0 ((no vacías index)/3)
    };

    explicit TopicRepairService(core::AppPaths appPaths);

    // tls.sh check_index <topic> [r]. `quiet` sólo suprime el notify del
    // original (no hay UI en Qt): se acepta y se ignora.
    CheckReport checkIndex(const QString &tlng, const QString &topic,
                           bool quiet = false);

private:
    // _restore: data desde el backup si falta; siempre re-escritura+listas.
    // Devuelve false si no existe backup (error original -> abort).
    bool restoreTopic(const QString &tlng, const QString &topic,
                      const QString &stts, CheckReport &rep);

    core::AppPaths m_appPaths;
};

} // namespace services
} // namespace idiomind