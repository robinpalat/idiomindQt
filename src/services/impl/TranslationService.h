// F5 - TranslationService: orquesta traducción de texto.
// Autoridad: Dev/ifs/mods/add/add.sh translate(), Dev/addons/Resources/scripts/Google translate.*
//
// Endpoint: translate.googleapis.com/translate_a/single?client=dict-chrome-ex
// Parsing diferenciado CJK vs europeo.
#pragma once

#include "services/interface/ITranslationProvider.h"

#include <QObject>
#include <QString>

namespace idiomind {
namespace services {

class TranslationService : public QObject
{
    Q_OBJECT
public:
    explicit TranslationService(ITranslationProvider *provider, QObject *parent = nullptr);

    // Traduce texto individual.
    TranslationResult translate(const QString &text,
                                const QString &srcLang,
                                const QString &dstLang);

    // Traduce lista de palabras (batch). Devuelve lista de traducciones.
    QStringList translateBatch(const QStringList &words,
                               const QString &srcLang,
                               const QString &dstLang);

private:
    ITranslationProvider *m_provider;
};

} // namespace services
} // namespace idiomind
