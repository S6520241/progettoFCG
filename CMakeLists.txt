cmake_minimum_required(VERSION 3.16) # Aggiornato per supportare SFML 3
project(SpaceInvadersEsame LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

include(FetchContent)

FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.2
    GIT_SHALLOW ON
)
FetchContent_MakeAvailable(SFML)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.3
)
FetchContent_MakeAvailable(glm)

add_library(common INTERFACE)
target_compile_features(common INTERFACE cxx_std_20)

find_package(OpenGL REQUIRED)

# --- TAPPA 01 ---
add_executable(Tappa01 Cartella-Tappa01/main.cpp)

target_link_libraries(Tappa01 PRIVATE common SFML::Graphics SFML::Window SFML::System)

# --- TAPPA 02 ---
add_executable(Tappa02 Cartella-Tappa02/main.cpp)
target_link_libraries(Tappa02 PRIVATE common SFML::Graphics SFML::Window SFML::System)

# --- TAPPA 03 ---
add_executable(Tappa03 Cartella-Tappa03/main.cpp)
target_link_libraries(Tappa03 PRIVATE common SFML::Graphics SFML::Window SFML::System)