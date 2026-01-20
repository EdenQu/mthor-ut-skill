# UT Skill - Professional Unit Test Workflow

A comprehensive unit test generation skill for iOS, Android, and C++ platforms.

## Quick Start

Simply type one of these commands in the chat:

```bash
# Generate UT for all changed files (compared to develop branch)
ut_changed

# Generate UT for changed files compared to specific branch
ut_changed --base=main

# Generate UT for all files in a directory
ut_path --path=phone/ios/Phone/Service

# Generate UT for specific files
ut_files --files="PhoneService.swift,CallManager.swift"
```

## Commands

### `ut_changed`

Analyzes git changes and generates unit tests for modified files.

```bash
ut_changed [--base=<branch>] [--platform=<ios|android|cpp|all>]
```

| Parameter | Default | Description |
|-----------|---------|-------------|
| `--base` | `origin/dev` | Base branch for comparison (uses remote to avoid stale local) |
| `--platform` | `all` | Filter by platform |

**Examples:**
```bash
ut_changed
ut_changed --base=origin/dev
ut_changed --base=origin/main --platform=ios
```

### `ut_path`

Generates unit tests for all source files in a directory.

```bash
ut_path --path=<directory> [--platform=<ios|android|cpp|all>]
```

| Parameter | Required | Description |
|-----------|----------|-------------|
| `--path` | Yes | Directory path to scan |
| `--platform` | No | Filter by platform |

**Examples:**
```bash
ut_path --path=phone/ios/Phone/Service
ut_path --path=video/android/module-video/src/main/java/com/glip/video
ut_path --path=phone/cpp/core-phone/src --platform=cpp
```

### `ut_files`

Generates unit tests for specific files.

```bash
ut_files --files="<file1,file2,...>" [--platform=<ios|android|cpp|all>]
```

| Parameter | Required | Description |
|-----------|----------|-------------|
| `--files` | Yes | Comma-separated file list |
| `--platform` | No | Filter by platform |

**Examples:**
```bash
ut_files --files="UserService.swift,AuthManager.swift"
ut_files --files="PhoneService.kt" --platform=android
```

## Workflow

The skill executes the following workflow:

### 1. File Analysis
- Detects changed files (local + branch diff)
- Filters by platform
- Excludes test files and non-testable files

### 2. Path Mapping
- Maps source files to test file locations
- Checks if test file already exists

### 3. Pre-Implementation
- Reads source file for analysis
- Finds similar tests for reference
- Identifies existing test coverage

### 4. Test Generation
- Generates tests following platform rules
- Achieves 100% branch coverage
- Achieves 100% code coverage
- Updates existing tests (no duplicates)

### 5. Build (iOS only)
- Prompts user to choose:
  - `make ios_install_with_source`
  - `make ios_install_with_binary_cache`

### 6. Test Execution
- Runs generated tests
- Collects coverage data

### 7. Coverage Report
- Displays line coverage
- Displays branch coverage
- Identifies missing coverage

## Coverage Requirements

All generated tests must achieve:
- ✅ **100% Line Coverage**
- ✅ **100% Branch Coverage**

## Platform Rules

The skill follows platform-specific testing rules:

| Platform | Rule File |
|----------|-----------|
| iOS | `.cursor/rules/unit-test/ios-unit-test.mdc` |
| Android | `.cursor/rules/unit-test/android-unit-test.mdc` |
| C++ | `.cursor/rules/unit-test/cpp-unit-test.mdc` |
| Common | `.cursor/rules/unit-test/unit-testing-principles.mdc` |

## Test Path Conventions

### iOS
```
Source: phone/ios/Phone/Service/PhoneService.swift
Test:   phone/ios/Phone/Tests/PhoneServiceTests.swift
```

### Android
```
Source: phone/android/module-phone/src/main/java/.../PhoneService.kt
Test:   phone/android/module-phone/src/test/java/.../PhoneServiceTest.kt
```

### C++
```
Source: phone/cpp/core-phone/src/cc_lt/live_transcript_service.cpp
Test:   phone/cpp/core-phone/tests/src/cc_lt/live_transcript_service_test.cpp
```

## Base Test Classes

| Platform | Base Class | Location |
|----------|------------|----------|
| iOS | `XCBaseTestCase` | `common/ios/Common/UnitTestCommon/` |
| Android | `BaseRobolectricTest` | Module test directories |
| C++ | `InitGlipCoreTest` | `*/tests/ut_base/` |

## Templates

Reference templates are available in:
- `templates/ios-test-template.swift`
- `templates/android-test-template.kt`
- `templates/cpp-test-template.cpp`

## File Structure

```
.cursor/skills/ut-skill/
├── README.md           # This file
├── skill.md            # Skill overview
├── workflow.md         # Detailed workflow steps
├── trigger.mdc         # Command trigger rule
├── prompts/
│   ├── ut_changed.md   # ut_changed command prompt
│   ├── ut_path.md      # ut_path command prompt
│   └── ut_files.md     # ut_files command prompt
└── templates/
    ├── ios-test-template.swift
    ├── android-test-template.kt
    └── cpp-test-template.cpp
```

## Key Principles

1. **No Duplicates**: Never create duplicate test files or methods
2. **Update Mode**: Modify existing tests rather than recreating
3. **Reference Patterns**: Use similar tests in the module as reference
4. **Follow Standards**: Strictly follow platform testing guidelines
5. **Complete Coverage**: Test all branches and code paths
