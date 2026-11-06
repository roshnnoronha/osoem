# OSOEM Agents Setup

## Prerequisites

- Linux or WSL (Windows Subsystem for Linux)
- CMake 3.14 or higher
- C++17 compatible compiler (g++)
- A running Osoem database (see [`../database/README.md`](../database/README.md))
- An OpenAI API key
- Required libraries: `libmysqlcppconn-dev`, `libcurl4-openssl-dev`, `libreadline-dev`

Install the dependencies with:

```bash
sudo apt install cmake g++ libmysqlcppconn-dev libcurl4-openssl-dev libreadline-dev
```

The nlohmann/json library is downloaded automatically by CMake during the first build.

## Layout

```
agents/
  config/       — db_config.h and api_config.h (shared by all agents)
  include/      — code shared by all agents: database, OpenAI client, exceptions, utils
  planner/      — Osoem Planner Agent
  reviewer/     — Osoem Reviewer Agent
  utilities/    — supporting tools (PDF to Markdown conversion)
```

Each agent has its own `CMakeLists.txt` and is built separately from its own folder.

## Setup

### 1. Configure the database and API connections

Copy the config templates from the `agents/` directory:

```bash
cp config/db_config.h.example config/db_config.h
cp config/api_config.h.example config/api_config.h
```

Edit `config/db_config.h` with your MySQL connection details:

```cpp
const std::string DB_HOST = "localhost";
const std::string DB_USER = "<username>";
const std::string DB_PASS = "<password>";
const std::string DB_NAME = "osoem_database";
const int DB_PORT = 3306;
```

Edit `config/api_config.h` with your OpenAI API key. The model and URLs can usually be left as they are:

```cpp
const std::string OPENAI_API_KEY = "sk-...";
const std::string OPENAI_MODEL   = "gpt-4o";
const std::string OPENAI_API_URL = "https://api.openai.com/v1/chat/completions";
const std::string OPENAI_EMBEDDING_MODEL = "text-embedding-3-small";
const std::string OPENAI_EMBEDDING_URL   = "https://api.openai.com/v1/embeddings";
```

Both files are listed in `.gitignore` so your credentials are not committed. The config values are compiled into the programs, so rebuild an agent after changing them.

### 2. Build the Planner Agent

From the `agents/planner/` directory:

```bash
cmake -S . -B build
cmake --build build
```

This compiles the `osoem_planner` executable to `build/osoem_planner`. Run it with:

```bash
./build/osoem_planner
```

### 3. Build the Reviewer Agent

From the `agents/reviewer/` directory:

```bash
cmake -S . -B build
cmake --build build
```

This compiles the `osoem_reviewer` executable to `build/osoem_reviewer`. The reviewer loads its specifications from a `specs/` folder relative to the current directory, so run it from `agents/reviewer/`:

```bash
./build/osoem_reviewer
```

Specifications are Markdown files placed in `reviewer/specs/`. On startup the reviewer indexes every `.md` file directly in that folder (subfolders are skipped) using the OpenAI embeddings API, so startup time and API usage grow with the size of the specifications.

To convert PDF specifications to Markdown, place them in `reviewer/specs/pdf/` and run:

```bash
pip install pymupdf
python3 specs/convert_pdfs.py
```

The converted files are written to `reviewer/specs/markdown/`. Move the ones you want indexed into `reviewer/specs/`.

## Troubleshooting

### `MySQL Connector/C++ not found` or `Readline not found`
- The development packages are missing. Install them with the `apt` command in [Prerequisites](#prerequisites).

### `api_config.h: No such file or directory`
- The config files have not been created. Follow [step 1](#1-configure-the-database-and-api-connections).

### MySQL connection errors
- Verify MySQL service is running: `sudo systemctl status mysql`
- Check the credentials in `config/db_config.h`, then rebuild the agent.

### `SpecIndexer: specs directory not found: specs`
- The reviewer was started from the wrong directory. Run it from `agents/reviewer/`.

### `SpecIndexer: no chunks found in specs`
- `reviewer/specs/` contains no `.md` files.
