// F5.1 - Tests: TextProcessor (clean_0..clean_9)
#include <QtTest/QtTest>
#include "services/impl/TextProcessor.h"

using namespace idiomind::services;

class tst_textprocessor : public QObject
{
    Q_OBJECT
private slots:
    void cleanSource();
    void cleanWordTarget();
    void cleanSentence();
    void cleanTopicName();
    void cleanRawText();
    void cleanOcrText();
    void splitCjkSentences();
    void splitWesternSentences();
    void cleanEditedText();
    void cleanSourceEmpty();
    void cleanSourceUnicode();
    void cleanSourceSpaces();
};

// --- clean_0: srce ---
void tst_textprocessor::cleanSource()
{
    // Normal
    QCOMPARE(TextProcessor::cleanSource("hello world"), QStringLiteral("Hello world"));
    // HTML strip
    QCOMPARE(TextProcessor::cleanSource("<b>test</b>"), QStringLiteral("Test"));
    // / → -
    QCOMPARE(TextProcessor::cleanSource("a/b"), QStringLiteral("A-b"));
    // Chars prohibidos
    QCOMPARE(TextProcessor::cleanSource("test:*|!"), QStringLiteral("Test"));
    // Capitalize
    QCOMPARE(TextProcessor::cleanSource("hello"), QStringLiteral("Hello"));
    // Trailing dash
    QCOMPARE(TextProcessor::cleanSource("test-"), QStringLiteral("Test"));
    // Leading dash
    QCOMPARE(TextProcessor::cleanSource("-test"), QStringLiteral("Test"));
}

void tst_textprocessor::cleanSourceEmpty()
{
    QCOMPARE(TextProcessor::cleanSource(""), QStringLiteral(""));
    QCOMPARE(TextProcessor::cleanSource("   "), QStringLiteral(""));
}

void tst_textprocessor::cleanSourceUnicode()
{
    // Smart quotes → straight quotes (then not removed by cleanSource)
    QCOMPARE(TextProcessor::cleanSource("\u2018hello\u2019"), QStringLiteral("'hello'"));
    // ¡
    QCOMPARE(TextProcessor::cleanSource("\u00A1hola"), QStringLiteral("Hola"));
    // ¿
    QCOMPARE(TextProcessor::cleanSource("\u00BFque"), QStringLiteral("Que"));
}

void tst_textprocessor::cleanSourceSpaces()
{
    QCOMPARE(TextProcessor::cleanSource("  hello   world  "), QStringLiteral("Hello world"));
    QCOMPARE(TextProcessor::cleanSource("hello\n\nworld"), QStringLiteral("Hello world"));
}

// --- clean_1: trgt ---
void tst_textprocessor::cleanWordTarget()
{
    // Normal
    QCOMPARE(TextProcessor::cleanWordTarget("hello"), QStringLiteral("Hello"));
    // Quotes eliminados
    QCOMPARE(TextProcessor::cleanWordTarget("\"test\""), QStringLiteral("Test"));
    // Parentesis eliminados
    QCOMPARE(TextProcessor::cleanWordTarget("test(1)"), QStringLiteral("Test1"));
    // Punto eliminado
    QCOMPARE(TextProcessor::cleanWordTarget("test."), QStringLiteral("Test"));
    // Coma eliminada
    QCOMPARE(TextProcessor::cleanWordTarget("test,"), QStringLiteral("Test"));
}

// --- clean_2: sentence ---
void tst_textprocessor::cleanSentence()
{
    // Normal
    QCOMPARE(TextProcessor::cleanSentence("hello world"), QStringLiteral("Hello world"));
    // Trailing period eliminado
    QCOMPARE(TextProcessor::cleanSentence("hello world."), QStringLiteral("Hello world"));
    // Trailing dash eliminado
    QCOMPARE(TextProcessor::cleanSentence("hello world-"), QStringLiteral("Hello world"));
    // HTML strip
    QCOMPARE(TextProcessor::cleanSentence("<p>test</p>"), QStringLiteral("Test"));
    // CJK mode
    QCOMPARE(TextProcessor::cleanSentence("test", true), QStringLiteral("Test"));
}

// --- clean_3: topic name ---
void tst_textprocessor::cleanTopicName()
{
    // Normal
    QCOMPARE(TextProcessor::cleanTopicName("My Topic"), QStringLiteral("My Topic"));
    // Trailing dot
    QCOMPARE(TextProcessor::cleanTopicName("Topic."), QStringLiteral("Topic"));
    // Pipe handling
    QCOMPARE(TextProcessor::cleanTopicName("Topic|Extra"), QStringLiteral("Topic"));
    // HTML strip
    QCOMPARE(TextProcessor::cleanTopicName("<b>Topic</b>"), QStringLiteral("Topic"));
}

// --- clean_4: raw text ---
void tst_textprocessor::cleanRawText()
{
    // Short text, single line — no capitalize (raw text)
    QCOMPARE(TextProcessor::cleanRawText("hello"), QStringLiteral("hello"));
    // Short text, multi-line → join
    QCOMPARE(TextProcessor::cleanRawText("hello\nworld"), QStringLiteral("hello world"));
    // Long text → keep __ separators
    QString longText(200, QLatin1Char('a'));
    QString result = TextProcessor::cleanRawText(longText, false, 180, 2);
    QVERIFY(!result.isEmpty());
}

// --- clean_6: OCR ---
void tst_textprocessor::cleanOcrText()
{
    // Simple text
    QCOMPARE(TextProcessor::cleanOcrText("hello world"), QStringLiteral("hello world"));
    // Sentence split on . followed by capital
    auto result = TextProcessor::cleanOcrText("test. Next sentence.");
    QVERIFY(result.contains(QChar('\n')));
}

// --- clean_7: CJK ---
void tst_textprocessor::splitCjkSentences()
{
    // Comma split
    auto result = TextProcessor::splitCjkSentences("a,b,c");
    QCOMPARE(result.count(QChar('\n')), 2);  // 2 newlines → 3 parts
    // 。split
    result = TextProcessor::splitCjkSentences("a。b");
    QCOMPARE(result.count(QChar('\n')), 1);
}

// --- clean_8: Western ---
void tst_textprocessor::splitWesternSentences()
{
    // Period split
    auto result = TextProcessor::splitWesternSentences("Hello. World.");
    QCOMPARE(result.count(QChar('\n')), 1);  // 1 newline → 2 parts
    // __ split
    result = TextProcessor::splitWesternSentences("a__b");
    QCOMPARE(result.count(QChar('\n')), 1);
}

// --- clean_9: edited text ---
void tst_textprocessor::cleanEditedText()
{
    QCOMPARE(TextProcessor::cleanEditedText("hello"), QStringLiteral("Hello"));
    // HTML strip
    QCOMPARE(TextProcessor::cleanEditedText("<b>test</b>"), QStringLiteral("Test"));
    // Chars prohibidos
    QCOMPARE(TextProcessor::cleanEditedText("test*|[]&<>+"), QStringLiteral("Test"));
}

QTEST_MAIN(tst_textprocessor)
#include "tst_textprocessor.moc"
