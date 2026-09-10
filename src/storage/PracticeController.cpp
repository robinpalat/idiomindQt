#include "storage/PracticeController.h"

#include "storage/dbs/SqliteDatabase.h"
#include "storage/topic/TopicDataRepository.h"

#include <QDate>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QRegularExpression>
#include <QRandomGenerator>
#include <QSet>
#include <QSqlQuery>
#include <QVariant>

#include <algorithm>

using namespace ::idiomind::core;

namespace idiomind {
namespace storage {

namespace {

QString sqlQuote(const QString &value)
{
    return QString(value).replace(QLatin1Char('\''), QStringLiteral("''"));
}

QStringList nonEmptyLines(const QStringList &lines)
{
    QStringList result;
    for (const QString &line : lines) {
        const QString value = line.trimmed();
        if (!value.isEmpty() && !value.startsWith(QLatin1Char('#')))
            result.append(value);
    }
    return result;
}

QStringList splitWords(const QString &value)
{
    return value.split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
}

} // namespace

PracticeController::PracticeController(AppPaths paths, QObject *parent)
    : QObject(parent)
    , m_paths(std::move(paths))
{
}

void PracticeController::setContext(const QString &tlng, const QString &topic)
{
    if (m_tlng == tlng && m_topic == topic)
        return;
    if (m_active)
        stop();
    m_tlng = tlng;
    m_topic = topic;
    clearCurrentItem();
    emit stateChanged();
}

bool PracticeController::hasConfiguration(int mode) const
{
    if (mode < 1 || mode > 5 || m_tlng.isEmpty() || m_topic.isEmpty())
        return false;
    return QFileInfo::exists(practiceDir() + QLatin1Char('/') + practiceName(mode));
}

bool PracticeController::configure(int mode, bool learning, bool sourceToTarget)
{
    if (mode < 1 || mode > 5 || m_tlng.isEmpty() || m_topic.isEmpty())
        return false;
    const QString group = learning ? QStringLiteral("1|10") : QStringLiteral("0|");
    rewritePracticeFile(practiceName(mode),
                        {group + QLatin1Char('|')
                         + (sourceToTarget ? QStringLiteral("1")
                                           : QStringLiteral("0"))});
    return true;
}

bool PracticeController::start(int mode)
{
    return startSession(m_tlng, m_topic, mode);
}

bool PracticeController::startSession(const QString &tlng, const QString &topic, int mode)
{
    if (mode < 1 || mode > 5 || tlng.isEmpty() || topic.isEmpty())
        return false;

    if (m_active)
        stop();

    m_tlng = tlng;
    m_topic = topic;
    m_mode = mode;
    m_round = 1;
    m_roundIndex = -1;
    m_processed = 0;
    m_easy = 0;
    m_learn = 0;
    m_hard = 0;
    m_reverse = false;
    m_groupMode = false;
    m_awaitingGroupDecision = false;
    m_groupTargets.clear();
    m_currentGroup.clear();
    m_sectionTotal = 0;
    m_failed.clear();
    m_results.clear();
    m_result.clear();
    m_finished = false;

    if (!prepareNotes(mode) || isLocked(mode)) {
        clearCurrentItem();
        m_active = false;
        emit stateChanged();
        return false;
    }

    const QString name = practiceName(mode);
    QFile::remove(practiceDir() + QLatin1Char('/') + name + QStringLiteral(".2"));
    QFile::remove(practiceDir() + QLatin1Char('/') + name + QStringLiteral(".3"));
    const QStringList settings = readPracticeFile(name);
    if (!settings.isEmpty()) {
        const QStringList fields = settings.first().split(QLatin1Char('|'));
        m_groupMode = fields.value(0) == QLatin1String("1");
        m_reverse = fields.value(2) == QLatin1String("1");
    }
    QStringList targets = readPracticeFile(name + QStringLiteral(".0"));
    m_sectionTotal = targets.size();
    const QStringList learnedLines = readPracticeFile(name + QStringLiteral(".1"));
    const QSet<QString> learned(learnedLines.cbegin(), learnedLines.cend());
    QStringList pending;
    if (m_groupMode) {
        m_groupTargets = readPracticeFile(name + QStringLiteral(".group"));
        if (m_groupTargets.isEmpty()) {
            m_groupTargets = targets;
            rewritePracticeFile(name + QStringLiteral(".group"), m_groupTargets);
        }
        for (const QString &target : m_groupTargets) {
            if (!learned.contains(target))
                pending.append(target);
            if (pending.size() == 10)
                break;
        }
        m_currentGroup = pending;
    } else {
        for (const QString &target : targets) {
            if (!learned.contains(target))
                pending.append(target);
        }
    }

    m_roundItems = itemsForTargets(pending);
    m_session = PracticeSession{};
    m_session.mode = mode;
    m_session.items = m_roundItems;
    m_session.total = m_roundItems.size();
    m_session.correct = 0;
    m_roundIndex = 0;

    // PractN is the persistent snapshot used by the legacy API. The active
    // queue remains in the practice .0 file, as it does in strt.sh.
    savePractList(tlng, topic, mode, targets);

    if (m_roundItems.isEmpty()) {
        m_active = false;
        m_finished = true;
        m_result = {{QStringLiteral("completed"), true},
                    {QStringLiteral("correct"), 0},
                    {QStringLiteral("total"), 0}};
        emit resultChanged();
        emit stateChanged();
        return false;
    }

    m_active = true;
    loadCurrentItem();
    emit stateChanged();
    return true;
}

bool PracticeController::prepareNotes(int mode)
{
    const QString dir = practiceDir();
    if (!QDir().mkpath(dir))
        return false;

    const QString name = practiceName(mode);
    const QString path = dir + QLatin1Char('/') + name + QStringLiteral(".0");
    if (!QFileInfo::exists(path)) {
        QStringList targets;
        const QList<Item> selected = selectItems(m_tlng, m_topic, mode);
        for (const Item &item : selected)
            targets.append(item.trgt);
        rewritePracticeFile(name + QStringLiteral(".0"), targets);
    }

    // strt.sh removes the previous failed-round files after lock handling;
    // .1 is deliberately retained as the learned subset.
    return true;
}

bool PracticeController::prepareGroup()
{
    if (!m_groupMode)
        return false;
    const QString name = practiceName(m_mode);
    const QStringList group = readPracticeFile(name + QStringLiteral(".group"));
    const QStringList learnedLines = readPracticeFile(name + QStringLiteral(".1"));
    const QSet<QString> learned(learnedLines.cbegin(), learnedLines.cend());
    QStringList next;
    for (const QString &target : group) {
        if (!learned.contains(target))
            next.append(target);
        if (next.size() == 10)
            break;
    }
    m_currentGroup = next;
    m_roundItems = itemsForTargets(next);
    m_roundIndex = m_roundItems.isEmpty() ? -1 : 0;
    return !m_roundItems.isEmpty();
}

bool PracticeController::isLocked(int mode) const
{
    const QString path = practiceDir() + QLatin1Char('/') + practiceName(mode)
        + QStringLiteral(".lock");
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    const QString content = QString::fromUtf8(file.readAll()).trimmed();
    file.close();
    if (!content.startsWith(QStringLiteral("wait")))
        return true;

    const QRegularExpression re(QStringLiteral("wait=\\\"(\\d+)\\\""));
    const QRegularExpressionMatch match = re.match(content);
    if (!match.hasMatch() || match.captured(1).toInt() != QDate::currentDate().day()) {
        QFile::remove(path);
        return false;
    }
    return true;
}

PracticeSession PracticeController::beginMode(const QString &tlng,
                                              const QString &topic, int mode)
{
    startSession(tlng, topic, mode);
    return m_session;
}

void PracticeController::recordResult(const QString &tlng, const QString &topic,
                                      const QString &trgt, bool isCorrect, int mode)
{
    if (m_tlng != tlng || m_topic != topic || m_mode != mode || !m_active)
        startSession(tlng, topic, mode);
    if (!m_active || m_roundIndex < 0 || m_roundIndex >= m_roundItems.size())
        return;

    if (m_roundItems.at(m_roundIndex).trgt != trgt)
        return;
    if (submitAnswer(isCorrect))
        next();
}

void PracticeController::finishSession(const QString &tlng, const QString &topic,
                                       PracticeSession &session)
{
    if (m_active && m_tlng == tlng && m_topic == topic && m_mode == session.mode)
        stop();
    session.total = session.items.size();
}

bool PracticeController::submit(const QVariant &answerValue)
{
    return submitAnswer(answerValue);
}

bool PracticeController::answer(const QVariant &answerValue)
{
    return submitAnswer(answerValue);
}

bool PracticeController::mark(bool known)
{
    return submitAnswer(known);
}

bool PracticeController::submitAnswer(const QVariant &answerValue)
{
    if (!m_active || m_answered || m_roundIndex < 0 || m_roundIndex >= m_roundItems.size())
        return false;

    int percentage = -1;
    const int category = categoryForAnswer(answerValue, &percentage);
    return submitCategory(category, percentage);
}

void PracticeController::showGroupDecision()
{
    m_awaitingGroupDecision = true;
    m_roundIndex = -1;
    m_question.clear();
    m_response.clear();
    m_options.clear();
    m_image.clear();
    m_audioSource.clear();
    m_answered = false;
    emit itemChanged();
    emit stateChanged();
}

bool PracticeController::again()
{
    if (!m_active || !m_groupMode || !m_awaitingGroupDecision)
        return false;
    const QString name = practiceName(m_mode);
    QStringList learned = readPracticeFile(name + QStringLiteral(".1"));
    for (const QString &target : m_currentGroup)
        learned.removeAll(target);
    rewritePracticeFile(name + QStringLiteral(".1"), learned);
    learned.removeDuplicates();
    rewritePracticeFile(name + QStringLiteral(".l"),
                        {QString::number(learned.size())});
    rewritePracticeFile(name + QStringLiteral(".2"), {});
    rewritePracticeFile(name + QStringLiteral(".3"), {});
    m_round = 1;
    m_easy = 0;
    m_learn = 0;
    m_hard = 0;
    m_failed.clear();
    m_awaitingGroupDecision = false;
    m_roundItems = itemsForTargets(m_currentGroup);
    m_roundIndex = m_roundItems.isEmpty() ? -1 : 0;
    loadCurrentItem();
    emit stateChanged();
    return !m_roundItems.isEmpty();
}

bool PracticeController::continueGroup()
{
    if (!m_active || !m_groupMode || !m_awaitingGroupDecision)
        return false;
    const QString name = practiceName(m_mode);
    QStringList learned = readPracticeFile(name + QStringLiteral(".1"));
    QStringList group = readPracticeFile(name + QStringLiteral(".group"));
    QStringList remaining;
    for (const QString &target : group) {
        if (!learned.contains(target))
            remaining.append(target);
    }
    rewritePracticeFile(name + QStringLiteral(".group"), remaining);
    rewritePracticeFile(name + QStringLiteral(".l"),
                       {QString::number(learned.size())});
    m_awaitingGroupDecision = false;
    m_round = 1;
    m_easy = 0;
    m_learn = 0;
    m_hard = 0;
    m_failed.clear();
    if (remaining.isEmpty()) {
        finishPractice(true);
        return false;
    }
    m_currentGroup.clear();
    for (const QString &target : remaining) {
        m_currentGroup.append(target);
        if (m_currentGroup.size() == 10)
            break;
    }
    m_roundItems = itemsForTargets(m_currentGroup);
    m_roundIndex = m_roundItems.isEmpty() ? -1 : 0;
    loadCurrentItem();
    emit stateChanged();
    return !m_roundItems.isEmpty();
}

bool PracticeController::submitCategory(int category, int percentage)
{
    if (category < 1 || category > 3 || m_roundIndex < 0
        || m_roundIndex >= m_roundItems.size())
        return false;

    const Item &item = m_roundItems.at(m_roundIndex);
    const QString target = item.trgt;
    writeOutcome(category, target, m_round);
    if (category == 1) {
        ++m_session.correct;
        if (m_round == 1)
            ++m_easy;
        else
            ++m_learn;
    } else {
        ++m_hard;
        if (!m_failed.contains(target))
            m_failed.append(target);
    }

    m_answered = true;
    ++m_processed;
    m_response = expectedAnswer();
    m_result = {
        {QStringLiteral("trgt"), target},
        {QStringLiteral("correct"), category == 1},
        {QStringLiteral("category"), category},
        {QStringLiteral("round"), m_round},
        {QStringLiteral("percentage"), percentage},
    };
    m_results.append(m_result);
    emit itemChanged();
    emit resultChanged();
    emit stateChanged();
    return true;
}

bool PracticeController::next()
{
    if (!m_active || !m_answered)
        return false;

    ++m_roundIndex;
    m_answered = false;
    m_response.clear();

    if (m_roundIndex < m_roundItems.size()) {
        loadCurrentItem();
        emit stateChanged();
        return true;
    }
    return advanceRound();
}

bool PracticeController::nextItem()
{
    return next();
}

bool PracticeController::advanceRound()
{
    const QStringList failed = m_failed;
    m_hard = failed.size();
    m_failed.clear();

    if (m_groupMode && (m_mode == 5 || failed.isEmpty() || m_round >= 3)) {
        showGroupDecision();
        return false;
    }

    if (m_mode == 5 || failed.isEmpty()) {
        const int sectionSize = readPracticeFile(practiceName(m_mode)
                                                 + QStringLiteral(".0"))
                                    .size();
        finishPractice(learnedBefore() + m_easy >= sectionSize);
        return false;
    }

    if (m_round >= 3) {
        finishPractice(false);
        return false;
    }

    ++m_round;
    if (m_round == 3)
        rewritePracticeFile(practiceName(m_mode) + QStringLiteral(".3"), {});
    m_roundItems = itemsForTargets(failed);
    m_roundIndex = 0;
    if (m_roundItems.isEmpty()) {
        finishPractice(false);
        return false;
    }
    loadCurrentItem();
    emit stateChanged();
    return true;
}

bool PracticeController::restart()
{
    if (m_tlng.isEmpty() || m_topic.isEmpty() || m_mode < 1 || m_mode > 5)
        return false;

    const QString dir = practiceDir();
    const QString name = practiceName(m_mode);
    const QStringList suffixes = {
        QStringLiteral(".0"), QStringLiteral(".1"), QStringLiteral(".2"),
        QStringLiteral(".3"), QStringLiteral(".srces"), QStringLiteral(".df"),
        QStringLiteral(".group"), QStringLiteral(".lock"), QStringLiteral(".l"),
        QString()
    };
    for (const QString &suffix : suffixes)
        QFile::remove(dir + QLatin1Char('/') + name + suffix);
    rewritePracticeFile(QStringLiteral(".%1").arg(m_mode), {QStringLiteral("1")});
    m_active = false;
    m_finished = false;
    clearCurrentItem();
    emit resultChanged();
    emit stateChanged();
    return true;
}

bool PracticeController::restartAll()
{
    if (m_tlng.isEmpty() || m_topic.isEmpty())
        return false;

    if (m_active)
        stop();

    QDir dir(practiceDir());
    if (dir.exists()) {
        const QStringList files = dir.entryList(QDir::Files | QDir::Hidden
                                                | QDir::NoDotAndDotDot);
        for (const QString &file : files)
            QFile::remove(dir.filePath(file));
    }
    QDir().mkpath(practiceDir());
    for (int n = 1; n <= 3; ++n) {
        QFile log(practiceDir() + QStringLiteral("/log%1").arg(n));
        log.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);
        log.close();
    }

