#include "core/learning/ReviewState.h"

namespace ReviewStateHelpers {

QString meaning(ReviewState state)
{
    switch (state) {
    case ReviewState::Paused:
        return QStringLiteral("paused / disabled");
    case ReviewState::Learning:
        return QStringLiteral("learning (active)");
    case ReviewState::Mastered:
        return QStringLiteral("mastered");
    case ReviewState::Waiting:
        return QStringLiteral("waiting (review due)");
    case ReviewState::WaitingAlternate:
        return QStringLiteral("waiting (review due, alternate)");
    case ReviewState::MidReview:
        return QStringLiteral("mid-review / \"Finalize review\" (readd topics)");
    case ReviewState::MidReviewAlternate:
        return QStringLiteral("mid-review (alternate)");
    case ReviewState::Ready100Odd:
        return QStringLiteral("ready for review (after 100% first interval)");
    case ReviewState::Ready100Even:
        return QStringLiteral("ready for review (alternate)");
    case ReviewState::Ready150Odd:
        return QStringLiteral("ready for review (after 150%)");
    case ReviewState::Ready150Even:
        return QStringLiteral("ready for review (alternate)");
    case ReviewState::Corrupted:
        return QStringLiteral("corrupted / addon topic without valid stts (auto-repair -> 1)");
    }
    return QStringLiteral("invalid");
}

}  // namespace ReviewStateHelpers