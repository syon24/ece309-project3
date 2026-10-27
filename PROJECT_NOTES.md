# Project 3: changes and submission guide

This directory follows the professor's starter layout. Start here, then read
[the design log](docs/design-log-p3.md) and [the testing notes](docs/testing-p3.md).
Your original Downloads folders were not edited.

## What changed

| File | Work completed |
|---|---|
| `include/tools/tool_registry.h` | Replaced the private placeholder with an Entry structure, vector buckets, an element count, and a rehash helper. Public declarations are unchanged. |
| `src/tools/tool_registry.cpp` | Implemented separate chaining, the Appendix A string hash, insertion, duplicate rejection, lookup, iteration, resizing, and measurements. Starts with 16 buckets and doubles before exceeding load 1.0. |
| `src/tools/workspace.cpp` | Implemented the six specified lexical checks. Rejects empty, NUL-containing, rooted, and escaping paths; returns normalized paths inside the workspace without filesystem access. |
| `src/tools/calculator_eval.cpp` | Implemented iterative two-stack evaluation, precedence, left associativity, parentheses, operand order, division-by-zero errors, and checks against stack underflow. |
| `tests/p3/test_p3.cpp` | Kept the five starter examples and added 24 tests, for 29 total. Covers every required workspace/calculator row, ownership, collisions, resizing, stable pointers, iteration, 10,000 tools, weak-hash measurements, and long/deep expressions. |
| `docs/design-log-p3.md` | Filled in the required 500–800-word log with design decisions, actual local measurements, a stack trace, error handling, and the required AI-use disclosure. |

## Files copied from your Project 2

These four files are byte-for-byte copies of your supplied Project 2 code:

- `include/core/conversation.h`
- `include/core/sentinel_scanner.h`
- `src/core/conversation.cpp` (from P2's `src/conversation.cpp`)
- `src/core/sentinel_scanner.cpp` (from P2's `src/sentinel_scanner.cpp`)

The build uses these files with `USE_P2_REFERENCE=OFF`. P3's provided
`message.h` is retained so messages can use `Role::Tool`.

## Other new files

- `PROJECT_NOTES.md`: this change list and the submission steps.
- `docs/testing-p3.md`: test results, limitations, and Linux commands.
- `.gitignore`: excludes generated builds, executables, demo output, and the
  eventual submission wrappers.
- `.gitattributes`: requests LF line endings in Git checkouts, including the
  Linux shell script and fixtures whose byte counts matter.

All other starter files are unchanged, including CMakeLists.txt, README.md,
P3_spec.md, the provided harness, JSON code, tools, tokenizer, reference code,
test framework, regression tests, loop tests, scripts, and golden outputs.
No build products or temporary Windows adapters are included here.

## Verification status

The local GCC build completed with `-Wall -Wextra -Wpedantic -Werror`.
All 14 provided P2 tests, 29 P3 tests, 9 tool-loop tests, and 7 CLI fixture
comparisons passed.

**The required Linux ASan/UBSan run is still outstanding.** WSL returned
`Wsl/E_ACCESSDENIED` in the editing session. Windows checks used a temporary
adapter for the test framework's POSIX temporary-directory function, with
sanitizers disabled. See [testing notes](docs/testing-p3.md) for the exact
distinction and the Linux commands.

## Before submitting

1. Read the implementation and review the design log. Its wording was drafted
   with Codex, as disclosed at the top. Revise it to reflect your understanding
   and be ready to explain the choices during the oral check-in.
2. Run the Linux build, sanitizers, and full CTest suite in
   [testing notes](docs/testing-p3.md). Resolve any failures before submission.
3. Create an empty GitHub repository for Project 3. Use the course's required
   visibility and access settings. Do not initialize it with a separate README
   if using the commands below.
4. From this directory, upload the project using Git. Replace the example URL
   with the URL of the repository you actually created:

   ```bash
   git init
   git add .
   git commit -m "Complete ECE 309 Project 3"
   git branch -M main
   git remote add origin https://github.com/YOUR-USERNAME/ece309-project3.git
   git push -u origin main
   ```

5. Check that the repository contains CMakeLists.txt, include/, src/, tests/,
   docs/, reference/, scripts/, and workspace/. Include the supplied files,
   including files under reference/, even though the reference is not selected.
6. When you are ready, create `github.txt` separately. Its entire contents
   should be the direct repository URL, for example
   `https://github.com/YOUR-USERNAME/ece309-project3`, with your actual username
   and repository name. Do not use this example as the submitted URL.
7. After committing every final change, create the ZIP backup outside the
   repository directory:

   ```bash
   git archive --format=zip --prefix=ece309-project3/ --output=../github.zip HEAD
   ```

   This packages the complete committed project, including the supplied
   reference and test files, without generated builds. The ZIP contains the
   committed version: commit any later edits and regenerate it.

Neither `github.txt` nor `github.zip` has been created, and nothing has been
uploaded to GitHub.