    for (int mode = 1; mode <= 5; ++mode) {
        rewritePracticeFile(QStringLiteral(".%1").arg(mode), {QStringLiteral("1")});
        prepareNotes(mode);
    }

    m_active = false;
    m_finished = false;
    m_mode = 0;
    m_groupMode = false;
    m_awaitingGroupDecision = false;
    m_sectionTotal = 0;
    clearCurrentItem();
    emit resultChanged();
    emit stateChanged();
    return true;
}

void PracticeController::acknowledgeResult()
{
    if (!m_finished || m_active)
        return;
    m_finished = false;
    clearCurrentItem();
    emit resultChanged();
    emit stateChanged();
}

void PracticeController::stop()
{
    if (!m_active)
        return;
    const int sectionSize = readPracticeFile(practiceName(m_mode)
                                             + QStringLiteral(".0"))
                                .size();
    finishPractice(learnedBefore() + m_easy >= sectionSize);
}

void PracticeController::finishPractice(bool completed)
{
    if (!m_active)
        return;

    const QString name = practiceName(m_mode);
    if (completed) {
        QFile::remove(practiceDir() + QLatin1Char('/') + name + QStringLiteral(".0"));
        QFile::remove(practiceDir() + QLatin1Char('/') + name + QStringLiteral(".2"));
        QFile::remove(practiceDir() + QLatin1Char('/') + name + QStringLiteral(".3"));
        writePracticeLogs(m_mode == 5 ? 2 : m_round);
        writeSessionLock(true);
        updatePracticeStats(21);
    } else {
        const int sectionSize = readPracticeFile(name + QStringLiteral(".0")).size();
        const int learned = learnedBefore() + m_easy;
        QFile progressFile(practiceDir() + QLatin1Char('/') + name + QStringLiteral(".l"));
        if (progressFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
            progressFile.write(QByteArray::number(learned));
            progressFile.write("\n");
            progressFile.close();
        }
        writePracticeLogs(m_mode == 5 ? 2 : m_round);
        const int value = sectionSize > 0 ? (100 * learned / sectionSize) : 0;
        updatePracticeStats(iconForProgress(value));
        if ((m_round > 1 || m_mode == 5) && m_learn > 0 && m_failed.isEmpty())
            writeSessionLock(false);
    }

    m_active = false;
    m_finished = true;
    m_result.insert(QStringLiteral("completed"), completed);
    m_result.insert(QStringLiteral("correct"), m_session.correct);
    m_result.insert(QStringLiteral("total"), m_session.total);
    m_result.insert(QStringLiteral("score"), score());
    emit resultChanged();
    emit stateChanged();
    emit finished(m_session.correct, m_session.total);
}

void PracticeController::clearCurrentItem()
{
    m_roundItems.clear();
    m_roundIndex = -1;
    m_question.clear();
    m_response.clear();
    m_options.clear();
    m_image.clear();
    m_audioSource.clear();
    m_answered = false;
    emit itemChanged();
}

void PracticeController::loadCurrentItem()
{
    if (m_roundIndex < 0 || m_roundIndex >= m_roundItems.size()) {
        clearCurrentItem();
        return;
    }

    const Item &item = m_roundItems.at(m_roundIndex);
    m_question = m_reverse ? item.srce : item.trgt;
    m_response.clear();
    m_options.clear();
    m_image.clear();
    m_audioSource.clear();

    if (m_mode == 2) {
        QStringList candidates;
        for (const Item &candidate : selectItems(m_tlng, m_topic, 2)) {
            const QString candidateAnswer = m_reverse ? candidate.trgt : candidate.srce;
            if (!candidateAnswer.isEmpty() && candidateAnswer != expectedAnswer())
                candidates.append(candidateAnswer);
        }
        candidates.removeDuplicates();
        std::shuffle(candidates.begin(), candidates.end(), *QRandomGenerator::global());
        while (candidates.size() > 3)
            candidates.removeLast();
        m_options = candidates;
        m_options.append(expectedAnswer());
        m_options.removeDuplicates();
        std::shuffle(m_options.begin(), m_options.end(), *QRandomGenerator::global());
    } else if (m_mode == 4) {
        const QString local = m_paths.topicDataDir(m_tlng, m_topic)
            + QStringLiteral("/images/") + item.trgt.toLower() + QStringLiteral(".jpg");
        const QString shared = m_paths.sharedDirForLanguage(m_tlng)
            + QStringLiteral("/images/") + item.trgt.toLower() + QStringLiteral("-1.jpg");
        if (QFileInfo::exists(local))
            m_image = local;
        else if (QFileInfo::exists(shared))
            m_image = shared;
        else {
            const QString missing = m_paths.dataRoot() + QStringLiteral("/images/imgmiss.jpg");
            if (QFileInfo::exists(missing))
                m_image = missing;
        }
    }

    const QString topicAudio = m_paths.topicDataDir(m_tlng, m_topic)
        + QLatin1Char('/') + item.cdid + QStringLiteral(".mp3");
    const QString audioWord = (m_mode == 3 && m_reverse) ? item.srce : item.trgt;
    const QString sharedAudio = m_paths.sharedDirForLanguage(m_tlng)
        + QStringLiteral("/audio/") + audioWord.toLower() + QStringLiteral(".mp3");
    if (QFileInfo::exists(topicAudio))
        m_audioSource = topicAudio;
    else if (QFileInfo::exists(sharedAudio))
        m_audioSource = sharedAudio;
    emit itemChanged();
}

QVariantMap PracticeController::currentItem() const
{
    if (m_roundIndex < 0 || m_roundIndex >= m_roundItems.size())
        return {};
    QVariantMap value = itemMap(m_roundItems.at(m_roundIndex));
    value.insert(QStringLiteral("question"), m_question);
    value.insert(QStringLiteral("answer"), m_response);
    value.insert(QStringLiteral("expectedAnswer"), expectedAnswer());
    value.insert(QStringLiteral("options"), m_options);
    value.insert(QStringLiteral("image"), m_image);
    value.insert(QStringLiteral("audioSource"), m_audioSource);
    return value;
}

QString PracticeController::state() const
{
    if (m_active)
        return m_answered ? QStringLiteral("answered") : QStringLiteral("question");
    if (m_finished)
        return QStringLiteral("finished");
    return QStringLiteral("idle");
}

QString PracticeController::expectedAnswer() const
{
    if (m_roundIndex < 0 || m_roundIndex >= m_roundItems.size())
        return {};
    const Item &item = m_roundItems.at(m_roundIndex);
    if (m_mode == 3 || m_mode == 5)
        return item.trgt;
    return m_reverse ? item.trgt : item.srce;
}

int PracticeController::score() const
{
    if (m_session.total <= 0)
        return 0;
    return std::min(100, 100 * m_session.correct / m_session.total);
}

double PracticeController::progress() const
{
    if (m_sectionTotal <= 0 && m_session.total <= 0)
        return 0.0;
    const int total = m_sectionTotal > 0 ? m_sectionTotal : m_session.total;
    const int learned = learnedBefore() + m_easy;
    return std::min(100.0, 100.0 * learned / total);
}

QStringList PracticeController::loadPractList(const QString &tlng,
                                               const QString &topic, int mode) const
{
    if (mode < 1 || mode > 5)
        return {};
    SqliteDatabase db(m_paths.topicDb(tlng, topic));
    if (!db.open())
        return {};
    const QString table = QStringLiteral("Pract%1").arg(mode);
    QSqlQuery query = db.query(QStringLiteral("select * from '%1';").arg(table));
    QStringList result;
    while (query.isActive() && query.next())
        result.append(query.value(0).toString());
    return result;
}

bool PracticeController::savePractList(const QString &tlng, const QString &topic,
                                       int mode, const QStringList &items) const
{
    if (mode < 1 || mode > 5)
        return false;
    SqliteDatabase db(m_paths.topicDb(tlng, topic));
    if (!db.open())
        return false;
    const QString table = QStringLiteral("Pract%1").arg(mode);
    if (!db.execute(QStringLiteral("delete from '%1';").arg(table)))
        return false;
    bool ok = true;
    for (const QString &item : items) {
        ok = db.execute(QStringLiteral("insert into '%1' (items_0) values ('%2');")
                            .arg(table, sqlQuote(item))) && ok;
    }
    return ok;
}

QList<Item> PracticeController::selectItems(const QString &tlng, const QString &topic,
                                            int mode) const
{
    if (mode < 1 || mode > 5)
        return {};

    TopicDataRepository dataRepo(m_paths);
    const auto decoded = dataRepo.loadItems(tlng, topic);
    const QStringList learning = dataRepo.learningList(tlng, topic);
    const QSet<QString> learningSet(learning.cbegin(), learning.cend());
    QSet<QString> sentenceSet;
    QSet<QString> wordSet;
    {
        SqliteDatabase db(m_paths.topicDb(tlng, topic));
        if (db.open()) {
            QSqlQuery sentences = db.query(QStringLiteral("select list from sentences;"));
            while (sentences.isActive() && sentences.next())
                sentenceSet.insert(sentences.value(0).toString());
            QSqlQuery words = db.query(QStringLiteral("select list from words;"));
            while (words.isActive() && words.next())
                wordSet.insert(words.value(0).toString());
        }
    }

    QString level;
    {
        SqliteDatabase db(m_paths.configDb());
        if (db.open())
            level = db.value(QStringLiteral("select level from opts limit 1;"));
    }

    QList<Item> result;
    QSet<QString> seen;
    for (const auto &decodedItem : decoded) {
        const Item &item = decodedItem.item;
        if (!learningSet.contains(item.trgt) || seen.contains(item.trgt))
            continue;
        if (mode <= 4 && (sentenceSet.contains(item.trgt) || item.isSentence()))
            continue;
        if (mode == 5 && (wordSet.contains(item.trgt) || item.isWord()))
            continue;
        if (mode == 4) {
            const QString local = m_paths.topicDataDir(tlng, topic)
                + QStringLiteral("/images/") + item.trgt.toLower() + QStringLiteral(".jpg");
            const QString shared = m_paths.sharedDirForLanguage(tlng)
                + QStringLiteral("/images/") + item.trgt.toLower() + QStringLiteral("-1.jpg");
            if (!QFileInfo::exists(local) && !QFileInfo::exists(shared))
                continue;
        }
        if (mode == 5 && level == QLatin1String("0")
            && splitWords(item.trgt).size() > 12)
            continue;
        result.append(item);
        seen.insert(item.trgt);
    }
    return result;
}

QList<Item> PracticeController::itemsForTargets(const QStringList &targets) const
{
    const auto all = [&]() {
        TopicDataRepository repo(m_paths);
        return repo.loadItems(m_tlng, m_topic);
    }();
    QHash<QString, QList<Item>> byTarget;
    for (const auto &decoded : all)
        byTarget[decoded.item.trgt].append(decoded.item);

    QList<Item> result;
    for (const QString &target : targets) {
        if (byTarget.contains(target) && !byTarget[target].isEmpty())
            result.append(byTarget[target].takeFirst());
    }
    return result;
}

QString PracticeController::practiceDir() const
{
    return m_paths.topicConfDir(m_tlng, m_topic) + QStringLiteral("/practice");
}

QString PracticeController::practiceName(int mode) const
{
    return mode == 1 ? QStringLiteral("a")
         : mode == 2 ? QStringLiteral("b")
         : mode == 3 ? QStringLiteral("c")
         : mode == 4 ? QStringLiteral("d")
                     : QStringLiteral("e");
}

QStringList PracticeController::readPracticeFile(const QString &name) const
{
    QFile file(practiceDir() + QLatin1Char('/') + name);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return {};
    QStringList lines;
    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        while (line.endsWith('\n') || line.endsWith('\r'))
            line.chop(1);
        const QString value = QString::fromUtf8(line);
        if (!value.trimmed().isEmpty() && !value.trimmed().startsWith(QLatin1Char('#')))
            lines.append(value);
    }
    return lines;
}

