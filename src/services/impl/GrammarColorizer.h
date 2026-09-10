// F5 - GrammarColorizer: coloreado gramatical de frases.
// Autoridad: Dev/ifs/mods/add/add.sh sentence_p() (líneas 119-231).
//
// Lookup en diccionarios SQLite (pronouns, nouns_adjetives, nouns_verbs,
// conjunctions, prepositions, adverbs, adjetives, verbs) → span con color.
#pragma once

#include <QList>
#include <QString>

namespace idiomind {
namespace services {

struct Part {
    QString token;
    QString color;  // vacío = sin color
};

class GrammarColorizer
{
public:
    // Colorea una frase: tokeniza → lookup en dict → envuelve en spans.
    // Devuelve la frase con markup Pango.
    QString colorize(const QString &sentence, const QString &dictPath) const;

    // Devuelve las partes individuales (para el modelo).
    QList<Part> colorizeParts(const QString &sentence, const QString &dictPath) const;

    // Colores del Bash (exactos).
    static QString colorForTable(const QString &tableName);
};

} // namespace services
} // namespace idiomind
