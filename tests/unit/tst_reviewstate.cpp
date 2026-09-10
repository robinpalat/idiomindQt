// F2.1 - ReviewState: contrato numérico de la máquina de estados.
#include <QtTest/QtTest>

#include "core/learning/ReviewState.h"

class TstReviewState : public QObject {
    Q_OBJECT
private slots:
    void valuesAreContract();
    void validity();
    void meanings();
    void evenOdd();
};

void TstReviewState::valuesAreContract()
{
    // docs/06-learning-system.md: los valores numéricos NO se renumeran.
    QCOMPARE(static_cast<int>(ReviewState::Paused), 0);
    QCOMPARE(static_cast<int>(ReviewState::Learning), 1);
    QCOMPARE(static_cast<int>(ReviewState::Mastered), 2);
    QCOMPARE(static_cast<int>(ReviewState::Waiting), 3);
    QCOMPARE(static_cast<int>(ReviewState::WaitingAlternate), 4);
    QCOMPARE(static_cast<int>(ReviewState::MidReview), 5);
    QCOMPARE(static_cast<int>(ReviewState::MidReviewAlternate), 6);
    QCOMPARE(static_cast<int>(ReviewState::Ready100Odd), 7);
    QCOMPARE(static_cast<int>(ReviewState::Ready100Even), 8);
    QCOMPARE(static_cast<int>(ReviewState::Ready150Odd), 9);
    QCOMPARE(static_cast<int>(ReviewState::Ready150Even), 10);
    QCOMPARE(static_cast<int>(ReviewState::Corrupted), 13);
}

void TstReviewState::validity()
{
    for (int v = 0; v <= 10; ++v)
        QVERIFY2(ReviewStateHelpers::isValid(v), qPrintable(QString::number(v)));
    QVERIFY(ReviewStateHelpers::isValid(13));
    for (int v : {11, 12, 14, 15, -1, 100})
        QVERIFY2(!ReviewStateHelpers::isValid(v), qPrintable(QString::number(v)));
}

void TstReviewState::meanings()
{
    // significados textuales de la tabla de docs/06
    const QList<QPair<ReviewState, QString>> expected = {
        {ReviewState::Paused, QStringLiteral("paused / disabled")},
        {ReviewState::Learning, QStringLiteral("learning (active)")},
        {ReviewState::Mastered, QStringLiteral("mastered")},
        {ReviewState::Waiting, QStringLiteral("waiting (review due)")},
        {ReviewState::WaitingAlternate,
         QStringLiteral("waiting (review due, alternate)")},
        {ReviewState::MidReview,
         QStringLiteral("mid-review / \"Finalize review\" (readd topics)")},
        {ReviewState::MidReviewAlternate, QStringLiteral("mid-review (alternate)")},
        {ReviewState::Ready100Odd,
         QStringLiteral("ready for review (after 100% first interval)")},
        {ReviewState::Ready100Even, QStringLiteral("ready for review (alternate)")},
        {ReviewState::Ready150Odd, QStringLiteral("ready for review (after 150%)")},
        {ReviewState::Ready150Even, QStringLiteral("ready for review (alternate)")},
        {ReviewState::Corrupted,
         QStringLiteral("corrupted / addon topic without valid stts (auto-repair -> 1)")},
    };
    for (const auto &e : expected)
        QCOMPARE(ReviewStateHelpers::meaning(e.first), e.second);
}

void TstReviewState::evenOdd()
{
    QVERIFY(ReviewStateHelpers::isEven(ReviewState::Paused));               // 0
    QVERIFY(!ReviewStateHelpers::isEven(ReviewState::Learning));            // 1
    QVERIFY(ReviewStateHelpers::isEven(ReviewState::Mastered));             // 2
    QVERIFY(!ReviewStateHelpers::isEven(ReviewState::Waiting));             // 3
    QVERIFY(ReviewStateHelpers::isEven(ReviewState::WaitingAlternate));     // 4
    QVERIFY(!ReviewStateHelpers::isEven(ReviewState::MidReview));           // 5
    QVERIFY(ReviewStateHelpers::isEven(ReviewState::MidReviewAlternate));   // 6
    QVERIFY(!ReviewStateHelpers::isEven(ReviewState::Ready100Odd));         // 7
    QVERIFY(ReviewStateHelpers::isEven(ReviewState::Ready100Even));         // 8
    QVERIFY(!ReviewStateHelpers::isEven(ReviewState::Ready150Odd));         // 9
    QVERIFY(ReviewStateHelpers::isEven(ReviewState::Ready150Even));         // 10
    QVERIFY(!ReviewStateHelpers::isEven(ReviewState::Corrupted));           // 13
}

QTEST_GUILESS_MAIN(TstReviewState)
#include "tst_reviewstate.moc"