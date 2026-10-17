#!/bin/bash

set -e

serve=false
if [[ "${1:-}" == "serve" ]]; then
    serve=true
    shift
fi

export COMPOSE_BAKE=true

docker compose run --rm --remove-orphans --user "$(id -u):$(id -g)" \
    emscripten bash -c \
    'rm -rf embuild && emcmake cmake -S . -B embuild && emmake make -C embuild -j"$(nproc)" "$@"' \
    -- "$@"

if [[ "$serve" == true ]]; then
    docker compose run --rm --remove-orphans -p 8000:8000 \
        emscripten python3 -m http.server 8000 \
        --bind 0.0.0.0 --directory embuild
fi
