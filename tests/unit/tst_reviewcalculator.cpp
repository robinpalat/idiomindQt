// F2.1 - ReviewCalculator: notice[], porcentaje, clasificación T1..T10 y
// transiciones mark_as_learned / mark_to_learn (docs/06 + mngr.sh/main.sh).
#include <QtTest/QtTest>
#include <QTimeZone>

#include "core/learning/ReviewCalculator.h"

class TstReviewCalculator : public QObject {
    Q_OBJECT
private slots:
    void noticeTable();
    void overduePercent();
    void daysSince();
    void sqliteDateValidation();
    void classifyEven();
    void classifyOdd();
    void classifyNoOpStates();
    void markAsLearnedTable();
    void markToLearnTable();
    void mastered();
};

void TstReviewCalculator::noticeTable()
{
    const QVector<int> expected = {0, 4, 7, 7, 10, 15, 15, 20, 30, 60};
    QCOMPARE(ReviewCalculator::reviewIntervals(), expected);

    const int table[] = {0, 4, 7, 7, 10, 15, 15, 20, 30, 60};
    for (int n = 0; n < 10; ++n)
        QCOMPARE(ReviewCalculator::daysToReviewAt(n), table[n]);
    QCOMPARE(ReviewCalculator::daysToReviewAt(10), -1);   // fuera de contrato
    QCOMPARE(ReviewCalculator::daysToReviewAt(11), -1);
    QCOMPARE(ReviewCalculator::daysToReviewAt(-1), -1);
}

void TstReviewCalculator::overduePercent()
{
    // 100*TM/notice[count], división entera con truncamiento hacia cero
    QCOMPARE(ReviewCalculator::overduePercent(4, 1), 100);    // 400/4
    QCOMPARE(ReviewCalculator::overduePercent(2, 1), 50);     // 200/4
    QCOMPARE(ReviewCalculator::overduePercent(9, 1), 225);    // 900/4
    QCOMPARE(ReviewCalculator::overduePercent(7, 2), 100);    // 700/7
    QCOMPARE(ReviewCalculator::overduePercent(10, 2), 142);   // 1000/7 = 142 (trunc)
    QCOMPARE(ReviewCalculator::overduePercent(150, 6), 1000); // 15000/15
    QCOMPARE(ReviewCalculator::overduePercent(90, 9), 150);   // 9000/60
    QCOMPARE(ReviewCalculator::overduePercent(30, 9), 50);    // 3000/60
    QCOMPARE(ReviewCalculator::overduePercent(0, 5), 0);
    QCOMPARE(ReviewCalculator::overduePercent(5, 0), 0);      // count==0: indef.
    QCOMPARE(ReviewCalculator::overduePercent(-4, 1), -100);  // trunc. hacia cero
}

void TstReviewCalculator::daysSince()
{
    namespace RC = ReviewCalculator;
    const QTimeZone tz = QTimeZone::systemTimeZone();

    // La fórmula es la del bash: `TM=$(( ($(date +%s) - $(date -d "$d" +%s)) / 86400 ))`
    // con la medianoche local del día de repaso. Si entre las dos fechas hay
    // cambio de hora, el resultado puede ser 1 día menor, igual que en el
    // original (el criterio correcto es el del epoch, no el calendárico).
    const auto bashDays = [&tz](const QDate &d, const QDateTime &now) {
        const QDateTime midnight(d, QTime(0, 0), tz);
        return int((now.toSecsSinceEpoch() - midnight.toSecsSinceEpoch()) / 86400);
    };
    const QDateTime midnight20260912(QDate(2026, 9, 12), QTime(0, 0), tz);

    // 8 días exactos (sin transición de hora: el valor coincide con el bash)
    QCOMPARE(RC::daysSince(QDate(2026, 9, 4), midnight20260912),
             bashDays(QDate(2026, 9, 4), midnight20260912));
    // 8d + 16.5h -> truncado a 8 (igual que $((...)))
    QCOMPARE(RC::daysSince(QDate(2026, 9, 4),
                           QDateTime(QDate(2026, 9, 12), QTime(16, 30), tz)),
             bashDays(QDate(2026, 9, 4), QDateTime(QDate(2026, 9, 12), QTime(16, 30), tz)));
    // mismo día -> 0 en cualquier TZ (16.5h < 24h, nunca cruza una transición)
    QCOMPARE(RC::daysSince(QDate(2026, 9, 12),
                           QDateTime(QDate(2026, 9, 12), QTime(16, 30), tz)),
             0);
    // fecha en el futuro -> negativo, truncado hacia cero
    QCOMPARE(RC::daysSince(QDate(2026, 9, 12),
                           QDateTime(QDate(2026, 9, 4), QTime(0, 0), tz)),
             bashDays(QDate(2026, 9, 12), QDateTime(QDate(2026, 9, 4), QTime(0, 0), tz)));
    // inválida -> 0 (la fila estaba vacía: calculate_review ni siquiera entra)
    QCOMPARE(RC::daysSince(QDate(), midnight20260912), 0);
}

