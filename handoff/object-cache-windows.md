# Object cache on Windows

Test changes `wmuwlysv` (deterministic order of polymorphic instances) and `qpzooluq` (object cache) from the Odin
checkout in a Visual Studio x64 Native Tools prompt.

On Windows the object cache is compiled out: `ODIN_OBJECT_CACHE=1` must change nothing. What needs native proof is
that the compiler still builds with `-W4 -WX`, because the cache code uses POSIX `open` and `flock` behind
`#if !defined(GB_SYSTEM_WINDOWS)`.

Build the compiler:

```bat
build.bat 1
```

It must build with no warnings. A failure looks like an error in `src\llvm_backend.cpp` about `fcntl.h`,
`sys\file.h`, `open`, `flock`, or unreachable code (C4702).

Build and run the demo twice with the cache requested:

```bat
set ODIN_OBJECT_CACHE=1
odin build examples\demo -o:minimal -debug -out:demo_cache.exe
odin build examples\demo -o:minimal -debug -out:demo_cache.exe
demo_cache.exe
set ODIN_OBJECT_CACHE=
```

Both builds must succeed and the demo must run. Neither build may print a line that starts with `object cache:`, and
no `.odin-objects` folder may appear next to `demo_cache.exe`.

Then build VIZZA's dev profile from the monorepo, which sets `ODIN_OBJECT_CACHE=1` for dev builds:

```bat
.\tool.bat build vizza
```

It must build as before, with no `object cache:` line.
