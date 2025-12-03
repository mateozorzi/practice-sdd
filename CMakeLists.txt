cmake_minimum_required(VERSION 3.24)

if (POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

if (NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 20)
endif()

set(CMAKE_CXX_EXTENSIONS ON)

project(nfs_tp
    VERSION 1.0
    HOMEPAGE_URL "https://github.com/URL-HERE"
    LANGUAGES CXX
    )

option(NFS_TESTS "Enable / disable tests." ON)
option(NFS_CLIENT "Enable / disable client program." ON)
option(NFS_SERVER "Enable / disable server program." ON)
option(NFS_EDITOR "Enable / disable editor program." ON)
option(NFS_MAKE_WARNINGS_AS_ERRORS "Enable / disable warnings as errors." ON)

message(CMAKE_CXX_COMPILER_ID="${CMAKE_CXX_COMPILER_ID}")

# Define target: lib common between client, server and editor programs
add_library(nfs_common STATIC)

# add the source files that make the lib
add_subdirectory(common/)


# set compiler warnings to the targets
include(cmake/CompilerWarnings.cmake)
set_project_warnings(nfs_common ${NFS_MAKE_WARNINGS_AS_ERRORS} FALSE)

# Say where the headers will be found required by library target
# Make this PUBLIC so any dependant linking lib will inherit this
# location (so our dependants will not have to explicitly request this)
target_include_directories(nfs_common PUBLIC .)

# Enable SDL2pp with Image, TTF and Mixer support
set(SDL2PP_WITH_IMAGE YES)
set(SDL2PP_WITH_TTF YES)
set(SDL2PP_WITH_MIXER YES)

# Install libSDL2 and libSDL2pp (for client or editor)
if(NFS_CLIENT OR NFS_EDITOR)
    include(FetchContent)
    FetchContent_Declare(
        SDL2
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG release-2.30.8
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
        OVERRIDE_FIND_PACKAGE
    )

    FetchContent_Declare(
        SDL2_image
        GIT_REPOSITORY https://github.com/libsdl-org/SDL_image.git
        GIT_TAG release-2.8.2
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
        OVERRIDE_FIND_PACKAGE
    )

    FetchContent_Declare(
        SDL2_mixer
        GIT_REPOSITORY https://github.com/libsdl-org/SDL_mixer.git
        GIT_TAG release-2.8.0
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
        OVERRIDE_FIND_PACKAGE
    )

    FetchContent_Declare(
        SDL2_ttf
        GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
        GIT_TAG release-2.22.0
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
        OVERRIDE_FIND_PACKAGE
    )

    FetchContent_Declare(
        libSDL2pp
        OVERRIDE_FIND_PACKAGE
        URL https://github.com/libSDL2pp/libSDL2pp/archive/cc198c9a5657048bee67ece82de620b2d5661084.zip
    )

    FetchContent_MakeAvailable(SDL2 SDL2_image SDL2_mixer SDL2_ttf libSDL2pp)
endif()

# Install yaml-cpp for server configuration
if(NFS_SERVER)
    include(FetchContent)

    FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.3
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
    )

    FetchContent_Declare(
        yaml-cpp
        GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
        GIT_TAG 0.8.0
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
    )
    set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "Disable yaml-cpp tests")
    set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "Disable yaml-cpp tools")
    set(YAML_CPP_BUILD_CONTRIB OFF CACHE BOOL "Disable yaml-cpp contrib")
    FetchContent_MakeAvailable()

    FetchContent_Declare(
        box2d
        GIT_REPOSITORY https://github.com/erincatto/box2d.git
        GIT_TAG v2.4.1
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
    )
    set(BOX2D_BUILD_UNIT_TESTS OFF CACHE BOOL "Disable Box2D unit tests")

    FetchContent_MakeAvailable(nlohmann_json yaml-cpp box2d)
endif()

