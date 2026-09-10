// F2.2 - ConfigRepository: persistencia básica del perfil de usuario.
// Autoridad: docs/04-databases.md, mkdb.sh create_cfgdb, c.conf, cnfg.sh,
// docs/09-qt-architecture.md §4.4.
//
// Alcance F2.2:
//  - cfgdb = $DC_s/config (SQLite, $HOME/.config/idiomind/config): esquema
//    EXACTO de create_cfgdb (no se altera DDL original ni hay migración);
//  - lang{tlng, slng}: idioma meta / idioma del usuario (1 fila, c.conf);
//  - opts: las 13 opciones del perfil (1 fila; `cdb 3` = update sin WHERE);
//  - topic activo: $DC_s/tpc (1ª línea = nombre, igual que sed -n 1p).
//
// NO se toca config.cfg (INI QSettings de otra aplicación; no es parte del
// modelo Bash: documentado en F2.2 como artefacto ajeno).
#pragma once

#include "core/config/AppPaths.h"

#include <QString>
#include <QVariantMap>

namespace idiomind {
namespace storage {

class SqliteDatabase;

class ConfigRepository
{
public:
    explicit ConfigRepository(core::AppPaths appPaths);

    // Crea $DC_s/config con el esquema y las filas de create_cfgdb si el
    // fichero no existe o no es SQLite (cnfg.sh línea 13-14). Idempotente.
    bool ensureProfile();

    // -- lang {tlng, slng} (1 única fila) --
    QString targetLanguage() const;   // select tlng from lang
    QString sourceLanguage() const;   // select slng from lang
    bool setTargetLanguage(const QString &tlng);
    bool setSourceLanguage(const QString &slng);
    bool setLanguages(const QString &tlng, const QString &slng);

    // -- opts (13 columnas, 1 única fila) --
    QVariantMap options() const;
    QString option(const QString &key) const;
    bool setOption(const QString &key, const QString &value);
    bool setOptions(const QVariantMap &values);

    // -- topic activo ($DC_s/tpc) --
    QString activeTopic() const;   // sed -n 1p
    bool setActiveTopic(const QString &topic);

private:
    SqliteDatabase *db() const;

    core::AppPaths m_appPaths;
    mutable SqliteDatabase *m_db = nullptr;
};

} // namespace storage
} // namespace idiomind