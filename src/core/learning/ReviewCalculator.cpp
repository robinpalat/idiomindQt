#include "core/learning/ReviewCalculator.h"

#include <QRegularExpression>
#include <QTimeZone>

namespace {
constexpr int kNotice[] = {0, 4, 7, 7, 10, 15, 15, 20, 30, 60};
constexpr int kNoticeCount = 10;
}

namespace ReviewCalculator {

QVector<int> reviewIntervals()
{
    QVector<int> v;
    v.reserve(kNoticeCount);
    for (int n : kNotice)
        v.append(n);
    return v;
}

int daysToReviewAt(int count)
{
    if (count < 0 || count >= kNoticeCount)
        return -1;
    return kNotice[count];
}

int overduePercent(int tmDays, int count)
{
    if (count <= 0 || count >= kNoticeCount)
        return 0;  // count==0: el original retorna sin calcular (val. indef.)
    const int days = kNotice[count];
    if (days <= 0)
        return 0;
    // $((100*TM/days_to_review)) — división entera, truncamiento hacia cero.
    return (100 * tmDays) / days;
}

int daysSince(const QDate &reviewDate, const QDateTime &now)
{
    if (!reviewDate.isValid())
        return 0;
    const QDateTime midnight(reviewDate, QTime(0, 0), QTimeZone::systemTimeZone());
    // `date +%s` del original usa TZ local; la resta de epoch / 86400 trunca
    // hacia cero (igual que $((...))).
    const qint64 secs = now.toSecsSinceEpoch() - midnight.toSecsSinceEpoch();
    return static_cast<int>(secs / 86400);
}

bool isValidSqliteDate(const QString &s)
{
    static const QRegularExpression re(
        QStringLiteral("^[0-9]{2}/[0-9]{2}/[0-9]{4}$"));
    return re.match(s).hasMatch();
}

bool isMastered(int countFilled)
{
    return countFilled >= 9;
}

ClassifyOutcome classify(ReviewState st, int percent, int topicDirAgeDays)
{
    // main.sh _get_list: normaliza no-numérico -> 1 y salta stts == 0.
    const int s = static_cast<int>(st);

    if (s == 3 || s == 4 || s == 7 || s == 8 || s == 9 || s == 10) {
        if (ReviewStateHelpers::isEven(st)) {
            // even: 4, 8, 10
            if (percent >= 150 && s == 8)
                return {ReviewState::Ready150Even, ListKind::T2, true};
            if (percent >= 100 && s < 8)
                return {ReviewState::Ready100Even, ListKind::T1, true};
            if (s == 8)
                return {ReviewState::Ready100Even, ListKind::T3, false};
            if (s == 10)
                return {ReviewState::Ready150Even, ListKind::T4, false};
            return {st, ListKind::None, false};  // p.ej. stts 4 con <100%
        }
        // odd: 3, 7, 9
        if (percent >= 150 && s == 7)
            return {ReviewState::Ready150Odd, ListKind::T2, true};
        if (percent >= 100 && s < 7)
            return {ReviewState::Ready100Odd, ListKind::T1, true};
        if (s == 7)
            return {ReviewState::Ready100Odd, ListKind::T3, false};
        if (s == 9)
            return {ReviewState::Ready150Odd, ListKind::T4, false};
        return {st, ListKind::None, false};  // p.ej. stts 3 con <100%
    }

    if (st == ReviewState::Mastered)  // 2
        return {ReviewState::Mastered, ListKind::T10, false};

    if (s == 5 || s == 6) {  // $((stts+stts%2)) = 6
        if (topicDirAgeDays > 20)
            return {st, ListKind::T7, false};
        return {st, ListKind::None, false};
    }

    return {st, ListKind::None, false};  // 0, 1, 13 y fuera de contrato
}

MarkLearnedOutcome markAsLearned(ReviewState st, int count)
{
    const int in = static_cast<int>(st);
    int cnt = count < 0 ? 0 : count;

    const bool guard = (st == ReviewState::Learning ||
                        st == ReviewState::MidReview ||
                        st == ReviewState::MidReviewAlternate);
    if (!guard)
        return {st, cnt, -1, false, false};

    const bool mast = (cnt == 8);

    if (cnt > 8) {  // cambiar de familiar a mastered (date9, stts 2)
        return {ReviewState::Mastered, cnt, 9, mast, true};
    }

    if (cnt > 0) {
        int s = in;
        if (cnt == 3)  // cambiar de fresh a familiar: paridad invertida
            s = s + 1;
        ++cnt;  // tpc_db 9 reviews date${cnt} hoy
        const ReviewState next =
            (s % 2) == 0 ? ReviewState::WaitingAlternate : ReviewState::Waiting;
        return {next, cnt, cnt, mast, true};
    }

    return {ReviewState::Waiting, 0, 1, mast, true};  // date1, stts 3
}

ReviewState markToLearn(ReviewState st, int count)
{
    const int cnt = count < 0 ? 0 : count;
    if (cnt >= 9)
        return ReviewState::Mastered;
    return (static_cast<int>(st) % 2) == 0 ? ReviewState::MidReviewAlternate
                                           : ReviewState::MidReview;
}

}  // namespace ReviewCalculator