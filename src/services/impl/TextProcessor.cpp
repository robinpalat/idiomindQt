#include "services/impl/TextProcessor.h"

#include <QRegularExpression>

namespace idiomind {
namespace services {

// --- Helpers ---

QString TextProcessor::joinLines(const QString &input)
{
    QString s = input;
    // Reemplazar \ literal por espacio
    s.replace(QStringLiteral("\\n"), QStringLiteral(" "));
    // Unir líneas
    s.replace(QLatin1Char('\n'), QLatin1Char(' '));
    // Unir tabs
    s.replace(QLatin1Char('\t'), QLatin1Char(' '));
    return s;
}

QString TextProcessor::normalizeSpaces(const QString &input)
{
    QString s = input;
    // Solo normalizar espacios y tabs, NO newlines (los newlines son separadores de oraciones)
    s.replace(QChar('\t'), QLatin1Char(' '));
    s.replace(QRegularExpression(QStringLiteral("[ ]+")), QStringLiteral(" "));
    return s.trimmed();
}

QString TextProcessor::stripHtml(const QString &input)
{
    QString s = input;
    s.remove(QRegularExpression(QStringLiteral("<[^>]*>")));
    return s;
}

QString TextProcessor::capitalizeFirst(const QString &input)
{
    if (input.isEmpty()) return input;
    QString s = input;
    // Capitalizar primer char no-espacio
    for (int i = 0; i < s.size(); ++i) {
        if (!s[i].isSpace()) {
            s[i] = s[i].toUpper();
            break;
        }
    }
    return s;
}

// --- clean_0: srce (traducción) para palabras ---
// echo "${1}" |sed 's/\\n/ /g' |sed ':a;N;$!ba;s/\n/ /g' \
// |sed "s/'/'/g" | sed "s/^-\(.*\)/\1/" \
// |sed 's/ \+/ /;s/^[ \t]*//;s/[ \t]*$//;s/-$//;s/^-//' \
// |sed 's/^ *//;s/ *$//g' |sed 's/^\s*./\U&\E/g' \
// |tr -d ':*|;!¿?[]&:<>+'  |sed 's/\¡//g' \
// |sed 's/<[^>]*>//g; s/ \+/ /; s|/|-|g'
QString TextProcessor::cleanSource(const QString &input)
{
    if (input.isEmpty()) return {};

    QString s = joinLines(input);

    // Smart quotes → straight
    s.replace(QChar(0x2018), QLatin1Char('\''));
    s.replace(QChar(0x2019), QLatin1Char('\''));
    s.replace(QChar(0x201C), QLatin1Char('"'));
    s.replace(QChar(0x201D), QLatin1Char('"'));

    // Strip HTML FIRST (antes de char removal)
    s = stripHtml(s);

    // Eliminar guión inicial
    if (s.startsWith(QLatin1Char('-'))) s = s.mid(1);

    // Normalizar espacios
    s = normalizeSpaces(s);

    // Eliminar trailing dash
    if (s.endsWith(QLatin1Char('-'))) s.chop(1);

    // Eliminar chars prohibidos: :*|;!¿?[]&:<>+
    s.remove(QLatin1Char(':'));
    s.remove(QLatin1Char('*'));
    s.remove(QLatin1Char('|'));
    s.remove(QLatin1Char(';'));
    s.remove(QLatin1Char('!'));
    s.remove(QChar(0x00BF));
    s.remove(QChar(0x00A1));
    s.remove(QLatin1Char('?'));
    s.remove(QLatin1Char('['));
    s.remove(QLatin1Char(']'));
    s.remove(QLatin1Char('&'));
    s.remove(QLatin1Char('<'));
    s.remove(QLatin1Char('>'));
    s.remove(QLatin1Char('+'));

    // Capitalizar DESPUÉS de todo
    s = capitalizeFirst(s);

    // Normalizar espacios
    s = normalizeSpaces(s);

    // / → -
    s.replace(QLatin1Char('/'), QLatin1Char('-'));

    return s.trimmed();
}

// --- clean_1: trgt (target) para palabras ---
// Similar a clean_0 pero también elimina ",(),.
QString TextProcessor::cleanWordTarget(const QString &input)
{
    if (input.isEmpty()) return {};

    QString s = joinLines(input);

    // Smart quotes → straight
    s.replace(QChar(0x2018), QLatin1Char('\''));
    s.replace(QChar(0x2019), QLatin1Char('\''));
    s.replace(QChar(0x201C), QLatin1Char('"'));
    s.replace(QChar(0x201D), QLatin1Char('"'));

    // Strip HTML FIRST
    s = stripHtml(s);

    // Eliminar guión inicial
    if (s.startsWith(QLatin1Char('-'))) s = s.mid(1);

    // Normalizar espacios
    s = normalizeSpaces(s);

    // Eliminar trailing dash
    if (s.endsWith(QLatin1Char('-'))) s.chop(1);

    // Eliminar chars prohibidos (más que clean_0: añade ",().)
    s.remove(QLatin1Char('*'));
    s.remove(QLatin1Char('|'));
    s.remove(QLatin1Char('"'));
    s.remove(QLatin1Char(','));
    s.remove(QLatin1Char(';'));
    s.remove(QLatin1Char('!'));
    s.remove(QChar(0x00BF));
    s.remove(QChar(0x00A1));
    s.remove(QLatin1Char('?'));
    s.remove(QLatin1Char('('));
    s.remove(QLatin1Char(')'));
    s.remove(QLatin1Char('['));
    s.remove(QLatin1Char(']'));
    s.remove(QLatin1Char('&'));
    s.remove(QLatin1Char(':'));
    s.remove(QLatin1Char('.'));
    s.remove(QLatin1Char('<'));
    s.remove(QLatin1Char('>'));
    s.remove(QLatin1Char('+'));

    // Capitalizar DESPUÉS de todo
    s = capitalizeFirst(s);

    // Normalizar espacios
    s = normalizeSpaces(s);

    // / → -
    s.replace(QLatin1Char('/'), QLatin1Char('-'));

    return s.trimmed();
}

// --- clean_2: trgt/srce para oraciones ---
QString TextProcessor::cleanSentence(const QString &input, bool isCjk)
{
    if (input.isEmpty()) return {};

    // Eliminar trailing .- (primero)
    int cutPos = input.size();
    for (int i = input.size() - 1; i >= 0; --i) {
        const QChar c = input[i];
        if (c == QLatin1Char('.') || c == QLatin1Char('-'))
            cutPos = i;
        else
            break;
    }
    QString s = (cutPos < input.size()) ? input.left(cutPos) : input;

    // Join lines
    s = joinLines(s);

    // Smart quotes → straight
    s.replace(QChar(0x2018), QLatin1Char('\''));
    s.replace(QChar(0x2019), QLatin1Char('\''));

    // quot; → "
    s.replace(QStringLiteral("quot;"), QStringLiteral("\""));

    // Strip HTML FIRST
    s = stripHtml(s);

    if (isCjk) {
        // CJK: eliminan *, otros normalizan
        s.remove(QLatin1Char('*'));
        // Normalizar tokens especiales a espacio
        for (const QChar c : QStringLiteral("&|{}[]<>+"))
            s.replace(c, QLatin1Char(' '));
    } else {
        // Occidental: normalizan tokens especiales
        for (const QChar c : QStringLiteral("*&|{}[]<>+"))
            s.replace(c, QLatin1Char(' '));
    }

    // Normalizar espacios
    s = normalizeSpaces(s);

    // Capitalizar primera letra
    s = capitalizeFirst(s);

    // Strip HTML
    s = stripHtml(s);

    // Capitalizar de nuevo post-HTML
    s = capitalizeFirst(s);

    return s.trimmed();
}

// --- clean_3: nombre de topic ---
QString TextProcessor::cleanTopicName(const QString &input)
{
    if (input.isEmpty()) return {};

    // Cortar en primer . , o -
    int cutPos = input.size();
    for (int i = 0; i < input.size(); ++i) {
        const QChar c = input[i];
        if (c == QLatin1Char(',') || c == QLatin1Char('.') || c == QLatin1Char('-')) {
            cutPos = i;
            break;
        }
    }
    QString s = (cutPos < input.size()) ? input.left(cutPos) : input;

    // Tomar solo antes del |
    if (s.contains(QLatin1Char('|')))
        s = s.section(QLatin1Char('|'), 0, 0);

    // Eliminar !, &, :
    s.remove(QLatin1Char('!'));
    s.remove(QLatin1Char('&'));
    s.remove(QLatin1Char(':'));

    // Normalizar espacios, smart quotes
    s = normalizeSpaces(s);
    s.replace(QChar(0x2018), QLatin1Char('\''));
    s.replace(QChar(0x2019), QLatin1Char('\''));

    // Escapar / → \/
    s.replace(QLatin1Char('/'), QStringLiteral("\\/"));

    // Capitalizar primera letra
    s = capitalizeFirst(s);

    // Eliminar ：(fullwidth colon)
    s.remove(QChar(0xFF1A));

    // Strip HTML
    s = stripHtml(s);

    // Eliminar ?. * {} []
    s.remove(QLatin1Char('?'));
    s.remove(QLatin1Char('.'));
    s.remove(QLatin1Char('*'));
    s.remove(QLatin1Char('{'));
    s.remove(QLatin1Char('}'));
    s.remove(QLatin1Char('['));
    s.remove(QLatin1Char(']'));

    // Normalizar tokens a espacio
    for (const QChar c : QStringLiteral("&:|<>+"))
        s.replace(c, QLatin1Char(' '));

    // Normalizar espacios
    s = normalizeSpaces(s);

    return s.trimmed();
}

// --- clean_4: texto raw de clipboard/OCR ---
QString TextProcessor::cleanRawText(const QString &input, bool isCjk,
                                     int sentenceChars, int sentenceLines)
{
    if (input.isEmpty()) return {};

    int charCount = input.size();
    int lineCount = input.count(QLatin1Char('\n')) + 1;

    if (charCount <= sentenceChars && lineCount > sentenceLines) {
        // Texto corto pero multi-línea: unir líneas
        QString s = input;
        if (s.startsWith(QLatin1Char('-'))) s = s.mid(1);
        s.remove(QLatin1Char('*'));
        for (const QChar c : QStringLiteral("&|{}[]<>+"))
            s.replace(c, QLatin1Char(' '));
        s.replace(QStringLiteral(" \u2014 "), QStringLiteral(" - "));
        s.replace(QStringLiteral("--"), QStringLiteral(" "));
        s.remove(QChar(0x02BA));  // ʺ
        s.remove(QChar(0x0376));  // Ͷ
        s.replace(QLatin1Char('\t'), QLatin1Char(' '));
        s = normalizeSpaces(s);
        return s.trimmed();
    } else if (charCount <= sentenceChars) {
        // Texto corto: unir en una línea
        QString s = joinLines(input);
        if (s.startsWith(QLatin1Char('-'))) s = s.mid(1);
        s.remove(QLatin1Char('*'));
        for (const QChar c : QStringLiteral("&|{}[]<>+"))
            s.replace(c, QLatin1Char(' '));
        s.replace(QStringLiteral(" \u2014 "), QStringLiteral(" - "));
        s.replace(QStringLiteral("--"), QStringLiteral(" "));
        s.remove(QChar(0x02BA));
        s.remove(QChar(0x0376));
        s.replace(QLatin1Char('\t'), QLatin1Char(' '));
        s = normalizeSpaces(s);
        return s.trimmed();
    } else {
        // Texto largo: unir con __ como separador de párrafo
        QString s = input;
        if (s.startsWith(QLatin1Char('-'))) s = s.mid(1);
        s.replace(QLatin1Char('\n'), QStringLiteral("__"));
        s.replace(QLatin1Char('\t'), QLatin1Char(' '));
        s.remove(QLatin1Char('*'));
        for (const QChar c : QStringLiteral("&|{}[]<>+"))
            s.replace(c, QLatin1Char(' '));
        s.replace(QStringLiteral(" \u2014 "), QStringLiteral("__"));
        s.replace(QStringLiteral("--"), QStringLiteral(" "));
        s.remove(QChar(0x02BA));
        s.remove(QChar(0x0376));
        s = normalizeSpaces(s);
        return s.trimmed();
    }
}

// --- clean_6: texto OCR ---
QString TextProcessor::cleanOcrText(const QString &input)
{
    if (input.isEmpty()) return {};

    QString s = input;
    s.replace(QStringLiteral("\\n"), QStringLiteral("."));

    // Eliminar líneas vacías, normalizar
    QStringList lines = s.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    s = lines.join(QLatin1Char('\n'));

    // Reemplazar — por salto de línea
    s.replace(QStringLiteral(" \u2014 "), QStringLiteral("\n"));

    // Eliminar ʺ Ͷ
    s.remove(QChar(0x02BA));
    s.remove(QChar(0x0376));

    // Normalizar espacios
    s = normalizeSpaces(s);

    // quot; → "
    s.replace(QStringLiteral("&quot;"), QStringLiteral("\""));

    // Split por oraciones: . ? ! … seguidos de mayúscula
    s.replace(QRegularExpression(QStringLiteral("\\. ([A-Z][^ ])")), QStringLiteral(".\n\\1"));
    s.replace(QRegularExpression(QStringLiteral("\\? ([A-Z][^ ])")), QStringLiteral("?\n\\1"));
    s.replace(QRegularExpression(QStringLiteral("\\! ([A-Z][^ ])")), QStringLiteral("!\n\\1"));
    s.replace(QRegularExpression(QStringLiteral("\\u2026 ([A-Z][^ ])")), QStringLiteral("\u2026\n\\1"));

    return s.trimmed();
}

// --- clean_7: CJK/Rusiano → split por oraciones ---
QString TextProcessor::splitCjkSentences(const QString &input)
{
    if (input.isEmpty()) return {};

    QString s = input.trimmed();
    s = normalizeSpaces(s);
    s.replace(QLatin1Char('\t'), QLatin1Char(' '));

    // Eliminar líneas vacías
    QStringList lines = s.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    s = lines.join(QLatin1Char('\n'));

    // Reemplazar — por salto de línea
    s.replace(QStringLiteral(" \u2014 "), QStringLiteral("\n"));

    // quot; → "  ,  &#039; → '
    s.replace(QStringLiteral("&quot;"), QStringLiteral("\""));
    s.replace(QStringLiteral("&#039;"), QStringLiteral("'"));

    // Strip HTML (iterativo)
    QRegularExpression htmlRe(QStringLiteral("<[^>]*>"));
    while (s.contains(htmlRe))
        s.remove(htmlRe);
    s.remove(QChar(0x00A3));  // £
    s.remove(QChar(0x00A7));  // §
    s.replace(QStringLiteral("&amp;"), QStringLiteral("&"));

    // Split: coma, 。, __ → salto de línea
    s.replace(QLatin1Char(','), QLatin1Char('\n'));
    s.replace(QChar(0x3002), QLatin1Char('\n'));  // 。
    s.replace(QStringLiteral("__"), QStringLiteral("\n"));

    // Normalizar espacios
    s = normalizeSpaces(s);

    return s.trimmed();
}

// --- clean_8: occidental → split por oraciones ---
QString TextProcessor::splitWesternSentences(const QString &input)
{
    if (input.isEmpty()) return {};

    QString s = input;
    // Eliminar [ ... ]
    s.remove(QRegularExpression(QStringLiteral("\\[ \\.\\.\\. \\]")));
    s.replace(QLatin1Char('\t'), QLatin1Char(' '));

    // Normalizar, strip HTML
    s = normalizeSpaces(s);
    s.replace(QStringLiteral("&quot;"), QStringLiteral("\""));
    s.replace(QStringLiteral("&#039;"), QStringLiteral("'"));

    QRegularExpression htmlRe(QStringLiteral("<[^>]*>"));
    while (s.contains(htmlRe))
        s.remove(htmlRe);
    s.remove(QChar(0x00A3));  // £
    s.remove(QChar(0x00A7));  // §
    s.replace(QStringLiteral("&amp;"), QStringLiteral("&"));

    // Split por oraciones: . ? ! … seguidos de mayúscula
    s.replace(QRegularExpression(QStringLiteral("\\. ([A-Z][^ ])")), QStringLiteral(".\n\\1"));
    s.replace(QStringLiteral(". "), QStringLiteral(" "));

    s.replace(QRegularExpression(QStringLiteral("\\? ([A-Z][^ ])")), QStringLiteral("?\\n\\1"));
    s.replace(QStringLiteral("? "), QStringLiteral(" "));

    s.replace(QRegularExpression(QStringLiteral("\\! ([A-Z][^ ])")), QStringLiteral("!\\n\\1"));
    s.replace(QStringLiteral("! "), QStringLiteral(" "));

    s.replace(QRegularExpression(QStringLiteral("\\u2026 ([A-Z][^ ])")), QStringLiteral("\u2026\\n\\1"));
    s.replace(QStringLiteral("\u2026 "), QStringLiteral(" "));

    // __ → salto de línea
    s.replace(QStringLiteral("__"), QStringLiteral("\n"));

    // Normalizar espacios
    s = normalizeSpaces(s);

    return s.trimmed();
}

// --- clean_9: texto editado/modificado ---
QString TextProcessor::cleanEditedText(const QString &input)
{
    if (input.isEmpty()) return {};

    // Cortar en primer , . -
    int cutPos = input.size();
    for (int i = 0; i < input.size(); ++i) {
        const QChar c = input[i];
        if (c == QLatin1Char(',') || c == QLatin1Char('.') || c == QLatin1Char('-')) {
            cutPos = i;
            break;
        }
    }
    QString s = (cutPos < input.size()) ? input.left(cutPos) : input;

    // Join lines
    s = joinLines(s);

    // Smart quotes
    s.replace(QChar(0x2018), QLatin1Char('\''));
    s.replace(QChar(0x2019), QLatin1Char('\''));

    // Strip HTML FIRST
    s = stripHtml(s);

    // Normalizar espacios
    s = normalizeSpaces(s);

    // Eliminar trailing dash
    if (s.endsWith(QLatin1Char('-'))) s.chop(1);

    // Capitalizar primera letra
    s = capitalizeFirst(s);

    // Eliminar chars prohibidos: *|[]&<>+
    s.remove(QLatin1Char('*'));
    s.remove(QLatin1Char('|'));
    s.remove(QLatin1Char('['));
    s.remove(QLatin1Char(']'));
    s.remove(QLatin1Char('&'));
    s.remove(QLatin1Char('<'));
    s.remove(QLatin1Char('>'));
    s.remove(QLatin1Char('+'));

    // Strip HTML
    s = stripHtml(s);

    // Normalizar espacios
    s = normalizeSpaces(s);

    return s.trimmed();
}

} // namespace services
} // namespace idiomind
