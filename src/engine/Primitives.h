#ifndef ENGINE_PRIMITIVES_H
#define ENGINE_PRIMITIVES_H

#include "engine/Mesh.h"

// Shared geometry the legacy GLUT/GLU primitive calls (glutSolidCube, glutSolidSphere,
// gluCylinder, gluDisk, glutSolidTorus) get mapped onto. Box and Sphere are true singletons
// (a unit shape, stretched via the model matrix - exactly like glutSolidCube(1.0)/
// glutSolidSphere(1.0,..) do under glScalef). Cylinder/Disk/Torus can't be represented by
// scaling a single unit shape (their radius RATIOS change the topology, e.g. a cone is a
// cylinder with topRadius=0), so those are cached per distinct (radii, resolution) tuple -
// call sites in this codebase always pass the same literal constants every frame, so each
// distinct shape is built once, ever, and reused from then on.
namespace Primitives {
    Mesh& unitBox();                                    // -0.5..0.5 cube, matches glutSolidCube(1.0)
    Mesh& unitSphere();                                  // radius-1 UV sphere at the origin
    Mesh& cylinder(float baseRadius, float topRadius, float height, int slices); // side only, z=0..height (topRadius=0 => a cone)
    Mesh& disk(float innerRadius, float outerRadius, int slices);               // flat annulus in the XY plane, facing +Z
    Mesh& torus(float innerRadius, float outerRadius, int sides, int rings);    // tube radius innerRadius, ring radius outerRadius
}

#endif // ENGINE_PRIMITIVES_H
