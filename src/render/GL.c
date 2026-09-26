#include <glad/gl.h>
#include <windows.h>

// Define all function pointers
#define DEFINE_GL_FUNC(name) __typeof__(name) name

// Core GL 1.0+
DEFINE_GL_FUNC(glClear);
DEFINE_GL_FUNC(glClearColor);
DEFINE_GL_FUNC(glClearDepth);
DEFINE_GL_FUNC(glDepthFunc);
DEFINE_GL_FUNC(glEnable);
DEFINE_GL_FUNC(glDisable);
DEFINE_GL_FUNC(glBlendFunc);
DEFINE_GL_FUNC(glViewport);
DEFINE_GL_FUNC(glDrawElements);
DEFINE_GL_FUNC(glGetString);
DEFINE_GL_FUNC(glGetError);
DEFINE_GL_FUNC(glGetIntegerv);
DEFINE_GL_FUNC(glGetFloatv);
DEFINE_GL_FUNC(glGetDoublev);
DEFINE_GL_FUNC(glGetBooleanv);
DEFINE_GL_FUNC(glTexImage2D);
DEFINE_GL_FUNC(glTexParameteri);
DEFINE_GL_FUNC(glTexParameterf);
DEFINE_GL_FUNC(glGenTextures);
DEFINE_GL_FUNC(glBindTexture);
DEFINE_GL_FUNC(glDeleteTextures);
DEFINE_GL_FUNC(glActiveTexture);
DEFINE_GL_FUNC(glGenFramebuffers);
DEFINE_GL_FUNC(glBindFramebuffer);
DEFINE_GL_FUNC(glFramebufferTexture2D);
DEFINE_GL_FUNC(glCheckFramebufferStatus);
DEFINE_GL_FUNC(glDeleteFramebuffers);
DEFINE_GL_FUNC(glGenRenderbuffers);
DEFINE_GL_FUNC(glBindRenderbuffer);
DEFINE_GL_FUNC(glRenderbufferStorage);
DEFINE_GL_FUNC(glDeleteRenderbuffers);
DEFINE_GL_FUNC(glReadPixels);
DEFINE_GL_FUNC(glScissor);
DEFINE_GL_FUNC(glPolygonMode);
DEFINE_GL_FUNC(glFrontFace);
DEFINE_GL_FUNC(glCullFace);
DEFINE_GL_FUNC(glDepthMask);
DEFINE_GL_FUNC(glStencilMask);
DEFINE_GL_FUNC(glStencilFunc);
DEFINE_GL_FUNC(glStencilOp);
DEFINE_GL_FUNC(glBlendFuncSeparate);
DEFINE_GL_FUNC(glBlendEquation);

// GL 1.2+ function pointers (loaded at runtime via wglGetProcAddress)
DEFINE_GL_FUNC(glGenVertexArrays);
DEFINE_GL_FUNC(glBindVertexArray);
DEFINE_GL_FUNC(glDeleteVertexArrays);
DEFINE_GL_FUNC(glGenBuffers);
DEFINE_GL_FUNC(glBindBuffer);
DEFINE_GL_FUNC(glBufferData);
DEFINE_GL_FUNC(glBufferSubData);
DEFINE_GL_FUNC(glDeleteBuffers);
DEFINE_GL_FUNC(glEnableVertexAttribArray);
DEFINE_GL_FUNC(glDisableVertexAttribArray);
DEFINE_GL_FUNC(glVertexAttribPointer);
DEFINE_GL_FUNC(glUseProgram);
DEFINE_GL_FUNC(glCreateProgram);
DEFINE_GL_FUNC(glDeleteProgram);
DEFINE_GL_FUNC(glCreateShader);
DEFINE_GL_FUNC(glShaderSource);
DEFINE_GL_FUNC(glCompileShader);
DEFINE_GL_FUNC(glGetShaderiv);
DEFINE_GL_FUNC(glGetShaderInfoLog);
DEFINE_GL_FUNC(glDeleteShader);
DEFINE_GL_FUNC(glAttachShader);
DEFINE_GL_FUNC(glLinkProgram);
DEFINE_GL_FUNC(glGetProgramiv);
DEFINE_GL_FUNC(glGetProgramInfoLog);
DEFINE_GL_FUNC(glUniform1i);
DEFINE_GL_FUNC(glUniform1f);
DEFINE_GL_FUNC(glUniform2f);
DEFINE_GL_FUNC(glUniform3f);
DEFINE_GL_FUNC(glUniform4f);
DEFINE_GL_FUNC(glUniformMatrix4fv);
DEFINE_GL_FUNC(glGetUniformLocation);
DEFINE_GL_FUNC(glActiveTexture);
DEFINE_GL_FUNC(glGenFramebuffers);
DEFINE_GL_FUNC(glBindFramebuffer);
DEFINE_GL_FUNC(glFramebufferTexture2D);
DEFINE_GL_FUNC(glFramebufferRenderbuffer);
DEFINE_GL_FUNC(glCheckFramebufferStatus);
DEFINE_GL_FUNC(glDeleteFramebuffers);
DEFINE_GL_FUNC(glGenRenderbuffers);
DEFINE_GL_FUNC(glBindRenderbuffer);
DEFINE_GL_FUNC(glRenderbufferStorage);
DEFINE_GL_FUNC(glDeleteRenderbuffers);
DEFINE_GL_FUNC(glBlendEquation);
DEFINE_GL_FUNC(glBlendFuncSeparate);
DEFINE_GL_FUNC(glFramebufferRenderbuffer);
DEFINE_GL_FUNC(glGetStringi);
DEFINE_GL_FUNC(glBindSampler);
DEFINE_GL_FUNC(glGetVertexAttribiv);
DEFINE_GL_FUNC(glGetVertexAttribPointerv);
DEFINE_GL_FUNC(glDrawElementsBaseVertex);

