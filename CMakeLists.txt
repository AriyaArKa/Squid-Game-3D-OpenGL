cmake_minimum_required(VERSION 3.20)

project(squid_game_opengl VERSION 1.0.0 LANGUAGES C CXX)

if(NOT WIN32)
    message(FATAL_ERROR "The current audio backend uses Windows MCI. Windows is required.")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenGL REQUIRED)

find_package(glfw3 CONFIG QUIET)
if(TARGET glfw)
    set(SQUID_GAME_GLFW_TARGET glfw)
elseif(TARGET glfw3::glfw)
    set(SQUID_GAME_GLFW_TARGET glfw3::glfw)
else()
    find_path(GLFW_INCLUDE_DIR GLFW/glfw3.h REQUIRED)
    find_library(GLFW_LIBRARY NAMES glfw3 glfw REQUIRED)
    add_library(squid_game_glfw UNKNOWN IMPORTED)
    set_target_properties(squid_game_glfw PROPERTIES
        IMPORTED_LOCATION "${GLFW_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${GLFW_INCLUDE_DIR}"
    )
    set(SQUID_GAME_GLFW_TARGET squid_game_glfw)
endif()

find_package(glm CONFIG QUIET)
if(TARGET glm::glm)
    set(SQUID_GAME_GLM_TARGET glm::glm)
else()
    find_path(GLM_INCLUDE_DIR glm/glm.hpp REQUIRED)
    add_library(squid_game_glm INTERFACE)
    target_include_directories(squid_game_glm INTERFACE "${GLM_INCLUDE_DIR}")
    set(SQUID_GAME_GLM_TARGET squid_game_glm)
endif()

set(SQUID_GAME_SOURCES
    src/main_modern.cpp
    src/audio/Audio.cpp
    src/camera/Camera.cpp
    src/contestants/Contestants.cpp
    src/doll/Doll.cpp
    src/doll/DollBody.cpp
    src/doll/DollHead.cpp
    src/doll/DollTree.cpp
    src/engine/Camera.cpp
    src/engine/EBO.cpp
    src/engine/GLCompat.cpp
    src/engine/Mesh.cpp
    src/engine/Primitives.cpp
    src/engine/Shader.cpp
    src/engine/Texture.cpp
    src/engine/VAO.cpp
    src/engine/VBO.cpp
    src/environment/Environment.cpp
    src/environment/EnvironmentClock.cpp
    src/environment/EnvironmentHouses.cpp
    src/glassbridge/GlassBridge.cpp
    src/glassbridge/GlassBridgeCamera.cpp
    src/glassbridge/GlassBridgeContestants.cpp
    src/glassbridge/GlassBridgeEnvironment.cpp
    src/glassbridge/GlassBridgeShards.cpp
    src/glassbridge/GlassBridgeSteps.cpp
    src/guards/Guards.cpp
    src/render/LightFixtures.cpp
    src/render/Renderer.cpp
    src/render/SceneTextures.cpp
    src/tools/Showcase.cpp
    src/vendor/glad/src/gl.c
    src/vendor/imgui/imgui.cpp
    src/vendor/imgui/imgui_draw.cpp
    src/vendor/imgui/imgui_tables.cpp
    src/vendor/imgui/imgui_widgets.cpp
    src/vendor/imgui/backends/imgui_impl_glfw.cpp
    src/vendor/imgui/backends/imgui_impl_opengl3.cpp
)

add_executable(squid_game_opengl ${SQUID_GAME_SOURCES})
target_compile_definitions(squid_game_opengl PRIVATE MODERN_OPENGL)
target_include_directories(squid_game_opengl PRIVATE
    src
    src/compat
    src/vendor/glad/include
    src/vendor/imgui
    src/vendor/imgui/backends
)
target_link_libraries(squid_game_opengl PRIVATE
    OpenGL::GL
    ${SQUID_GAME_GLFW_TARGET}
    ${SQUID_GAME_GLM_TARGET}
    winmm
    gdi32
)

set_target_properties(squid_game_opengl PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_custom_command(TARGET squid_game_opengl POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/assets"
            "$<TARGET_FILE_DIR:squid_game_opengl>/assets"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/src/shaders"
            "$<TARGET_FILE_DIR:squid_game_opengl>/src/shaders"
    COMMENT "Copying runtime assets"
)
