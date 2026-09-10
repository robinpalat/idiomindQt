// F3.2/F3.2b - LegacyImporter: instalación de un `.idmnd` legacy (JSON plano
// de 3 líneas) en el perfil, replicando main.sh:216-423 (heritage de import).
//
// Autoridad: main.sh (import head), ifs/tls.sh check_format_1 (validación),
// ifs/mkdb.sh create_tpcdb (DB), ifs/cmns.sh tpc_db (escrituras/lecturas).
//
// Fidelidad de comportamiento (todas verificadas contra el Bash):
//  - validación estructural IdmndCodec (líneas/JSON/1ª línea) + validación
//    de VALORES de check_format_1 (reglas efectivas, ver .cpp);
//  - slng acepta traducciones extra tras coma ("Español, English, French"):
//    se valida la parte primaria contra kSlangs; las extras se ignoran
//    (Bash: otranslations solo se usa en preview, no en import);
//  - chars prohibidos: name/orig (~ `*/@=`), autr/cntt (~ `*/()=`, Bash:
//    regex `\*|\/|$|\)|\(|=`), stts (~ `*/@=-`). El Bash original tiene un
//    bug: `$` y `|` al final de la regex la hacen siempre-verdadera, y
//    `invalid()` usa `& exit 1` (subshell) por lo que nunca detiene el
//    import. Implementamos la intención (rechazar los chars prohibidos);
//  - tope de 120 topics por idioma y dedup "name (i)" 1..50 (main.sh:293-300;
//    caído de 50 -> " (51)", igual que el fallback del bucle Bash);
//  - check_file: note, download y practice/log1..3 se crean VACÍOS;
//  - mkdb tpc + `tpc_db 9 id ...`: la fila id recibe name, slng, tlng, autr,
//    ctgy, ilnk, orig, dtec, dtei=today, nwrd, nsnt, nimg, naud, nsze, levl.
//    NO se importan cntt, dteu, info ni stts (quedan '', como en el Bash);
//  - slng/tlng mismatch: si el slng del topic difiere del usuario, crea
//    translations/active + slng_err (codec); el reinicio de tray es runtime.
//  - data: LÍNEAS PLANAS de 25 segmentos (trgt + orden canónico json), que
//    conservan slXX e imgr (los ficheros reales los llevan);
//  - tablas: learning siempre, words/sentences según type, marks si
//    mark==TRUE, y Data (14 columnas, sin imgr ni slXX: el python del
//    original no las inserta);
//  - colorize 1 tras el import (index) y `echo 1 > stts`.
//  - NO hay truncamiento a 200 ítems en import (solo en _restore, F3.1).
//  - NO se restaura multimedia (ZIPs/imágenes/audio): pendiente F3.x+.
#pragma once

#include "core/config/AppPaths.h"
#include "core/topic/Item.h"
#include "core/topic/TopicInfo.h"

#include <QByteArray>
#include <QString>

namespace idiomind {
namespace storage {

struct IdmndDocument;

class LegacyImporter
{
public:
    explicit LegacyImporter(core::AppPaths appPaths);

    // Idioma del perfil del usuario (para detectar slng/tlng mismatch).
    void setProfileLanguages(const QString &slng, const QString &tlng);
    QString profileSlng() const { return m_profileSlng; }
    QString profileTlng() const { return m_profileTlng; }

    // Validación PURA de un .idmnd (check_format_1): sin efectos sobre el
    // perfil. `ok` = formato válido (3 líneas + JSON + 1ª línea + reglas de
    // valores) y `doc` = documento parseado (uso en tests/preview).
    static IdmndDocument validate(const QByteArray &text, bool *ok,
                                  QString *error = nullptr);

    // Instala el .idmnd en el idioma `tlng`. Devuelve el NOMBRE FINAL del
    // topic creado (después del dedup) en `topicName`. `ok` = false si la
    // validación falla ("Format"/"<n> Lines!") o si se alcanza el tope de
    // topics. Acepta tanto el .idmnd plano como el paquete ZIP .idmnd con
    // topic.idmnd, imágenes y audio.
    bool importText(const QString &tlng, const QByteArray &idmnd,
                    QString *topicName = nullptr, QString *error = nullptr) const;
    bool importFile(const QString &tlng, const QString &path,
                    QString *topicName = nullptr, QString *error = nullptr) const;

    // Mismatch resultante del import (para tests).
    struct MismatchResult {
        bool slngMismatch = false;
        bool tlngMismatch = false;
    };
    MismatchResult lastMismatch() const { return m_lastMismatch; }

private:
    bool install(const IdmndDocument &doc, const QString &tlng,
                 QString *topicName, QString *error) const;
    bool installZip(const QByteArray &zipData, const QString &tlng,
                    QString *topicName, QString *error) const;

    core::AppPaths m_appPaths;
    QString m_profileSlng;
    QString m_profileTlng;
    mutable MismatchResult m_lastMismatch;
};

} // namespace storage
} // namespace idiomind
