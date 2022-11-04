#include "graphicsplugin.h"
#include "GLUtils.h"
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES3/gl3.h>
#include <GLES3/gl32.h>
#include "util.h"

#define VERTEX_POS_INDX  0
#define TEXTURE_POS_INDX 1

namespace {
    constexpr float DarkSlateGray[] = {0.184313729f, 0.309803933f, 0.309803933f, 1.0f};

    struct OpenGLESGraphicsPlugin : public IGraphicsPlugin {
        OpenGLESGraphicsPlugin() = default;

        OpenGLESGraphicsPlugin(const OpenGLESGraphicsPlugin &) = delete;

        OpenGLESGraphicsPlugin &operator=(const OpenGLESGraphicsPlugin &) = delete;

        OpenGLESGraphicsPlugin(OpenGLESGraphicsPlugin &&) = delete;

        OpenGLESGraphicsPlugin &operator=(OpenGLESGraphicsPlugin &&) = delete;

        ~OpenGLESGraphicsPlugin() override {
            if (mFboProgram) {
                glDeleteProgram(mFboProgram);
            }

            if (mVboIds[0]) {
                glDeleteBuffers(3, mVboIds);
            }

            if (mVaoId) {
                glDeleteVertexArrays(1, &mVaoId);
            }

            if (mFboId) {
                glDeleteFramebuffers(1, &mFboId);
            }

            if (mFboProgram) {
                GLUtils::DeleteProgram(mFboProgram);
            }
        }

        void InitShader() {
            GLfloat vVertices[] = {
                    -1.0f, -1.0f, 0.0f,
                    1.0f, -1.0f, 0.0f,
                    -1.0f, 1.0f, 0.0f,
                    1.0f, 1.0f, 0.0f,
            };

            GLfloat vTexCoors[] = {
                    0.0f, 1.0f,
                    1.0f, 1.0f,
                    0.0f, 0.0f,
                    1.0f, 0.0f,
            };

            GLushort indices[] = {0, 1, 2, 1, 3, 2};

            char vShaderStr[] =
                    "#version 300 es                            \n"
                    "layout(location = 0) in vec4 a_position;   \n"
                    "layout(location = 1) in vec2 a_texCoord;   \n"
                    "out vec2 v_texCoord;                       \n"
                    "void main()                                \n"
                    "{                                          \n"
                    "   gl_Position = a_position;               \n"
                    "   v_texCoord = a_texCoord;                \n"
                    "}                                          \n";

            char fFboShaderStr[] =
                    "#version 300 es\n"
                    "#extension GL_OES_EGL_image_external_essl3 : require\n"
                    "precision mediump float;\n"
                    "in vec2 v_texCoord;\n"
                    "layout(location = 0) out vec4 outColor;\n"
                    "uniform samplerExternalOES s_TextureMap;\n"
                    "void main()\n"
                    "{\n"
                    "    outColor = texture(s_TextureMap, v_texCoord);\n"
                    "}";

            mFboProgram = GLUtils::CreateProgram(vShaderStr, fFboShaderStr, mVertexShader, mFragmentShader);

            if (mFboProgram == GL_NONE) {
                LOGE("InitShader mProgram == GL_NONE");
                return;
            }

            glGenBuffers(3, mVboIds);
            glBindBuffer(GL_ARRAY_BUFFER, mVboIds[0]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vVertices), vVertices, GL_STATIC_DRAW);

            glBindBuffer(GL_ARRAY_BUFFER, mVboIds[1]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vTexCoors), vTexCoors, GL_STATIC_DRAW);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mVboIds[2]);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

            glGenVertexArrays(1, &mVaoId);
            glBindVertexArray(mVaoId);

            glBindBuffer(GL_ARRAY_BUFFER, mVboIds[0]);
            glEnableVertexAttribArray(VERTEX_POS_INDX);
            glVertexAttribPointer(VERTEX_POS_INDX, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (const void *) 0);
            glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);

            glBindBuffer(GL_ARRAY_BUFFER, mVboIds[1]);
            glEnableVertexAttribArray(TEXTURE_POS_INDX);
            glVertexAttribPointer(TEXTURE_POS_INDX, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (const void *) 0);
            glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mVboIds[2]);
            glBindVertexArray(GL_NONE);

            glGenFramebuffers(1, &mFboId);
        }

        void InitializeDevice() override {
            InitContext();
            InitShader();
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

            config = 0;
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

        void RenderView(bool frameValid, int width, int height, uint64_t sdk2DTexture, uint64_t oesTexture) override {

            glViewport(0, 0, width, height);
            glClearColor(DarkSlateGray[0], DarkSlateGray[1], DarkSlateGray[2], DarkSlateGray[3]);

            glBindFramebuffer(GL_FRAMEBUFFER, mFboId);
            glBindTexture(GL_TEXTURE_2D, sdk2DTexture);
            GO_CHECK_GL_ERROR();
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, sdk2DTexture, 0);
            GO_CHECK_GL_ERROR();
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
                LOGE("glCheckFramebufferStatus status != GL_FRAMEBUFFER_COMPLETE");
                return;
            }
            glUseProgram(mFboProgram);
            glBindVertexArray(mVaoId);

            if (frameValid) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_EXTERNAL_OES, oesTexture);
                GO_CHECK_GL_ERROR();
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (const void *) 0);
                GO_CHECK_GL_ERROR();
            } else {
                //glClear(GL_COLOR_BUFFER_BIT);
            }

            glBindVertexArray(0);
            glBindTexture(GL_TEXTURE_EXTERNAL_OES, 0);
            glBindTexture(GL_TEXTURE_2D, 0);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

    private:
        GLuint mVaoId = 0;
        GLuint mVboIds[3] = {0};

        GLuint mFboId = GL_NONE;
        GLuint mFboProgram = GL_NONE;
        GLuint mVertexShader = GL_NONE;
        GLuint mFragmentShader = GL_NONE;
    };
}  // namespace

std::shared_ptr<IGraphicsPlugin> CreateGraphicsPlugin_OpenGLES() {
    return std::make_shared<OpenGLESGraphicsPlugin>();
}

