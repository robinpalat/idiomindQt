#include "services/impl/GoogleTranslateProvider.h"

#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>

namespace idiomind {
namespace services {

GoogleTranslateProvider::GoogleTranslateProvider(QObject *parent)
    : QObject(parent), m_nam(new QNetworkAccessManager(this))
{
}

GoogleTranslateProvider::~GoogleTranslateProvider() = default;

TranslationResult GoogleTranslateProvider::translate(const QString &text,
                                                      const QString &srcLang,
                                                      const QString &dstLang)
{
    // Endpoint exacto del Bash:
    // https://translate.googleapis.com/translate_a/single?client=dict-chrome-ex&sl=$2&tl=$3&dt=t&q=$1
    QUrl url(QStringLiteral("https://translate.googleapis.com/translate_a/single"));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("client"), QStringLiteral("dict-chrome-ex"));
    query.addQueryItem(QStringLiteral("sl"), srcLang);
    query.addQueryItem(QStringLiteral("tl"), dstLang);
    query.addQueryItem(QStringLiteral("dt"), QStringLiteral("t"));
    query.addQueryItem(QStringLiteral("q"), text);
    url.setQuery(query);

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/x-www-form-urlencoded"));

    QNetworkReply *reply = m_nam->get(request);
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QTimer::singleShot(10000, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        reply->deleteLater();
        return {};
    }

    const QByteArray response = reply->readAll();
    reply->deleteLater();

    const QString translated = parseResponse(response, dstLang);
    if (translated.isEmpty())
        return {};

    return {translated, true};
}

QString GoogleTranslateProvider::parseResponse(const QByteArray &response,
                                                const QString &dstLang) const
{
    // El Bash parsea la respuesta de forma diferente para CJK/Russian vs europeo:
    //
    // CJK/Russian (ja, zh-cn, ru):
    //   grep -oP '(?<=trgt=\").*(?=\",\")'
    //   sedimentando [[\[] y ],[] como separadores
    //
    // Europeo (de, en, es, fr, it, pt, vi):
    //   Igual pero con awk -F'null' '{print $1}'
    //
    // El formato JSON del endpoint es un array anidado:
    //   [[["translation","original",...],...],...]
    //
    // Simplificación: parseamos el JSON y extraemos el primer string del
    // primer array anidado.

    QJsonDocument doc = QJsonDocument::fromJson(response);
    if (!doc.isArray())
        return {};

    const QJsonArray outer = doc.array();
    if (outer.isEmpty())
        return {};

    // La primera fila es [[["translated","original",...],...],...]
    const QJsonArray firstRow = outer[0].toArray();
    if (firstRow.isEmpty())
        return {};

    // Cada elemento es [["translated","original",...], source_offset, length]
    const QJsonArray firstTranslation = firstRow[0].toArray();
    if (firstTranslation.isEmpty())
        return {};

    QString result;
    for (const QJsonValue &segment : firstTranslation) {
        const QJsonArray seg = segment.toArray();
        if (seg.size() >= 1 && seg[0].isString()) {
            result += seg[0].toString();
        }
    }

    return result;
}

} // namespace services
} // namespace idiomind
