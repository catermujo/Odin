# SOA enum indexing on Windows

Test change `qszvnkxk` from the Odin checkout in a Visual Studio x64 Native Tools prompt.

Build the compiler:

```bat
build.bat 1
```

Run both SOA test builds:

```bat
odin test tests\issues\test_issue_soa_enumerated_array_field_projection.odin -file -define:ODIN_TEST_FANCY=false
odin test tests\issues\test_issue_soa_enumerated_array_field_projection.odin -file -o:speed -define:ODIN_TEST_FANCY=false
```

Each build must pass all 11 tests. The tests cover enum selectors, storage offsets, slice rows, and bounds checks.

Run the rejection test:

```bat
odin test tests\issues\test_issue_soa_enum_index_reject.odin -file
```

This command must exit with code 1 and report one enum-type error.

Run the internal suite:

```bat
odin test tests\internal -all-packages -define:ODIN_TEST_FANCY=false
```

All 303 tests must pass. A compiler crash, wrong result, or extra diagnostic is a failure.

Windows execution is not verified on the macOS host.
