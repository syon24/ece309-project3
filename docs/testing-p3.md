# Project 3 testing

## Required final check: Linux

The specification requires Linux with AddressSanitizer and
UndefinedBehaviorSanitizer. The local Windows checks below are preliminary.
Use WSL2, a department Linux machine, or a Linux VM with CMake, a C++17 compiler,
and its sanitizer libraries installed.

Open a terminal in the directory containing this project's CMakeLists.txt.
Use a fresh Linux build directory; do not reuse a Windows build cache.

```bash
cmake -S . -B build-linux -DUSE_P2_REFERENCE=OFF -DMH_SANITIZE=ON -DMH_WERROR=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build-linux -j2
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 ctest --test-dir build-linux --output-on-failure
```

A successful run must report all four CTest entries passing:
`p2_regression`, `p3`, `tool_loop`, and `e2e`, with no sanitizer errors.
The build must finish without warnings. The seven CLI sessions are grouped
under the single `e2e` CTest entry. These are acceptance criteria, not a claim
that this Linux run has already happened.

To see the scale measurements or run one area:

```bash
./build-linux/test_p3 reg_
./build-linux/test_p3 ws_
./build-linux/test_p3 calc_
```

The measured 64-bit scale result should be:

```text
10000 tools: buckets=16384 load=0.61035156 longest_chain=6 weak_longest_chain=9000
```

## Try the harness

```bash
./build-linux/miniharness --tools
./build-linux/miniharness --script scripts/tools_demo.script < tests/e2e/tools_demo.input
./build-linux/miniharness --script scripts/escape_attempt.script < tests/e2e/escape_attempt.input
./build-linux/miniharness --no-tools --script scripts/greeting.script < tests/e2e/greeting.input
```

Run these from the project root. The tools demo creates
`workspace/notes/summary.txt`, which is ignored by Git. To repeat the demo from
its original state, remove that generated summary file. The golden test
runner uses temporary workspace copies and does not change the submitted
fixture files.

## Checks completed on October 25, 2026

Platform: Windows, GCC 13.1.0 from the installed CLion toolchain, with CMake
and Ninja. The supplied CMakeLists.txt was used unchanged.
`USE_P2_REFERENCE=OFF`, `MH_WERROR=ON`, and `MH_SANITIZE=OFF` were selected.

| Check | Result |
|---|---|
| Build with -Wall -Wextra -Wpedantic -Werror | Passed; no warnings |
| Supplied P2 regression suite | 14 / 14 passed |
| P3 suite | 29 / 29 passed |
| Supplied tool-loop suite | 9 / 9 passed |
| CLI golden-fixture comparisons | 7 / 7 passed |
| 10,000-tool lookup and pointer stability | Passed |
| Load factor after every insertion | At most 1.0 |
| Final bucket count / load / longest chain | 16,384 / 0.61035156 / 6 |
| Weak-hash longest chain | 9,000 |
| 20,000-term sum | Passed; result 20,000 |
| 5,000 nested parentheses | Passed; result 7 |
| Linux ASan, UBSan, and leak detection | Not run; WSL access denied |

The P3 suite contains 10 registry, 8 workspace, and 11 calculator cases.
Registry tests include duplicate destruction, eventual destruction of owned
tools, direct collisions, resizing, unique hash values, and for_each visiting
every tool exactly once. Workspace tests check both accepted output paths
and explanations for rejected paths. Calculator tests include the exact
division-by-zero error and malformed token vectors that would otherwise
risk stack underflow.

## Limits of the Windows checks

The supplied test helper uses the POSIX `mkdtemp` function. A temporary header
outside this project supplied an equivalent directory-creation helper for
the local Windows tests. It was injected only through local build flags;
none of the supplied files was changed for Windows.

A separate local Python runner executed all seven supplied CLI scenarios:
escape_attempt, greeting, malformed_calls, runaway, split_test, tools_demo,
and tools_list. It used fresh workspace copies, the original input and flag
files, the original expected outputs, and the same error-message
normalization as run_e2e.sh. It also normalized Windows text-mode line
endings before comparison. All exit statuses matched.

The supplied Bash runner itself has not been run under Linux. Windows does
not establish the required Linux sanitizer result or all Linux filesystem
behavior. No successful ASan, UBSan, or leak-check run is claimed.
