 #include <algorithm>
 #include <iostream>
 #include <limits>
#include <unordered_map>
#include <fstream>
#include <sstream>

#include "model.h"

struct Face {
    int vi[3];  // 顶点索引
    int ti[3];  // 材质索引
    int ni[3];  // 法线索引
};

Model::Model(const std::string& name, const std::string& filepath) : Object(name) {
    LoadObj(filepath);

    computeBoundingBox();

    initGLResources();

    initBoxGLResources();

    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        cleanup();
        throw std::runtime_error("OpenGL Error: " + std::to_string(error));
    }
}

Model::Model(const std::string& name, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
    : Object(name), _vertices(vertices), _indices(indices) {

    computeBoundingBox();

    initGLResources();

    initBoxGLResources();

    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        cleanup();
        throw std::runtime_error("OpenGL Error: " + std::to_string(error));
    }
}

Model::Model(Model&& rhs) noexcept
    : Object(name), _vertices(std::move(rhs._vertices)), _indices(std::move(rhs._indices)),
    _boundingBox(std::move(rhs._boundingBox)), _vao(rhs._vao), _vbo(rhs._vbo), _ebo(rhs._ebo),
    _boxVao(rhs._boxVao), _boxVbo(rhs._boxVbo), _boxEbo(rhs._boxEbo) {
    _vao = 0;
    _vbo = 0;
    _ebo = 0;
    _boxVao = 0;
    _boxVbo = 0;
    _boxEbo = 0;
}

Model::~Model() {
    cleanup();
}

void Model::renderInspector() {
    Object::renderInspector();
}

void Model::LoadObj(const std::string& filepath) {
    std::ifstream in;
    in.open(filepath, std::ifstream::in);
    if (!in.is_open()) {
        std::cerr << "Can't open " + filepath << "\n";
        return;
    }

    std::string line;
    std::vector<glm::vec3> verts, norms;
    std::vector<glm::vec2> texCoords;
    std::vector<Face> faces;
    while (!in.eof()) {
        std::getline(in, line);
        std::istringstream iss(line.c_str());

        std::string prefix;
        iss >> prefix;
        if (prefix == "v") {
            glm::vec3 v{};
            iss >> v.x >> v.y >> v.z;
            verts.push_back(v);
        } else if (prefix == "vt") {
            glm::vec2 t{};
            iss >> t.x >> t.y;
            texCoords.push_back(t);
        } else if (prefix == "vn") {
            glm::vec3 n{};
            iss >> n.x >> n.y >> n.z;
            norms.push_back(n);
        } else if (prefix == "f") {
            int vis[4]{}, tis[4]{}, nis[4]{};
             
            std::string vertexStr;
            int i = 0;
            while (iss >> vertexStr && i < 4) {  // 处理最多4个顶点
                // 使用正则解析格式
                size_t firstSlash = vertexStr.find('/');
                size_t secondSlash = vertexStr.find('/', firstSlash + 1);

                if (firstSlash == std::string::npos) {
                    // v
                    vis[i] = std::stoi(vertexStr) - 1;
                    tis[i] = -1;
                    nis[i] = -1;
                } else if (secondSlash == std::string::npos) {
                    // v/t
                    vis[i] = std::stoi(vertexStr.substr(0, firstSlash)) - 1;
                    tis[i] = std::stoi(vertexStr.substr(firstSlash + 1)) - 1;
                    nis[i] = -1;
                } else if (firstSlash + 1 == secondSlash) {
                    // v//n
                    vis[i] = std::stoi(vertexStr.substr(0, firstSlash)) - 1;
                    tis[i] = -1;
                    nis[i] = std::stoi(vertexStr.substr(secondSlash + 1)) - 1;
                } else {
                    // v/t/n
                    vis[i] = std::stoi(vertexStr.substr(0, firstSlash)) - 1;
                    tis[i] = std::stoi(vertexStr.substr(firstSlash + 1, secondSlash - firstSlash - 1)) - 1;
                    nis[i] = std::stoi(vertexStr.substr(secondSlash + 1)) - 1;
                }
                i++;
            }

            // 如果面包含四个顶点，将其分割为两个三角形
            if (i == 4) {
                // 第一个三角形 v1, v2, v3
                Face f1 = { { vis[0], vis[1], vis[2] }, { tis[0], tis[1], tis[2] }, { nis[0], nis[1], nis[2] } };
                faces.push_back(f1);

                // 第二个三角形 v1, v3, v4
                Face f2 = { { vis[0], vis[2], vis[3] }, { tis[0], tis[2], tis[3] }, { nis[0], nis[2], nis[3] } };
                faces.push_back(f2);
            } else {
                Face f = { { vis[0], vis[1], vis[2] }, { tis[0], tis[1], tis[2] }, { nis[0], nis[1], nis[2] } };
                faces.push_back(f);
            }
        }
    }

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    std::unordered_map<Vertex, uint32_t> uniqueVertices;

    for (const auto& f : faces) {
        for (int i = 0; i < 3; i++) {
            Vertex vertex{};
            vertex.position = verts[f.vi[i]];
            if (f.ni[i] >= 0)
                vertex.normal = norms[f.ni[i]];
            if (f.ti[i] >= 0)
                vertex.texCoord = texCoords[f.ti[i]];

            if (uniqueVertices.count(vertex) == 0) {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }

            indices.push_back(uniqueVertices[vertex]);
        }
    }

    _vertices = vertices;
    _indices = indices;

    in.close();
}

