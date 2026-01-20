# UT Skill Workflow Implementation

## Command Recognition Pattern

When user input matches these patterns, trigger the UT Skill:

```regex
^ut_changed(\s+.*)?$
^ut_path\s+--path=.+$
^ut_files\s+--files=.+$
```

---

## Workflow Execution Rules

### ⚠️ Critical Rules

1. **NEVER end conversation** - Continue after every step, even after user choices
2. **Complete ALL steps** - Execute entire workflow without interruption
3. **Non-blocking prompts** - When user input needed, wait for response then continue
4. **Track progress** - Record start/end time for each step

### Progress Tracking

Initialize at workflow start:

```
workflow_start_time = now()
step_times = {}
current_step = 1
total_steps = 11
```

Display progress after each step:

```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
📊 UT Workflow Progress ({current_step}/{total_steps})
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
[✅] Step 1: Parse Command              ({step_times[1]}s)
[✅] Step 2: Detect Changed Files       ({step_times[2]}s)
[✅] Step 3: Filter Source Files        ({step_times[3]}s)
[✅] Step 4: Detect Platform            ({step_times[4]}s)
[✅] Step 5: Map to Test Paths          ({step_times[5]}s)
[🔄] Step 6: Pre-Implementation Analysis (in progress...)
[ ] Step 7: Generate/Update Tests
[ ] Step 8: Compile UT Files
[ ] Step 9: Build Prompt (if needed)
[ ] Step 10: Run Tests
[ ] Step 11: Coverage Report
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
⏱️ Elapsed: {elapsed}s
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
```

### User Choice Handling

When user choice is needed:

1. Display choice prompt with options
2. **WAIT** for user response (do NOT proceed without input)
3. After user responds, **immediately continue** to next step
4. **NEVER** end conversation after choice

```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
🔔 Action Required
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
{prompt_message}

Options:
  1️⃣  {option_1}
  2️⃣  {option_2}
  3️⃣  {option_3}

⏳ Waiting for your input...
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
```

---

## Step-by-Step Workflow

### Step 1: Parse Command

Extract parameters from user command:

```
Command: ut_changed [--base=<branch>] [--platform=<ios|android|cpp|all>]
         ut_path --path=<dir> [--platform=<ios|android|cpp|all>]
         ut_files --files="<file1,file2>" [--platform=<ios|android|cpp|all>]

Defaults:
  --base=origin/dev  (use remote branch to avoid stale local branch)
  --platform=all
```

### Step 2: Detect Changed Files

**For `ut_changed`:**
```bash
# 1. Get local changes
git status --porcelain | grep -E "^\s*[MADRCU]" | awk '{print $2}'

# 2. Fetch latest remote to ensure accurate comparison
git fetch origin dev 2>/dev/null

# 3. Get branch diff using remote branch (avoid stale local branch)
git diff ${base}...HEAD --name-only
# Default: git diff origin/dev...HEAD --name-only

# 4. Combine and deduplicate
```

**For `ut_path`:**
```bash
# Find all source files
find "${path}" -type f \( -name "*.swift" -o -name "*.kt" -o -name "*.java" -o -name "*.cpp" -o -name "*.hpp" \)
```

**For `ut_files`:**
```bash
# Validate each file exists
for file in ${files//,/ }; do
  [ -f "$file" ] && echo "$file"
done
```

### Step 3: Filter Source Files

**Include patterns:**
```
*Service*.{swift,kt,cpp}
*Manager*.{swift,kt,cpp}
*ViewModel*.{swift,kt}
*Presenter*.{swift,kt}
*Handler*.{swift,kt,cpp}
*Controller*.{swift,cpp}  (excluding ViewControllers)
*Utils*.{swift,kt,cpp}
*Helper*.{swift,kt}
```

**Exclude patterns:**
```
*Test*.{swift,kt,cpp}
*Tests.{swift,kt}
*_test.cpp
Mock*.{swift,kt,cpp}
Fake*.{swift,kt,cpp}
*View.swift
*Cell.swift
*ViewController.swift
*Activity.kt
*Fragment.kt
*Adapter.kt
```

### Step 4: Detect Platform

