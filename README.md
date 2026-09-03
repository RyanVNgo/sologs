> [!NOTE]
> **Archived — no longer maintained.**
>
> This project was primarily developed to learn/explore/improve at various concepts
> and technologies. Namely:
> 
> - project/file structure
> - cmake build configurations
> - test friendly design
> - API design
> - HTTP frameworks
> - containerized testing
> - benchmarking
>
> I wanted to improve at software development through experimentation and practice so
> most of these targets had no formal goal and some only came up as development went on.
> My current practices have since evolved, so this repository no longer fully represents
> my approach to software development and I no longer see fit to update/maintain this.
>
> Ultimately, this code is preserved as a record of my proficiency at the time.

# sologs

A HTTP logging server built in C++20 with Drogon and SQLite. Accepts structured log
entries via HTTP and provides key-based auth.

## Build

```bash
cmake --preset release-ninja
cmake --build --preset sologs-release-ninja
```

## Quick start

```bash
# Set a bootstrap admin key on first run
export SOLOGS_BOOTSTRAP_KEY="my-secret-admin-key"

# Start the server
./build/release/bin/sologs
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

### `GET /auth`

List API keys with optional filters. Requires `AuthRead` or `Admin` permission.

| Query param | Description |
|---|---|
| `uuid` | Filter by key UUID |
| `name` | Filter by key name |
| `permissions` | Comma-separated permission list (e.g. `LogWrite,LogRead`) |
| `created_after` | Filter by creation timestamp (`YYYY-MM-DD HH:MM:SS`) |
| `created_before` | Filter by creation timestamp (`YYYY-MM-DD HH:MM:SS`) |
| `expires_after` | Filter by expiration timestamp (`YYYY-MM-DD HH:MM:SS`) |
| `expires_before` | Filter by expiration timestamp (`YYYY-MM-DD HH:MM:SS`) |
| `is_valid` | `true` (default) or `false` — includes/excludes expired keys |
| `limit` | Max results (default: 100) |

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