void Model::ExportObj(const std::string& filepath) const {
    std::ofstream out(filepath);
    if (!out.is_open()) {
        std::cerr << "Can't open file: " << filepath << std::endl;
        return;
    }

    std::unordered_map<glm::vec3, uint32_t> uniqueNormals;
    std::vector<glm::vec3> normals;
    std::unordered_map<glm::vec2, uint32_t> uniqueTexCoords;
    std::vector<glm::vec2> texCoords;

    // 写入顶点数据
    for (const auto& vertex : _vertices) {
        out << "v " << vertex.position.x << " " << vertex.position.y << " " << vertex.position.z << "\n";
        // 去除重复法向量，uv坐标
        if (uniqueNormals.count(vertex.normal) == 0 && vertex.normal != glm::zero<glm::vec3>()) {
            uniqueNormals[vertex.normal] = normals.size() + 1;  // .obj 索引从 1 开始
            normals.push_back(vertex.normal);
        }
        if (uniqueTexCoords.count(vertex.texCoord) == 0 && vertex.texCoord != glm::zero<glm::vec2>()) {
            uniqueTexCoords[vertex.texCoord] = texCoords.size() + 1;  // .obj 索引从 1 开始
            texCoords.push_back(vertex.texCoord);
        }
    }

    for (const auto& normal : normals) {
        out << "vn " << normal.x << " " << normal.y << " " << normal.z << "\n";
    }

    for (const auto& texCoord : texCoords) {
        out << "vt " << texCoord.x << " " << texCoord.y << "\n";
    }

    // 写入面数据
    for (size_t i = 0; i < _indices.size(); i += 3) {
        out << "f ";
        for (int j = 0; j < 3; ++j) {
            size_t idx = _indices[i + j] + 1;  // .obj 索引从 1 开始
            const Vertex& vertex = _vertices[_indices[i + j]];

            int texIdx = uniqueTexCoords.count(vertex.texCoord) ? uniqueTexCoords[vertex.texCoord] : 0;
            int normIdx = uniqueNormals.count(vertex.normal) ? uniqueNormals[vertex.normal] : 0;

            if (texIdx == 0 && normIdx == 0) {
                out << idx << " ";
            }
            else if (texIdx == 0) {
                out << idx << "//" << normIdx << " ";
            }
            else if (normIdx == 0) {
                out << idx << "/" << texIdx << " ";
            }
            else {
                out << idx << "/" << texIdx << "/" << normIdx << " ";
            }
        }
        out << "\n";
    }

    out.close();
}

BoundingBox Model::getBoundingBox() const {
    return _boundingBox;
}

