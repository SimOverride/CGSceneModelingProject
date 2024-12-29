#include "skybox.h"

SkyBox::SkyBox(const std::vector<std::string>& textureFilenames) {
    GLfloat vertices[] = {-1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f,
                          1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f,

                          -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f,
                          -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,

                          1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,
                          1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f,

                          -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
                          1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,

                          -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,
                          1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f,

                          -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f,
                          1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f};

    // create vao and vbo
    glGenVertexArrays(1, &_vao);
    glGenBuffers(1, &_vbo);

    glBindVertexArray(_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);

    glBindVertexArray(0);

    try {
        // init texture
        _texture.reset(new ImageTextureCubemap(textureFilenames));

        const char* vsCode =
            "#version 330 core\n"
            "layout(location = 0) in vec3 aPosition;\n"
            "out vec3 texCoord;\n"
            "uniform mat4 projection;\n"
            "uniform mat4 view;\n"
            "void main() {\n"
            "   texCoord = aPosition;\n"
            "   gl_Position = (projection * view * vec4(aPosition, 1.0f)).xyww;\n"
            "}\n";

        const char* fsCode =
            "#version 330 core\n"
            "layout(location = 0) out vec3 gPosition;\n"
            "layout(location = 1) out vec3 gNormal;\n"
            "layout(location = 2) out vec3 gAlbedo;\n"
            "layout(location = 3) out vec3 gKa;\n"
            "layout(location = 4) out vec3 gKs;\n"
            "layout(location = 5) out float gNs;\n"

            "in vec3 texCoord;\n"
            "uniform samplerCube cubemap;\n"
            "void main() {\n"
            "   gPosition = vec3(0.0);\n"
            "   gNormal = vec3(0.0);\n"
            "   gAlbedo = texture(cubemap, texCoord).rgb;\n"
            "   gKa = vec3(0.0);\n"
            "   gKs = vec3(0.0);\n"
            "   gNs = 0.0;\n"
            "}\n";

        _shader.reset(new GLSLProgram);
        _shader->attachVertexShader(vsCode);
        _shader->attachFragmentShader(fsCode);
        _shader->link();
    } catch (const std::exception&) {
        cleanup();
        throw;
    }

    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::stringstream ss;
        ss << "skybox creation failure, (code " << error << ")";
        cleanup();
        throw std::runtime_error(ss.str());
    }
}

SkyBox::SkyBox(SkyBox&& rhs) noexcept
    : _vao(rhs._vao), _vbo(rhs._vbo), _texture(std::move(rhs._texture)),
      _shader(std::move(rhs._shader)) {
    rhs._vao = 0;
    rhs._vbo = 0;
}

SkyBox::~SkyBox() {
    cleanup();
}

void SkyBox::draw(const glm::mat4& projection, const glm::mat4& view) {
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_DEPTH_WRITEMASK);
    _shader->use();

    _shader->setUniformMat4("projection", projection);
    _shader->setUniformMat4("view", glm::mat4(glm::mat3(view)));
    _texture->bind();

    glBindVertexArray(_vao);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glEnable(GL_DEPTH_WRITEMASK);
    glDepthFunc(GL_LESS);
}

void SkyBox::cleanup() {
    if (_vbo != 0) {
        glDeleteBuffers(1, &_vbo);
        _vbo = 0;
    }

    if (_vao != 0) {
        glDeleteVertexArrays(1, &_vao);
        _vao = 0;
    }
}