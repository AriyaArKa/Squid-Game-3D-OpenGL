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
    static void drawSkyDome();
    static void drawArenaGround();
    static void drawPaintedSkyWalls();
    static void drawWheatFieldTrim();
    static void drawBackgroundHouses();
    static void drawVillageHouse(float x, float z, float rotY); // one house (used 9 times)
    static void drawBoundaryLines();
    static void drawOverheadDigitalClock(float remainingSeconds);
};

#endif // ENVIRONMENT_H