void PracticeController::appendPracticeLine(const QString &name, const QString &value) const
{
    if (value.isEmpty())
        return;
    QDir().mkpath(practiceDir());
    QFile file(practiceDir() + QLatin1Char('/') + name);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
        return;
    if (file.size() > 0)
        file.write("\n");
    file.write(value.toUtf8());
    file.close();
}

void PracticeController::rewritePracticeFile(const QString &name, const QStringList &lines) const
{
    QDir().mkpath(practiceDir());
    QFile file(practiceDir() + QLatin1Char('/') + name);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
        return;
    for (const QString &line : lines) {
        file.write(line.toUtf8());
        file.write("\n");
    }
    file.close();
}

void PracticeController::writeOutcome(int category, const QString &trgt, int round) const
{
    if (m_mode == 5) {
        appendPracticeLine(QStringLiteral("e.%1").arg(category), trgt);
    } else if (round == 1) {
        appendPracticeLine(practiceName(m_mode) + (category == 1
                              ? QStringLiteral(".1") : QStringLiteral(".2")), trgt);
    } else if (round >= 2 && category == 3) {
        appendPracticeLine(practiceName(m_mode) + QStringLiteral(".3"), trgt);
    }
}

void PracticeController::writeSessionLock(bool completed) const
{
    const QString path = practiceDir() + QLatin1Char('/') + practiceName(m_mode)
        + QStringLiteral(".lock");
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
        return;
    if (completed)
        file.write(QDate::currentDate().toString(QStringLiteral("ddd dd MMMM")).toUtf8());
    else
        file.write(QStringLiteral("wait=\"%1\"").arg(QDate::currentDate().day()).toUtf8());
    file.write("\n");
    file.close();
}

