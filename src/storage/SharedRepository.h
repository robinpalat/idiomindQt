// F4 - SharedRepository: persistencia de las listas T1..T10 en shrdb.
// Autoridad: ifs/mkdb.sh (create_shrdb), main.sh _get_list, mngr.sh mark_*.
//
// shrdb = $DM_tls/data/config (shared DB). Tablas T1..T10, cada una con
// columna `list TEXT` que contiene nombres de topics.
#pragma once

#include "core/config/AppPaths.h"
#include "core/learning/ReviewCalculator.h"

#include <QString>
#include <QStringList>

namespace idiomind {
namespace storage {

using ReviewCalculatorKind = ::ReviewCalculator::ListKind;

class SharedRepository
{
public:
    explicit SharedRepository(core::AppPaths appPaths);

    // Añade un topic a la lista Tn correspondiente a ListKind.
    bool insertTopic(const QString &tlng, ReviewCalculatorKind kind,
                     const QString &topic) const;

    // Elimina un topic de la lista Tn.
    bool removeTopic(const QString &tlng, ReviewCalculatorKind kind,
                     const QString &topic) const;

    // Elimina un topic de TODAS las listas T1..T10.
    bool removeAllLists(const QString &tlng, const QString &topic) const;

    // Lee todos los topics de una lista Tn.
    QStringList listOf(const QString &tlng,
                       ReviewCalculatorKind kind) const;

    // Lee una tabla T1..T10 que no tiene un ReviewCalculatorKind específico.
    QStringList listOfNumber(const QString &tlng, int number) const;

    // Verifica si un topic está en una lista.
    bool contains(const QString &tlng, ReviewCalculatorKind kind,
                  const QString &topic) const;

    // Asegura que las tablas T1..T10 existen (create if not exists).
    bool ensureTables(const QString &tlng) const;

private:
    static QString tableName(ReviewCalculatorKind kind);
    core::AppPaths m_appPaths;
};

} // namespace storage
} // namespace idiomind
