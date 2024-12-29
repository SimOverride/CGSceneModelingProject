#include "primitive_factory.h"

Model* PrimitiveFactory::createCube(std::string name, float size) {
    std::vector<Vertex> vertices = {
        {{-size, -size, -size}, {0, 0, -1}, {0, 0}},
        {{ size, -size, -size}, {0, 0, -1}, {1, 0}},
        {{ size,  size, -size}, {0, 0, -1}, {1, 1}},
        {{-size,  size, -size}, {0, 0, -1}, {0, 1}},

        {{-size, -size,  size}, {0, 0,  1}, {0, 0}},
        {{ size, -size,  size}, {0, 0,  1}, {1, 0}},
        {{ size,  size,  size}, {0, 0,  1}, {1, 1}},
        {{-size,  size,  size}, {0, 0,  1}, {0, 1}},

        {{-size,  size,  size}, {0, 1, 0}, {0, 0}},
        {{-size,  size, -size}, {0, 1, 0}, {0, 1}},
        {{ size,  size, -size}, {0, 1, 0}, {1, 1}},
        {{ size,  size,  size}, {0, 1, 0}, {1, 0}},

        {{-size, -size,  size}, {0, -1, 0}, {0, 0}},
        {{ size, -size,  size}, {0, -1, 0}, {1, 0}},
        {{ size, -size, -size}, {0, -1, 0}, {1, 1}},
        {{-size, -size, -size}, {0, -1, 0}, {0, 1}},

        {{ size, -size,  size}, {1, 0, 0}, {0, 0}},
        {{ size,  size,  size}, {1, 0, 0}, {1, 0}},
        {{ size,  size, -size}, {1, 0, 0}, {1, 1}},
        {{ size, -size, -size}, {1, 0, 0}, {0, 1}},

        {{-size, -size,  size}, {-1, 0, 0}, {0, 0}},
        {{-size, -size, -size}, {-1, 0, 0}, {1, 0}},
        {{-size,  size, -size}, {-1, 0, 0}, {1, 1}},
        {{-size,  size,  size}, {-1, 0, 0}, {0, 1}},
    };

    std::vector<uint32_t> indices = {
        0, 2, 1, 2, 0, 3, // Front
        4, 5, 6, 6, 7, 4, // Back
        8, 9, 10, 10, 11, 8, // Top
        12, 14, 13, 14, 12, 15, // Bottom
        16, 18, 17, 18, 16, 19, // Right
        20, 22, 21, 22, 20, 23  // Left
    };

    return new Model(name, vertices, indices);
}

Model* PrimitiveFactory::createSphere(std::string name, float radius, int sectors, int stacks) {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    for (int stack = 0; stack <= stacks; stack++) {
        float phi = stack * glm::pi<float>() / stacks;
        for (int sector = 0; sector <= sectors; sector++) {
            float theta = sector * 2 * glm::pi<float>() / sectors;

            glm::vec3 position(
                radius * sin(phi) * cos(theta),
                radius * cos(phi),
                radius * sin(phi) * sin(theta)
            );
            glm::vec3 normal = glm::normalize(position);
            glm::vec2 texCoord(
                static_cast<float>(sector) / sectors,
                static_cast<float>(stack) / stacks
            );

            vertices.push_back({ position, normal, texCoord });
        }
    }

    for (int stack = 0; stack < stacks; stack++) {
        for (int sector = 0; sector < sectors; sector++) {
            int current = stack * (sectors + 1) + sector;
            int next = current + sectors + 1;

            indices.push_back(current);
            indices.push_back(next);
            indices.push_back(current + 1);

            indices.push_back(current + 1);
            indices.push_back(next);
            indices.push_back(next + 1);
        }
    }

    return new Model(name, vertices, indices);
}

Model* PrimitiveFactory::createPlane(std::string name, float width, float height, int segmentsX, int segmentsY) {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    float dx = width / segmentsX;
    float dy = height / segmentsY;

    for (int y = 0; y <= segmentsY; y++) {
        for (int x = 0; x <= segmentsX; x++) {
            glm::vec3 position(-width / 2 + x * dx, 0.0f, -height / 2 + y * dy);
            glm::vec3 normal(0.0f, 1.0f, 0.0f);  // 平面法线指向 +Y
            glm::vec2 texCoord(static_cast<float>(x) / segmentsX, static_cast<float>(y) / segmentsY);

            vertices.push_back({ position, normal, texCoord });
        }
    }
    
    for (int y = 0; y < segmentsY; y++) {
        for (int x = 0; x < segmentsX; x++) {
            int current = y * (segmentsX + 1) + x;
            int next = current + (segmentsX + 1);

            indices.push_back(current);
            indices.push_back(next);
            indices.push_back(current + 1);

            indices.push_back(current + 1);
            indices.push_back(next);
            indices.push_back(next + 1);
        }
    }

    return new Model(name, vertices, indices);
}

