#include "star.h"
#include <cmath>

Star::Star(const glm::vec2& position, float rotation, float radius, float aspect)
    : _position(position), _rotation(rotation), _radius(radius) {
    // TODO: assemble the vertex data of the star
    // write your code here
    // -------------------------------------
    float angle_step = glm::radians(36.0f);
    
    for (int i = 0; i < 5; i++) {
        float angle = glm::radians(90.0f) + rotation + i * angle_step * 2;
        float x1 = position.x + radius * cos(angle);
        float y1 = position.y + radius * sin(angle) * aspect;

        angle += angle_step * 4;
        float x2 = position.x + radius * cos(angle);
        float y2 = position.y + radius * sin(angle) * aspect;
        
        angle += angle_step * 3;
        float x3 = position.x + radius / 3.0f * cos(angle);
        float y3 = position.y + radius / 3.0f * sin(angle) * aspect;
        _vertices.push_back(glm::vec2(x1, y1));
        _vertices.push_back(glm::vec2(x2, y2));
        _vertices.push_back(glm::vec2(x3, y3));
    }
    // for (int i = 0; i < 5; ++i) {
    //     _vertices.push_back( ... );
    // }
    // -------------------------------------

    glGenVertexArrays(1, &_vao);
    glGenBuffers(1, &_vbo);

    glBindVertexArray(_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(
        GL_ARRAY_BUFFER, sizeof(glm::vec2) * _vertices.size(), _vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

Star::Star(Star&& rhs) noexcept
    : _position(rhs._position), _rotation(rhs._rotation), _radius(rhs._radius), _vao(rhs._vao),
      _vbo(rhs._vbo) {
    rhs._vao = 0;
    rhs._vbo = 0;
}

Star::~Star() {
    if (_vbo) {
        glDeleteVertexArrays(1, &_vbo);
        _vbo = 0;
    }

    if (_vao) {
        glDeleteVertexArrays(1, &_vao);
        _vao = 0;
    }
}

void Star::draw() const {
    glBindVertexArray(_vao);
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(_vertices.size()));
}