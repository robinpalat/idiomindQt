// F2.1 - Item: un ítem del Idiomind original.
// Autoridad: docs/03-data-model.md, docs/05-topic-format.md, default/vars (líneas 1 y 2).
#pragma once

#include <QJsonValue>
#include <QMap>
#include <QString>

namespace ItemDefs {
// Claves de la línea plana (default/vars línea 2), en orden canónico.
constexpr int flatFieldCount() { return 14; }
inline const char *flatFieldName(int i) {
    static constexpr const char *names[] = {
        "trgt", "srce", "exmp", "defn", "note", "wrds", "grmr",
        "tags", "mark", "refr", "imag", "link", "cdid", "type"};
    return names[i];
}
// Claves del objeto JSON del ítem .idmnd (default/vars línea 1 + slvi del
// formato real). Orden exacto del formato documentado.
inline const char *jsonFieldName(int i) {
    static constexpr const char *names[] = {
        "srce", "slch", "slde", "slen", "sles", "slfr", "slit",
        "slja", "slpt", "slru", "slvi", "exmp", "defn", "note",
        "wrds", "grmr", "tags", "mark", "refr", "imag", "imgr",
        "link", "cdid", "type"};
    return names[i];
}
constexpr int jsonFieldCount() { return 24; }
// Sufijos de las traducciones alternativas slXX.
inline constexpr const char *translationSuffixes[] = {
    "ch", "de", "en", "es", "fr", "it", "ja", "pt", "ru", "vi"};
constexpr int translationSuffixCount() { return 10; }
}

namespace idiomind {
namespace core {

class Item {
public:
    Item() = default;

    QString trgt;  // texto en lengua meta (palabra o frase)
    QString srce;  // traducción a la lengua nativa
    QString exmp;  // frase de ejemplo (solo word)
    QString defn;  // definición de diccionario
    QString note;  // notas del usuario
    QString wrds;  // pares word_translation unidos por '_' (t_r)
    QString grmr;  // etiquetas gramaticales del colorizador (HTML)
    QString tags;  // etiquetas del usuario
    QString mark;  // "TRUE" si está marcado
    QString refr;  // URL/referencia
    QString imag;  // referencia de imagen
    QString imgr;  // solo presente en el objeto JSON .idmnd (real files)
    QString link;  // enlace/referencia
    QString cdid;  // id de creación / uuid
    QString type;  // "1" = word, otro = sentence

    // Traducciones alternativas slXX: clave = sufijo del código
    // (ch,de,en,es,fr,it,ja,pt,ru,vi), valor = traducción al alt-idioma.
    QMap<QString, QString> translations;

    bool isWord() const;                     // type == "1"
    bool isSentence() const;                 // type != "1" (dw contribución)
    bool isMarked() const;

    // --- .idmnd JSON ----------------------------------------------------------
    // Objeto JSON del ítem (sin la clave trgt: en .idmnd el trgt es la clave
    // externa del objeto). Inserción en el orden exacto del formato.
    QJsonValue toJson() const;
    // Forma textual ordenada del objeto (el .idmnd de 3 líneas requiere el
    // orden de campos exacto; QJsonDocument reordena las claves al
    // serializar, por eso el emisor es manual). JSON válido y con escaping
    // estándar (\" \\ y \uXXXX para <0x20). Sin la clave trgt.
    QString toJsonText() const;
    // Construye un ítem desde el objeto JSON de .idmnd. `trgt` se usa como
    // clave externa cuando el objeto no la lleva dentro.
    static Item fromJson(const QJsonValue &value, const QString &trgt = QString());

    // Lista ordenada de claves JSON (para un futuro writer .idmnd 3-líneas).
    static QList<QString> jsonFieldOrder();

    bool operator==(const Item &o) const;
    bool operator!=(const Item &o) const { return !(*this == o); }
};

} // namespace core
} // namespace idiomind