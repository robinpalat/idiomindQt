// F2.1 - ReviewCalculator: lógica pura de repaso.
// Autoridad: docs/06-learning-system.md, ifs/cmns.sh calculate_review,
// mngr.sh mark_as_learned / mark_to_learn, main.sh _get_list (clasificación T1..T10).
//
// Lógica PURO comparación: no filesystem, no SQLite, no QML, no red, no muta
// archivos. Todas las operaciones que tocan el mundo exterior entran como
// argumentos (now, topicDirAgeDays).
#pragma once

#include "core/learning/ReviewState.h"

#include <QDate>
#include <QDateTime>
#include <QVector>

// notice[] del original (sets.cfg): días entre repasos indexados por
// count_date_reviews. Contrato fijo.
namespace ReviewCalculator {

// notice[] completo, en orden (documentar para Settings::reviewIntervals()).
QVector<int> reviewIntervals();

// notice[count] para count en [0,9]; -1 fuera de rango (el original deja la
// variable indefinida para count >= 10; no se inventa un valor).
int daysToReviewAt(int count);

// days_to_review_porcent = 100 * TM / notice[count] con división entera de
// bash ($((100*TM/days))) — truncamiento hacia cero. count==0 -> 0
// (el original retorna temprano sin calcular; documentado en calculate_review).
int overduePercent(int tmDays, int count);

// Días entre la fecha de repaso y `now`, igual que el original:
// ((epoch(now) - epoch(midnight local de la fecha)) / 86400) truncado.
int daysSince(const QDate &reviewDate, const QDateTime &now);

// validación estricta del formato SQLite "^[0-9]{2}/[0-9]{2}/[0-9]{4}$"
// (calculate_review).
bool isValidSqliteDate(const QString &s);

// count_date_reviews >= 9  => mastered (docs/06).
bool isMastered(int countFilled);

// ---------------------------------------------------------------------------
// Clasificación en listas T1..T4/T7/T10 (main.sh _get_list). Devuelve el
// estado destino (puede promocionar 7->9, 8->10, etc.) y la lista destino.
// `percent` es days_to_review_porcent; `topicDirAgeDays` solo se usa para
// los estados 5/6 (condición dir > 20 días).
// ---------------------------------------------------------------------------
enum class ListKind { None, T1, T2, T3, T4, T7, T10 };

struct ClassifyOutcome {
    ReviewState state;
    ListKind list;
    bool changed;  // true si el original habría echo un nuevo stts
};

ClassifyOutcome classify(ReviewState st, int percent, int topicDirAgeDays = 0);

// ---------------------------------------------------------------------------
// Transiciones documentadas de mark_as_learned / mark_to_learn (mngr.sh).
// Pura: sólo calcula, no escribe fechas.
// ---------------------------------------------------------------------------
struct MarkLearnedOutcome {
    ReviewState state;
    int count;        // valor que el original deja en count_date_reviews
    int writtenDate;  // columna dateN escrita (-1 si no hubo cambios)
    bool masteredFlag;// mast: count == 8 a la entrada
    bool changed;     // true si stts pertenecía a {1,5,6}
};

// mark_as_learned: sólo actúa con stts en {1,5,6}; count==0 -> date1+stts3;
// count==3 -> paridad invertida (stts+1 antes del even/odd); count==8 -> mast
// TRUE; count>8 -> date9 + mastered(2).
MarkLearnedOutcome markAsLearned(ReviewState st, int count);

// mark_to_learn: count>=9 -> mastered(2); si no, even->6 / odd->5.
ReviewState markToLearn(ReviewState st, int count);

}  // namespace ReviewCalculator