#!/usr/bin/env bash
set -euo pipefail

DB_NAME="osoem_database"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

usage() {
    echo "Usage: $0 [-u USER] [-h HOST] [-p] [-r] [-s]"
    echo "  -u USER   MySQL user (default: root)"
    echo "  -h HOST   MySQL host (default: localhost)"
    echo "  -p        Prompt for password"
    echo "  -s        Seed with sample data"
    exit 1
}

DB_USER="root"
DB_HOST="localhost"
USE_PASSWORD=false
SEED=false

while getopts "u:h:ps" opt; do
    case $opt in
        u) DB_USER="$OPTARG" ;;
        h) DB_HOST="$OPTARG" ;;
        p) USE_PASSWORD=true ;;
        s) SEED=true ;;
        *) usage ;;
    esac
done

echo "WARNING: This will DROP and recreate any existing '$DB_NAME' database, permanently deleting all data."
read -rp "Proceed? [y/N] " CONFIRM
case "$CONFIRM" in
    [yY][eE][sS]|[yY]) ;;
    *) echo "Aborted."; exit 0 ;;
esac

if $USE_PASSWORD; then
    read -rsp "MySQL password: " DB_PASS
    echo
    MYSQL="mysql -u $DB_USER -h $DB_HOST --password=$DB_PASS"
else
    MYSQL="mysql -u $DB_USER -h $DB_HOST"
fi

echo "Dropping database '$DB_NAME' if it exists..."
$MYSQL -e "DROP DATABASE IF EXISTS \`$DB_NAME\`;"

echo "Creating database '$DB_NAME'..."
$MYSQL -e "CREATE DATABASE IF NOT EXISTS \`$DB_NAME\` CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;"

echo "Applying schema..."
$MYSQL "$DB_NAME" < "$SCRIPT_DIR/schema/schema.sql"

echo "Creating temporary admin account..."
$MYSQL "$DB_NAME" < "$SCRIPT_DIR/seed/admin_reset.sql"
echo "WARNING: Temporary admin is for first time login only. Delete after use for security reasons."

if $SEED; then
    echo "Seeding sample employee and project data..."
    $MYSQL "$DB_NAME" < "$SCRIPT_DIR/seed/trial_project.sql"
fi

echo "Done. Database '$DB_NAME' is ready."
