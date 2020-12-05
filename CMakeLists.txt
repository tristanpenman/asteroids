cmake_minimum_required(VERSION 3.19)
project(asteroids)

if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR)
    message(FATAL_ERROR "You must set your binary directory different from your source")
endif()

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    if(CMAKE_C_FLAGS MATCHES "/W[0-4]")
        string(REGEX REPLACE "/W[0-4]" "/W2" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    else()
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W2")
    endif()
    include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/SDL2-2.0.9/include
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/SDL2_mixer-2.0.4/include
    )
    link_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/SDL2-2.0.9/lib/x64
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/SDL2_mixer-2.0.4/lib/x64
    )
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -Wall -Wno-long-long -pedantic")
    if(EMSCRIPTEN)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -Wall -s USE_SDL=2 --closure 0 -Wno-gnu-zero-variadic-macro-arguments")
    else()
        include(FindPkgConfig)
        pkg_search_module(SDL2 REQUIRED sdl2)
        pkg_search_module(SDL2MIXER REQUIRED SDL2_mixer)
        include_directories(
            ${SDL2_INCLUDE_DIRS}
            ${SDL2MIXER_INCLUDE_DIRS}
        )
        link_directories(
            ${SDL2_LIBRARY_DIRS}
            ${SDL2MIXER_LIBRARY_DIRS}
        )
    endif()
endif()

find_package(OpenGL REQUIRED)

set(SRCS
    src/asteroid.c
    src/collision.c
    src/data.c
    src/draw.c
    src/game.c
    src/highscores.c
    src/initials.c
    src/level.c
    src/loop.c
    src/main.c
    src/mixer.c
    src/timing.c
    src/titlescreen.c
    src/transition.c
    src/util.c
    src/vec.c
    src/video.c
)

file(GLOB ASSETS ${CMAKE_CURRENT_SOURCE_DIR}/assets/*)

if(MSVC)
    add_executable(${PROJECT_NAME} WIN32 ${SRCS})
    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
                ${CMAKE_CURRENT_BINARY_DIR}/$(Configuration)/assets
    )
    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${ASSETS}
                ${CMAKE_CURRENT_BINARY_DIR}/$(Configuration)/assets
    )
    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/SDL2-2.0.9/lib/x64/SDL2.dll
                ${CMAKE_CURRENT_BINARY_DIR}/$(Configuration)
    )
    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/SDL2_mixer-2.0.4/lib/x64/SDL2_mixer.dll
                ${CMAKE_CURRENT_BINARY_DIR}/$(Configuration)
    )
    target_link_libraries(${PROJECT_NAME}
        opengl32
        SDL2
        SDL2main
        SDL2_mixer
    )
    set_target_properties(
        ${PROJECT_NAME} PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/$(Configuration)"
    )
else()
    add_executable(${PROJECT_NAME} ${SRCS})
    if(EMSCRIPTEN)
        configure_file(web/asteroids.html asteroids.html COPYONLY)
        set(CMAKE_EXECUTABLE_SUFFIX ".js")
        set(PRELOAD "--use-preload-plugins --preload-file ${CMAKE_CURRENT_SOURCE_DIR}/assets@assets")
        set(EMCC_LINKER_FLAGS "-s GL_FFP_ONLY=1 -s LEGACY_GL_EMULATION=1 -s \"EXTRA_EXPORTED_RUNTIME_METHODS=['Pointer_stringify']\" --emrun ${PRELOAD}")
        set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS ${EMCC_LINKER_FLAGS})
    else()
        file(COPY ${ASSETS} DESTINATION assets)
        target_link_libraries(${PROJECT_NAME}
            ${OPENGL_LIBRARIES}
            ${SDL2_LIBRARIES}
            ${SDL2MIXER_LIBRARIES}
            ${OS_LIBRARIES}
        )
    endif()
endif()