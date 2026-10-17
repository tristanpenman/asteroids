#!/bin/bash
set -euo pipefail

export COMPOSE_BAKE=true

docker compose run --build --rm --remove-orphans --user "$(id -u):$(id -g)" \
    n64 make -f n64/Makefile "$@"
