#include "services/impl/PlaybackController.h"

#include "storage/dbs/SqliteDatabase.h"
#include "storage/topic/TopicDataRepository.h"

#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QUrl>
#include <QSet>

namespace idiomind::services {

PlaybackController::PlaybackController(core::AppPaths paths, QObject *parent)
    : QObject(parent)
    , m_paths(std::move(paths))
    , m_audioOutput(this)
{
    m_player.setAudioOutput(&m_audioOutput);
    connect(&m_player, &QMediaPlayer::playbackStateChanged, this,
            [this](QMediaPlayer::PlaybackState) { emit stateChanged(); });
    connect(&m_player, &QMediaPlayer::mediaStatusChanged, this,
            [this](QMediaPlayer::MediaStatus status) {
                if (status == QMediaPlayer::EndOfMedia)
                    next();
            });
    connect(&m_player, &QMediaPlayer::errorOccurred, this,
            [this](QMediaPlayer::Error, const QString &message) {
                emit errorChanged(message);
            });
}

void PlaybackController::setContext(const QString &tlng, const QString &topic)
{
    if (m_tlng == tlng && m_topic == topic)
        return;
    stop();
    m_tlng = tlng;
    m_topic = topic;
    m_queue.clear();
    m_currentIndex = -1;
    emit queueChanged();
    emit itemChanged();
}

void PlaybackController::setRepeat(bool value)
{
    if (m_repeat == value)
        return;
    m_repeat = value;
    emit stateChanged();
}

void PlaybackController::setAudioEnabled(bool value)
{
    if (m_audioEnabled == value)
        return;
    m_audioEnabled = value;
    if (!m_audioEnabled)
        m_player.stop();
    emit stateChanged();
}

bool PlaybackController::start(const QVariantMap &options)
{
    m_player.stop();
    m_queue = buildQueue(options);
    m_currentIndex = m_queue.isEmpty() ? -1 : 0;
    m_active = !m_queue.isEmpty();
    emit queueChanged();
    emit itemChanged();
    emit stateChanged();
    if (m_active)
        loadCurrent(true);
    return m_active;
}

void PlaybackController::play()
{
    if (!m_active) {
        if (m_queue.isEmpty())
            return;
        m_active = true;
        if (m_currentIndex < 0)
            m_currentIndex = 0;
        loadCurrent(true);
        return;
    }
    if (m_audioEnabled && !audioSource().isEmpty())
        m_player.play();
    emit stateChanged();
}

void PlaybackController::pause()
{
    m_player.pause();
    emit stateChanged();
}

void PlaybackController::stop()
{
    m_player.stop();
    if (!m_active)
        return;
    m_active = false;
    emit stateChanged();
}

void PlaybackController::next()
{
    if (m_queue.isEmpty())
        return;
    if (m_currentIndex + 1 >= m_queue.size()) {
        if (!m_repeat) {
            m_player.stop();
            m_active = false;
            emit stateChanged();
            return;
        }
        m_currentIndex = 0;
    } else {
        ++m_currentIndex;
    }
    loadCurrent(m_active);
}

void PlaybackController::previous()
{
    if (m_queue.isEmpty())
        return;
    if (m_currentIndex > 0)
        --m_currentIndex;
    else if (m_repeat)
        m_currentIndex = m_queue.size() - 1;
    loadCurrent(m_active);
}

QString PlaybackController::target() const
{
    return currentItem().value(QStringLiteral("trgt")).toString();
}

QString PlaybackController::source() const
{
    return currentItem().value(QStringLiteral("srce")).toString();
}

QString PlaybackController::audioSource() const
{
    if (m_currentIndex < 0 || m_currentIndex >= m_queue.size())
        return {};
    return resolveAudio(m_queue.at(m_currentIndex));
}

QVariantMap PlaybackController::currentItem() const
{
    if (m_currentIndex < 0 || m_currentIndex >= m_queue.size())
        return {};
    const core::Item &item = m_queue.at(m_currentIndex);
    return {
        {QStringLiteral("trgt"), item.trgt},
        {QStringLiteral("srce"), item.srce},
        {QStringLiteral("cdid"), item.cdid},
        {QStringLiteral("audioSource"), resolveAudio(item)}
    };
}

void PlaybackController::loadCurrent(bool autoplay)
{
    emit itemChanged();
    if (!autoplay || !m_audioEnabled)
        return;
    const QString path = audioSource();
    if (path.isEmpty()) {
        emit errorChanged(QStringLiteral("No hay audio disponible para este elemento"));
        return;
    }
    m_player.setSource(QUrl::fromLocalFile(path));
    m_player.play();
}

QList<core::Item> PlaybackController::buildQueue(const QVariantMap &options) const
{
    storage::TopicDataRepository repo(m_paths);
    const auto decoded = repo.loadItems(m_tlng, m_topic);
    const QStringList learning = repo.learningList(m_tlng, m_topic);
    const QStringList learnt = repo.learntList(m_tlng, m_topic);
    const QStringList marks = repo.marksList(m_tlng, m_topic);
    const bool words = options.value(QStringLiteral("words")).toBool();
    const bool sentenceOption = options.value(QStringLiteral("sentences")).toBool();
    const bool marked = options.value(QStringLiteral("marked")).toBool();
    const bool learn = options.value(QStringLiteral("learning")).toBool();
    const bool difficult = options.value(QStringLiteral("difficult")).toBool();

    QSet<QString> wordSet;
    QSet<QString> sentenceSet;
    storage::SqliteDatabase db(m_paths.topicDb(m_tlng, m_topic));
    if (db.open()) {
        QSqlQuery wordQuery = db.query(QStringLiteral("select list from words;"));
        while (wordQuery.next()) wordSet.insert(wordQuery.value(0).toString());
        QSqlQuery sentenceQuery = db.query(QStringLiteral("select list from sentences;"));
        while (sentenceQuery.next()) sentenceSet.insert(sentenceQuery.value(0).toString());
    }
    QSet<QString> learntSet(learnt.cbegin(), learnt.cend());
    QStringList log2;
    QStringList log3;
    for (int n = 2; n <= 3; ++n) {
        QFile log(m_paths.topicConfDir(m_tlng, m_topic)
                  + QStringLiteral("/practice/log%1").arg(n));
        if (log.open(QIODevice::ReadOnly | QIODevice::Text)) {
            const QStringList lines = QString::fromUtf8(log.readAll())
                                           .split(QLatin1Char('\n'), Qt::SkipEmptyParts);
            (n == 2 ? log2 : log3) = lines;
        }
    }
    QSet<QString> log3Set(log3.cbegin(), log3.cend());
    QHash<QString, core::Item> itemByTarget;
    for (const auto &decodedItem : decoded)
        itemByTarget.insert(decodedItem.item.trgt, decodedItem.item);

    QList<core::Item> queue;
    const auto appendTargets = [&](const QStringList &targets) {
        for (const QString &target : targets) {
            if (itemByTarget.contains(target))
                queue.append(itemByTarget.value(target));
        }
    };

    if (words || sentenceOption) {
        if (words && sentenceOption) {
            appendTargets(learning);
        } else if (words) {
            QStringList selected;
            for (const QString &target : learning)
                if (!sentenceSet.contains(target)) selected.append(target);
            appendTargets(selected);
        } else {
            QStringList selected;
            for (const QString &target : learning)
                if (!wordSet.contains(target)) selected.append(target);
            appendTargets(selected);
        }
    }
    if (marked) {
        QStringList selected;
        for (const QString &target : marks)
            if (!learntSet.contains(target)) selected.append(target);
        appendTargets(selected);
    }
    if (learn) {
        QStringList selected;
        for (const QString &target : log2)
            if (!log3Set.contains(target)) selected.append(target);
        appendTargets(selected);
    }
    if (difficult)
        appendTargets(log3);
    return queue;
}

QString PlaybackController::resolveAudio(const core::Item &item) const
{
    const QString topicPath = m_paths.topicDataDir(m_tlng, m_topic)
        + QLatin1Char('/') + item.cdid + QStringLiteral(".mp3");
    if (QFileInfo::exists(topicPath))
        return topicPath;
    const QString sharedPath = m_paths.sharedDirForLanguage(m_tlng)
        + QStringLiteral("/audio/") + item.trgt.toLower() + QStringLiteral(".mp3");
    return QFileInfo::exists(sharedPath) ? sharedPath : QString();
}

} // namespace idiomind::services
