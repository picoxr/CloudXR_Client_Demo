#include "graphicsplugin.h"
#include "GLUtils.h"
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES3/gl3.h>
#include <GLES3/gl32.h>
#include "util.h"

namespace {

    struct OpenGLESGraphicsPlugin : public IGraphicsPlugin {
        OpenGLESGraphicsPlugin() = default;
        OpenGLESGraphicsPlugin(const OpenGLESGraphicsPlugin &) = delete;
        OpenGLESGraphicsPlugin &operator=(const OpenGLESGraphicsPlugin &) = delete;
        OpenGLESGraphicsPlugin(OpenGLESGraphicsPlugin &&) = delete;
        OpenGLESGraphicsPlugin &operator=(OpenGLESGraphicsPlugin &&) = delete;
        ~OpenGLESGraphicsPlugin() override {}

        void InitializeDevice() override {
            InitContext();
        }

        bool InitContext() {
            EGLint numConfigs;
            EGLConfig config = nullptr;
            EGLContext context;
            EGLSurface tinySurface;
            EGLSurface mainSurface;
            EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
            eglInitialize(display, nullptr, nullptr);
            const int MAX_CONFIGS = 1024;
            EGLConfig configs[MAX_CONFIGS];
            eglGetConfigs(display, configs, MAX_CONFIGS, &numConfigs);
            const EGLint configAttribs[] = {EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8,
                                            EGL_ALPHA_SIZE, 8, EGL_DEPTH_SIZE, 24,
                                            EGL_SAMPLE_BUFFERS, 0, EGL_SAMPLES,
                                            0, EGL_NONE};

            for (int i = 0; i < numConfigs; i++) {
                EGLint value = 0;
                eglGetConfigAttrib(display, configs[i], EGL_RENDERABLE_TYPE, &value);
                if ((value & EGL_OPENGL_ES3_BIT) != EGL_OPENGL_ES3_BIT) {
                    continue;
                }

                // Without EGL_KHR_surfaceless_context, the config needs to support both pbuffers and window surfaces.
                eglGetConfigAttrib(display, configs[i], EGL_SURFACE_TYPE, &value);
                if ((value & (EGL_WINDOW_BIT | EGL_PBUFFER_BIT)) != (EGL_WINDOW_BIT | EGL_PBUFFER_BIT)) {
                    continue;
                }

                int j = 0;
                for (; configAttribs[j] != EGL_NONE; j += 2) {
                    eglGetConfigAttrib(display, configs[i], configAttribs[j], &value);
                    if (value != configAttribs[j + 1]) {
                        break;
                    }
                }
                if (configAttribs[j] == EGL_NONE) {
                    config = configs[i];
                    break;
                }
            }
            if (config == 0) {
                LOGE("Failed to find EGLConfig");
                return false;
            }
            EGLint contextAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE, EGL_NONE, EGL_NONE};
            context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs);
            if (context == EGL_NO_CONTEXT) {
                LOGE("eglCreateContext() failed");
                return false;
            }
            const EGLint surfaceAttribs[] = {EGL_WIDTH, 16, EGL_HEIGHT, 16, EGL_NONE};
            tinySurface = eglCreatePbufferSurface(display, config, surfaceAttribs);
            if (tinySurface == EGL_NO_SURFACE) {
                LOGE("eglCreatePbufferSurface() failed");
                eglDestroyContext(display, context);
                context = EGL_NO_CONTEXT;
                return false;
            }
            mainSurface = tinySurface;
            eglMakeCurrent(display, mainSurface, mainSurface, context);
            return true;
        }
    };
}  // namespace

std::shared_ptr<IGraphicsPlugin> CreateGraphicsPlugin_OpenGLES() {
    return std::make_shared<OpenGLESGraphicsPlugin>();
}

