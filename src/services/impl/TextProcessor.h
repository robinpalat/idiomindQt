// F5.1 - TextProcessor: procesamiento de texto (clean_0..clean_9).
// Autoridad: Dev/ifs/mods/add/add.sh, Dev/ifs/mods/add/add.sh process(),
// Dev/mngr.sh edit dialogs.
//
// Mapa de uso:
//   clean_0 — srce (traducción) para palabras
//   clean_1 — trgt (target) para palabras
//   clean_2 — trgt/srce para oraciones + ediciones
//   clean_3 — nombre de topic (no en add-note, solo new_topic/rename)
//   clean_4 — texto raw de clipboard/OCR
//   clean_5 — MUERTO (no se usa)
//   clean_6 — texto OCR (tesseract)
//   clean_7 — texto CJK/Rusiano → split por oraciones
//   clean_8 — texto occidental → split por oraciones
//   clean_9 — texto editado/modificado (mngr.sh)
#pragma once

#include <QString>
#include <QStringList>

namespace idiomind {
namespace services {

class TextProcessor
{
public:
    // clean_0: limpia srce (traducción) para palabras.
    // remove *,|;!?[]&:<>+, ¡, HTML, normaliza espacios, /→-
    static QString cleanSource(const QString &input);

    // clean_1: limpia trgt (target) para palabras.
    // Similar a clean_0 pero también elimina ",(),.
    static QString cleanWordTarget(const QString &input);

    // clean_2: limpia trgt/srce para oraciones.
    //language-aware: ja/zh-cn/ru eliminan *, otros normalizan.
    // Elimina trailing .-, normaliza quotes, espacios.
    static QString cleanSentence(const QString &input, bool isCjk = false);

    // clean_3: limpia nombre de topic.
    // Elimina delimitadores, slashes, especiales, escapa backslashes.
    static QString cleanTopicName(const QString &input);

    // clean_4: limpia texto raw de clipboard/OCR.
    // language-aware: CJK preserva saltos, occidental une líneas.
    // Preserva __ como separador de párrafo.
    static QString cleanRawText(const QString &input, bool isCjk = false,
                                int sentenceChars = 180, int sentenceLines = 2);

    // clean_6: limpia texto OCR (tesseract output).
    static QString cleanOcrText(const QString &input);

    // clean_7: texto CJK/Rusiano → split por oraciones (coma, 。, __, —).
    static QString splitCjkSentences(const QString &input);

    // clean_8: texto occidental → split por oraciones (. ? ! … __).
    static QString splitWesternSentences(const QString &input);

    // clean_9: limpia texto editado/modificado (mngr.sh edit dialog).
    static QString cleanEditedText(const QString &input);

private:
    // Helpers
    static QString joinLines(const QString &input);
    static QString normalizeSpaces(const QString &input);
    static QString stripHtml(const QString &input);
    static QString capitalizeFirst(const QString &input);
};

} // namespace services
} // namespace idiomind