# Program section
# ---------------
if(NFS_CLIENT)
    set(CMAKE_AUTOUIC ON)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    find_package(Qt6 COMPONENTS Core REQUIRED)
    find_package(Qt6 COMPONENTS Widgets REQUIRED)
    find_package(Qt6 COMPONENTS Core Widgets Multimedia REQUIRED)

    add_executable(nfs_client)

    # Make them depend on nfs_common lib
    add_dependencies(nfs_client nfs_common SDL2pp::SDL2pp)

    # Add source files and enable warnings
    add_subdirectory(client Lobby)

    set_project_warnings(nfs_client ${NFS_MAKE_WARNINGS_AS_ERRORS} FALSE)

    # add libSDL2pp's headers
    target_include_directories(nfs_client PRIVATE ${libSDL2pp_SOURCE_DIR})

    # Define DATA_PATH for the client
    target_compile_definitions(nfs_client PRIVATE DATA_PATH="${PROJECT_SOURCE_DIR}/client/cars")

    # Link the nfs_common lib target
    target_link_libraries(nfs_client nfs_common SDL2pp::SDL2pp SDL2_image::SDL2_image SDL2_ttf::SDL2_ttf SDL2_mixer::SDL2_mixer nlohmann_json::nlohmann_json Qt6::Widgets Qt6::Core Qt6::Multimedia)
endif()

if(NFS_SERVER)
    add_executable(nfs_server)

    # Make them depend on nfs_common lib
    add_dependencies(nfs_server nfs_common)

    # Add source files and enable warnings
    add_subdirectory(server)

    set_project_warnings(nfs_server ${NFS_MAKE_WARNINGS_AS_ERRORS} FALSE)

    # Link the nfs_common lib target
    target_link_libraries(nfs_server nfs_common nlohmann_json::nlohmann_json yaml-cpp::yaml-cpp box2d)
endif()

if(NFS_EDITOR)
    set(CMAKE_AUTOUIC ON)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    find_package(Qt6 COMPONENTS Core REQUIRED)
    find_package(Qt6 COMPONENTS Widgets REQUIRED)

    add_executable(nfs_editor)

    # Make them depend on nfs_common lib
    add_dependencies(nfs_editor nfs_common SDL2pp::SDL2pp)
	    
    # Add source files and enable warnings
    add_subdirectory(editor EditorMapas)

    set_project_warnings(nfs_editor ${NFS_MAKE_WARNINGS_AS_ERRORS} FALSE)

    # add libSDL2pp's headers
    target_include_directories(nfs_editor PRIVATE ${libSDL2pp_SOURCE_DIR})
	

    # Link the nfs_common lib target
    target_link_libraries(nfs_editor nfs_common Qt6::Widgets Qt6::Core)
endif()


# Testing section
# ---------------

if(NFS_TESTS)
    # Let's use google test
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
    )

    # For Windows: Prevent overriding the parent project's compiler/linker settings
    # (borrowed from https://google.github.io/googletest/quickstart-cmake.html)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    
    include(GoogleTest)

    # target for testing
    add_executable(nfs_tests)

    # Change this if you have to.
    add_dependencies(nfs_tests nfs_common SDL2pp::SDL2pp)

    # Say where to find the xoz headers
    target_include_directories(nfs_tests PUBLIC .)

    # Add the source files to the nfs_tests target
    add_subdirectory(tests/)

    # Set the compiler warnings to the tests target (but be a little lesser strict)
    set_project_warnings(nfs_tests ${NFS_MAKE_WARNINGS_AS_ERRORS} TRUE)

    # Say where googletest headers can be found
    target_include_directories(nfs_tests
        PUBLIC
        ${gtest_SOURCE_DIR}/include
        ${gmock_SOURCE_DIR}/include
        ${libSDL2pp_SOURCE_DIR}
        )

    # Change this if you have to.
    target_link_libraries(nfs_tests
        nfs_common
        GTest::gtest_main
        gtest
        gmock
        SDL2pp::SDL2pp SDL2_image::SDL2_image SDL2_ttf::SDL2_ttf SDL2_mixer::SDL2_mixer
        nlohmann_json::nlohmann_json
    )
endif()
