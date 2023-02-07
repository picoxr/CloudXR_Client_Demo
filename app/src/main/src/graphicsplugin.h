#pragma once
#include "PxrApi.h"
#include "util.h"

struct IGraphicsPlugin {
    virtual ~IGraphicsPlugin() = default;
    virtual void InitializeDevice() = 0;
};

// Create a opengles graphics plugin.
std::shared_ptr<IGraphicsPlugin> CreateGraphicsPlugin_OpenGLES();
