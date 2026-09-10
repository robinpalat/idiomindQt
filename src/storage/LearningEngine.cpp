#include "storage/LearningEngine.h"

#include "core/learning/Review.h"
#include "core/topic/FlatItemCodec.h"
#include "core/topic/Topic.h"
#include "storage/dbs/SqliteDatabase.h"
#include "storage/topic/ReviewRepository.h"
#include "storage/topic/TopicDataRepository.h"

#include <QDate>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>

using namespace ::idiomind::core;

namespace idiomind {
namespace storage {

LearningEngine::LearningEngine(AppPaths paths)
    : m_paths(std::move(paths))
{
}

MarkResult LearningEngine::markAsLearned(const QString &tlng, const QString &topic)
{
    TopicDataRepository dataRepo(m_paths);
    ReviewRepository reviewRepo(m_paths);

    const QString sttsStr = dataRepo.readStts(tlng, topic);
    Topic tmp;
    tmp.setStts(sttsStr);
    const ReviewState st = static_cast<ReviewState>(tmp.statusValue());

    if (st != ReviewState::Learning &&
        st != ReviewState::MidReview &&
        st != ReviewState::MidReviewAlternate)
        return {};

    Review review = reviewRepo.load(tlng, topic);
    const int count = review.countFilled();

    const auto outcome = ReviewCalculator::markAsLearned(st, count);

    // Escribir fecha si corresponde
    if (outcome.writtenDate >= 1 && outcome.writtenDate <= 10) {
        review.setDate(outcome.writtenDate, QDate::currentDate());
        reviewRepo.store(tlng, topic, review);
    }

    // Escribir stts
    dataRepo.writeStts(tlng, topic,
                       QString::number(static_cast<int>(outcome.state)));

    // Reconstruir learning/learnt desde data
    rebuildLearningTables(tlng, topic, outcome.masteredFlag);

    // Limpiar practice logs
    clearPracticeLogs(tlng, topic);

    // Limpiar T1..T10
    clearTopicLists(tlng, topic);

    return {outcome.changed, outcome.state, outcome.writtenDate, outcome.masteredFlag};
}

bool LearningEngine::markToLearn(const QString &tlng, const QString &topic)
{
    TopicDataRepository dataRepo(m_paths);
    ReviewRepository reviewRepo(m_paths);

    const QString sttsStr = dataRepo.readStts(tlng, topic);
    Topic tmp;
    tmp.setStts(sttsStr);
    const ReviewState st = static_cast<ReviewState>(tmp.statusValue());

    Review review = reviewRepo.load(tlng, topic);
    const int count = review.countFilled();

    const ReviewState newState = ReviewCalculator::markToLearn(st, count);

    // Escribir stts
    dataRepo.writeStts(tlng, topic,
                       QString::number(static_cast<int>(newState)));

    // Si count >= 9, guardar repass = count en config
    if (count >= 9) {
        const QString dbPath = m_paths.topicDb(tlng, topic);
        SqliteDatabase db(dbPath);
        if (db.open()) {
            db.execute(QStringLiteral("update config set repass='%1';").arg(count));
        }
    }

    // Reconstruir learning con TODOS los items
    rebuildAllLearning(tlng, topic);

    // Eliminar lk si existe
    const QString conf = m_paths.topicConfDir(tlng, topic);
    QFile::remove(conf + QStringLiteral("/lk"));

    // Limpiar T1..T10
    clearTopicLists(tlng, topic);

    return true;
}

int LearningEngine::calculateReview(const QString &tlng, const QString &topic) const
{
    TopicDataRepository dataRepo(m_paths);
    ReviewRepository reviewRepo(m_paths);

    const QString sttsStr = dataRepo.readStts(tlng, topic);
    Topic tmp;
    tmp.setStts(sttsStr);
    const ReviewState st = static_cast<ReviewState>(tmp.statusValue());

    Review review = reviewRepo.load(tlng, topic);
    const int count = review.countFilled();

    if (count < 1)
        return 0;

    const QDate lastDate = review.lastDate();
    if (!lastDate.isValid())
        return 0;

    if (!ReviewCalculator::isValidSqliteDate(review.sqliteDate(count))) {
        // Error: fecha inválida → limpiar y reintentar con date1 = hoy
        reviewRepo.clear(tlng, topic);
        review.setDate(1, QDate::currentDate());
        reviewRepo.store(tlng, topic, review);
        return 0;
    }

    const QDateTime now = QDateTime::currentDateTime();
    const int tm = ReviewCalculator::daysSince(lastDate, now);
    return ReviewCalculator::overduePercent(tm, count);
}

ReviewCalculator::ClassifyOutcome LearningEngine::classifyTopic(
    const QString &tlng, const QString &topic) const
{
    TopicDataRepository dataRepo(m_paths);

    const QString sttsStr = dataRepo.readStts(tlng, topic);
    Topic tmp;
    tmp.setStts(sttsStr);
    const ReviewState st = static_cast<ReviewState>(tmp.statusValue());

    const int percent = calculateReview(tlng, topic);

    // topicDirAgeDays: días desde la última modificación del directorio .conf
    int dirAgeDays = 0;
    const QString conf = m_paths.topicConfDir(tlng, topic);
    const QFileInfo fi(conf);
    if (fi.exists()) {
        const QDateTime mtime = fi.lastModified();
        dirAgeDays = static_cast<int>(
            QDateTime::currentDateTime().toSecsSinceEpoch() - mtime.toSecsSinceEpoch()) / 86400;
    }

    return ReviewCalculator::classify(st, percent, dirAgeDays);
}

void LearningEngine::clearTopicLists(const QString &tlng, const QString &topic) const
{
    const QString shrdb = m_paths.sharedDb(tlng);
    SqliteDatabase db(shrdb);
    if (!db.open())
        return;
    for (int n = 1; n <= 10; ++n) {
        db.execute(QStringLiteral("delete from T%1 where list='%2';")
                       .arg(n).arg(topic));
    }
}

void LearningEngine::assignTopicToList(const QString &tlng,
                                       const QString &topic,
                                       ReviewCalculator::ListKind list) const
{
    if (list == ReviewCalculator::ListKind::None)
        return;
    const QString shrdb = m_paths.sharedDb(tlng);
    SqliteDatabase db(shrdb);
    if (!db.open())
        return;
    const QString table = QStringLiteral("T%1").arg(
        static_cast<int>(list));
    db.execute(QStringLiteral("insert into %1 (list) values ('%2');")
                   .arg(table, topic));
}

bool LearningEngine::rebuildLearningTables(const QString &tlng, const QString &topic,
                                           bool masterFlag) const
{
    const QString dbPath = m_paths.topicDb(tlng, topic);
    TopicDataRepository dataRepo(m_paths);
    const auto items = dataRepo.loadItems(tlng, topic);

    SqliteDatabase db(dbPath);
    if (!db.open())
        return false;

    db.execute(QStringLiteral("delete from learnt;"));
    db.execute(QStringLiteral("delete from learning;"));

    for (const auto &decoded : items) {
        const QString &trgt = decoded.item.trgt;
        if (trgt.isEmpty() || trgt == QLatin1String(" "))
            continue;
        const QString table = masterFlag
            ? QStringLiteral("learning")
            : QStringLiteral("learnt");
        db.execute(QStringLiteral("insert into %1 (list) values ('%2');")
                       .arg(table, trgt));
    }
    return true;
}

bool LearningEngine::rebuildAllLearning(const QString &tlng, const QString &topic) const
{
    const QString dbPath = m_paths.topicDb(tlng, topic);
    TopicDataRepository dataRepo(m_paths);
    const auto items = dataRepo.loadItems(tlng, topic);

    SqliteDatabase db(dbPath);
    if (!db.open())
        return false;

    db.execute(QStringLiteral("delete from learnt;"));
    db.execute(QStringLiteral("delete from learning;"));

    for (const auto &decoded : items) {
        const QString &trgt = decoded.item.trgt;
        if (trgt.isEmpty() || trgt == QLatin1String(" "))
            continue;
        db.execute(QStringLiteral("insert into learning (list) values ('%2');")
                       .arg(trgt));
    }
    return true;
}

bool LearningEngine::clearPracticeLogs(const QString &tlng, const QString &topic)
{
    const QString conf = m_paths.topicConfDir(tlng, topic);
    m_logs.clear(conf);
    return true;
}

} // namespace storage
} // namespace idiomind
