---
name: mthor-ut-skill
description: Professional Unit Test generation skill for iOS, Android, and C++ with auto error fixing and reference-first workflow
version: 1.0.0
author: eden_qu
tags:
  - unit-test
  - testing
  - ios
  - android
  - cpp
  - swift
  - kotlin
  - test-generation
  - code-quality
requires: []
---

# MThor UT Skill

Professional Unit Test generation skill for Cursor IDE supporting iOS (Swift/XCTest), Android (Kotlin/JUnit), and C++ (GoogleTest/GMock).

## Overview

This skill automates unit test generation with:
- Multi-platform support (iOS, Android, C++)
- 8-type compilation error classification and auto-fix
- Reference-first workflow (learn from existing passing tests)
- 100% branch and code coverage target
- Smart partial coverage for changed methods only

## When to Use

Use this skill when you need to:
- Generate unit tests for changed files in a git branch
- Create comprehensive tests for a specific directory
- Add tests for specific source files
- Ensure 100% test coverage for your code

## Commands

| Command | Description | Example |
|---------|-------------|---------|
| `ut_changed` | Generate UT for git changed files | `ut_changed --base=origin/dev` |
| `ut_path` | Generate UT for files in specified path | `ut_path --path=phone/ios/Phone/Service` |
| `ut_files` | Generate UT for specific files | `ut_files --files="UserService.swift,AuthManager.swift"` |
| `ut_run` | Run all changed UT files in current branch | `ut_run --base=origin/dev` |

## Process

### Phase 1: File Analysis
- Detect changed files using git diff
- Filter by platform (iOS/Android/C++)
- Map source files to test file locations

### Phase 2: Reference Existing Tests
- **CRITICAL**: Always search existing passing tests first
- Copy exact patterns for imports, mocks, stubs
- Match naming conventions from similar tests

### Phase 3: Test Generation
- Generate tests following AAA pattern (Arrange-Act-Assert)
- Cover all branches and edge cases
- For `ut_changed` with no existing UT: only cover changed methods

### Phase 4: Compilation & Fix
- Compile test files to verify syntax
- Auto-classify and fix 8 types of compilation errors:
  1. Module Not Found → Add import
  2. Type Not Found → Fix type name
  3. Member Not Found → Check API
  4. Method Signature → Fix params
  5. Mock Issues → Fix stub syntax
  6. Access Control → Use @testable
  7. Protocol Conformance → Implement methods
  8. Type Mismatch → Fix type cast

### Phase 5: Run & Report
- Execute tests and verify all pass
- Generate coverage report

## Output Format

```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
📊 Coverage Report for <ClassName>
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
📁 File: <file_path>
📈 Line Coverage:   XX.X% (XX/XX lines)
🌿 Branch Coverage: XX.X% (XX/XX branches)
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
✅ Meets 100% coverage requirement: [YES/NO]
```

## Examples

### Example 1: Generate tests for changed files

```
User: ut_changed --base=origin/dev

AI: Analyzing changes against origin/dev...
Found 5 changed files:
- UserService.swift (iOS)
- AuthManager.kt (Android)
- config_parser.cpp (C++)

Generating tests with 100% coverage...
[Progress tracking displayed]
All tests passed! Coverage: 100%
```

### Example 2: Generate tests for a directory

```
User: ut_path --path=phone/ios/Phone/Telephony

AI: Scanning directory...
Found 12 source files.
Checking existing tests...
3 files need new tests, 5 files need updated tests.

Generating tests...
[Compilation error detected]
Fixing: Type 'MockCallService' not found
→ Searching passing tests for correct pattern...
→ Found in CallManagerTests.swift, applying fix...

All tests passed!
```

## Platform-Specific Templates

### iOS (Swift/XCTest)
```swift
import XCTest
@testable import YourModule

final class ServiceTests: XCBaseTestCase {
    private var sut: Service!
    
    override func setUp() {
        super.setUp()
        sut = Service()
    }
    
    func test_methodName_scenario_expectedResult() {
        // Arrange
        // Act
        // Assert
    }
}
```

### Android (Kotlin/JUnit)
```kotlin
@RunWith(RobolectricTestRunner::class)
class ServiceTest : BaseRobolectricTest() {
    private lateinit var sut: Service
    private val mockDep: Dependency = docker()
    
    @Test
    fun testMethodName_scenario_expectedResult() {
        // Arrange
        // Act
        // Assert
    }
}
```