Model* PrimitiveFactory::createCylinder(std::string name, float radius, float height, int radialSegments, int heightSegments) {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    float angleStep = 2.0f * glm::pi<float>() / radialSegments;
    float heightStep = height / heightSegments;

    // 创建圆柱侧面顶点
    for (int h = 0; h <= heightSegments; ++h) {
        float y = -height / 2 + h * heightStep;
        for (int r = 0; r <= radialSegments; ++r) {
            float angle = r * angleStep;
            float x = radius * cos(angle);
            float z = radius * sin(angle);

            glm::vec3 position(x, y, z);
            glm::vec3 normal(cos(angle), 0.0f, sin(angle));
            glm::vec2 texCoord(static_cast<float>(r) / radialSegments, static_cast<float>(h) / heightSegments);

            vertices.push_back({ position, normal, texCoord });
        }
    }

    // 创建圆柱侧面索引
    for (int h = 0; h < heightSegments; ++h) {
        for (int r = 0; r < radialSegments; ++r) {
            int current = h * (radialSegments + 1) + r;
            int next = current + radialSegments + 1;

            indices.push_back(current);
            indices.push_back(next);
            indices.push_back(current + 1);

            indices.push_back(current + 1);
            indices.push_back(next);
            indices.push_back(next + 1);
        }
    }

    // 创建底面和顶面
    int baseCenterIndex = vertices.size();
    vertices.push_back({ glm::vec3(0.0f, -height / 2, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.5f, 0.5f) });
    for (int r = 0; r <= radialSegments; ++r) {
        float angle = r * angleStep;
        float x = radius * cos(angle);
        float z = radius * sin(angle);

        vertices.push_back({ glm::vec3(x, -height / 2, z), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.5f + 0.5f * cos(angle), 0.5f + 0.5f * sin(angle)) });
        if (r > 0) {
            indices.push_back(baseCenterIndex);
            indices.push_back(baseCenterIndex + r);
            indices.push_back(baseCenterIndex + r + 1);
        }
    }

    int topCenterIndex = vertices.size();
    vertices.push_back({ glm::vec3(0.0f, height / 2, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.5f, 0.5f) });
    for (int r = 0; r <= radialSegments; ++r) {
        float angle = r * angleStep;
        float x = radius * cos(angle);
        float z = radius * sin(angle);

        vertices.push_back({ glm::vec3(x, height / 2, z), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.5f + 0.5f * cos(angle), 0.5f + 0.5f * sin(angle)) });
        if (r > 0) {
            indices.push_back(topCenterIndex);
            indices.push_back(topCenterIndex + r + 1);
            indices.push_back(topCenterIndex + r);
        }
    }

    return new Model(name, vertices, indices);
}

Model* PrimitiveFactory::createCone(std::string name, float radius, float height, int radialSegments) {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    float angleStep = 2.0f * glm::pi<float>() / radialSegments;

    // 创建圆锥侧面顶点
    glm::vec3 apex(0.0f, height / 2, 0.0f);
    for (int r = 0; r <= radialSegments; ++r) {
        float angle = r * angleStep;
        float x = radius * cos(angle);
        float z = radius * sin(angle);

        glm::vec3 position(x, -height / 2, z);
        glm::vec3 normal = glm::normalize(glm::vec3(x, height / 2, z));
        glm::vec2 texCoord(static_cast<float>(r) / radialSegments, 1.0f);

        vertices.push_back({ position, normal, texCoord });
    }
    vertices.push_back({ apex, glm::normalize(apex), glm::vec2(0.5f, 0.0f) });

    // 创建圆锥底面索引
    int baseCenterIndex = vertices.size();
    vertices.push_back({ glm::vec3(0.0f, -height / 2, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.5f, 0.5f) });
    for (int r = 0; r <= radialSegments; ++r) {
        float angle = r * angleStep;
        float x = radius * cos(angle);
        float z = radius * sin(angle);

        vertices.push_back({ glm::vec3(x, -height / 2, z), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.5f + 0.5f * cos(angle), 0.5f + 0.5f * sin(angle)) });
        if (r > 0) {
            indices.push_back(baseCenterIndex);
            indices.push_back(baseCenterIndex + r);
            indices.push_back(baseCenterIndex + r + 1);
        }
    }

    // 创建侧面索引
    for (int r = 0; r < radialSegments; ++r) {
        indices.push_back(r);
        indices.push_back(r + 1);
        indices.push_back(vertices.size() - radialSegments - 1);
    }

    return new Model(name, vertices, indices);
}

