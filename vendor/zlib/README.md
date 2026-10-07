# Windows zlib archive

`libz.lib` contains x64 COFF objects built from the unmodified zlib 1.2.12 sources.
It uses the static C runtime (`/MT`) and disables MSVC whole-program optimization (`/GL-`).
RAD Linker does not support MSVC LTCG objects. The previous archive's `compress.obj`
contained LTCG data, so using `compressBound` together with `compress2` failed to link.
Do not rebuild this archive with `/GL` or substitute an archive built with `/MD`.

Source: <https://zlib.net/fossils/zlib-1.2.12.tar.gz>

SHA-256: `91844808532e5ce316b3c010929493c0244f3d37593afd6de04f71821d5136d9`

From an x64 Visual Studio developer prompt, run:

```bat
build_windows.bat PATH_TO_EXTRACTED_ZLIB_1_2_12
```

The committed archive was built with MSVC 19.44.35207. Objects carry embedded debug
information (`/Z7`) so consumers do not need a separate zlib PDB.

Verify the default linker and release code generation from the Odin checkout:

```text
odin test vendor:zlib
odin test vendor:zlib -o:speed -disable-assert
```

These tests exercise compression bounds, compression, decompression, and CRC through
the real native library. They must link and execute before replacing the archive.