### C++ (GoogleTest)
```cpp
class ServiceTest : public InitGlipCoreTest {
protected:
    std::shared_ptr<Service> m_sut;
};

TEST_F(ServiceTest, methodName_scenario_expectedResult) {
    // Arrange
    // Act
    // Assert
}
```

## ut_run Command

The `ut_run` command runs all changed UT files (committed and uncommitted) in the current branch.

### ut_run Workflow

```
┌─────────────────────────────────────────────────────────────────┐
│  ut_run --base=origin/dev                                        │
└─────────────────────┬───────────────────────────────────────────┘
                      ▼
┌─────────────────────────────────────────────────────────────────┐
│  Step 1: Detect Changed UT Files                                 │
│  - git diff <base>...HEAD --name-only (committed changes)        │
│  - git status --porcelain (uncommitted changes)                  │
│  - Filter: *Tests.swift, *Test.kt, *_test.cpp                   │
└─────────────────────┬───────────────────────────────────────────┘
                      ▼
┌─────────────────────────────────────────────────────────────────┐
│  Step 2: Group by Platform                                       │
│  - iOS: *Tests.swift                                             │
│  - Android: *Test.kt                                             │
│  - C++: *_test.cpp                                               │
└─────────────────────┬───────────────────────────────────────────┘
                      ▼
┌─────────────────────────────────────────────────────────────────┐
│  Step 3: iOS Pre-build (if iOS tests exist)                      │
│  - Run: make ios_install_with_binary_cache                       │
│  - Wait for completion (monitor output)                          │
│  - Only proceed after successful completion                      │
└─────────────────────┬───────────────────────────────────────────┘
                      ▼
┌─────────────────────────────────────────────────────────────────┐
│  Step 4: Run Tests by Platform                                   │
│  - iOS: xcodebuild test -only-testing:<TestTarget>/<TestClass>  │
│  - Android: ./gradlew :module:testDebugUnitTest --tests "..."   │
│  - C++: cmake --build && ./build/tests/<test>_test              │
└─────────────────────┬───────────────────────────────────────────┘
                      ▼
┌─────────────────────────────────────────────────────────────────┐
│  Step 5: Report Results                                          │
│  - Show passed/failed count per platform                         │
│  - List failed tests with details                                │
│  - Total execution time                                          │
└─────────────────────────────────────────────────────────────────┘
```

### iOS Pre-build Requirement

For iOS tests, `make ios_install_with_binary_cache` MUST complete before running tests:

```bash
# Step 1: Run make command and wait for completion
cd app/ios/Glip
make ios_install_with_binary_cache 2>&1 | tee /tmp/ios_install.log

# Step 2: Check exit code
if [ $? -eq 0 ]; then
    echo "✅ iOS dependencies installed successfully"
else
    echo "❌ iOS dependency installation failed"
    # Show error and stop
    exit 1
fi

# Step 3: Only then run xcodebuild tests
xcodebuild test -workspace Glip.xcworkspace ...
```

### Example ut_run Usage

```
User: ut_run

AI: 
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
📊 ut_run Progress
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
[✅] Step 1: Detect Changed UT Files     (2.1s)
     Found 5 changed test files:
     - iOS: 3 files
     - Android: 1 file
     - C++: 1 file

[🔄] Step 2: iOS Pre-build               (in progress...)
     Running: make ios_install_with_binary_cache
     ...
[✅] Step 2: iOS Pre-build               (45.2s)

[🔄] Step 3: Run iOS Tests               (in progress...)
     Running 3 test classes...
[✅] Step 3: Run iOS Tests               (120.5s)
     ✅ ConferenceInteractorTests: 75 passed
     ✅ CallManagerTests: 42 passed
     ✅ VoIPServiceTests: 28 passed

[✅] Step 4: Run Android Tests           (15.3s)
     ✅ UserServiceTest: 12 passed

[✅] Step 5: Run C++ Tests               (8.7s)
     ✅ config_parser_test: 18 passed

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
✅ All 175 tests passed!
⏱️ Total Time: 191.8s
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
```

## Notes

- Always reference existing passing tests before writing new code
- When fixing errors, prioritize patterns from passing test cases
- The skill tracks progress and timing for each workflow step
- Compilation errors are never skipped - always analyzed and fixed
- For `ut_run`, iOS pre-build is mandatory and must complete before tests run

## Installation

```bash
npm install mthor-ut-skill
```

Or copy skill files to `.cursor/skills/ut-skill/` directory.

## Links

- npm: https://www.npmjs.com/package/mthor-ut-skill
- Author: eden_qu
