#include <filesystem>
#include <random>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <stb.h>

#include "editor.h"
#include "primitive_factory.h"
#include "base/utils.h"

const std::string geometryVsRelPath = "shader/geometry.vert";
const std::string geometryFsRelPath = "shader/geometry.frag";

const std::string ssaoFsRelPath = "shader/ssao.frag";
const std::string ssaoBlurFsRelPath = "shader/ssao_blur.frag";
const std::string ssaoLightingFsRelPath = "shader/ssao_lighting.frag";

const std::string brightColorFsRelPath = "shader/extract_bright_color.frag";
const std::string gaussianBlurFsRelPath = "shader/gaussian_blur.frag";
const std::string blendBloomMapFsRelPath = "shader/blend_bloom_map.frag";

const std::string quadVsRelPath = "shader/quad.vert";
const std::string quadFsRelPath = "shader/quad.frag";

Editor::Editor(const Options& options) : Application(options) {
    _camera.reset(new PerspectiveCamera(glm::radians(60.0f), 1.0f * _windowWidth / _windowHeight, 0.3f, 1000.0f));
    _camera->transform.position.z = 10.0f;

    Model* ground = PrimitiveFactory::createPlane("Ground", 10, 10, 1, 1);
    ground->transform.position = glm::vec3(0, -2, 0);
    _models.push_back(ground);

    _ambientLight.reset(new AmbientLight("Ambient Light"));
    _ambientLight->intensity = 0.3f;
    _ambientLight->color = glm::vec3(0.21f, 0.239f, 0.3135f);

    DirectionalLight* directionalLight = new DirectionalLight("Directional Light");
    directionalLight->transform.rotation = glm::quat(glm::vec3(glm::radians(-129.0f), glm::radians(-18.0f), 0));
    directionalLight->color = glm::vec3(1, 0.9568627f, 0.8392157f);
    _directionalLights.push_back(directionalLight);

    PointLight* pointLight = new PointLight("Point Light");
    pointLight->transform.position = glm::vec3(-2.21f, 1.18f, 6.68f);
    pointLight->color = glm::vec3(0.8716981f, 0.4588751f, 0.4588751f);
    _pointLights.push_back(pointLight);

    _screenQuad.reset(new FullscreenQuad);

    float defaultData[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    _defaultTexture.reset(new Texture2D(GL_RGBA, 1, 1, GL_RGBA, GL_FLOAT, defaultData));
    _defaultTexture->bind();
    _defaultTexture->setParamterInt(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    _defaultTexture->setParamterInt(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    _defaultTexture->setParamterInt(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    _defaultTexture->setParamterInt(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    _defaultTexture->unbind();

    initGeometryPassResources();

    initSSAOPassResources();

    initBloomPassResources();

    initShaders();

    // init imGUI
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(_window, true);
    ImGui_ImplOpenGL3_Init();

    // 设置初始位置
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    renderScenePanel();
    ImGui::SetNextWindowPos(ImVec2(_windowWidth * 0.70, 0));
    renderInspectorPanel();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

Editor::~Editor() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void Editor::initGeometryPassResources() {
    _gPosition.reset(new Texture2D(GL_RGB32F, _windowWidth, _windowHeight, GL_RGB, GL_FLOAT));
    _gPosition->bind();
    _gPosition->setParamterInt(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    _gPosition->setParamterInt(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    _gPosition->setParamterInt(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    _gPosition->setParamterInt(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    _gPosition->unbind();

    _gNormal.reset(new Texture2D(GL_RGB32F, _windowWidth, _windowHeight, GL_RGB, GL_FLOAT));
    _gNormal->bind();
    _gNormal->setParamterInt(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    _gNormal->setParamterInt(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    _gNormal->setParamterInt(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    _gNormal->setParamterInt(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    _gNormal->unbind();

    _gAlbedo.reset(new Texture2D(GL_RGB32F, _windowWidth, _windowHeight, GL_RGB, GL_FLOAT));
    _gAlbedo->bind();
    _gAlbedo->setParamterInt(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    _gAlbedo->setParamterInt(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    _gAlbedo->setParamterInt(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    _gAlbedo->setParamterInt(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    _gAlbedo->unbind();

    _gKa.reset(new Texture2D(GL_RGB32F, _windowWidth, _windowHeight, GL_RGB, GL_FLOAT));
    _gKa->bind();
    _gKa->setParamterInt(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    _gKa->setParamterInt(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    _gKa->setParamterInt(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    _gKa->setParamterInt(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    _gKa->unbind();

    _gKs.reset(new Texture2D(GL_RGB32F, _windowWidth, _windowHeight, GL_RGB, GL_FLOAT));
    _gKs->bind();
    _gKs->setParamterInt(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    _gKs->setParamterInt(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    _gKs->setParamterInt(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    _gKs->setParamterInt(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    _gKs->unbind();

    _gNs.reset(new Texture2D(GL_RGB32F, _windowWidth, _windowHeight, GL_RED, GL_FLOAT));
    _gNs->bind();
    _gNs->setParamterInt(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    _gNs->setParamterInt(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    _gNs->setParamterInt(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    _gNs->setParamterInt(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    _gNs->unbind();

    _gDepth.reset(new Texture2D(
        GL_DEPTH_COMPONENT, _windowWidth, _windowHeight, GL_DEPTH_COMPONENT, GL_FLOAT));
    _gDepth->bind();
    _gDepth->setParamterInt(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    _gDepth->setParamterInt(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    _gDepth->setParamterInt(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    _gDepth->setParamterInt(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    _gBufferFBO.reset(new Framebuffer);
    _gBufferFBO->bind();
    _gBufferFBO->drawBuffers({ GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5 });
    _gBufferFBO->attachTexture2D(*_gPosition, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D);
    _gBufferFBO->attachTexture2D(*_gNormal, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D);
    _gBufferFBO->attachTexture2D(*_gAlbedo, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D);
    _gBufferFBO->attachTexture2D(*_gKa, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D);
    _gBufferFBO->attachTexture2D(*_gKs, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D);
    _gBufferFBO->attachTexture2D(*_gNs, GL_COLOR_ATTACHMENT5, GL_TEXTURE_2D);
    _gBufferFBO->attachTexture2D(*_gDepth, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D);

    if (_gBufferFBO->checkStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        throw std::runtime_error("_gBufferFBO is imcomplete for rendering");
    }

    _gBufferFBO->unbind();

    _gBufferShader.reset(new GLSLProgram);
    _gBufferShader->attachVertexShaderFromFile(getAssetFullPath(geometryVsRelPath));
    _gBufferShader->attachFragmentShaderFromFile(getAssetFullPath(geometryFsRelPath));
    _gBufferShader->link();
}

void Editor::initSSAOPassResources() {
    _ssaoFBO.reset(new Framebuffer);
    _ssaoFBO->bind();
    _ssaoFBO->drawBuffer(GL_COLOR_ATTACHMENT0);
    for (int i = 0; i < 2; ++i) {
        _ssaoResult[i].reset(new Texture2D(GL_R32F, _windowWidth, _windowHeight, GL_RED, GL_FLOAT));
        _ssaoResult[i]->bind();
        _ssaoResult[i]->setParamterInt(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        _ssaoResult[i]->setParamterInt(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        _ssaoResult[i]->setParamterInt(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        _ssaoResult[i]->setParamterInt(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    _ssaoFBO->attachTexture2D(*_ssaoResult[0], GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D);

    if (_ssaoFBO->checkStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        throw std::runtime_error("_ssaoFBO is imcomplete for rendering");
    }

    _ssaoFBO->unbind();

    _ssaoBlurFBO.reset(new Framebuffer);
    _ssaoBlurFBO->bind();
    _ssaoBlurFBO->drawBuffer(GL_COLOR_ATTACHMENT0);
    _ssaoBlurFBO->unbind();

    std::default_random_engine e;
    std::uniform_real_distribution<float> u(0.0f, 1.0f);

    for (unsigned int i = 0; i < 64; ++i) {
        glm::vec3 sample(u(e) * 2.0f - 1.0f, u(e) * 2.0f - 1.0f, u(e));
        sample = glm::normalize(sample);
        sample *= u(e);
        float scale = float(i) / 64.0f;

        // scale samples s.t. they're more aligned to center of kernel
        scale = glm::mix(0.1f, 1.0f, scale * scale);
        sample *= scale;
        _sampleVecs.push_back(sample);
    }

    std::vector<glm::vec3> ssaoNoises;
    for (unsigned int i = 0; i < 16; i++) {
        // rotate around z-axis (in tangent space)
        glm::vec3 noise(u(e) * 2.0f - 1.0f, u(e) * 2.0f - 1.0f, 0.0f);
        ssaoNoises.push_back(noise);
    }

    _ssaoNoise.reset(new Texture2D(GL_RGB32F, 4, 4, GL_RGB, GL_FLOAT, ssaoNoises.data()));
    _ssaoNoise->bind();
    _ssaoNoise->setParamterInt(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    _ssaoNoise->setParamterInt(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    _ssaoNoise->setParamterInt(GL_TEXTURE_WRAP_S, GL_REPEAT);
    _ssaoNoise->setParamterInt(GL_TEXTURE_WRAP_T, GL_REPEAT);
    _ssaoNoise->unbind();

    _ssaoShader.reset(new GLSLProgram);
    _ssaoShader->attachVertexShaderFromFile(getAssetFullPath(quadVsRelPath));
    _ssaoShader->attachFragmentShaderFromFile(getAssetFullPath(ssaoFsRelPath));
    _ssaoShader->link();

    _ssaoBlurShader.reset(new GLSLProgram);
    _ssaoBlurShader->attachVertexShaderFromFile(getAssetFullPath(quadVsRelPath));
    _ssaoBlurShader->attachFragmentShaderFromFile(getAssetFullPath(ssaoBlurFsRelPath));
    _ssaoBlurShader->link();

    _ssaoLightingShader.reset(new GLSLProgram);
    _ssaoLightingShader->attachVertexShaderFromFile(getAssetFullPath(quadVsRelPath));
    _ssaoLightingShader->attachFragmentShaderFromFile(getAssetFullPath(ssaoLightingFsRelPath));
    _ssaoLightingShader->link();
}

void Editor::initBloomPassResources() {
    _bloomFBO.reset(new Framebuffer);
    _bloomFBO->bind();
    _bloomFBO->drawBuffer(GL_COLOR_ATTACHMENT0);

    _bloomMap.reset(new Texture2D(GL_RGBA32F, _windowWidth, _windowHeight, GL_RGBA, GL_FLOAT));
    _bloomMap->bind();
    _bloomMap->setParamterInt(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    _bloomMap->setParamterInt(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    _bloomMap->setParamterInt(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    _bloomMap->setParamterInt(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    _bloomFBO->attachTexture2D(*_bloomMap, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D);
    for (int i = 0; i < 2; ++i) {
        _brightColorMap[i].reset(
            new Texture2D(GL_RGBA32F, _windowWidth, _windowHeight, GL_RGBA, GL_FLOAT));
        _brightColorMap[i]->bind();
        _brightColorMap[i]->setParamterInt(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        _brightColorMap[i]->setParamterInt(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        _brightColorMap[i]->setParamterInt(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        _brightColorMap[i]->setParamterInt(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    _bloomFBO->attachTexture2D(*_gDepth, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D);

    if (_bloomFBO->checkStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        throw std::runtime_error("_bloomFBO is imcomplete for rendering");
    }

    _bloomFBO->unbind();

    _brightColorFBO.reset(new Framebuffer);
    _brightColorFBO->bind();
    _brightColorFBO->drawBuffer(GL_COLOR_ATTACHMENT0);
    _brightColorFBO->unbind();

    _blurFBO.reset(new Framebuffer);
    _blurFBO->bind();
    _blurFBO->drawBuffer(GL_COLOR_ATTACHMENT0);
    _blurFBO->unbind();

    _brightColorShader.reset(new GLSLProgram);
    _brightColorShader->attachVertexShaderFromFile(getAssetFullPath(quadVsRelPath));
    _brightColorShader->attachFragmentShaderFromFile(getAssetFullPath(brightColorFsRelPath));
    _brightColorShader->link();

    _blurShader.reset(new GLSLProgram);
    _blurShader->attachVertexShaderFromFile(getAssetFullPath(quadVsRelPath));
    _blurShader->attachFragmentShaderFromFile(getAssetFullPath(gaussianBlurFsRelPath));
    _blurShader->link();

    _blendShader.reset(new GLSLProgram);
    _blendShader->attachVertexShaderFromFile(getAssetFullPath(quadVsRelPath));
    _blendShader->attachFragmentShaderFromFile(getAssetFullPath(blendBloomMapFsRelPath));
    _blendShader->link();
}

void Editor::initShaders() {
    _drawScreenShader.reset(new GLSLProgram);
    _drawScreenShader->attachVertexShaderFromFile(getAssetFullPath(quadVsRelPath));
    _drawScreenShader->attachFragmentShaderFromFile(getAssetFullPath(quadFsRelPath));
    _drawScreenShader->link();
}

void Editor::handleInput() {
    if (_input.keyboard.keyStates[GLFW_KEY_ESCAPE] != GLFW_RELEASE) {
        glfwSetWindowShouldClose(_window, true);
        return;
    }

    if (ImGui::IsAnyItemActive()) {
        return;
    }

    constexpr float cameraMoveSpeed = 5.0f;
    constexpr float cameraRotateSpeed = 0.1f;

    float mouse_movement_in_x_direction = _input.mouse.move.xNow - _input.mouse.move.xOld;
    float mouse_movement_in_y_direction = _input.mouse.move.yNow - _input.mouse.move.yOld;
    
    if (_input.keyboard.keyStates[GLFW_KEY_F] != GLFW_RELEASE) {
        zoomToFit();
    }

    // 平移控制 - 左键拖动
    if (_input.mouse.press.left) {
        // 根据鼠标移动更新相机的位置，使用上下左右方向进行平移
        if (mouse_movement_in_x_direction != 0) {
            _camera->transform.position -= cameraMoveSpeed * _deltaTime * mouse_movement_in_x_direction * _camera->transform.getRight();
        }
        if (mouse_movement_in_y_direction != 0) {
            _camera->transform.position -= cameraMoveSpeed * _deltaTime * mouse_movement_in_y_direction * _camera->transform.getUp();
        }
    }

    if (_input.mouse.press.right) {
        if (_input.keyboard.keyStates[GLFW_KEY_W] != GLFW_RELEASE) {
            _camera->transform.position += cameraMoveSpeed * _deltaTime * _camera->transform.getFront();
        }

        if (_input.keyboard.keyStates[GLFW_KEY_A] != GLFW_RELEASE) {
            _camera->transform.position -= cameraMoveSpeed * _deltaTime * _camera->transform.getRight();
        }

        if (_input.keyboard.keyStates[GLFW_KEY_S] != GLFW_RELEASE) {
            _camera->transform.position -= cameraMoveSpeed * _deltaTime * _camera->transform.getFront();
        }

        if (_input.keyboard.keyStates[GLFW_KEY_D] != GLFW_RELEASE) {
            _camera->transform.position += cameraMoveSpeed * _deltaTime * _camera->transform.getRight();
        }

        if (_input.keyboard.keyStates[GLFW_KEY_E] != GLFW_RELEASE) {
            _camera->transform.position += cameraMoveSpeed * _deltaTime * _camera->transform.getUp();
        }

        if (_input.keyboard.keyStates[GLFW_KEY_Q] != GLFW_RELEASE) {
            _camera->transform.position -= cameraMoveSpeed * _deltaTime * _camera->transform.getUp();
        }

        // 水平旋转
        if (mouse_movement_in_x_direction != 0) {
            float delta_angle_x = mouse_movement_in_x_direction * cameraRotateSpeed;
            glm::quat qx = glm::angleAxis(glm::radians(-delta_angle_x), glm::vec3(0.0f, 1.0f, 0.0f));
            _camera->transform.rotation = qx * _camera->transform.rotation;
        }

        // 垂直旋转
        if (mouse_movement_in_y_direction != 0) {
            float delta_angle_y = mouse_movement_in_y_direction * cameraRotateSpeed;
            glm::quat qy = glm::angleAxis(glm::radians(-delta_angle_y), _camera->transform.getRight());
            _camera->transform.rotation = qy * _camera->transform.rotation;
        }
    }

    _input.forwardState();
}

void Editor::renderFrame() {
    showFpsInWindowTitle();
    
    renderScene();
    renderUI();
}

std::vector<std::string> Editor::getModelFiles() const {
    std::vector<std::string> modelFiles;
    for (const auto& entry : std::filesystem::directory_iterator(getAssetFullPath("obj"))) {
        if (entry.is_regular_file() && entry.path().extension() == ".obj") {
            modelFiles.push_back(entry.path().filename().string());
        }
    }
    return modelFiles;
}

void Editor::renderScene() {
    glClearColor(_clearColor.r, _clearColor.g, _clearColor.b, _clearColor.a);

    // deferred rendering: geometry pass
    _gBufferFBO->bind();
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    _gBufferShader->use();
    _gBufferShader->setUniformMat4("projection", _camera->getProjectionMatrix());
    _gBufferShader->setUniformMat4("view", _camera->getViewMatrix());

    for (auto* model : _models) {
        _gBufferShader->setUniformMat4("model", model->transform.getLocalMatrix());
        _gBufferShader->setUniformVec3("material.ka", model->material.ka);
        _gBufferShader->setUniformVec3("material.kd", model->material.kd);
        _gBufferShader->setUniformVec3("material.ks", model->material.ks);
        _gBufferShader->setUniformFloat("material.ns", model->material.ns);
        if (model->material.texture.get() != nullptr) {
            model->material.texture->bind();
        } else {
            _defaultTexture->bind();
        }
        model->draw();
    }
    _gBufferFBO->unbind();
    
    // deferred rendering: lighting passes
    // + SSAO pass
    if (_enableSSAO) {
        glDisable(GL_DEPTH_TEST);

        _ssaoFBO->bind();

        _ssaoShader->use();
        _ssaoShader->setUniformInt("gPosition", 0);
        _gPosition->bind(0);
        _ssaoShader->setUniformInt("gNormal", 1);
        _gNormal->bind(1);
        _ssaoShader->setUniformInt("gDepth", 2);
        _gDepth->bind(2);
        _ssaoShader->setUniformInt("noiseMap", 3);
        _ssaoNoise->bind(3);
        for (size_t i = 0; i < _sampleVecs.size(); ++i) {
            _ssaoShader->setUniformVec3("sampleVecs[" + std::to_string(i) + "]", _sampleVecs[i]);
        }

        _ssaoShader->setUniformInt("screenWidth", _windowWidth);
        _ssaoShader->setUniformInt("screenHeight", _windowHeight);
        _ssaoShader->setUniformFloat("zNear", ((PerspectiveCamera*)_camera.get())->znear);
        _ssaoShader->setUniformFloat("zFar", ((PerspectiveCamera*)_camera.get())->zfar);
        _ssaoShader->setUniformMat4("projection", _camera->getProjectionMatrix());
        _screenQuad->draw();

        _ssaoFBO->unbind();

        _ssaoBlurFBO->bind();

        _currentReadBuffer = 0;
        _currentWriteBuffer = 1;
        _ssaoBlurShader->use();
        for (int pass = 0; pass < 5; ++pass) {
            _ssaoBlurFBO->attachTexture2D(
                *_ssaoResult[_currentWriteBuffer], GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D);
            _ssaoBlurShader->setUniformInt("ssaoResult", 0);
            _ssaoResult[_currentReadBuffer]->bind(0);
            _screenQuad->draw();

            std::swap(_currentReadBuffer, _currentWriteBuffer);
        }
        
        _ssaoBlurFBO->unbind();
    } else {
        _currentReadBuffer = 0;
        static const std::vector<float> ones(_windowWidth * _windowHeight, 1.0f);
        _ssaoResult[0]->bind();
        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_R32F, _windowWidth, _windowHeight, 0, GL_RED, GL_FLOAT,
            ones.data());
        _ssaoResult[0]->unbind();
    }

    // + bloom pass
    _bloomFBO->bind();
    glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT);
    
    _ssaoLightingShader->use();
    
    _ssaoLightingShader->setUniformVec3("ambientLight.color", _ambientLight->color);
    _ssaoLightingShader->setUniformFloat("ambientLight.intensity", _ambientLight->intensity);
    _ssaoLightingShader->setUniformInt("nDirectionalLight", _directionalLights.size());
   
    for (size_t i = 0; i < _directionalLights.size(); i++) {
        _ssaoLightingShader->setUniformVec3("directionalLights[" + std::to_string(i) + "].direction", _directionalLights[i]->transform.getFront());
        _ssaoLightingShader->setUniformFloat("directionalLights[" + std::to_string(i) + "].intensity", _directionalLights[i]->intensity);
        _ssaoLightingShader->setUniformVec3("directionalLights[" + std::to_string(i) + "].color", _directionalLights[i]->color);
    }
    _ssaoLightingShader->setUniformInt("nPointLight", _pointLights.size());
    for (size_t i = 0; i < _pointLights.size(); i++) {
        _ssaoLightingShader->setUniformVec3("pointLights[" + std::to_string(i) + "].position", _pointLights[i]->transform.position);
        _ssaoLightingShader->setUniformFloat("pointLights[" + std::to_string(i) + "].intensity", _pointLights[i]->intensity);
        _ssaoLightingShader->setUniformVec3("pointLights[" + std::to_string(i) + "].color", _pointLights[i]->color);
        _ssaoLightingShader->setUniformFloat("pointLights[" + std::to_string(i) + "].kc", _pointLights[i]->kc);
        _ssaoLightingShader->setUniformFloat("pointLights[" + std::to_string(i) + "].kq", _pointLights[i]->kq);
        _ssaoLightingShader->setUniformFloat("pointLights[" + std::to_string(i) + "].kl", _pointLights[i]->kl);
    }
    _ssaoLightingShader->setUniformInt("nSpotLight", _spotLights.size());
    for (size_t i = 0; i < _spotLights.size(); i++) {
        _ssaoLightingShader->setUniformVec3("spotLights[" + std::to_string(i) + "].position", _spotLights[i]->transform.position);
        _ssaoLightingShader->setUniformVec3("spotLights[" + std::to_string(i) + "].direction", _spotLights[i]->transform.getFront());
        _ssaoLightingShader->setUniformFloat("spotLights[" + std::to_string(i) + "].intensity", _spotLights[i]->intensity);
        _ssaoLightingShader->setUniformVec3("spotLights[" + std::to_string(i) + "].color", _spotLights[i]->color);
        _ssaoLightingShader->setUniformFloat("spotLights[" + std::to_string(i) + "].angle", _spotLights[i]->angle);
        _ssaoLightingShader->setUniformFloat("spotLights[" + std::to_string(i) + "].kc", _spotLights[i]->kc);
        _ssaoLightingShader->setUniformFloat("spotLights[" + std::to_string(i) + "].kq", _spotLights[i]->kq);
        _ssaoLightingShader->setUniformFloat("spotLights[" + std::to_string(i) + "].kl", _spotLights[i]->kl);
    }
    
    _ssaoLightingShader->setUniformVec3("viewPos", _camera->transform.position);

    _ssaoLightingShader->setUniformInt("gPosition", 0);
    _gPosition->bind(0);
    _ssaoLightingShader->setUniformInt("gNormal", 1);
    _gNormal->bind(1);
    _ssaoLightingShader->setUniformInt("gAlbedo", 2);
    _gAlbedo->bind(2);
    _ssaoLightingShader->setUniformInt("gKa", 3);
    _gKa->bind(3);
    _ssaoLightingShader->setUniformInt("gKs", 4);
    _gKs->bind(4);
    _ssaoLightingShader->setUniformInt("gNs", 5);
    _gNs->bind(5);
    _ssaoLightingShader->setUniformInt("ssaoResult", 6);
    _ssaoResult[_currentReadBuffer]->bind(6);

    _screenQuad->draw();

    glEnable(GL_DEPTH_TEST);

    _bloomFBO->unbind();

    if (_enableBloom) {
        extractBrightColor(*_bloomMap);
        blurBrightColor();
        combineSceneMapAndBloomBlur(*_bloomMap);
    } else {
        glDisable(GL_DEPTH_TEST);
        _drawScreenShader->use();
        _drawScreenShader->setUniformInt("frame", 0);
        _bloomMap->bind(0);
        _screenQuad->draw();
    }

}

void Editor::renderUI() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    renderScenePanel();
    renderInspectorPanel();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Editor::renderInspectorPanel() {
    const auto flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings;
    
    if (!ImGui::Begin("Inspector", nullptr, flags)) {
        ImGui::End();
        return;
    }

    if (selectedObject != nullptr) {
        selectedObject->renderInspector();
        if (dynamic_cast<AmbientLight*>(selectedObject) == nullptr && ImGui::Button("Delete Object")) {
            ImGui::OpenPopup("Delete Object");
        }
    } else {
        ImGui::Text("No object selected");
    }

    renderPopupModal();
    ImGui::End();
}

static char objectNameBuffer[128] = "";
enum LightType {
    Directional,
    Point,
    Spot
};
static LightType lightType = Directional;
enum AddModelOption {
    OpenFile,
    PrimitiveShape
};
static AddModelOption option = OpenFile;

void Editor::renderScenePanel() {
    const auto flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings;
    
    if (!ImGui::Begin("Scene", nullptr, flags)) {
        ImGui::End();
        return;
    }

    int itemCount = 0;
    if (ImGui::CollapsingHeader("Models              ")) {
        for (int i = 0; i < _models.size(); i++) {
            std::string label = _models[i]->name + "##" + std::to_string(itemCount++);
            if (ImGui::Selectable(label.c_str(), selectedObject == _models[i])) {
                selectedObject = _models[i];
            }
        }

        if (ImGui::Button("       Add Model       ")) {
            ImGui::OpenPopup("Add Model");
            option = OpenFile;
            // 清空输入缓冲区
            memset(objectNameBuffer, 0, sizeof(objectNameBuffer));
            memcpy(objectNameBuffer, "New Model", sizeof("New Model"));
        }
    }

    if (ImGui::CollapsingHeader("Lights              ")) {
        if (ImGui::Selectable(_ambientLight->name.c_str(), selectedObject == _ambientLight.get())) {
            selectedObject = _ambientLight.get();
        }
        for (int i = 0; i < _directionalLights.size(); i++) {
            std::string label = _directionalLights[i]->name + "##" + std::to_string(itemCount++);
            if (ImGui::Selectable(label.c_str(), selectedObject == _directionalLights[i])) {
                selectedObject = _directionalLights[i];
            }
        }
        for (int i = 0; i < _pointLights.size(); i++) {
            std::string label = _pointLights[i]->name + "##" + std::to_string(itemCount++);
            if (ImGui::Selectable(label.c_str(), selectedObject == _pointLights[i])) {
                selectedObject = _pointLights[i];
            }
        }
        for (int i = 0; i < _spotLights.size(); i++) {
            std::string label = _spotLights[i]->name + "##" + std::to_string(itemCount++);
            if (ImGui::Selectable(label.c_str(), selectedObject == _spotLights[i])) {
                selectedObject = _spotLights[i];
            }
        }

        if (ImGui::Button("       Add Light       ")) {
            ImGui::OpenPopup("Add Light");
            lightType = Directional;
            // 清空输入缓冲区
            memset(objectNameBuffer, 0, sizeof(objectNameBuffer));
            memcpy(objectNameBuffer, "New Light", sizeof("New Light"));
        }
    }

    ImGui::Separator();
    ImGui::Checkbox("bloom", &_enableBloom);
    ImGui::SameLine();
    ImGui::Checkbox("ssao", &_enableSSAO);

    if (_input.keyboard.keyStates[GLFW_KEY_P] != GLFW_RELEASE) {
        ImGui::OpenPopup("Screen Shot");
        // 清空输入缓冲区
        memset(objectNameBuffer, 0, sizeof(objectNameBuffer));
    }

    renderPopupModal();
    ImGui::End();
}

void Editor::renderPopupModal() {
    if (ImGui::BeginPopupModal("Add Model", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        try {
            renderAddModelPanel();
        } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    } else if (ImGui::BeginPopupModal("Add Light", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Enter Light Name:");
        ImGui::InputText("##LightName", objectNameBuffer, sizeof(objectNameBuffer));

        ImGui::RadioButton("Directional Light", (int*)&lightType, Directional);
        ImGui::RadioButton("Point Light", (int*)&lightType, Point);
        ImGui::RadioButton("Spot Light", (int*)&lightType, Spot);

        // 确认按钮
        if (ImGui::Button("OK", ImVec2(220, 0))) {
            if (strlen(objectNameBuffer) > 0) {
                switch (lightType) {
                case Directional:
                    _directionalLights.push_back(new DirectionalLight(objectNameBuffer));
                    break;
                case Point:
                    _pointLights.push_back(new PointLight(objectNameBuffer));
                    break;
                case Spot:
                    _spotLights.push_back(new SpotLight(objectNameBuffer));
                    break;
                }
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();

        // 取消按钮
        if (ImGui::Button("Cancel", ImVec2(220, 0))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    } else if (ImGui::BeginPopupModal("Delete Object", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        // 确认按钮
        if (ImGui::Button("OK", ImVec2(220, 0))) {
            auto it0 = std::remove(_models.begin(), _models.end(), dynamic_cast<Model*>(selectedObject));
            if (it0 != _models.end()) {
                _models.erase(it0, _models.end());
            }
            auto it1 = std::remove(_directionalLights.begin(), _directionalLights.end(), dynamic_cast<DirectionalLight*>(selectedObject));
            if (it1 != _directionalLights.end()) {
                _directionalLights.erase(it1, _directionalLights.end());
            }
            auto it2 = std::remove(_pointLights.begin(), _pointLights.end(), dynamic_cast<PointLight*>(selectedObject));
            if (it2 != _pointLights.end()) {
                _pointLights.erase(it2, _pointLights.end());
            }
            auto it3 = std::remove(_spotLights.begin(), _spotLights.end(), dynamic_cast<SpotLight*>(selectedObject));
            if (it3 != _spotLights.end()) {
                _spotLights.erase(it3, _spotLights.end());
            }
            delete selectedObject;
            selectedObject = nullptr;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();

        // 取消按钮
        if (ImGui::Button("Cancel", ImVec2(220, 0))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    } else if (ImGui::BeginPopupModal("Screen Shot", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Enter File Name:");
        ImGui::InputText("##FileName", objectNameBuffer, sizeof(objectNameBuffer));
        // 确认按钮
        if (ImGui::Button("OK", ImVec2(220, 0)) && strlen(objectNameBuffer) > 0) {
            captureScreen("../screenshots/" + std::string(objectNameBuffer) + ".png");
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();

        // 取消按钮
        if (ImGui::Button("Cancel", ImVec2(220, 0))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void Editor::renderAddModelPanel() {
    ImGui::Text("Enter Model Name:");
    ImGui::InputText("##ModelName", objectNameBuffer, sizeof(objectNameBuffer));
    
    ImGui::Text("Create Model:");
    ImGui::RadioButton("Open File", (int*)&option, OpenFile);
    ImGui::RadioButton("Primitive Shape", (int*)&option, PrimitiveShape);

    static std::string selectedModel = ""; // 保存当前选中的自定义模型名
    static std::vector<std::string> modelFiles = getModelFiles(); // 获取模型文件列表

    switch (option) {
    case OpenFile:
        // 模型选择下拉菜单
        ImGui::Text("Select Model:");
        if (ImGui::BeginCombo("##ModelSelector", selectedModel.c_str())) {
            for (const auto& model : modelFiles) {
                bool isSelected = (selectedModel == model);
                if (ImGui::Selectable(model.c_str(), isSelected)) {
                    selectedModel = model;
                }
                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
        if (ImGui::Button("OK", ImVec2(220, 0))) {
            if (strlen(objectNameBuffer) > 0 && !selectedModel.empty()) {
                _models.push_back(new Model(objectNameBuffer, getAssetFullPath("obj/" + selectedModel)));
            }
            ImGui::CloseCurrentPopup();
        }
        break;
    case PrimitiveShape:
        // 基础形状参数
        static std::string selectedShape = "Plane";
        static const char* shapes[] = { "Plane", "Cube", "Sphere", "Cylinder", "Cone", "Prism", "Frustum" };
        ImGui::Text("Select Shape:");
        if (ImGui::BeginCombo("##ShapeSelector", selectedShape.c_str())) {
            for (int i = 0; i < IM_ARRAYSIZE(shapes); ++i) {
                bool isSelected = (selectedShape == shapes[i]);
                if (ImGui::Selectable(shapes[i], isSelected)) {
                    selectedShape = shapes[i];
                }
                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        // 根据选择的形状动态显示参数
        if (selectedShape == "Plane") {
            static float width = 1.0f, height = 1.0f;
            static int segmentsX = 1, segmentsY = 1;
            ImGui::InputFloat("Width", &width);
            ImGui::InputFloat("Height", &height);
            ImGui::InputInt("Segments X", &segmentsX);
            ImGui::InputInt("Segments Y", &segmentsY);

            if (ImGui::Button("OK", ImVec2(220, 0))) {
                if (strlen(objectNameBuffer) > 0) {
                    _models.push_back(PrimitiveFactory::createPlane(objectNameBuffer, width, height, segmentsX, segmentsY));
                }
                ImGui::CloseCurrentPopup();
            }
        } else if (selectedShape == "Cube") {
            static float size = 1.0f;
            ImGui::InputFloat("Size", &size);

            if (ImGui::Button("OK", ImVec2(220, 0))) {
                if (strlen(objectNameBuffer) > 0) {
                    _models.push_back(PrimitiveFactory::createCube(objectNameBuffer, size));
                }
                ImGui::CloseCurrentPopup();
            }
        } else if (selectedShape == "Sphere") {
            static float radius = 1.0f;
            static int sectors = 16, stacks = 16;
            ImGui::InputFloat("Radius", &radius);
            ImGui::InputInt("Sectors", &sectors);
            ImGui::InputInt("Stacks", &stacks);

            if (ImGui::Button("OK", ImVec2(220, 0))) {
                if (strlen(objectNameBuffer) > 0) {
                    _models.push_back(PrimitiveFactory::createSphere(objectNameBuffer, radius, sectors, stacks));
                }
                ImGui::CloseCurrentPopup();
            }
        } else if (selectedShape == "Cylinder") {
            static float radius = 1.0f, height = 2.0f;
            static int radialSegments = 16, heightSegments = 1;
            ImGui::InputFloat("Radius", &radius);
            ImGui::InputFloat("Height", &height);
            ImGui::InputInt("Radial Segments", &radialSegments);
            ImGui::InputInt("Height Segments", &heightSegments);

            if (ImGui::Button("OK", ImVec2(220, 0))) {
                if (strlen(objectNameBuffer) > 0) {
                    _models.push_back(PrimitiveFactory::createCylinder(objectNameBuffer, radius, height, radialSegments, heightSegments));
                }
                ImGui::CloseCurrentPopup();
            }
        } else if (selectedShape == "Cone") {
            static float radius = 1.0f, height = 2.0f;
            static int radialSegments = 16;
            ImGui::InputFloat("Radius", &radius);
            ImGui::InputFloat("Height", &height);
            ImGui::InputInt("Radial Segments", &radialSegments);

            if (ImGui::Button("OK", ImVec2(220, 0))) {
                if (strlen(objectNameBuffer) > 0) {
                    _models.push_back(PrimitiveFactory::createCone(objectNameBuffer, radius, height, radialSegments));
                }
                ImGui::CloseCurrentPopup();
            }
        } else if (selectedShape == "Prism") {
            static float radius = 1.0f, height = 2.0f;
            static int sides = 3, heightSegments = 1;
            ImGui::InputFloat("Radius", &radius);
            ImGui::InputFloat("Height", &height);
            ImGui::InputInt("Sides", &sides);
            ImGui::InputInt("Height Segments", &heightSegments);

            if (ImGui::Button("OK", ImVec2(220, 0))) {
                if (strlen(objectNameBuffer) > 0) {
                    _models.push_back(PrimitiveFactory::createPrism(objectNameBuffer, radius, height, sides, heightSegments));
                }
                ImGui::CloseCurrentPopup();
            }
        } else if (selectedShape == "Frustum") {
            static float bottomRadius = 1.0f, topRadius = 0.5f, height = 2.0f;
            static int sides = 3, heightSegments = 1;
            ImGui::InputFloat("Bottom Radius", &bottomRadius);
            ImGui::InputFloat("Top Radius", &topRadius);
            ImGui::InputFloat("Height", &height);
            ImGui::InputInt("Sides", &sides);
            ImGui::InputInt("Height Segments", &heightSegments, 1, 100);

            if (ImGui::Button("OK", ImVec2(220, 0))) {
                if (strlen(objectNameBuffer) > 0) {
                    _models.push_back(PrimitiveFactory::createFrustum(objectNameBuffer, bottomRadius, topRadius, height, sides, heightSegments));
                }
                ImGui::CloseCurrentPopup();
            }
        }
        break;
    }
    ImGui::SameLine();
    // 取消按钮
    if (ImGui::Button("Cancel", ImVec2(220, 0))) {
        ImGui::CloseCurrentPopup();
    }
}

void Editor::extractBrightColor(const Texture2D& sceneMap) {
    _brightColorFBO->bind();
    _brightColorFBO->attachTexture2D(*_brightColorMap[0], GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D);
    _brightColorShader->use();
    _brightColorShader->setUniformInt("sceneMap", 0);
    sceneMap.bind(0);
    _screenQuad->draw();
    _brightColorFBO->unbind();
}

void Editor::blurBrightColor() {
    _blurFBO->bind();
    _blurFBO->drawBuffer(GL_COLOR_ATTACHMENT0);
    _blurShader->use();
    bool horizontal = true;
    _blurShader->setUniformInt("image", 0);
    _currentReadBuffer = 0;
    _currentWriteBuffer = 1;

    for (int pass = 0; pass < 20; ++pass) {
        _blurShader->setUniformBool("horizontal", horizontal);
        _blurFBO->attachTexture2D(
            *_brightColorMap[_currentWriteBuffer], GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D);
        _brightColorMap[_currentReadBuffer]->bind(0);
        _screenQuad->draw();
        horizontal = !horizontal;
        std::swap(_currentReadBuffer, _currentWriteBuffer);
    }

    _blurFBO->unbind();
}

void Editor::combineSceneMapAndBloomBlur(const Texture2D& sceneMap) {
    glDisable(GL_DEPTH_TEST);
    _blendShader->use();

    _blendShader->setUniformInt("scene", 0);
    sceneMap.bind(0);

    _blendShader->setUniformInt("bloomBlur", 1);
    _brightColorMap[_currentReadBuffer]->bind(1);

    _screenQuad->draw();
}

void Editor::zoomToFit() {
    if (selectedObject == nullptr) {
        return;
    }
    Model* model = dynamic_cast<Model*>(selectedObject);
    if (model == nullptr) {
        return;
    }

    BoundingBox& bbox = model->getBoundingBox();

    glm::vec3 center = (bbox.max + bbox.min) * 0.5f;
    float radius = glm::distance(bbox.max, bbox.min) * 0.5f;

    // 计算距离，使目标适合屏幕
    float marginFactor = 1.1f;
    float distance = radius / std::tan(_camera->fovy / 2.0f) * marginFactor;

    // 更新相机位置，使相机朝向目标中心
    _camera->transform.position = center - distance * _camera->transform.getFront();
    _camera->transform.lookAt(center);
}

void Editor::captureScreen(const std::string& filepath) const {
    std::vector<unsigned char> pixels(_windowWidth * _windowHeight * 4);

    // 读取帧缓冲中的像素数据
    glReadPixels(0, 0, _windowWidth, _windowHeight, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

    std::vector<unsigned char> flippedPixels(_windowWidth * _windowHeight * 4);
    for (int y = 0; y < _windowHeight; ++y) {
        std::copy(
            pixels.begin() + y * _windowWidth * 4,
            pixels.begin() + (y + 1) * _windowWidth * 4,
            flippedPixels.begin() + (_windowHeight - 1 - y) * _windowWidth * 4
        );
    }

    // 写入 PNG 文件
    stbi_write_png(filepath.c_str(), _windowWidth, _windowHeight, 4, flippedPixels.data(), _windowWidth * 4);
}