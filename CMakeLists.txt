cmake_minimum_required(VERSION 3.16)
project(SHOViewer CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ── Fetch third-party UI libraries from GitHub ────────────────────────────────
include(FetchContent)

FetchContent_Declare(imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.91.8
    GIT_SHALLOW    TRUE
)

FetchContent_Declare(imguizmo
    GIT_REPOSITORY https://github.com/CedricGuillemet/ImGuizmo.git
    GIT_TAG        master
    GIT_SHALLOW    TRUE
)

# Populate without calling add_subdirectory (neither lib ships a usable CMakeLists)
FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
    FetchContent_Populate(imgui)
endif()

FetchContent_GetProperties(imguizmo)
if(NOT imguizmo_POPULATED)
    FetchContent_Populate(imguizmo)
endif()

# ── System dependencies ───────────────────────────────────────────────────────
find_package(OpenGL REQUIRED)
find_package(GLEW   REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)

find_package(glm QUIET)

# ── Include paths ─────────────────────────────────────────────────────────────
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/vendor          # stb, json, tiny_gltf
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${imguizmo_SOURCE_DIR}
    ${SDL2_INCLUDE_DIRS}
)

# ── Application sources ───────────────────────────────────────────────────────
set(APP_SOURCES
    src/main.cpp
    src/Common.cpp
    src/PS2Texture.cpp
    src/Loader.cpp
    src/UI.cpp
)

# ── ImGui sources (fetched) ───────────────────────────────────────────────────
set(IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl2.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

# ── ImGuizmo sources (fetched) ────────────────────────────────────────────────
set(IMGUIZMO_SOURCES
    ${imguizmo_SOURCE_DIR}/ImGuizmo.cpp
)

# ── Target ────────────────────────────────────────────────────────────────────
add_executable(SHOViewer
    ${APP_SOURCES}
    ${IMGUI_SOURCES}
    ${IMGUIZMO_SOURCES}
)

target_compile_options(SHOViewer PRIVATE -Wall)

target_link_libraries(SHOViewer
    OpenGL::GL
    GLEW::GLEW
    ${SDL2_LIBRARIES}
)

target_compile_options(SHOViewer PRIVATE ${SDL2_CFLAGS_OTHER})

# ── Install (optional) ────────────────────────────────────────────────────────
install(TARGETS SHOViewer DESTINATION bin)
