// F4 - PracticeController: modos de práctica 1..5.
// Fuente de comportamiento: practice/strt.sh.
#pragma once

#include "core/config/AppPaths.h"
#include "core/learning/PracticeLogs.h"
#include "core/topic/Item.h"

#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>

namespace idiomind {
namespace storage {

struct PracticeSession {
    int mode = 0;
    QList<core::Item> items;
    int total = 0;
    int correct = 0;
};

class PracticeController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool active READ active NOTIFY stateChanged)
    Q_PROPERTY(bool answered READ answered NOTIFY stateChanged)
    Q_PROPERTY(bool finished READ finished NOTIFY stateChanged)
    Q_PROPERTY(bool awaitingGroupDecision READ awaitingGroupDecision NOTIFY stateChanged)
    Q_PROPERTY(bool learningMode READ learningMode NOTIFY stateChanged)
    Q_PROPERTY(bool sourceToTarget READ sourceToTarget NOTIFY stateChanged)
    Q_PROPERTY(int mode READ mode NOTIFY stateChanged)
    Q_PROPERTY(int round READ round NOTIFY stateChanged)
    Q_PROPERTY(int currentIndex READ currentIndex NOTIFY stateChanged)
    Q_PROPERTY(int total READ total NOTIFY stateChanged)
    Q_PROPERTY(int correct READ correct NOTIFY stateChanged)
    Q_PROPERTY(int score READ score NOTIFY stateChanged)
    Q_PROPERTY(double progress READ progress NOTIFY stateChanged)
    Q_PROPERTY(QVariantMap currentItem READ currentItem NOTIFY itemChanged)
    Q_PROPERTY(QString question READ question NOTIFY itemChanged)
    Q_PROPERTY(QString response READ response NOTIFY itemChanged)
    Q_PROPERTY(QString expectedAnswer READ expectedAnswer NOTIFY itemChanged)
    Q_PROPERTY(QStringList options READ options NOTIFY itemChanged)
    Q_PROPERTY(QString image READ image NOTIFY itemChanged)
    Q_PROPERTY(QString audioSource READ audioSource NOTIFY itemChanged)
    Q_PROPERTY(QString state READ state NOTIFY stateChanged)
    Q_PROPERTY(QVariantMap result READ result NOTIFY resultChanged)
    Q_PROPERTY(QVariantList results READ results NOTIFY resultChanged)
    Q_PROPERTY(int practiceRevision READ practiceRevision NOTIFY stateChanged)

public:
    explicit PracticeController(core::AppPaths paths, QObject *parent = nullptr);

    bool active() const { return m_active; }
    bool answered() const { return m_answered; }
    bool finished() const { return m_finished; }
    bool awaitingGroupDecision() const { return m_awaitingGroupDecision; }
    bool learningMode() const { return m_groupMode; }
    bool sourceToTarget() const { return m_reverse; }
    int mode() const { return m_mode; }
    int round() const { return m_round; }
    int currentIndex() const { return m_roundIndex; }
    int total() const { return m_session.total; }
    int correct() const { return m_session.correct; }
    int score() const;
    double progress() const;
    QVariantMap currentItem() const;
    QString question() const { return m_question; }
    QString response() const { return m_response; }
    QString expectedAnswer() const;
    QStringList options() const { return m_options; }
    QString image() const { return m_image; }
    QString audioSource() const { return m_audioSource; }
    QString state() const;
    QVariantMap result() const { return m_result; }
    QVariantList results() const { return m_results; }
    int practiceRevision() const { return m_processed; }

    void setContext(const QString &tlng, const QString &topic);

    Q_INVOKABLE bool start(int mode);
    Q_INVOKABLE bool hasConfiguration(int mode) const;
    Q_INVOKABLE bool configure(int mode, bool learning, bool sourceToTarget);
    Q_INVOKABLE bool submit(const QVariant &answer);
    Q_INVOKABLE bool answer(const QVariant &answer);
    Q_INVOKABLE bool mark(bool known);
    Q_INVOKABLE bool next();
    Q_INVOKABLE bool nextItem();
    Q_INVOKABLE bool restart();
    Q_INVOKABLE bool restartAll();
    Q_INVOKABLE bool again();
    Q_INVOKABLE bool continueGroup();
    Q_INVOKABLE void acknowledgeResult();
    Q_INVOKABLE void stop();
    Q_INVOKABLE int practiceIcon(int mode) const;
    Q_INVOKABLE int practiceRemaining(int mode) const;

    // API de almacenamiento existente, conservada para consumidores no-QML.
    PracticeSession beginMode(const QString &tlng, const QString &topic, int mode);

    void recordResult(const QString &tlng, const QString &topic,
                      const QString &trgt, bool correct, int mode);

    void finishSession(const QString &tlng, const QString &topic,
                       PracticeSession &session);

    QStringList loadPractList(const QString &tlng, const QString &topic,
                              int mode) const;

    bool savePractList(const QString &tlng, const QString &topic,
                       int mode, const QStringList &items) const;

signals:
    void stateChanged();
    void itemChanged();
    void resultChanged();
    void finished(int correct, int total);

private:
    core::AppPaths m_paths;
    QString m_tlng;
    QString m_topic;
    int m_mode = 0;
    int m_round = 0;
    int m_roundIndex = -1;
    int m_processed = 0;
    int m_easy = 0;
    int m_learn = 0;
    int m_hard = 0;
    bool m_reverse = false;
    bool m_groupMode = false;
    bool m_awaitingGroupDecision = false;
    bool m_active = false;
    bool m_answered = false;
    bool m_finished = false;
    QString m_question;
    QString m_response;
    QStringList m_options;
    QString m_image;
    QString m_audioSource;
    QStringList m_failed;
    QList<core::Item> m_roundItems;
    QVariantMap m_result;
    QVariantList m_results;
    PracticeSession m_session;
    QStringList m_groupTargets;
    QStringList m_currentGroup;
    int m_sectionTotal = 0;

    QList<core::Item> selectItems(const QString &tlng, const QString &topic,
                                  int mode) const;
    bool startSession(const QString &tlng, const QString &topic, int mode);
    bool prepareNotes(int mode);
    bool prepareGroup();
    bool startGroup(const QStringList &targets);
    void showGroupDecision();
    bool isLocked(int mode) const;
    void loadCurrentItem();
    bool submitAnswer(const QVariant &answer);
    bool submitCategory(int category, int percentage = -1);
    bool advanceRound();
    void finishPractice(bool completed);
    void clearCurrentItem();
    void writeOutcome(int category, const QString &trgt, int round) const;
    void appendPracticeLine(const QString &name, const QString &value) const;
    void rewritePracticeFile(const QString &name, const QStringList &lines) const;
    void writeSessionLock(bool completed) const;
    void updatePracticeStats(int icon) const;
    void writeWeeklyLog(const QString &line) const;
    void writePracticeLogs(int step) const;
    QString practiceDir() const;
    QString practiceName(int mode) const;
    QStringList readPracticeFile(const QString &name) const;
    QList<core::Item> itemsForTargets(const QStringList &targets) const;
    static QVariantMap itemMap(const core::Item &item);
    static QString cleanAnswer(const QString &value);
    static int sentenceMatch(const QString &typed, const QString &target);
    int categoryForAnswer(const QVariant &answer, int *percentage) const;
    int learnedBefore() const;
    int iconForProgress(int value) const;
};

} // namespace storage
} // namespace idiomind
