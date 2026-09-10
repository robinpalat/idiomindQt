// F5 - Tests: TranslationService, GrammarColorizer, ImageProcessor, AddNoteFlow
#include <QCoreApplication>
#include <QDate>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QTemporaryDir>
#include <QtTest/QtTest>

#include "core/config/AppPaths.h"
#include "core/topic/Item.h"
#include "services/impl/TranslationService.h"
#include "services/impl/TtsPipelineService.h"
#include "services/impl/GrammarColorizer.h"
#include "services/impl/ImageProcessor.h"
#include "services/impl/AddNoteFlow.h"

using namespace idiomind::core;
using namespace idiomind::services;

// --- Mock translation provider ---
class MockTranslationProvider : public ITranslationProvider {
public:
    TranslationResult translate(const QString &text, const QString &srcLang,
                                const QString &dstLang) override {
        Q_UNUSED(srcLang); Q_UNUSED(dstLang);
        // Simple mock: prepend "TR:" to text
        return {QStringLiteral("TR:%1").arg(text), true};
    }
};

class tst_services : public QObject {
    Q_OBJECT
private slots:
    void translationServiceBasic();
    void translationServiceBatch();
    void grammarColorizerEmptyDict();
    void imageProcessorResize();
    void addNoteFlowClean();
    void addNoteFlowProcessWord();
    void addNoteFlowProcessSentence();
    void ttsPipelineCache();
};

// --- TranslationService ---

void tst_services::translationServiceBasic()
{
    MockTranslationProvider provider;
    TranslationService svc(&provider);
    auto r = svc.translate("hello", "en", "es");
    QVERIFY(r.ok);
    QCOMPARE(r.text, QStringLiteral("TR:hello"));
}

void tst_services::translationServiceBatch()
{
    MockTranslationProvider provider;
    TranslationService svc(&provider);
    QStringList words = {"hello", "world"};
    auto results = svc.translateBatch(words, "en", "es");
    QCOMPARE(results.size(), 2);
    QCOMPARE(results[0], QStringLiteral("TR:hello"));
    QCOMPARE(results[1], QStringLiteral("TR:world"));
}

// --- GrammarColorizer ---

void tst_services::grammarColorizerEmptyDict()
{
    GrammarColorizer gz;
    // Sin dict path válido → sin colores, solo tokens
    QTemporaryDir h; QVERIFY(h.isValid());
    const QString dictPath = h.path() + "/empty.db";
    const QString result = gz.colorize("hello world", dictPath);
    QCOMPARE(result, QStringLiteral("hello world"));
}

// --- ImageProcessor ---

void tst_services::imageProcessorResize()
{
    QTemporaryDir h; QVERIFY(h.isValid());
    ImageProcessor ip;

    QImage img(800, 600, QImage::Format_RGB32);
    img.fill(Qt::blue);

    const QString outPath = h.path() + "/card.jpg";
    QVERIFY(ip.resizeToCard(img, outPath));
    QVERIFY(QFileInfo::exists(outPath));

    QImage result(outPath);
    QCOMPARE(result.width(), 400);
    QCOMPARE(result.height(), 270);
}

// --- AddNoteFlow ---

void tst_services::addNoteFlowClean()
{
    // Test cleanText (replica clean_0 del Bash)
    QCOMPARE(AddNoteFlow::cleanText("  hello  "), QStringLiteral("Hello"));
    // HTML tags: capitalize first non-space char (<), then remove tags → "test"
    QCOMPARE(AddNoteFlow::cleanText("<b>test</b>"), QStringLiteral("test"));
    QCOMPARE(AddNoteFlow::cleanText("hello   world"), QStringLiteral("Hello world"));
    QCOMPARE(AddNoteFlow::cleanText(""), QStringLiteral(""));
    QCOMPARE(AddNoteFlow::cleanText("   "), QStringLiteral(""));
    QCOMPARE(AddNoteFlow::cleanText("test:*|"), QStringLiteral("Test"));
    QCOMPARE(AddNoteFlow::cleanText("-hello"), QStringLiteral("Hello"));
    QCOMPARE(AddNoteFlow::cleanText("hello-"), QStringLiteral("Hello"));
    QCOMPARE(AddNoteFlow::cleanText("a/b"), QStringLiteral("A-b"));
}

void tst_services::addNoteFlowProcessWord()
{
    QTemporaryDir h; QVERIFY(h.isValid());
    AppPaths p(h.path(), h.path());
    MockTranslationProvider provider;
    TranslationService transSvc(&provider);
    AddNoteFlow flow(p, &transSvc);

    QDir().mkpath(p.tlngDb("English").left(p.tlngDb("English").lastIndexOf('/')));
    auto result = flow.process("hello", "English", "TestTopic");
    QVERIFY(result.ok);
    QCOMPARE(result.item.trgt, QStringLiteral("Hello"));
    QCOMPARE(result.item.type, QStringLiteral("1"));
    QVERIFY(!result.item.srce.isEmpty());
    QVERIFY(!result.item.cdid.isEmpty());
}

void tst_services::addNoteFlowProcessSentence()
{
    QTemporaryDir h; QVERIFY(h.isValid());
    AppPaths p(h.path(), h.path());
    MockTranslationProvider provider;
    TranslationService transSvc(&provider);
    AddNoteFlow flow(p, &transSvc);

    QDir().mkpath(p.tlngDb("English").left(p.tlngDb("English").lastIndexOf('/')));
    auto result = flow.process("good morning", "English", "TestTopic");
    QVERIFY(result.ok);
    QCOMPARE(result.item.trgt, QStringLiteral("Good morning"));
    QCOMPARE(result.item.type, QStringLiteral("2"));
    QVERIFY(!result.item.srce.isEmpty());
}

// --- TtsPipelineService ---

void tst_services::ttsPipelineCache()
{
    QTemporaryDir h; QVERIFY(h.isValid());
    AppPaths p(h.path(), h.path());
    TtsPipelineService tts(p);

    // Sin providers → ensureWord devuelve false
    QVERIFY(!tts.ensureWord("hello", "English"));

    // Crear directorio de audio y fichero cache manual
    const QString audioDir = p.sharedDirForLanguage("English") + "/audio";
    QDir().mkpath(audioDir);
    QFile cache(audioDir + "/hello.mp3");
    QVERIFY(cache.open(QIODevice::WriteOnly));
    cache.write("fake audio");
    cache.close();

    // Con fichero cache → ensureWord devuelve true (skip)
    QVERIFY(tts.ensureWord("hello", "English"));
}

QTEST_MAIN(tst_services)
#include "tst_services.moc"
