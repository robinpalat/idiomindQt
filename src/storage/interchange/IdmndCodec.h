// F3.2 - IdmndCodec: codec del formato de intercambio nativo `.idmnd`
// (JSON plano de 3 líneas del Idiomind original).
//
// Autoridad: ficheros reales Web/idiomind.com/public/english/**/*.idmnd,
// default/vars (líneas 1-3), ifs/mods/export/Idiomind Topic (idmnd).sh,
// ifs/tls.sh check_format_1, main.sh (import heritage).
//
// Formato (3 líneas de contenido + '\n' final):
//   1ª: {"items":{
//   2ª: ítems uno tras otro (coma entre ellos):
//         "<trgt>":{"srce":"...","slch":"...", ..., "type":"1"}
//       y la cola `}},` (cierre del ítem + cierre del objeto items + ',').
//   3ª: metadatos del topic SIN llaves: "name":...,"slng":...,"stts":"0"
//
// El orden de ítems y el orden de campos DEL FORMATO son parte del contrato:
// QJsonObject reordena las claves al serializar y QJsonDocument no conserva
// el orden de los ítems, por eso la 2ª línea se emite/lee con un escáner
// manual (equilibrio de llaves + Item::fromJson para el cuerpo).
#pragma once

#include "core/topic/Item.h"
#include "core/topic/TopicInfo.h"

#include <QByteArray>
#include <QList>
#include <QString>

namespace idiomind {
namespace storage {

// Documento .idmnd de 3 líneas: metadatos + ítems en el orden del fichero.
struct IdmndDocument
{
    core::TopicInfo info;
    QList<core::Item> items;
};

class IdmndCodec
{
public:
    // Parsea el texto completo del .idmnd. `ok` = false cuando la estructura
    // no es la del formato (misma razón que check_format_1 del original):
    //  - wc -l != 3                -> error "%1 Lines!"
    //  - JSON global inválido      -> error "Format"
    //  - 1ª línea != {"items":{    -> error "Format"
    //  - 2ª línea no escaneable    -> error "Format"
    //  - 3ª línea sin metadatos    -> error "Format"
    // La validación de VALORES de la 3ª línea (check_format_1) la hace
    // LegacyImporter; aquí bastan la validez estructural y los campos.
    static IdmndDocument parse(const QByteArray &text, bool *ok = nullptr,
                               QString *error = nullptr);
    static IdmndDocument parseFile(const QString &path, bool *ok = nullptr,
                                   QString *error = nullptr);

    // Emite el .idmnd canónico: 3 líneas + '\n'. Ítems en el orden de `doc`,
    // campos en el orden canónico (ItemDefs::jsonFieldName) y escaping JSON
    // estándar (el del resto del proyecto). Para los ficheros reales del
    // formato se cumple write(parse(file)) == file (bytes).
    static QByteArray write(const IdmndDocument &doc);

private:
    // Escaneo manual de la 2ª línea: `"<trgt>":{...}` uno tras otro y cola
    // `},` (cierre del objeto items). Conserva el orden de los ítems.
    static bool parseItemsLine(const QString &line, QList<core::Item> &out,
                               QString *error);
    // Lee un string JSON `"..."` (escapes \", \\, \uXXXX...) a partir de `i`.
    // Deja `i` tras la comilla de cierre; `out` recibe el valor desescapado.
    static bool readJsonString(const QString &s, int &i, QString &out);
    static void skipWs(const QString &s, int &i);
};

} // namespace storage
} // namespace idiomind