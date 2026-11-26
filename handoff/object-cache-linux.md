# Object cache on Linux

Test changes `wmuwlysv` (deterministic order of polymorphic instances) and `qpzooluq` (object cache) from the Odin
checkout on a native Linux host or the Steam Deck.

The cache uses POSIX `open`, `flock` and `rename` on Linux exactly as on macOS, where it was tested. What needs
native proof is that the lock and the rename behave the same on Linux file systems, and that GNU ld or lld links the
objects from the cache folder.

Build the compiler:

```sh
./build_odin.sh release-native
```

Build the demo twice with the cache, then run it:

```sh
rm -rf /tmp/odin-object-cache && mkdir /tmp/odin-object-cache
ODIN_OBJECT_CACHE=1 ./odin build examples/demo -o:minimal -debug -out:/tmp/odin-object-cache/demo
ODIN_OBJECT_CACHE=1 ./odin build examples/demo -o:minimal -debug -out:/tmp/odin-object-cache/demo
/tmp/odin-object-cache/demo
ls -a /tmp/odin-object-cache/.odin-objects/demo
```

Expected:

- The first build prints `object cache: 0 reused, N generated`.
- The second build prints `object cache: M reused, K generated`, with K at most 1 (the `builtin` module, whose debug
  information holds the compile time).
- The demo runs as it does without the cache.
- The folder holds N `.o` files and `.lock`, and no `.tmp` files.

Run three builds of the same output at once:

```sh
for i in 1 2 3; do (ODIN_OBJECT_CACHE=1 ./odin build examples/demo -o:minimal -debug -out:/tmp/odin-object-cache/demo 2>&1 | grep -c "object cache") & done; wait
```

All three must succeed, and exactly one prints `1` (the build that held the lock); the others print `0`.

A failure looks like a link error about a missing object, a crash in the demo, `.tmp` files left in the cache folder,
or more than one concurrent build printing `1`.

Then build a dev profile from the monorepo twice, which sets `ODIN_OBJECT_CACHE=1` for dev builds:

```sh
./tool build vizza
./tool build vizza
```

The second build must print `object cache:` with all modules but at most one reused, and VIZZA must run as before.
