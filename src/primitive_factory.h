#pragma once

#include "base/model.h"

enum PrimitiveShape {
    Cube,
    Sphere,
    Plane,
    Cylinder,
    Cone,
    Prism,
    Frustum
};

class PrimitiveFactory {
public:
    static Model* createCube(std::string name, float size);

    static Model* createSphere(std::string name, float radius, int sectors, int stacks);

    static Model* createPlane(std::string name, float width, float height, int segmentsX = 1, int segmentsY = 1);

    static Model* createCylinder(std::string name, float radius, float height, int radialSegments, int heightSegments);

    static Model* createCone(std::string name, float radius, float height, int radialSegments);

    static Model* createPrism(std::string name, float radius, float height, int sides, int heightSegments = 1);

    static Model* createFrustum(std::string name, float bottomRadius, float topRadius, float height, int sides, int heightSegments = 1);
};