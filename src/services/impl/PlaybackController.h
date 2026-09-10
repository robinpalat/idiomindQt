#pragma once

#include "core/config/AppPaths.h"
#include "core/topic/Item.h"

#include <QAudioOutput>
#include <QMediaPlayer>
#include <QObject>
#include <QVariantMap>

namespace idiomind::services {

class PlaybackController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool active READ active NOTIFY stateChanged)
    Q_PROPERTY(bool playing READ playing NOTIFY stateChanged)
    Q_PROPERTY(bool repeat READ repeat WRITE setRepeat NOTIFY stateChanged)
    Q_PROPERTY(bool audioEnabled READ audioEnabled WRITE setAudioEnabled NOTIFY stateChanged)
    Q_PROPERTY(int currentIndex READ currentIndex NOTIFY itemChanged)
    Q_PROPERTY(int totalItems READ totalItems NOTIFY queueChanged)
    Q_PROPERTY(QString target READ target NOTIFY itemChanged)
    Q_PROPERTY(QString source READ source NOTIFY itemChanged)
    Q_PROPERTY(QString audioSource READ audioSource NOTIFY itemChanged)
    Q_PROPERTY(QVariantMap currentItem READ currentItem NOTIFY itemChanged)

public:
    explicit PlaybackController(core::AppPaths paths, QObject *parent = nullptr);

    bool active() const { return m_active; }
    bool playing() const { return m_player.playbackState() == QMediaPlayer::PlayingState; }
    bool repeat() const { return m_repeat; }
    bool audioEnabled() const { return m_audioEnabled; }
    int currentIndex() const { return m_currentIndex; }
    int totalItems() const { return m_queue.size(); }
    QString target() const;
    QString source() const;
    QString audioSource() const;
    QVariantMap currentItem() const;

    void setContext(const QString &tlng, const QString &topic);
    void setRepeat(bool value);
    void setAudioEnabled(bool value);

    Q_INVOKABLE bool start(const QVariantMap &options = {});
    Q_INVOKABLE void play();
    Q_INVOKABLE void pause();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void next();
    Q_INVOKABLE void previous();

signals:
    void stateChanged();
    void queueChanged();
    void itemChanged();
    void errorChanged(const QString &message);

private:
    core::AppPaths m_paths;
    QString m_tlng;
    QString m_topic;
    QList<core::Item> m_queue;
    int m_currentIndex = -1;
    bool m_active = false;
    bool m_repeat = false;
    bool m_audioEnabled = true;
    QMediaPlayer m_player;
    QAudioOutput m_audioOutput;

    QList<core::Item> buildQueue(const QVariantMap &options) const;
    QString resolveAudio(const core::Item &item) const;
    void loadCurrent(bool autoplay);
};

} // namespace idiomind::services
