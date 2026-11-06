# OSOEM Admin Setup

## Prerequisites

- Linux or WSL (Windows Subsystem for Linux)
- CMake 3.10 or higher
- C++17 compatible compiler (g++)
- MySQL/MariaDB server
- Required libraries: `libmysqlcppconn-dev`, `libreadline-dev`, `libssl-dev`

## Setup

### 1. Create the database

Run the database setup script from the `database/` directory (see: ../database/README.md for more information). This drops and recreates the `osoem_database` MySQL database, applies the schema, and inserts a temporary admin account for first-time login.

### 2. Build the admin tool

Run the setup script from the `admin/` directory:

```bash
./setup.sh
```

This will:
1. Prompt for database connection details (host, port, user, password) and write `config/db_config.h`
2. Configure the CMake build (creates the `build/` directory if it does not exist)
3. Compile the `osoem_admin` executable to `build/osoem_admin`

Options:
```
./setup.sh -d    # Install build dependencies via apt (requires sudo), then build
./setup.sh -c    # Reconfigure db_config.h (overwrite existing), then build
./setup.sh -cd   # Both: install deps and reconfigure
```

The `-d` flag installs the following packages via `apt`:
```bash
sudo apt install cmake g++ libmysqlcppconn-dev libreadline-dev libssl-dev
```

For Fedora/RHEL, install dependencies manually before running `./setup.sh`:
```bash
sudo dnf install cmake gcc-c++ mysql-connector-c++-devel readline-devel openssl-devel
```

### 3. First-time login

The database setup creates a temporary admin account (`email: admin`, no password) in a department named `IT`. Use it to create your real admin employee, then delete it.

**Log in as the temporary admin:**
```bash
./build/osoem_admin -u admin
# Press Enter when prompted for password (no password set)
```

**Create a department and your admin employee:**
```
cd Organization/Departments
ad "departmentname:<department_name>"
cd "<department_name>"
ad
# Fill in the prompts: FirstName, LastName, Email (must contain '@'), Password, Admin (y/n)
# Answer 'y' to Admin so the new account has admin privileges
```

Alternatively, skip creating a department and add your employee to the existing `IT` department.

**Log out (`exit` or `quit`) and log back in as your real admin account:**
```bash
./build/osoem_admin -u your@email.com
```

> **Security warning:** Delete the temporary admin account after creating your real one. It has no password and full admin privileges.
> Employees must be removed by ID. On a fresh database the temporary admin has ID 1; use `ls` to confirm.
> ```
> cd Organization/Departments/IT
> ls
> rm 1
> ```

## Troubleshooting

### MySQL Connection Errors
- Verify MySQL service is running: `sudo systemctl status mysql`
- Check database credentials in `config/db_config.h`
- Ensure the `osoem_database` database exists (the name is fixed by `setup.sh`)
- Verify the database user has appropriate permissions

### Build Errors
- Ensure all dependencies are installed: `libmysqlcppconn-dev`, `libreadline-dev`, `libssl-dev`
- Check CMake output for missing library messages
- Verify C++17 support in your compiler

