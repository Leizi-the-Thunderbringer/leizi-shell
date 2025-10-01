# Repository Guidelines

## Project Structure & Module Organization
Core shell logic sits in `src/main.cpp`; keep the REPL lean by moving stable helpers into `src/completion/`, `src/prompt/`, or `src/builtin/` as they mature. Build outputs belong in `build/` (CMake) or IDE-generated `cmake-build-*`. Scripts like `install.sh` and `test.sh` stay at the repository root. CI definitions live in `.github/workflows/` and mirror the local workflow.

## Build, Test, and Development Commands
- `cmake -B build -DCMAKE_BUILD_TYPE=Debug` configures an out-of-source tree with compile commands.
- `cmake --build build --parallel` compiles the binary; switch to `-DCMAKE_BUILD_TYPE=Release` for benchmarks.
- `./build/leizi` runs the shell; `./test.sh` performs the smoke tests used in CI.
- `ctest --test-dir build` (or `cmake --build build --target test`) executes any CTest cases you register.
- `cmake --install build --prefix dist` prepares a staging directory when packaging releases.

## Coding Style & Naming Conventions
Follow the C++20 guidance in `CONTRIBUTING.md`: four-space indentation, `PascalCase` classes, `camelCase` functions and variables, `UPPER_CASE` constants, and trailing underscores for private data when clarity is needed. Prefer RAII, `const` correctness, and standard smart pointers; avoid raw allocation in new code. Keep prompt/completion helpers focused and header-backed. Before pushing, spot-check with `clang-tidy src/main.cpp -p build` and `cppcheck --enable=all --std=c++20 src/` to match CI expectations.

## Testing Guidelines
Extend automated coverage alongside features. Update `test.sh` with new commands only if they stay snappy (<5s) and deterministic. For richer scenarios, create fixtures under `tests/` and register them through `add_test(...)` so they run via `ctest`. Document any manual verification steps in the PR description and capture edge cases around variables, job control, and prompt rendering.

## Commit & Pull Request Guidelines
Use Conventional Commits (`feat:`, `fix:`, `docs:`, etc.), mirroring examples such as `chore: update .gitignore to exclude .env and .mcp.json files`. Summaries stay imperative and under 72 characters; use bodies for rationale, breaking changes, or follow-up tasks. PRs should link issues when relevant, summarise the behavior change, list local test commands, and mention UX or risk notes. Do not request review until Linux and macOS CI checks succeed.

## Security & Runtime Tips
Assume user input is hostile: validate paths, escape expansions, and guard process spawning with clear ownership of foreground PIDs. Avoid committing secrets—`.env` is already ignored—and prefer environment variables for local overrides. When touching signal handling or job control, rerun interactive smoke tests to confirm interrupts and exits behave as expected.
