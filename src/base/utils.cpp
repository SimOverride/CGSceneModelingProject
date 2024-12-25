#include "utils.h"
#include <cfloat>

void saveFramebufferToImage(Framebuffer* framebuffer, int width, int height, const std::string& colorFile, const std::string& depthFile) {
    framebuffer->bind();

    // 读取颜色缓冲
    std::vector<unsigned char> colorData(width * height * 4); // RGBA
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, colorData.data());

    // 读取深度缓冲
    std::vector<float> depthData(width * height);
    glReadPixels(0, 0, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, depthData.data());

    // 保存颜色图像
    stbi_write_png(colorFile.c_str(), width, height, 4, colorData.data(), width * 4);

    // 处理并保存深度图像
    std::vector<unsigned char> depthImage(width * height);
    for (int i = 0; i < width * height; ++i) {
        depthImage[i] = static_cast<unsigned char>(depthData[i] * 255); // 归一化到0-255
    }
    stbi_write_png(depthFile.c_str(), width, height, 1, depthImage.data(), width); // 单通道深度图

    framebuffer->unbind();
}

void saveRGBTextureToImage(Texture2D* texture, int width, int height, const std::string& file) {
    texture->bind();
    std::vector<float> imageData(width * height * 4);
    // 读取深度缓冲数据
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, imageData.data());
    stbi_write_png(file.c_str(), width, height, 4, imageData.data(), width * 4);
    texture->unbind();
}

void saveDepthTextureToImage(Texture2D* texture, int width, int height, const std::string& file) {
    texture->bind();
    std::vector<unsigned char> depthData(width * height);
    // 读取深度缓冲数据
    glGetTexImage(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, GL_FLOAT, depthData.data());
    // 查找深度数据的最小值和最大值
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
            depth = 0.5f; // 如果最小值等于最大值，设置为中间值
        }
        imageData[i] = static_cast<unsigned char>(depth * 255.0f);
    }
    stbi_write_png(file.c_str(), width, height, 1, imageData.data(), width);
    texture->unbind();
}