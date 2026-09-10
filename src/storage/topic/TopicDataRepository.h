// F3.1 - TopicDataRepository: persistencia de ítems, índice y ficheros de
// estado (.conf/data, .conf/index, stts, stts.bk, note) sobre el layout
// original. También las listas SQLite del topic (learning/learnt/words/
// sentences/marks) que se mantienen en cada operación, igual que el Bash.
//
// Autoridad: docs/research/f3.1-bash-behavior.md (verbatim), ifs/mods/add/
// add.sh index(), mngr.sh edit_item/delete_item_ok, ifs/tls.sh colorize.
// NO repara nunca (eso es TopicRepairService). NO escribe al leer.
//
// Reglas de fidelidad:
//  - append  = `index()` no-edit de mods/add (template de 14 campos + tablas
//    + 3 líneas de índice).
//  - update  = `index edit` + sed por nombre de campo de edit_item: sólo se
//    cambian los campos indicados; los segmentos desconocidos (slXX/imgr) se
//    conservan byte a byte (reencodeItem).
//  - remove  = delete_item_ok (físico: línea, listas, logs, mp3/jpg, lk).
#pragma once

#include "core/config/AppPaths.h"
#include "core/topic/FlatItemCodec.h"
#include "core/topic/Item.h"

#include <QList>
#include <QString>

namespace idiomind {
namespace storage {

class TopicDataRepository
{
public:
    using DecodedItem = core::FlatItemCodec::DecodedItem;

    explicit TopicDataRepository(core::AppPaths appPaths);

    // -- ítems (data) -------------------------------------------------------
    // Lee .conf/data: una línea plana por ítem; devuelve Item + segmentos en
    // el orden original (para re-escribir byte a byte). Sólo lectura.
    QList<DecodedItem> loadItems(const QString &tlng, const QString &topic) const;

    // Reescribe .conf/data desde una lista de segmentos (reencodeItem).
    // Cada línea termina en '\n'; sin separador en blanco (sed '/^$/d' es de
    // reparación). No toca las tablas de la DB.
    bool writeItems(const QString &tlng, const QString &topic,
                    const QList<DecodedItem> &items) const;

    // `index()` de mods/add (rama no-edit): añade el ítem al final de data,
    // a learning (+ words/sentences según type) y las 3 líneas de index.
    // type==1 -> unset wrds/grmr/link/defn; type!=1 -> unset defn.
    // Devuelve false (sin efecto) si trgt vacío, falta .conf, o ya existe una
    // línea que contenga el texto literal `trgt{<trgt>` (guard del original).
    bool appendItem(const QString &tlng, const QString &topic, const core::Item &item);

    // Equivalente de edit_item (mismo topic) + index edit. `changes` recoge
    // los campos editados; los que coincidan con el valor actual no se
    // reescriben (sed old==new no-op). Si changes.trgt difiere del actual:
    //  - renombra en learning/learnt/words/sentences/marks (tpc_db 7) y en
    //    los ficheros practice (sustitución de línea completa);
    //  - en la línea de data: trgt -> nuevo capital y grmr -> (type==2 ?
    //    nuevo trgt : "") tal y como hace el sed del original (quirk);
    //  - srce sólo se reemplaza si el llamador lo varía (el original lo
    //    re-traduce por red; eso es F7 — se documenta).
    // Cambio de type: mueve entre words/sentences. mark TRUE -> inserta en
    // marks; mark FALSE -> NO-OP (reproduce el bug `tpc_db 4 marks "$trgt"`).
    // Al final regenera el índice (colorize) si algo cambió.
    bool updateItem(const QString &tlng, const QString &topic,
                    const QString &targetTrgt, const core::Item &changes);

    // delete_item_ok: borra la línea de data, las filas de las 5 listas, las
    // coincidencias en practice/log1..3, el mp3 por cdid y la imagen por
    // trgt en minúsculas. Si data < 200 líneas y existe .conf/lk lo borra;
    // si existe .conf/feeds añade trgt a .conf/exclude. Regenera el índice.
    // NO toca reviews ni los contadores nwrd/nsnt de id (quirk, documentado).
    bool removeItem(const QString &tlng, const QString &topic, const QString &trgt);

    // -- índice (.conf/index) -----------------------------------------------
    // Líneas tal cual (grupos de 3: texto / chk / srce). Vacío si falta fichero.
    QStringList loadIndex(const QString &tlng, const QString &topic) const;

    struct IndexReport
    {
        int indexCount = 0;      // (líneas no vacías del índice)/3
        int learningCount = 0;   // filas no vacías de learning
        bool valid = false;      // indexCount == learningCount
    };

    // Comprobación pura (tls.sh check_index): nunca escribe.
    IndexReport validateIndex(const QString &tlng, const QString &topic) const;

    // colorize (tls.sh:1005): regenera .conf/index con SOLO los ítems que
    // están en learning. chkFlag == 1 -> TRUE en líneas de log1 cuando
    // config.acheck == TRUE; el resto FALSE. Aplica a las marcas <b><big> y
    // a los colores de log2/log3. También `touch` del directorio del topic.
    bool rebuildIndex(const QString &tlng, const QString &topic,
                      bool chkFlag = false);

    // -- ficheros de estado ------------------------------------------------
    QString readStts(const QString &tlng, const QString &topic) const;
    bool writeStts(const QString &tlng, const QString &topic,
                   const QString &value) const;
    // update_lists.sh:12: `mv stts stts.bk; echo 0 > stts`
    bool moveSttsToBackup(const QString &tlng, const QString &topic) const;
    // check_index/tpc.sh: stts <- contenido COMPLETO de stts.bk; borra stts.bk.
    bool restoreSttsFromBackup(const QString &tlng, const QString &topic) const;
    bool removeSttsBackup(const QString &tlng, const QString &topic) const;

    QString readNote(const QString &tlng, const QString &topic) const;
    bool writeNote(const QString &tlng, const QString &topic,
                   const QString &text) const;

    // Acceso crudo a las listas del topic (tpc_db 5) para el repair y tests.
    QStringList learningList(const QString &tlng, const QString &topic) const;
    QStringList learntList(const QString &tlng, const QString &topic) const;
    QStringList marksList(const QString &tlng, const QString &topic) const;

    // F6-C.3: operaciones per-item en learning/learnt
    bool removeLearningItem(const QString &tlng, const QString &topic,
                            const QString &trgt);
    bool insertLearntItem(const QString &tlng, const QString &topic,
                          const QString &trgt);

    // Aplica `touch` a $DM_tlt (marca de actividad que usa mkmn/check_index).
    static void bumpTopicMtime(const QString &topicDataDirPath);

private:
    static QStringList readLines(const QString &filePath);
    static QStringList practiceLogs(const QString &confDir);
    static QStringList tableRows(const QString &dbPath, const QString &table);
    static int countRows(const QString &dbPath, const QString &table);
    static bool updateListRows(const QString &dbPath, const QString &table,
                               const QString &newValue, const QString &oldValue);
    static bool deleteRow(const QString &dbPath, const QString &table,
                          const QString &value);
    static bool insertRow(const QString &dbPath, const QString &table,
                          const QString &value);
    static QString configAcheck(const QString &dbPath);

    core::AppPaths m_appPaths;
};

} // namespace storage
} // namespace idiomind