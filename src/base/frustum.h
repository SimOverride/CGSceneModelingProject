#pragma once

#include "bounding_box.h"
#include "plane.h"
#include <iostream>

struct Frustum {
public:
    Plane planes[6];
    enum {
        LeftFace = 0,
        RightFace = 1,
        BottomFace = 2,
        TopFace = 3,
        NearFace = 4,
        FarFace = 5
    };

    bool intersect(const BoundingBox& aabb, const glm::mat4& modelMatrix) const {
        // TODO: judge whether the frustum intersects the bounding box
        // Note: this is for Bonus project 'Frustum Culling'
        // write your code here
        // ------------------------------------------------------------
        glm::vec3 vertices[8] = {
            glm::vec3(aabb.min.x, aabb.min.y, aabb.min.z),
            glm::vec3(aabb.min.x, aabb.min.y, aabb.max.z),
            glm::vec3(aabb.min.x, aabb.max.y, aabb.min.z),
            glm::vec3(aabb.min.x, aabb.max.y, aabb.max.z),
            glm::vec3(aabb.max.x, aabb.min.y, aabb.min.z),
            glm::vec3(aabb.max.x, aabb.min.y, aabb.max.z),
            glm::vec3(aabb.max.x, aabb.max.y, aabb.min.z),
            glm::vec3(aabb.max.x, aabb.max.y, aabb.max.z),
        };

        glm::vec3 min = glm::vec3(FLT_MAX);
        glm::vec3 max = glm::vec3(-FLT_MAX);

        for (const auto& vertex : vertices) {
            glm::vec3 transformedVertex = glm::vec3(modelMatrix * glm::vec4(vertex, 1.0f));
            min = glm::min(min, transformedVertex);
            max = glm::max(max, transformedVertex);
        }
        glm::vec3 center = (min + max) / 2.0f;
        glm::vec3 extends = max - center;

        for (Plane plane : planes) {
            float r = extends[0] * abs(plane.normal[0]) + extends[1] * abs(plane.normal[1]) + extends[2] * abs(plane.normal[2]);
            if (-r > plane.getSignedDistanceToPoint(center))
                return false;
        }
        return true;
        // ------------------------------------------------------------
    }
};

inline std::ostream& operator<<(std::ostream& os, const Frustum& frustum) {
    os << "frustum: \n";
    os << "planes[Left]:   " << frustum.planes[0] << "\n";
    os << "planes[Right]:  " << frustum.planes[1] << "\n";
    os << "planes[Bottom]: " << frustum.planes[2] << "\n";
    os << "planes[Top]:    " << frustum.planes[3] << "\n";
    os << "planes[Near]:   " << frustum.planes[4] << "\n";
    os << "planes[Far]:    " << frustum.planes[5] << "\n";

    return os;
}