// F5 - AddNoteFlow: pipeline de agregar nota (sin UI).
// Autoridad: Dev/ifs/mods/add/add.sh process().
//
// Flujo: text → clean → translate → grammar colorize → build Item → append.
// TTS e imágenes son opcionales (providers externos).
#pragma once

#include "core/config/AppPaths.h"
#include "core/topic/Item.h"
#include "services/impl/TranslationService.h"
#include "services/impl/GrammarColorizer.h"

#include <QObject>
#include <QString>

namespace idiomind {
namespace services {

struct AddNoteResult {
    bool ok = false;
    core::Item item;
    QString error;
};

class AddNoteFlow : public QObject
{
    Q_OBJECT
public:
    AddNoteFlow(core::AppPaths paths,
                TranslationService *transSvc,
                QObject *parent = nullptr);

    // Procesa un texto y devuelve el Item listo para añadir.
    // topicName se usa para guardar word pairs en tlngdb.
    AddNoteResult process(const QString &text, const QString &tlng,
                          const QString &topicName);

    // Limpieza de texto (replica clean_0..clean_9 del Bash).
    static QString cleanText(const QString &input);

private:
    core::AppPaths m_paths;
    TranslationService *m_transSvc;

    // sentence_p: tokeniza, traduce, colorea gramaticalmente.
    void sentencePartition(const QString &trgt, const QString &srce,
                           const QString &tlng, const QString &topicName,
                           QString *grmr, QString *wrds);

    // word_p: traduce una palabra.
    void wordPartition(const QString &trgt, const QString &tlng,
                       const QString &topicName, QString *srceOut);

    // Guarda word pair en tlngdb (replica lógica de sentence_p/word_p).
    void saveWordPair(const QString &word, const QString &translation,
                      const QString &example, const QString &tlng);
};

} // namespace services
} // namespace idiomind
