#pragma once

#include "framebuffer.h"
#include "texture2d.h"

void saveFramebufferToImage(Framebuffer* framebuffer, int width, int height, const std::string& colorFile, const std::string& depthFile);
void saveDepthTextureToImage(Texture2D* texture, int width, int height, const std::string& file);
void saveDepthTextureToImage(Texture2D* texture, int width, int height, const std::string& file);