void TstReviewCalculator::sqliteDateValidation()
{
    QVERIFY(ReviewCalculator::isValidSqliteDate(QStringLiteral("09/04/2026")));
    QVERIFY(ReviewCalculator::isValidSqliteDate(QStringLiteral("01/31/2026")));
    QVERIFY(!ReviewCalculator::isValidSqliteDate(QStringLiteral("9/4/2026")));
    QVERIFY(!ReviewCalculator::isValidSqliteDate(QStringLiteral("09/04/26")));
    QVERIFY(!ReviewCalculator::isValidSqliteDate(QStringLiteral("09/04/2026 ")));
    QVERIFY(!ReviewCalculator::isValidSqliteDate(QStringLiteral(" 09/04/2026")));
    QVERIFY(!ReviewCalculator::isValidSqliteDate(QStringLiteral("2026-09-04")));
    QVERIFY(!ReviewCalculator::isValidSqliteDate(QStringLiteral("abc")));
    QVERIFY(!ReviewCalculator::isValidSqliteDate(QString()));
}

void TstReviewCalculator::classifyEven()
{
    namespace RC = ReviewCalculator;
    // even (4,8,10): >=150% && ==8 -> 10/T2 ; >=100% && <8 -> 8/T1 ;
    // ==8 -> T3 ; ==10 -> T4 ; resto (p.ej. 4 con <100%) sin lista.
    RC::ClassifyOutcome o;
    o = RC::classify(ReviewState::Ready100Even, 150);   // stts 8
    QCOMPARE(o.state, ReviewState::Ready150Even);
    QCOMPARE(o.list, RC::ListKind::T2);
    QVERIFY(o.changed);

    o = RC::classify(ReviewState::Ready100Even, 149);   // stts 8, <150
    QCOMPARE(o.state, ReviewState::Ready100Even);
    QCOMPARE(o.list, RC::ListKind::T3);
    QVERIFY(!o.changed);

    o = RC::classify(ReviewState::WaitingAlternate, 100);  // stts 4
    QCOMPARE(o.state, ReviewState::Ready100Even);
    QCOMPARE(o.list, RC::ListKind::T1);
    QVERIFY(o.changed);

    o = RC::classify(ReviewState::WaitingAlternate, 200);  // stts 4, <8
    QCOMPARE(o.state, ReviewState::Ready100Even);
    QCOMPARE(o.list, RC::ListKind::T1);

    o = RC::classify(ReviewState::WaitingAlternate, 99);   // 4 con <100%
    QCOMPARE(o.state, ReviewState::WaitingAlternate);
    QCOMPARE(o.list, RC::ListKind::None);
    QVERIFY(!o.changed);

    o = RC::classify(ReviewState::Ready150Even, 1000);     // stts 10
    QCOMPARE(o.state, ReviewState::Ready150Even);
    QCOMPARE(o.list, RC::ListKind::T4);
    QVERIFY(!o.changed);
}

