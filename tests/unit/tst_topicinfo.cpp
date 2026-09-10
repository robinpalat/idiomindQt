// F2.1 - TopicInfo: metadata serializable (docs/05, default/vars línea 3).
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QtTest/QtTest>

#include "core/topic/TopicInfo.h"

using namespace idiomind::core;

class TstTopicInfo : public QObject {
    Q_OBJECT
private slots:
    void fieldNamesOrder();
    void fromRealFixtureInfo();
    void nszeStaysString();
    void jsonRoundTrip();
    void emptyRoundTrip();
};

void TstTopicInfo::fieldNamesOrder()
{
    // tsets (sets.cfg) / vars línea 3: el orden es parte del formato
    QStringList names;
    for (int i = 0; i < TopicInfo::fieldCount(); ++i)
        names << QLatin1String(TopicInfo::fieldName(i));
    QCOMPARE(names,
             QStringList({"name", "slng", "tlng", "autr", "cntt", "ctgy",
                          "ilnk", "orig", "dtec", "dteu", "dtei", "nwrd",
                          "nsnt", "nimg", "naud", "nsze", "levl", "info",
                          "stts"}));
    QCOMPARE(TopicInfo::fieldName(-1), nullptr);
    QCOMPARE(TopicInfo::fieldName(19), nullptr);
}

void TstTopicInfo::fromRealFixtureInfo()
{
    // 3ª línea real (tiene llaves a la izquierda en el formato 3-líneas)
    const QString infoText =
        QStringLiteral("\"name\":\"I Want to Swim\",\"slng\":\"Espa\u00f1ol\","
                       "\"tlng\":\"English\",\"autr\":\"GCheerup\",\"cntt\":\"\","
                       "\"ctgy\":\"storyteller\","
                       "\"ilnk\":\"i_want_to_swim9611c7f61103fbf8d9f3\","
                       "\"orig\":\"I Want to Swim\",\"dtec\":\"2017-07-10\","
                       "\"dteu\":\"2017-07-10\",\"dtei\":\"\",\"nwrd\":\"0\","
                       "\"nsnt\":\"25\",\"nimg\":\"0\",\"naud\":\"93\","
                       "\"nsze\":\"2.2M\",\"levl\":\"1\",\"info\":\" \","
                       "\"stts\":\"0\"}");

    const TopicInfo t = TopicInfo::fromJsonText(infoText);
    QCOMPARE(t.name(), QStringLiteral("I Want to Swim"));
    QCOMPARE(t.slng(), QStringLiteral("Espa\u00f1ol"));
    QCOMPARE(t.tlng(), QStringLiteral("English"));
    QCOMPARE(t.autr(), QStringLiteral("GCheerup"));
    QCOMPARE(t.cntt(), QString());
    QCOMPARE(t.ctgy(), QStringLiteral("storyteller"));
    QCOMPARE(t.ilnk(), QStringLiteral("i_want_to_swim9611c7f61103fbf8d9f3"));
    QCOMPARE(t.orig(), QStringLiteral("I Want to Swim"));
    QCOMPARE(t.dtec(), QStringLiteral("2017-07-10"));
    QCOMPARE(t.dteu(), QStringLiteral("2017-07-10"));
    QCOMPARE(t.dtei(), QString());
    QCOMPARE(t.nwrd(), QStringLiteral("0"));
    QCOMPARE(t.nsnt(), QStringLiteral("25"));
    QCOMPARE(t.nimg(), QStringLiteral("0"));
    QCOMPARE(t.naud(), QStringLiteral("93"));
    QCOMPARE(t.nsze(), QStringLiteral("2.2M"));
    QCOMPARE(t.levl(), QStringLiteral("1"));
    QCOMPARE(t.info(), QStringLiteral(" "));
    QCOMPARE(t.stts(), QStringLiteral("0"));
}

