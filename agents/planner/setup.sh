#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

if [ ! -f "$SCRIPT_DIR/config/db_config.h" ]; then
    cp "$SCRIPT_DIR/config/db_config.h.example" "$SCRIPT_DIR/config/db_config.h"
    echo "Created config/db_config.h — edit it with your database credentials."
fi

if [ ! -f "$SCRIPT_DIR/config/api_config.h" ]; then
    cp "$SCRIPT_DIR/config/api_config.h.example" "$SCRIPT_DIR/config/api_config.h"
    echo "Created config/api_config.h — edit it with your OpenAI API key."
fi

mkdir -p "$SCRIPT_DIR/build"
cd "$SCRIPT_DIR/build"
cmake ..
cmake --build . --target osoem_planner

echo ""
echo "Build complete. Run with: $SCRIPT_DIR/build/osoem_planner"
