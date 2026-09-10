#include "services/impl/TranslationService.h"

namespace idiomind {
namespace services {

TranslationService::TranslationService(ITranslationProvider *provider, QObject *parent)
    : QObject(parent), m_provider(provider)
{
}

TranslationResult TranslationService::translate(const QString &text,
                                                const QString &srcLang,
                                                const QString &dstLang)
{
    if (!m_provider || text.isEmpty())
        return {};
    return m_provider->translate(text, srcLang, dstLang);
}

QStringList TranslationService::translateBatch(const QStringList &words,
                                               const QString &srcLang,
                                               const QString &dstLang)
{
    QStringList results;
    results.reserve(words.size());
    for (const QString &w : words) {
        const auto r = translate(w, srcLang, dstLang);
        results.append(r.ok ? r.text : QString());
    }
    return results;
}

} // namespace services
} // namespace idiomind
