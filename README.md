# Mars Arena Bot

Warcraft III game hosting bot based on [GHost++](https://github.com/uakfdotb/ghostpp).
Compiled natively for Linux (Fedora 43) — no Wine required.

## Docker

On push to `main`, GitHub Actions builds the Docker image and pushes it to GHCR.

```bash
# Build locally
docker build -t mars-arena-bot .

# Run with custom config and maps
docker run -d \
  -v ./ghost.cfg:/ghost/ghost.cfg \
  -v ./maps:/ghost/maps \
  -v replays:/ghost/replays \
  -p 6112:6112/udp \
  ghcr.io/mars-arena/mars-arena-bot:latest
```

## Configuration

GHost++ reads `default.cfg` first (baked into the image), then `ghost.cfg` (mount at runtime).

Key settings for `ghost.cfg`:

| Setting | Default | Description |
|---------|---------|-------------|
| `bot_hostport` | `6112` | UDP port for LAN broadcast |
| `bot_maxgames` | `5` | Max simultaneous games |
| `bot_latency` | `100` | Game latency in ms |
| `bot_mappath` | `maps` | Path to map files |
| `bot_mapcfgpath` | `mapcfgs` | Path to map configs |
| `bot_replaypath` | `replays` | Path to replay files |
| `db_type` | `sqlite3` | Database backend (`sqlite3` or `mysql`) |
| `db_sqlite3_file` | `ghost.dbs` | SQLite database file |
| `bot_commandtrigger` | `!` | Chat command prefix |

Full reference: see `default.cfg` and `MANUAL` in the repo.

## Structure

```
ghost/          — GHost++ C++ source code
bncsutil/       — Battle.net login utilities (library)
StormLib/       — MPQ archive reader (library)
CascLib/        — CASC file reader for WC3 1.29+ (library)
mapcfgs/        — Map configuration files
default.cfg     — Base configuration (do not edit, override in ghost.cfg)
language*.cfg   — Localization files
```

## Upstream

Fork of [uakfdotb/ghostpp](https://github.com/uakfdotb/ghostpp). See `MANUAL` for the original GHost++ documentation.
