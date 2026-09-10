// F5 - GoogleTranslateProvider: traducción vía Google Translate API (dict-chrome-ex).
// Autoridad: Dev/addons/Resources/scripts/Google translate.Traslator online.Translate.various
//
// Endpoint exacto del Bash:
//   https://translate.googleapis.com/translate_a/single?client=dict-chrome-ex
//   &sl=$src&tl=$dst&dt=t&q=$text
//
// Parsing diferenciado CJK/Russian vs europeo (replica el sed/grep del Bash).
#pragma once

#include "services/interface/ITranslationProvider.h"

#include <QObject>
#include <QNetworkAccessManager>

namespace idiomind {
namespace services {

class GoogleTranslateProvider : public QObject, public ITranslationProvider
{
    Q_OBJECT
public:
    GoogleTranslateProvider(QObject *parent = nullptr);
    ~GoogleTranslateProvider() override;

    TranslationResult translate(const QString &text,
                                const QString &srcLang,
                                const QString &dstLang) override;

private:
    QNetworkAccessManager *m_nam;

    QString parseResponse(const QByteArray &response, const QString &dstLang) const;
};

} // namespace services
} // namespace idiomind