void PracticeController::writeWeeklyLog(const QString &line) const
{
    if (line.isEmpty())
        return;
    const QString logsDir = m_paths.configRoot() + QStringLiteral("/logs");
    QDir().mkpath(logsDir);
    const int week = QDate::currentDate().weekNumber();
    QFile file(logsDir + QStringLiteral("/%1.log").arg(week));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
        return;
    file.write(line.toUtf8());
    file.write("\n");
    file.close();
}

void PracticeController::writePracticeLogs(int step) const
{
    const QString name = practiceName(m_mode);
    const QString stts = TopicDataRepository(m_paths).readStts(m_tlng, m_topic);
    const auto encode = [](const QStringList &lines) {
        return lines.isEmpty() ? QString() : lines.join(QLatin1Char('|')) + QLatin1Char('|');
    };

    if (m_mode == 5) {
        for (int category = 1; category <= 3; ++category) {
            const QStringList lines = readPracticeFile(QStringLiteral("e.%1").arg(category));
            if (!lines.isEmpty())
                writeWeeklyLog(QStringLiteral("s%1.%2.s%1.<%3>")
                                   .arg(category).arg(encode(lines), stts));
        }
    } else {
        const QStringList one = readPracticeFile(name + QStringLiteral(".1"));
        const QStringList two = readPracticeFile(name + QStringLiteral(".2"));
        const QStringList three = readPracticeFile(name + QStringLiteral(".3"));
        if (!one.isEmpty())
            writeWeeklyLog(QStringLiteral("w1.%1.w1.<%2>").arg(encode(one), stts));
        if (!two.isEmpty()) {
            QStringList remaining = two;
            for (const QString &line : three)
                remaining.removeAll(line);
            if (!remaining.isEmpty())
                writeWeeklyLog(QStringLiteral("w2.%1.w2.<%2>").arg(encode(remaining), stts));
        }
        if (!three.isEmpty())
            writeWeeklyLog(QStringLiteral("w3.%1.w3.<%2>").arg(encode(three), stts));
    }

    // save_score: the persistent logs are rebuilt from every mode's .1/.2/.3
    // files, not just from the mode that was displayed most recently.
    PracticeLogs logs;
    const QString confDir = m_paths.topicConfDir(m_tlng, m_topic);
    logs.load(confDir);
    const QDir dir(practiceDir());
    const QStringList oneFiles = dir.entryList({QStringLiteral("*.1")}, QDir::Files,
                                                QDir::Name);
    const QStringList twoFiles = dir.entryList({QStringLiteral("*.2")}, QDir::Files,
                                                QDir::Name);
    const QStringList threeFiles = dir.entryList({QStringLiteral("*.3")}, QDir::Files,
                                                  QDir::Name);

    auto uniqueSorted = [](const QStringList &values) {
        QStringList result = values;
        result.removeDuplicates();
        result.sort();
        return result;
    };
    auto collect = [&](const QStringList &files, QHash<QString, int> *counts) {
        QStringList values;
        for (const QString &file : files) {
            const QStringList lines = readPracticeFile(file);
            values.append(lines);
            if (counts) {
                for (const QString &line : uniqueSorted(lines))
                    ++(*counts)[line];
            }
        }
        return uniqueSorted(values);
    };

    QHash<QString, int> firstCounts;
    const QStringList allFirst = collect(oneFiles, &firstCounts);
    QStringList log1;
    if (oneFiles.size() >= 3) {
        for (const QString &line : allFirst) {
            if (firstCounts.value(line) >= 3)
                log1.append(line);
        }
    }
    const QStringList sentenceFirst = uniqueSorted(readPracticeFile(QStringLiteral("e.1")));
    log1.append(sentenceFirst);
    logs.setRawLog(1, log1.join(QLatin1Char('\n')));

    if (!twoFiles.isEmpty())
        logs.setRawLog(2, collect(twoFiles, nullptr).join(QLatin1Char('\n')));
    if (!threeFiles.isEmpty())
        logs.setRawLog(3, collect(threeFiles, nullptr).join(QLatin1Char('\n')));

    if (step == 3) {
        QStringList log2 = logs.entries(2);
        const QStringList hardLines = logs.entries(3);
        const QSet<QString> hard(hardLines.cbegin(), hardLines.cend());
        log2.erase(std::remove_if(log2.begin(), log2.end(), [&](const QString &line) {
            return hard.contains(line);
        }), log2.end());
        logs.setRawLog(2, log2.join(QLatin1Char('\n')));
    }
    logs.save(confDir);
}

