// F4 - PracticeLogs: persistencia de log1..log3 ($DC_tlt/practice/).
// Autoridad: docs/10-practice.md, practice/strt.sh, ifs/tls.sh colorize.
//
// log1 = historial de práctica del índice (auto-check), log2/log3 = ítems
// marcados como fallidos en ciclos recientes. Los 3 logs alimentan
// colorize/IndexBuilder para colorear el índice (rojo=naranja=plain).
//
// Formato: una línea por trgt, sin orden garantizado. Bash: `$(< logN)` + Python `in`.
#pragma once

#include <QSet>
#include <QString>
#include <QStringList>

namespace idiomind {
namespace core {

class PracticeLogs
{
public:
    PracticeLogs() = default;

    // Carga los 3 logs desde los ficheros practice/log1..3.
    bool load(const QString &confDir);

    // Persiste los 3 logs a los ficheros.
    bool save(const QString &confDir) const;

    // Vacía los 3 logs y reescribe los ficheros vacíos.
    bool clear(const QString &confDir);

    // Contenido crudo (para comparación byte-a-byte con Bash).
    QString rawLog(int n) const;  // n = 1..3
    void setRawLog(int n, const QString &content);

    // Búsqueda (replica Python `item in logN` — substring sobre contenido completo).
    bool inLog(int n, const QString &trgt) const;

    // Añade un trgt a un log (append línea).
    void append(int n, const QString &trgt);

    // Elimina un trgt de todos los logs.
    void remove(const QString &trgt);

    // Todos los trgt de un log.
    QStringList entries(int n) const;

    // Logs como QSet para búsqueda O(1).
    QSet<QString> log1Set() const { return m_log1Set; }
    QSet<QString> log2Set() const { return m_log2Set; }
    QSet<QString> log3Set() const { return m_log3Set; }

private:
    QString m_raw1, m_raw2, m_raw3;
    QSet<QString> m_log1Set, m_log2Set, m_log3Set;

    static void rebuildSet(const QString &raw, QSet<QString> &set);
};

} // namespace core
} // namespace idiomind
