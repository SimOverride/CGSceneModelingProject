#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "base/model.h"
#include "editor.h"

const std::string modelRelPath = "obj/bunny.obj";

Editor::Editor(const Options& options) : Application(options) {
    
}

Editor::~Editor() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void Editor::handleInput() {
    
}

void Editor::renderFrame() {
    // showFpsInWindowTitle();

    
    
}