void PracticeController::updatePracticeStats(int icon) const
{
    if (m_mode < 1 || m_mode > 5)
        return;
    QDir().mkpath(practiceDir());
    QFile iconFile(practiceDir() + QStringLiteral("/.%1").arg(m_mode));
    if (iconFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        iconFile.write(QByteArray::number(icon));
        iconFile.write("\n");
        iconFile.close();
    }
    SqliteDatabase db(m_paths.topicDb(m_tlng, m_topic));
    if (!db.open())
        return;
    const QString prefix = QStringLiteral("Pract%1").arg(m_mode);
    db.execute(QStringLiteral("update Practice_stats set %1_icon=%2, %1_sess=%1_sess+1;")
                   .arg(prefix).arg(icon));
}

int PracticeController::learnedBefore() const
{
    QFile file(practiceDir() + QLatin1Char('/') + practiceName(m_mode)
               + QStringLiteral(".l"));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return 0;
    return QString::fromUtf8(file.readAll()).trimmed().toInt();
}

int PracticeController::iconForProgress(int value) const
{
    int n = 1;
    int boundary = 1;
    while (n < 21) {
        if (value <= boundary)
            return n;
        boundary += 5;
        ++n;
    }
    return 20;
}

int PracticeController::practiceIcon(int mode) const
{
    if (mode < 1 || mode > 5 || m_tlng.isEmpty() || m_topic.isEmpty())
        return 1;
    const int icon = readPracticeFile(QStringLiteral(".%1").arg(mode)).value(0).toInt();
    return icon >= 1 && icon <= 21 ? icon : 1;
}

