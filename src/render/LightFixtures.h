#ifndef LIGHT_FIXTURES_H
#define LIGHT_FIXTURES_H

#include "core/Types.h"

// -----------------------------------------------------------------------------
// 3D LIGHT FIXTURE MODELS (Physical visual representations of the 3 light types)
// - Directional Light: 3D Sun in RLGL / Overhead Floodlight Array in Glass Bridge
// - Point Light: Arena Floodlight Tower in RLGL / Industrial Chandelier in Glass Bridge
// - Spot Light: Theatrical Follow-Spotlight Projector & Volumetric Tracking Cone
// -----------------------------------------------------------------------------
namespace LightFixtures {
    // Calculates directional vector from spherical coordinates (Elevation & Azimuth)
    void updateSunDirection(DirLightData& dir, float sunElevation, float sunAzimuth);

    // Red Light Green Light fixtures
    void drawDirectionalSun(const DirLightData& dir, float sunElevation, float sunAzimuth, bool showSunRays);
    void drawArenaFloodlight(const PointLightData& pt);

    // Glass Bridge fixtures
    void drawOverheadFloodlightArray(const DirLightData& dir, float sunElevation, float sunAzimuth, bool showSunRays);
    void drawBridgeChandelier(const PointLightData& pt);
    void drawFollowSpotlight(const SpotLightData& sp, bool showSunRays);
    // Wall lamp for one of the glass bridge hall's fill point lights; `side` = -1 left wall, +1 right wall
    void drawWallLamp(const PointLightData& pt, bool enabled, float side);
}

#endif // LIGHT_FIXTURES_H
