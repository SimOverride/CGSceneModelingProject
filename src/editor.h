#pragma once

#include "base/application.h"
#include "base/camera.h"

class Editor : public Application {
public:
	Editor(const Options& options);

	~Editor();

	void handleInput() override;

	void renderFrame() override;
private:
	std::unique_ptr<PerspectiveCamera> _camera;

};