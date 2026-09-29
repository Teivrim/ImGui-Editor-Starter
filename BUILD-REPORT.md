# Build report — ImGui Editor Starter Kit

- **Date:** 2026-09-29
- **Result:** builds
- **Time:** configure + build, single pass
- **Toolchain:** CMake 3.30, Ninja, MinGW-w64 g++ 14.2, Windows 11
- **Artefact:** `build/EditorCore.exe`, 2 508 471 bytes
- **Network:** not required

## What had to be fixed to get here

The project never compiled, and configure failed before the compiler
ever ran. Three separate causes:

1. **GLAD could not be fetched.** `FetchContent` pulled `Dav1dde/glad`
   at tag `v2.0.8`, which contains no `src/glad.c`. Configure died with
   `Cannot find source file`. A 4 KB stub in `third_party/glad_loader`
   had no implementation at all.
   Now vendored whole as `third_party/glad/gl.h` (GLAD 2, MIT, 326 KB,
   header-only), compiled in exactly one TU: `src/render/glad_impl.c`
   with `GLAD_GL_IMPLEMENTATION` defined.

2. **Vendored dependencies were ignored.** GLFW, Dear ImGui and stb were
   committed to `third_party` (61 MB) while CMakeLists fetched all three
   from the network. The repository carried copies the build refused to
   use. All `FetchContent` removed; the build is now fully offline.

3. **The GLAD call used a GLAD 1 cast.** `gladLoadGL((void* (*)(const
   char*))glfwGetProcAddress)` does not convert to `GLADloadfunc` in
   GLAD 2. Replaced with the correct cast.

## Regression guard

`verify_no_glew` in CI fails the build if GLEW or the glad FetchContent
comes back, or if the vendored `third_party/glad/gl.h` disappears.
Those were the two ways this broke, and both are silent until a clean
machine tries to build.