int PracticeController::practiceRemaining(int mode) const
{
    if (mode < 1 || mode > 5 || m_tlng.isEmpty() || m_topic.isEmpty())
        return 0;
    const QString name = practiceName(mode);
    const QStringList all = readPracticeFile(name + QStringLiteral(".0"));
    if (all.isEmpty())
        return 0;
    const int learned = readPracticeFile(name + QStringLiteral(".1")).size();
    const qsizetype remaining = all.size() - static_cast<qsizetype>(learned);
    return remaining > 0 ? static_cast<int>(remaining) : 0;
}

QVariantMap PracticeController::itemMap(const Item &item)
{
    return {
        {QStringLiteral("trgt"), item.trgt},
        {QStringLiteral("srce"), item.srce},
        {QStringLiteral("exmp"), item.exmp},
        {QStringLiteral("defn"), item.defn},
        {QStringLiteral("note"), item.note},
        {QStringLiteral("wrds"), item.wrds},
        {QStringLiteral("grmr"), item.grmr},
        {QStringLiteral("tags"), item.tags},
        {QStringLiteral("mark"), item.mark},
        {QStringLiteral("link"), item.link},
        {QStringLiteral("cdid"), item.cdid},
        {QStringLiteral("type"), item.type},
    };
}

QString PracticeController::cleanAnswer(const QString &value)
{
    QString cleaned = value.toLower();
    cleaned.remove(QRegularExpression(QStringLiteral("[,!\\?¿¡\\(\\)\\\"\\-\\[\\]\\.\\:\\|“”&:]")));
    return cleaned.simplified();
}

