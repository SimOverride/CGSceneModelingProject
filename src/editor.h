#pragma once

#include "base/application.h"
#include "base/camera.h"
#include "base/light.h"
#include "base/object.h"
#include "base/glsl_program.h"
#include "base/fullscreen_quad.h"
#include "base/framebuffer.h"
#include "base/texture2d.h"
#include "base/model.h"

class Editor : public Application {
public:
	Editor(const Options& options);

	~Editor();

	void handleInput() override;

	void renderFrame() override;
private:
	std::unique_ptr<PerspectiveCamera> _camera;

	std::vector<Model*> _models;

	std::unique_ptr<AmbientLight> _ambientLight;
	std::vector<Light*> _lights;

	Object* selectedObject = nullptr;

	std::unique_ptr<GLSLProgram> _drawScreenShader;
	std::unique_ptr<FullscreenQuad> _screenQuad;

	std::unique_ptr<Framebuffer> _gBufferFBO;
	std::unique_ptr<GLSLProgram> _gBufferShader;
	std::unique_ptr<Texture2D> _gPosition;
	std::unique_ptr<Texture2D> _gNormal;
	std::unique_ptr<Texture2D> _gAlbedo;
	std::unique_ptr<Texture2D> _gDepth;

	std::unique_ptr<Texture2D> _ssaoNoise;
	std::unique_ptr<Texture2D> _ssaoResult[2];
	std::unique_ptr<Framebuffer> _ssaoFBO;
	std::unique_ptr<Framebuffer> _ssaoBlurFBO;

	std::vector<glm::vec3> _sampleVecs;

	std::unique_ptr<GLSLProgram> _ssaoShader;
	std::unique_ptr<GLSLProgram> _ssaoBlurShader;
	std::unique_ptr<GLSLProgram> _ssaoLightingShader;

	std::unique_ptr<Framebuffer> _bloomFBO;
	std::unique_ptr<Framebuffer> _blurFBO;
	std::unique_ptr<Framebuffer> _brightColorFBO;

	std::unique_ptr<Texture2D> _bloomMap;
	std::unique_ptr<Texture2D> _brightColorMap[2];

	std::unique_ptr<GLSLProgram> _lightShader;
	std::unique_ptr<GLSLProgram> _brightColorShader;
	std::unique_ptr<GLSLProgram> _blurShader;
	std::unique_ptr<GLSLProgram> _blendShader;

	uint32_t _currentReadBuffer = 0;
	uint32_t _currentWriteBuffer = 1;

	std::vector<std::string> getModelFiles() const;

	void initGeometryPassResources();
	void initSSAOPassResources();
	void initBloomPassResources();
	void initShaders();

	void renderScene();

	void renderUI();
	void renderScenePanel();
	void renderInspectorPanel();
	void renderPopupModal();

	void extractBrightColor(const Texture2D& sceneMap);
	void blurBrightColor();
	void combineSceneMapAndBloomBlur(const Texture2D& sceneMap);
};