```
.swift          → iOS
.kt, .java      → Android
.cpp, .hpp, .h  → C++
```

### Step 5: Map to Test Paths

**iOS:**
```
Source: phone/ios/Phone/Service/PhoneService.swift
Test:   phone/ios/Phone/Tests/PhoneServiceTests.swift

Source: video/ios/Video/RCV/Meeting/MeetingController.swift
Test:   video/ios/Video/Tests/RCV/Meeting/MeetingControllerTests.swift

Rule: Find nearest Tests directory, append "Tests" to filename
```

**Android:**
```
Source: phone/android/module-phone/src/main/java/com/glip/phone/service/PhoneService.kt
Test:   phone/android/module-phone/src/test/java/com/glip/phone/service/PhoneServiceTest.kt

Rule: Replace 'main' with 'test', append "Test" to class name
```

**C++:**
```
Source: phone/cpp/core-phone/src/cc_lt/live_transcript_service.cpp
Test:   phone/cpp/core-phone/tests/src/cc_lt/live_transcript_service_test.cpp

Rule: Insert 'tests/' after module root, append "_test" before extension
```

### Step 6: Pre-Implementation Analysis

For each source file:

1. **Check Test File Existence:**
   ```bash
   if [ -f "$test_file" ]; then
     echo "UPDATE mode"
     HAS_EXISTING_UT=true
   else
     echo "CREATE mode"
     HAS_EXISTING_UT=false
   fi
   ```

2. **Determine Coverage Scope (for `ut_changed` command):**

   | Command | Has Existing UT? | Coverage Scope |
   |---------|------------------|----------------|
   | `ut_changed` | ✅ Yes | Cover ENTIRE file |
   | `ut_changed` | ❌ No | Cover ONLY CHANGED methods |
   | `ut_path` | Any | Cover ENTIRE file |
   | `ut_files` | Any | Cover ENTIRE file |

3. **For `ut_changed` with NO existing UT - Extract Changed Methods:**
   ```bash
   # Get detailed diff to identify changed functions/methods
   git diff ${base}...HEAD -- "$source_file" | grep -E "^@@.*@@" 
   
   # Parse diff hunks to find:
   # - New methods (added functions)
   # - Modified methods (changed functions)
   # - Ignore deleted methods (no tests needed)
   ```

   **iOS (Swift) - Parse changed methods:**
   ```bash
   # Extract function signatures from diff
   git diff ${base}...HEAD -- "$source_file" | grep -E "^\+.*func\s+\w+"
   ```

   **Android (Kotlin) - Parse changed methods:**
   ```bash
   git diff ${base}...HEAD -- "$source_file" | grep -E "^\+.*fun\s+\w+"
   ```

   **C++ - Parse changed functions:**
   ```bash
   git diff ${base}...HEAD -- "$source_file" | grep -E "^\+.*(void|bool|int|std::)\s+\w+\s*\("
   ```

4. **⚠️ CRITICAL: Find & Read Similar Tests FIRST (Before Reading Source):**
   
   ```bash
   # Step 4.1: Find ALL test files in same directory
   find $(dirname "$test_file") -name "*Tests.swift" -o -name "*Test.kt" -o -name "*_test.cpp"
   
   # Step 4.2: Read 2-3 similar tests to learn patterns
   # Focus on:
   # - Import statements
   # - Base class inheritance  
   # - Mock creation patterns (rcClassMock, docker, etc.)
   # - Stub configuration (rcObjectStub, stubber.when)
   # - Assertion styles
   ```

   **Extract These Patterns from Existing Tests:**
   | Pattern | iOS Example | Android Example |
   |---------|-------------|-----------------|
   | Mock Creation | `rcClassMock(for: Type.self)` | `docker<Type>()` |
   | Stub Setup | `rcObjectStub(for: mock) { stubber, mock in ... }` | `every { mock.method() } returns value` |
   | Return Value | `stubber.when(mock.method()).thenReturn(value)` | `returns value` |
   | Assertions | `XCTAssertEqual`, `XCTAssertTrue` | `assertEquals`, `assertTrue` |

5. **Read Source File:**
   - Extract all public methods/functions (for full coverage)
   - OR extract only changed methods (for partial coverage)
   - Identify dependencies (imports/includes)
   - Check class hierarchy
   - Assess testability

