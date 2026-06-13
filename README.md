# sologs — simple opinionated logging server

A lightweight HTTP logging server built with C++20 and Drogon. Accepts structured log
entries via HTTP, stores them in SQLite, and provides key-based auth.

## Dependencies

### System packages

| Package | Notes |
|---|---|
| `build-essential` (g++ ≥ 10) | C++20 support required |
| `cmake` | ≥ 3.20 |
| `git` | Needed by CPM/FetchContent to clone dependencies |
| `libssl-dev` | OpenSSL headers + shared libs |
| `libjsoncpp-dev` | Required by Drogon |
| `uuid-dev` | Required by Drogon |
| `zlib1g-dev` | Required by Drogon |

Ubuntu/Debian one-liner:

```bash
sudo apt install build-essential cmake git libssl-dev libjsoncpp-dev uuid-dev zlib1g-dev
```

## Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel "$(nproc)"
```

With tests:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON
cmake --build build --parallel "$(nproc)"
ctest --test-dir build --output-on-failure
```

## Quick start

```bash
# Set a bootstrap admin key on first run
export SOLOGS_BOOTSTRAP_KEY="my-secret-admin-key"

# Start the server
./build/bin/sologs
```

On first launch the server creates `./sologs.sqlite` and `./sologs-auth.sqlite`
and bootstraps an admin key from `$SOLOGS_BOOTSTRAP_KEY`.

## API

All authenticated endpoints require an `Authorization: Bearer <key>` header.

### `GET /health`

Health check. No auth required.

### `POST /auth`

Create a new API key. Requires `Admin` permission.

```json
{
  "name": "my-service",
  "permissions": ["LogWrite"],
  "expires_at": "2026-12-31 23:59:59"
}
```

### `POST /logs`

Submit a log entry. Requires `LogWrite` or `Admin`.

```json
{
  "message": "Something happened",
  "level": "info",
  "source": "my-service"
}
```

### `GET /logs`

Query stored logs. Requires `LogRead` or `Admin`.

| Query param | Description |
|---|---|
| `level` | Filter by level (e.g. `info`, `error`) |
| `source` | Filter by source |
| `since` | Start timestamp (`YYYY-MM-DD HH:MM:SS`) |
| `until` | End timestamp (`YYYY-MM-DD HH:MM:SS`) |
| `limit` | Max results (default: all) |


## License

MIT
