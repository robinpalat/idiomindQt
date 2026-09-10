#include "services/impl/GrammarColorizer.h"

#include "storage/dbs/SqliteDatabase.h"

#include <QSqlQuery>
#include <QRegularExpression>

namespace idiomind {
namespace services {

// Colores exactos del Bash (sentence_p):
// pronouns=#3E539A, nouns_adjetives=#496E60, nouns_verbs=#62426A,
// conjunctions=#90B33B, prepositions=#D67B2D, adverbs=#9C68BD,
// adjetives=#3E8A3B, verbs=#CF387F
static const QMap<QString, QString> kTableColors = {
    {QStringLiteral("pronouns"), QStringLiteral("#3E539A")},
    {QStringLiteral("nouns_adjetives"), QStringLiteral("#496E60")},
    {QStringLiteral("nouns_verbs"), QStringLiteral("#62426A")},
    {QStringLiteral("conjunctions"), QStringLiteral("#90B33B")},
    {QStringLiteral("prepositions"), QStringLiteral("#D67B2D")},
    {QStringLiteral("adverbs"), QStringLiteral("#9C68BD")},
    {QStringLiteral("adjetives"), QStringLiteral("#3E8A3B")},
    {QStringLiteral("verbs"), QStringLiteral("#CF387F")},
};

static const QStringList kTableOrder = {
    QStringLiteral("pronouns"),
    QStringLiteral("nouns_adjetives"),
    QStringLiteral("nouns_verbs"),
    QStringLiteral("conjunctions"),
    QStringLiteral("prepositions"),
    QStringLiteral("adverbs"),
    QStringLiteral("adjetives"),
    QStringLiteral("verbs"),
};

QString GrammarColorizer::colorForTable(const QString &tableName)
{
    return kTableColors.value(tableName);
}

static QString lookupColor(const QString &word, const QString &dictPath)
{
    ::idiomind::storage::SqliteDatabase db(dictPath);
    if (!db.open())
        return {};
    // Bash: `sqlite3 $db "select items from <table> where items is '${w}';"`
    // El word se busca en minúsculas, igual que el Bash (`${wrd,,}`)
    const QString lower = word.toLower();
    for (const QString &table : kTableOrder) {
        QSqlQuery q = db.query(QStringLiteral("select items from %1 where items is '%2';")
                                   .arg(table, lower));
        if (q.next())
            return kTableColors.value(table);
    }
    return {};
}

QList<Part> GrammarColorizer::colorizeParts(const QString &sentence,
                                              const QString &dictPath) const
{
    QList<Part> parts;
    const QStringList tokens = sentence.split(QLatin1Char(' '), Qt::SkipEmptyParts);
    for (const QString &token : tokens) {
        const QString color = lookupColor(token, dictPath);
        parts.append({token, color});
    }
    return parts;
}

QString GrammarColorizer::colorize(const QString &sentence,
                                    const QString &dictPath) const
{
    const QList<Part> parts = colorizeParts(sentence, dictPath);
    QString result;
    for (int i = 0; i < parts.size(); ++i) {
        if (i > 0)
            result += QLatin1Char(' ');
        if (!parts[i].color.isEmpty()) {
            result += QStringLiteral("<span color='%1'>%2</span>")
                          .arg(parts[i].color, parts[i].token);
        } else {
            result += parts[i].token;
        }
    }
    return result;
}

} // namespace services
} // namespace idiomind
