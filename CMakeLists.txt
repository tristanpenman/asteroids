cmake_minimum_required(VERSION 3.19)
project(asteroids)

if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR)
    message(FATAL_ERROR "You must set your binary directory different from your source")
endif()

if(MSVC)
    if(CMAKE_C_FLAGS MATCHES "/W[0-4]")
        string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    else()
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W4")
    endif()
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -Wall -Wno-long-long -pedantic")
endif()

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

find_package(OpenGL REQUIRED)

add_executable(${PROJECT_NAME}
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

if(EMSCRIPTEN)
    configure_file(web/asteroids.html asteroids.html COPYONLY)
    set(CMAKE_EXECUTABLE_SUFFIX ".js")
    set(PRELOAD "--use-preload-plugins --preload-file ${CMAKE_CURRENT_SOURCE_DIR}/assets@assets")
    set(EMCC_LINKER_FLAGS "-s GL_FFP_ONLY=1 -s LEGACY_GL_EMULATION=1 -s \"EXTRA_EXPORTED_RUNTIME_METHODS=['Pointer_stringify']\" --emrun ${PRELOAD}")
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS ${EMCC_LINKER_FLAGS})
else()
    file(GLOB ASSETS ${CMAKE_CURRENT_SOURCE_DIR}/assets/*)
    file(COPY ${ASSETS} DESTINATION assets)
    target_link_libraries(${PROJECT_NAME}
        ${OPENGL_LIBRARIES}
        ${SDL2_LIBRARIES}
        ${SDL2MIXER_LIBRARIES}
        ${OS_LIBRARIES}
        )
endif()
