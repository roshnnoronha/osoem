# Osoem Dashboard

The Osoem dashboard is the web front end for the Osoem engineering project management database. From it you can view projects, activities, tasks, artefacts and milestones, and book time.

It is written in plain PHP 8 with mysqli and Bootstrap, and has no framework or Composer dependencies. The web root is `dashboard/public/`, and the application code is kept outside it in `dashboard/src/`.

> **Deployment status:** Deployment of the dashboard is still a work in progress .For now the only supported way to try the app is to run it locally, as described below.

## Prerequisites

- **Apache 2.4** with **PHP 8** (`mod_php`) and the `mysqli` extension enabled
  - Debian/Ubuntu/WSL: `sudo apt install apache2 libapache2-mod-php php-mysql`
  - Check that the PHP module is loaded with `apache2ctl -M | grep php`
  - Check that `mysqli` is available with `php -m | grep mysqli`
- **MySQL 8** (or a compatible MariaDB) server running locally, plus the `mysql` command-line client
- **Bash**, to run `database/setup.sh`

## Running locally

All commands are run from the repository root.

### 1. Create and seed the database

```bash
./database/setup.sh -s -p
```

This drops and recreates the `osoem_database` database, applies the schema, creates a temporary admin account, and loads the sample project data (`-s`). Use `-u USER` and `-h HOST` if you are not connecting as `root@localhost`. See `database/README.md` for all options.

> **Warning:** `setup.sh` always drops any existing `osoem_database` database. It asks for confirmation before deleting anything.

### 2. Configure the database connection

The dashboard reads its connection details from the environment variables `DB_HOST`, `DB_USERNAME`, `DB_PASSWORD` and `DB_DATABASE`. For local use, put them in `dashboard/.env`:

```bash
cp dashboard/.env.example dashboard/.env
```

Then edit `dashboard/.env`:

```ini
DB_HOST=localhost
DB_USERNAME=root
DB_PASSWORD=your_mysql_password
DB_DATABASE=osoem_database
```

`DB_USERNAME` and `DB_PASSWORD` must both be present (the password can be left empty if your MySQL user has no password). `DB_HOST` defaults to `localhost` and `DB_DATABASE` defaults to `osoem_database`. Variables already set in the server's environment (for example with `SetEnv` in the Apache config) take precedence over the values in `.env`. Variables exported in your own shell are not seen by Apache.

### 3. Configure Apache

Create a site config at `/etc/apache2/sites-available/osoem.conf` that serves `dashboard/public` on port 8000. Replace `/path/to/osoem` with the absolute path to your clone of the repository (run `pwd` from the repository root to get it):

```apache
Listen 127.0.0.1:8000

<VirtualHost 127.0.0.1:8000>
    ServerName localhost
    DocumentRoot /path/to/osoem/dashboard/public
    DirectoryIndex index.php

    <Directory /path/to/osoem/dashboard/public>
        Options -Indexes +FollowSymLinks
        AllowOverride None
        Require all granted
    </Directory>

    ErrorLog ${APACHE_LOG_DIR}/osoem_error.log
    CustomLog ${APACHE_LOG_DIR}/osoem_access.log combined
</VirtualHost>
```

Only `dashboard/public` is exposed. `dashboard/src` and `dashboard/.env` sit outside the document root, so Apache never serves them directly.

Apache runs as the `www-data` user, which needs read access to `dashboard/` (including `.env`) and execute (traverse) permission on every parent directory. If the repository is under your home directory and you get `403 Forbidden`, run `chmod o+x` on the parent directories (e.g. `chmod o+x ~`).

### 4. Enable the site and start Apache

```bash
sudo a2ensite osoem
sudo apache2ctl configtest
sudo service apache2 restart
```

`configtest` should print `Syntax OK`. On systems using systemd you can use `sudo systemctl restart apache2` instead.

### 5. Open the app

Go to <http://localhost:8000> in your browser. On the sign-in page, click **Use the demo account** to fill in the credentials of a sample user from the seed data, then click **Sign in**.

PHP errors are written to `/var/log/apache2/osoem_error.log`.

To stop serving the app, disable the site and reload Apache:

```bash
sudo a2dissite osoem
sudo service apache2 reload
```

## Troubleshooting

| Symptom | Likely cause |
|---------|--------------|
| `Database credentials not configured` | `dashboard/.env` is missing, or `DB_USERNAME`/`DB_PASSWORD` is not set in it. |
| `Connection failed: Access denied ...` | The username or password in `dashboard/.env` is wrong. |
| `Connection failed: Unknown database ...` | The database has not been created yet. Run step 1. |
| `Call to undefined function mysqli...` / `Class "mysqli" not found` | The PHP `mysqli` extension is not installed or not enabled. Install `php-mysql` and restart Apache. |
| Browser shows raw PHP source or downloads `index.php` | `mod_php` is not enabled. Run `sudo a2enmod php8.x` (matching your PHP version) and restart Apache. |
| `403 Forbidden` | Apache cannot read `dashboard/public` or traverse a parent directory. See the permissions note in step 3. |
| `Invalid email id.` with the demo account | The database was set up without `-s`, so the sample data is missing. |
| `Address already in use` when starting Apache | Port 8000 is taken. Change both the `Listen` and `<VirtualHost>` ports in `osoem.conf`, e.g. to `8080`. |

## Security notes for local use

- The site config above listens on `127.0.0.1` only, so the app is not reachable from other machines. Do not change it to listen on all interfaces without proper hardening (HTTPS, a dedicated database user with limited privileges, etc.).
- `setup.sh` creates a temporary admin account (`database/seed/admin_reset.sql`). Delete it once you have created your own admin user with the admin CLI (`admin/`).
