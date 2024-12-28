#include <imgui.h>

#include "object.h"

Object::Object(const std::string & name) : name(name) {

}

void Object::renderInspector() {
	ImGui::Text("%s", name.c_str());
	
	ImGui::DragFloat3("Position", &transform.position[0], 0.1f);
	
	// �༭rotation
	glm::vec3 eulerAngles = glm::degrees(glm::eulerAngles(transform.rotation));
	if (ImGui::DragFloat3("Rotation", &eulerAngles[0], 1.0f)) {
		transform.rotation = glm::quat(glm::radians(eulerAngles));  // ���޸ĵ�ŷ����ת��Ϊ��Ԫ��
	}

	ImGui::DragFloat3("Scale", &transform.scale[0], 0.01f, 0.0f, 10.0f);
}