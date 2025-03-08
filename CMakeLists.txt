# CMakeLists.txt for Towers vs. Invaders
cmake_minimum_required(VERSION 3.12)

project(enchanted_grotto)

# use C++17
set (CMAKE_CXX_STANDARD 17)

# nice hierarchichal structure in MSVC
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Option to build the game (default: ON)
option(BUILD_GAME "Build the game executable and library" ON)
option(BUILD_TESTING "Build the tests" OFF)

# detect OS
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(IS_OS_MAC 1)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(IS_OS_LINUX 1)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(IS_OS_WINDOWS 1)
else()
    message(FATAL_ERROR "OS ${CMAKE_SYSTEM_NAME} was not recognized")
endif()

# Create executable target

# Generate the shader folder location to the header
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/ext/project_path.hpp.in" "${CMAKE_CURRENT_SOURCE_DIR}/ext/project_path.hpp")

# You can switch to use the file GLOB for simplicity but at your own risk
file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.hpp)

# external libraries will be installed into /usr/local/include and /usr/local/lib but that folder is not automatically included in the search on MACs
if (IS_OS_MAC)
    include_directories(/usr/local/include)
    link_directories(/usr/local/lib)
    # 2024-09-24 - added for M-series Mac's
    include_directories(/opt/homebrew/include)
    link_directories(/opt/homebrew/lib)
endif()

if(BUILD_GAME)
    add_executable(${PROJECT_NAME} ${SOURCE_FILES})
    target_include_directories(${PROJECT_NAME} PUBLIC src/)

    # Added this so policy CMP0065 doesn't scream
    set_target_properties(${PROJECT_NAME} PROPERTIES ENABLE_EXPORTS 0)

    # External header-only libraries in the ext/
    target_include_directories(${PROJECT_NAME} PUBLIC ext/stb_image/)
    target_include_directories(${PROJECT_NAME} PUBLIC ext/gl3w)
    target_include_directories(${PROJECT_NAME} PUBLIC ext)  # For nlohmann/json.hpp

    # Find OpenGL
    find_package(OpenGL REQUIRED)

    if (OPENGL_FOUND)
       target_include_directories(${PROJECT_NAME} PUBLIC ${OPENGL_INCLUDE_DIR})
       target_link_libraries(${PROJECT_NAME} PUBLIC ${OPENGL_gl_LIBRARY})
    endif()
endif()

set(glm_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/glm/cmake/glm) # if necessary
find_package(glm REQUIRED)

# glfw, sdl could be precompiled (on windows) or installed by a package manager (on OSX and Linux)
if (IS_OS_LINUX OR IS_OS_MAC)
    # Try to find packages rather than to use the precompiled ones
    # Since we're on OSX or Linux, we can just use pkgconfig.
    find_package(PkgConfig REQUIRED)

    pkg_search_module(GLFW REQUIRED glfw3)

    pkg_search_module(SDL2 REQUIRED sdl2)
    pkg_search_module(SDL2MIXER REQUIRED SDL2_mixer)

    if(BUILD_GAME)
        # Link Frameworks on OSX
        if (IS_OS_MAC)
           find_library(COCOA_LIBRARY Cocoa)
           find_library(CF_LIBRARY CoreFoundation)
           target_link_libraries(${PROJECT_NAME} PUBLIC ${COCOA_LIBRARY} ${CF_LIBRARY})
        endif()
        
        # Increase warning level
        target_compile_options(${PROJECT_NAME} PUBLIC "-Wall")
    endif()
elseif (IS_OS_WINDOWS)
# https://stackoverflow.com/questions/17126860/cmake-link-precompiled-library-depending-on-os-and-architecture
    set(GLFW_FOUND TRUE)
    set(SDL2_FOUND TRUE)

    # include directories
    set(GLFW_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/ext/glfw/include")
    set(SDL2_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/ext/sdl/include/SDL")

    # library files
    set(GLFW_LIBRARIES "${CMAKE_CURRENT_SOURCE_DIR}/ext/glfw/lib/glfw3dll-x64.lib")
    set(SDL2_LIBRARIES "${CMAKE_CURRENT_SOURCE_DIR}/ext/sdl/lib/SDL2-x64.lib")
    set(SDL2MIXER_LIBRARIES "${CMAKE_CURRENT_SOURCE_DIR}/ext/sdl/lib/SDL2_mixer-x64.lib")

    if(BUILD_GAME)
        # matching DLLs
        set(GLFW_DLL "${CMAKE_CURRENT_SOURCE_DIR}/ext/glfw/lib/glfw3-x64.dll")
        set(SDL_DLL "${CMAKE_CURRENT_SOURCE_DIR}/ext/sdl/lib/SDL2-x64.dll")
        set(SDLMIXER_DLL "${CMAKE_CURRENT_SOURCE_DIR}/ext/sdl/lib/SDL2_mixer-x64.dll")

        # copy DLLs to build folder and remove if necessary name
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${GLFW_DLL}"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/glfw3.dll")

        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${SDL_DLL}"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/SDL2.dll")

        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${SDLMIXER_DLL}"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/SDL2_mixer.dll")

        # increase warning level from default 3 to 4
        add_compile_options(/w4)

        # turn warning "not all control paths return a value" into an error
        add_compile_options(/we4715)

        # use sane exception handling
        add_compile_options(/EHsc)

        # turn warning C4239 into an error
        add_compile_options(/we4239)
    endif()
