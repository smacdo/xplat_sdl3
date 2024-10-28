cmake_minimum_required(VERSION 3.16)
project(xplat_sdl3)

include(FetchContent)

#==============================================================================#
# Project wide settings                                                        #
#==============================================================================#
# Set ON to build a version of SDL3 from Git and set the OFF to use a locally
# installed SDL3.
option(USE_VENDORED_LIBS "Use vendored libraries" ON)

# Set output directory for build artifacts, which ensures that the SDL dynamic
# library is put into the build directory.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

#==============================================================================#
# SDL3                                                                         #
#==============================================================================#
# Force the project to link a static build of SDL3 rather than rely on a dynamic
# library. Static builds are preferred to avoid crashes at program start up on
# iOS builds.
set(SDL_SHARED OFF)
set(SDL_STATIC ON)

# Pull in the SDL3 package.
if(USE_VENDORED_LIBS)
  # Check out a fresh copy of SDL3 when using vendored libraries.
  FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG        e292d1f # preview-3.1.3
  )

  FetchContent_MakeAvailable(SDL3)
else()
  # Find SDL3 on the local machine when vendored library usage is disabled.
  find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-static SDL3-shared)
endif()

# Other third-party code.
add_subdirectory(third-party/stb)

#==============================================================================#
# Forge engine                                                                 #
#==============================================================================#
### Library definition and source code files.
add_library(forge STATIC
  headers/forge/content.h
  headers/forge/support/sdl_support.h
  headers/forge/support/stb_support.h
  src/forge/content.cpp
  src/forge/support/sdl_support.cpp
  src/forge/support/stb_support.cpp
)

### Build configuration.
# Set header root to be headers/.
target_include_directories(forge PUBLIC headers/ PRIVATE src/)

# Set the C++ standard to C++/20.
target_compile_features(forge PUBLIC cxx_std_20)

# Link to SDL3 and other third party libraries.
target_link_libraries(forge PUBLIC SDL3::SDL3-static)
target_link_libraries(forge PUBLIC stb_image)

#==============================================================================#
# Game application target                                                      #
#==============================================================================#
### Game configuration. 
set(GAME_EXE_NAME xplat_sdl3)
set(GAME_AUTHOR Scott MacDonald)
set(GAME_COPYRIGHT "Copyright © 2024 Scott MacDonald. All rights reserved.")

set(APPLE_TEAM_ID "X6VYF32YUU")
set(APPLE_CODE_SIGN_IDENTITY "Apple Development")  # `/usr/bin/env xcrun security find-identity -v -p codesigning`
set(APP_BUNDLE_IDENTIFIER "com.smacdo.xplatsdl3") 
set(APP_VERSION "1.0")

set(APPLE_DEVICE_FAMILY "1")  # 1 = iOS, 2 = iPad and set to "1,2" for both.
set(IOS_DEPLOYMENT_TARGET "17")

### Declare the top level game exe target.
# On Android the SDL Java code is hardcoded to load the program entry points
# from a dynamic library called "libmain.so". Otherwise on all other platforms
# the game exe is a standard CMake executable target.
if (ANDROID)
  set(GAME_EXE_NAME main)
  add_library(${GAME_EXE_NAME} SHARED)
else()
  add_executable(${GAME_EXE_NAME})
endif()

### Source code files.
target_sources(${GAME_EXE_NAME}
  PRIVATE
    src/main.cpp
)

### Game asset files.
set(GAME_CONTENT_FILES
  content/bubble.png
)

### Build configuration.
# Set the C++ standard to C++/20.
target_compile_features(${GAME_EXE_NAME} PUBLIC cxx_std_20)

# Link to SDL3 and other third party libraries.
target_link_libraries(${GAME_EXE_NAME} PUBLIC forge)

target_link_libraries(${GAME_EXE_NAME} PUBLIC SDL3::SDL3-static)
target_compile_definitions(${GAME_EXE_NAME} PUBLIC SDL_MAIN_USE_CALLBACKS)

target_link_libraries(${GAME_EXE_NAME} PUBLIC stb_image)

### Platform specific support.
# Emscripten (web builds).
if (CMAKE_SYSTEM_NAME MATCHES Emscripten)
  set(CMAKE_EXECUTABLE_SUFFIX ".html" CACHE INTERNAL "")
