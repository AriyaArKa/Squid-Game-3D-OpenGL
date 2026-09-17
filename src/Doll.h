#ifndef DOLL_H
#define DOLL_H

#include "Types.h"

class DollManager {
public:
    static void init(Doll& doll);
    static void update(Doll& doll, float dt);
    static void draw(const Doll& doll);
    static void drawDeadTree(float x, float y, float z);
};

#endif // DOLL_H
