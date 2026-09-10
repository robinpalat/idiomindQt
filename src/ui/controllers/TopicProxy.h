// F6-B.1/F6-B.2 - TopicProxy: fachada delgada para QML.
// Autoridad: doc 09 §5.1 (TopicProxy), chng.sh (topic selector).
//
// Expone a QML: lista de topics, topic activo, ítems del índice.
// NO contiene lógica de negocio; delega en TopicRepository y modelos.
#pragma once

#include "core/config/AppPaths.h"
#include "core/topic/Topic.h"
#include "storage/topic/TopicRepository.h"
#include "storage/topic/TopicDataRepository.h"
#include "storage/topic/ReviewRepository.h"
#include "storage/PracticeController.h"
#include "storage/SharedRepository.h"
#include "storage/LearningEngine.h"
#include "storage/interchange/LegacyImporter.h"
#include "services/impl/PlaybackController.h"
#include "ui/models/TopicListModel.h"
#include "ui/models/IndexListModel.h"
#include "ui/models/TaskListModel.h"

#include <QObject>
#include <QString>

namespace idiomind {
namespace ui {

class TopicProxy : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString activeTopic READ activeTopic NOTIFY activeTopicChanged)
    Q_PROPERTY(QString activeLanguage READ activeLanguage WRITE setActiveLanguage NOTIFY activeLanguageChanged)
    Q_PROPERTY(int topicCount READ topicCount NOTIFY topicsChanged)
    Q_PROPERTY(TopicListModel* topicModel READ topicModel CONSTANT)
    Q_PROPERTY(IndexListModel* indexModel READ indexModel CONSTANT)
    Q_PROPERTY(QString note READ note NOTIFY noteChanged)
    Q_PROPERTY(TaskListModel* taskModel READ taskModel CONSTANT)
    Q_PROPERTY(IndexListModel* learntModel READ learntModel CONSTANT)
    Q_PROPERTY(QString stts READ stts NOTIFY metadataChanged)
    Q_PROPERTY(QString nwrd READ nwrd NOTIFY metadataChanged)
    Q_PROPERTY(QString nsnt READ nsnt NOTIFY metadataChanged)
    Q_PROPERTY(QString nimg READ nimg NOTIFY metadataChanged)
    Q_PROPERTY(QString dtec READ dtec NOTIFY metadataChanged)
    Q_PROPERTY(QString autr READ autr NOTIFY metadataChanged)
    Q_PROPERTY(QString slng READ slng NOTIFY metadataChanged)
    Q_PROPERTY(QString tlng READ tlng NOTIFY metadataChanged)
    Q_PROPERTY(QString ctgy READ ctgy NOTIFY metadataChanged)
    Q_PROPERTY(QString levl READ levl NOTIFY metadataChanged)
    Q_PROPERTY(QString nsze READ nsze NOTIFY metadataChanged)
    Q_PROPERTY(QString actionLabel READ actionLabel NOTIFY metadataChanged)
    Q_PROPERTY(bool reviewPanelVisible READ reviewPanelVisible NOTIFY metadataChanged)
    Q_PROPERTY(QString levelLabel READ levelLabel NOTIFY metadataChanged)
    Q_PROPERTY(QString reviewLabel READ reviewLabel NOTIFY metadataChanged)
    Q_PROPERTY(int reviewDays READ reviewDays NOTIFY metadataChanged)
    Q_PROPERTY(int reviewPercent READ reviewPercent NOTIFY metadataChanged)
    Q_PROPERTY(QString reviewIntervalsLabel READ reviewIntervalsLabel NOTIFY metadataChanged)
    Q_PROPERTY(bool reviewReady READ reviewReady NOTIFY metadataChanged)
    Q_PROPERTY(QString topicLineColor READ topicLineColor NOTIFY metadataChanged)
    Q_PROPERTY(QString levelLabelColor READ levelLabelColor NOTIFY metadataChanged)
    Q_PROPERTY(storage::PracticeController* practiceController READ practiceController CONSTANT)
    Q_PROPERTY(services::PlaybackController* playbackController READ playbackController CONSTANT)

public:
    explicit TopicProxy(core::AppPaths paths, QObject *parent = nullptr);

    QString activeLanguage() const { return m_language; }
    void setActiveLanguage(const QString &tlng);

    QString activeTopic() const { return m_activeTopic; }