void TstTopicInfo::nszeStaysString()
{
    // "8,6M" / "3,6M": coma decimal española; NUNCA se convierte a double
    const TopicInfo t =
        TopicInfo::fromJsonText(QStringLiteral(
            "\"nsze\":\"3,6M\",\"stts\":\"0\"}"));
    QCOMPARE(t.nsze(), QStringLiteral("3,6M"));
    QCOMPARE(t.toJson().value(QStringLiteral("nsze")).toVariant().toString(),
             QStringLiteral("3,6M"));
    QVERIFY(t.toJson().value(QStringLiteral("nsze")).isString());
    QVERIFY(!t.toJson().value(QStringLiteral("nsze")).isDouble());
}

void TstTopicInfo::jsonRoundTrip()
{
    const QString infoText =
        QStringLiteral("\"name\":\"Nazca Lines\",\"slng\":\"Espa\u00f1ol\","
                       "\"tlng\":\"English\",\"autr\":\"\",\"cntt\":\"\","
                       "\"ctgy\":\"culture\",\"ilnk\":\"nazca_lines88888888888888888888\","
                       "\"orig\":\"Nazca Lines\",\"dtec\":\"2017-04-01\","
                       "\"dteu\":\"2017-04-01\",\"dtei\":\"\",\"nwrd\":\"13\","
                       "\"nsnt\":\"22\",\"nimg\":\"12\",\"naud\":\"171\","
                       "\"nsze\":\"2.2M\",\"levl\":\"1\",\"info\":\" \","
                       "\"stts\":\"0\"}");
    const TopicInfo t = TopicInfo::fromJsonText(infoText);
    QCOMPARE(TopicInfo::fromJson(t.toJson()), t);

    // claves y orden tsets en el texto emitido (QJsonDocument reordena
    // claves; el contrato de formato exige el orden tsets exacto)
    const QString text = t.toJsonText();
    QVERIFY(text.startsWith(QStringLiteral("\"name\":\"Nazca Lines\"")));
    QVERIFY(text.contains(QStringLiteral("\"slng\":\"Espa\u00f1ol\"")));
    QVERIFY(text.endsWith(QStringLiteral("\"stts\":\"0\"")));
    QVERIFY(!text.startsWith(QChar::fromLatin1('{')));
    QVERIFY(text.indexOf(QStringLiteral("\"nimg\"")) <
            text.indexOf(QStringLiteral("\"naud\"")));
    QVERIFY(text.indexOf(QStringLiteral("\"nsze\"")) <
            text.indexOf(QStringLiteral("\"levl\"")));

    // el texto es JSON válido: envuelto entre llaves debe parsear igual
    const QJsonDocument doc = QJsonDocument::fromJson(
        (QStringLiteral("{") + text + QStringLiteral("}")).toUtf8());
    QVERIFY2(doc.isObject(), qPrintable(text));
    QCOMPARE(doc.object().size(), TopicInfo::fieldCount());
    QCOMPARE(TopicInfo::fromJson(doc.object()), t);
}

void TstTopicInfo::emptyRoundTrip()
{
    const TopicInfo t;
    QCOMPARE(TopicInfo::fromJson(t.toJson()), t);
    const QString text =
        QStringLiteral("{\"name\":\"\",\"slng\":\"\",\"tlng\":\"\",\"autr\":\"\","
                       "\"cntt\":\"\",\"ctgy\":\"\",\"ilnk\":\"\",\"orig\":\"\","
                       "\"dtec\":\"\",\"dteu\":\"\",\"dtei\":\"\",\"nwrd\":\"\","
                       "\"nsnt\":\"\",\"nimg\":\"\",\"naud\":\"\",\"nsze\":\"\","
                       "\"levl\":\"\",\"info\":\"\",\"stts\":\"\"}");
    QCOMPARE(TopicInfo::fromJsonText(text), t);
}

QTEST_GUILESS_MAIN(TstTopicInfo)
#include "tst_topicinfo.moc"