#include "ui/controllers/TopicProxy.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QTextStream>

namespace idiomind {
namespace ui {

namespace {

QString levelLabelFor(int status, int count)
{
    const QStringList levels = {
        QStringLiteral("Fresh Topic"),
        QStringLiteral("Fresh Topic"),
        QStringLiteral("Fresh Topic"),
        QStringLiteral("Fresh Topic"),
        QStringLiteral("Familiar Topic"),
        QStringLiteral("Familiar Topic"),
        QStringLiteral("Familiar Topic"),
        QStringLiteral("Familiar Topic"),
        QStringLiteral("Familiar Topic"),
        QStringLiteral("Mastered Topic")
    };
    if (status == 2)
        return QStringLiteral("Mastered Topic");
    return levels.value(qBound(0, count, 9));
}

QString reviewLabelFor(int status, int count)
{
    if (count <= 0)
        return {};
    const QStringList reviewing = {
        QStringLiteral("Aprendiendo..."),
        QStringLiteral("Revisando por primera vez"),
        QStringLiteral("Revisando por segunda vez"),
        QStringLiteral("Revisando por tercera vez"),
        QStringLiteral("Revisando por cuarta vez"),
        QStringLiteral("Revisando por quinta vez"),
        QStringLiteral("Revisando por sexta vez"),
        QStringLiteral("Revisando por séptima vez"),
        QStringLiteral("Revisión final"),
        QStringLiteral("Revisión final")
    };
    const QStringList waiting = {
        QString(),
        QStringLiteral("Esperando la primera revisión"),
        QStringLiteral("Esperando la segunda revisión"),
        QStringLiteral("Esperando la tercera revisión"),
        QStringLiteral("Esperando la cuarta revisión"),
        QStringLiteral("Esperando la quinta revisión"),
        QStringLiteral("Esperando la sexta revisión"),
        QStringLiteral("Esperando la séptima revisión"),
        QStringLiteral("Esperando la octava revisión"),
        QStringLiteral("Esperando la novena revisión"),
        QStringLiteral("Segundo recordatorio de revisión")
    };
    const QStringList ready = {
        QString(),
        QStringLiteral("Listo para la primera revisión"),
        QStringLiteral("Listo para la segunda revisión"),
        QStringLiteral("Listo para la tercera revisión"),
        QStringLiteral("Listo para la cuarta revisión"),
        QStringLiteral("Listo para la quinta revisión"),
        QStringLiteral("Listo para la sexta revisión"),
        QStringLiteral("Listo para la séptima revisión"),
        QStringLiteral("Listo para la octava revisión"),
        QStringLiteral("Listo para la revisión final"),
        QStringLiteral("Segundo recordatorio de revisión")
    };

    const int safeCount = qBound(0, count, 10);
    if (status == 3 || status == 4)
        return waiting.value(safeCount);
    if (status >= 7 && status <= 10)
        return ready.value(safeCount);
    if (status == 1 || status == 5 || status == 6)
        return reviewing.value(qMin(safeCount, reviewing.size() - 1));
    return {};
}

QString reviewIntervalsLabelFor(int count)
{
    const QStringList intervals = {
        QStringLiteral("4"), QStringLiteral("7"), QStringLiteral("7"),
        QStringLiteral("10"), QStringLiteral("15"), QStringLiteral("15"),
        QStringLiteral("20"), QStringLiteral("30")
    };
    QStringList rendered;
    for (int i = 0; i < intervals.size(); ++i) {
        const QString value = intervals.at(i);
        rendered.append(i + 1 == count
                            ? QStringLiteral("<u><b>%1</b></u>").arg(value)
                            : QStringLiteral("<span color='#888888'>%1</span>").arg(value));
    }
    return rendered.join(QStringLiteral(" | "));
}

} // namespace

TopicProxy::TopicProxy(core::AppPaths paths, QObject *parent)
    : QObject(parent)
    , m_paths(std::move(paths))
    , m_repo(m_paths)
    , m_dataRepo(m_paths)
    , m_shrRepo(m_paths)
    , m_engine(m_paths)
    , m_topicModel(new TopicListModel(this))
    , m_indexModel(new IndexListModel(this))
    , m_learntModel(new IndexListModel(this))
    , m_taskModel(new TaskListModel(this))
    , m_practiceController(new storage::PracticeController(m_paths, this))
    , m_playbackController(new services::PlaybackController(m_paths, this))
{
}

void TopicProxy::setActiveLanguage(const QString &tlng)
{
    if (m_language == tlng)
        return;
    m_language = tlng;
    m_practiceController->setContext(m_language, m_activeTopic);
    m_playbackController->setContext(m_language, m_activeTopic);
    emit activeLanguageChanged(tlng);
    refreshTopics();
}

void TopicProxy::refreshTopics()
{
    if (m_language.isEmpty())
        return;
    const core::TopicList topics = m_repo.listTopics(m_language);
    m_topicModel->setTopics(topics);
    emit topicsChanged();
}

void TopicProxy::selectTopic(const QString &name)
{
    if (name.isEmpty())
        return;
    if (m_activeTopic == name)
        return;
    m_activeTopic = name;
    m_practiceController->setContext(m_language, m_activeTopic);
    m_playbackController->setContext(m_language, m_activeTopic);
    loadIndexForTopic(name);
    loadNoteForTopic(name);
    loadMetadataForTopic(name);
    loadLearntForTopic(name);
    saveActiveTopic();
    emit activeTopicChanged(name);
}

void TopicProxy::loadIndexForTopic(const QString &topic)
{
    if (m_language.isEmpty() || topic.isEmpty()) {
        m_indexModel->clear();
        return;
    }
    const QStringList lines = m_dataRepo.loadIndex(m_language, topic);
    m_indexModel->loadFromIndexLines(lines);
    emit indexChanged();
}

QString TopicProxy::activeTopicStts() const
{
    if (m_activeTopic.isEmpty() || m_language.isEmpty())
        return {};
    const core::Topic topic = m_repo.loadTopic(m_language, m_activeTopic);
    return topic.stts();
}

void TopicProxy::loadNoteForTopic(const QString &topic)
{
    if (m_language.isEmpty() || topic.isEmpty()) {
        if (!m_note.isEmpty()) {
            m_note.clear();
            emit noteChanged();
        }
        return;
    }
    const QString newNote = m_dataRepo.readNote(m_language, topic);
    if (newNote != m_note) {
        m_note = newNote;
        emit noteChanged();
    }
}

void TopicProxy::saveNote(const QString &text)
{
    if (m_activeTopic.isEmpty() || m_language.isEmpty())
        return;
    if (m_dataRepo.writeNote(m_language, m_activeTopic, text)) {
        m_note = text;
        emit noteChanged();
    }
}

void TopicProxy::loadMetadataForTopic(const QString &topic)
{
    if (m_language.isEmpty() || topic.isEmpty()) {
        m_stts.clear(); m_nwrd.clear(); m_nsnt.clear(); m_nimg.clear();
        m_dtec.clear(); m_autr.clear(); m_slng.clear(); m_tlng.clear();
        m_ctgy.clear(); m_levl.clear(); m_nsze.clear();
        m_reviewPanelVisible = false;
        m_levelLabel.clear(); m_reviewLabel.clear(); m_reviewIntervalsLabel.clear();
        m_topicLineColor.clear(); m_levelLabelColor.clear();
        m_reviewDays = 0; m_reviewPercent = 0; m_reviewReady = false;
        emit metadataChanged();
        return;
    }
    const core::Topic t = m_repo.loadTopic(m_language, topic);
    const core::TopicInfo info = t.info();
    m_stts = t.stts();
    m_nwrd = info.nwrd();
    m_nsnt = info.nsnt();
    m_nimg = info.nimg();
    m_dtec = info.dtec();
    m_autr = info.autr();
    m_slng = info.slng();
    m_tlng = info.tlng();
    m_ctgy = info.ctgy();
    m_levl = info.levl();
    m_nsze = info.nsze();

    const int status = m_stts.toInt();
    storage::ReviewRepository reviewRepo(m_paths);
    const int count = reviewRepo.countFilled(m_language, topic);
    m_reviewPanelVisible = status == 3 || status == 4
                           || (status >= 7 && status <= 10);
    m_levelLabel = levelLabelFor(status, count);
    m_reviewLabel = reviewLabelFor(status, count);
    m_reviewDays = ReviewCalculator::daysToReviewAt(count);
    m_reviewPercent = m_engine.calculateReview(m_language, topic);
    m_reviewReady = m_reviewPercent >= 100;
    m_reviewIntervalsLabel = reviewIntervalsLabelFor(count);

    if (status == 2) {
        m_topicLineColor = QStringLiteral("#6A4C7E");
        m_levelLabelColor = QStringLiteral("#844DB1");
    } else if (status % 2 == 0) {
        m_topicLineColor = QStringLiteral("#7E5D4C");
        m_levelLabelColor = QStringLiteral("#A36A53");
    } else {
        m_topicLineColor = QStringLiteral("#62ACBA");
        m_levelLabelColor = QStringLiteral("#62ACBA");
    }

    emit metadataChanged();
}

void TopicProxy::refreshTasks()
{
    if (m_language.isEmpty()) {
        m_taskModel->clear();
        return;
    }

    m_shrRepo.ensureTables(m_language);
    QList<ui::TaskItem> tasks;

    const QStringList mastered = m_shrRepo.listOf(m_language,
                                                   storage::ReviewCalculatorKind::T10);
    auto append = [&](const QString &topic, const QString &tag, const QString &title,
                      const QString &action) {
        tasks.append({topic, tag, title, QStringLiteral("#607D8B"), action});
    };
    auto appendList = [&](const QStringList &topics, const QString &tag,
                          const QString &description, const QString &action,
                          bool excludeMastered) {
        for (int i = topics.size() - 1; i >= 0; --i) {
            const QString &topic = topics.at(i);
            if (excludeMastered && (mastered.contains(topic) || topic == m_activeTopic))
                continue;
            if (!topic.isEmpty())
                append(topic, tag, description + QStringLiteral(" — ") + topic, action);
        }
    };

    if (QFileInfo::exists(m_paths.tmpDir() + QStringLiteral("/tasks_init")))
        append({}, QStringLiteral("INFO"), QStringLiteral("Aprende a usar Idiomind"),
               QStringLiteral("help"));

    if (!m_activeTopic.isEmpty() && !mastered.contains(m_activeTopic)
        && !m_dataRepo.learningList(m_language, m_activeTopic).isEmpty()) {
        append(m_activeTopic, QStringLiteral("PLAY"),
               QStringLiteral("Reproducir — ") + m_activeTopic, QStringLiteral("play"));
        append(m_activeTopic, QStringLiteral("PRACTICE"),
               QStringLiteral("Practicar — ") + m_activeTopic, QStringLiteral("practice"));
    }

    appendList(m_shrRepo.listOfNumber(m_language, 8), QStringLiteral("T8"),
               QStringLiteral("Reanudar práctica"), QStringLiteral("practice"), true);
    appendList(m_shrRepo.listOfNumber(m_language, 6), QStringLiteral("T6"),
               QStringLiteral("Volver a practicar"), QStringLiteral("practice"), true);
    appendList(m_shrRepo.listOfNumber(m_language, 5), QStringLiteral("T5"),
               QStringLiteral("Practicar"), QStringLiteral("practice"), true);

    appendList(m_shrRepo.listOf(m_language, storage::ReviewCalculatorKind::T1),
               QStringLiteral("T1"), QStringLiteral("Revisar"), QStringLiteral("review"), false);
    appendList(m_shrRepo.listOf(m_language, storage::ReviewCalculatorKind::T2),
               QStringLiteral("T2"), QStringLiteral("Revisar urgentemente"), QStringLiteral("review"), false);
    appendList(m_shrRepo.listOf(m_language, storage::ReviewCalculatorKind::T3),
               QStringLiteral("T3"), QStringLiteral("Revisar"), QStringLiteral("review"), false);
    appendList(m_shrRepo.listOf(m_language, storage::ReviewCalculatorKind::T4),
               QStringLiteral("T4"), QStringLiteral("Revisar urgentemente"), QStringLiteral("review"), false);
    appendList(m_shrRepo.listOf(m_language, storage::ReviewCalculatorKind::T7),
               QStringLiteral("T7"), QStringLiteral("Finalizar revisión"), QStringLiteral("review"), false);

    m_taskModel->setTasks(tasks);
    emit tasksChanged();
}

void TopicProxy::activateTask(const QString &action, const QString &topic)
{
    m_taskModel->removeTask(topic, action);
    if (!topic.isEmpty())
        selectTopic(topic);
    emit taskActionRequested(action);
}

void TopicProxy::loadLearntForTopic(const QString &topic)
{
    if (m_language.isEmpty() || topic.isEmpty()) {
        m_learntModel->clear();
        return;
    }

    const QStringList learnt = m_dataRepo.learntList(m_language, topic);
    if (learnt.isEmpty()) {
        m_learntModel->clear();
        return;
    }

    const auto items = m_dataRepo.loadItems(m_language, topic);
    QMap<QString, QString> srceMap;
    for (const auto &d : items)
        srceMap.insert(d.item.trgt, d.item.srce);

    QStringList lines;
    for (const QString &trgt : learnt) {
        const QString srce = srceMap.value(trgt);
        lines.append(trgt);
        lines.append(QStringLiteral("FALSE"));
        lines.append(srce);
    }

    m_learntModel->loadFromIndexLines(lines);
    emit learntChanged();
}

// F6-B.6: acción contextual del footer (replica items_list.sh)
QString TopicProxy::actionLabel() const
{
    const int s = m_stts.toInt();

    if (s == 1)
        return QStringLiteral("Marcar Como Aprendido");
    if (s == 5 || s == 6)
        return QStringLiteral("Finalizar Repaso");
    if (s == 3 || s == 4 || s == 7 || s == 8 || s == 9 || s == 10)
        return QStringLiteral("Volver a Repasar");
    return {};
}

bool TopicProxy::performAction()
{
    if (m_activeTopic.isEmpty() || m_language.isEmpty())
        return false;

    const int s = m_stts.toInt();

    // mark_as_learned: stts ∈ {1, 5, 6}
    if (s == 1 || s == 5 || s == 6) {
        const auto result = m_engine.markAsLearned(m_language, m_activeTopic);
        if (!result.changed)
            return false;
    }
    // mark_to_learn: stts ∈ {3, 4, 7, 8, 9, 10}
    else if (s == 3 || s == 4 || s == 7 || s == 8 || s == 9 || s == 10) {
        if (!m_engine.markToLearn(m_language, m_activeTopic))
            return false;
    }
    else {
        return false;
    }

    // Refresh all views after operation
    loadMetadataForTopic(m_activeTopic);
    loadIndexForTopic(m_activeTopic);
    loadLearntForTopic(m_activeTopic);
    refreshTasks();

    return true;
}

bool TopicProxy::markItemAsLearned(const QString &trgt)
{
    if (trgt.isEmpty() || m_activeTopic.isEmpty() || m_language.isEmpty())
        return false;

    // Remove from learning
    m_dataRepo.removeLearningItem(m_language, m_activeTopic, trgt);

    // Insert into learnt
    m_dataRepo.insertLearntItem(m_language, m_activeTopic, trgt);

    // Rebuild index
    m_dataRepo.rebuildIndex(m_language, m_activeTopic, true);

    // Refresh models
    loadIndexForTopic(m_activeTopic);
    loadLearntForTopic(m_activeTopic);

    return true;
}

bool TopicProxy::restartPractice()
{
    if (m_activeTopic.isEmpty() || m_language.isEmpty())
        return false;

    // Replica strt.sh restart: elimina archivos de práctica y recrea logs
    const QString practiceDir = m_paths.topicConfDir(m_language, m_activeTopic)
                                + QStringLiteral("/practice");

    // Eliminar archivos de práctica (.0, .1, .2, .3, .srces, .df, .group, .lock, .l)
    const QStringList patterns = {
        "*.0", "*.1", "*.2", "*.3", "*.srces", "*.df",
        "*.group", "*.lock", "*.l", ".*", "a", "b", "c", "d", "e"
    };
    QDir dir(practiceDir);
    for (const QString &pat : patterns) {
        const QStringList files = dir.entryList({pat}, QDir::Files);
        for (const QString &f : files)
            QFile::remove(practiceDir + QLatin1Char('/') + f);
    }

    // Recrear logs vacíos
    for (int i = 1; i <= 3; ++i) {
        QFile log(practiceDir + QStringLiteral("/log%1").arg(i));
        log.open(QIODevice::WriteOnly | QIODevice::Truncate);
        log.close();
    }

    return true;
}

// F6-B.8: persistencia del topic activo ($DC_s/tpc)

void TopicProxy::loadActiveTopic()
{
    if (m_language.isEmpty())
        return;

    const QString tpcPath = m_paths.configRoot() + QStringLiteral("/tpc");
    QFile f(tpcPath);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        // Archivo no existe: sin topic activo
        return;
    }
    const QString name = QString::fromUtf8(f.readLine()).trimmed();
    f.close();

