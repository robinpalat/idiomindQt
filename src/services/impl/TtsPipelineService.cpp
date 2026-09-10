#include "services/impl/TtsPipelineService.h"

#include <QDir>
#include <QFileInfo>

namespace idiomind {
namespace services {

TtsPipelineService::TtsPipelineService(core::AppPaths paths, QObject *parent)
    : QObject(parent), m_paths(std::move(paths))
{
}

void TtsPipelineService::addProvider(ITtsProvider *provider)
{
    if (provider)
        m_providers.append(provider);
}

bool TtsPipelineService::ensureWord(const QString &word, const QString &lang)
{
    return synthesizeWithCache(word, lang, word.toLower());
}

bool TtsPipelineService::ensureSentence(const QString &sentence, const QString &lang)
{
    const QString key = sentence.left(20).toLower().replace(QLatin1Char(' '), QLatin1Char('_'));
    return synthesizeWithCache(sentence, lang, key);
}

void TtsPipelineService::ensureWordsFromList(const QStringList &words, const QString &lang)
{
    for (const QString &w : words)
        ensureWord(w, lang);
}

bool TtsPipelineService::synthesizeWithCache(const QString &text, const QString &lang,
                                              const QString &cacheKey)
{
    const QString audioDir = m_paths.sharedDirForLanguage(lang) + QStringLiteral("/audio");
    QDir().mkpath(audioDir);
    const QString cachePath = audioDir + QStringLiteral("/%1.mp3").arg(cacheKey);

    // Caché: si ya existe y no está vacío, skip
    if (QFileInfo::exists(cachePath) && QFileInfo(cachePath).size() > 0)
        return true;

    // Intentar cada provider en orden
    QDir outDir(audioDir);
    for (ITtsProvider *p : m_providers) {
        QString outFile;
        if (p->synthesize(text, lang, outDir, &outFile) && !outFile.isEmpty()) {
            // Renombrar a cacheKey.mp3 si es necesario
            const QString finalPath = outDir.filePath(cacheKey + QStringLiteral(".mp3"));
            if (outFile != finalPath) {
                QFile::remove(finalPath);
                QFile::rename(outFile, finalPath);
            }
            return true;
        }
    }
    return false;
}

} // namespace services
} // namespace idiomind