    TopicListModel* topicModel() const { return m_topicModel; }
    IndexListModel* indexModel() const { return m_indexModel; }
    TaskListModel* taskModel() const { return m_taskModel; }
    IndexListModel* learntModel() const { return m_learntModel; }
    storage::PracticeController* practiceController() const { return m_practiceController; }
    services::PlaybackController* playbackController() const { return m_playbackController; }

    int topicCount() const { return m_topicModel->count(); }

    Q_INVOKABLE void refreshTopics();
    Q_INVOKABLE void selectTopic(const QString &name);
    Q_INVOKABLE QString activeTopicStts() const;
    Q_INVOKABLE void refreshTasks();
    Q_INVOKABLE void activateTask(const QString &action, const QString &topic);

    // F4.1: importar un archivo .idmnd
    Q_INVOKABLE bool importIdmnd(const QString &filePath);

    // F6-B.3: nota del topic activo
    QString note() const { return m_note; }
    Q_INVOKABLE void saveNote(const QString &text);

    // F6-B.4: metadata del topic activo
    QString stts() const { return m_stts; }
    QString nwrd() const { return m_nwrd; }
    QString nsnt() const { return m_nsnt; }
    QString nimg() const { return m_nimg; }
    QString dtec() const { return m_dtec; }
    QString autr() const { return m_autr; }
    QString slng() const { return m_slng; }
    QString tlng() const { return m_tlng; }
    QString ctgy() const { return m_ctgy; }
    QString levl() const { return m_levl; }
    QString nsze() const { return m_nsze; }

    // F6-B.6: acción contextual del footer
    QString actionLabel() const;
    bool reviewPanelVisible() const { return m_reviewPanelVisible; }
    QString levelLabel() const { return m_levelLabel; }
    QString reviewLabel() const { return m_reviewLabel; }
    int reviewDays() const { return m_reviewDays; }
    int reviewPercent() const { return m_reviewPercent; }
    QString reviewIntervalsLabel() const { return m_reviewIntervalsLabel; }
    bool reviewReady() const { return m_reviewReady; }
    QString topicLineColor() const { return m_topicLineColor; }
    QString levelLabelColor() const { return m_levelLabelColor; }
    Q_INVOKABLE bool performAction();

    // F6-C.3: marcar un ítem individual como aprendido
    Q_INVOKABLE bool markItemAsLearned(const QString &trgt);

    // F6-C.4: reiniciar todas las prácticas del topic activo
    Q_INVOKABLE bool restartPractice();

    // F6-B.8: persistencia del topic activo
    Q_INVOKABLE void loadActiveTopic();
    Q_INVOKABLE void saveActiveTopic();

    // F6-C.1: helper stts → asset path
    Q_INVOKABLE QString stateImage(const QString &stts) const;

    // Viewer: obtener datos completos de un Item por posición en el índice
    Q_INVOKABLE QVariantMap itemAt(int index) const;
    Q_INVOKABLE int itemCount() const;

signals:
    void activeTopicChanged(const QString &name);
    void activeLanguageChanged(const QString &tlng);
    void topicsChanged();
    void indexChanged();
    void noteChanged();
    void metadataChanged();
    void tasksChanged();
    void taskActionRequested(const QString &action);
    void learntChanged();

private:
    void loadIndexForTopic(const QString &topic);
    void loadNoteForTopic(const QString &topic);
    void loadMetadataForTopic(const QString &topic);
    void loadLearntForTopic(const QString &topic);

    core::AppPaths m_paths;
    storage::TopicRepository m_repo;
    storage::TopicDataRepository m_dataRepo;
    storage::SharedRepository m_shrRepo;
    storage::LearningEngine m_engine;
    TopicListModel *m_topicModel;
    IndexListModel *m_indexModel;
    IndexListModel *m_learntModel;
    TaskListModel *m_taskModel;
    storage::PracticeController *m_practiceController;
    services::PlaybackController *m_playbackController;
    QString m_language;
    QString m_activeTopic;
    QString m_note;
    QString m_stts, m_nwrd, m_nsnt, m_nimg, m_dtec;
    QString m_autr, m_slng, m_tlng, m_ctgy, m_levl, m_nsze;
    bool m_reviewPanelVisible = false;
    QString m_levelLabel, m_reviewLabel, m_reviewIntervalsLabel, m_topicLineColor, m_levelLabelColor;
    int m_reviewDays = 0;
    int m_reviewPercent = 0;
    bool m_reviewReady = false;
};

} // namespace ui
} // namespace idiomind