6. **Parse Existing Tests (if updating):**
   - Extract existing test method names
   - Identify covered scenarios
   - Find gaps in coverage

### Step 7: Generate/Update Tests

**⚠️ CRITICAL: Before Writing ANY Code, Reference Existing Tests:**

1. **Read at least 2 similar passing tests** in the same directory
2. **Copy EXACT patterns** for:
   - Import statements (don't guess, copy!)
   - Mock creation syntax (`rcClassMock`, `docker`, etc.)
   - Stub configuration (`rcObjectStub`, `stubber.when`, `thenReturn`)
   - Assertion patterns
3. **Match naming conventions** from existing tests
4. **Use same base class** as other tests in directory

```bash
# Before writing ConferenceInteractorTests.swift:
# 1. List existing tests
ls app/ios/Glip/GlipTests/Phone/Telephony/

# 2. Read similar test for patterns
cat app/ios/Glip/GlipTests/Phone/Telephony/CallManagerTests.swift | head -100

# 3. Extract mock patterns
grep -A 10 "rcClassMock\|rcObjectStub" CallManagerTests.swift
```

**Coverage Scope Decision:**

```
┌─────────────────────────────────────────────────────┐
│  Command Type?                                       │
└─────────────────────┬───────────────────────────────┘
                      │
    ┌─────────────────┴─────────────────┐
    ▼                                   ▼
┌───────────┐                     ┌─────────────┐
│ut_changed │                     │ut_path/files│
└─────┬─────┘                     └──────┬──────┘
      │                                  │
      ▼                                  ▼
┌─────────────────┐               ┌─────────────────┐
│Has Existing UT? │               │Cover ENTIRE file│
└────────┬────────┘               └─────────────────┘
         │
   ┌─────┴─────┐
   ▼           ▼
┌─────┐     ┌─────┐
│ Yes │     │ No  │
└──┬──┘     └──┬──┘
   │           │
   ▼           ▼
┌──────────┐ ┌───────────────────────┐
│ENTIRE    │ │ONLY CHANGED methods   │
│file      │ │(from git diff)        │
└──────────┘ └───────────────────────┘
```

**Coverage Requirements (within scope):**
- ✅ 100% Line Coverage of targeted methods
- ✅ 100% Branch Coverage of targeted methods

**Test Categories to Cover:**
1. Happy path (normal execution)
2. Error conditions (all error paths)
3. Edge cases (nil, empty, boundary values)
4. All branches (if/else, switch, guard)
5. Async operations (success, failure, timeout)

**For Partial Coverage (`ut_changed` + no existing UT):**
- Only test methods that appear in `git diff`
- Add comment header indicating partial coverage:
  ```swift
  // MARK: - Tests for changed methods only
  // Full coverage can be added with: ut_files --files="<filename>"
  ```

**Duplicate Prevention:**
- If test method exists with same scenario → SKIP
- If test method exists but outdated → UPDATE
- If no test exists → CREATE

**Track File Status:**
- Record which files are CREATED (new) vs UPDATED (existing)
- This determines whether dependency installation is needed

### Step 8: Compile UT Files (Required)

**After generating tests, MUST compile to verify syntax:**

**iOS:**
```bash
# Compile the test file to check for errors (does NOT run tests)
cd app/ios/Glip
xcodebuild build-for-testing \
  -workspace Glip.xcworkspace \
  -scheme <TestScheme> \
  -destination 'platform=iOS Simulator,id=<simulator_id>' \
  -only-testing:<TestTarget>/<TestClass> \
  2>&1 | tee /tmp/ut_compile.log

# Check for compilation errors
if grep -q "error:" /tmp/ut_compile.log; then
  echo "❌ Compilation failed - fix errors before proceeding"
  # Parse and fix errors, then retry compilation
fi
```

**Android:**
```bash
./gradlew :module-<name>:compileDebugUnitTestKotlin --info
```

**C++:**
```bash
cd <module>/cpp/core-<module>
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --target <test_target> 2>&1 | tee /tmp/ut_compile.log
```

**Compilation Loop:**
1. Attempt compilation
2. If errors found:
   - Parse error messages
   - Fix syntax/import/dependency issues
   - Retry compilation
3. Repeat until compilation succeeds
4. Only proceed to next step when ALL test files compile successfully

### Step 9: iOS Build Prompt (Conditional)

**⚠️ IMPORTANT: Only show this prompt when:**
1. Platform is iOS AND
2. At least one test file was CREATED (new file, not just updated)

**If only UPDATED existing test files → Skip this step entirely and proceed to Step 10**

**When new test files were created, display:**

```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
🔔 Action Required - iOS Build
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

📁 New files created:
   • <list of NEW test files>

📝 Files updated:
   • <list of UPDATED test files>

🔧 New test files detected. Dependency installation may be required.

Options:
   1️⃣  make ios_install_with_binary_cache (faster, recommended)
   2️⃣  make ios_install_with_source (slower, for new deps)
   3️⃣  Skip - I'll handle dependencies manually

⏳ Waiting for your input (1, 2, or 3)...
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
```

**⚠️ NON-BLOCKING PROMPT RULES:**
1. Display prompt and WAIT for user input
2. After user responds:
   - If 1 or 2: Execute chosen command, show progress
   - If 3: Skip installation
3. **IMMEDIATELY continue to Step 10** after handling choice
4. **NEVER end conversation** - workflow MUST complete all steps

### Step 10: Run Tests

**iOS:**
```bash
# Get available simulator
SIMULATOR_ID=$(xcrun simctl list devices available | grep "iPhone" | grep -E "\\(iOS 18" | head -1 | grep -oE "[A-F0-9-]{36}")

xcodebuild test \
  -workspace app/ios/Glip/Glip.xcworkspace \
  -scheme <TestScheme> \
  -destination "platform=iOS Simulator,id=${SIMULATOR_ID}" \
  -only-testing:<TestTarget>/<TestClass> \
  -enableCodeCoverage YES \
  2>&1 | tee /tmp/ut_test.log
```

**Android:**
```bash
./gradlew :module-<name>:testDebugUnitTest \
  --tests "com.glip.<module>.<package>.<TestClass>" \
  --info
```

**C++:**
```bash
cd <module>/cpp/core-<module>
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
./build/tests/<test_name>_test
```

### Step 11: Output Coverage Report

```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
📊 Unit Test Coverage Report
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

📁 <SourceFile>
   ├─ 📈 Line Coverage:   XX.X% (XX/XX lines)
   ├─ 🌿 Branch Coverage: XX.X% (XX/XX branches)
   └─ ✅ Status: PASS / ⚠️ NEEDS IMPROVEMENT

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
📋 Summary
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
   Total Files:    X
   Passing:        X (100% coverage)
   Needs Work:     X (incomplete coverage)
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
```

---

## Platform-Specific Templates

### iOS Test Template

```swift
import XCTest
@testable import <ModuleName>
import UnitTestCommon

final class <ClassName>Tests: XCBaseTestCase {
    // MARK: - Properties
    private var sut: <ClassName>!
    private var mock<Dependency>: Mock<Dependency>!

    // MARK: - Setup & Teardown
    override func setUp() {
        super.setUp()
        mock<Dependency> = Mock<Dependency>()
        sut = <ClassName>(dependency: mock<Dependency>)
    }

    override func tearDown() {
        sut = nil
        mock<Dependency> = nil
        super.tearDown()
    }

    // MARK: - <MethodName> Tests

    func test<MethodName>_<Scenario>_<ExpectedResult>() {
        // Arrange
        
        // Act
        
        // Assert
    }
}
```

### Android Test Template

```kotlin
package com.glip.<module>.<package>

import com.glip.BaseRobolectricTest
import org.junit.After
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith
import org.robolectric.RobolectricTestRunner
import kotlin.test.*

@RunWith(RobolectricTestRunner::class)
class <ClassName>Test : BaseRobolectricTest() {

    private lateinit var sut: <ClassName>
    private val mock<Dependency>: <DependencyType> = docker()

    @Before
    fun setup() {
        sut = <ClassName>(mock<Dependency>)
    }

    @After
    fun tearDown() {
        // cleanup
    }

    @Test
    fun test<MethodName><Scenario><ExpectedResult>() {
        // Arrange
        
        // Act
        
        // Assert
    }
}
```

### C++ Test Template

```cpp
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../../ut_base/init_glipcore_test.hpp"

#define private public
#define protected public
#include "<source_header>"
#undef private
#undef protected

using namespace glip_mobile;
using namespace ::testing;

class <ClassName>Test : public InitGlipCoreTest {
public:
    void SetUp() override {
        InitGlipCoreTest::SetUp();
        // Initialize SUT and mocks
    }

    void TearDown() override {
        // Cleanup
        InitGlipCoreTest::TearDown();
    }

protected:
    std::shared_ptr<<ClassName>> m_sut;
};

TEST_F(<ClassName>Test, test<FunctionName>_<Scenario>_<ExpectedResult>) {
    // Arrange
    
    // Act
    
    // Assert
}
```

---

## Error Handling

| Error | Detection | Resolution |
|-------|-----------|------------|
| No changes found | Empty file list | Inform user, suggest `ut_path` or `ut_files` |
| Test file exists | File check | Switch to UPDATE mode |
| Compilation failure | grep "error:" in log | Analyze error type, apply specific fix |
| Import/include error | Missing module/header | Add missing imports, check dependencies |
| Syntax error | Compiler error message | Fix syntax issue in generated code |
| Build failure | Non-zero exit | Suggest clean build, check dependencies |
| Test failure | Test exit code | Show failure details, suggest fixes |
| Missing coverage | Coverage < 100% | List uncovered lines/branches |

---

## Compilation Error Analysis & Resolution

### ⚠️ IMPORTANT: Never skip compilation errors. Always analyze and fix.

### Error Type Classification

When compilation fails, classify the error into one of these categories:

```
┌─────────────────────────────────────────────────────────────────────────┐
│ Error Type              │ Pattern                    │ Fix Strategy    │
├─────────────────────────┼────────────────────────────┼─────────────────┤
│ 1. Module Not Found     │ "No such module"           │ Add import      │
│ 2. Type Not Found       │ "Cannot find 'X' in scope" │ Fix type name   │
│ 3. Member Not Found     │ "has no member 'X'"        │ Check API       │
│ 4. Method Signature     │ "No exact matches"         │ Fix params      │
│ 5. Mock Configuration   │ "thenReturn" errors        │ Fix stub syntax │
│ 6. Access Control       │ "is inaccessible"          │ Use @testable   │
│ 7. Protocol Conformance │ "does not conform"         │ Implement method│
│ 8. Type Mismatch        │ "Cannot convert"           │ Fix type cast   │
└─────────────────────────┴────────────────────────────┴─────────────────┘
```

### Error Type 1: Module Not Found

**Pattern:**
```
error: No such module 'ModuleName'
```

**Resolution Steps:**
1. Search codebase for similar test files that import the module
2. Check if module name is correct (case-sensitive)
3. Add missing import statement
4. If module doesn't exist, find the correct module that contains the type

**Example Fix:**
```swift
// Before (missing import)
@testable import PhoneImplement

// After (add missing module)
@testable import PhoneImplement
import PhoneInterface  // Add this for GlipIMultiPartyConferenceSessionInfo
```

### Error Type 2: Type Not Found (Cannot find 'X' in scope)

**Pattern:**
```
error: Cannot find 'TypeName' in scope
```

**Resolution Steps:**
1. Search codebase: `grep -r "class TypeName\|struct TypeName\|protocol TypeName\|typealias TypeName"`
2. Check if type is in a different module → add import
3. Check if type name is misspelled → fix spelling
4. Check if type is internal/private → use protocol or public interface
5. If type is from Objective-C → check bridging header

**Example Fix:**
```swift
// Error: Cannot find 'GlipMultiPartyConferenceSessionInfo' in scope
// Search reveals: GlipIMultiPartyConferenceSessionInfo (note the 'I' prefix)

// Fix: Use correct type name
let mockSession = rcClassMock(for: GlipIMultiPartyConferenceSessionInfo.self)
```

### Error Type 3: Member Not Found (has no member)

**Pattern:**
```
error: Value of type 'X' has no member 'Y'
```

**Resolution Steps:**
1. Check the actual class/protocol definition for correct method/property name
2. Search: `grep -r "func methodName\|var propertyName" path/to/source`
3. Check if the member is in an extension
4. Check if the member exists only in subclass
5. Update to use correct member name or alternative API

**Example Fix:**
```swift
// Error: Value of type 'NSObject' has no member 'getId'
// The mock type is wrong - should mock the protocol, not NSObject

// Fix: Check actual protocol and use correct mock
let mockSession: GlipIMultiPartyConferenceSessionInfo = ...
mockSession.sessionId  // Use actual property name
```

### Error Type 4: Method Signature Mismatch (No exact matches)

**Pattern:**
```
error: No exact matches in call to instance method 'methodName'
```

**Resolution Steps:**
1. Check the actual method signature in source code
2. Verify parameter types and order
3. Check if method has been updated/deprecated
4. Verify return type matches

**Example Fix:**
```swift
// Error: No exact matches in call to instance method 'thenReturn'
// The stub return type doesn't match

// Before
stubber.when(mock.uuid() as NSString?).thenReturn("uuid-string")

// After (return type should match method signature)
stubber.when(mock.uuid()).thenReturn("uuid-string" as NSString)
```

### Error Type 5: Mock Configuration Issues

**Pattern:**
```
error: No exact matches in call to 'thenReturn'
error: Cannot convert value of type 'X' to expected argument type 'Y'
```

**Resolution Steps:**
1. Check what the original method returns
2. Ensure mock return type matches exactly
3. For optionals, wrap in Optional explicitly
4. For protocol types, ensure mock conforms

**Example Fix:**
```swift
// For methods returning optional NSString
stubber.when(mock.getId()).thenReturn("session-123" as NSString?)

// For methods returning protocol types
stubber.when(mock.getSession()).thenReturn(mockSession as GlipIMultiPartyConferenceSessionInfo?)
```

### Error Type 6: Access Control Issues

**Pattern:**
```
error: 'X' is inaccessible due to 'internal' protection level
```

**Resolution Steps:**
1. Ensure `@testable import ModuleName` is used
2. Check if the type/method is truly internal or private
3. If private, test through public interface instead
4. Consider if the test design needs adjustment

### Error Type 7: Protocol Conformance

**Pattern:**
```
error: Type 'X' does not conform to protocol 'Y'
```

**Resolution Steps:**
1. Check all required protocol methods
2. Add missing method implementations
3. Verify method signatures match exactly
4. Check for associated types

### Error Type 8: Type Mismatch

**Pattern:**
```
error: Cannot convert value of type 'X' to expected argument type 'Y'
```

**Resolution Steps:**
1. Check expected type in method signature
2. Add explicit type cast if needed
3. Use correct initializer or factory method
4. Verify generic type parameters

---

## Compilation Error Resolution Workflow

### ⚠️ PRIORITY: Search Passing Tests First!

When fixing errors, **ALWAYS** search existing passing tests before searching source code:

```bash
# 1. Find passing tests that use the same type/method
grep -rn "GlipIMultiPartyConferenceSessionInfo" GlipTests/ | grep -v "error"

# 2. Find tests with similar mock patterns
grep -A 10 "rcClassMock.*for:.*RCRTCCall" GlipTests/Phone/ | head -30

# 3. Find correct stub syntax from passing tests
grep -B 2 -A 5 "thenReturn" GlipTests/Phone/*Tests.swift | head -50

# 4. Copy EXACT pattern from passing test
```

**Why This Works:**
- Passing tests have PROVEN correct syntax
- Same project imports & dependencies
- No guessing at API behavior
- Faster fix iteration

### Resolution Flow

```
┌─────────────────────────────────────────────┐
│  Compilation Failed                          │
└─────────────────┬───────────────────────────┘
                  ▼
┌─────────────────────────────────────────────┐
│  Parse Error Messages                        │
│  - Extract error type                        │
│  - Extract file and line number              │
│  - Extract problematic code                  │
└─────────────────┬───────────────────────────┘
                  ▼
┌─────────────────────────────────────────────┐
│  Classify Error Type (1-8)                   │
└─────────────────┬───────────────────────────┘
                  ▼
┌─────────────────────────────────────────────┐
│  ⭐ FIRST: Search Passing Tests             │
│  - Find tests using same types              │
│  - Find tests with similar patterns         │
│  - Copy exact working syntax                 │
└─────────────────┬───────────────────────────┘
                  ▼
┌─────────────────────────────────────────────┐
│  Search for Correct Usage (Priority Order)   │
│  1. FIRST: Find PASSING tests with same      │
│     type/pattern - copy their exact syntax   │
│  2. THEN: Check source for type definitions  │
│  3. LAST: Check imports in other tests       │
└─────────────────┬───────────────────────────┘
                  ▼
┌─────────────────────────────────────────────┐
│  Apply Fix Based on Error Type               │
│  - Copy EXACT pattern from passing test      │
│  - Edit the test file                        │
│  - Add/modify imports                        │
│  - Fix type names/method calls               │
└─────────────────┬───────────────────────────┘
                  ▼
┌─────────────────────────────────────────────┐
│  Retry Compilation                           │
└─────────────────┬───────────────────────────┘
                  ▼
          ┌──────────────┐
          │ Still Errors?│
          └──────┬───────┘
                 │
       ┌─────────┴─────────┐
       ▼                   ▼
   ┌───────┐           ┌───────┐
   │  Yes  │           │  No   │
   └───┬───┘           └───┬───┘
       │                   │
       ▼                   ▼
┌──────────────────┐ ┌───────────────────────────┐
│ Repeat Analysis  │ │ Proceed to Run Tests      │
│ (max 5 attempts) │ └───────────────────────────┘
└──────────────────┘
```

### Maximum Retry Attempts

- Set `MAX_COMPILE_RETRIES = 5`
- After 5 failed attempts, display detailed error report and ask user for guidance
- NEVER silently skip errors

## Compilation Error Resolution Flow

```
┌─────────────────────────────────────┐
│  Generate/Update Test Files         │
└─────────────────┬───────────────────┘
                  ▼
┌─────────────────────────────────────┐
│  Compile Test Files                  │
└─────────────────┬───────────────────┘
                  ▼
          ┌──────────────┐
          │ Errors?      │
          └──────┬───────┘
                 │
       ┌─────────┴─────────┐
       ▼                   ▼
   ┌───────┐           ┌───────┐
   │  Yes  │           │  No   │
   └───┬───┘           └───┬───┘
       │                   │
       ▼                   │
┌──────────────────┐       │
│ Parse Error Msgs │       │
│ Fix Issues       │       │
│ Re-generate Code │       │
└────────┬─────────┘       │
         │                 │
         └──────┬──────────┘
                ▼
        ┌───────────────┐
        │ Retry Compile │
        └───────┬───────┘
                │
                ▼ (loop until success)
        ┌───────────────────────────┐
        │ Proceed to Next Step      │
        └───────────────────────────┘
```

---

## Workflow Completion Summary

**After ALL steps complete, display final summary:**

```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
✅ UT Workflow Complete
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

📊 Step Timing:
   ├─ Step 1:  Parse Command              {time}s
   ├─ Step 2:  Detect Changed Files       {time}s
   ├─ Step 3:  Filter Source Files        {time}s
   ├─ Step 4:  Detect Platform            {time}s
   ├─ Step 5:  Map to Test Paths          {time}s
   ├─ Step 6:  Pre-Implementation         {time}s
   ├─ Step 7:  Generate/Update Tests      {time}s
   ├─ Step 8:  Compile UT Files           {time}s
   ├─ Step 9:  Build (if needed)          {time}s
   ├─ Step 10: Run Tests                  {time}s
   └─ Step 11: Coverage Report            {time}s

📁 Files Summary:
   ├─ Source files analyzed:  X
   ├─ Test files created:     X
   ├─ Test files updated:     X
   └─ Total test methods:     X

📈 Coverage Summary:
   ├─ Line Coverage:    XX.X%
   └─ Branch Coverage:  XX.X%

⏱️ Total Time: {total_time}s
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
```

**⚠️ IMPORTANT:**
- Workflow is complete but conversation continues
- User can ask follow-up questions or run new commands
- Do NOT terminate or end the conversation
