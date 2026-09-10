// F5 - TtsPipelineService: síntesis de voz con caché y prioridad.
// Autoridad: Dev/ifs/mods/add/add.sh tts_word(), tts_sentence(), fetch_audio().
//
// Caché: $DM_tls/audio/<word>.mp3. Prioridad: providers en orden.
#pragma once

#include "services/interface/ITtsProvider.h"
#include "core/config/AppPaths.h"

#include <QObject>
#include <QStringList>
#include <QVector>

namespace idiomind {
namespace services {

class TtsPipelineService : public QObject
{
    Q_OBJECT
public:
    explicit TtsPipelineService(core::AppPaths paths, QObject *parent = nullptr);

    void addProvider(ITtsProvider *provider);

    // Sintetiza una palabra (con caché).
    bool ensureWord(const QString &word, const QString &lang);

    // Sintetiza una frase (con caché).
    bool ensureSentence(const QString &sentence, const QString &lang);

    // Batch: lista de palabras.
    void ensureWordsFromList(const QStringList &words, const QString &lang);

private:
    core::AppPaths m_paths;
    QVector<ITtsProvider *> m_providers;

    bool synthesizeWithCache(const QString &text, const QString &lang,
                             const QString &cacheKey);
};

} // namespace services
} // namespace idiomind
