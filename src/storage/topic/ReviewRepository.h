// F3.1 - ReviewRepository: persistencia de la fila `reviews` (date1..date10)
// de un topic.
//
// Autoridad: docs/04-databases.md, ifs/cmns.sh tpc_db, mngr.sh mark_as_learned,
// docs/research/f3.1-bash-behavior.md §5/§7.
//
// Invariante del original: UNA única fila por topic. Las fechas se guardan
// como texto SQLite "%m/%d/%Y"; columna vacía = sin fecha. `clear()` replica
// `tpc_db 6 reviews` + `insert into reviews (date1) values ('')`.
#pragma once

#include "core/config/AppPaths.h"
#include "core/learning/Review.h"

#include <QDate>
#include <QString>

namespace idiomind {
namespace storage {

class ReviewRepository
{
public:
    explicit ReviewRepository(core::AppPaths appPaths);

    // Fila reviews del topic. Devuelve un Review vacío si no hay fila o el
    // fichero SQLite no existe. No escribe nunca.
    Review load(const QString &tlng, const QString &topic) const;

    // Persiste el vector completo de fechas manteniendo la fila única:
    // crea la fila `(date1) values ('')` si no existe y actualiza cada
    // columna con el contenido del Review.
    bool store(const QString &tlng, const QString &topic, const Review &review) const;

    // `tpc_db 6 reviews` + `insert (date1) values ('')`: borra todo y deja la
    // fila con date1=''.
    bool clear(const QString &tlng, const QString &topic) const;

    // check_index: si stts > 1 y date1 está vacía -> date1 = hoy (SQLite).
    bool backfillDate1IfEmpty(const QString &tlng, const QString &topic) const;

    // Número de columnas no vacías (equivale a `grep -c '[^[:space:]]'`).
    int countFilled(const QString &tlng, const QString &topic) const;

private:
    static int filledColumns(const QString &dbPath);
    static bool writeColumn(const QString &dbPath, int column, const QString &value);
    static bool rowExists(const QString &dbPath);

    core::AppPaths m_appPaths;
};

} // namespace storage
} // namespace idiomind