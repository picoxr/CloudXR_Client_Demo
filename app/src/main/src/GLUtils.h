#ifndef GL_UTILS_H_
#define GL_UTILS_H_

#include <GLES3/gl3.h>
#include <string>

#define SHADER_TO_STRING(s) #s

#define MATH_PI 3.1415926535897932384626433832802

class GLUtils {
public:
    static GLuint LoadShader(GLenum shaderType, const char *pSource);

    static GLuint CreateProgram(const char *pVertexShaderSource, const char *pFragShaderSource,
                                GLuint &vertexShaderHandle,
                                GLuint &fragShaderHandle);

    static void DeleteProgram(GLuint &program);

    static void CheckGLError(const char *pGLOperation);

};

#endif