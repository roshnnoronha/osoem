# Database Setup

Use `setup.sh` to create and initialise the `osoem_database` MySQL database.

> **Warning:** The script always drops and recreates the database. A confirmation prompt is shown before any data is deleted.

## Usage

```
./database/setup.sh [-u USER] [-h HOST] [-p] [-s]

  -u USER   MySQL user (default: root)
  -h HOST   MySQL host (default: localhost)
  -p        Prompt for password
  -s        Seed with sample data
```

## Examples

```bash
# Basic setup (creates schema, connects as root@localhost, no password)
./database/setup.sh

# With password prompt
./database/setup.sh -p

# Custom user and host
./database/setup.sh -u myuser -h 192.168.1.10 -p

# Setup and seed with sample data
./database/setup.sh -s -p
```

## What it does

1. Prompts for confirmation, then drops and recreates the `osoem_database` database (utf8mb4)
2. Applies `database/schema/schema.sql`
3. Seeds a temporary admin account from `database/seed/admin_reset.sql` — **delete after first login**
4. Optionally seeds sample project data from `database/seed/trial_project.sql` (`-s`)
