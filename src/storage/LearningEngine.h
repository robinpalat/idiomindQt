// F4 - LearningEngine: orquestador del ciclo de aprendizaje.
// Autoridad: mngr.sh mark_as_learned / mark_to_learn, ifs/cmns.sh calculate_review,
// main.sh _get_list (T1..T10).
//
// Orquesta: ReviewCalculator (lógica pura) + ReviewRepository (fechas) +
// TopicDataRepository (stts, data, learning/learnt, index, practice logs) +
// SharedRepository (T1..T10).
//
// NO toca UI/QML. Los callbacks de notificación se omiten (pendiente F6).
#pragma once

#include "core/config/AppPaths.h"
#include "core/learning/PracticeLogs.h"
#include "core/learning/ReviewCalculator.h"
#include "core/learning/ReviewState.h"

#include <QString>

namespace idiomind {
namespace storage {

struct MarkResult {
    bool changed = false;
    ReviewState newState;
    int writtenDate = -1;
    bool masteredFlag = false;
};

class LearningEngine
{
public:
    explicit LearningEngine(core::AppPaths paths);

    MarkResult markAsLearned(const QString &tlng, const QString &topic);
    bool markToLearn(const QString &tlng, const QString &topic);
    int calculateReview(const QString &tlng, const QString &topic) const;
    ReviewCalculator::ClassifyOutcome classifyTopic(
        const QString &tlng, const QString &topic) const;
    void clearTopicLists(const QString &tlng, const QString &topic) const;
    void assignTopicToList(const QString &tlng, const QString &topic,
                           ReviewCalculator::ListKind list) const;
    bool rebuildLearningTables(const QString &tlng, const QString &topic,
                               bool masterFlag) const;
    bool rebuildAllLearning(const QString &tlng, const QString &topic) const;
    bool clearPracticeLogs(const QString &tlng, const QString &topic);

    core::PracticeLogs &practiceLogs() { return m_logs; }
    const core::PracticeLogs &practiceLogs() const { return m_logs; }

private:
    core::AppPaths m_paths;
    mutable core::PracticeLogs m_logs;
};

} // namespace storage
} // namespace idiomind
