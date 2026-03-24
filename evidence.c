#include "defs.h"

void evidence_set(EvidenceByte* bits, enum EvidenceType ev) {
    *bits |= ev;
}

void evidence_clear(EvidenceByte* bits, enum EvidenceType ev) {
    *bits &= ~ev;
}

bool evidence_has(EvidenceByte bits, enum EvidenceType ev) {
    return (bits & ev) != 0;
}