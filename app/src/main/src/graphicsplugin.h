#pragma once
#include "PxrApi.h"
#include "util.h"

struct IGraphicsPlugin {
    virtual ~IGraphicsPlugin() = default;

    virtual void InitializeDevice() = 0;

    virtual void RenderView(int width, int height, uint64_t sdk2DTexture, uint64_t oesTexture) = 0;

};

// Create a opengles graphics plugin.
std::shared_ptr<IGraphicsPlugin> CreateGraphicsPlugin_OpenGLES();
