// F2.1 - ReviewState: máquina de estados de un topic.
// Autoridad: docs/06-learning-system.md. Los valores numéricos son contrato:
// 0..10 y 13. NO renumerar.
#pragma once

#include <QString>

enum class ReviewState : int {
    Paused = 0,              // pausado / deshabilitado
    Learning = 1,            // learning (activo)
    Mastered = 2,            // mastered
    Waiting = 3,             // waiting (repaso pendiente)
    WaitingAlternate = 4,    // waiting  (alterno)
    MidReview = 5,           // mid-review / "Finalize review" (readd)
    MidReviewAlternate = 6,  // mid-review (alterno)
    Ready100Odd = 7,         // listo para repaso tras 100% del primer intervalo
    Ready100Even = 8,        // listo para repaso (alterno)
    Ready150Odd = 9,         // listo para repaso tras 150%
    Ready150Even = 10,       // listo para repaso (alterno)
    Corrupted = 13           // corrupto / addon sin stts válido (auto-reparar -> 1)
};

namespace ReviewStateHelpers {

// Valores admitidos por el contrato (0..10 y 13).
inline bool isValid(int v)
{
    return (v >= 0 && v <= 10) || v == 13;
}

// Significado documentado en docs/06-learning-system.md (tabla stts).
QString meaning(ReviewState state);

inline bool isEven(ReviewState state)
{
    const int v = static_cast<int>(state);
    return v != 13 && (v % 2) == 0;
}

}  // namespace ReviewStateHelpers