/*
 * glad_impl.c — the single translation unit that compiles GLAD 2.
 *
 * GLAD 2 splits its header into declarations and definitions. The
 * glad_gl* function pointers are declared everywhere (extern), but they
 * are DEFINED only in a translation unit that defines
 * GLAD_GL_IMPLEMENTATION before the include. Without this file the
 * linker reports "undefined reference to glad_glViewport" and dozens
 * more, one per entry point the project touches.
 *
 * Every other file in the project includes glad/gl.h WITHOUT that macro,
 * because those files only need the declarations.
 *
 * This file is C, not C++: the GLAD 2 implementation is written in C.
 * That is why CMakeLists.txt declares LANGUAGES C CXX.
 */

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
