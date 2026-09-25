#ifndef DOLL_HEAD_H
#define DOLL_HEAD_H

#include "core/Types.h"

// The doll's head: porcelain face (nose/lips/blush), eyes (with the Red Light laser-scan
// cue), eyebrows, hair sculpture, ears, and pigtails. Kept separate from the body/tree so
// the face can be tuned on its own.
namespace DollHead {
    void drawHead(const Doll& doll);
}

#endif // DOLL_HEAD_H
