#==============================================================================#
# Forge engine                                                                 #
#==============================================================================#
### Library definition and source code files.
add_library(forge STATIC
  headers/forge/content.h
  headers/forge/game.h
  headers/forge/support/sdl_support.h
  headers/forge/support/stb_support.h
  src/content.cpp
  src/game.cpp
  src/support/sdl_support.cpp
  src/support/stb_support.cpp
)

### Unit tests
include(GoogleTest)

add_executable(test_forge_example "tests/test_example.cpp")
target_link_libraries(test_forge_example PUBLIC GTest::gtest_main forge)
target_compile_features(test_forge_example PUBLIC cxx_std_20)

### Build configuration.
# Set header root to be headers/.
target_include_directories(forge PUBLIC headers PRIVATE src)

# Set the C++ standard to C++/20.
target_compile_features(forge PUBLIC cxx_std_20)

# Link to SDL3 and other third party libraries.
target_link_libraries(forge PUBLIC SDL3::SDL3-static)
target_link_libraries(forge PUBLIC stb_image)