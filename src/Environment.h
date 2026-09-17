#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

class EnvironmentManager {
public:
    static void init();
    static void draw(float gameTimer);
    static void drawArenaGround();
    static void drawPaintedSkyWalls();
    static void drawWheatFieldTrim();
    static void drawBackgroundHouses();
    static void drawBoundaryLines();
    static void drawOverheadDigitalClock(float remainingSeconds);
};

#endif // ENVIRONMENT_H

