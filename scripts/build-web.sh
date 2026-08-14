#!/bin/bash

export COMPOSE_BAKE=true

docker compose run --rm --remove-orphans --user "$(id -u):$(id -g)" \
    emscripten bash -c \
    'rm -rf embuild && emcmake cmake -S . -B embuild && emmake make -C embuild -j"$(nproc)" "$@"' \
    -- "$@"