    if (name.isEmpty())
        return;

    // Verificar que el topic existe
    if (!m_repo.topicExists(m_language, name))
        return;

    // Seleccionar sin persistir (ya estamos leyendo de persistencia)
    if (m_activeTopic == name)
        return;
    m_activeTopic = name;
    m_practiceController->setContext(m_language, m_activeTopic);
    loadIndexForTopic(name);
    loadNoteForTopic(name);
    loadMetadataForTopic(name);
    loadLearntForTopic(name);
    emit activeTopicChanged(name);
}

void TopicProxy::saveActiveTopic()
{
    const QString tpcPath = m_paths.configRoot() + QStringLiteral("/tpc");
    QDir().mkpath(m_paths.configRoot());
    QFile f(tpcPath);
    if (f.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        f.write(m_activeTopic.toUtf8());
        f.write("\n");
        f.close();
    }
}

QString TopicProxy::stateImage(const QString &stts) const
{
    // Mapa stts → asset (replica de mkmn: img.{stts}.png)
    static const QMap<QString, QString> imageMap = {
        {"0",  QStringLiteral("qrc:/images/states/img.0.png")},
        {"1",  QStringLiteral("qrc:/images/states/img.1.png")},
        {"2",  QStringLiteral("qrc:/images/states/img.2.png")},
        {"3",  QStringLiteral("qrc:/images/states/img.3.png")},
        {"4",  QStringLiteral("qrc:/images/states/img.4.png")},
        {"5",  QStringLiteral("qrc:/images/states/img.5.png")},
        {"6",  QStringLiteral("qrc:/images/states/img.6.png")},
        {"7",  QStringLiteral("qrc:/images/states/img.7.png")},
        {"8",  QStringLiteral("qrc:/images/states/img.8.png")},
        {"9",  QStringLiteral("qrc:/images/states/img.9.png")},
        {"10", QStringLiteral("qrc:/images/states/img.10.png")},
        {"13", QStringLiteral("qrc:/images/states/img.13.png")},
    };
    return imageMap.value(stts, QStringLiteral("qrc:/images/states/img.13.png"));
}

bool TopicProxy::importIdmnd(const QString &filePath)
{
    if (filePath.isEmpty())
        return false;

    storage::LegacyImporter importer(m_paths);
    QString topicName;
    QString error;

    if (!importer.importFile(m_language, filePath, &topicName, &error)) {
        qWarning() << "Import failed:" << error;
        return false;
    }

    refreshTopics();
    return true;
}

QVariantMap TopicProxy::itemAt(int index) const
{
    if (m_activeTopic.isEmpty() || m_language.isEmpty() || index < 0)
        return {};

    const QStringList lines = m_dataRepo.loadIndex(m_language, m_activeTopic);
    const auto items = m_dataRepo.loadItems(m_language, m_activeTopic);

    // Mantener la misma correspondencia que IndexListModel: grupos de tres
    // líneas, ignorando los grupos cuyo texto visible está vacío.
    static QRegularExpression htmlRe(QStringLiteral("<[^>]*>"));
    int visibleIndex = 0;
    int dataIndex = 0;
    for (int lineIndex = 0; lineIndex + 2 < lines.size(); lineIndex += 3) {
        const QString textLine = lines.at(lineIndex).trimmed();
        if (textLine.isEmpty())
            continue;

        const QString plainTrgt = QString(textLine).remove(htmlRe).trimmed();
        const QString srceText = lines.at(lineIndex + 2).trimmed();

        // El índice se escribe recorriendo data en su orden original. Avanzar
        // el cursor evita volver a elegir el primer trgt cuando hay repetidos.
        while (dataIndex < items.size()) {
            const auto &d = items.at(dataIndex++);
            if (d.item.trgt != plainTrgt || d.item.srce.trimmed() != srceText)
                continue;

            if (visibleIndex == index) {
            QVariantMap map;
            map["trgt"] = d.item.trgt;
            map["srce"] = d.item.srce;
            map["exmp"] = d.item.exmp;
            map["defn"] = d.item.defn;
            map["note"] = d.item.note;
            map["wrds"] = d.item.wrds;
            map["grmr"] = d.item.grmr;
            map["tags"] = d.item.tags;
            map["mark"] = d.item.mark;
            map["link"] = d.item.link;
            map["type"] = d.item.type;
            map["cdid"] = d.item.cdid;
            const QString topicAudio = m_paths.topicDataDir(m_language, m_activeTopic)
                + QLatin1Char('/') + d.item.cdid + QStringLiteral(".mp3");
            const QString sharedAudio = m_paths.sharedDirForLanguage(m_language)
                + QStringLiteral("/audio/") + d.item.trgt.toLower() + QStringLiteral(".mp3");
            if (QFileInfo::exists(topicAudio))
                map["audioSource"] = topicAudio;
            else if (QFileInfo::exists(sharedAudio))
                map["audioSource"] = sharedAudio;
            return map;
            }

            ++visibleIndex;
            break;
        }
    }
    return {};
}

int TopicProxy::itemCount() const
{
    return m_indexModel ? m_indexModel->itemCount() : 0;
}

} // namespace ui
} // namespace idiomind
