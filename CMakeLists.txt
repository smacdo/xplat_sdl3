cmake_minimum_required(VERSION 3.16)
include(FetchContent)

# Set output directory for build artifacts, which ensures that the SDL dynamic
# library is put into the build directory.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

#==============================================================================#
#  Project Config                                                              #
#==============================================================================#
project(xplat_sdl3)

set(GAME_EXE_NAME xplat_sdl3)
set(GAME_AUTHOR Scott MacDonald)

option(USE_VENDORED_LIBS "Use vendored libraries" ON)

# TODO: Move this down?
if (ANDROID)
  # The Java code in SDL on Android is hardcoded to load libmain.so so the
  # program executable needs to be switched to `main`.
  set(GAME_EXE_NAME main)
  add_library(${GAME_EXE_NAME} SHARED)
else()
  add_executable(${GAME_EXE_NAME})
endif()

# TODO: Only include ios storyboard on iOS
target_sources(${GAME_EXE_NAME}
  PRIVATE
    src/main.cpp
)

if (APPLE)
  target_sources(${GAME_EXE_NAME} PRIVATE logo.png src/iosLaunchScreen.storyboard)
endif()

# TODO: move line below
# TODO: c++ latest supported for iOS
target_compile_features(${GAME_EXE_NAME} PUBLIC cxx_std_20)

if (CMAKE_SYSTEM_NAME MATCHES Emscripten)
  set(CMAKE_EXECUTABLE_SUFFIX ".html" CACHE INTERNAL "")
endif()

#==============================================================================#
# SDL3                                                                         #
#==============================================================================#
if(USE_VENDORED_LIBS)
  FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG        e292d1f # preview-3.1.3
  )

  FetchContent_MakeAvailable(SDL3)
else()
  # 1. Look for a SDL3 package,
  # 2. look for the SDL3-shared component, and
  # 3. fail if the shared component cannot be found.
  find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared)
endif()

target_link_libraries(${GAME_EXE_NAME} PUBLIC SDL3::SDL3)
target_compile_definitions(${GAME_EXE_NAME} PUBLIC SDL_MAIN_USE_CALLBACKS)


# XCode settings
set_target_properties(${EXECUTABLE_NAME} PROPERTIES 
    # Make an app bundle rather than a bare executable
    MACOSX_BUNDLE TRUE
    # Info.plist is required for Apple mobile builds otherwise the app will not
    # launch.
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/src/Info.plist.in"

    XCODE_GENERATE_SCHEME TRUE
    XCODE_ATTRIBUTE_BUNDLE_IDENTIFIER "com.smacdo.${GAME_EXE_NAME}"
	  XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.smacdo.${GAME_EXE_NAME}"
	  XCODE_ATTRIBUTE_CURRENTYEAR "${CURRENTYEAR}"
)

# Visual Studio settings
set_property(
  # The default project.
  DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT "${GAME_EXE_NAME}"
)

# TODO: TEST BELOW CODE!
# Ensures the app bundle that was built is valid for distribution on macOS.
# NOTE: `fixup_bundle` does not work on iOS so the project needs to link to the
#       static library variant of SDL3, or manually copy dylibs + set rpaths.
#if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
if(CMAKE_GENERATOR STREQUAL Xcode)
    INSTALL(CODE 
        "include(BundleUtilities)
        fixup_bundle(\"$<TARGET_BUNDLE_DIR:${GAME_EXE_NAME}>\" \"\" \"\")
        " 
        COMPONENT Runtime
    )
  install(TARGETS ${GAME_EXE_NAME} BUNDLE DESTINATION ./install)
endif()
