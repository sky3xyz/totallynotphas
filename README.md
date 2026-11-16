# totallyNotPhas

This was my final project for my Introduction to Systems Programming (COMP 2401) Class. 
A Phasmophobia-inspired ghost-hunting simulation written in C. A ghost picks one of 24 types and haunts a 24-room house while a team of hunters wanders the rooms with their equipment, collecting evidence. Three matching pieces of evidence identify the ghost; hunters can also leave early by getting scared or bored. Results are tallied in the van at the end of the hunt.


## How it works

- The ghost and every hunter run as **pthreads** sharing one in-memory house: rooms, their connections, and the evidence left in each room.
- Every hunter carries one device (EMF reader, thermometer, spirit box, etc.) and moves between connected rooms, leaving when their fear or boredom threshold is reached. Evidence they collect is shared with the rest of the team.
- The simulation ends when all hunters are done. The van (casefile) reviews the shared evidence, infers the ghost type, and prints per-hunter and per-room results.
- For extra credit, a **multi-process variant** (`multi_simulation`) uses the same code with a different entry point, that spawns child simulations via `execv` and aggregates results across runs.

## Architecture

| Module | Responsibility |
|---|---|
| `headers/defs.h` | Shared types and constants: rooms, evidence, 24 ghost types |
| `src/house.c`, `room.c`, `roomStack.c` | House model: the room graph and traversal |
| `src/ghost.c` | Ghost thread: spawns evidence, wanders the house |
| `src/hunter.c`, `hunterArray.c` | Hunter threads and their collection management |
| `src/evidence.c`, `casefile.c` | Evidence handling and the end-of-hunt review |
| `src/deadlock.c`, `helpers.c` | Synchronization and shared utilities |
| `src/main.c` | Threaded simulation entry point |
| `src/main_multi.c` | Multi-process runner entry point |

It runs concurrently. Hunter threads can race for room occupancy, evidence is produced and consumed concurrently, and the whole thing is synchronized with mutexes and semaphores (see `defs.h`). It was tested by compiling with the `-fsanitize=thread` gcc flag for races, and `valgrind` with the `--leak-check=full` and `--show-leak-kinds=all` flags for leaks.

## Build and run

```bash
make                        # build both executables
./totally_not_phasmophoiba  # run one threaded simulation
```

```bash
./totally_not_phasmophoiba [-report N]   # one simulation. -report N runs it in automated mode with N hunters
./multi_simulation NSIMS [NHUNTERS]      # run NSIMS child simulations (minimum 30) and aggregate results
```

Extras:

```bash
make debug    # build with -g for gdb / valgrind
make clean    # remove objects, executables, and logs
```

A `justfile` is included with the same targets plus valgrind recipes (`just debug-build`, `just dbv`).

## Testing

- `misc/tester.py` - Python script that builds, runs, and checks simulations
- `misc/validate_logs.py` - validates the per-run CSV logs against the project spec
- Regular race condition and memory leak testing during development