void TstReviewCalculator::classifyOdd()
{
    namespace RC = ReviewCalculator;
    RC::ClassifyOutcome o;
    o = RC::classify(ReviewState::Ready100Odd, 150);    // stts 7
    QCOMPARE(o.state, ReviewState::Ready150Odd);
    QCOMPARE(o.list, RC::ListKind::T2);
    QVERIFY(o.changed);

    o = RC::classify(ReviewState::Ready100Odd, 149);
    QCOMPARE(o.state, ReviewState::Ready100Odd);
    QCOMPARE(o.list, RC::ListKind::T3);
    QVERIFY(!o.changed);

    o = RC::classify(ReviewState::Waiting, 100);        // stts 3
    QCOMPARE(o.state, ReviewState::Ready100Odd);
    QCOMPARE(o.list, RC::ListKind::T1);
    QVERIFY(o.changed);

    o = RC::classify(ReviewState::Waiting, 99);
    QCOMPARE(o.state, ReviewState::Waiting);
    QCOMPARE(o.list, RC::ListKind::None);

    o = RC::classify(ReviewState::Ready150Odd, 1000);   // stts 9
    QCOMPARE(o.state, ReviewState::Ready150Odd);
    QCOMPARE(o.list, RC::ListKind::T4);
    QVERIFY(!o.changed);
}

void TstReviewCalculator::classifyNoOpStates()
{
    namespace RC = ReviewCalculator;
    // stts 2 -> T10
    RC::ClassifyOutcome o = RC::classify(ReviewState::Mastered, 0);
    QCOMPARE(o.state, ReviewState::Mastered);
    QCOMPARE(o.list, RC::ListKind::T10);
    QVERIFY(!o.changed);

    // stts 5/6: T7 sólo si el dir del topic está inactivo >20 días
    o = RC::classify(ReviewState::MidReview, 0, 21);
    QCOMPARE(o.state, ReviewState::MidReview);
    QCOMPARE(o.list, RC::ListKind::T7);
    o = RC::classify(ReviewState::MidReview, 0, 20);
    QCOMPARE(o.list, RC::ListKind::None);
    o = RC::classify(ReviewState::MidReviewAlternate, 0, 21);
    QCOMPARE(o.list, RC::ListKind::T7);
    o = RC::classify(ReviewState::MidReviewAlternate, 0, 20);
    QCOMPARE(o.list, RC::ListKind::None);

    // 0, 1 y 13 no generan listas (main.sh los salta o no entra en el bloque)
    o = RC::classify(ReviewState::Paused, 100);
    QCOMPARE(o.list, RC::ListKind::None);
    o = RC::classify(ReviewState::Learning, 100);
    QCOMPARE(o.list, RC::ListKind::None);
    o = RC::classify(ReviewState::Corrupted, 100);
    QCOMPARE(o.list, RC::ListKind::None);
}

