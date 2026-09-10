// F5 - ITranslationProvider: interfaz para traducción de texto.
#pragma once
#include <QString>
#include <QStringList>

namespace idiomind {
namespace services {

struct TranslationResult {
    QString text;
    bool ok = false;
};

class ITranslationProvider {
public:
    virtual ~ITranslationProvider() = default;
    virtual TranslationResult translate(const QString &text,
                                        const QString &srcLang,
                                        const QString &dstLang) = 0;
};

} // namespace services
} // namespace idiomind
