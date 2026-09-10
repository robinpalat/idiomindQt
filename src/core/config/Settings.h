#pragma once

#include <QList>
#include <QString>

namespace idiomind {
namespace core {

/**
 * Configuración de la aplicación (infraestructura F0, doc 09 §3.14).
 *
 * F0: SOLO la infraestructura. Todavía no lee ningún recurso ni fichero.
 *
 * TODO(F2): implementar loadFromResource() y loadFromUserConfig() para
 * parsear, de acuerdo con el original:
 *   - resources/default/sets.cfg  (docs 02/06/08):
 *       notice[] = intervalos de repaso           -> reviewIntervals()
 *       slangs[]/tlangs[] (mapas nombre->código)  -> languageNameToCode()
 *       labels_level / labels_status / label_serie
 *       sentence_lines / sentence_chars / sentence_words_level0
 *       parámetros espeak (sAmplitude..sWordgap)  (doc 07)
 *       useragent (c.conf)                        (doc 02)
 *       yad_version (obsoleto en Qt)              (doc 01)
 *   - overrides del usuario en cfgdb (tabla opts/lang, doc 04) -> loadFromUserConfig().
 *
 * Los valores expuestos en F0 son LOS DOCUMENTADOS en el análisis
 * (docs 06/08), no inventados. El soporte de overrides por usuario es F2+.
 */
class Settings
{
public:
    Settings();

    /// Infraestructura de carga (F0): no-op. Ver TODO arriba.
    void loadFromResource();

    // -- valores documentados del análisis (docs 06/08) --
    // notice[] del sets.cfg: días entre repasos por count_date_reviews.
    QList<int> reviewIntervals() const;   // {0,4,7,7,10,15,15,20,30,60}
    int sentenceChars() const;            // sentence_chars    = 180
    int sentenceWordsLevel0() const;      // sentence_words_level0 = 12
    int sentenceLines() const;            // sentence_lines    = 2

    bool isLoaded() const;                // false hasta que exista load real (F2)
    void reset();

private:
    QList<int> m_reviewIntervals;
    int m_sentenceChars = 0;
    int m_sentenceWordsLevel0 = 0;
    int m_sentenceLines = 0;
    bool m_loaded = false;
};

} // namespace core
} // namespace idiomind