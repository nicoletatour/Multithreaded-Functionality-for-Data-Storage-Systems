# Multithreaded-Functionality-for-Data-Storage-Systems
Thread‑Safe Storage Engine & Benchmark (OS Assignment)

This repo contains the **files I modified** to add multithreading to a simple key–value storage engine and a **benchmark** tool that measures random **write/read** throughput with multiple threads. Goal: **many readers / one writer** with correct synchronization (mutex + condition variables) and clear timing metrics.

## Files
- **bench.h** — constants (key/value sizes), benchmark structs, global metrics, and synchronization needed by the benchmark.
- **bench.c** — CLI benchmark: parses args, runs `write`, `read`, or `readwrite` modes, spawns threads, measures **sec/op**, **ops/sec**, and total time.
- **kiwi.c** — glue layer between the benchmark and the DB API: opens/closes the DB, distributes work per thread, calls `db_add` / `db_get`, and updates global stats thread‑safely.
- **db.h** — DB struct and API; adds **readers/writer synchronization** fields: `pthread_mutex_t`, two `pthread_cond_t`, `reader_count`, `writing` flag.
- **db.c** — storage engine implementation: initializes sync, makes **writes exclusive** (blocks readers), allows **concurrent reads**, and signals conditions appropriately.


## Design highlights
- **Readers–Writer pattern:** multiple concurrent readers, single exclusive writer.
- **Thread‑safe aggregation:** benchmark threads update shared cost/metrics via a mutex.
- **Streaming results:** prints environment/sizes and consolidated metrics per run/mode.

**Arguments**
- `count` — number of operations
- `threads` — number of benchmark threads
- `writePercent` — % of writes in mixed mode (0–100)
- `random` — if `1`, use random keys (optional)

**Output metrics**
- `sec/op`, **estimated** `ops/sec`, total **elapsed time** (seconds), and the configured sizes.

## Requirements
- POSIX system with **pthreads**.
- C toolchain (`gcc`).

## Notes
- This repository includes **only the files I changed** (`bench.c/.h`, `kiwi.c`, `db.c/.h`). 
- The original project has more sources and a Makefile.
