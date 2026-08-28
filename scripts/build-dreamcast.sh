#!/bin/bash
set -euo pipefail

export COMPOSE_BAKE=true

docker compose run --build --rm --remove-orphans --user "$(id -u):$(id -g)" \
    dreamcast bash -lc 'source /opt/toolchains/dc/kos/environ.sh && make -f dreamcast/Makefile "$@"' \
    bash "$@"
