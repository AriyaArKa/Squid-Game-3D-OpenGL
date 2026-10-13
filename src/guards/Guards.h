#ifndef GUARDS_H
#define GUARDS_H

#include "core/Types.h"
#include <vector>

class GuardsManager {
public:
    static void init(std::vector<Guard>& guards);
    static void update(std::vector<Guard>& guards, float dt);
    static void draw(const std::vector<Guard>& guards);
    static void triggerShot(std::vector<Guard>& guards, float targetX, float targetZ);
    static void drawSingleGuard(const Guard& guard);
};

#endif // GUARDS_H
