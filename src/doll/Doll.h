#ifndef DOLL_H
#define DOLL_H

#include "core/Types.h"

// Orchestrates the Young-hee Doll: the green/red light + head-turn state machine, and
// composing the DollHead/DollBody/DollTree sub-modules for drawing. See DollHead.cpp,
// DollBody.cpp and DollTree.cpp for the actual geometry.
class DollManager {
public:
    static void init(Doll& doll);
    static void update(Doll& doll, float dt);
    static void draw(const Doll& doll);
};

#endif // DOLL_H