void Model::draw() const {
    glBindVertexArray(_vao);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(_indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Model::drawBoundingBox() const {
    glBindVertexArray(_boxVao);
    glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

GLuint Model::getVao() const {
    return _vao;
}

GLuint Model::getBoundingBoxVao() const {
    return _boxVao;
}

size_t Model::getVertexCount() const {
    return _vertices.size();
}

size_t Model::getFaceCount() const {
    return _indices.size() / 3;
}

void Model::initGLResources() {
    // create a vertex array object
    glGenVertexArrays(1, &_vao);
    // create a vertex buffer object
    glGenBuffers(1, &_vbo);
    // create a element array buffer
    glGenBuffers(1, &_ebo);

    glBindVertexArray(_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(
        GL_ARRAY_BUFFER, sizeof(Vertex) * _vertices.size(), _vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER, _indices.size() * sizeof(uint32_t), _indices.data(),
        GL_STATIC_DRAW);

    // specify layout, size of a vertex, data type, normalize, sizeof vertex array, offset of the
    // attribute
    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void Model::computeBoundingBox() {
    float minX = std::numeric_limits<float>::max();
    float minY = std::numeric_limits<float>::max();
    float minZ = std::numeric_limits<float>::max();
    float maxX = -std::numeric_limits<float>::max();
    float maxY = -std::numeric_limits<float>::max();
    float maxZ = -std::numeric_limits<float>::max();

    for (const auto& v : _vertices) {
        minX = std::min(v.position.x, minX);
        minY = std::min(v.position.y, minY);
        minZ = std::min(v.position.z, minZ);
        maxX = std::max(v.position.x, maxX);
        maxY = std::max(v.position.y, maxY);
        maxZ = std::max(v.position.z, maxZ);
    }

    _boundingBox.min = glm::vec3(minX, minY, minZ);
    _boundingBox.max = glm::vec3(maxX, maxY, maxZ);
}

void Model::initBoxGLResources() {
    std::vector<glm::vec3> boxVertices = {
        glm::vec3(_boundingBox.min.x, _boundingBox.min.y, _boundingBox.min.z),
        glm::vec3(_boundingBox.max.x, _boundingBox.min.y, _boundingBox.min.z),
        glm::vec3(_boundingBox.min.x, _boundingBox.max.y, _boundingBox.min.z),
        glm::vec3(_boundingBox.max.x, _boundingBox.max.y, _boundingBox.min.z),
        glm::vec3(_boundingBox.min.x, _boundingBox.min.y, _boundingBox.max.z),
        glm::vec3(_boundingBox.max.x, _boundingBox.min.y, _boundingBox.max.z),
        glm::vec3(_boundingBox.min.x, _boundingBox.max.y, _boundingBox.max.z),
        glm::vec3(_boundingBox.max.x, _boundingBox.max.y, _boundingBox.max.z),
    };

    std::vector<uint32_t> boxIndices = {0, 1, 0, 2, 0, 4, 3, 1, 3, 2, 3, 7,
                                        5, 4, 5, 1, 5, 7, 6, 4, 6, 7, 6, 2};

    glGenVertexArrays(1, &_boxVao);
    glGenBuffers(1, &_boxVbo);
    glGenBuffers(1, &_boxEbo);

    glBindVertexArray(_boxVao);
    glBindBuffer(GL_ARRAY_BUFFER, _boxVbo);
    glBufferData(
        GL_ARRAY_BUFFER, boxVertices.size() * sizeof(glm::vec3), boxVertices.data(),
        GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _boxEbo);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER, boxIndices.size() * sizeof(uint32_t), boxIndices.data(),
        GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void Model::cleanup() {
    if (_boxEbo) {
        glDeleteBuffers(1, &_boxEbo);
        _boxEbo = 0;
    }

    if (_boxVbo) {
        glDeleteBuffers(1, &_boxVbo);
        _boxVbo = 0;
    }

    if (_boxVao) {
        glDeleteVertexArrays(1, &_boxVao);
        _boxVao = 0;
    }

    if (_ebo != 0) {
        glDeleteBuffers(1, &_ebo);
        _ebo = 0;
    }

    if (_vbo != 0) {
        glDeleteBuffers(1, &_vbo);
        _vbo = 0;
    }

    if (_vao != 0) {
        glDeleteVertexArrays(1, &_vao);
        _vao = 0;
    }
}