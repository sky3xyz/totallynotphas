# justfile

set shell := ["bash", "-cu"]

target := "totally_not_phasmophoiba"

build:
    make

debug-build:
    make debug

commit message:
    git add -A
    git commit -m "{{message}}"

run *args:
    just build
    "./{{target}}" {{args}}

debug *args:
    just debug-build
    "./{{target}}" {{args}}

valgrind *args:
    just debug-build
    valgrind \
      --leak-check=full \
      --track-origins=yes \
      "./{{target}}" {{args}}