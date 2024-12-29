#pragma once

#include <string>
#include <vector>

#include "bounding_box.h"
#include "gl_utility.h"
#include "object.h"
#include "vertex.h"
#include "texture2d.h"

struct Material {
    glm::vec3 ka = glm::vec3(0.3f, 0.3f, 0.3f);
    glm::vec3 kd = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 ks = glm::vec3(1.0f, 1.0f, 1.0f);
    float ns = 25.0f;
    
    std::shared_ptr<Texture2D> texture;
};

class Model : public Object {
public:
    Material material;

    Model(const std::string& name, const std::string& filepath);

    Model(const std::string& name, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

    Model(Model&& rhs) noexcept;

    virtual ~Model();

    void renderInspector() override;

    void exportObj(const std::string& filepath) const;

    GLuint getVao() const;

    GLuint getBoundingBoxVao() const;

    size_t getVertexCount() const;

    size_t getFaceCount() const;

    BoundingBox getBoundingBox() const;

    BoundingBox getTransformedBoundingBox() const;

    virtual void draw() const;

    virtual void drawBoundingBox() const;

    const std::vector<uint32_t>& getIndices() const {
        return _indices;
    }
    const std::vector<Vertex>& getVertices() const {
        return _vertices;
    }
    const Vertex& getVertex(int i) const {
        return _vertices[i];
    }

protected:
    // vertices of the table represented in model's own coordinate
    std::vector<Vertex> _vertices;
    std::vector<uint32_t> _indices;

    // bounding box
    BoundingBox _boundingBox;

    // opengl objects
    GLuint _vao = 0;
    GLuint _vbo = 0;
    GLuint _ebo = 0;

    GLuint _boxVao = 0;
    GLuint _boxVbo = 0;
    GLuint _boxEbo = 0;

    void loadObj(const std::string& filepath);

    void computeBoundingBox();

    void initGLResources();

    void initBoxGLResources();

    void cleanup();
};