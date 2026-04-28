#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CONFIG_FILE="$SCRIPT_DIR/config/db_config.h"
CONFIG_EXAMPLE="$SCRIPT_DIR/config/db_config.h.example"
BUILD_DIR="$SCRIPT_DIR/build"

usage() {
    echo "Usage: $0 [-c] [-d]"
    echo "  -c    Reconfigure db_config.h (overwrite existing)"
    echo "  -d    Install build dependencies via apt (requires sudo)"
    exit 1
}

RECONFIGURE=false
INSTALL_DEPS=false

while getopts "cd" opt; do
    case $opt in
        c) RECONFIGURE=true ;;
        d) INSTALL_DEPS=true ;;
        *) usage ;;
    esac
done

if $INSTALL_DEPS; then
    echo "Installing build dependencies..."
    sudo apt update
    sudo apt install -y cmake g++ libmysqlcppconn-dev libreadline-dev libssl-dev
fi

MISSING=()
command -v cmake &>/dev/null || MISSING+=("cmake")
command -v g++   &>/dev/null || MISSING+=("g++")
IT [[ ${#MISSING[@]} -gt 0 ]]; then
    echo "ERROR: Missing required tools: ${MISSING[*]}"
    echo "Install with: sudo apt install cmake g++ libmysqlcppconn-dev libreadline-dev libssl-dev"
    exit 1
fi

if [[ ! -f "$CONFIG_FILE" ]] || $RECONFIGURE; then
    echo "Configuring database connection (press Enter to accept defaults)..."
    read -rp "  Host     [localhost]: " DB_HOST;  DB_HOST="${DB_HOST:-localhost}"
    read -rp "  Port     [3306]:      " DB_PORT;  DB_PORT="${DB_PORT:-3306}"
    read -rp "  User:                 " DB_USER
    read -rsp "  Password:            " DB_PASS;  echo

    {
        echo '#ifndef DB_CONFIG_H'
        echo '#define DB_CONFIG_H'
        echo ''
        echo '#include <string>'
        echo ''
        echo 'namespace config {'
        printf '    const std::string DB_HOST = "%s";\n' "$DB_HOST"
        printf '    const std::string DB_USER = "%s";\n' "$DB_USER"
        printf '    const std::string DB_PASS = "%s";\n' "$DB_PASS"
        echo '    const std::string DB_NAME = "osoem_database";'
        printf '    const int DB_PORT = %s;\n' "$DB_PORT"
        echo '}'
        echo ''
        echo '#endif // DB_CONFIG_H'
    } > "$CONFIG_FILE"

    echo "Written: config/db_config.h"
else
    echo "config/db_config.h already exists (use -c to reconfigure)."
fi

echo "Configuring build..."
cmake -S "$SCRIPT_DIR" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release

echo "Building osoem_admin..."
cmake --build "$BUILD_DIR" --target=osoem_admin -- -j"$(nproc)"

echo "Done. Executable: build/osoem_admin"