endif()

# iOS / MacOS build support.
if(APPLE)
  # Force the game to build as a MacOS application bundle.
  #set_target_properties(${GAME_EXE_NAME} PROPERTIES MACOSX_BUNDLE TRUE)

  # Add the application logo and app launch screen scene.
  target_sources(${GAME_EXE_NAME} PRIVATE logo.png src/iosLaunchScreen.storyboard)

  # Copy game content files to the app resources bundle.
  target_sources(${GAME_EXE_NAME} PRIVATE ${GAME_CONTENT_FILES})

  set_source_files_properties(
    ${GAME_CONTENT_FILES}
    PROPERTIES
      MACOSX_PACKAGE_LOCATION Resources/content
  )

  # Various properties required for building and signing apps.
  set(MACOSX_BUNDLE_EXECUTABLE_NAME "${GAME_EXE_NAME}")
  set(MACOSX_BUNDLE_BUNDLE_NAME "${APP_BUNDLE_IDENTIFIER}")
  set(MACOSX_BUNDLE_INFO_STRING "${APP_BUNDLE_IDENTIFIER}")
  set(MACOSX_BUNDLE_GUI_IDENTIFIER "${APP_BUNDLE_IDENTIFIER}")
  set(MACOSX_BUNDLE_LONG_VERSION_STRING "${APP_VERSION}")
  set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${APP_VERSION}")
  set(MACOSX_BUNDLE_BUNDLE_VERSION "${APP_VERSION}")
  set(MACOSX_BUNDLE_COPYRIGHT "${GAME_COPYRIGHT}")
  set(MACOSX_BUNDLE_ICON_FILE "logo.png")
  set(MACOSX_DEPLOYMENT_TARGET "${IOS_DEPLOYMENT_TARGET}")

  # Apply additional XCode project settings.
  set_target_properties(${GAME_EXE_NAME} PROPERTIES
    XCODE_ATTRIBUTE_DEVELOPMENT_TEAM  "${APPLE_TEAM_ID}"
    XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "${CODE_SIGN_IDENTITY}"
    XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET "${IOS_DEPLOYMENT_TARGET}"
    XCODE_ATTRIBUTE_BUNDLE_IDENTIFIER "${APP_BUNDLE_IDENTIFIER}"
    XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "${APP_BUNDLE_IDENTIFIER}"
    XCODE_ATTRIBUTE_CURRENTYEAR "${CURRENTYEAR}"
    XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET "${IOS_DEPLOYMENT_TARGET}"

    # Set the app's linker search path to the default location on iOS.
    #XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS "@executable_path/Frameworks"
    
    # Make an app bundle rather than a bare executable
    MACOSX_BUNDLE TRUE

    # Info.plist is required for Apple mobile builds otherwise the app will not
    # launch.
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/src/Info.plist.in"
    XCODE_GENERATE_SCHEME TRUE
  )


endif()

# Visual Studio settings
set_property(
  # The default project.
  DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT "${GAME_EXE_NAME}"
)

#==============================================================================#
# Game assets                                                                  #
#==============================================================================#
# TODO: Verify this works for non-Apple platforms.
macro(add_assets)
    cmake_parse_arguments("MY" "TARGET" "FILES" ${ARGN})

    foreach(src ${MY_SOURCES})
        set(OUTF "${CMAKE_CURRENT_BINARY_DIR}/assets/${src}")
        get_filename_component(PARENT_DIR "${OUTF}" DIRECTORY)
        add_custom_command(
            OUTPUT "${OUTF}"
            COMMAND echo "Copy ${src}"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${PARENT_DIR}"
            COMMAND  cp -r "${CMAKE_CURRENT_SOURCE_DIR}/assets/${src}" "${OUTF}"
            DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${src}"
            VERBATIM
        )

        # src in order to get listed in the IDE, OUTF to declare the build relationship
        target_sources("${MY_TARGET}" PRIVATE "${src}" "${OUTF}")
    endforeach()
endmacro()

add_assets(TARGET ${GAME_EXE_NAME}
  FILES
    bubble.png  
)

#add_custom_target(copy_assets
#  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_LIST_DIR}/copy-assets.cmake
#)

#add_dependencies(${GAME_EXE_NAME} copy_assets)