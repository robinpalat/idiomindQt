// F2.2 - TopicRepository: persistencia de topics sobre el layout original.
// Autoridad: docs/09-qt-architecture.md §4.1, docs/02-filesystem.md,
// docs/04-databases.md, mkdb.sh (create_tpcdb), add.sh new_topic,
// mngr.sh (mkmn / delete_topic), ifs/tpc.sh chek_topic, ifs/tls.sh check_index.
//
// Operaciones cubiertas por comportamiento real del original (solo esas;
// el resto — save/appendItem/persistReview/repair/check_index — es F3):
//   listTopics(tlng)   -> mkmn (find ... -exec ls -tNd; recientes + antiguas)
//   topicExists        -> [ -d "$DM_tl/<tpc>" ]
//   loadTopic          -> lectura del estado tal cual está en disco
//   createTopic        -> new_topic + chek_topic + create_tpcdb (sin UI/yad)
//   removeTopic        -> delete_topic (parte de ficheros, sin diálogos)
//
// DECISIONES DE DISEÑO F2.2:
//  - AppPaths es la ÚNICA fuente de rutas; nunca hay $HOME hardcodeado.
//    La "raíz alternativa" se consigue inyectando AppPaths(homeDir=tmp),
//    por lo que los tests NO tocan ~/.idiomind ni ~/.config/idiomind.
//  - listTopics replica `-mtime -80` (menos de 80 días) y `-mtime +79`
//    (>= 80 días), ordenado por mtime descendente en cada grupo. En empates
//    de mtime se desempata por nombre ascendente para garantizar el orden
//    determinista que exige F2.2 (el original no garantizaba empates).
//  - El stts runtime es el FICHERO .conf/stts (autoridad real: tpc.sh/mkmn
//    leen el fichero). La columna id.stts de la DB es export/legacy y NO se
//    usa para info.stts().
//  - createTopic: si el nombre ya existe, deduplica con sufijo " (n)" igual
//    que new_topic (1..50); cap de 120 topics; nombre > 55 se rechaza.
//  - El .share/index (2 líneas por topic, artefacto de display) y mkmn
//    completo quedan para F3 junto con check_index.
#pragma once

#include "core/config/AppPaths.h"
#include "core/topic/Topic.h"
#include "core/topic/TopicInfo.h"
#include "core/topic/TopicList.h"

#include <QString>

namespace idiomind {
namespace storage {

class SqliteDatabase;

class TopicRepository
{
public:
    explicit TopicRepository(core::AppPaths appPaths);

    // Topics del idioma en el orden del original (mkmn): primero los
    // modificados hace < 80 días (más reciente primero), después el resto.
    // Omitidos los directorios ocultos (./.share, nombres con '.' inicial).
    core::TopicList listTopics(const QString &tlng) const;

    // Un topic existe si su directorio es un directorio real no oculto.
    bool topicExists(const QString &tlng, const QString &name) const;

    // Estado tal cual: stts/.conf, note, ítems de data (FlatItemCodec) e
    // info desde la DB id (si existe). NO crea ni repara nada (eso es F3).
    core::Topic loadTopic(const QString &tlng, const QString &name) const;

    // new_topic + chek_topic + create_tpcdb, sin UI. Crea la estructura
    // .conf/{data,stts,note,practice/log1..3,images/} y la DB tpc con el DDL
    // EXACTO de mkdb.sh. Si `activate` -> escribe $DC_s/tpc.
    bool createTopic(const QString &tlng, const core::TopicInfo &info,
                     bool activate = false);

    // delete_topic (solo ficheros): borra el directorio del topic y su backup.
    // Limpia $DC_s/tpc si apuntaba a él. Devuelve false si no existía.
    bool removeTopic(const QString &tlng, const QString &name);

    // mkdb.sh tpc (F3.1/TopicRepairService): recrea el fichero SQLite tpc del
    // topic desde cero (DDL + filas de id/config/reviews/Practice_stats de
    // create_tpcdb). Se usa cuando `file` no detecta SQLite en check_index.
    bool recreateTopicDatabase(const QString &tlng, const QString &name);

private:
    bool ensureTopicDatabase(const QString &tlng, const core::TopicInfo &info);
    bool topicCountBelowLimit(const QString &tlng) const;

    core::AppPaths m_appPaths;
};

} // namespace storage
} // namespace idiomind