endif()

# if we can't find the include and lib, then report error and quit.
if (NOT GLFW_FOUND OR NOT SDL2_FOUND)
    if (NOT GLFW_FOUND)
        message(FATAL_ERROR "Can't find GLFW." )
    else ()
        message(FATAL_ERROR "Can't find SDL." )
    endif()
endif()

if(BUILD_GAME)
    target_include_directories(${PROJECT_NAME} PUBLIC ${GLFW_INCLUDE_DIRS})
    target_include_directories(${PROJECT_NAME} PUBLIC ${SDL2_INCLUDE_DIRS})

    target_link_libraries(${PROJECT_NAME} PUBLIC ${GLFW_LIBRARIES} ${SDL2_LIBRARIES} ${SDL2MIXER_LIBRARIES} glm::glm)

    # needed to add this for Linux
    if(IS_OS_LINUX)
        target_link_libraries(${PROJECT_NAME} PUBLIC glfw ${CMAKE_DL_LIBS})
    endif()
endif()

# Only include testing setup if BUILD_TESTING is ON
if(BUILD_TESTING)
    # Based on googletest docs: http://google.github.io/googletest/quickstart-cmake.html

    include(FetchContent)
    FetchContent_Declare(
      googletest
      URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
    )

    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()

    # Add test directory
    add_subdirectory(test)
endif()

# RmlUI (optional)
option(USE_RMLUI "Enable RmlUi for game UI" ON)

if(USE_RMLUI)
  # Try to find RmlUi through vcpkg first
  set(RmlUi_ROOT "${CMAKE_SOURCE_DIR}/ext/RmlUi/Build")
  find_package(Freetype REQUIRED)
  find_package(RmlUi REQUIRED)
  
  if(RmlUi_FOUND)
    message(STATUS "RmlUI found via vcpkg - UI system will be enabled")
    target_link_libraries(${PROJECT_NAME} PUBLIC RmlUi::RmlUi)
    target_compile_definitions(${PROJECT_NAME} PUBLIC USE_RMLUI=1)
    
    # Add Windows-specific configuration
    if(IS_OS_WINDOWS)
      # When using vcpkg's RmlUi, we need to use dynamic linking
      target_compile_definitions(${PROJECT_NAME} PUBLIC RMLUI_DYNAMIC_LINKING=1)
      
      # Create font directory in build
      file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/data/fonts)
      
      # Copy font file to build directory
      file(COPY "${CMAKE_SOURCE_DIR}/data/fonts/OpenSans-Regular.ttf" 
            DESTINATION "${CMAKE_BINARY_DIR}/data/fonts")
      if(EXISTS "${CMAKE_BINARY_DIR}/Debug")
        file(COPY "${CMAKE_SOURCE_DIR}/data/fonts/OpenSans-Regular.ttf" 
              DESTINATION "${CMAKE_BINARY_DIR}/Debug/data/fonts")
      endif()
      if(EXISTS "${CMAKE_BINARY_DIR}/Release")
        file(COPY "${CMAKE_SOURCE_DIR}/data/fonts/OpenSans-Regular.ttf" 
              DESTINATION "${CMAKE_BINARY_DIR}/Release/data/fonts")
      endif()
    endif()
    
    # Add Mac-specific configuration
    if(IS_OS_MAC)
      # Create font directory in build
      file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/data/fonts)
      
      # Copy fonts from ext directory to build directory
      file(COPY "${CMAKE_SOURCE_DIR}/data/fonts/OpenSans-Regular.ttf" 
            DESTINATION "${CMAKE_BINARY_DIR}/data/fonts")
    endif()
  else()
    message(WARNING "RmlUI not found - UI system will be disabled")
    target_compile_definitions(${PROJECT_NAME} PUBLIC USE_RMLUI=0)
  endif()
else()
  message(STATUS "RmlUI disabled - UI system will not be available")
  target_compile_definitions(${PROJECT_NAME} PUBLIC USE_RMLUI=0)
endif()
