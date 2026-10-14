import sys

from llvm_error_cleanup import run_native_llvm_test

run = run_native_llvm_test("thread_pool_publication.cpp")
sys.stdout.write(run.stdout)
sys.stderr.write(run.stderr)
assert run.returncode == 0, run.returncode
assert "Thread pool publication accounting passed" in run.stdout, run.stdout
