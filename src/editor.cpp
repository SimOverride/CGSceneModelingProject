#include <filesystem>
#include <random>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "editor.h"

const std::string geometryVsRelPath = "shader/bonus3/geometry.vert";
const std::string geometryFsRelPath = "shader/bonus3/geometry.frag";

const std::string ssaoFsRelPath = "shader/bonus3/ssao.frag";
const std::string ssaoBlurFsRelPath = "shader/bonus3/ssao_blur.frag";
const std::string ssaoLightingFsRelPath = "shader/bonus3/ssao_lighting.frag";

const std::string lightVsRelPath = "shader/bonus3/light.vert";
const std::string lightFsRelPath = "shader/bonus3/light.frag";

const std::string brightColorFsRelPath = "shader/bonus3/extract_bright_color.frag";
const std::string gaussianBlurFsRelPath = "shader/bonus3/gaussian_blur.frag";
const std::string blendBloomMapFsRelPath = "shader/bonus3/blend_bloom_map.frag";

const std::string quadVsRelPath = "shader/bonus3/quad.vert";
const std::string quadFsRelPath = "shader/bonus3/quad.frag";

Editor::Editor(const Options& options) : Application(options) {
    _camera.reset(new PerspectiveCamera(glm::radians(60.0f), 1.0f * _windowWidth / _windowHeight, 0.3f, 1000.0f));
    _camera->transform.position.z = 10.0f;

    _ambientLight.reset(new AmbientLight("Ambient Light"));

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
    ImGui::SetNextWindowPos(ImVec2(_windowWidth * 0.75, 0));
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
    _gNormal->unbind();

    _gDepth.reset(new Texture2D(
        GL_DEPTH_COMPONENT, _windowWidth, _windowHeight, GL_DEPTH_COMPONENT, GL_FLOAT));
    _gDepth->bind();
    _gDepth->setParamterInt(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    _gDepth->setParamterInt(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    _gDepth->setParamterInt(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    _gDepth->setParamterInt(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    _gBufferFBO.reset(new Framebuffer);
    _gBufferFBO->bind();
    _gBufferFBO->drawBuffers({ GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 });
    _gBufferFBO->attachTexture2D(*_gPosition, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D);
    _gBufferFBO->attachTexture2D(*_gNormal, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D);
    _gBufferFBO->attachTexture2D(*_gAlbedo, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D);
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

    // TODO: modify ssao.frag
    _ssaoShader.reset(new GLSLProgram);
    _ssaoShader->attachVertexShaderFromFile(getAssetFullPath(quadVsRelPath));
    _ssaoShader->attachFragmentShaderFromFile(getAssetFullPath(ssaoFsRelPath));
    _ssaoShader->link();

    _ssaoBlurShader.reset(new GLSLProgram);
    _ssaoBlurShader->attachVertexShaderFromFile(getAssetFullPath(quadVsRelPath));
    _ssaoBlurShader->attachFragmentShaderFromFile(getAssetFullPath(ssaoBlurFsRelPath));
    _ssaoBlurShader->link();

    // TODO: modify ssao_lighting.frag
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

    _lightShader.reset(new GLSLProgram);
    _lightShader->attachVertexShaderFromFile(getAssetFullPath(lightVsRelPath));
    _lightShader->attachFragmentShaderFromFile(getAssetFullPath(lightFsRelPath));
    _lightShader->link();

    // TODO: modify extract_bright_color.frag
    _brightColorShader.reset(new GLSLProgram);
    _brightColorShader->attachVertexShaderFromFile(getAssetFullPath(quadVsRelPath));
    _brightColorShader->attachFragmentShaderFromFile(getAssetFullPath(brightColorFsRelPath));
    _brightColorShader->link();

    // TODO: modify gaussian_blur.frag
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
    
    constexpr float cameraMoveSpeed = 5.0f;
    constexpr float cameraRotateSpeed = 0.02f;

    float mouse_movement_in_x_direction = _input.mouse.move.xNow - _input.mouse.move.xOld;
    float mouse_movement_in_y_direction = _input.mouse.move.yNow - _input.mouse.move.yOld;
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

        if (_input.keyboard.keyStates[GLFW_KEY_R] != GLFW_RELEASE) {
            _camera->transform.position += cameraMoveSpeed * _deltaTime * _camera->transform.getUp();
        }

        if (_input.keyboard.keyStates[GLFW_KEY_E] != GLFW_RELEASE) {
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
    } else {
        ImGui::Text("No object selected");
    }

    ImGui::End();
}

static char objectNameBuffer[128] = "";

void Editor::renderScenePanel() {
    const auto flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings;
    
    if (!ImGui::Begin("Scene", nullptr, flags)) {
        ImGui::End();
        return;
    }

    if (ImGui::CollapsingHeader("Models")) {
        for (int i = 0; i < _models.size(); i++) {
            std::string label = _models[i]->name + "##" + std::to_string(i);
            if (ImGui::Selectable(label.c_str(), selectedObject == _models[i])) {
                selectedObject = _models[i];
            }
        }

        if (ImGui::Button("Add Model")) {
            ImGui::OpenPopup("Add Model");
            // 清空输入缓冲区
            memset(objectNameBuffer, 0, sizeof(objectNameBuffer));
            memcpy(objectNameBuffer, "New Model", sizeof("New Model"));
        }
    }

    if (ImGui::CollapsingHeader("Lights")) {
        if (ImGui::Selectable(_ambientLight->name.c_str(), selectedObject == _ambientLight.get())) {
            selectedObject = _ambientLight.get();
        }

        if (ImGui::Button("Add Light")) {
            ImGui::OpenPopup("Add Light");
            // 清空输入缓冲区
            memset(objectNameBuffer, 0, sizeof(objectNameBuffer));
        }
    }

    renderPopupModal();
    ImGui::End();
}

void Editor::renderPopupModal() {
    if (ImGui::BeginPopupModal("Add Model", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Enter Model Name:");
        ImGui::InputText("##ModelName", objectNameBuffer, sizeof(objectNameBuffer));

        static std::string selectedModel = ""; // 保存当前选中的模型名
        static std::vector<std::string> modelFiles = getModelFiles(); // 获取模型文件列表
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

        // 确认按钮
        if (ImGui::Button("OK", ImVec2(220, 0))) {
            if (strlen(objectNameBuffer) > 0 && !selectedModel.empty()) {
                _models.push_back(new Model(objectNameBuffer, getAssetFullPath("obj/" + selectedModel)));
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();

        // 取消按钮
        if (ImGui::Button("Cancel", ImVec2(220, 0))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    } else if (ImGui::BeginPopupModal("Add Model", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {

    }
}