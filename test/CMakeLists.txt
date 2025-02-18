# Create test library
add_library(test_lib STATIC
    registry.cpp
    ../src/tinyECS/tiny_ecs.cpp
    ../src/item_system.cpp
    ../src/tinyECS/registry.cpp
)

# Add include directories for test library
target_include_directories(test_lib PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/ext
    ${CMAKE_SOURCE_DIR}/ext/gl3w
    ${CMAKE_SOURCE_DIR}/ext/glm
    ${CMAKE_SOURCE_DIR}/test
    ${OPENGL_INCLUDE_DIR}
    ${GLFW_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIRS}
)

# Link required libraries
target_link_libraries(test_lib PUBLIC
    ${OPENGL_LIBRARIES}
    glfw
    ${SDL2_LIBRARIES}
    ${SDL2MIXER_LIBRARIES}
    glm::glm
)

# Test executable
add_executable(
  item_system_test
  item_system_test.cpp
)

# Link against GoogleTest and test library
target_link_libraries(
  item_system_test
  PRIVATE
  GTest::gtest_main
  test_lib
)

# Include directories for test executable
target_include_directories(item_system_test PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/ext
    ${CMAKE_SOURCE_DIR}/ext/gl3w
    ${CMAKE_SOURCE_DIR}/ext/glm
    ${CMAKE_SOURCE_DIR}/test
    ${OPENGL_INCLUDE_DIR}
    ${GLFW_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIRS}
)

# Discover tests
include(GoogleTest)
gtest_discover_tests(item_system_test) 