Model* PrimitiveFactory::createPrism(std::string name, float radius, float height, int sides, int heightSegments) {
    if (sides < 3) {
        throw std::invalid_argument("A prism must have at least 3 sides.");
    }

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    float angleStep = 2.0f * glm::pi<float>() / sides;
    float heightStep = height / heightSegments;

    // 创建底面和顶面顶点
    for (int h = 0; h <= heightSegments; ++h) {
        float y = -height / 2 + h * heightStep;
        for (int s = 0; s < sides; ++s) {
            float angle = s * angleStep;
            float x = radius * cos(angle);
            float z = radius * sin(angle);

            glm::vec3 position(x, y, z);
            glm::vec3 normal(cos(angle), 0.0f, sin(angle));
            glm::vec2 texCoord(static_cast<float>(s) / sides, static_cast<float>(h) / heightSegments);

            vertices.push_back({ position, normal, texCoord });
        }
    }

    // 创建侧面索引
    for (int h = 0; h < heightSegments; ++h) {
        for (int s = 0; s < sides; ++s) {
            int current = h * sides + s;
            int next = current + sides;

            indices.push_back(current);
            indices.push_back(next);
            indices.push_back((current + 1) % sides + h * sides);

            indices.push_back((current + 1) % sides + h * sides);
            indices.push_back(next);
            indices.push_back((next + 1) % sides + (h + 1) * sides);
        }
    }

    // 底面索引
    int baseCenterIndex = vertices.size();
    vertices.push_back({ glm::vec3(0.0f, -height / 2, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.5f, 0.5f) });
    for (int s = 0; s < sides; ++s) {
        float angle = s * angleStep;
        float x = radius * cos(angle);
        float z = radius * sin(angle);

        vertices.push_back({ glm::vec3(x, -height / 2, z), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.5f + 0.5f * cos(angle), 0.5f + 0.5f * sin(angle)) });
        if (s > 0) {
            indices.push_back(baseCenterIndex);
            indices.push_back(baseCenterIndex + s);
            indices.push_back(baseCenterIndex + s + 1);
        }
    }

    // 顶面索引
    int topCenterIndex = vertices.size();
    vertices.push_back({ glm::vec3(0.0f, height / 2, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.5f, 0.5f) });
    for (int s = 0; s < sides; ++s) {
        float angle = s * angleStep;
        float x = radius * cos(angle);
        float z = radius * sin(angle);

        vertices.push_back({ glm::vec3(x, height / 2, z), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.5f + 0.5f * cos(angle), 0.5f + 0.5f * sin(angle)) });
        if (s > 0) {
            indices.push_back(topCenterIndex);
            indices.push_back(topCenterIndex + s + 1);
            indices.push_back(topCenterIndex + s);
        }
    }

    return new Model(name, vertices, indices);
}

Model* PrimitiveFactory::createFrustum(std::string name, float bottomRadius, float topRadius, float height, int sides, int heightSegments) {
    if (sides < 3) {
        throw std::invalid_argument("A frustum must have at least 3 sides.");
    }

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    float angleStep = 2.0f * glm::pi<float>() / sides;
    float heightStep = height / heightSegments;

    // 创建顶点
    for (int h = 0; h <= heightSegments; ++h) {
        float t = static_cast<float>(h) / heightSegments;
        float y = -height / 2 + h * heightStep;
        float radius = bottomRadius + t * (topRadius - bottomRadius);

        for (int s = 0; s < sides; ++s) {
            float angle = s * angleStep;
            float x = radius * cos(angle);
            float z = radius * sin(angle);

            glm::vec3 position(x, y, z);
            glm::vec3 normal = glm::normalize(glm::vec3(cos(angle), (bottomRadius - topRadius) / height, sin(angle)));
            glm::vec2 texCoord(static_cast<float>(s) / sides, t);

            vertices.push_back({ position, normal, texCoord });
        }
    }

    // 创建侧面索引
    for (int h = 0; h < heightSegments; ++h) {
        for (int s = 0; s < sides; ++s) {
            int current = h * sides + s;
            int next = current + sides;

            indices.push_back(current);
            indices.push_back(next);
            indices.push_back((current + 1) % sides + h * sides);

            indices.push_back((current + 1) % sides + h * sides);
            indices.push_back(next);
            indices.push_back((next + 1) % sides + (h + 1) * sides);
        }
    }

    // 底面索引
    int baseCenterIndex = vertices.size();
    vertices.push_back({ glm::vec3(0.0f, -height / 2, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.5f, 0.5f) });
    for (int s = 0; s < sides; ++s) {
        float angle = s * angleStep;
        float x = bottomRadius * cos(angle);
        float z = bottomRadius * sin(angle);

        vertices.push_back({ glm::vec3(x, -height / 2, z), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.5f + 0.5f * cos(angle), 0.5f + 0.5f * sin(angle)) });
        if (s > 0) {
            indices.push_back(baseCenterIndex);
            indices.push_back(baseCenterIndex + s);
            indices.push_back(baseCenterIndex + s + 1);
        }
    }

    // 顶面索引
    int topCenterIndex = vertices.size();
    vertices.push_back({ glm::vec3(0.0f, height / 2, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.5f, 0.5f) });
    for (int s = 0; s < sides; ++s) {
        float angle = s * angleStep;
        float x = topRadius * cos(angle);
        float z = topRadius * sin(angle);

        vertices.push_back({ glm::vec3(x, height / 2, z), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.5f + 0.5f * cos(angle), 0.5f + 0.5f * sin(angle)) });
        if (s > 0) {
            indices.push_back(topCenterIndex);
            indices.push_back(topCenterIndex + s + 1);
            indices.push_back(topCenterIndex + s);
        }
    }

    return new Model(name, vertices, indices);
}