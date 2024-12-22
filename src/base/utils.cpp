#include "utils.h"
#include <cfloat>

void saveFramebufferToImage(Framebuffer* framebuffer, int width, int height, const std::string& colorFile, const std::string& depthFile) {
    framebuffer->bind();

    // ��ȡ��ɫ����
    std::vector<unsigned char> colorData(width * height * 4); // RGBA
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, colorData.data());

    // ��ȡ��Ȼ���
    std::vector<float> depthData(width * height);
    glReadPixels(0, 0, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, depthData.data());

    // ������ɫͼ��
    stbi_write_png(colorFile.c_str(), width, height, 4, colorData.data(), width * 4);

    // �������������ͼ��
    std::vector<unsigned char> depthImage(width * height);
    for (int i = 0; i < width * height; ++i) {
        depthImage[i] = static_cast<unsigned char>(depthData[i] * 255); // ��һ����0-255
    }
    stbi_write_png(depthFile.c_str(), width, height, 1, depthImage.data(), width); // ��ͨ�����ͼ

    framebuffer->unbind();
}

void saveRGBTextureToImage(Texture2D* texture, int width, int height, const std::string& file) {
    texture->bind();
    std::vector<float> imageData(width * height * 4);
    // ��ȡ��Ȼ�������
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, imageData.data());
    stbi_write_png(file.c_str(), width, height, 4, imageData.data(), width * 4);
    texture->unbind();
}

void saveDepthTextureToImage(Texture2D* texture, int width, int height, const std::string& file) {
    texture->bind();
    std::vector<unsigned char> depthData(width * height);
    // ��ȡ��Ȼ�������
    glGetTexImage(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, GL_FLOAT, depthData.data());
    // ����������ݵ���Сֵ�����ֵ
    float minDepth = FLT_MAX;
    float maxDepth = -FLT_MAX;
    for (int i = 0; i < width * height; ++i) {
        float depth = depthData[i];
        if (depth < minDepth) minDepth = depth;
        if (depth > maxDepth) maxDepth = depth;
    }
    std::vector<unsigned char> imageData(width * height);
    for (int i = 0; i < width * height; ++i) {
        float depth = depthData[i];
        if (maxDepth != minDepth) {
            depth = (depth - minDepth) / (maxDepth - minDepth);
        }
        else {
            depth = 0.5f; // �����Сֵ�������ֵ������Ϊ�м�ֵ
        }
        imageData[i] = static_cast<unsigned char>(depth * 255.0f);
    }
    stbi_write_png(file.c_str(), width, height, 1, imageData.data(), width);
    texture->unbind();
}