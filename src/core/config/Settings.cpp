#include "core/config/Settings.h"

namespace idiomind {
namespace core {

Settings::Settings()
{
    // Valores documentados en el análisis del original (docs 06/08).
    // El load real de sets.cfg/cfgdb es TODO(F2), ver header.
    m_reviewIntervals  = { 0, 4, 7, 7, 10, 15, 15, 20, 30, 60 }; // notice[]
    m_sentenceChars    = 180;   // sentence_chars
    m_sentenceWordsLevel0 = 12; // sentence_words_level0
    m_sentenceLines    = 2;     // sentence_lines
}

void Settings::loadFromResource()
{
    // TODO(F2): parsear resources/default/sets.cfg y overrides del usuario.
    m_loaded = false;
}

QList<int> Settings::reviewIntervals() const
{
    return m_reviewIntervals;
}

int Settings::sentenceChars() const
{
    return m_sentenceChars;
}

int Settings::sentenceWordsLevel0() const
{
    return m_sentenceWordsLevel0;
}

int Settings::sentenceLines() const
{
    return m_sentenceLines;
}

bool Settings::isLoaded() const
{
    return m_loaded;
}

void Settings::reset()
{
    m_reviewIntervals.clear();
    m_sentenceChars = 0;
    m_sentenceWordsLevel0 = 0;
    m_sentenceLines = 0;
    m_loaded = false;
}

} // namespace core
} // namespace idiomind