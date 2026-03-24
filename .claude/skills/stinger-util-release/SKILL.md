---
name: stinger-util-release
description: |
  Prepares stinger-cpp-utils for release: runs clang-format, compiles, runs unit tests, optionally bumps the version in CMakeLists.txt, builds a commit message from the diff, and pushes to origin. Use this skill whenever the user wants to release, publish, cut a version, prepare for release, or ship stinger-cpp-utils — even if they just say "release it" or "let's ship".
---

# stinger-cpp-utils Release Workflow

Walk through these steps in order. If any step fails (formatting issues that can't be auto-fixed, compilation errors, test failures), stop immediately and tell the user what went wrong — do not continue the release.

---

## Step 1 — Run clang-format

Format all C++ source and header files using the project's `.clang-format` config:

```bash
find src include -name '*.cpp' -o -name '*.hpp' | xargs clang-format -i
```

After formatting, check if anything changed:

```bash
git diff --stat
```

If files were modified, tell the user which files were reformatted. These changes will be included in the release commit (the user will get a chance to review the full diff before committing).

---

## Step 2 — Compile

Make sure the project builds cleanly. If no `build/` directory exists, configure first:

```bash
cmake -B build -DSTINGER_UTILS_BUILD_TESTS=ON -DSTINGER_UTILS_BUILD_MOCK=ON
```

Then build:

```bash
cmake --build build
```

If compilation fails, ask if the user would like to stop or have the agent automatically attempt to fix the errors. If they choose to fix, analyze the compiler output, identify the errors, and attempt to correct them in the source code. After making changes, try building again. Repeat this process until it either builds successfully or the user decides to stop.

---

## Step 3 — Run unit tests

```bash
cd build && ctest --output-on-failure && cd ..
```

If any test fails, **stop and report the failures**. Do not proceed with failing tests.

---

## Step 4 — Version bump

Read the current version from the `project()` directive in `CMakeLists.txt`. It follows the pattern:

```cmake
project(StingerUtils VERSION X.Y.Z LANGUAGES CXX)
```

Show the user the current version and ask which component to bump:
- **Major** (X) — breaking changes
- **Minor** (Y) — new functionality, backwards-compatible
- **Patch** (Z) — bug fixes (most common)
- **Skip** — no version bump this release

If they choose to bump, edit the `project()` line in `CMakeLists.txt` with the new version. Use a precise string replacement — only change the VERSION value on that specific line.

After bumping, rebuild to make sure the version change didn't break anything:

```bash
cmake --build build
```

---

## Step 5 — Review changes and build commit message

Show the user the full diff of what will be committed:

```bash
git diff
git diff --cached
git ls-files --others --exclude-standard
```

Present these and let the user choose which files to stage. Do **not** automatically `git add -A` — let the user decide. Suggest staging all changed files as a reasonable default, but respect their choice.

Then examine the staged diff to write a commit message. Look at what actually changed — the formatted files, the version bump, any code changes that were already in the working tree. Write a clear, concise commit message following the project's existing convention (lowercase imperative, like the existing history: "Add function to hash a string", "fix timestring parsing").

Present the commit message to the user and ask them to approve or edit it before committing.

---

## Step 6 — Commit

Once the user approves the commit message:

```bash
git commit -m "<approved message>"
```

---

## Step 7 — Push

Ask the user if they'd like to push to origin. If yes:

```bash
git push origin HEAD
```

If they decline, let them know the commit is local and they can push later when ready.