void TstReviewCalculator::markAsLearnedTable()
{
    namespace RC = ReviewCalculator;
    // guard: sólo stts en {1,5,6}
    RC::MarkLearnedOutcome o;
    o = RC::markAsLearned(ReviewState::Waiting, 3);   // stts 3
    QVERIFY(!o.changed);
    QCOMPARE(o.state, ReviewState::Waiting);
    QCOMPARE(o.writtenDate, -1);

    // count==0 -> date1 + stts 3
    o = RC::markAsLearned(ReviewState::Learning, 0);
    QVERIFY(o.changed);
    QCOMPARE(o.state, ReviewState::Waiting);
    QCOMPARE(o.writtenDate, 1);
    QCOMPARE(o.count, 0);          // el original no incrementa la variable
    QVERIFY(!o.masteredFlag);

    // count==3 -> paridad invertida (stts+1 antes del even/odd)
    o = RC::markAsLearned(ReviewState::Learning, 3);  // 1+1=2 even -> 4
    QCOMPARE(o.state, ReviewState::WaitingAlternate);
    QCOMPARE(o.writtenDate, 4);
    QCOMPARE(o.count, 4);
    o = RC::markAsLearned(ReviewState::MidReview, 3); // 5+1=6 even -> 4
    QCOMPARE(o.state, ReviewState::WaitingAlternate);
    o = RC::markAsLearned(ReviewState::MidReviewAlternate, 3); // 6+1=7 odd -> 3
    QCOMPARE(o.state, ReviewState::Waiting);

    // conteos normales: even/odd del stts de entrada
    o = RC::markAsLearned(ReviewState::Learning, 4);  // 1 odd -> 3
    QCOMPARE(o.state, ReviewState::Waiting);
    QCOMPARE(o.writtenDate, 5);
    QCOMPARE(o.count, 5);
    o = RC::markAsLearned(ReviewState::MidReview, 4); // 5 odd -> 3
    QCOMPARE(o.state, ReviewState::Waiting);
    o = RC::markAsLearned(ReviewState::MidReviewAlternate, 4); // 6 even -> 4
    QCOMPARE(o.state, ReviewState::WaitingAlternate);

    // count==8 -> mast=TRUE; siguiente repaso -> count>8 -> date9 + mastered
    o = RC::markAsLearned(ReviewState::Learning, 8);
    QVERIFY(o.changed);
    QVERIFY(o.masteredFlag);
    QCOMPARE(o.writtenDate, 9);
    QCOMPARE(o.count, 9);
    QCOMPARE(o.state, ReviewState::Waiting);

    o = RC::markAsLearned(ReviewState::Learning, 9);
    // mast=(count==8): con count==9 el flag es FALSE (aunque el estado pasa a
    // Mastered y los ítems van a `learnt` en el original)
    QVERIFY(!o.masteredFlag);
    QCOMPARE(o.state, ReviewState::Mastered);
    QCOMPARE(o.writtenDate, 9);
    QCOMPARE(o.count, 9);

    // count==3 con stts 5: 6 even -> 4 (comportamiento de paridad del original)
    o = RC::markAsLearned(ReviewState::MidReview, 3);
    QCOMPARE(o.state, ReviewState::WaitingAlternate);
}

void TstReviewCalculator::markToLearnTable()
{
    namespace RC = ReviewCalculator;
    QCOMPARE(RC::markToLearn(ReviewState::Learning, 9), ReviewState::Mastered);
    QCOMPARE(RC::markToLearn(ReviewState::Learning, 8), ReviewState::MidReview);  // odd->5
    QCOMPARE(RC::markToLearn(ReviewState::MidReview, 8), ReviewState::MidReview); // 5 odd->5
    QCOMPARE(RC::markToLearn(ReviewState::MidReviewAlternate, 8),
             ReviewState::MidReviewAlternate);                                     // 6 even->6
    QCOMPARE(RC::markToLearn(ReviewState::Waiting, 1), ReviewState::MidReview);   // 3 odd->5
    QCOMPARE(RC::markToLearn(ReviewState::Mastered, 0), ReviewState::MidReviewAlternate); // 2 even->6
    QCOMPARE(RC::markToLearn(ReviewState::Corrupted, 0), ReviewState::MidReview); // 13 odd->5
    QCOMPARE(RC::markToLearn(ReviewState::Paused, 0), ReviewState::MidReviewAlternate); // 0 even->6
}

void TstReviewCalculator::mastered()
{
    QVERIFY(!ReviewCalculator::isMastered(0));
    QVERIFY(!ReviewCalculator::isMastered(8));
    QVERIFY(ReviewCalculator::isMastered(9));
    QVERIFY(ReviewCalculator::isMastered(10));
}

QTEST_GUILESS_MAIN(TstReviewCalculator)
#include "tst_reviewcalculator.moc"