int gladLoadGL(void* (*load)(const char*)) {
    #define GL_LOAD(name) do { \
        name = (void*)load(#name); \
        if (!name) name = (void*)GetProcAddress(GetModuleHandleA("opengl32"), #name); \
    } while(0)

    // Core GL 1.0+
    GL_LOAD(glClear);
    GL_LOAD(glClearColor);
    GL_LOAD(glClearDepth);
    GL_LOAD(glDepthFunc);
    GL_LOAD(glEnable);
    GL_LOAD(glDisable);
    GL_LOAD(glBlendFunc);
    GL_LOAD(glViewport);
    GL_LOAD(glDrawElements);
    GL_LOAD(glGetString);
    GL_LOAD(glGetError);
    GL_LOAD(glGetIntegerv);
    GL_LOAD(glGetFloatv);
    GL_LOAD(glGetDoublev);
    GL_LOAD(glGetBooleanv);
    GL_LOAD(glTexImage2D);
    GL_LOAD(glTexParameteri);
    GL_LOAD(glTexParameterf);
    GL_LOAD(glGenTextures);
    GL_LOAD(glBindTexture);
    GL_LOAD(glDeleteTextures);
    GL_LOAD(glActiveTexture);
    GL_LOAD(glGenFramebuffers);
    GL_LOAD(glBindFramebuffer);
    GL_LOAD(glFramebufferTexture2D);
    GL_LOAD(glCheckFramebufferStatus);
    GL_LOAD(glDeleteFramebuffers);
    GL_LOAD(glGenRenderbuffers);
    GL_LOAD(glBindRenderbuffer);
    GL_LOAD(glRenderbufferStorage);
    GL_LOAD(glDeleteRenderbuffers);
    GL_LOAD(glReadPixels);
    GL_LOAD(glScissor);
    GL_LOAD(glPolygonMode);
    GL_LOAD(glFrontFace);
    GL_LOAD(glCullFace);
    GL_LOAD(glDepthMask);
    GL_LOAD(glStencilMask);
    GL_LOAD(glStencilFunc);
    GL_LOAD(glStencilOp);
    GL_LOAD(glBlendFuncSeparate);
    GL_LOAD(glBlendEquation);

    // GL 1.2+ extensions
    GL_LOAD(glGenVertexArrays);
    GL_LOAD(glBindVertexArray);
    GL_LOAD(glDeleteVertexArrays);
    GL_LOAD(glGenBuffers);
    GL_LOAD(glBindBuffer);
    GL_LOAD(glBufferData);
    GL_LOAD(glBufferSubData);
    GL_LOAD(glDeleteBuffers);
    GL_LOAD(glEnableVertexAttribArray);
    GL_LOAD(glDisableVertexAttribArray);
    GL_LOAD(glVertexAttribPointer);
    GL_LOAD(glUseProgram);
    GL_LOAD(glCreateProgram);
    GL_LOAD(glDeleteProgram);
    GL_LOAD(glCreateShader);
    GL_LOAD(glShaderSource);
    GL_LOAD(glCompileShader);
    GL_LOAD(glGetShaderiv);
    GL_LOAD(glGetShaderInfoLog);
    GL_LOAD(glDeleteShader);
    GL_LOAD(glAttachShader);
    GL_LOAD(glLinkProgram);
    GL_LOAD(glGetProgramiv);
    GL_LOAD(glGetProgramInfoLog);
    GL_LOAD(glUniform1i);
    GL_LOAD(glUniform1f);
    GL_LOAD(glUniform2f);
    GL_LOAD(glUniform3f);
    GL_LOAD(glUniform4f);
    GL_LOAD(glUniformMatrix4fv);
    GL_LOAD(glGetUniformLocation);
    GL_LOAD(glActiveTexture);
    GL_LOAD(glGenFramebuffers);
    GL_LOAD(glBindFramebuffer);
    GL_LOAD(glFramebufferTexture2D);
    GL_LOAD(glFramebufferRenderbuffer);
    GL_LOAD(glCheckFramebufferStatus);
    GL_LOAD(glDeleteFramebuffers);
    GL_LOAD(glGenRenderbuffers);
    GL_LOAD(glBindRenderbuffer);
    GL_LOAD(glRenderbufferStorage);
    GL_LOAD(glDeleteRenderbuffers);
    GL_LOAD(glBlendEquation);
    GL_LOAD(glBlendFuncSeparate);
    GL_LOAD(glFramebufferRenderbuffer);
    GL_LOAD(glGetStringi);
    GL_LOAD(glBindSampler);
    GL_LOAD(glGetVertexAttribiv);
    GL_LOAD(glGetVertexAttribPointerv);
    GL_LOAD(glDrawElementsBaseVertex);

    #undef GL_LOAD
    return 1;
}
