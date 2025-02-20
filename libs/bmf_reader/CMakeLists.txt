### Library definition and source code files.
add_library(bmf_reader STATIC
        headers/bmf_reader/bmf_reader.h
        src/bmf_reader.cpp
)

### Unit tests
#include(GoogleTest)

#add_executable(test_bmf_reader "tests/test_example.cpp")
#target_link_libraries(test_bmf_reader PUBLIC GTest::gtest_main bmf_reader)
#target_compile_features(test_bmf_reader PUBLIC cxx_std_20)

### Build configuration.
# Set header root to be headers/.
target_include_directories(bmf_reader PUBLIC headers PRIVATE src)

# Set the C++ standard to C++/20.
target_compile_features(bmf_reader PUBLIC cxx_std_20)
