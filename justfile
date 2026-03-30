# justfile

set shell := ["bash", "-cu"]

target := "totally_not_phasmophoiba"

build:
    make

debug-build:
    make debug

commit message:
    make clean
    git add -A
    git commit -m "{{message}}"

push:
    git push origin main

run *args:
    just build
    "./{{target}}" {{args}}

debug:
    just debug-build
    "./{{target}}"

debug-a *args:
    just debug-build
    "./{{target}}" {{args}}

valgrind *args:
    just debug-build
    valgrind \
      -s \
      --leak-check=full \
      --track-origins=yes \
      --show-leak-kinds=all \
      "./{{target}}" {{args}}

dbv:
    just debug-build
    just valgrind

test:
    rm -rf log_*.csv
    python ./misc/tester.py
    python ./misc/validate_logs.py
