#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

// Implementation is split across three files by concern:
//   Environment.cpp        - arena shell: ground, sky walls, wheat trim, boundary lines
//   EnvironmentHouses.cpp  - background Korean village houses
//   EnvironmentClock.cpp   - overhead 3D digital scoreboard clock
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

