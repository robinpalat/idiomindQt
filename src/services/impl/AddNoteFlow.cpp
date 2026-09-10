#include "services/impl/AddNoteFlow.h"

#include "core/topic/FlatItemCodec.h"
#include "storage/dbs/SqliteDatabase.h"
#include "storage/topic/TopicDataRepository.h"

#include <QDate>
#include <QDir>
#include <QRegularExpression>
#include <QUuid>

using namespace idiomind::core;

namespace idiomind {
namespace services {

AddNoteFlow::AddNoteFlow(core::AppPaths paths,
                          TranslationService *transSvc,
                          QObject *parent)
    : QObject(parent), m_paths(std::move(paths)), m_transSvc(transSvc)
{
}

AddNoteResult AddNoteFlow::process(const QString &text, const QString &tlng,
                                    const QString &topicName)
{
    AddNoteResult result;

    // clean (replica clean_0..clean_9 del Bash)
    const QString cleaned = cleanText(text.trimmed());
    if (cleaned.isEmpty()) {
        result.error = QStringLiteral("empty after clean");
        return result;
    }

    // Determinar si es palabra o frase (replica check_s del Bash)
    const bool isWord = !cleaned.contains(QLatin1Char(' ')) &&
                        cleaned.size() <= 50;

    Item item;
    item.trgt = cleaned;
    item.cdid = QUuid::createUuid().toString().remove(QLatin1Char('{'))
                    .remove(QLatin1Char('}')).remove(QLatin1Char('-'));
    item.type = isWord ? QStringLiteral("1") : QStringLiteral("2");

    // Translate
    if (m_transSvc) {
        // slng = idioma del usuario (from tlngdb config)
        // Simplificación: usamos "Spanish" como slng por defecto
        const QString srcLang = QStringLiteral("Spanish");
        const auto r = m_transSvc->translate(cleaned, tlng, srcLang);
        if (r.ok)
            item.srce = r.text;
    }

    if (item.srce.isEmpty()) {
        item.srce = cleaned;  // fallback
    }

    // Sentence partition: grammar colorize + word pairs
    if (!isWord) {
        QString grmr, wrds;
        sentencePartition(item.trgt, item.srce, tlng, topicName, &grmr, &wrds);
        item.grmr = grmr;
        item.wrds = wrds;
    } else {
        // word_p: translate word
        QString srceWord;
        wordPartition(item.trgt, tlng, topicName, &srceWord);
        if (!srceWord.isEmpty())
            item.srce = srceWord;
    }

    result.item = item;
    result.ok = true;
    return result;
}

QString AddNoteFlow::cleanText(const QString &input)
{
    // Replica exacta de clean_0 (add.sh:261-268):
    // echo "${1}" |sed 's/\\n/ /g' |sed ':a;N;$!ba;s/\n/ /g' \
    // |sed "s/'/'/g" | sed "s/^-\(.*\)/\1/" \
    // |sed 's/ \+/ /;s/^[ \t]*//;s/[ \t]*$//;s/-$//;s/^-//' \
    // |sed 's/^ *//;s/ *$//g' |sed 's/^\s*./\U&\E/g' \
    // |tr -d ':*|;!¿?[]&:<>+'  |sed 's/\¡//g' \
    // |sed 's/<[^>]*>//g; s/ \+/ /; s|/|-|g'

    if (input.isEmpty())
        return {};

    QString s = input;

    // 1. Reemplazar \ literal por espacio
    s.replace(QStringLiteral("\\n"), QStringLiteral(" "));

    // 2. Unir líneas (reemplazar saltos de línea por espacio)
    s.replace(QLatin1Char('\n'), QLatin1Char(' '));

    // 3. Smart quotes → straight quotes
    s.replace(QChar(0x2018), QLatin1Char('\''));  // '
    s.replace(QChar(0x2019), QLatin1Char('\''));  // '
    s.replace(QChar(0x201C), QLatin1Char('"'));   // "
    s.replace(QChar(0x201D), QLatin1Char('"'));   // "

    // 4. Eliminar guión inicial: "^-(.*)" → "\1"
    if (s.startsWith(QLatin1Char('-')))
        s = s.mid(1);

    // 5. Normalizar espacios
    s.replace(QRegularExpression(QStringLiteral("\\s+")), QStringLiteral(" "));

    // 6. Trim
    s = s.trimmed();

    // 7. Eliminar guión final
    if (s.endsWith(QLatin1Char('-')))
        s.chop(1);

    // 8. Capitalizar primera letra
    if (!s.isEmpty())
        s[0] = s[0].toUpper();

    // 9. Eliminar HTML tags (antes de chars prohibidos)
    s.remove(QRegularExpression(QStringLiteral("<[^>]*>")));

    // 10. Eliminar chars prohibidos (replica tr -d ':*|;!¿?[]&:<>+')
    s.remove(QLatin1Char(':'));
    s.remove(QLatin1Char('*'));
    s.remove(QLatin1Char('|'));
    s.remove(QLatin1Char(';'));
    s.remove(QLatin1Char('!'));
    s.remove(QChar(0x00BF));  // ¿
    s.remove(QChar(0x00A1));  // ¡
    s.remove(QLatin1Char('['));
    s.remove(QLatin1Char(']'));
    s.remove(QLatin1Char('&'));
    s.remove(QLatin1Char('<'));
    s.remove(QLatin1Char('>'));
    s.remove(QLatin1Char('+'));

    // 11. Normalizar espacios otra vez
    s.replace(QRegularExpression(QStringLiteral("\\s+")), QStringLiteral(" "));

    // 13. Reemplazar / por -
    s.replace(QLatin1Char('/'), QLatin1Char('-'));

    return s.trimmed();
}

void AddNoteFlow::sentencePartition(const QString &trgt, const QString &srce,
                                     const QString &tlng, const QString &topicName,
                                     QString *grmr, QString *wrds)
{
    Q_UNUSED(topicName);
    // Tokenize target sentence
    const QStringList targetWords = trgt.split(QLatin1Char(' '), Qt::SkipEmptyParts);

    // Tokenize source sentence
    const QStringList sourceWords = srce.split(QLatin1Char(' '), Qt::SkipEmptyParts);

    // Grammar colorize (replica sentence_p Python coloring)
    const QString dictPath = QStringLiteral("/usr/share/idiomind/default/dicts/") + tlng;
    GrammarColorizer colorizer;
    *grmr = colorizer.colorize(trgt, dictPath);

    // Word pairs: target_source joined by '_'
    QStringList pairs;
    const int count = qMin(targetWords.size(), sourceWords.size());
    for (int i = 0; i < count; ++i) {
        const QString t = targetWords[i];
        const QString s = sourceWords[i];
        if (!t.isEmpty() && !s.isEmpty()) {
            pairs.append(t + QLatin1Char('_') + s);
            saveWordPair(t, s, trgt, tlng);
        }
    }
    *wrds = pairs.join(QLatin1Char('_'));
}

void AddNoteFlow::wordPartition(const QString &trgt, const QString &tlng,
                                 const QString &topicName, QString *srceOut)
{
    Q_UNUSED(topicName);
    Q_UNUSED(tlng);
    // La traducción ya se hizo en process(). Aquí solo guardamos el par.
    // El srceOut ya viene del translate.
    *srceOut = QString();
}

void AddNoteFlow::saveWordPair(const QString &word, const QString &translation,
                                const QString &example, const QString &tlng)
{
    const QString dbPath = m_paths.tlngDb(tlng);
    ::idiomind::storage::SqliteDatabase db(dbPath);
    if (!db.open())
        return;

    const QString table = QStringLiteral("Words");
    QString w = word;
    w.replace(QLatin1Char('\''), QStringLiteral("''"));
    QString t = translation;
    t.replace(QLatin1Char('\''), QStringLiteral("''"));
    QString e = example;
    e.replace(QLatin1Char('\''), QStringLiteral("''"));

    // Check if word exists
    QSqlQuery q = db.query(QStringLiteral("select Word from %1 where Word is '%2';")
                               .arg(table, w));
    if (!q.next()) {
        // Insert new word
        db.execute(QStringLiteral("insert into %1 (Word,Example) values ('%2','%3');")
                       .arg(table, w, t));
    } else {
        // Update translation if empty
        QSqlQuery q2 = db.query(QStringLiteral("select Translation from %1 where Word is '%2';")
                                    .arg(table, w));
        if (q2.next() && q2.value(0).toString().isEmpty()) {
            db.execute(QStringLiteral("update %1 set Translation='%2' where Word='%3';")
                           .arg(table, t, w));
        }
    }
    // Also insert into monthly table
    const QString monthTable = QStringLiteral("T%1").arg(
        QDate::currentDate().toString(QStringLiteral("MMyy")));
    db.execute(QStringLiteral("create table if not exists %1 (Word TEXT, Translation TEXT);")
                   .arg(monthTable));
    db.execute(QStringLiteral("insert into %1 (Word,Translation) values ('%2','%3');")
                   .arg(monthTable, w, t));
}

} // namespace services
} // namespace idiomind
