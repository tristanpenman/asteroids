cmake_minimum_required(VERSION 3.10)
project(asteroids)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR)
    message(FATAL_ERROR "You must set your binary directory different from your source")
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")

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
    set(OpenGL_GL_PREFERENCE "LEGACY")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -Wall -Wno-long-long -pedantic")
    if(EMSCRIPTEN)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -s USE_SDL=2 -s USE_SDL_MIXER=2 -Wno-gnu-zero-variadic-macro-arguments")
    elseif(NOT WIN32)
        find_package(PkgConfig REQUIRED)
        execute_process(
            COMMAND ${PKG_CONFIG_EXECUTABLE} --exists sdl2 SDL2_mixer
            RESULT_VARIABLE SDL_DEPENDENCIES_MISSING
        )
        if(SDL_DEPENDENCIES_MISSING)
            message(FATAL_ERROR
                "SDL2 development files and SDL2_mixer are required. "
                "Install libsdl2-dev and libsdl2-mixer-dev on Debian/Ubuntu, "
                "or sdl2 and sdl2_mixer with Homebrew."
            )
        endif()
        pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)
        pkg_check_modules(SDL2MIXER REQUIRED IMPORTED_TARGET SDL2_mixer)
        set(SDL_LIBRARIES PkgConfig::SDL2 PkgConfig::SDL2MIXER)
    else()
        set(SDL_LIBRARIES SDL2 SDL2_mixer)
    endif()
endif()

find_package(OpenGL REQUIRED)

set(GAME_SRCS
    src/game/collision.c
    src/game/data.c
    src/game/draw.c
    src/game/entities.c
    src/game/game.c
    src/game/gameover.c
    src/game/highscores.c
    src/game/initials.c
    src/game/leaderboard.c
    src/game/level.c
    src/game/sandbox.c
    src/game/text.c
    src/game/titlescreen.c
    src/game/transition.c
    src/game/util.c
    src/game/vec.c
)

set(DESKTOP_PLATFORM_SRCS
    src/platform/desktop/canvas.c
    src/platform/desktop/debug.c
    src/platform/desktop/input.c
    src/platform/desktop/loop.c
    src/platform/desktop/main.c
    src/platform/desktop/mixer.c
    src/platform/desktop/storage.c
    src/platform/desktop/timing.c
    src/platform/desktop/video.c
)

set(SRCS ${GAME_SRCS} ${DESKTOP_PLATFORM_SRCS})

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
        configure_file(web/index.html index.html COPYONLY)
        set(CMAKE_EXECUTABLE_SUFFIX ".js")
        set(PRELOAD "--use-preload-plugins --preload-file ${CMAKE_CURRENT_SOURCE_DIR}/assets@assets")
        set(EMCC_LINKER_FLAGS "-s USE_SDL=2 -s USE_SDL_MIXER=2 -s GL_FFP_ONLY=1 -s LEGACY_GL_EMULATION=1 --emrun ${PRELOAD}")
        set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS ${EMCC_LINKER_FLAGS})
    else()
        file(COPY ${ASSETS} DESTINATION assets)
        target_link_libraries(${PROJECT_NAME}
            m
            ${OPENGL_LIBRARIES}
            ${SDL_LIBRARIES}
            ${OS_LIBRARIES}
        )
    endif()
endif()

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/game
    ${CMAKE_CURRENT_SOURCE_DIR}/src/platform
)

target_compile_definitions(${PROJECT_NAME} PRIVATE ASTEROIDS_PLATFORM_DESKTOP=1)
