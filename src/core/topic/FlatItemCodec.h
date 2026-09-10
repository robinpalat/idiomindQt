// F2.1 - FlatItemCodec: línea plana original <-> Item.
// Autoridad: docs/03-data-model.md, default/vars línea 2, ifs/cmns.sh get_item.
#pragma once

#include "core/topic/Item.h"

#include <QList>
#include <QPair>
#include <QString>

namespace idiomind {
namespace core {

// Codifica/decodifica la línea plana de $DC_tlt/data. El original escribe el
// formato canónico de 14 campos (default/vars línea 2) SOLO al añadir ítems
// nuevos; las líneas existentes pueden usar el formato web/import de 25
// campos (slXX + imgr, ver docs/research/f3.1-bash-behavior.md §1b) y deben
// conservarse byte a byte. `decode`/`encode` siguen siendo get_item /
// dataset writer; `decodeWithSegments`/`reencodeItem` preservan los segmentos
// extra y su orden (análogo al re-escaneo que hace _restore del original).
namespace FlatItemCodec {

// Item -> línea plana canónica (14 campos, orden de default/vars línea 2).
// NO hay escaping: el original escribe los valores tal cual (get_item/
// dataset writer); un '}' dentro de un valor rompe el re-lecturado igual que
// el original. No se "mejora" el formato.
QString encode(const Item &item);

// Línea plana -> Item. Espejo de get_item (ifs/cmns.sh): busca cada campo por
// su nombre y toma el texto hasta el primer '}'. Los campos ausentes quedan
// vacíos. Los campos extra de líneas antiguas se ignoran (el nombre de campo
// es la autoridad, no el índice).
Item decode(const QString &line);

// -- F3.1: preservación de segmentos (round-trip byte a byte) ----------------

// Un segmento `name{value` tal y como lo interpreta el original al partir la
// línea por '}' (clean/re_split de _restore).
using Segment = QPair<QString, QString>;  // first = nombre, second = valor

struct DecodedItem
{
    Item item;              // item decodificado (campos canónicos)
    QList<Segment> segments;  // todos los segmentos en el orden original
};

// Línea plana -> Item + segmentos ordenados. Parte por '}' igual que el
// original y toma `value` hasta el primer '}' siguiente a `name{`.
DecodedItem decodeWithSegments(const QString &line);

// Re-ensambla `orig.segments` tomando el valor ACTUAL de `current` para los
// campos canónicos (trgt..type) y conservando byte a byte el valor original
// de los segmentos desconocidos (slXX, imgr...). Equivale al `sed` de edición
// del original: sólo cambian los campos que el llamador varió.
QString reencodeItem(const DecodedItem &orig, const Item &current);

}  // namespace FlatItemCodec
} // namespace core
} // namespace idiomind