int PracticeController::sentenceMatch(const QString &typed, const QString &target)
{
    QStringList out = splitWords(cleanAnswer(typed));
    QStringList in = splitWords(cleanAnswer(target));
    if (in.size() > 6) {
        out.erase(std::remove_if(out.begin(), out.end(), [](const QString &word) {
            return word.size() <= 1;
        }), out.end());
        in.erase(std::remove_if(in.begin(), in.end(), [](const QString &word) {
            return word.size() <= 1;
        }), in.end());
    }
    if (in.isEmpty())
        return 0;
    int hits = 0;
    for (const QString &word : out) {
        if (in.contains(word))
            ++hits;
    }
    return 100 * hits / in.size();
}

int PracticeController::categoryForAnswer(const QVariant &answerValue, int *percentage) const
{
    if (percentage)
        *percentage = -1;
    if (m_roundIndex < 0 || m_roundIndex >= m_roundItems.size())
        return 3;

    const Item &item = m_roundItems.at(m_roundIndex);
    if (m_mode == 5) {
        if (answerValue.typeId() == QMetaType::Bool) {
            const bool known = answerValue.toBool();
            if (percentage)
                *percentage = known ? 100 : 0;
            return known ? 1 : 3;
        }
        const int matched = sentenceMatch(answerValue.toString(), item.trgt);
        if (percentage)
            *percentage = matched;
        return matched >= 90 ? 1 : matched >= 50 ? 2 : 3;
    }

    bool known = false;
    if (m_mode == 2 && answerValue.canConvert<int>()
        && answerValue.typeId() != QMetaType::QString) {
        const int index = answerValue.toInt();
        known = index >= 0 && index < m_options.size()
            && m_options.at(index) == expectedAnswer();
    } else if (answerValue.typeId() == QMetaType::Bool) {
        known = answerValue.toBool();
    } else {
        known = answerValue.toString().trimmed() == expectedAnswer().trimmed();
    }
    return known ? 1 : 3;
}

} // namespace storage
} // namespace idiomind
